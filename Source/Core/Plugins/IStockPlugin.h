#pragma once

#include <JuceHeader.h>
#include "../../AudioEngine/Nodes/Node.h"

namespace Nimbus {

class IStockPlugin : public Node {
public:
    virtual ~IStockPlugin() = default;

    virtual juce::String getName() const = 0;
    virtual juce::String getCategory() const = 0;
    virtual bool isMidiEffect() const { return false; }
    
    // Creates the embedded UI editor for this plugin. 
    // The caller takes ownership of the returned component.
    virtual juce::Component* createEditor() = 0;
    
    virtual bool isBypassed() const = 0;
    virtual void setBypassed(bool b) = 0;
    
    // Legacy mapping to prevent modifying all stock plugins at once
    virtual void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {}
    virtual void releaseResources() {}
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {}

    // Node interface implementation mapping to legacy
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override {
        Node::prepare(sampleRate, maximumExpectedSamplesPerBlock);
        prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    }
    
    void process(const ProcessContext& context) override {
        if (context.buffer && context.midiMessages) {
            processBlock(*context.buffer, *context.midiMessages);
        }
    }
    
    // State persistence
    virtual void getStateInformation(juce::MemoryBlock& destData) {}
    virtual void setStateInformation(const void* data, int sizeInBytes) {}
};

} // namespace Nimbus
