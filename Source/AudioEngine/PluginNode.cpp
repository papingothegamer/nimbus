#include "PluginNode.h"

namespace Nimbus {

PluginNode::PluginNode(juce::AudioPluginInstance* instance)
    : pluginInstance(instance)
{
    // A real DAW would handle latency compensation here, but we keep it simple for now.
}

PluginNode::~PluginNode() {
    // We no longer own pluginInstance, so we shouldn't call releaseResources() here.
    // Doing so would break the instance for the new graph that just swapped in.
}

void PluginNode::prepare(double sampleRate, int maximumExpectedSamplesPerBlock) {
    Node::prepare(sampleRate, maximumExpectedSamplesPerBlock);
    if (pluginInstance) {
        if (pluginInstance->getSampleRate() != sampleRate || pluginInstance->getBlockSize() != maximumExpectedSamplesPerBlock) {
            pluginInstance->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        }
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
