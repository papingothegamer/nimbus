#pragma once

#include "IAudioNode.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <mutex>

namespace Nimbus {

/**
 * Wraps a juce::AudioPluginInstance into our IAudioNode graph system.
 */
class PluginNode : public IAudioNode {
public:
    // Takes ownership of the plugin instance.
    PluginNode(std::unique_ptr<juce::AudioPluginInstance> instance);
    ~PluginNode() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioPluginInstance* getPluginInstance() const { return pluginInstance.get(); }
    
    bool isBypassed() const { return bypassed; }
    void setBypassed(bool b) { bypassed = b; }

private:
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    bool isPrepared = false;
    bool bypassed = false;
};

} // namespace Nimbus
