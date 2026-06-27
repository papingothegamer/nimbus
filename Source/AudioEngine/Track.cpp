#include "Track.h"
#include "Transport.h"
#include "AudioRecorder.h"
#include "MidiRecorder.h"

namespace Nimbus {

Track::Track(Transport* t) : transport(t) {}

void Track::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    uiMidiCollector.reset(sampleRate);

    trackBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    
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

    if (muted_.load(std::memory_order_relaxed)) {
        trackBuffer.clear();
        meter.processBlock(trackBuffer);
        return;
    }

    trackBuffer.setSize(2, buffer.getNumSamples(), true, false, true);
    trackBuffer.clear();
    trackMidiBuffer.clear();

    // 1. Process the source generator into the track buffer
    if (source) {
        source->processBlock(trackBuffer, trackMidiBuffer);
        if (!trackMidiBuffer.isEmpty()) {
            static int logCounter = 0;
            if (logCounter++ % 10 == 0) { // Log occasionally to prevent spam
                juce::Logger::writeToLog("Track has " + juce::String(trackMidiBuffer.getNumEvents()) + " MIDI events from source!");
            }
        }
    }

    // Add any UI-injected MIDI messages
    uiMidiCollector.removeNextBlockOfMessages(trackMidiBuffer, trackBuffer.getNumSamples());

    // Route live MIDI input only if armed
    if (armed_.load(std::memory_order_relaxed)) {
        trackMidiBuffer.addEvents(midiMessages, 0, trackBuffer.getNumSamples(), 0);
        
        // Route live AUDIO input if no instrument is present
        if (inputBufferPtr != nullptr && instrument == nullptr) {
            int inCh = inputChannelIndex_.load(std::memory_order_relaxed);
            if (inCh == -1) {
                for (int ch = 0; ch < std::min(trackBuffer.getNumChannels(), inputBufferPtr->getNumChannels()); ++ch) {
                    trackBuffer.addFrom(ch, 0, *inputBufferPtr, ch, 0, trackBuffer.getNumSamples());
                }
            } else if (inCh >= 0 && inCh < inputBufferPtr->getNumChannels()) {
                // Duplicate mono input across all track channels
                for (int ch = 0; ch < trackBuffer.getNumChannels(); ++ch) {
                    trackBuffer.addFrom(ch, 0, *inputBufferPtr, inCh, 0, trackBuffer.getNumSamples());
                }
            }
        }
        
        // Record the live input if transport is recording
        if (recorder_ != nullptr && transport != nullptr && transport->isRecording()) {
            recorder_->pushSamples(trackBuffer, trackBuffer.getNumSamples());
        }
        
        if (midiRecorder_ != nullptr && transport != nullptr && transport->isRecording()) {
            double startPos = transport->getCurrentPosition();
            midiRecorder_->pushEvents(midiMessages, trackBuffer.getNumSamples(), startPos);
        }
    }

    // 1.5 Process instrument plugin (synth consumes MIDI, produces audio)
    if (instrument) {
        instrument->processBlock(trackBuffer, trackMidiBuffer);
        
        // Only mute instrument output when transport is stopped AND track is disarmed
        // This prevents stray sounds from plugin UI interaction, but allows playback of clips
        if (!armed_.load(std::memory_order_relaxed) && transport != nullptr && !transport->isPlaying() && source == nullptr) {
            trackBuffer.clear();
        }
    }
    
    // Record MIDI if recording
    if (midiRecorder_) {
        double blockStartPos = transport ? transport->getCurrentPosition() : 0.0;
        midiRecorder_->pushEvents(trackMidiBuffer, trackBuffer.getNumSamples(), blockStartPos);
    }

    // 2. Process insert plugins
    insertGraph.processBlock(trackBuffer, trackMidiBuffer);

    // 3. Apply volume and panning
    fader.processBlock(trackBuffer, trackMidiBuffer);

    // 4. Update the level meter
    meter.processBlock(trackBuffer);

    // 5. Sum the isolated track buffer into the master buffer
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        if (ch < trackBuffer.getNumChannels()) {
            buffer.addFrom(ch, 0, trackBuffer, ch, 0, trackBuffer.getNumSamples());
        }
    }
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
