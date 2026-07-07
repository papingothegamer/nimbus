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
    juce::ListBox categoriesList;
    juce::ListBox itemsList;
    juce::TextEditor searchBox;
    juce::TextButton scanButton{"Scan for Plugins"};
    
    std::unique_ptr<juce::Drawable> searchIcon;

    class CategoriesModel;
    std::unique_ptr<CategoriesModel> catModel;

    class PluginItemsModel;
    std::unique_ptr<PluginItemsModel> pluginModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideBrowserComponent)
};

} // namespace Nimbus::MainLayout
