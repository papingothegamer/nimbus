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
    nameLabel.setEditable(false, true, false);
    nameLabel.addListener(this);

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setButtonText(DesignSystem::Iconography::Solo);
    soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);

    addAndMakeVisible(armButton);
    armButton.setClickingTogglesState(true);
    armButton.setButtonText(DesignSystem::Iconography::RecordArm);
    armButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::RecordDanger);

    // Routing combo boxes (placeholder items)
    addAndMakeVisible(sourceLabel);
    addAndMakeVisible(destLabel);
    
    sourceLabel.setFont(juce::Font(10.0f));
    destLabel.setFont(juce::Font(10.0f));
    sourceLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    destLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);

    addAndMakeVisible(sourceBox);
    addAndMakeVisible(destBox);
    
    sourceBox.addItem("Ext. In", 1);
    sourceBox.setSelectedItemIndex(0);
    
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
    foldButton.setButtonText(isFolded ? DesignSystem::Iconography::Fold : DesignSystem::Iconography::Unfold);
    
    startTimerHz(30);
}

TrackHeaderComponent::~TrackHeaderComponent() {
    stopTimer();
    engine.getTimelineProject().removeListener(this);
}

void TrackHeaderComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel) {
        // We'd add setTrackName to TimelineProject if needed, skipping for now
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
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) { /* Rename */ }
            else if (result == 2) { engine.addTrack(false); }
            else if (result == 3) { engine.addTrack(true); }
            else if (result == 4) { /* Duplicate */ }
            else if (result == 5) { engine.getTimelineProject().removeTrack(trackIndex); }
            else if (result == 6) { engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks()); }
        });
    }
}

void TrackHeaderComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        numberButton.setToggleState(!isMuted, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackSelectionChanged() {
    repaint();
}

void TrackHeaderComponent::trackFoldStateChanged(int track, bool isFolded) {
    if (track == trackIndex) {
        foldButton.setButtonText(isFolded ? DesignSystem::Iconography::Fold : DesignSystem::Iconography::Unfold);
        resized();
        repaint();
    }
}

void TrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    numberButton.setButtonText(juce::String(trackIndex + 1));
    nameLabel.setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ModuleBackground.brighter(0.1f));
    } else {
        g.fillAll(DesignSystem::Colors::ModuleBackground);
    }
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    // Right separator (header is on left side)
    g.fillRect(getWidth() - 1, 0, 1, getHeight());

    // Draw VU Meter or MIDI Activity on the right edge
    int meterWidth = 8;
    auto meterBounds = juce::Rectangle<int>(getWidth() - meterWidth - 4, 4, meterWidth, getHeight() - 8);
    
    g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.2f));
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

void TrackHeaderComponent::timerCallback() {
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
    
    numberButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    
    if (!isGroup && !isFolded) {
        armButton.setBounds(topRow.removeFromRight(20).reduced(2));
        soloButton.setBounds(topRow.removeFromRight(20).reduced(2));
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
