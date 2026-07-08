#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::MainLayout {

// A thin draggable bar for resizing the sidebar columns
class ColumnResizerBar : public juce::Component {
public:
    ColumnResizerBar() = default;

    void paint(juce::Graphics& g) override {
        g.fillAll(DesignSystem::Colors::Divider);
    }

    void mouseDown(const juce::MouseEvent&) override {
        dragStartWidth = currentWidth;
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        int newWidth = dragStartWidth + e.getDistanceFromDragStartX();
        currentWidth = juce::jlimit(minWidth, maxWidth, newWidth);
        if (onWidthChanged) onWidthChanged(currentWidth);
    }

    int currentWidth = 110;
    int minWidth = 70;
    int maxWidth = 200;
    int dragStartWidth = 110;
    std::function<void(int)> onWidthChanged;
};

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
    ColumnResizerBar columnResizer;
    int leftColumnWidth = 110;
    
    std::unique_ptr<juce::Drawable> searchIcon;

    class CategoriesModel;
    std::unique_ptr<CategoriesModel> catModel;

    class PluginItemsModel;
    std::unique_ptr<PluginItemsModel> pluginModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideBrowserComponent)
};

} // namespace Nimbus::MainLayout
