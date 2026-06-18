#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::MainLayout {

class DeviceChainComponent : public juce::Component, private juce::Timer {
public:
    DeviceChainComponent(NimbusEngine& engine);
    ~DeviceChainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    
    // Updates the view based on the currently selected track
    void updateChain();
    
    void timerCallback() override;

private:
    NimbusEngine& engine;
    
    class PluginBox;
    std::vector<std::unique_ptr<PluginBox>> pluginBoxes;
    
    int currentTrackIndex = -1;
    std::unique_ptr<juce::Drawable> addPluginIcon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceChainComponent)
};

} // namespace Nimbus::MainLayout
