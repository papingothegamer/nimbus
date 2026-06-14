#include "DetailViewComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

DetailViewComponent::DetailViewComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(placeholderLabel);
    placeholderLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    placeholderLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    placeholderLabel.setJustificationType(juce::Justification::centred);
}

DetailViewComponent::~DetailViewComponent() = default;

void DetailViewComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Top border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 0, getWidth(), 1);
}

void DetailViewComponent::resized() {
    placeholderLabel.setBounds(getLocalBounds());
}

} // namespace Nimbus::MainLayout
