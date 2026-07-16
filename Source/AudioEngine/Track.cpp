#include "Track.h"
#include "Transport.h"
#include "AudioRecorder.h"
#include "MidiRecorder.h"

namespace Nimbus {

Track::Track(TrackID id, bool isStereo, Transport* t) : id_(id), isStereo_(isStereo), transport(t) {}

void Track::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    uiMidiCollector.reset(sampleRate);

    trackBuffer.setSize(isStereo_ ? 2 : 1, maximumExpectedSamplesPerBlock);
    stereoPanBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    
    if (source) source->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    if (instrument) instrument->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    insertGraph.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    fader.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void Track::releaseResources() {
    if (source) source->releaseResources();
    if (instrument) instrument->releaseResources();
    insertGraph.releaseResources();
    fader.releaseResources();
}

void Track::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    const juce::SpinLock::ScopedLockType sl(processLock);

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = isStereo_ ? 2 : 1;
    // Buffers are allocated once in prepareToPlay.  A device changing its block
    // size must re-enter prepareToPlay; allocating on this callback would cause
    // an audible real-time priority fault.
    if (numSamples > currentBlockSize || trackBuffer.getNumChannels() < numChannels) {
        buffer.clear();
        return;
    }

    juce::AudioBuffer<float> trackBlock(trackBuffer.getArrayOfWritePointers(), numChannels, numSamples);

    if (muted_.load(std::memory_order_relaxed)) {
        trackBlock.clear();
        meter.processBlock(trackBlock);
        return;
    }

    trackBlock.clear();
    trackMidiBuffer.clear();

    // 1. Process the source generator into the track buffer
    if (source) {
        source->processBlock(trackBlock, trackMidiBuffer);
        if (!trackMidiBuffer.isEmpty()) {
            static int logCounter = 0;
            if (logCounter++ % 10 == 0) { // Log occasionally to prevent spam
                juce::Logger::writeToLog("Track has " + juce::String(trackMidiBuffer.getNumEvents()) + " MIDI events from source!");
            }
        }
    }

    // Add any UI-injected MIDI messages
    uiMidiCollector.removeNextBlockOfMessages(trackMidiBuffer, numSamples);

    // Route live MIDI input only if armed
    if (armed_.load(std::memory_order_relaxed)) {
        trackMidiBuffer.addEvents(midiMessages, 0, numSamples, 0);
        
        // Route live AUDIO input if no instrument is present
        if (inputBufferPtr != nullptr && instrument == nullptr) {
            int inCh = inputChannelIndex_.load(std::memory_order_relaxed);
            if (inCh == -1) {
                for (int ch = 0; ch < std::min(trackBlock.getNumChannels(), inputBufferPtr->getNumChannels()); ++ch) {
                    trackBlock.addFrom(ch, 0, *inputBufferPtr, ch, 0, numSamples);
                }
            } else if (inCh >= 0 && inCh < inputBufferPtr->getNumChannels()) {
                // Duplicate mono input across all track channels
                for (int ch = 0; ch < trackBlock.getNumChannels(); ++ch) {
                    trackBlock.addFrom(ch, 0, *inputBufferPtr, inCh, 0, numSamples);
                }
            }
        }
        
        // Record the live input if transport is recording
        if (recorder_ != nullptr && transport != nullptr && transport->isRecording()) {
            recorder_->pushSamples(trackBlock, numSamples);
        }
        
        if (midiRecorder_ != nullptr && transport != nullptr && transport->isRecording()) {
            double startPos = transport->getCurrentPosition();
            midiRecorder_->pushEvents(midiMessages, numSamples, startPos);
        }
    }

    // 1.5 Process instrument plugin (synth consumes MIDI, produces audio)
    if (instrument) {
        instrument->processBlock(trackBlock, trackMidiBuffer);
        
        // Only mute instrument output when transport is stopped AND track is disarmed
        // This prevents stray sounds from plugin UI interaction, but allows playback of clips
        if (!armed_.load(std::memory_order_relaxed) && transport != nullptr && !transport->isPlaying() && source == nullptr) {
            trackBlock.clear();
        }
    }
    
    // Record MIDI if recording
    if (midiRecorder_) {
        double blockStartPos = transport ? transport->getCurrentPosition() : 0.0;
        midiRecorder_->pushEvents(trackMidiBuffer, numSamples, blockStartPos);
    }

    // 2. Process insert plugins
    insertGraph.processBlock(trackBlock, trackMidiBuffer);

    // 3. Apply Track Fader and Pan
    // If the track is mono, we upmix it to a stereo buffer before passing to the GainNode so panning works.
    if (!isStereo_ && buffer.getNumChannels() >= 2) {
        juce::AudioBuffer<float> stereoBlock(stereoPanBuffer.getArrayOfWritePointers(), 2, numSamples);
        stereoBlock.copyFrom(0, 0, trackBlock, 0, 0, numSamples);
        stereoBlock.copyFrom(1, 0, trackBlock, 0, 0, numSamples);
        
        fader.processBlock(stereoBlock, trackMidiBuffer);
        
        for (int ch = 0; ch < 2; ++ch) {
            buffer.addFrom(ch, 0, stereoBlock, ch, 0, numSamples);
        }
    } else {
        fader.processBlock(trackBlock, trackMidiBuffer);
        
        for (int ch = 0; ch < std::min(buffer.getNumChannels(), trackBlock.getNumChannels()); ++ch) {
            buffer.addFrom(ch, 0, trackBlock, ch, 0, numSamples);
        }
    }

    // 5. Update Level Meter (Meter gets the pre-fader track buffer so we see signal even if volume is 0)
    meter.processBlock(trackBlock);
}

int Track::getLatencySamples() const {
    int totalLatency = 0;
    if (source) totalLatency += source->getLatencySamples();
    totalLatency += insertGraph.getLatencySamples();
    return totalLatency;
}

void Track::setSourceNode(std::unique_ptr<IAudioNode> sourceNode) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    source = std::move(sourceNode);
    if (source && currentSampleRate > 0) {
        source->prepareToPlay(currentSampleRate, currentBlockSize);
    }
}

void Track::setInstrumentPlugin(std::unique_ptr<IAudioNode> instrumentNode) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    instrument = std::move(instrumentNode);
    if (instrument && currentSampleRate > 0) {
        instrument->prepareToPlay(currentSampleRate, currentBlockSize);
    }
}

void Track::addInsertPlugin(std::unique_ptr<IAudioNode> pluginNode) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    insertGraph.addNode(std::move(pluginNode));
}

void Track::removeInsertPlugin(IAudioNode* pluginNode) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    insertGraph.removeNode(pluginNode);
}

void Track::setVolume(float gainLinear) {
    fader.setGainLinear(gainLinear);
}

void Track::setPan(float panValue) {
    fader.setPan(panValue);
}

void Track::setMuted(bool muted) { muted_.store(muted); }
void Track::setSoloed(bool soloed) { soloed_.store(soloed); }
void Track::setArmed(bool armed) { armed_.store(armed); }

} // namespace Nimbus
