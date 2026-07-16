#include "GroupTrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::Timeline {

void GroupTrackHeaderComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        if (auto svg = juce::Drawable::createFromImageData(data, size)) {
            // MAKE ICONS WHITE
            svg->replaceColour(juce::Colours::black, juce::Colours::white);
            btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
        }
    }
}

GroupTrackHeaderComponent::GroupTrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    bool isMuted = engine.getTimelineProject().getTrack(trackIndex).isMuted;
    
    addAndMakeVisible(foldButton);
    foldButton.setClickingTogglesState(true);
    loadSvgIcon(foldButton, isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold);
    foldButton.onClick = [this] {
        bool currentlyFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
        engine.getTimelineProject().setTrackFolded(trackIndex, !currentlyFolded);
    };

    addAndMakeVisible(powerToggle);
    powerToggle.setClickingTogglesState(true);
    powerToggle.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    powerToggle.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
    powerToggle.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState());
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setToggleState(isMuted, juce::dontSendNotification);
    loadSvgIcon(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
    muteButton.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, muteButton.getToggleState());
    };

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    loadSvgIcon(soloButton, DesignSystem::Iconography::Solo);
    soloButton.onClick = [this] {
        engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState());
    };

    engine.getTimelineProject().addListener(this);
}

GroupTrackHeaderComponent::~GroupTrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
}

void GroupTrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    
    if (trackIndex >= 0 && trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : "Group", juce::dontSendNotification);

        loadSvgIcon(foldButton, trackModel.isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold);
        
        muteButton.setToggleState(trackModel.isMuted, juce::dontSendNotification);
        loadSvgIcon(muteButton, trackModel.isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
        
        soloButton.setToggleState(trackModel.isSoloed, juce::dontSendNotification);
    }
}

void GroupTrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        loadSvgIcon(foldButton, isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold);
    }
}

void GroupTrackHeaderComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
        loadSvgIcon(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
    }
}

void GroupTrackHeaderComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) {
        soloButton.setToggleState(isSoloed, juce::dontSendNotification);
    }
}

void GroupTrackHeaderComponent::paint(juce::Graphics& g) {
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f));
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d)); 
    }
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.drawRect(getLocalBounds(), 1);
}

// Notice the getHeight abort trap is gone completely!
void GroupTrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    bounds.removeFromRight(8); // VU meter alignment padding

    soloButton.setBounds(bounds.removeFromRight(24).reduced(2));
    bounds.removeFromRight(2);
    muteButton.setBounds(bounds.removeFromRight(24).reduced(2));
    
    foldButton.setBounds(bounds.removeFromLeft(20).reduced(2));
    powerToggle.setButtonText(""); 
    
    // INCREASED WIDTH to match track header
    powerToggle.setBounds(bounds.removeFromLeft(30).reduced(2));
    
    nameLabel.setBounds(bounds);
}

} // namespace Nimbus::Timeline