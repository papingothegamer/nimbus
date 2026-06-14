#include "AudioGraph.h"

namespace Nimbus {

AudioGraph::AudioGraph()
    : nodeAddQueue(128) // Capacity for pending nodes
{
}

AudioGraph::~AudioGraph() = default;

void AudioGraph::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    for (auto& node : nodes) {
        node->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    }
}

void AudioGraph::releaseResources() {
    for (auto& node : nodes) {
        node->releaseResources();
    }
}

void AudioGraph::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // 1. Process pending nodes from the queue (Lock-free)
    std::unique_ptr<IAudioNode> pendingNode;
    while (nodeAddQueue.pop(pendingNode)) {
        if (pendingNode) {
            // Must prepare it on the audio thread if it was added during playback
            // (In a real DAW, you'd double buffer the graph to do this on a background thread,
            // but this is safe as long as prepareToPlay in TestToneNode doesn't allocate)
            pendingNode->prepareToPlay(currentSampleRate, currentBlockSize);
            nodes.push_back(std::move(pendingNode));
        }
    }

    // 2. Process all nodes sequentially
    for (auto& node : nodes) {
        node->processBlock(buffer, midiMessages);
    }
}

void AudioGraph::addNode(std::unique_ptr<IAudioNode> newNode) {
    // Push the node into the lock-free queue so the audio thread picks it up
    bool success = nodeAddQueue.push(std::move(newNode));
    jassert(success && "Node add queue is full!");
}

} // namespace Nimbus
