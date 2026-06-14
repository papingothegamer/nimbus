#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

class MockCategoriesModel : public juce::ListBoxModel {
public:
    juce::StringArray items = {"Sounds", "Drums", "Instruments", "Audio Effects", "MIDI Effects", "Plugins", "Clips", "Samples"};
    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("  " + items[rowNumber], 0, 0, width, height, juce::Justification::centredLeft, true);
    }
};

class MockItemsModel : public juce::ListBoxModel {
public:
    juce::StringArray items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowIsSelected) g.fillAll(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("  " + items[rowNumber], 0, 0, width, height, juce::Justification::centredLeft, true);
    }
};

static MockCategoriesModel catModel;
static MockItemsModel itemModel;

SideBrowserComponent::SideBrowserComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colours::grey);
    searchBox.setColour(juce::TextEditor::backgroundColourId, DesignSystem::Colors::AppBackground);
    searchBox.setColour(juce::TextEditor::textColourId, DesignSystem::Colors::TextPrimary);
    searchBox.setFont(DesignSystem::Typography::getPrimaryFont());

    addAndMakeVisible(categoriesList);
    categoriesList.setModel(&catModel);
    categoriesList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    categoriesList.setRowHeight(24);

    addAndMakeVisible(itemsList);
    itemsList.setModel(&itemModel);
    itemsList.setColour(juce::ListBox::backgroundColourId, DesignSystem::Colors::PanelBackground);
    itemsList.setRowHeight(24);
}

SideBrowserComponent::~SideBrowserComponent() {
    categoriesList.setModel(nullptr);
    itemsList.setModel(nullptr);
}

void SideBrowserComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Right border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
}

void SideBrowserComponent::resized() {
    auto bounds = getLocalBounds().withTrimmedRight(1);
    
    searchBox.setBounds(bounds.removeFromTop(30).reduced(2));
    
    auto catWidth = bounds.getWidth() / 2;
    categoriesList.setBounds(bounds.removeFromLeft(catWidth));
    itemsList.setBounds(bounds);
}

} // namespace Nimbus::MainLayout
