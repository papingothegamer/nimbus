#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::MainLayout {

class SideBrowserComponent : public juce::Component, private juce::Timer {
public:
    SideBrowserComponent(NimbusEngine& engine);
    ~SideBrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    NimbusEngine& engine;
    
    juce::TextButton pluginsTab{"Plugins"};
    juce::TextButton samplesTab{"Samples"};
    juce::TextButton filesTab{"Files"};
    
    juce::TextEditor searchBox;
    juce::ListBox itemsList;
    juce::TextButton scanButton{"Scan for VST3s"};

    class PluginItemsModel;
    std::unique_ptr<PluginItemsModel> pluginModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideBrowserComponent)
};

} // namespace Nimbus::MainLayout
