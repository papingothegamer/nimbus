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

    int currentWidth = 150;
    int minWidth = 70;
    int maxWidth = 250;
    int dragStartWidth = 150;
    std::function<void(int)> onWidthChanged;
};

class PreviewPlayerComponent : public juce::Component, public juce::ChangeListener, private juce::Timer {
public:
    PreviewPlayerComponent(NimbusEngine& engine);
    ~PreviewPlayerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void loadFile(const juce::File& file);
    void play();
    void stop();

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    NimbusEngine& engine;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioThumbnail thumbnail;
    
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageFitted};
    std::unique_ptr<juce::Drawable> playIcon, pauseIcon;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewPlayerComponent)
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
    juce::ListBox foldersList;
    juce::ListBox itemsList;
    juce::TextEditor searchBox;
    juce::TextButton addFolderButton;
    std::unique_ptr<juce::FileChooser> fileChooser;
    ColumnResizerBar columnResizer;
    PreviewPlayerComponent previewPlayer;
    int leftColumnWidth = 150;
    
    std::unique_ptr<juce::Drawable> searchIcon;

    class CategoriesModel;
    std::unique_ptr<CategoriesModel> catModel;

    class PluginItemsModel;
    std::unique_ptr<PluginItemsModel> pluginModel;

    class StockEffectsModel;
    std::unique_ptr<StockEffectsModel> stockEffectsModel;

    class FoldersModel;
    std::unique_ptr<FoldersModel> foldersModel;

    class FileItemsModel;
    std::unique_ptr<FileItemsModel> fileModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideBrowserComponent)
};

} // namespace Nimbus::MainLayout
