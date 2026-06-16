#include "AudioGraph.h"

namespace Nimbus {

AudioGraph::AudioGraph() : nodeAddQueue(128), nodeRemoveQueue(128) {}

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
    // 1. Drain the add queue and add new nodes to our vector
    std::unique_ptr<IAudioNode> pendingNode;
    while (nodeAddQueue.pop(pendingNode)) {
        if (pendingNode) {
            if (currentSampleRate > 0) {
                pendingNode->prepareToPlay(currentSampleRate, currentBlockSize);
            }
            nodes.push_back(std::move(pendingNode));
        }
    }
    
    // 2. Drain the remove queue
    IAudioNode* pendingRemove = nullptr;
    while (nodeRemoveQueue.pop(pendingRemove)) {
        if (pendingRemove) {
            auto it = std::remove_if(nodes.begin(), nodes.end(),
                [pendingRemove](const std::unique_ptr<IAudioNode>& ptr) {
                    return ptr.get() == pendingRemove;
                });
            nodes.erase(it, nodes.end());
        }
    }

    // 3. Process all active nodes
    for (auto& node : nodes) {
        node->processBlock(buffer, midiMessages);
    }
}

void AudioGraph::addNode(std::unique_ptr<IAudioNode> newNode) {
    if (newNode) {
        nodeAddQueue.push(std::move(newNode));
    }
}

void AudioGraph::removeNode(IAudioNode* nodeToRemove) {
    if (nodeToRemove) {
        nodeRemoveQueue.push(nodeToRemove);
    }
}

} // namespace Nimbus
