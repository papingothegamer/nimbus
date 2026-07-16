#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::Timeline {

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    addAndMakeVisible(foldButton);
    addAndMakeVisible(groupIndicator);
    foldButton.onClick = [this] {
        bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
        engine.getTimelineProject().setTrackFolded(trackIndex, !isFolded);
    };

    addAndMakeVisible(selectButton);
    selectButton.setClickingTogglesState(true);
    selectButton.setToggleState(false, juce::dontSendNotification);
    selectButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.7f));
    
    selectButton.onClick = [this] {
        engine.getTimelineProject().setTrackSelected(trackIndex, selectButton.getToggleState());
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Mute);
    muteButton.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, muteButton.getToggleState());
    };

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);
    soloButton.onClick = [this] {
        engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState());
    };

    addAndMakeVisible(panSlider);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0, juce::dontSendNotification);
    panSlider.setTextValueSuffix("");
    panSlider.onValueChange = [this] {
        // Handle pan
    };

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(-36.0, 12.0, 0.1);
    gainSlider.setValue(0.0, juce::dontSendNotification);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.onValueChange = [this] {
        // Handle gain
    };

    lPan.setFont(10.0f); addAndMakeVisible(lPan);
    rPan.setFont(10.0f); addAndMakeVisible(rPan);
    mGain.setFont(10.0f); addAndMakeVisible(mGain);
    pGain.setFont(10.0f); addAndMakeVisible(pGain);

    addAndMakeVisible(linkIcon);
    linkIcon.setButtonText(DesignSystem::Iconography::Stereo);
    linkIcon.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    linkIcon.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
    linkIcon.setMouseCursor(juce::MouseCursor::NormalCursor);
    linkIcon.setInterceptsMouseClicks(false, false);
    linkIcon.setVisible(false);


    engine.getTimelineProject().addListener(this);
    
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    foldButton.setButtonText(isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold);
}

TrackHeaderComponent::~TrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().removeChangeListener(this);
}

void TrackHeaderComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel) {
        engine.getTimelineProject().setTrackName(trackIndex, nameLabel.getText());
    }
}

void TrackHeaderComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
}

void TrackHeaderComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
}

void TrackHeaderComponent::updateInputSources() {
}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isShiftDown()) {
        int lastSelected = -1;
        auto& sel = engine.getTimelineProject().getSelectedTracks();
        if (sel.getNumRanges() > 0) lastSelected = sel.getRange(0).getStart(); // heuristic
        
        if (lastSelected != -1) {
            engine.getTimelineProject().selectTrackRange(lastSelected, trackIndex);
        } else {
            engine.getTimelineProject().setTrackSelected(trackIndex, true);
        }
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
            if (result == 1) { /* Rename */ }
            else if (result == 2) { engine.addTrack(false); }
            else if (result == 3) { engine.addTrack(true); }
            else if (result == 5) { engine.getTimelineProject().removeTrack(trackIndex); }
            else if (result == 6) { engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks()); }
            else if (result == 7) { 
                int t1 = selectedTracks[0];
                int t2 = selectedTracks[1];
                engine.getTimelineProject().linkTracks(t1, t2);
            }
            else if (result == 8) { engine.getTimelineProject().unlinkTrack(trackIndex); }
        });
    }
}

void TrackHeaderComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) {
        soloButton.setToggleState(isSoloed, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackArmChanged(int track, bool isArmed) {
    // Arm logic not needed in basic audacity header, but handled silently if added later
}

void TrackHeaderComponent::trackSelectionChanged() {
    auto& track = engine.getTimelineProject().getTrack(trackIndex);
    linkIcon.setVisible(!track.linkedTrackId.isNull());
    repaint();
}

void TrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        foldButton.setButtonText(isFolded ? DesignSystem::Iconography::Unfold : DesignSystem::Iconography::Fold);
        resized();
        repaint();
    }
}

void TrackHeaderComponent::trackNameChanged(int track, const juce::String& newName) {
    if (track == trackIndex) {
        if (!nameLabel.isBeingEdited()) {
            nameLabel.setText(newName, juce::dontSendNotification);
        }
    }
}

void TrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    if (trackIndex >= 0 && trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : "Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
    } else {
        nameLabel.setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
    }
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ComponentBackground);
    } else {
        g.fillAll(DesignSystem::Colors::AppBackground.brighter(0.05f));
    }
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    // Draw VU Meter or MIDI Activity on the right edge
    int meterWidth = 4; // sleek thin meter
    auto meterBounds = juce::Rectangle<int>(getWidth() - meterWidth, 1, meterWidth, getHeight() - 2);
    
    g.setColour(DesignSystem::Colors::AppBackground.darker(0.2f));
    g.fillRect(meterBounds);

    if (currentLevel > 0.0f) {
        juce::ColourGradient cg(juce::Colours::lime, meterBounds.getBottomLeft().toFloat(),
                                juce::Colours::red, meterBounds.getTopLeft().toFloat(), false);
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
        repaint(getWidth() - 16, 0, 16, getHeight());
    }
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    
    // Group indicator on the left
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    if (!track.parentGroupId.isNull()) {
        groupIndicator.setVisible(true);
        groupIndicator.setBounds(bounds.removeFromLeft(10));
        
        // Determine if it's the last in the group
        bool isLast = true;
        for (int i = trackIndex + 1; i < engine.getTimelineProject().getNumTracks(); ++i) {
            if (engine.getTimelineProject().getTrack(i).parentGroupId == track.parentGroupId) {
                isLast = false;
                break;
            }
        }
        groupIndicator.setIsLastInGroup(isLast);
    } else {
        groupIndicator.setVisible(false);
    }
    
    // Remove space for VU meter on the far right
    bounds.removeFromRight(12);
    
    // Indent based on grouping
    bool isGroupChild = !engine.getTimelineProject().getTrack(trackIndex).parentGroupId.isNull();
    if (isGroupChild) {
        bounds.removeFromLeft(15);
    }

    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    bool isGroup = engine.getTimelineProject().getTrack(trackIndex).isGroup;
    
    // Top Row logic (Select, Name)
    auto topRow = bounds.removeFromTop(24);
    if (isGroup) {
        foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    } else {
        topRow.removeFromLeft(4); // Spacing if not group
    }
    
    selectButton.setBounds(topRow.removeFromLeft(50).reduced(2));
    nameLabel.setBounds(topRow.reduced(2, 0));

    if (isFolded || isGroup) {
        muteButton.setVisible(false);
        soloButton.setVisible(false);
        panSlider.setVisible(false);
        gainSlider.setVisible(false);
        lPan.setVisible(false);
        rPan.setVisible(false);
        mGain.setVisible(false);
        pGain.setVisible(false);
    } else {
        muteButton.setVisible(true);
        soloButton.setVisible(true);
        panSlider.setVisible(true);
        gainSlider.setVisible(true);
        lPan.setVisible(true);
        rPan.setVisible(true);
        mGain.setVisible(true);
        pGain.setVisible(true);

        auto controlsArea = bounds.reduced(2, 2);
        
        auto msRow = controlsArea.removeFromTop(24);
        muteButton.setBounds(msRow.removeFromLeft(msRow.getWidth() / 2).reduced(2));
        soloButton.setBounds(msRow.reduced(2));

        auto pRow = controlsArea.removeFromTop(20);
        lPan.setBounds(pRow.removeFromLeft(15));
        rPan.setBounds(pRow.removeFromRight(15));
        panSlider.setBounds(pRow);
        
        auto gRow = controlsArea.removeFromTop(20);
        mGain.setBounds(gRow.removeFromLeft(15));
        pGain.setBounds(gRow.removeFromRight(15));
        gainSlider.setBounds(gRow);
    }
}

} // namespace Nimbus::Timeline
