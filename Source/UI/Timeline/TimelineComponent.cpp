#include "TimelineComponent.h"
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus {

SeekingBarComponent::SeekingBarComponent() {}

void SeekingBarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    
    // Draw dummy tick marks
    g.setColour(DesignSystem::Colors::TextSecondary);
    for (int i = 0; i < getWidth(); i += 50) {
        g.drawLine(i, getHeight() - 5, i, getHeight(), 1.0f);
    }
}

void SeekingBarComponent::mouseDown(const juce::MouseEvent& event) {
    // In a real implementation, we'd update Transport based on x position
}

void SeekingBarComponent::mouseDrag(const juce::MouseEvent& event) {
    // Update playhead while dragging
}

TimelineComponent::TimelineComponent(NimbusEngine& e) 
    : engine(e)
{
    addAndMakeVisible(seekingBar);

    // Register as listener
    engine.getTimelineProject().addListener(this);

    // Load existing tracks
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        trackAdded(i, engine.getTimelineProject().getTrack(i));
    }

    startTimerHz(60); // 60fps playhead updates
}

TimelineComponent::~TimelineComponent() {
    engine.getTimelineProject().removeListener(this);
}

void TimelineComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);

    // Draw Arrangement View Grid
    int headerWidth = 150;
    int lanesWidth = getWidth() - headerWidth;
    
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
    for (int i = 0; i < lanesWidth; i += 50) {
        g.drawLine(i, 24, i, getHeight(), 1.0f);
    }
}

void TimelineComponent::paintOverChildren(juce::Graphics& g) {
    int headerWidth = 150;
    int lanesWidth = getWidth() - headerWidth;

    // Draw Playhead
    double positionSamples = engine.getTransport().getCurrentPosition();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double positionSeconds = positionSamples / sampleRate;
    int playheadX = headerWidth + static_cast<int>(positionSeconds * 50.0);
    
    if (playheadX >= headerWidth && playheadX < getWidth()) {
        g.setColour(DesignSystem::Colors::PrimaryAction);
        g.drawLine(playheadX, 0, playheadX, getHeight(), 2.0f);
        
        // Playhead triangle
        juce::Path p;
        p.addTriangle(playheadX - 5, 0, playheadX + 5, 0, playheadX, 10);
        g.fillPath(p);
    }
}

void TimelineComponent::trackAdded(int trackIndex, const TrackModel& track) {
    auto* lane = new Timeline::TrackLaneComponent(engine, trackIndex);
    trackLanes.add(lane);
    addAndMakeVisible(lane);

    auto* header = new Timeline::TrackHeaderComponent(engine, trackIndex);
    trackHeaders.add(header);
    addAndMakeVisible(header);

    resized();
}

void TimelineComponent::trackRemoved(int trackIndex) {
    // Not implemented for Phase 4 MVP
}

void TimelineComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Seeking Bar at the top
    seekingBar.setBounds(bounds.removeFromTop(24).withTrimmedRight(150)); // Avoid header area

    int headerWidth = 150;
    int trackHeight = 80;
    
    // Left side: Lanes
    auto lanesArea = bounds.removeFromLeft(bounds.getWidth() - headerWidth);
    
    // Right side: Headers
    auto headersArea = bounds;
    
    for (int i = 0; i < trackHeaders.size(); ++i) {
        trackLanes[i]->setBounds(lanesArea.removeFromTop(trackHeight).reduced(0, 1));
        trackHeaders[i]->setBounds(headersArea.removeFromTop(trackHeight).reduced(0, 1));
    }
}

void TimelineComponent::timerCallback() {
    repaint();
}

} // namespace Nimbus
