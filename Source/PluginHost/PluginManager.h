#pragma once

#include <JuceHeader.h>
#include <memory>

namespace Nimbus {

/**
 * Manages plugin formats and instantiates audio plugin instances.
 */
class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    // Returns a new plugin instance from a given file path.
    // If it fails, returns nullptr and sets errorMessage.
    std::unique_ptr<juce::AudioPluginInstance> loadPlugin(const juce::String& pluginPath, juce::String& errorMessage);

    juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }
    juce::KnownPluginList& getKnownPluginList() { return knownPluginList; }

private:
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList knownPluginList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager)
};

} // namespace Nimbus
