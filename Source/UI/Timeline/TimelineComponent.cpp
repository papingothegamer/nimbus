#include "TimelineComponent.h"
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"
#include "GroupTrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus {

SeekingBarComponent::SeekingBarComponent(NimbusEngine& e, double& pps, double& scrollX) 
    : engine(e), pixelsPerSecond(pps), scrollOffsetX(scrollX) {}

void SeekingBarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    
    g.setColour(DesignSystem::Colors::TextSecondary);
    g.setFont(juce::Font(10.0f));

    double tempo = engine.getTransport().getTempo();
    if (tempo <= 0.0) tempo = 120.0;
    
    double secondsPerBeat = 60.0 / tempo;
    double pixelsPerBeat = pixelsPerSecond * secondsPerBeat;
    
    // We assume 4/4 time signature for now
    double pixelsPerBar = pixelsPerBeat * 4.0;
    
    // Determine grid resolution based on zoom (pixelsPerBar)
    double resolution = 4.0; // Draw every beat
    if (pixelsPerBar < 50.0) {
        resolution = 1.0; // Draw every bar only
    } else if (pixelsPerBar > 200.0) {
        resolution = 16.0; // Draw 16th notes
    }

    double startPixel = -std::fmod(scrollOffsetX, pixelsPerBar);
    double firstBarNumber = std::floor(scrollOffsetX / pixelsPerBar);
    
    int barCount = 0;
    for (double i = startPixel; i < getWidth(); i += pixelsPerBeat, ++barCount) {
        int x = static_cast<int>(std::round(i));
        if (x < 0) continue;
        
        double currentBeatRaw = firstBarNumber * 4.0 + barCount;
        int barNum = static_cast<int>(std::floor(currentBeatRaw / 4.0)) + 1;
        int beatNum = static_cast<int>(std::fmod(currentBeatRaw, 4.0)) + 1;
        
        if (beatNum == 1) { // It's a bar line
            g.drawLine(x, getHeight() - 10, x, getHeight(), 1.0f);
            
            int barInterval = 1;
            if (pixelsPerBar < 10.0) barInterval = 16;
            else if (pixelsPerBar < 20.0) barInterval = 8;
            else if (pixelsPerBar < 50.0) barInterval = 4;
            else if (pixelsPerBar < 80.0) barInterval = 2;

            if (barNum == 1 || barNum % barInterval == 0) {
                // If zoomed in far enough, display full format (e.g., 1.1.0 or just 1)
                juce::String text = (pixelsPerBar > 200.0) ? juce::String(barNum) + ".1.0" : juce::String(barNum);
                g.drawText(text, x + 2, 2, 50, 10, juce::Justification::topLeft, false);
            }
        } else if (resolution >= 4.0) { // It's a beat line
            g.drawLine(x, getHeight() - 5, x, getHeight(), 1.0f);
            if (pixelsPerBar > 100.0) { // Only draw beat text if zoomed in enough
                juce::String text = (pixelsPerBar > 200.0) ? juce::String(barNum) + "." + juce::String(beatNum) + ".0" : juce::String(barNum) + "." + juce::String(beatNum);
                g.drawText(text, x + 2, 8, 40, 10, juce::Justification::topLeft, false);
            }
        }
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
    int headerWidth = 150;
    int lanesWidth = getWidth() - headerWidth;
    
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
    
    // Grid spacing based on zoom
    double pixelsPerBeat = pixelsPerSecond * (60.0 / engine.getTransport().getTempo());
    
    double startX = -std::fmod(scrollOffsetX, pixelsPerBeat);
    for (double i = startX; i < lanesWidth; i += pixelsPerBeat) {
        int x = static_cast<int>(std::round(i));
        if (x >= 0) {
            g.drawLine(x, 24, x, getHeight(), 1.0f);
        }
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
    if (track.isGroup) {
        trackHeaders.insert(trackIndex, new Timeline::GroupTrackHeaderComponent(engine, trackIndex));
    } else {
        trackHeaders.insert(trackIndex, new Timeline::TrackHeaderComponent(engine, trackIndex));
    }
    trackLanes.insert(trackIndex, new Timeline::TrackLaneComponent(engine, *this, trackIndex));
    
    for (int i = trackIndex + 1; i < trackHeaders.size(); ++i) {
        if (auto* groupHeader = dynamic_cast<Timeline::GroupTrackHeaderComponent*>(trackHeaders[i])) {
            groupHeader->setTrackIndex(i);
        } else if (auto* standardHeader = dynamic_cast<Timeline::TrackHeaderComponent*>(trackHeaders[i])) {
            standardHeader->setTrackIndex(i);
        }
        trackLanes[i]->setTrackIndex(i);
    }
    
    addAndMakeVisible(trackHeaders[trackIndex]);
    addAndMakeVisible(trackLanes[trackIndex]);
    resized();
}

void TimelineComponent::trackRemoved(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < trackLanes.size()) {
        trackLanes.remove(trackIndex);
        trackHeaders.remove(trackIndex);
        
        for (int i = trackIndex; i < trackHeaders.size(); ++i) {
            if (auto* groupHeader = dynamic_cast<Timeline::GroupTrackHeaderComponent*>(trackHeaders[i])) {
                groupHeader->setTrackIndex(i);
            } else if (auto* standardHeader = dynamic_cast<Timeline::TrackHeaderComponent*>(trackHeaders[i])) {
                standardHeader->setTrackIndex(i);
            }
            trackLanes[i]->setTrackIndex(i);
        }
        resized();
    }
}

void TimelineComponent::tracksGrouped() {
    trackHeaders.clear();
    trackLanes.clear();
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        const auto& track = engine.getTimelineProject().getTrack(i);
        if (track.isGroup) {
            trackHeaders.add(new Timeline::GroupTrackHeaderComponent(engine, i));
        } else {
            trackHeaders.add(new Timeline::TrackHeaderComponent(engine, i));
        }
        trackLanes.add(new Timeline::TrackLaneComponent(engine, *this, i));
        addAndMakeVisible(trackHeaders.getLast());
        addAndMakeVisible(trackLanes.getLast());
    }
    resized();
}

void TimelineComponent::trackFoldStateChanged(int trackIndex, bool isFolded) {
    resized();
}

void TimelineComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Seeking Bar at the top
    seekingBar.setBounds(bounds.removeFromTop(24).withTrimmedRight(150)); // Avoid header area

    int headerWidth = 150;
    int trackHeight = 110;
    
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
        
        int headerWidth = 150;
        int lanesWidth = getWidth() - headerWidth;
        
        int playheadScreenX = playheadAbsoluteX - static_cast<int>(scrollOffsetX);
        
        // Continuous scroll: keep playhead at 1/3 of the screen width
        int targetPlayheadScreenX = lanesWidth / 3;
        
        if (playheadAbsoluteX > targetPlayheadScreenX) {
            scrollOffsetX = playheadAbsoluteX - targetPlayheadScreenX;
        } else {
            scrollOffsetX = 0.0;
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
    }
}



} // namespace Nimbus
