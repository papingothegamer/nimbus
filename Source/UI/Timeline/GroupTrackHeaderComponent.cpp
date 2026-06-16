#include "GroupTrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::Timeline {

GroupTrackHeaderComponent::GroupTrackHeaderComponent(NimbusEngine& e, int tIndex)
    : engine(e), trackIndex(tIndex)
{
    engine.getTimelineProject().addListener(this);

    addAndMakeVisible(nameLabel);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    nameLabel.setText(track.name, juce::dontSendNotification);

    addAndMakeVisible(numberButton);
    numberButton.setButtonText(juce::String(trackIndex + 1));
    numberButton.setClickingTogglesState(true);
    numberButton.setToggleState(true, juce::dontSendNotification);
    numberButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.7f));
    numberButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey.withAlpha(0.3f));
    numberButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    numberButton.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
    numberButton.onClick = [this] {
        bool isMuted = !numberButton.getToggleState();
        engine.getTimelineProject().setTrackMuted(trackIndex, isMuted);
    };

    addAndMakeVisible(foldButton);
    foldButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentWhite);
    foldButton.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
    foldButton.onClick = [this]() {
        const auto& trk = engine.getTimelineProject().getTrack(trackIndex);
        engine.getTimelineProject().setTrackFolded(trackIndex, !trk.isFolded);
    };

    addAndMakeVisible(muteButton);
    muteButton.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::PanelBackground);
    muteButton.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
    
    addAndMakeVisible(soloButton);
    soloButton.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::PanelBackground);
    soloButton.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);

    addAndMakeVisible(sourceBox);
    sourceBox.addItem("Ext. In", 1);
    sourceBox.setSelectedItemIndex(0);
    destBox.addItem("Master", 1);
    destBox.setSelectedItemIndex(0);

    addAndMakeVisible(sourceLabel);
    addAndMakeVisible(destLabel);
    
    sourceLabel.setFont(juce::Font(10.0f));
    destLabel.setFont(juce::Font(10.0f));
    sourceLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    destLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);

    updateSelectionState();
    updateFoldState();
}

GroupTrackHeaderComponent::~GroupTrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
}

void GroupTrackHeaderComponent::paint(juce::Graphics& g) {
    bool isSelected = engine.getTimelineProject().isTrackSelected(trackIndex);
    
    // Group headers usually have a slightly different or stronger background
    juce::Colour bgColor = isSelected ? DesignSystem::Colors::PrimaryAction.withAlpha(0.2f) : DesignSystem::Colors::PanelBackground.brighter(0.1f);
    g.fillAll(bgColor);
    
    // Right border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
    
    // Bottom border
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void GroupTrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    
    // Reserve space for VU meter on far right (to match regular tracks)
    bounds.removeFromRight(12);
    
    // Top Row logic
    auto topRow = bounds.removeFromTop(24);
    foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    numberButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    
    soloButton.setBounds(topRow.removeFromRight(20).reduced(2));
    muteButton.setBounds(topRow.removeFromRight(20).reduced(2));
    
    nameLabel.setBounds(topRow.reduced(2, 0));
    
    auto routingArea = bounds.reduced(2, 2);
    auto leftColumn = routingArea.removeFromLeft(routingArea.getWidth() / 2).reduced(2, 0);
    
    sourceLabel.setBounds(leftColumn.removeFromTop(12));
    sourceBox.setBounds(leftColumn.removeFromTop(18).reduced(0, 1));
    
    leftColumn.removeFromTop(2); // Spacing
    
    destLabel.setBounds(leftColumn.removeFromTop(12));
    destBox.setBounds(leftColumn.removeFromTop(18).reduced(0, 1));
}

void GroupTrackHeaderComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
    if (trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        nameLabel.setText(track.name, juce::dontSendNotification);
        numberButton.setButtonText(juce::String(trackIndex + 1));
    }
    updateSelectionState();
    repaint();
}

void GroupTrackHeaderComponent::trackSelectionChanged() {
    updateSelectionState();
    repaint();
}

void GroupTrackHeaderComponent::trackFoldStateChanged(int changedTrackIndex, bool isFolded) {
    if (changedTrackIndex == trackIndex) {
        updateFoldState();
    }
}

void GroupTrackHeaderComponent::updateSelectionState() {
    bool isSelected = engine.getTimelineProject().isTrackSelected(trackIndex);
    if (isSelected) {
        nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    } else {
        nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    }
}

void GroupTrackHeaderComponent::updateFoldState() {
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    foldButton.setButtonText(track.isFolded ? ">" : "v");
}

void GroupTrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isRightButtonDown()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Rename", true, false);
        menu.addSeparator();
        menu.addItem(2, "Insert Audio Track", true, false);
        menu.addItem(3, "Insert MIDI Track", true, false);
        menu.addSeparator();
        menu.addItem(4, "Duplicate", false, false);
        menu.addItem(5, "Delete", true, false);
        menu.addSeparator();
        menu.addItem(6, "Ungroup Tracks", true, false);
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) { /* Rename */ }
            else if (result == 2) { engine.addTrack(false); }
            else if (result == 3) { engine.addTrack(true); }
            else if (result == 4) { /* Duplicate */ }
            else if (result == 5) { engine.getTimelineProject().removeTrack(trackIndex); }
            else if (result == 6) { engine.getTimelineProject().ungroupTracks(trackIndex); }
        });
    } else {
        bool clearExisting = !event.mods.isShiftDown() && !event.mods.isCtrlDown() && !event.mods.isCommandDown();
        
        if (event.mods.isShiftDown()) {
            engine.getTimelineProject().selectTrackRange(engine.getTimelineProject().getLastSelectedTrack(), trackIndex);
        } else if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
            engine.getTimelineProject().toggleTrackSelection(trackIndex);
        } else {
            engine.getTimelineProject().setTrackSelected(trackIndex, clearExisting);
        }
    }
}

void GroupTrackHeaderComponent::mouseDoubleClick(const juce::MouseEvent& event) {
    // Fold/unfold on double click
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    engine.getTimelineProject().setTrackFolded(trackIndex, !track.isFolded);
}

} // namespace Nimbus::Timeline
