#pragma once

#include "Node.h"
#include <vector>
#include <memory>
#include <atomic>
#include "../LevelMeasurer.h"

namespace Nimbus {

class TrackNode : public Node {
public:
    TrackNode() {
        smoothedVolume.setCurrentAndTargetValue(1.0f);
        smoothedPan.setCurrentAndTargetValue(0.0f);
    }

    void addInput(std::unique_ptr<Node> input) {
        inputs.push_back(std::move(input));
    }
    
    void addPlugin(std::unique_ptr<Node> plugin) {
        plugins.push_back(std::move(plugin));
    }

    void prepare(double sampleRate, int blockSize) override {
        Node::prepare(sampleRate, blockSize);
        tempBuffer.setSize(2, blockSize, false, true, true);
        
        smoothedVolume.reset(sampleRate, 0.015); // 15ms ramp
        smoothedPan.reset(sampleRate, 0.015);
        
        if (targetVolume) smoothedVolume.setCurrentAndTargetValue(targetVolume->load(std::memory_order_relaxed));
        if (targetPan) smoothedPan.setCurrentAndTargetValue(targetPan->load(std::memory_order_relaxed));

        for (auto& input : inputs)
            if (input) input->prepare(sampleRate, blockSize);

        for (auto& plugin : plugins)
            if (plugin) plugin->prepare(sampleRate, blockSize);
    }

    void process(const ProcessContext& context) override {
        bool isMuted = targetMute ? targetMute->load(std::memory_order_relaxed) : false;
        bool isSoloed = targetSolo ? targetSolo->load(std::memory_order_relaxed) : false;
        bool anyTrackSoloed = anySolo ? anySolo->load(std::memory_order_relaxed) : false;
        
        if (anyTrackSoloed && !isSoloed) isMuted = true;
        
        if (!context.buffer) return;
        
        // 1. Process and mix inputs (clips)
        context.buffer->clear();
        for (auto& input : inputs) {
            if (input) {
                tempBuffer.setSize(context.buffer->getNumChannels(), context.buffer->getNumSamples(), false, false, true);
                tempBuffer.clear();
                
                ProcessContext subContext = context;
                subContext.buffer = &tempBuffer;
                
                input->process(subContext);
                
                if (!isMuted) {
                    for (int ch = 0; ch < context.buffer->getNumChannels(); ++ch) {
                        if (ch < tempBuffer.getNumChannels()) {
                            context.buffer->addFrom(ch, 0, tempBuffer, ch, 0, context.buffer->getNumSamples());
                        }
                    }
                }
            }
        }
        
        if (isMuted) {
            if (levelMeasurer) levelMeasurer->processBuffer(*context.buffer);
            return; // Skip plugins and volume processing to save CPU
        }
        
        // 2. Process plugins in series
        for (auto& plugin : plugins) {
            if (plugin) {
                plugin->process(context);
            }
        }

        // 3. Apply volume and pan (smoothed)
        if (targetVolume) smoothedVolume.setTargetValue(targetVolume->load(std::memory_order_relaxed));
        if (targetPan) smoothedPan.setTargetValue(targetPan->load(std::memory_order_relaxed));
        
        if (smoothedVolume.isSmoothing()) {
            smoothedVolume.applyGain(*context.buffer, context.buffer->getNumSamples());
        } else {
            context.buffer->applyGain(smoothedVolume.getCurrentValue());
        }

        if (context.buffer->getNumChannels() == 2) {
            if (smoothedPan.isSmoothing()) {
                // Apply sample-by-sample panning
                float* left = context.buffer->getWritePointer(0);
                float* right = context.buffer->getWritePointer(1);
                for (int i = 0; i < context.buffer->getNumSamples(); ++i) {
                    float p = smoothedPan.getNextValue();
                    float panValue = (p + 1.0f) * 0.5f;
                    float leftGain = std::cos(panValue * juce::MathConstants<float>::halfPi);
                    float rightGain = std::sin(panValue * juce::MathConstants<float>::halfPi);
                    left[i] *= leftGain;
                    right[i] *= rightGain;
                }
            } else {
                float p = smoothedPan.getCurrentValue();
                if (p != 0.0f) {
                    float panValue = (p + 1.0f) * 0.5f;
                    float leftGain = std::cos(panValue * juce::MathConstants<float>::halfPi);
                    float rightGain = std::sin(panValue * juce::MathConstants<float>::halfPi);
                    context.buffer->applyGain(0, 0, context.buffer->getNumSamples(), leftGain);
                    context.buffer->applyGain(1, 0, context.buffer->getNumSamples(), rightGain);
                }
            }
        }
        
        // 4. Measure peaks
        if (levelMeasurer) {
            levelMeasurer->processBuffer(*context.buffer);
        }
    }
    
    void bindState(std::atomic<float>* vol, std::atomic<float>* pan, std::atomic<bool>* mute, std::atomic<bool>* solo, std::atomic<bool>* anySoloPtr, LevelMeasurer* measurer) {
        targetVolume = vol;
        targetPan = pan;
        targetMute = mute;
        targetSolo = solo;
        anySolo = anySoloPtr;
        levelMeasurer = measurer;
    }

private:
    std::vector<std::unique_ptr<Node>> inputs;
    std::vector<std::unique_ptr<Node>> plugins;
    juce::AudioBuffer<float> tempBuffer;
    
    std::atomic<float>* targetVolume = nullptr;
    std::atomic<float>* targetPan = nullptr;
    std::atomic<bool>* targetMute = nullptr;
    std::atomic<bool>* targetSolo = nullptr;
    std::atomic<bool>* anySolo = nullptr;
    
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedVolume;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedPan;
    
    LevelMeasurer* levelMeasurer = nullptr;
};

} // namespace Nimbus
