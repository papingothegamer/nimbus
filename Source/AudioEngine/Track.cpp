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
    tempBuffer.setSize(isStereo_ ? 2 : 1, maximumExpectedSamplesPerBlock);
    groupBuffer_.setSize(isStereo_ ? 2 : 1, maximumExpectedSamplesPerBlock);
    groupBuffer_.clear();

    if (source) source->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    if (instrument) instrument->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    midiInsertGraph.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    insertGraph.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    fader.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    outputDelayLine.prepare(sampleRate, maximumExpectedSamplesPerBlock);
}

void Track::releaseResources() {
    if (source) source->releaseResources();
    if (instrument) instrument->releaseResources();
    midiInsertGraph.releaseResources();
    insertGraph.releaseResources();
    fader.releaseResources();
}

void Track::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    const juce::SpinLock::ScopedLockType sl(processLock);

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = isStereo_ ? 2 : 1;

    if (numSamples > currentBlockSize || trackBuffer.getNumChannels() < numChannels) {
        buffer.clear();
        return;
    }

    tempBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
    tempBuffer.clear();

    juce::MidiBuffer tempMidi;

    // 1. If this is a Group track, mix in the groupBuffer (which has the summed children)
    if (isGroup_) {
        for (int ch = 0; ch < tempBuffer.getNumChannels(); ++ch) {
            if (ch < groupBuffer_.getNumChannels()) {
                tempBuffer.addFrom(ch, 0, groupBuffer_, ch, 0, numSamples);
            }
        }
    } else {
        juce::AudioBuffer<float> trackBlock(trackBuffer.getArrayOfWritePointers(), numChannels, numSamples);
        trackBlock.clear();
        trackMidiBuffer.clear();

        // 1. Process the source generator into the track buffer
        if (source) {
            source->processBlock(trackBlock, trackMidiBuffer);
        }

        // Add any UI-injected MIDI messages
        uiMidiCollector.removeNextBlockOfMessages(trackMidiBuffer, numSamples);

        // Route live MIDI input only if armed
        if (armed_.load(std::memory_order_relaxed)) {
            bool shouldProcessMidi = (instrument != nullptr);
            
            if (shouldProcessMidi) {
                trackMidiBuffer.addEvents(midiMessages, 0, numSamples, 0);
            }
            
            // Route live AUDIO input if no instrument is present
            if (inputBufferPtr != nullptr && instrument == nullptr) {
                int inCh = inputChannelIndex_.load(std::memory_order_relaxed);
                if (inCh == -1) {
                    for (int ch = 0; ch < std::min(trackBlock.getNumChannels(), inputBufferPtr->getNumChannels()); ++ch) {
                        trackBlock.addFrom(ch, 0, *inputBufferPtr, ch, 0, numSamples);
                    }
                } else if (inCh >= 0 && inCh < inputBufferPtr->getNumChannels()) {
                    for (int ch = 0; ch < trackBlock.getNumChannels(); ++ch) {
                        trackBlock.addFrom(ch, 0, *inputBufferPtr, inCh, 0, numSamples);
                    }
                }
            }
            
            // Record the live input if transport is recording
            if (recorder_ != nullptr && transport != nullptr && transport->isRecording()) {
                recorder_->pushSamples(trackBlock, numSamples);
            }
            
            if (midiRecorder_ != nullptr && transport != nullptr && transport->isRecording() && shouldProcessMidi) {
                double startPos = transport->getCurrentPosition();
                midiRecorder_->pushEvents(midiMessages, numSamples, startPos);
            }
        }

        // 1.2 Process MIDI Effects
        juce::AudioBuffer<float> dummyAudio(0, numSamples);
        midiInsertGraph.processBlock(dummyAudio, trackMidiBuffer);

        // 1.5 Process instrument plugin (synth consumes MIDI, produces audio)
        if (instrument) {
            instrument->processBlock(trackBlock, trackMidiBuffer);
            if (!armed_.load(std::memory_order_relaxed) && transport != nullptr && !transport->isPlaying() && source == nullptr) {
                trackBlock.clear();
            }
        }
        
        // Record MIDI if recording
        if (midiRecorder_) {
            double blockStartPos = transport ? transport->getCurrentPosition() : 0.0;
            midiRecorder_->pushEvents(trackMidiBuffer, numSamples, blockStartPos);
        }
        
        // Copy standard track block into tempBuffer
        for (int ch = 0; ch < tempBuffer.getNumChannels(); ++ch) {
            if (ch < trackBlock.getNumChannels()) {
                tempBuffer.copyFrom(ch, 0, trackBlock, ch, 0, numSamples);
            }
        }
        tempMidi.addEvents(trackMidiBuffer, 0, numSamples, 0);
    }

    // 3. Apply inserts (Group FX or standard track FX)
    insertGraph.processBlock(tempBuffer, tempMidi);

    // 4. Apply track gain and Pan
    if (!isStereo_ && buffer.getNumChannels() >= 2) {
        juce::AudioBuffer<float> stereoBlock(stereoPanBuffer.getArrayOfWritePointers(), 2, numSamples);
        stereoBlock.copyFrom(0, 0, tempBuffer, 0, 0, numSamples);
        stereoBlock.copyFrom(1, 0, tempBuffer, 0, 0, numSamples);
        
        fader.processBlock(stereoBlock, tempMidi);
        
        if (!armed_.load(std::memory_order_relaxed)) {
            outputDelayLine.process(stereoBlock);
        }
        
        if (!muted_.load(std::memory_order_relaxed) && !silencedBySolo_.load(std::memory_order_relaxed)) {
            for (int ch = 0; ch < 2; ++ch) {
                buffer.addFrom(ch, 0, stereoBlock, ch, 0, numSamples);
            }
        }
    } else {
        fader.processBlock(tempBuffer, tempMidi);
        
        if (!armed_.load(std::memory_order_relaxed)) {
            outputDelayLine.process(tempBuffer);
        }
        
        if (!muted_.load(std::memory_order_relaxed) && !silencedBySolo_.load(std::memory_order_relaxed)) {
            for (int ch = 0; ch < std::min(buffer.getNumChannels(), tempBuffer.getNumChannels()); ++ch) {
                buffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);
            }
        }
    }

    // Measure peak level from the output
    meter.processBlock(tempBuffer);
}

int Track::getLatencySamples() const {
    const juce::SpinLock::ScopedLockType sl(processLock);
    int totalLatency = 0;
    if (instrument) totalLatency += instrument->getLatencySamples();
    totalLatency += midiInsertGraph.getLatencySamples();
    totalLatency += insertGraph.getLatencySamples();
    return totalLatency;
}

void Track::setSourceNode(std::unique_ptr<IAudioNode> sourceNode) {
    if (sourceNode && currentSampleRate > 0) {
        sourceNode->prepareToPlay(currentSampleRate, currentBlockSize);
    }
    
    std::unique_ptr<IAudioNode> oldSource;
    {
        const juce::SpinLock::ScopedLockType sl(processLock);
        oldSource = std::move(source);
        source = std::move(sourceNode);
    }
    // oldSource is safely deleted here, outside the spinlock
}

void Track::addMidiInsertPlugin(std::unique_ptr<IAudioNode> pluginNode) {
    if (pluginNode && currentSampleRate > 0) {
        pluginNode->prepareToPlay(currentSampleRate, currentBlockSize);
    }
    const juce::SpinLock::ScopedLockType sl(processLock);
    midiInsertGraph.addNode(std::move(pluginNode));
}

void Track::removeMidiInsertPlugin(IAudioNode* pluginNode) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    midiInsertGraph.removeNode(pluginNode);
}

void Track::setInstrumentPlugin(std::unique_ptr<IAudioNode> instrumentNode) {
    if (instrumentNode && currentSampleRate > 0) {
        instrumentNode->prepareToPlay(currentSampleRate, currentBlockSize);
    }
    
    std::unique_ptr<IAudioNode> oldInstrument;
    {
        const juce::SpinLock::ScopedLockType sl(processLock);
        oldInstrument = std::move(instrument);
        instrument = std::move(instrumentNode);
    }
    // oldInstrument is safely deleted here, outside the spinlock
}

void Track::addInsertPlugin(std::unique_ptr<IAudioNode> pluginNode) {
    if (pluginNode && currentSampleRate > 0) {
        pluginNode->prepareToPlay(currentSampleRate, currentBlockSize);
    }
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
void Track::setArmed(bool shouldBeArmed) {
    armed_.store(shouldBeArmed);
}

void Track::setCompensationDelay(int samples) {
    outputDelayLine.setDelaySamples(samples);
}

void Track::clearGroupBuffer() {
    groupBuffer_.clear();
}

} // namespace Nimbus
