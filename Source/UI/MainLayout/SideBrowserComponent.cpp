#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

// Removed CategoriesModel

class SideBrowserComponent::PluginItemsModel : public juce::ListBoxModel {
public:
    PluginItemsModel(NimbusEngine& e) : engine(e) {}
    
    void updateList(bool instrumentsOnly) {
        plugins.clear();
        auto& list = engine.getPluginManager().getKnownPluginList();
        for (int i = 0; i < list.getNumTypes(); ++i) {
            auto* type = list.getType(i);
            if (instrumentsOnly && !type->isInstrument) continue;
            if (!instrumentsOnly && type->isInstrument) continue;
            plugins.push_back(*type);
        }
    }

    int getNumRows() override { return plugins.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("  " + plugins[rowNumber].name, 0, 0, width, height, juce::Justification::centredLeft, true);
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= plugins.size()) return;
        auto& desc = plugins[row];
        
        auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
        if (!selectedTracks.isEmpty()) {
            int trackIndex = selectedTracks.getRange(0).getStart();
            juce::String err;
            auto instance = engine.getPluginManager().loadPlugin(desc.fileOrIdentifier, err);
            if (instance != nullptr) {
                auto node = std::make_unique<PluginNode>(std::move(instance));
                auto track = engine.getMixer()->getTrack(trackIndex);
                if (track) {
                    track->addInsertPlugin(std::move(node));
                }
            } else {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Plugin Load Error", err);
            }
        }
    }

private:
    NimbusEngine& engine;
    std::vector<juce::PluginDescription> plugins;
};

SideBrowserComponent::SideBrowserComponent(NimbusEngine& e) : engine(e) {
    pluginModel = std::make_unique<PluginItemsModel>(engine);

    addAndMakeVisible(pluginsTab);
    addAndMakeVisible(samplesTab);
    addAndMakeVisible(filesTab);
    
    pluginsTab.setClickingTogglesState(true);
    samplesTab.setClickingTogglesState(true);
    filesTab.setClickingTogglesState(true);
    
    pluginsTab.setRadioGroupId(101);
    samplesTab.setRadioGroupId(101);
    filesTab.setRadioGroupId(101);
    
    pluginsTab.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    pluginsTab.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    samplesTab.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    samplesTab.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    filesTab.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    filesTab.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);

    pluginsTab.setToggleState(true, juce::dontSendNotification);
    
    pluginsTab.onClick = [this] {
        pluginModel->updateList(false);
        itemsList.setModel(pluginModel.get());
        itemsList.updateContent();
        scanButton.setVisible(true);
    };
    samplesTab.onClick = [this] { itemsList.setModel(nullptr); itemsList.updateContent(); scanButton.setVisible(false); };
    filesTab.onClick = [this] { itemsList.setModel(nullptr); itemsList.updateContent(); scanButton.setVisible(false); };

    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colours::grey);
    searchBox.setColour(juce::TextEditor::backgroundColourId, DesignSystem::Colors::AppBackground);
    searchBox.setColour(juce::TextEditor::textColourId, DesignSystem::Colors::TextPrimary);
    searchBox.setFont(DesignSystem::Typography::getPrimaryFont());

    addAndMakeVisible(itemsList);
    itemsList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    itemsList.setRowHeight(24);
    
    addAndMakeVisible(scanButton);
    scanButton.onClick = [this] {
        if (!engine.getPluginManager().isScanning()) {
            engine.getPluginManager().startScanning();
            // Start a timer to poll for completion
            startTimer(500);
        }
    };
    
    pluginsTab.onClick(); // Populate initial view
}

SideBrowserComponent::~SideBrowserComponent() {
    stopTimer();
    itemsList.setModel(nullptr);
}

void SideBrowserComponent::timerCallback() {
    if (!engine.getPluginManager().isScanning()) {
        stopTimer();
        // Force refresh
        if (pluginsTab.getToggleState()) {
            pluginsTab.onClick();
        }
    }
}

void SideBrowserComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Right border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
    
    // Search icon
    int dataSize = 0;
    if (auto* data = BinaryData::getNamedResource("search_svg", dataSize)) {
        if (auto drawable = juce::Drawable::createFromImageData(data, dataSize)) {
            drawable->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
            auto iconBounds = getLocalBounds().removeFromTop(30).removeFromBottom(36).reduced(5).removeFromLeft(20).toFloat();
            iconBounds = juce::Rectangle<float>(10, 30 + 10, 16, 16); // Manually position near the search box
            drawable->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }
}

void SideBrowserComponent::resized() {
    auto bounds = getLocalBounds();
    
    auto tabsArea = bounds.removeFromTop(30);
    int tabW = tabsArea.getWidth() / 3;
    pluginsTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(2));
    samplesTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(2));
    filesTab.setBounds(tabsArea.reduced(2));
    
    // Add padding to the search box for the icon
    auto searchBounds = bounds.removeFromTop(36).reduced(5);
    searchBox.setBounds(searchBounds.withTrimmedLeft(24));
    
    if (scanButton.isVisible()) {
        scanButton.setBounds(bounds.removeFromBottom(30).reduced(2));
    }
    
    itemsList.setBounds(bounds);
}

} // namespace Nimbus::MainLayout
