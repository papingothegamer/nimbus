#pragma once

#include "IAudioNode.h"
#include <vector>
#include <memory>
#include <juce_core/juce_core.h>

namespace Nimbus {

/**
 * AudioGraph is an IAudioNode that itself contains and manages other nodes.
 * It is responsible for calling processBlock on all child nodes sequentially.
 * 
 * Uses a LockFreeQueue to safely add new nodes from the UI thread without
 * interrupting the Audio thread.
 */
class AudioGraph : public IAudioNode {
public:
    AudioGraph();
    ~AudioGraph() override;

    // IAudioNode implementation
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    int getLatencySamples() const override;

    /**
     * Adds a node to the graph. Thread-safe. Can be called from the UI thread.
     * The node is pushed into a lock-free queue and appended to the active
     * vector by the audio thread on the next process callback.
     */
    void addNode(std::unique_ptr<IAudioNode> newNode);
    void removeNode(IAudioNode* nodeToRemove);
    
    // Returns the current active nodes in the graph
    const std::vector<std::unique_ptr<IAudioNode>>& getNodes() const { return nodes; }
    juce::SpinLock& getProcessLock() const { return processLock; }

private:
    std::vector<std::unique_ptr<IAudioNode>> nodes;
    mutable juce::SpinLock processLock;
    
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};

} // namespace Nimbus
