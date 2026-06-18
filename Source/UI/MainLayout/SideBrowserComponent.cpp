#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include <algorithm>

namespace Nimbus::MainLayout {

// Removed CategoriesModel

class SideBrowserComponent::PluginItemsModel : public juce::ListBoxModel {
public:
    PluginItemsModel(NimbusEngine& e) : engine(e) {}
    
    struct ListItem {
        bool isHeader = false;
        juce::String headerText;
        juce::PluginDescription desc;
    };
    
    void updateList(bool instrumentsOnly) {
        items.clear();
        std::vector<juce::PluginDescription> rawPlugins;
        auto& list = engine.getPluginManager().getKnownPluginList();
        for (int i = 0; i < list.getNumTypes(); ++i) {
            auto* type = list.getType(i);
            rawPlugins.push_back(*type);
        }
        // Sort by manufacturer then name
        std::sort(rawPlugins.begin(), rawPlugins.end(), [](const auto& a, const auto& b) {
            if (a.manufacturerName != b.manufacturerName)
                return a.manufacturerName.compareIgnoreCase(b.manufacturerName) < 0;
            return a.name.compareIgnoreCase(b.name) < 0;
        });
        
        juce::String currentMfg;
        for (auto& p : rawPlugins) {
            if (p.manufacturerName != currentMfg) {
                currentMfg = p.manufacturerName;
                items.push_back({true, currentMfg, {}});
            }
            items.push_back({false, "", p});
        }
    }

    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto& item = items[rowNumber];
        if (item.isHeader) {
            g.fillAll(DesignSystem::Colors::ComponentBackground);
            g.setColour(DesignSystem::Colors::TextSecondary);
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            g.drawText("  " + item.headerText.toUpperCase(), 0, 0, width, height, juce::Justification::centredLeft, true);
        } else {
            if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
            g.setColour(DesignSystem::Colors::TextPrimary);
            g.setFont(DesignSystem::Typography::getPrimaryFont());
            g.drawText("    " + item.desc.name, 0, 0, width, height, juce::Justification::centredLeft, true);
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) return;
        
        auto& desc = item.desc;
        
        auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
        if (!selectedTracks.isEmpty()) {
            int trackIndex = selectedTracks.getRange(0).getStart();
            juce::String err;
            auto instance = engine.getPluginManager().loadPlugin(desc.fileOrIdentifier, err);
            if (instance != nullptr) {
                auto node = std::make_unique<PluginNode>(std::move(instance));
                auto track = engine.getMixer()->getTrack(trackIndex);
                if (track) {
                    if (desc.isInstrument) {
                        track->setInstrumentPlugin(std::move(node));
                    } else {
                        track->addInsertPlugin(std::move(node));
                    }
                }
            } else {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Plugin Load Error", err);
            }
        }
    }

private:
    NimbusEngine& engine;
    std::vector<ListItem> items;
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
    
    int dataSize = 0;
    if (auto* data = BinaryData::getNamedResource("search_svg", dataSize)) {
        searchIcon = juce::Drawable::createFromImageData(data, dataSize);
        if (searchIcon) searchIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
    }
    
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
    
    // Left border (browser is on right side)
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 0, 1, getHeight());
    
    // Search icon
    if (searchIcon) {
        auto iconBounds = juce::Rectangle<float>(10, 30 + 10, 16, 16); // Manually position near the search box
        searchIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
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
