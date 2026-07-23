#include "GroupTrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::Timeline {

void GroupTrackHeaderComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName, float rotationDegrees) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        // BRUTE FORCE SVG TO WHITE
        juce::String svgStr(data, (size_t)size);
        svgStr = svgStr.replace("fill=\"#000000\"", "fill=\"#ffffff\"")
                       .replace("fill=\"#212121\"", "fill=\"#ffffff\"")
                       .replace("fill=\"currentColor\"", "fill=\"#ffffff\"")
                       .replace("<svg ", "<svg fill=\"#ffffff\" color=\"#ffffff\" ");

        if (auto xml = juce::XmlDocument::parse(svgStr)) {
            if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                svg->setTransformToFit(juce::Rectangle<float>(0, 0, 16, 16), juce::RectanglePlacement::centred);
                if (rotationDegrees != 0.0f) {
                    svg->setTransform(svg->getTransform().rotated(juce::degreesToRadians(rotationDegrees), 8.0f, 8.0f));
                }
                btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
            }
        }
    }
}

GroupTrackHeaderComponent::GroupTrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    bool isMuted = engine.getTimelineProject().getTrack(trackIndex).isMuted;
    
    addAndMakeVisible(foldButton);
    foldButton.setClickingTogglesState(true);
    
    auto createRotatedIcon = [](const juce::String& name, float rot) -> std::unique_ptr<juce::Drawable> {
        int size = 0;
        if (const char* data = BinaryData::getNamedResource(name.toUTF8(), size)) {
            juce::String svgStr(data, (size_t)size);
            svgStr = svgStr.replace("fill=\"#000000\"", "fill=\"#ffffff\"")
                           .replace("fill=\"#212121\"", "fill=\"#ffffff\"")
                           .replace("fill=\"currentColor\"", "fill=\"#ffffff\"")
                           .replace("<svg ", "<svg fill=\"#ffffff\" color=\"#ffffff\" ");
            if (auto xml = juce::XmlDocument::parse(svgStr)) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->setTransformToFit(juce::Rectangle<float>(0, 0, 10, 10), juce::RectanglePlacement::centred);
                    if (rot != 0.0f) {
                        svg->setTransform(svg->getTransform().rotated(juce::degreesToRadians(rot), 5.0f, 5.0f));
                    }
                    return svg;
                }
            }
        }
        return nullptr;
    };

    auto unfoldedSvg = createRotatedIcon(DesignSystem::Iconography::Fold, 0.0f);
    auto foldedSvg = createRotatedIcon(DesignSystem::Iconography::Fold, -90.0f);
    foldButton.setImages(unfoldedSvg.get(), nullptr, nullptr, nullptr, foldedSvg.get(), nullptr, nullptr, nullptr);
    foldButton.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    foldButton.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
    foldButton.setToggleState(isFolded, juce::dontSendNotification);
    foldButton.onClick = [this] {
        bool currentlyFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
        engine.getTimelineProject().setTrackFolded(trackIndex, !currentlyFolded);
    };

    addAndMakeVisible(powerToggle);
    powerToggle.setClickingTogglesState(true);
    powerToggle.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    powerToggle.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    powerToggle.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    powerToggle.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
    powerToggle.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState());
    };

    // Track Name
    addAndMakeVisible(trackNameLabel);
    trackNameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    trackNameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    trackNameLabel.setJustificationType(juce::Justification::centredLeft);
    trackNameLabel.setEditable(false, true, false);
    trackNameLabel.onTextChange = [this] {
        engine.getTimelineProject().setTrackName(trackIndex, trackNameLabel.getText());
    };

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
        powerToggle.setButtonText(juce::String(trackIndex + 1));
        trackNameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : "Group Track", juce::dontSendNotification);

        foldButton.setToggleState(trackModel.isFolded, juce::dontSendNotification);
        
        muteButton.setToggleState(trackModel.isMuted, juce::dontSendNotification);
        loadSvgIcon(muteButton, trackModel.isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
        
        soloButton.setToggleState(trackModel.isSoloed, juce::dontSendNotification);
    }
}

void GroupTrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        foldButton.setToggleState(isFolded, juce::dontSendNotification);
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
    bool isSelected = engine.getTimelineProject().isTrackSelected(trackIndex);
    
    // Background with a slightly distinct tint for groups
    if (isSelected) {
        g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f).interpolatedWith(DesignSystem::Colors::PrimaryAction, 0.1f));
    } else {
        g.fillAll(DesignSystem::Colors::ModuleBackground.darker(0.2f).interpolatedWith(juce::Colours::black, 0.2f));
    }
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
    
    // Blue line at top
    g.setColour(DesignSystem::Colors::PrimaryAction);
    g.fillRect(0, 0, getWidth(), 4);
    
    // Blue line down the left side
    g.fillRect(0, 0, 4, getHeight());
}

void GroupTrackHeaderComponent::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromBottom(1); // divider
    bounds.removeFromTop(4); // top border
    bounds.removeFromLeft(4); // left border
    bounds.removeFromLeft(12); // Indent for the new left border

    auto topRow = bounds.removeFromTop(30);
    
    // Add 8px baseline padding
    topRow.removeFromLeft(8);
    
    // Fold Button
    foldButton.setBounds(topRow.removeFromLeft(20).withSizeKeepingCentre(16, 16));
    topRow.removeFromLeft(4); // space
    
    // Power Toggle (Number Box)
    powerToggle.setBounds(topRow.removeFromLeft(24).withSizeKeepingCentre(24, 20));
    topRow.removeFromLeft(4); // space
    
    // Track Name takes remaining space on top row
    trackNameLabel.setBounds(topRow);

    // Bottom Row for Mute/Solo
    if (bounds.getHeight() >= 24) {
        auto bottomRow = bounds.removeFromTop(24);
        int startX = bottomRow.getX() + 52; // Indent past fold and power buttons
        
        soloButton.setBounds(startX + 2, bottomRow.getY() + 2, 16, 20);
        startX += 24;
        muteButton.setBounds(startX + 2, bottomRow.getY() + 2, 16, 20);
    } else {
        soloButton.setBounds(0, 0, 0, 0);
        muteButton.setBounds(0, 0, 0, 0);
    }
}

} // namespace Nimbus::Timeline