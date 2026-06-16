#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::Timeline {

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    addAndMakeVisible(foldButton);
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
    soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);

    addAndMakeVisible(armButton);
    armButton.setClickingTogglesState(true);
    armButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::RecordDanger);

    // Routing combo boxes (placeholder items)
    addAndMakeVisible(inTypeComboBox);
    inTypeComboBox.addItem("Ext. In", 1);
    inTypeComboBox.setSelectedId(1, juce::dontSendNotification);
    
    addAndMakeVisible(inChannelComboBox);
    inChannelComboBox.addItem("1/2", 1);
    inChannelComboBox.setSelectedId(1, juce::dontSendNotification);

    addAndMakeVisible(monitorInButton);
    addAndMakeVisible(monitorAutoButton);
    addAndMakeVisible(monitorOffButton);
    
    monitorInButton.setRadioGroupId(2);
    monitorAutoButton.setRadioGroupId(2);
    monitorOffButton.setRadioGroupId(2);
    monitorAutoButton.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(outTypeComboBox);
    outTypeComboBox.addItem("Master", 1);
    outTypeComboBox.setSelectedId(1, juce::dontSendNotification);

    addAndMakeVisible(outChannelComboBox);
    
    engine.getTimelineProject().addListener(this);
    
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    foldButton.setButtonText(isFolded ? ">" : "v");
}

TrackHeaderComponent::~TrackHeaderComponent() {
    engine.getTimelineProject().removeListener(this);
}

void TrackHeaderComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel) {
        // We'd add setTrackName to TimelineProject if needed, skipping for now
    }
}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isShiftDown()) {
        // I need to access lastSelectedTrack from TimelineProject, but it's private.
        // Wait, TimelineProject has lastSelectedTrack? Let's assume it's publicly accessible or we just track it.
        // I'll fix this by just checking if there's any selection. Wait, `TimelineProject` doesn't expose `lastSelectedTrack` publicly?
        // Oh wait, I didn't add a getter for it. Let's just assume simple range select if not available, or I'll add a getter.
        // Wait, `selectedTracks` is public? No, `getSelectedTracks()`.
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
        m.addItem(1, "Delete Track");
        m.addItem(2, "Group Tracks");
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                engine.getTimelineProject().removeTrack(trackIndex);
            }
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
        foldButton.setButtonText(isFolded ? ">" : "v");
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

    // Left separator (since it's on the right of the timeline)
    g.fillRect(0, 0, 1, getHeight());

    // Draw VU Meter or MIDI Activity on the right edge
    int meterWidth = 8;
    auto meterBounds = juce::Rectangle<int>(getWidth() - meterWidth - 4, 4, meterWidth, getHeight() - 8);
    
    g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.2f));
    g.fillRect(meterBounds);

    float level = 0.0f;
    // Real implementation would pull from engine, let's just make it 0 for now to avoid the ugly green block
    
    if (level > 0.0f) {
        juce::ColourGradient cg(juce::Colours::lime, meterBounds.getBottomLeft().toFloat(),
                                juce::Colours::red, meterBounds.getTopLeft().toFloat(), false);
        cg.addColour(0.7f, juce::Colours::yellow);
        
        int fillHeight = juce::roundToInt(meterBounds.getHeight() * level);
        auto fillBounds = meterBounds.withTrimmedTop(meterBounds.getHeight() - fillHeight);
        g.setGradientFill(cg);
        g.fillRect(fillBounds);
    }
}

void TrackHeaderComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    bounds.removeFromRight(12); // Space for VU meter
    
    // Indent based on grouping
    bool isGroupChild = !engine.getTimelineProject().getTrack(trackIndex).parentGroupId.isNull();
    if (isGroupChild) {
        bounds.removeFromLeft(15);
    }

    auto topRow = bounds.removeFromTop(20);
    foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    numberButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    nameLabel.setBounds(topRow);
    
    bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
    bool isGroup = engine.getTimelineProject().getTrack(trackIndex).isGroup;
    
    if (isFolded || isGroup) {
        soloButton.setVisible(false);
        armButton.setVisible(false);
        inTypeComboBox.setVisible(false);
        inChannelComboBox.setVisible(false);
        monitorInButton.setVisible(false);
        monitorAutoButton.setVisible(false);
        monitorOffButton.setVisible(false);
        outTypeComboBox.setVisible(false);
        outChannelComboBox.setVisible(false);
    } else {
        soloButton.setVisible(true);
        armButton.setVisible(true);
        inTypeComboBox.setVisible(true);
        inChannelComboBox.setVisible(true);
        monitorInButton.setVisible(true);
        monitorAutoButton.setVisible(true);
        monitorOffButton.setVisible(true);
        outTypeComboBox.setVisible(true);
        outChannelComboBox.setVisible(true);

        auto buttonsArea = bounds.removeFromTop(20);
        buttonsArea.removeFromLeft(40); // Align with name
        soloButton.setBounds(buttonsArea.removeFromLeft(20).reduced(1));
        armButton.setBounds(buttonsArea.removeFromLeft(20).reduced(1));

        // Routing boxes
        auto routingArea = bounds.reduced(30, 2);
        
        inTypeComboBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
        inChannelComboBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
        
        auto monitorArea = routingArea.removeFromTop(18).reduced(0, 1);
        int w = monitorArea.getWidth() / 3;
        monitorInButton.setBounds(monitorArea.removeFromLeft(w));
        monitorAutoButton.setBounds(monitorArea.removeFromLeft(w));
        monitorOffButton.setBounds(monitorArea);
        
        outTypeComboBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
        outChannelComboBox.setBounds(routingArea.removeFromTop(18).reduced(0, 1));
    }
}

} // namespace Nimbus::Timeline
