#pragma once

#include "Node.h"
#include <vector>
#include <memory>

namespace Nimbus {

class TrackNode : public Node {
public:
    TrackNode() = default;

    void addInput(std::unique_ptr<Node> input) {
        inputs.push_back(std::move(input));
    }
    
    void addPlugin(std::unique_ptr<Node> plugin) {
        plugins.push_back(std::move(plugin));
    }

    void prepare(double sampleRate, int blockSize) override {
        Node::prepare(sampleRate, blockSize);
        tempBuffer.setSize(2, blockSize, false, true, true);
        
        for (auto& input : inputs)
            if (input) input->prepare(sampleRate, blockSize);

        for (auto& plugin : plugins)
            if (plugin) plugin->prepare(sampleRate, blockSize);
    }

    void process(const ProcessContext& context) override {
        if (!context.buffer || isMuted) return;
        
        // 1. Process and mix inputs (clips)
        context.buffer->clear();
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
        
        // 2. Process plugins in series
        for (auto& plugin : plugins) {
            if (plugin) {
                plugin->process(context);
            }
        }

        // 3. Apply volume and pan
        if (volume != 1.0f) {
            context.buffer->applyGain(volume);
        }
        // Very basic panning logic (assumes stereo buffer)
        if (pan != 0.0f && context.buffer->getNumChannels() == 2) {
            float leftGain = (pan < 0.0f) ? 1.0f : (1.0f - pan);
            float rightGain = (pan > 0.0f) ? 1.0f : (1.0f + pan);
            context.buffer->applyGain(0, 0, context.buffer->getNumSamples(), leftGain);
            context.buffer->applyGain(1, 0, context.buffer->getNumSamples(), rightGain);
        }
    }
    
    void setVolume(float v) { volume = v; }
    void setPan(float p) { pan = p; }
    void setMuted(bool m) { isMuted = m; }

private:
    std::vector<std::unique_ptr<Node>> inputs;
    std::vector<std::unique_ptr<Node>> plugins;
    juce::AudioBuffer<float> tempBuffer;
    float volume = 1.0f;
    float pan = 0.0f;
    bool isMuted = false;
};

} // namespace Nimbus
