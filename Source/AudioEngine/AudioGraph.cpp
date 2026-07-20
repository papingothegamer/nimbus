#include "AudioGraph.h"

namespace Nimbus {

AudioGraph::AudioGraph() {}

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
    const juce::SpinLock::ScopedLockType sl(processLock);

    // 3. Process all active nodes
    for (auto& node : nodes) {
        node->processBlock(buffer, midiMessages);
    }
}

int AudioGraph::getLatencySamples() const {
    int totalLatency = 0;
    for (const auto& node : nodes) {
        totalLatency += node->getLatencySamples();
    }
    return totalLatency;
}

void AudioGraph::addNode(std::unique_ptr<IAudioNode> newNode) {
    if (newNode) {
        if (currentSampleRate > 0) {
            newNode->prepareToPlay(currentSampleRate, currentBlockSize);
        }
        const juce::SpinLock::ScopedLockType sl(processLock);
        nodes.push_back(std::move(newNode));
    }
}

void AudioGraph::removeNode(IAudioNode* nodeToRemove) {
    if (nodeToRemove) {
        const juce::SpinLock::ScopedLockType sl(processLock);
        auto it = std::remove_if(nodes.begin(), nodes.end(),
            [nodeToRemove](const std::unique_ptr<IAudioNode>& ptr) {
                return ptr.get() == nodeToRemove;
            });
        nodes.erase(it, nodes.end());
    }
}

} // namespace Nimbus
