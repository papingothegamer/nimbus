#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>

namespace Nimbus {

/**
 * Base class for a lock-free audio processing node.
 * Mirrors Tracktion's te::Node architecture.
 */
struct ProcessContext {
    juce::AudioBuffer<float>* buffer = nullptr;
    juce::MidiBuffer* midiMessages = nullptr;
    double currentPositionSamples = 0.0;
    bool isPlaying = false;
    double tempo = 120.0;
};

class Node {
public:
    Node() = default;
    virtual ~Node() = default;

    /** Prepares the node for playback. Must be called before process(). */
    virtual void prepare(double sampleRate, int blockSize) {
        currentSampleRate = sampleRate;
        currentBlockSize = blockSize;
    }

    /** Processes the audio. Must be lock-free and real-time safe. */
    virtual void process(const ProcessContext& context) = 0;

    /** Returns latency in samples. */
    virtual int getLatencySamples() const { return 0; }

protected:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};

/**
 * A Node that simply mixes its child nodes together.
 */
class MixerNode : public Node {
public:
    MixerNode() = default;
    
    void addInput(std::unique_ptr<Node> input) {
        inputs.push_back(std::move(input));
    }

    void prepare(double sampleRate, int blockSize) override {
        Node::prepare(sampleRate, blockSize);
        tempBuffer.setSize(2, blockSize, false, true, true);
        
        for (auto& input : inputs)
            if (input) input->prepare(sampleRate, blockSize);
    }

    void process(const ProcessContext& context) override {
        if (!context.buffer) return;
        
        for (auto& input : inputs) {
            if (input) {
                tempBuffer.setSize(context.buffer->getNumChannels(), context.buffer->getNumSamples(), false, false, true);
                tempBuffer.clear();
                
                ProcessContext subContext = context;
                subContext.buffer = &tempBuffer;
                
                input->process(subContext);
                
                for (int ch = 0; ch < context.buffer->getNumChannels(); ++ch) {
                    if (ch < tempBuffer.getNumChannels()) {
                        context.buffer->addFrom(ch, 0, tempBuffer, ch, 0, context.buffer->getNumSamples());
                    }
                }
            }
        }
    }

private:
    std::vector<std::unique_ptr<Node>> inputs;
    juce::AudioBuffer<float> tempBuffer;
};

} // namespace Nimbus
