#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

class SideBrowserComponent::CategoriesModel : public juce::ListBoxModel {
public:
    juce::StringArray items = {"Sounds", "Drums", "Instruments", "Audio Effects", "MIDI Effects", "Plugins", "Clips", "Samples"};
    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("  " + items[rowNumber], 0, 0, width, height, juce::Justification::centredLeft, true);
    }
    
    std::function<void(int)> onCategorySelected;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (onCategorySelected) onCategorySelected(row);
    }
};

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
    catModel = std::make_unique<CategoriesModel>();
    pluginModel = std::make_unique<PluginItemsModel>(engine);

    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colours::grey);
    searchBox.setColour(juce::TextEditor::backgroundColourId, DesignSystem::Colors::AppBackground);
    searchBox.setColour(juce::TextEditor::textColourId, DesignSystem::Colors::TextPrimary);
    searchBox.setFont(DesignSystem::Typography::getPrimaryFont());

    addAndMakeVisible(categoriesList);
    categoriesList.setModel(catModel.get());
    categoriesList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    categoriesList.setRowHeight(24);
    
    catModel->onCategorySelected = [this](int row) {
        if (row == 2) { // Instruments
            pluginModel->updateList(true);
            itemsList.setModel(pluginModel.get());
            itemsList.updateContent();
        } else if (row == 3 || row == 5) { // Audio Effects or Plugins
            pluginModel->updateList(false);
            itemsList.setModel(pluginModel.get());
            itemsList.updateContent();
        } else {
            itemsList.setModel(nullptr);
            itemsList.updateContent();
        }
    };

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
}

SideBrowserComponent::~SideBrowserComponent() {
    stopTimer();
    categoriesList.setModel(nullptr);
    itemsList.setModel(nullptr);
}

void SideBrowserComponent::timerCallback() {
    if (!engine.getPluginManager().isScanning()) {
        stopTimer();
        // Force refresh if plugins are selected
        int row = categoriesList.getSelectedRow();
        if (catModel->onCategorySelected) {
            catModel->onCategorySelected(row);
        }
    }
}

void SideBrowserComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Right border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
}

void SideBrowserComponent::resized() {
    auto bounds = getLocalBounds();
    searchBox.setBounds(bounds.removeFromTop(40).reduced(5));
    
    scanButton.setBounds(bounds.removeFromBottom(30).reduced(2));

    auto topHalf = bounds.removeFromTop(bounds.getHeight() / 3);
    categoriesList.setBounds(topHalf);
    
    itemsList.setBounds(bounds);
}

} // namespace Nimbus::MainLayout
