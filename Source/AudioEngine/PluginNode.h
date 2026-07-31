#pragma once

#include "Nodes/Node.h"
#include <JuceHeader.h>
#include <memory>
#include <atomic>

namespace Nimbus {

/**
 * Wraps a juce::AudioPluginInstance into our Node graph system.
 */
class PluginNode : public Node {
public:
    PluginNode(juce::AudioPluginInstance* pluginInstance);
    ~PluginNode() override;

    // Node
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void process(const ProcessContext& context) override;
    int getLatencySamples() const override;

    juce::AudioPluginInstance* getPluginInstance() const { return pluginInstance; }
    
    bool isBypassed() const { return bypassed; }
    void setBypassed(bool b) { bypassed = b; }

private:
    juce::AudioPluginInstance* pluginInstance = nullptr;
    bool isPrepared = false;
    bool bypassed = false;
};

} // namespace Nimbus
