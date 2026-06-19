#include "Track.h"
#include "Transport.h"

namespace Nimbus {

Track::Track(Transport* t) : transport(t) {}

void Track::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

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
    if (muted_.load(std::memory_order_relaxed)) {
        trackBuffer.clear();
        meter.processBlock(trackBuffer);
        return;
    }

    trackBuffer.clear();
    trackMidiBuffer.clear();

    // 1. Process the source generator into the track buffer
    if (source) {
        source->processBlock(trackBuffer, trackMidiBuffer);
    }

    // Route live MIDI input only if armed
    if (armed_.load(std::memory_order_relaxed)) {
        trackMidiBuffer.addEvents(midiMessages, 0, trackBuffer.getNumSamples(), 0);
    }

    // 1.5 Process instrument plugin (synth consumes MIDI, produces audio)
    if (instrument) {
        instrument->processBlock(trackBuffer, trackMidiBuffer);
        
        // Mute instrument if track is disarmed and transport is stopped.
        // This prevents the instrument from making sounds if the user interacts with its UI while disarmed.
        if (!armed_.load(std::memory_order_relaxed) && transport != nullptr && !transport->isPlaying()) {
            trackBuffer.clear();
        }
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
    source = std::move(sourceNode);
    if (source && currentSampleRate > 0) {
        source->prepareToPlay(currentSampleRate, currentBlockSize);
    }
}

void Track::setInstrumentPlugin(std::unique_ptr<IAudioNode> instrumentNode) {
    instrument = std::move(instrumentNode);
    if (instrument && currentSampleRate > 0) {
        instrument->prepareToPlay(currentSampleRate, currentBlockSize);
    }
}

void Track::addInsertPlugin(std::unique_ptr<IAudioNode> pluginNode) {
    insertGraph.addNode(std::move(pluginNode));
}

void Track::removeInsertPlugin(IAudioNode* pluginNode) {
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
