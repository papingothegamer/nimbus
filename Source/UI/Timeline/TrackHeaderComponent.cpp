#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace {
    class AbletonHorizontalLAF : public juce::LookAndFeel_V4 {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override {
            float trackHeight = 4.0f;
            juce::Rectangle<float> trackBounds(x, y + (height - trackHeight) * 0.5f, width, trackHeight);
            g.setColour(juce::Colour(0xff121212));
            g.fillRoundedRectangle(trackBounds, 2.0f);

            juce::Rectangle<float> fillBounds(x, trackBounds.getY(), sliderPos - x, trackHeight);
            g.setColour(juce::Colour(0xffdca823)); 
            g.fillRoundedRectangle(fillBounds, 2.0f);

            float thumbSize = 12.0f;
            juce::Rectangle<float> thumbBounds(sliderPos - thumbSize * 0.5f, y + (height - thumbSize) * 0.5f, thumbSize, thumbSize);
            g.setColour(juce::Colour(0xff888888));
            g.fillEllipse(thumbBounds);
        }
    };

    class AbletonRotaryLAF : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                              const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override {
            float radius = juce::jmin(width / 2.0f, height / 2.0f) - 2.0f;
            float centreX = x + width * 0.5f;
            float centreY = y + height * 0.5f;
            float rx = centreX - radius;
            float ry = centreY - radius;
            float rw = radius * 2.0f;
            float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour(juce::Colour(0xff121212));
            g.fillEllipse(rx, ry, rw, rw);
            g.setColour(juce::Colour(0xff888888));
            g.drawEllipse(rx, ry, rw, rw, 1.5f);

            juce::Path p;
            p.addRectangle(-1.0f, -radius, 2.0f, radius * 0.8f);
            p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
            g.setColour(juce::Colours::white);
            g.fillPath(p);
        }
    };

    AbletonHorizontalLAF horizontalLAF;
    AbletonRotaryLAF rotaryLAF;
}

namespace Nimbus::Timeline {

void TrackHeaderComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& normalIcon, juce::Colour normalColor, const juce::String& activeIcon, juce::Colour activeColor, float rotationDegrees) {
    auto createTinted = [](const juce::String& name, juce::Colour tint, float rot) -> std::unique_ptr<juce::Drawable> {
        int size = 0;
        if (const char* data = BinaryData::getNamedResource(name.toUTF8(), size)) {
            juce::String str(data, (size_t)size);
            str = str.replace("currentColor", "#000000").replace("#212121", "#000000");
            if (auto xml = juce::XmlDocument::parse(str)) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->replaceColour(juce::Colours::black, tint);
                    svg->setTransformToFit(juce::Rectangle<float>(0, 0, 16, 16), juce::RectanglePlacement::centred);
                    if (rot != 0.0f) {
                        svg->setTransform(svg->getTransform().rotated(juce::degreesToRadians(rot), 8.0f, 8.0f));
                    }
                    return svg;
                }
            }
        }
        return nullptr;
    };

    auto normalSvg = createTinted(normalIcon, normalColor, rotationDegrees);
    auto activeSvg = createTinted(activeIcon, activeColor, rotationDegrees);
    if (normalSvg && activeSvg) {
        btn.setImages(normalSvg.get(), nullptr, nullptr, nullptr, activeSvg.get(), nullptr, nullptr, nullptr);
    } else if (normalSvg) {
        btn.setImages(normalSvg.get(), nullptr, nullptr, nullptr, normalSvg.get(), nullptr, nullptr, nullptr);
    }
}

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;

    addAndMakeVisible(foldButton);
    foldButton.setClickingTogglesState(true);
    
    auto createRotatedIcon = [](const juce::String& name, float rot) -> std::unique_ptr<juce::Drawable> {
        int size = 0;
        if (const char* data = BinaryData::getNamedResource(name.toUTF8(), size)) {
            juce::String str(data, (size_t)size);
            str = str.replace("currentColor", "#000000").replace("#212121", "#000000");
            if (auto xml = juce::XmlDocument::parse(str)) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->replaceColour(juce::Colours::black, juce::Colours::white);
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
    powerToggle.setButtonText(juce::String(trackIndex + 1));
    powerToggle.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    powerToggle.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    powerToggle.setToggleState(true, juce::dontSendNotification);
    powerToggle.onClick = [this] {
        engine.getTimelineProject().setTrackSelected(trackIndex, true);
        engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState());
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    loadSvgIcon(muteButton, DesignSystem::Iconography::VolumeOff, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::Unmute, juce::Colours::white);
    muteButton.onClick = [this] {
        engine.getTimelineProject().setTrackSelected(trackIndex, true);
        engine.getTimelineProject().setTrackMuted(trackIndex, muteButton.getToggleState());
    };

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    loadSvgIcon(soloButton, DesignSystem::Iconography::Solo, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::Solo, juce::Colours::black);
    soloButton.onClick = [this] {
        engine.getTimelineProject().setTrackSelected(trackIndex, true);
        engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState());
    };

    addAndMakeVisible(armButton);
    armButton.setClickingTogglesState(true);
    armButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    armButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    loadSvgIcon(armButton, DesignSystem::Iconography::RecordArm, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::RecordArm, juce::Colours::white);
    armButton.onClick = [this] {
        engine.getTimelineProject().setTrackSelected(trackIndex, true);
        engine.getTimelineProject().setTrackArmed(trackIndex, armButton.getToggleState());
    };

    engine.getTimelineProject().addListener(this);
}

TrackHeaderComponent::~TrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().removeChangeListener(this);
}

void TrackHeaderComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel) {
        auto newText = nameLabel.getText();
        if (newText.trim().isEmpty()) {
            const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
            juce::String defaultName = "Mono Audio Track";
            if (trackModel.isGroup) defaultName = "Group Track";
            else if (trackModel.isMidi) defaultName = "MIDI Track";
            else if (trackModel.isStereo) defaultName = "Stereo Audio Track";
            nameLabel.setText(defaultName, juce::dontSendNotification);
            engine.getTimelineProject().setTrackName(trackIndex, defaultName);
        } else {
            engine.getTimelineProject().setTrackName(trackIndex, newText);
        }
    }
}
void TrackHeaderComponent::comboBoxChanged(juce::ComboBox*) {}
void TrackHeaderComponent::changeListenerCallback(juce::ChangeBroadcaster*) {}
void TrackHeaderComponent::updateInputSources() {}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isShiftDown()) {
        int lastSelected = engine.getTimelineProject().getLastSelectedTrack();
        if (lastSelected != -1) engine.getTimelineProject().selectTrackRange(lastSelected, trackIndex);
        else engine.getTimelineProject().setTrackSelected(trackIndex, true);
    } else if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
        engine.getTimelineProject().toggleTrackSelection(trackIndex);
    } else if (!event.mods.isPopupMenu()) {
        engine.getTimelineProject().setTrackSelected(trackIndex, true);
    }

    if (event.mods.isPopupMenu()) {
        juce::PopupMenu m;
        m.addItem(1, "Rename", true, false);
        m.addSeparator();
        m.addItem(2, "Insert Audio Track", true, false);
        m.addItem(3, "Insert MIDI Track", true, false);
        m.addSeparator();
        m.addItem(4, "Duplicate", true, false);
        m.addItem(5, "Delete", true, false);
        m.addSeparator();
        m.addItem(6, "Group Tracks", true, false);
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 2) { engine.addTrack(false); }
            else if (result == 3) { engine.addTrack(true); }
            else if (result == 4) { engine.getTimelineProject().duplicateTrack(trackIndex); }
            else if (result == 5) { engine.getTimelineProject().removeTrack(trackIndex); }
            else if (result == 6) { engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks()); }
        });
    }
}

void TrackHeaderComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) {
        soloButton.setToggleState(isSoloed, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackArmChanged(int track, bool isArmed) {
    if (track == trackIndex) {
        armButton.setToggleState(isArmed, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackSelectionChanged() { repaint(); }

void TrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        foldButton.setToggleState(isFolded, juce::dontSendNotification);
        resized();
        repaint();
    }
}

void TrackHeaderComponent::trackNameChanged(int track, const juce::String& newName) {
    if (track == trackIndex && !nameLabel.isBeingEdited()) nameLabel.setText(newName, juce::dontSendNotification);
}

void TrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    int displayNum = trackIndex + 1;

    if (trackIndex >= 0 && trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
        powerToggle.setButtonText(juce::String(displayNum));
        
        juce::String defaultName = "Mono Audio Track";
        if (trackModel.isGroup) defaultName = "Group Track";
        else if (trackModel.isMidi) defaultName = "MIDI Track";
        else if (trackModel.isStereo) defaultName = "Stereo Audio Track";
        
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : defaultName, juce::dontSendNotification);
        
        foldButton.setToggleState(trackModel.isFolded, juce::dontSendNotification);
        
        muteButton.setToggleState(trackModel.isMuted, juce::dontSendNotification);
        soloButton.setToggleState(trackModel.isSoloed, juce::dontSendNotification);
        armButton.setToggleState(engine.getTimelineProject().isTrackArmed(trackIndex), juce::dontSendNotification);
    }
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f));
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d)); 
    }
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.drawRect(getLocalBounds(), 1);

    // VERTICAL VU METER AT THE RIGHT EDGE
    auto meterBounds = getLocalBounds().removeFromRight(6).reduced(1, 2);
    g.setColour(DesignSystem::Colors::AppBackground.darker(0.2f));
    g.fillRect(meterBounds);

    if (currentLevel > 0.0f) {
        juce::ColourGradient cg(juce::Colours::lime, meterBounds.getBottomLeft().toFloat(), juce::Colours::red, meterBounds.getTopLeft().toFloat(), false);
        cg.addColour(0.7f, juce::Colours::yellow);
        
        int fillHeight = juce::roundToInt(meterBounds.getHeight() * currentLevel);
        auto fillBounds = meterBounds.withTrimmedTop(meterBounds.getHeight() - fillHeight);
        g.setGradientFill(cg);
        g.fillRect(fillBounds);
    }
}

void TrackHeaderComponent::updateMeters() {
    float newLevel = engine.getTrackPeakLevel(trackIndex);
    if (std::abs(newLevel - currentLevel) > 0.01f) {
        currentLevel = newLevel;
        repaint();
    }
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2).withTrimmedRight(6); // make room for vertical meter

    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    if (!track.parentGroupId.isNull()) {
        groupIndicator.setVisible(true);
        groupIndicator.setBounds(bounds.removeFromLeft(10));
        bounds.removeFromLeft(8); 
    } else {
        groupIndicator.setVisible(false);
    }

    auto topRow = bounds.removeFromTop(20);
    if (track.isGroup) {
        foldButton.setVisible(true);
        foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    } else {
        foldButton.setVisible(false);
    }
    powerToggle.setBounds(topRow.removeFromLeft(30).reduced(2));
    
    if (getHeight() < 40) { // Collapsed Layout
        armButton.setVisible(false);
        muteButton.setVisible(true);
        soloButton.setVisible(true);

        auto btnArea = topRow.removeFromRight(48);
        soloButton.setBounds(btnArea.removeFromLeft(24).reduced(2));
        muteButton.setBounds(btnArea.removeFromLeft(24).reduced(2));

        nameLabel.setBounds(topRow.reduced(2, 0));
    } else { // Expanded Layout
        muteButton.setVisible(true);
        soloButton.setVisible(true);
        armButton.setVisible(true);

        nameLabel.setBounds(topRow.reduced(2, 0));

        if (bounds.getHeight() >= 24) {
            auto bottomRow = bounds.removeFromTop(24);
            armButton.setBounds(bottomRow.removeFromLeft(24).reduced(2));
            soloButton.setBounds(bottomRow.removeFromLeft(24).reduced(2));
            muteButton.setBounds(bottomRow.removeFromLeft(24).reduced(2));
        } else {
            armButton.setBounds(0,0,0,0);
            soloButton.setBounds(0,0,0,0);
            muteButton.setBounds(0,0,0,0);
        }
    }
}

} // namespace Nimbus::Timeline