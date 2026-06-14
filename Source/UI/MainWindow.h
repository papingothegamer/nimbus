#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "Timeline/TimelineComponent.h"
#include "PluginWindow.h"

namespace Nimbus {

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(juce::String name, NimbusEngine& engine);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    class MainContentComponent : public juce::Component {
    public:
        MainContentComponent(NimbusEngine& engine);
        void resized() override;
    private:
        TimelineComponent timelineComponent;
    };

    MainContentComponent mainContent;

    std::unique_ptr<PluginWindow> pluginWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace Nimbus
