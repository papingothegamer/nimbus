#include "TimelineComponent.h"
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"
#include "GroupTrackHeaderComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

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
    
    // Draw Loop Brace
    if (engine.getTransport().isLooping()) {
        double loopStart = engine.getTransport().getLoopStartSamples();
        double loopEnd = engine.getTransport().getLoopEndSamples();
        if (loopEnd > loopStart) {
            double startRate = loopStart / engine.getTransport().getSampleRate();
            double endRate = loopEnd / engine.getTransport().getSampleRate();
            float startX = static_cast<float>(startRate * pixelsPerSecond - scrollOffsetX);
            float endX = static_cast<float>(endRate * pixelsPerSecond - scrollOffsetX);
            
            if (endX > 0 && startX < getWidth()) {
                g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.3f));
                g.fillRect(startX, 0.0f, endX - startX, static_cast<float>(getHeight()));
                
                g.setColour(DesignSystem::Colors::PrimaryAction);
                g.fillRect(startX, 0.0f, endX - startX, 3.0f);
                
                g.fillRect(startX, 0.0f, 2.0f, 6.0f);
                g.fillRect(endX - 2.0f, 0.0f, 2.0f, 6.0f);
            }
        }
    }
}

void SeekingBarComponent::mouseDown(const juce::MouseEvent& event) {
    float x = event.position.x;
    float y = event.position.y;
    double loopStart = engine.getTransport().getLoopStartSamples();
    double loopEnd = engine.getTransport().getLoopEndSamples();
    double sr = engine.getTransport().getSampleRate();
    if (sr <= 0) sr = 44100.0;
    
    float startX = static_cast<float>((loopStart / sr) * pixelsPerSecond - scrollOffsetX);
    float endX = static_cast<float>((loopEnd / sr) * pixelsPerSecond - scrollOffsetX);
    
    dragMode = Seeking;
    lastDragX = x;
    if (y < 12.0f) {
        if (std::abs(x - startX) < 5.0f && engine.getTransport().isLooping()) {
            dragMode = DraggingLoopStart;
        } else if (std::abs(x - endX) < 5.0f && engine.getTransport().isLooping()) {
            dragMode = DraggingLoopEnd;
        } else {
            dragMode = CreatingLoop;
            double posSeconds = (x + scrollOffsetX) / pixelsPerSecond;
            dragStartSamples = static_cast<float>(posSeconds * sr);
            engine.getTransport().setLoopRegion(dragStartSamples, dragStartSamples);
            engine.getTransport().setLooping(true);
        }
    }
    
    if (dragMode == Seeking && onSeek) {
        onSeek(x);
    }
}

void SeekingBarComponent::mouseDrag(const juce::MouseEvent& event) {
    float x = event.position.x;
    lastDragX = x;
    double sr = engine.getTransport().getSampleRate();
    if (sr <= 0) sr = 44100.0;
    double currentSamples = ((x + scrollOffsetX) / pixelsPerSecond) * sr;
    
    if (dragMode == Seeking && onSeek) {
        onSeek(x);
    } else if (dragMode == DraggingLoopStart) {
        double currentEnd = engine.getTransport().getLoopEndSamples();
        engine.getTransport().setLoopRegion(std::min(currentSamples, currentEnd - 1.0), currentEnd);
    } else if (dragMode == DraggingLoopEnd) {
        double currentStart = engine.getTransport().getLoopStartSamples();
        engine.getTransport().setLoopRegion(currentStart, std::max(currentSamples, currentStart + 1.0));
    } else if (dragMode == CreatingLoop) {
        engine.getTransport().setLoopRegion(std::min((double)dragStartSamples, currentSamples), std::max((double)dragStartSamples, currentSamples));
    }
    repaint();
}

void SeekingBarComponent::mouseUp(const juce::MouseEvent& event) {
    dragMode = None;
}

TimelineComponent::TimelineComponent(NimbusEngine& e) 
    : engine(e)
{
    addAndMakeVisible(seekingBar);
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&trackContainer, false);
    viewport.setScrollBarsShown(true, false);

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

    int headerWidth = 150;
    int lanesWidth = getWidth() - headerWidth;

    // Draw "TRACKS" header in the top-left corner
    auto tracksHeaderBounds = juce::Rectangle<int>(0, 0, headerWidth, 24);
    g.setColour(DesignSystem::Colors::PanelBackground);
    g.fillRect(tracksHeaderBounds);
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(tracksHeaderBounds.removeFromBottom(1));
    g.fillRect(headerWidth - 1, 0, 1, 24); // Right border
    g.setColour(DesignSystem::Colors::TextSecondary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f).boldened());
    g.drawText("TRACKS", juce::Rectangle<int>(0, 0, headerWidth, 24).reduced(8, 0), juce::Justification::centredLeft);

    // Draw Arrangement View Grid
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
    
    // Grid spacing based on zoom
    double pixelsPerBeat = pixelsPerSecond * (60.0 / engine.getTransport().getTempo());
    
    double startX = -std::fmod(scrollOffsetX, pixelsPerBeat);
    for (double i = startX; i < lanesWidth; i += pixelsPerBeat) {
        int x = static_cast<int>(std::round(i)) + headerWidth;
        if (x >= headerWidth) {
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
    int playheadX = static_cast<int>(positionSeconds * pixelsPerSecond) - scrollOffsetX + headerWidth;
    
    if (playheadX >= headerWidth && playheadX < headerWidth + lanesWidth) {
        // Draw inside a clipped region
        g.saveState();
        g.reduceClipRegion(headerWidth, 0, lanesWidth, getHeight());

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
        auto* groupHeader = new Timeline::GroupTrackHeaderComponent(engine, trackIndex);
        trackHeaders.insert(trackIndex, groupHeader);
        groupHeader->setTrackIndex(trackIndex);
    } else {
        auto* standardHeader = new Timeline::TrackHeaderComponent(engine, trackIndex);
        trackHeaders.insert(trackIndex, standardHeader);
        standardHeader->setTrackIndex(trackIndex);
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
    
    trackContainer.addAndMakeVisible(trackHeaders[trackIndex]);
    trackContainer.addAndMakeVisible(trackLanes[trackIndex]);
    resized();
}

void TimelineComponent::trackRemoved(int trackIndex)
{
    // 1. Remove the UI components from the arrays
    trackHeaders.remove(trackIndex);
    trackLanes.remove(trackIndex);

    // 2. Shift the index of all subsequent tracks down by 1
    for (int i = trackIndex; i < trackHeaders.size(); ++i)
    {
        // Safely cast the generic juce::Component back to your specific track headers
        if (auto* header = dynamic_cast<Timeline::TrackHeaderComponent*>(trackHeaders[i])) {
            header->setTrackIndex(i);
        }
        else if (auto* groupHeader = dynamic_cast<Timeline::GroupTrackHeaderComponent*>(trackHeaders[i])) {
            groupHeader->setTrackIndex(i);
        }
        
        trackLanes[i]->setTrackIndex(i);
    }

    // 3. Force a layout recalculation
    resized();
    repaint();
}

void TimelineComponent::tracksGrouped() {
    trackHeaders.clear();
    trackLanes.clear();
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        const auto& track = engine.getTimelineProject().getTrack(i);
        auto clips = engine.getTimelineProject().getClipsOnTrack(i);
        juce::Logger::writeToLog("Track " + juce::String(i) + " (" + track.name + ") has " + juce::String(clips.size()) + " clips.");
        
        if (track.isGroup) {
            auto* groupHeader = new Timeline::GroupTrackHeaderComponent(engine, i);
            trackHeaders.add(groupHeader);
            groupHeader->setTrackIndex(i);
        } else {
            auto* standardHeader = new Timeline::TrackHeaderComponent(engine, i);
            trackHeaders.add(standardHeader);
            standardHeader->setTrackIndex(i);
        }
        auto* trackLane = new Timeline::TrackLaneComponent(engine, *this, i);
        trackLanes.add(trackLane);
        trackLane->setTrackIndex(i);
        
        trackContainer.addAndMakeVisible(trackHeaders.getLast());
        trackContainer.addAndMakeVisible(trackLanes.getLast());
    }
    resized();
}

void TimelineComponent::trackFoldStateChanged(int trackIndex, bool isFolded) {
    resized();
}

void TimelineComponent::resized() {
    auto bounds = getLocalBounds();
    
    int headerWidth = 150;
    int standardTrackHeight = 65; // Shrunk to Audacity proportions

    // Seeking Bar at the top
    seekingBar.setBounds(bounds.removeFromTop(24).withTrimmedLeft(headerWidth));
    viewport.setBounds(bounds);
    
    int currentY = 0;
    int containerWidth = bounds.getWidth() - viewport.getScrollBarThickness();
    
    for (int i = 0; i < trackHeaders.size(); ++i) {
        int currentTrackHeight = standardTrackHeight; 
        bool isHidden = false;
        bool isChild = false;
        
        if (i < engine.getTimelineProject().getNumTracks()) {
            const auto& track = engine.getTimelineProject().getTrack(i);
            
            // Check if track is a child
            if (!track.parentGroupId.isNull()) {
                isChild = true;
                // Hide children if parent group is folded
                for (int j = 0; j < engine.getTimelineProject().getNumTracks(); ++j) {
                    const auto& parentTrack = engine.getTimelineProject().getTrack(j);
                    if (parentTrack.id == track.parentGroupId) {
                        if (parentTrack.isFolded) isHidden = true;
                        break;
                    }
                }
            }
            
            // Compact heights for folded tracks and groups
            if (track.isGroup) {
                currentTrackHeight = track.isFolded ? 35 : standardTrackHeight; 
            } else if (track.isFolded) {
                currentTrackHeight = 35;
            }
        }
        
        if (isHidden) {
            trackHeaders[i]->setVisible(false);
            trackLanes[i]->setVisible(false);
        } else {
            trackHeaders[i]->setVisible(true);
            trackLanes[i]->setVisible(true);
            
            trackHeaders[i]->setBounds(0, currentY, headerWidth, currentTrackHeight - 1);
            trackLanes[i]->setBounds(headerWidth, currentY, containerWidth - headerWidth, currentTrackHeight - 1);
            
            // Safety net: ensure clips are always in sync with data model when resized
            auto expectedClips = engine.getTimelineProject().getClipsOnTrack(i);
            if (trackLanes[i]->getNumClipComponents() != expectedClips.size()) {
                trackLanes[i]->updateClips();
            }
            
            currentY += currentTrackHeight;
        }
    }
    
    trackContainer.setBounds(0, 0, containerWidth, currentY);
}
void TimelineComponent::timerCallback() {
    bool shouldRepaint = false;
    
    if (engine.getTransport().isPlaying()) {
        if (engine.isFollowPlayheadEnabled()) {
            double positionSamples = engine.getTransport().getCurrentPosition();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0.0) sampleRate = 48000.0;
            
            double positionSeconds = positionSamples / sampleRate;
            int playheadAbsoluteX = static_cast<int>(positionSeconds * pixelsPerSecond);
            
            int headerWidth = 150;
            int lanesWidth = getWidth() - headerWidth;
            
            int targetPlayheadScreenX = lanesWidth / 3;
            
            if (playheadAbsoluteX > targetPlayheadScreenX) {
                scrollOffsetX = playheadAbsoluteX - targetPlayheadScreenX;
            } else {
                scrollOffsetX = 0.0;
            }
            
            for (auto* lane : trackLanes) {
                lane->resized();
            }
        }
        shouldRepaint = true;
    } else if (seekingBar.dragMode != SeekingBarComponent::DragMode::None) {
        // Auto-scroll when dragging playhead off-screen
        float x = seekingBar.lastDragX;
        bool scrolled = false;
        if (x < 0) {
            scrollOffsetX += x * 0.2f; // speed multiplier
            scrollOffsetX = juce::jmax(0.0, scrollOffsetX);
            seekingBar.onSeek(0);
            scrolled = true;
        } else if (x > seekingBar.getWidth()) {
            scrollOffsetX += (x - seekingBar.getWidth()) * 0.2f;
            seekingBar.onSeek(seekingBar.getWidth());
            scrolled = true;
        }
        
        if (scrolled) {
            for (auto* lane : trackLanes) {
                lane->resized();
            }
        }
        shouldRepaint = true;
    }
    
    for (auto* header : trackHeaders) {
        if (auto* trackHeader = dynamic_cast<Timeline::TrackHeaderComponent*>(header)) {
            trackHeader->updateMeters();
        }
    }
    
    if (shouldRepaint) {
        repaint();
    }
}

void TimelineComponent::zoom(double factor) {
    double positionSamples = engine.getTransport().getCurrentPosition();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double positionSeconds = positionSamples / sampleRate;
    
    // playhead screen position relative to timeline start
    int headerWidth = 150;
    double playheadAbsoluteXBefore = positionSeconds * pixelsPerSecond;
    double playheadScreenX = playheadAbsoluteXBefore - scrollOffsetX;
    
    pixelsPerSecond = juce::jlimit(10.0, 200.0, pixelsPerSecond * factor);
    
    if (onZoomLevelChanged) onZoomLevelChanged(static_cast<int>(pixelsPerSecond));
    
    double playheadAbsoluteXAfter = positionSeconds * pixelsPerSecond;
    scrollOffsetX = juce::jmax(0.0, playheadAbsoluteXAfter - playheadScreenX);
    
    for (auto* lane : trackLanes) {
        lane->resized();
    }
    repaint();
}

void TimelineComponent::setZoomLevel(int percentage) {
    double playheadScreenX = 0;
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    double positionSeconds = engine.getTransport().getCurrentPosition() / sampleRate;
    if (positionSeconds > 0) {
        playheadScreenX = (positionSeconds * pixelsPerSecond) - scrollOffsetX;
    }
    
    pixelsPerSecond = juce::jlimit(10.0, 200.0, static_cast<double>(percentage));
    
    if (positionSeconds > 0) {
        double playheadAbsoluteXAfter = positionSeconds * pixelsPerSecond;
        scrollOffsetX = juce::jmax(0.0, playheadAbsoluteXAfter - playheadScreenX);
    }
    
    if (onZoomLevelChanged) onZoomLevelChanged(static_cast<int>(pixelsPerSecond));
    
    resized();
    repaint();
}

void TimelineComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
        // Zoom
        double zoomDelta = wheel.deltaY > 0 ? 1.1 : 0.9;
        if (wheel.deltaY == 0) zoomDelta = 1.0;
        zoom(zoomDelta);
    } else {
        if (std::abs(wheel.deltaX) > 0.0f) {
            scrollOffsetX += wheel.deltaX * 500.0;
            scrollOffsetX = juce::jmax(0.0, scrollOffsetX);
            for (auto* lane : trackLanes) {
                lane->resized();
            }
            repaint();
        }
    }
}

bool TimelineComponent::isInterestedInFileDrag(const juce::StringArray& files) {
    for (auto& file : files) {
        if (file.endsWithIgnoreCase(".wav") || file.endsWithIgnoreCase(".aiff") ||
            file.endsWithIgnoreCase(".flac") || file.endsWithIgnoreCase(".mp3") ||
            file.endsWithIgnoreCase(".mid") || file.endsWithIgnoreCase(".midi")) {
            return true;
        }
    }
    return false;
}

void TimelineComponent::filesDropped(const juce::StringArray& files, int x, int y) {
    int headerWidth = 150;
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double droppedTimeSeconds = static_cast<double>(juce::jmax(0, x - headerWidth) + scrollOffsetX) / pixelsPerSecond;
    double droppedSample = droppedTimeSeconds * sampleRate;
    
    int yOffset = viewport.getViewPositionY() + y - 24; // 24 is seeking bar height
    int currentY = 0;
    int targetTrackIndex = -1;
    
    for (int i = 0; i < trackHeaders.size(); ++i) {
        if (!trackHeaders[i]->isVisible()) continue;
        int height = trackHeaders[i]->getHeight();
        if (yOffset >= currentY && yOffset < currentY + height) {
            targetTrackIndex = i;
            break;
        }
        currentY += height;
    }
    
    for (auto& filePath : files) {
        juce::File file(filePath);
        if (!file.existsAsFile()) continue;
        
        bool isAudio = file.hasFileExtension(".wav") || file.hasFileExtension(".aiff") ||
                       file.hasFileExtension(".flac") || file.hasFileExtension(".mp3");
        bool isMidi = file.hasFileExtension(".mid") || file.hasFileExtension(".midi");
        
        if (!isAudio && !isMidi) continue;
        
        if (targetTrackIndex == -1) {
            TrackModel newTrack;
            newTrack.name = file.getFileNameWithoutExtension();
            targetTrackIndex = engine.getTimelineProject().getNumTracks();
            engine.getTimelineProject().addTrack(newTrack);
        } else {
            engine.getTimelineProject().setTrackName(targetTrackIndex, file.getFileNameWithoutExtension());
        }
        
        if (isAudio) {
            juce::MessageBoxOptions options = juce::MessageBoxOptions()
                   .withIconType(juce::MessageBoxIconType::QuestionIcon)
                   .withTitle("Match DAW Tempo?")
                   .withMessage("Do you want to match this clip to the project's current BPM (" + juce::String(engine.getTransport().getTempo(), 1) + " BPM)?")
                   .withButton("Match Tempo (Auto)")
                   .withButton("Specify Tempo...")
                   .withButton("Keep Original");
                   
            juce::AlertWindow::showAsync(options, [this, file, droppedSample, sampleRate, targetTrackIndex](int result) {
                auto clip = std::make_shared<AudioClip>(file, static_cast<int>(droppedSample), static_cast<int>(sampleRate * 4.0));
                clip->name = file.getFileNameWithoutExtension();
                
                if (result == 1 || result == 2) { 
                    clip->preservePitch = true;
                    clip->matchDawTempo = true;
                    clip->originalBpm = 120.0;
                } else {
                    clip->preservePitch = false;
                    clip->matchDawTempo = false;
                }
                
                engine.getTimelineProject().addClipToTrack(targetTrackIndex, clip);
                
                if (result == 2) {
                    // Specify tempo: auto-select the clip to open the clip properties
                    engine.getTimelineProject().setSelectedClip(clip);
                }
            });
        } else if (isMidi) {
            auto clip = std::make_shared<MidiClip>(static_cast<int>(droppedSample), static_cast<int>(sampleRate * 4.0));
            clip->name = file.getFileNameWithoutExtension();
            engine.getTimelineProject().addClipToTrack(targetTrackIndex, clip);
        }
        
        droppedSample += sampleRate * 4.0;
    }
}

void TimelineComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) return; // Global context menu could go here
    
    int headerWidth = 150;
    if (event.position.x < headerWidth) return;
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double clickSeconds = (event.position.x - headerWidth + scrollOffsetX) / pixelsPerSecond;
    double startSamples = clickSeconds * sampleRate;
    
    auto& project = engine.getTimelineProject();
    project.setSelectedClip(AnyClipPtr{});
    if (!event.mods.isShiftDown()) project.clearTimeSelection();
    
    project.setTimeSelection(startSamples, startSamples);
    selectionStartTrackIndex = std::max(0, project.getNumTracks() - 1);
    
    // Check if we actually clicked inside a lane by finding total track height
    int yOffset = viewport.getViewPositionY() + event.position.y - 24;
    int currentY = 0;
    for (int i = 0; i < trackHeaders.size(); ++i) {
        if (trackHeaders[i]->isVisible()) {
            if (yOffset >= currentY && yOffset < currentY + trackHeaders[i]->getHeight()) {
                selectionStartTrackIndex = i;
                break;
            }
            currentY += trackHeaders[i]->getHeight();
        }
    }
    
    if (selectionStartTrackIndex >= 0 && selectionStartTrackIndex < project.getNumTracks()) {
        project.addTimeSelectedTrack(selectionStartTrackIndex);
    }
    isDraggingSelection = true;
}

void TimelineComponent::mouseDrag(const juce::MouseEvent& event) {
    if (isDraggingSelection) {
        int headerWidth = 150;
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        double currentSeconds = (event.position.x - headerWidth + scrollOffsetX) / pixelsPerSecond;
        double endSamples = currentSeconds * sampleRate;
        
        auto& project = engine.getTimelineProject();
        project.setTimeSelection(project.getTimeSelectionStart(), endSamples);
        
        int yOffset = viewport.getViewPositionY() + event.position.y - 24;
        int currentY = 0;
        int draggedTrackIndex = std::max(0, project.getNumTracks() - 1);
        
        for (int i = 0; i < trackHeaders.size(); ++i) {
            if (trackHeaders[i]->isVisible()) {
                if (yOffset >= currentY && yOffset < currentY + trackHeaders[i]->getHeight()) {
                    draggedTrackIndex = i;
                    break;
                }
                currentY += trackHeaders[i]->getHeight();
            }
        }
        
        if (draggedTrackIndex >= 0 && selectionStartTrackIndex >= 0) {
            int startTrack = std::min(selectionStartTrackIndex, draggedTrackIndex);
            int endTrack = std::max(selectionStartTrackIndex, draggedTrackIndex);
            
            juce::SparseSet<int> selectedTracks;
            selectedTracks.addRange(juce::Range<int>(startTrack, endTrack + 1));
            project.setTimeSelectedTracks(selectedTracks);
        }
    }
}

void TimelineComponent::mouseUp(const juce::MouseEvent& event) {
    isDraggingSelection = false;
}

} // namespace Nimbus
