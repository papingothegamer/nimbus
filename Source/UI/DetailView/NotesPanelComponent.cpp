#include "NotesPanelComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

NotesPanelComponent::NotesPanelComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(DesignSystem::Typography::getSecondaryFont().withStyle(juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    
    addAndMakeVisible(reverseButton);
    addAndMakeVisible(invertButton);
    addAndMakeVisible(legatoButton);
    addAndMakeVisible(duplicateButton);
}

NotesPanelComponent::~NotesPanelComponent() = default;

void NotesPanelComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.02f));
    
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(getLocalBounds(), 1);
}

void NotesPanelComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    
    titleLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(8); // Spacer
    
    int btnHeight = 22;
    
    auto row1 = area.removeFromTop(btnHeight);
    reverseButton.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(2));
    invertButton.setBounds(row1.reduced(2));
    
    auto row2 = area.removeFromTop(btnHeight);
    legatoButton.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2));
    duplicateButton.setBounds(row2.reduced(2));
}

} // namespace Nimbus::DetailView
