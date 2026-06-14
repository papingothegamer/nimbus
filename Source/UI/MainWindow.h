#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/MainLayout/TopToolbarComponent.h"
#include "UI/MainLayout/SideBrowserComponent.h"
#include "UI/MainLayout/BottomMixerComponent.h"
#include "UI/Timeline/TimelineComponent.h"
#include "UI/DesignSystem/NimbusLookAndFeel.h"
#include "UI/PluginWindow.h"
#include "UI/MainLayout/DetailViewComponent.h"

namespace Nimbus {

class MainWindow : public juce::DocumentWindow {
public:
    class MainContentComponent : public juce::Component {
    public:
        MainContentComponent(NimbusEngine& engine);
        void resized() override;
        void paint(juce::Graphics& g) override;

        void toggleBrowser();
        void toggleDetailView();
        
        NimbusEngine& getEngine() { return engine; }

    private:
        NimbusEngine& engine;
        MainLayout::TopToolbarComponent topToolbar;
        MainLayout::SideBrowserComponent sideBrowser;
        MainLayout::BottomMixerComponent bottomMixer;
        MainLayout::DetailViewComponent detailView;
        TimelineComponent timelineComponent;

        bool isBrowserVisible = true;
        bool isDetailViewVisible = false;

        juce::StretchableLayoutManager verticalLayout;
        juce::StretchableLayoutManager horizontalLayout;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
    };

    MainWindow(juce::String name, NimbusEngine& engineToUse);
    ~MainWindow() override;

    void closeButtonPressed() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    DesignSystem::NimbusLookAndFeel lookAndFeel;
    MainContentComponent mainContent;
    std::unique_ptr<PluginWindow> pluginWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace Nimbus
