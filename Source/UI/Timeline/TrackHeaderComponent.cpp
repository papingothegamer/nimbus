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

    addAndMakeVisible(numberButton);
    numberButton.setButtonText(juce::String(trackIndex + 1));
    numberButton.setClickingTogglesState(true);
    numberButton.setToggleState(true, juce::dontSendNotification); // Active by default
    numberButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.7f));
    numberButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey.withAlpha(0.3f)); // Muted state
    numberButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    numberButton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);

    numberButton.onClick = [this] {
        bool isMuted = !numberButton.getToggleState();
        engine.getTimelineProject().setTrackMuted(trackIndex, isMuted);
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(linkIcon);
    linkIcon.setButtonText(DesignSystem::Iconography::Stereo);
    linkIcon.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    linkIcon.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
    linkIcon.setMouseCursor(juce::MouseCursor::NormalCursor);
    linkIcon.setInterceptsMouseClicks(false, false);
    linkIcon.setVisible(false);

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setButtonText(DesignSystem::Iconography::Solo);
    soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);

    addAndMakeVisible(armButton);
    armButton.setClickingTogglesState(true);
    armButton.setButtonText(DesignSystem::Iconography::RecordArm);
    armButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::RecordDanger);
    
    armButton.onClick = [this] {
        engine.getTimelineProject().setTrackArmed(trackIndex, armButton.getToggleState());
    };

    soloButton.onClick = [this] {
        engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState());
    };

    // Routing combo boxes (placeholder items)
    addAndMakeVisible(sourceLabel);
    addAndMakeVisible(destLabel);
    
    sourceLabel.setFont(juce::Font(10.0f));
    destLabel.setFont(juce::Font(10.0f));
    sourceLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    destLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);

    addAndMakeVisible(sourceBox);
    addAndMakeVisible(destBox);
    
    sourceBox.addListener(this);
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().addChangeListener(this);
    updateInputSources();
    
    destBox.addItem("Master", 1);
    destBox.setSelectedItemIndex(0);

    addAndMakeVisible(monitorInButton);
    addAndMakeVisible(monitorAutoButton);
    addAndMakeVisible(monitorOffButton);
    
    monitorInButton.setButtonText("In");
    monitorAutoButton.setButtonText("Auto");
    monitorOffButton.setButtonText("Off");
    
    monitorInButton.setClickingTogglesState(true);
    monitorAutoButton.setClickingTogglesState(true);
    monitorOffButton.setClickingTogglesState(true);
    
    monitorInButton.setRadioGroupId(2);
    monitorAutoButton.setRadioGroupId(2);
    monitorOffButton.setRadioGroupId(2);
    monitorAutoButton.setToggleState(true, juce::dontSendNotification);


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
    if (comboBoxThatHasChanged == &sourceBox) {
        int index = sourceBox.getSelectedItemIndex();
        int inputChannel = index - 1; // 0 = All Channels (-1), 1 = In 1 (0), 2 = In 2 (1)
        engine.getTimelineProject().setTrackInputChannel(trackIndex, inputChannel);
    }
}

void TrackHeaderComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &engine.getAudioDeviceManager().getJuceAudioDeviceManager()) {
        updateInputSources();
    }
}

void TrackHeaderComponent::updateInputSources() {
    int currentSelection = sourceBox.getSelectedItemIndex();
    
    sourceBox.clear();
    if (auto* device = engine.getAudioDeviceManager().getJuceAudioDeviceManager().getCurrentAudioDevice()) {
        auto activeChannels = device->getActiveInputChannels();
        auto channelNames = device->getInputChannelNames();
        
        sourceBox.addItem("All Channels", 1);
        int itemIndex = 2;
        
        for (int i = 0; i < channelNames.size(); ++i) {
            if (activeChannels[i]) {
                sourceBox.addItem(channelNames[i], itemIndex++);
            }
        }
    }
    
    if (sourceBox.getNumItems() == 0) {
        sourceBox.addItem("No Input", 1);
    }
    
    int modelIndex = engine.getTimelineProject().getTrack(trackIndex).inputChannelIndex;
    int desiredSelection = modelIndex + 1; // -1 becomes 0 (All Channels), 0 becomes 1
    
    if (desiredSelection >= 0 && desiredSelection < sourceBox.getNumItems()) {
        sourceBox.setSelectedItemIndex(desiredSelection, juce::dontSendNotification);
    } else {
        sourceBox.setSelectedItemIndex(0, juce::dontSendNotification);
    }
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
        numberButton.setToggleState(!isMuted, juce::dontSendNotification);
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
    numberButton.setButtonText(juce::String(trackIndex + 1));
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    nameLabel.setText(track.name.isNotEmpty() ? track.name : "Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
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
    
    // Top Row logic (Name, Number, Solo, Arm)
    auto topRow = bounds.removeFromTop(24);
    if (isGroup) {
        foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    } else {
        topRow.removeFromLeft(4); // Spacing if not group
    }
    
    numberButton.setBounds(topRow.removeFromLeft(24).reduced(2));
    
    if (!isGroup && !isFolded) {
        armButton.setBounds(topRow.removeFromRight(20).reduced(2));
        soloButton.setBounds(topRow.removeFromRight(20).reduced(2));
    }
    
    if (linkIcon.isVisible()) {
        linkIcon.setBounds(topRow.removeFromRight(16).reduced(2));
    }
    
    nameLabel.setBounds(topRow.reduced(2, 0));
    
    if (isFolded || isGroup) {
        soloButton.setVisible(false);
        armButton.setVisible(false);
        sourceLabel.setVisible(false);
        destLabel.setVisible(false);
        sourceBox.setVisible(false);
        destBox.setVisible(false);
        monitorInButton.setVisible(false);
        monitorAutoButton.setVisible(false);
        monitorOffButton.setVisible(false);
    } else {
        soloButton.setVisible(true);
        armButton.setVisible(true);
        sourceLabel.setVisible(true);
        destLabel.setVisible(true);
        sourceBox.setVisible(true);
        destBox.setVisible(true);
        monitorInButton.setVisible(true);
        monitorAutoButton.setVisible(true);
        monitorOffButton.setVisible(true);

        // Routing Area
        auto routingArea = bounds.reduced(2, 2);
        
        // Hide labels to save vertical space
        sourceLabel.setVisible(false);
        destLabel.setVisible(false);
        
        // Stack vertically
        sourceBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
        routingArea.removeFromTop(2);
        
        auto monitorArea = routingArea.removeFromTop(16); 
        int w = monitorArea.getWidth() / 3;
        monitorInButton.setBounds(monitorArea.removeFromLeft(w).reduced(1, 0));
        monitorAutoButton.setBounds(monitorArea.removeFromLeft(w).reduced(1, 0));
        monitorOffButton.setBounds(monitorArea.reduced(1, 0));
        
        routingArea.removeFromTop(2);
        
        destBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
    }
}

} // namespace Nimbus::Timeline
