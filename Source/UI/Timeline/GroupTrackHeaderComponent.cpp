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
    powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
    powerToggle.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState());
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.onTextChange = [this] {
        auto newText = nameLabel.getText();
        if (newText.trim().isEmpty()) {
            nameLabel.setText("Group Track", juce::dontSendNotification);
            engine.getTimelineProject().setTrackName(trackIndex, "Group Track");
        } else {
            engine.getTimelineProject().setTrackName(trackIndex, newText);
        }
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
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : "Group Track", juce::dontSendNotification);

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
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f));
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d)); 
    }
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.drawRect(getLocalBounds(), 1);
}

void GroupTrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    bounds.removeFromRight(6); // VU meter alignment padding

    soloButton.setBounds(bounds.removeFromRight(20).reduced(1));
    bounds.removeFromRight(4);
    muteButton.setBounds(bounds.removeFromRight(20).reduced(1));
    
    bounds.removeFromLeft(4);
    foldButton.setBounds(bounds.removeFromLeft(16).reduced(2).withTrimmedTop(1));
    
    bounds.removeFromLeft(4);
    powerToggle.setBounds(bounds.removeFromLeft(22).reduced(2));
    
    bounds.removeFromLeft(4);
    bounds.removeFromRight(2);
    
    // Keep height small enough to prevent JUCE from auto-wrapping text onto two lines
    nameLabel.setBounds(bounds.withSizeKeepingCentre(bounds.getWidth(), 18));
}

} // namespace Nimbus::Timeline