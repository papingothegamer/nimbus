#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include <algorithm>

namespace Nimbus::MainLayout {

class SideBrowserComponent::CategoriesModel : public juce::ListBoxModel {
public:
    CategoriesModel() {
        categories = {
            "Sounds", "Drums", "Instruments", "Audio Effects", "MIDI Effects", 
            "Plugins", "Samples", "Files"
        };
    }
    
    int getNumRows() override { return categories.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(DesignSystem::Colors::ComponentBackground);
        
        g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText(" " + categories[rowNumber], 0, 0, width, height, juce::Justification::centredLeft, true);
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (onCategorySelected) onCategorySelected(categories[row]);
    }

    std::vector<juce::String> categories;
    std::function<void(const juce::String&)> onCategorySelected;
};

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
            if (expandedMakers.count(currentMfg) > 0) {
                items.push_back({false, "", p});
            }
        }
    }

    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto& item = items[rowNumber];
        if (item.isHeader) {
            g.fillAll(DesignSystem::Colors::ComponentBackground);
            g.setColour(DesignSystem::Colors::TextSecondary);
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            
            // Draw chevron
            bool isExpanded = expandedMakers.count(item.headerText) > 0;
            juce::String chevron = isExpanded ? DesignSystem::Iconography::Fold : DesignSystem::Iconography::Unfold;
            
            // Render text
            g.drawText("  " + item.headerText.toUpperCase(), 24, 0, width - 24, height, juce::Justification::centredLeft, true);
            
            // Simple chevron representation (if we had access to SVG rendering here easily, we'd use it, 
            // but we can just use text characters for now if needed, or leave it to standard lookandfeel)
            g.drawText(isExpanded ? "-" : "+", 8, 0, 16, height, juce::Justification::centred, true);
        } else {
            if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
            g.setColour(DesignSystem::Colors::TextPrimary);
            g.setFont(DesignSystem::Typography::getPrimaryFont());
            g.drawText("        " + item.desc.name, 0, 0, width, height, juce::Justification::centredLeft, true);
        }
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) {
            if (expandedMakers.count(item.headerText) > 0) {
                expandedMakers.erase(item.headerText);
            } else {
                expandedMakers.insert(item.headerText);
            }
            // Need to notify component to update list. 
            // The cleanest way is to call updateList and updateContent, but we don't have a direct reference to the ListBox here.
            // We can just rely on a callback or pass the listbox in.
            if (onModelChanged) onModelChanged();
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

    std::function<void()> onModelChanged;

private:
    NimbusEngine& engine;
    std::vector<ListItem> items;
    std::set<juce::String> expandedMakers;
};

SideBrowserComponent::SideBrowserComponent(NimbusEngine& e) : engine(e) {
    catModel = std::make_unique<CategoriesModel>();
    pluginModel = std::make_unique<PluginItemsModel>(engine);
    
    pluginModel->onModelChanged = [this]() {
        pluginModel->updateList(false);
        itemsList.updateContent();
    };
    
    catModel->onCategorySelected = [this](const juce::String& cat) {
        if (cat == "Plugins") {
            pluginModel->updateList(false);
            itemsList.setModel(pluginModel.get());
        } else {
            itemsList.setModel(nullptr); // Empty placeholder for other categories
        }
        itemsList.updateContent();
    };

    categoriesList.setModel(catModel.get());
    categoriesList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    categoriesList.setRowHeight(24);
    addAndMakeVisible(categoriesList);
    
    itemsList.setModel(pluginModel.get());
    itemsList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::ModuleBackground);
    itemsList.setRowHeight(24);
    addAndMakeVisible(itemsList);
    
    catModel->onCategorySelected("Plugins"); // Default selection
    categoriesList.selectRow(5); // Index of "Plugins"
    
    searchBox.setTextToShowWhenEmpty("Search...", DesignSystem::Colors::TextSecondary);
    searchBox.setColour(juce::TextEditor::backgroundColourId, DesignSystem::Colors::AppBackground);
    searchBox.setColour(juce::TextEditor::outlineColourId, DesignSystem::Colors::ComponentBorder);
    searchBox.setColour(juce::TextEditor::textColourId, DesignSystem::Colors::TextPrimary);
    addAndMakeVisible(searchBox);
    searchBox.setFont(DesignSystem::Typography::getPrimaryFont());

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
}

SideBrowserComponent::~SideBrowserComponent() {
    stopTimer();
    itemsList.setModel(nullptr);
    categoriesList.setModel(nullptr);
}

void SideBrowserComponent::timerCallback() {
    if (!engine.getPluginManager().isScanning()) {
        stopTimer();
        // Force refresh
        pluginModel->updateList(false);
        itemsList.updateContent();
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
    auto topBar = bounds.removeFromTop(36).reduced(4);
    
    searchBox.setBounds(topBar.removeFromLeft(bounds.getWidth() - 30).reduced(2));
    
    auto leftCol = bounds.removeFromLeft(110);
    categoriesList.setBounds(leftCol);
    
    scanButton.setBounds(bounds.removeFromBottom(30).reduced(2));
    
    itemsList.setBounds(bounds);
}

} // namespace Nimbus::MainLayout
