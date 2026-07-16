#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::Timeline {

void TrackHeaderComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName, juce::Colour color) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        juce::String svgStr(data, (size_t)size);
        juce::String hexColor = color.toDisplayString(false).replace("#", "");
        juce::String fillStr = "fill=\"#" + hexColor + "\"";
        
        svgStr = svgStr.replace("fill=\"#000000\"", fillStr)
                       .replace("fill=\"#212121\"", fillStr)
                       .replace("fill=\"currentColor\"", fillStr)
                       .replace("<svg ", "<svg " + fillStr + " color=\"#" + hexColor + "\" ");

        if (auto xml = juce::XmlDocument::parse(svgStr)) {
            if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
            }
        }
    }
}

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;

    addAndMakeVisible(foldButton);
    foldButton.setClickingTogglesState(true);
    loadSvgIcon(foldButton, isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold, juce::Colours::grey);
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
    powerToggle.onClick = [this] { engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState()); };

    addAndMakeVisible(nameLabel);
    nameLabel.setText("Audio " + juce::String(trackIndex + 1), juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    loadSvgIcon(muteButton, DesignSystem::Iconography::Unmute, juce::Colour(0xff2f363d));
    muteButton.onClick = [this] { 
        bool isMuted = muteButton.getToggleState();
        engine.getTimelineProject().setTrackMuted(trackIndex, isMuted);
        loadSvgIcon(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute, isMuted ? juce::Colours::white : juce::Colour(0xff2f363d));
    };

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    loadSvgIcon(soloButton, DesignSystem::Iconography::Solo, juce::Colours::grey);
    soloButton.onClick = [this] {
        bool isSoloed = soloButton.getToggleState();
        engine.getTimelineProject().setTrackSoloed(trackIndex, isSoloed);
        loadSvgIcon(soloButton, DesignSystem::Iconography::Solo, isSoloed ? juce::Colours::yellow : juce::Colours::grey);
    };

    addAndMakeVisible(armButton);
    armButton.setClickingTogglesState(true);
    bool isArmed = engine.getTimelineProject().isTrackArmed(trackIndex);
    loadSvgIcon(armButton, DesignSystem::Iconography::RecordArm, isArmed ? juce::Colours::red : juce::Colours::grey);
    armButton.setToggleState(isArmed, juce::dontSendNotification);
    armButton.onClick = [this] {
        bool armed = armButton.getToggleState();
        engine.getTimelineProject().setTrackArmed(trackIndex, armed);
        loadSvgIcon(armButton, DesignSystem::Iconography::RecordArm, armed ? juce::Colours::red : juce::Colours::grey);
    };

    addAndMakeVisible(fader);
    fader.slider.onValueChange = [this]() {
        if (trackIndex != -1) {
            engine.getTimelineProject().setTrackVolume(trackIndex, static_cast<float>(fader.slider.getValue()));
        }
    };

    engine.getTimelineProject().addListener(this);
}

TrackHeaderComponent::~TrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().removeChangeListener(this);
}

void TrackHeaderComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel) engine.getTimelineProject().setTrackName(trackIndex, nameLabel.getText());
}
void TrackHeaderComponent::comboBoxChanged(juce::ComboBox*) {}
void TrackHeaderComponent::changeListenerCallback(juce::ChangeBroadcaster*) {}
void TrackHeaderComponent::updateInputSources() {}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isShiftDown()) {
        int lastSelected = -1;
        auto& sel = engine.getTimelineProject().getSelectedTracks();
        if (sel.getNumRanges() > 0) lastSelected = sel.getRange(0).getStart();
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
        m.addItem(4, "Duplicate", false, false);
        m.addItem(5, "Delete", true, false);
        m.addSeparator();
        m.addItem(6, "Group Tracks", true, false);
        
        auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
        if (selectedTracks.size() == 2 && selectedTracks.contains(trackIndex)) {
            m.addItem(7, "Link Selected Tracks", true, false);
        }
        if (!engine.getTimelineProject().getTrack(trackIndex).linkedTrackId.isNull()) {
            m.addItem(8, "Unlink Track", true, false);
        }
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this, selectedTracks](int result) {
            if (result == 2) { engine.addTrack(false); }
            else if (result == 3) { engine.addTrack(true); }
            else if (result == 5) { engine.getTimelineProject().removeTrack(trackIndex); }
            else if (result == 6) { engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks()); }
            else if (result == 7) { engine.getTimelineProject().linkTracks(selectedTracks[0], selectedTracks[1]); }
            else if (result == 8) { engine.getTimelineProject().unlinkTrack(trackIndex); }
        });
    }
}

void TrackHeaderComponent::trackVolumeChanged(int track, float volume) {
    if (track == trackIndex) {
        fader.slider.setValue(volume, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
        loadSvgIcon(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute, isMuted ? juce::Colours::white : juce::Colour(0xff2f363d));
    }
}

void TrackHeaderComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) {
        soloButton.setToggleState(isSoloed, juce::dontSendNotification);
        loadSvgIcon(soloButton, DesignSystem::Iconography::Solo, isSoloed ? juce::Colours::yellow : juce::Colours::grey);
    }
}

void TrackHeaderComponent::trackArmChanged(int track, bool isArmed) {
    if (track == trackIndex) {
        armButton.setToggleState(isArmed, juce::dontSendNotification);
        loadSvgIcon(armButton, DesignSystem::Iconography::RecordArm, isArmed ? juce::Colours::red : juce::Colours::grey);
    }
}

void TrackHeaderComponent::trackSelectionChanged() {
    repaint();
}

void TrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        loadSvgIcon(foldButton, isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold, juce::Colours::grey);
        resized();
        repaint();
    }
}

void TrackHeaderComponent::trackNameChanged(int track, const juce::String& newName) {
    if (track == trackIndex && !nameLabel.isBeingEdited()) {
        nameLabel.setText(newName, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    int displayNum = 1;
    for (int i = 0; i < trackIndex; ++i) {
        if (!engine.getTimelineProject().getTrack(i).isGroup) displayNum++;
    }

    if (trackIndex >= 0 && trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
        
        fader.setTrackInfo(trackModel.type, trackModel.isStereo);
        fader.slider.setValue(trackModel.volume, juce::dontSendNotification);

        powerToggle.setButtonText(trackModel.isGroup ? "" : juce::String(displayNum));
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : (trackModel.isGroup ? "Group" : "Audio " + juce::String(displayNum)), juce::dontSendNotification);
        
        loadSvgIcon(foldButton, trackModel.isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold, juce::Colours::grey);
        muteButton.setToggleState(trackModel.isMuted, juce::dontSendNotification);
        loadSvgIcon(muteButton, trackModel.isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute, trackModel.isMuted ? juce::Colours::white : juce::Colour(0xff2f363d));
        
        soloButton.setToggleState(trackModel.isSoloed, juce::dontSendNotification);
        loadSvgIcon(soloButton, DesignSystem::Iconography::Solo, trackModel.isSoloed ? juce::Colours::yellow : juce::Colours::grey);

        bool isArmed = engine.getTimelineProject().isTrackArmed(trackIndex);
        armButton.setToggleState(isArmed, juce::dontSendNotification);
        loadSvgIcon(armButton, DesignSystem::Iconography::RecordArm, isArmed ? juce::Colours::red : juce::Colours::grey);
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
}

void TrackHeaderComponent::updateMeters() {
    float newLevel = engine.getTrackPeakLevel(trackIndex);
    fader.setLevel(newLevel);
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);

    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    if (!track.parentGroupId.isNull()) {
        groupIndicator.setVisible(true);
        groupIndicator.setBounds(bounds.removeFromLeft(10));
        bounds.removeFromLeft(8); // Indent
    } else {
        groupIndicator.setVisible(false);
    }

    auto topRow = bounds.removeFromTop(20);
    if (track.isGroup) {
        foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    }
    
    powerToggle.setBounds(topRow.removeFromLeft(30).reduced(2));
    nameLabel.setBounds(topRow.reduced(2, 0));

    if (track.isFolded || track.isGroup) {
        muteButton.setVisible(false);
        soloButton.setVisible(false);
        armButton.setVisible(false);
        fader.setVisible(false);
    } else {
        muteButton.setVisible(true);
        soloButton.setVisible(true);
        armButton.setVisible(true);
        fader.setVisible(true);

        auto controlsArea = bounds.reduced(2, 2);
        
        // Fader on the right side
        fader.setBounds(controlsArea.removeFromRight(24));
        controlsArea.removeFromRight(4); // padding

        auto btnRow = controlsArea.removeFromTop(24);
        armButton.setBounds(btnRow.removeFromLeft(24).reduced(2));
        soloButton.setBounds(btnRow.removeFromLeft(24).reduced(2));
        btnRow.removeFromLeft(2);
        muteButton.setBounds(btnRow.removeFromLeft(24).reduced(2));
    }
}

} // namespace Nimbus::Timeline