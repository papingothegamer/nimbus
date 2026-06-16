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
    if (onSeek) {
        onSeek(event.position.x);
    }
}

void SeekingBarComponent::mouseDrag(const juce::MouseEvent& event) {
    if (onSeek) {
        onSeek(event.position.x);
    }
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

    seekingBar.onSeek = [this](float xPos) {
        float clampedX = juce::jmax(0.0f, xPos + (float)scrollOffsetX);
        double newPositionSeconds = static_cast<double>(clampedX) / pixelsPerSecond;
        double newPositionSamples = newPositionSeconds * engine.getTransport().getSampleRate();
        engine.getTransport().setPosition(newPositionSamples);
    };

    startTimerHz(60); // 60fps playhead updates
}

TimelineComponent::~TimelineComponent() {
    engine.getTimelineProject().removeListener(this);
}

void TimelineComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);

    // Draw Arrangement View Grid
    int headerWidth = 280;
    int lanesWidth = getWidth() - headerWidth;
    
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
    
    // Grid spacing based on zoom
    double pixelsPerBeat = pixelsPerSecond * (60.0 / engine.getTransport().getTempo());
    int gridSpacing = juce::jmax(10, static_cast<int>(pixelsPerBeat));
    
    int startX = -(static_cast<int>(scrollOffsetX) % gridSpacing);
    for (int i = startX; i < lanesWidth; i += gridSpacing) {
        g.drawLine(i, 24, i, getHeight(), 1.0f);
    }
}

void TimelineComponent::paintOverChildren(juce::Graphics& g) {
    int headerWidth = 280;
    int lanesWidth = getWidth() - headerWidth;

    // Draw Playhead
    double positionSamples = engine.getTransport().getCurrentPosition();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double positionSeconds = positionSamples / sampleRate;
    int playheadX = static_cast<int>(positionSeconds * pixelsPerSecond) - scrollOffsetX;
    
    if (playheadX >= 0 && playheadX < lanesWidth) {
        // Draw inside a clipped region
        g.saveState();
        g.reduceClipRegion(0, 0, lanesWidth, getHeight());

        g.setColour(DesignSystem::Colors::PrimaryAction);
        g.drawLine(playheadX, 0, playheadX, getHeight(), 2.0f);
        
        // Playhead triangle
        juce::Path p;
        p.addTriangle(playheadX - 5, 0, playheadX + 5, 0, playheadX, 10);
        g.fillPath(p);
        
        g.restoreState();
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
    if (trackIndex >= 0 && trackIndex < trackLanes.size()) {
        auto* lane = trackLanes.removeAndReturn(trackIndex);
        delete lane;
        auto* header = trackHeaders.removeAndReturn(trackIndex);
        delete header;
        
        for (int i = trackIndex; i < trackHeaders.size(); ++i) {
            trackHeaders[i]->setTrackIndex(i);
            trackLanes[i]->setTrackIndex(i);
        }
        resized();
    }
}

void TimelineComponent::trackFoldStateChanged(int trackIndex, bool isFolded) {
    resized();
}

void TimelineComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Seeking Bar at the top
    seekingBar.setBounds(bounds.removeFromTop(24).withTrimmedRight(280)); // Avoid header area

    int headerWidth = 280;
    int trackHeight = 80;
    
    // Left side: Lanes
    auto lanesArea = bounds.removeFromLeft(bounds.getWidth() - headerWidth);
    
    // Right side: Headers
    auto headersArea = bounds;
    
    for (int i = 0; i < trackHeaders.size(); ++i) {
        int currentTrackHeight = trackHeight; // default
        bool isHidden = false;
        
        if (i < engine.getTimelineProject().getNumTracks()) {
            const auto& track = engine.getTimelineProject().getTrack(i);
            
            // Check if parent group is folded
            if (!track.parentGroupId.isNull()) {
                // Find parent group
                for (int j = 0; j < engine.getTimelineProject().getNumTracks(); ++j) {
                    const auto& parentTrack = engine.getTimelineProject().getTrack(j);
                    if (parentTrack.id == track.parentGroupId) {
                        if (parentTrack.isFolded) {
                            isHidden = true;
                        }
                        break;
                    }
                }
            }
            
            if (track.isGroup && track.isFolded) {
                currentTrackHeight = 32; // Folded compact height
            } else if (track.isFolded) {
                currentTrackHeight = 32;
            } else if (track.isGroup) {
                currentTrackHeight = 32; // Group tracks are generally small if we want, or default
            }
        }
        
        if (isHidden) {
            currentTrackHeight = 0;
            trackHeaders[i]->setVisible(false);
            trackLanes[i]->setVisible(false);
        } else {
            trackHeaders[i]->setVisible(true);
            trackLanes[i]->setVisible(true);
            trackLanes[i]->setBounds(lanesArea.removeFromTop(currentTrackHeight).reduced(0, 1));
            trackHeaders[i]->setBounds(headersArea.removeFromTop(currentTrackHeight).reduced(0, 1));
        }
    }
}

void TimelineComponent::timerCallback() {
    if (engine.getTransport().isPlaying() && engine.isFollowPlayheadEnabled()) {
        double positionSamples = engine.getTransport().getCurrentPosition();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        
        double positionSeconds = positionSamples / sampleRate;
        int playheadAbsoluteX = static_cast<int>(positionSeconds * pixelsPerSecond);
        
        int headerWidth = 280;
        int lanesWidth = getWidth() - headerWidth;
        
        int playheadScreenX = playheadAbsoluteX - static_cast<int>(scrollOffsetX);
        if (playheadScreenX > lanesWidth * 0.75) {
            scrollOffsetX = playheadAbsoluteX - (lanesWidth * 0.75);
        }
    }
    repaint();
}

void TimelineComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
        // Zoom
        double zoomDelta = wheel.deltaY > 0 ? 1.1 : 0.9;
        pixelsPerSecond = juce::jlimit(5.0, 500.0, pixelsPerSecond * zoomDelta);
        repaint();
    } else {
        // Scroll logic (to be implemented)
    }
}

} // namespace Nimbus
