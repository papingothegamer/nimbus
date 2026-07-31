#include "PluginNode.h"

namespace Nimbus {

PluginNode::PluginNode(juce::AudioPluginInstance* instance)
    : pluginInstance(instance)
{
    // A real DAW would handle latency compensation here, but we keep it simple for now.
}

PluginNode::~PluginNode() {
    // Release resources on destruction if needed
    if (isPrepared && pluginInstance) {
        pluginInstance->releaseResources();
    }
}

void PluginNode::prepare(double sampleRate, int maximumExpectedSamplesPerBlock) {
    Node::prepare(sampleRate, maximumExpectedSamplesPerBlock);
    if (pluginInstance) {
        pluginInstance->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        isPrepared = true;
    }
}

void PluginNode::process(const ProcessContext& context) {
    if (!pluginInstance || !isPrepared || bypassed) return;
    if (!context.buffer || !context.midiMessages) return;

    pluginInstance->processBlock(*context.buffer, *context.midiMessages);
}

int PluginNode::getLatencySamples() const {
    if (pluginInstance)
        return pluginInstance->getLatencySamples();
    return 0;
}

} // namespace Nimbus
