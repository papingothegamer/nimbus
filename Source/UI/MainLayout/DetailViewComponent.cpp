#include "DetailViewComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

DetailViewComponent::DetailViewComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(placeholderLabel);
    placeholderLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    placeholderLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    placeholderLabel.setJustificationType(juce::Justification::centred);
    engine.getTimelineProject().addListener(this);
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
    placeholderLabel.setBounds(getLocalBounds());
}

void DetailViewComponent::trackAdded(int trackIndex, const TrackModel& track) {}
void DetailViewComponent::trackRemoved(int trackIndex) {}

void DetailViewComponent::trackSelectionChanged() {
    auto& sel = engine.getTimelineProject().getSelectedTracks();
    if (sel.getNumRanges() > 0) {
        int trackIndex = sel.getRange(0).getStart();
        if (trackIndex < engine.getTimelineProject().getNumTracks()) {
            const auto& track = engine.getTimelineProject().getTrack(trackIndex);
            
            juce::String type = track.isMidi ? "MIDI" : "Audio";
            if (sel.getTotalRange().getLength() > 1 || sel.getNumRanges() > 1) {
                // We have multiple tracks selected
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

} // namespace Nimbus::MainLayout
