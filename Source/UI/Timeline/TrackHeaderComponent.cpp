#include "TrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::Timeline {

void TrackHeaderComponent::setupSvgButton(juce::DrawableButton& btn, const char* svgData, int svgSize) {
    std::unique_ptr<juce::Drawable> svg = juce::Drawable::createFromImageData(svgData, svgSize);
    if (svg != nullptr) {
        btn.setImages(svg.get(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
}

TrackHeaderComponent::TrackHeaderComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    addAndMakeVisible(foldButton);
    addAndMakeVisible(groupIndicator);
    
    foldButton.onClick = [this] {
        bool isFolded = engine.getTimelineProject().getTrack(trackIndex).isFolded;
        engine.getTimelineProject().setTrackFolded(trackIndex, !isFolded);
    };

    // Power Toggle (Track Number)
    addAndMakeVisible(powerToggle);
    powerToggle.setButtonText(juce::String(trackIndex + 1));
    powerToggle.setToggleState(true, juce::dontSendNotification);
    powerToggle.onClick = [this] {
        // Toggle track availability (mapped to mute for now)
        engine.getTimelineProject().setTrackMuted(trackIndex, !powerToggle.getToggleState());
    };

    addAndMakeVisible(nameLabel);
    nameLabel.setText("Audio " + juce::String(trackIndex + 1), juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f));
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkblue);
    muteButton.onClick = [this] {
        engine.getTimelineProject().setTrackMuted(trackIndex, muteButton.getToggleState());
    };

    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkgoldenrod);
    soloButton.onClick = [this] {
        engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState());
    };

    addAndMakeVisible(armButton);
    // Bind the SVG here. Verify 'recordarmtrack_svg' matches your BinaryData namespace exactly!
    // setupSvgButton(armButton, BinaryData::recordarmtrack_svg, BinaryData::recordarmtrack_svgSize);
    armButton.setClickingTogglesState(true);
    armButton.setToggleState(engine.getTimelineProject().isTrackArmed(trackIndex), juce::dontSendNotification);
    armButton.onClick = [this] {
        engine.getTimelineProject().setTrackArmed(trackIndex, armButton.getToggleState());
    };

    addAndMakeVisible(panSlider);
    panSlider.setRange(-1.0, 1.0, 0.01);
    panSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(-36.0, 12.0, 0.1);
    gainSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(effectsButton);

    addAndMakeVisible(linkIcon);
    linkIcon.setButtonText(DesignSystem::Iconography::Stereo);
    linkIcon.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
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

void TrackHeaderComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {}
void TrackHeaderComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {}
void TrackHeaderComponent::updateInputSources() {}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isShiftDown()) {
        int lastSelected = -1;
        auto& sel = engine.getTimelineProject().getSelectedTracks();
        if (sel.getNumRanges() > 0) lastSelected = sel.getRange(0).getStart();
        
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
        powerToggle.setToggleState(!isMuted, juce::dontSendNotification);
    }
}

void TrackHeaderComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) soloButton.setToggleState(isSoloed, juce::dontSendNotification);
}

void TrackHeaderComponent::trackArmChanged(int track, bool isArmed) {
    if (track == trackIndex) armButton.setToggleState(isArmed, juce::dontSendNotification);
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
    powerToggle.setButtonText(juce::String(trackIndex + 1));
    
    if (trackIndex >= 0 && trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& trackModel = engine.getTimelineProject().getTrack(trackIndex);
        nameLabel.setText(trackModel.name.isNotEmpty() ? trackModel.name : "Audio " + juce::String(trackIndex + 1), juce::dontSendNotification);
    } else {
        nameLabel.setText("Audio " + juce::String(trackIndex + 1), juce::dontSendNotification);
    }
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f));
    } else {
        g.fillAll(juce::Colour(0xff2d2d2d)); // Audacity Dark Background
    }
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.drawRect(getLocalBounds(), 1);

    // VU Meter logic
    int meterWidth = 4; 
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
    auto bounds = getLocalBounds().reduced(4);
    
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    if (!track.parentGroupId.isNull()) {
        groupIndicator.setVisible(true);
        groupIndicator.setBounds(bounds.removeFromLeft(10));
        
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
    
    bounds.removeFromRight(12); // Margin for VU meter
    if (!track.parentGroupId.isNull()) bounds.removeFromLeft(15); 

    bool isFolded = track.isFolded;
    bool isGroup = track.isGroup;
    
    auto topRow = bounds.removeFromTop(24);
    if (isGroup) {
        foldButton.setBounds(topRow.removeFromLeft(20).reduced(2));
    }
    
    powerToggle.setBounds(topRow.removeFromLeft(24).reduced(2));
    nameLabel.setBounds(topRow.reduced(2, 0));

    if (isFolded || isGroup) {
        muteButton.setVisible(false);
        soloButton.setVisible(false);
        armButton.setVisible(false);
        panSlider.setVisible(false);
        gainSlider.setVisible(false);
        effectsButton.setVisible(false);
    } else {
        muteButton.setVisible(true);
        soloButton.setVisible(true);
        armButton.setVisible(true);
        panSlider.setVisible(true);
        gainSlider.setVisible(true);
        effectsButton.setVisible(true);

        auto controlsArea = bounds.reduced(2, 2);
        
        // Mid Row: Arm, Sliders, Mute, Solo
        auto midRow = controlsArea.removeFromTop(24);
        armButton.setBounds(midRow.removeFromLeft(24).reduced(2));
        midRow.removeFromLeft(8); 
        
        soloButton.setBounds(midRow.removeFromRight(24).reduced(2));
        midRow.removeFromRight(2);
        muteButton.setBounds(midRow.removeFromRight(24).reduced(2));
        midRow.removeFromRight(8); 
        
        // Sliders stack
        auto sliderArea = midRow;
        panSlider.setBounds(sliderArea.removeFromTop(12));
        gainSlider.setBounds(sliderArea.removeFromTop(12));
        
        // Bottom Row: Effects
        auto bottomRow = controlsArea.removeFromBottom(22);
        effectsButton.setBounds(bottomRow.reduced(10, 0));
    }
}

} // namespace Nimbus::Timeline