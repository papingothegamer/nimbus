#include "PluginNode.h"

namespace Nimbus {

PluginNode::PluginNode(std::unique_ptr<juce::AudioPluginInstance> instance)
    : pluginInstance(std::move(instance))
{
    // A real DAW would handle latency compensation here, but we keep it simple for now.
}

PluginNode::~PluginNode() {
    // Release resources on destruction if needed
    if (isPrepared && pluginInstance) {
        pluginInstance->releaseResources();
    }
}

void PluginNode::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    if (pluginInstance) {
        pluginInstance->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        isPrepared = true;
    }
}

void PluginNode::releaseResources() {
    if (pluginInstance) {
        pluginInstance->releaseResources();
        isPrepared = false;
    }
}

void PluginNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (!pluginInstance || !isPrepared) return;

    // The JUCE processBlock expects an AudioBuffer and MidiBuffer.
    // If the plugin acts as an instrument, it might replace the buffer.
    // If it's an effect, it processes it in-place.
    
    // Some plugins expect the exact number of channels they were initialized with,
    // but JUCE's AudioPluginInstance wrapper usually handles channel mapping inside.
    pluginInstance->processBlock(buffer, midiMessages);
}

} // namespace Nimbus
