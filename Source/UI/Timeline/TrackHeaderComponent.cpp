#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::Timeline {

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    addAndMakeVisible(nameLabel);
    nameLabel.setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Mute);

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);

    addAndMakeVisible(volSlider);
    volSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

TrackHeaderComponent::~TrackHeaderComponent() = default;

void TrackHeaderComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::ModuleBackground);
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    // Left separator (since it's on the right of the timeline)
    g.fillRect(0, 0, 1, getHeight());
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    
    nameLabel.setBounds(bounds.removeFromTop(20));
    
    auto buttonsArea = bounds.removeFromTop(20);
    muteButton.setBounds(buttonsArea.removeFromLeft(20));
    buttonsArea.removeFromLeft(4);
    soloButton.setBounds(buttonsArea.removeFromLeft(20));

    volSlider.setBounds(bounds.withSizeKeepingCentre(40, 40));
}

} // namespace Nimbus::Timeline
