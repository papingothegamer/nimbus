#include "DetailViewComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

DetailViewComponent::DetailViewComponent(NimbusEngine& e) : engine(e), pianoRoll(e), pianoRollTimeline(e), clipProperties(e), audioClipView(e), deviceChain(e) {
    juce::Logger::writeToLog("DetailViewComponent constructor start");
    addChildComponent(placeholderLabel);
    addChildComponent(clipProperties);
    addChildComponent(pianoRollTimeline);
    addChildComponent(pianoRoll);
    addChildComponent(audioClipView);
    addChildComponent(deviceChain);
    
    addAndMakeVisible(clipTabButton);
    addAndMakeVisible(deviceTabButton);
    
    clipTabButton.setClickingTogglesState(true);
    clipTabButton.setRadioGroupId(1);
    deviceTabButton.setClickingTogglesState(true);
    deviceTabButton.setRadioGroupId(1);
    
    deviceTabButton.setToggleState(true, juce::dontSendNotification);
    
    clipTabButton.onClick = [this] { showDeviceView = false; resized(); };
    deviceTabButton.onClick = [this] { showDeviceView = true; resized(); };
    
    placeholderLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    placeholderLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    placeholderLabel.setJustificationType(juce::Justification::centred);
    
    placeholderLabel.setVisible(true);
    
    engine.getTimelineProject().addListener(this);
    juce::Logger::writeToLog("DetailViewComponent constructor end");
}

DetailViewComponent::~DetailViewComponent() {
    engine.getTimelineProject().removeListener(this);
}

void DetailViewComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Top border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 0, getWidth(), 1);
}

void DetailViewComponent::resized() {
    auto area = getLocalBounds();
    
    auto tabsArea = area.removeFromBottom(25);
    deviceTabButton.setBounds(tabsArea.removeFromRight(80).reduced(2));
    clipTabButton.setBounds(tabsArea.removeFromRight(80).reduced(2));
    
    placeholderLabel.setBounds(area);
    
    if (showDeviceView) {
        pianoRoll.setVisible(false);
        pianoRollTimeline.setVisible(false);
        clipProperties.setVisible(false);
        audioClipView.setVisible(false);
        
        deviceChain.setVisible(true);
        deviceChain.setBounds(area);
    } else {
        deviceChain.setVisible(false);
        
        int propsWidth = 260;
        clipProperties.setBounds(area.removeFromLeft(propsWidth));
        
        auto timelineArea = area.removeFromTop(20);
        pianoRollTimeline.setBounds(timelineArea);
        pianoRoll.setBounds(area);
        audioClipView.setBounds(area);
        
        trackSelectionChanged(); // Restore clip view visibility logic
    }
}

void DetailViewComponent::trackAdded(int trackIndex, const TrackModel& track) {}
void DetailViewComponent::trackRemoved(int trackIndex) {}

void DetailViewComponent::trackSelectionChanged() {
    if (showDeviceView) {
        placeholderLabel.setVisible(false);
        return;
    }
    
    if (engine.getTimelineProject().getSelectedClip().valueless_by_exception() || 
        (std::holds_alternative<std::shared_ptr<AudioClip>>(engine.getTimelineProject().getSelectedClip()) && std::get<std::shared_ptr<AudioClip>>(engine.getTimelineProject().getSelectedClip()) == nullptr) ||
        (std::holds_alternative<std::shared_ptr<MidiClip>>(engine.getTimelineProject().getSelectedClip()) && std::get<std::shared_ptr<MidiClip>>(engine.getTimelineProject().getSelectedClip()) == nullptr)) {
        
        pianoRoll.setVisible(false);
        pianoRollTimeline.setVisible(false);
        clipProperties.setVisible(false);
        audioClipView.setVisible(false);
        
        placeholderLabel.setVisible(true);
        
        auto& sel = engine.getTimelineProject().getSelectedTracks();
        if (sel.getNumRanges() > 0) {
            int trackIndex = sel.getRange(0).getStart();
            if (trackIndex < engine.getTimelineProject().getNumTracks()) {
                const auto& track = engine.getTimelineProject().getTrack(trackIndex);
                
                juce::String type = track.isMidi ? "MIDI" : "Audio";
                if (sel.getTotalRange().getLength() > 1 || sel.getNumRanges() > 1) {
                    int count = 0;
                    for (int i = 0; i < sel.getNumRanges(); ++i) count += sel.getRange(i).getLength();
                    placeholderLabel.setText("Selected " + juce::String(count) + " Tracks properties...", juce::dontSendNotification);
                } else {
                    placeholderLabel.setText("Selected " + type + " Track properties... (" + track.name + ")", juce::dontSendNotification);
                }
            }
        } else {
            placeholderLabel.setText("No track selected...", juce::dontSendNotification);
        }
    }
}

void DetailViewComponent::selectedClipChanged() {
    auto clip = engine.getTimelineProject().getSelectedClip();
    bool hasValidClip = false;
    
    if (std::holds_alternative<std::shared_ptr<MidiClip>>(clip)) {
        auto midiClip = std::get<std::shared_ptr<MidiClip>>(clip);
        if (midiClip) {
            hasValidClip = true;
            placeholderLabel.setVisible(false);
            clipProperties.setVisible(true);
            clipProperties.setMidiMode(true);
            clipProperties.updateClipInfo("MIDI Clip", midiClip->getStartSample(), midiClip->getLengthSamples());
            
            pianoRollTimeline.setVisible(true);
            pianoRollTimeline.setMidiClip(midiClip);
            
            pianoRoll.setVisible(true);
            pianoRoll.setMidiClip(midiClip);
            
            audioClipView.setVisible(false);
            audioClipView.setAudioClip(nullptr);
        }
    } else if (std::holds_alternative<std::shared_ptr<AudioClip>>(clip)) {
        auto audioClip = std::get<std::shared_ptr<AudioClip>>(clip);
        if (audioClip) {
            hasValidClip = true;
            pianoRoll.setVisible(false);
            pianoRollTimeline.setVisible(false);
            clipProperties.setVisible(true);
            clipProperties.setMidiMode(false);
            clipProperties.updateClipInfo("Audio Clip", audioClip->getStartSample(), audioClip->getLengthSamples());
            
            audioClipView.setVisible(true);
            audioClipView.setAudioClip(audioClip);
            
            placeholderLabel.setVisible(false);
        }
    }
    
    if (!hasValidClip) {
        pianoRoll.setMidiClip(nullptr);
        pianoRollTimeline.setMidiClip(nullptr);
        pianoRoll.setVisible(false);
        pianoRollTimeline.setVisible(false);
        audioClipView.setVisible(false);
        audioClipView.setAudioClip(nullptr);
        clipProperties.setVisible(false);
        
        placeholderLabel.setVisible(true);
        trackSelectionChanged(); // Restores placeholder text based on track selection
    }
}

} // namespace Nimbus::MainLayout
