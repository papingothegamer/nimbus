#include "Track.h"

namespace Nimbus {

Track::Track() = default;

void Track::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    trackBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    
    if (source) source->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    insertGraph.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    fader.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void Track::releaseResources() {
    if (source) source->releaseResources();
    insertGraph.releaseResources();
    fader.releaseResources();
}

void Track::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    trackBuffer.clear();

    // 1. Process the source generator into the track buffer
    if (source) {
        source->processBlock(trackBuffer, midiMessages);
    }

    // 2. Process insert plugins
    insertGraph.processBlock(trackBuffer, midiMessages);

    // 3. Apply volume and panning
    fader.processBlock(trackBuffer, midiMessages);

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

} // namespace Nimbus
