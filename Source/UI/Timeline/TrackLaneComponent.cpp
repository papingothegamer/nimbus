#include "TimelineComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::Timeline {

TrackLaneComponent::TrackLaneComponent(NimbusEngine& e, Nimbus::TimelineComponent& t, int tIndex) 
    : engine(e), timeline(t), trackIndex(tIndex) {
    engine.getTimelineProject().addListener(this);
    updateClips();
}

TrackLaneComponent::~TrackLaneComponent() {
    engine.getTimelineProject().removeListener(this);
}

void TrackLaneComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);

    // The parent component is painted behind this lane, so the arrangement
    // grid must be rendered here to remain visible above the lane background.
    const auto tempo = juce::jmax(1.0, engine.getTransport().getTempo());
    const auto pixelsPerBeat = timeline.getPixelsPerSecond() * (60.0 / tempo);
    if (pixelsPerBeat >= 1.0) {
        const auto scrollX = timeline.getScrollOffsetX();
        const auto firstBeatX = -std::fmod(scrollX, pixelsPerBeat);
        for (double x = firstBeatX; x < getWidth(); x += pixelsPerBeat) {
            if (x < 0.0) continue;
            const auto beat = static_cast<int>(std::floor((x + scrollX) / pixelsPerBeat));
            const auto isBar = (beat % 4) == 0;
            g.setColour(DesignSystem::Colors::Divider.withAlpha(isBar ? 0.42f : 0.20f));
            g.drawVerticalLine(juce::roundToInt(x), 0.0f, static_cast<float>(getHeight()));
        }
    }
    // If selected, highlight
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRect(getLocalBounds());
    }
    
    // Recording indicator
    if (engine.getTransport().isRecording() && engine.getTimelineProject().isTrackArmed(trackIndex)) {
        g.setColour(juce::Colours::red.withAlpha(0.15f));
        g.fillRect(getLocalBounds());
    }
    
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TrackLaneComponent::paintOverChildren(juce::Graphics& g) {
    // Selection highlight
    auto& project = engine.getTimelineProject();
    double selectionStartSamples = project.getTimeSelectionStart();
    double selectionEndSamples = project.getTimeSelectionEnd();
    
    if (selectionStartSamples >= 0 && selectionEndSamples >= 0 && selectionStartSamples != selectionEndSamples && project.getTimeSelectedTracks().contains(trackIndex)) {
        double minSamples = std::min(selectionStartSamples, selectionEndSamples);
        double maxSamples = std::max(selectionStartSamples, selectionEndSamples);
        
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        double pixelsPerSecond = timeline.getPixelsPerSecond();
        double scrollX = timeline.getScrollOffsetX();
        
        int x1 = static_cast<int>((minSamples / sampleRate) * pixelsPerSecond - scrollX);
        int x2 = static_cast<int>((maxSamples / sampleRate) * pixelsPerSecond - scrollX);
        
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        g.fillRect(x1, 0, x2 - x1, getHeight());
        
        // Draw selection boundaries
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.8f));
        g.fillRect(x1, 0, 1, getHeight());
        g.fillRect(x2 - 1, 0, 1, getHeight());
    }
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}


void TrackLaneComponent::mouseDoubleClick(const juce::MouseEvent& event) {
    if (trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        if (track.isMidi) {
            // Adaptive grid 1/16th note default
            // Snap double-click position to nearest 1/16th beat or beat
            // For now, let's create a 1-bar clip (4 beats in 4/4)
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0.0) sampleRate = 48000.0;
            double clipLengthSamples = (secondsPerBeat * 4.0) * sampleRate;

            double pixelsPerSecond = timeline.getPixelsPerSecond();
            double clickSeconds = (event.position.x + timeline.getScrollOffsetX()) / pixelsPerSecond;
            double snappedSeconds = std::round(clickSeconds / secondsPerBeat) * secondsPerBeat;
            double startSamples = snappedSeconds * sampleRate;

            auto midiClip = std::make_shared<MidiClip>(startSamples, clipLengthSamples);
            // Empty clip, no notes!
            
            engine.getTimelineProject().addClipToTrack(trackIndex, midiClip);
        }
    }
}

void TrackLaneComponent::trackClipsChanged(int changedTrackIndex) {
    if (changedTrackIndex == trackIndex) {
        updateClips();
    }
}

void TrackLaneComponent::timeSelectionChanged() {
    repaint();
}

void TrackLaneComponent::trackNameChanged(int changedTrackIndex, const juce::String& newName) {
    if (changedTrackIndex == trackIndex) {
        auto clips = engine.getTimelineProject().getClipsOnTrack(trackIndex);
        for (auto clip : clips) {
            std::visit([&](auto&& c) { c->setName(newName); }, clip);
        }
        repaint();
        engine.getTimelineProject().notifyClipModified();
    }
}

void TrackLaneComponent::resized() {
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double pixelsPerSecond = timeline.getPixelsPerSecond();
    double scrollX = timeline.getScrollOffsetX();
    
    for (auto* clipComp : clipComponents) {
        auto clip = clipComp->getClip();
        
        double startSamples = 0.0;
        double lengthSamples = 0.0;
        
        if (std::holds_alternative<std::shared_ptr<MidiClip>>(clip)) {
            if (auto mc = std::get<std::shared_ptr<MidiClip>>(clip)) {
                startSamples = mc->getStartSample();
                lengthSamples = mc->getLengthSamples();
            }
        } else if (std::holds_alternative<std::shared_ptr<AudioClip>>(clip)) {
            if (auto ac = std::get<std::shared_ptr<AudioClip>>(clip)) {
                startSamples = ac->getStartSample();
                lengthSamples = ac->getLengthSamples();
            }
        }
        
        double startSeconds = startSamples / sampleRate;
        double lengthSeconds = lengthSamples / sampleRate;
        
        int x = static_cast<int>(startSeconds * pixelsPerSecond - scrollX);
        int w = static_cast<int>(lengthSeconds * pixelsPerSecond);
        
        clipComp->setBounds(x, 0, w, getHeight());
    }
}

void TrackLaneComponent::updateClips() {
    auto clips = engine.getTimelineProject().getClipsOnTrack(trackIndex);
    
    for (int i = clipComponents.size(); --i >= 0;) {
        auto clipData = clipComponents[i]->getClip();
        if (std::find(clips.begin(), clips.end(), clipData) == clips.end()) {
            clipComponents.remove(i);
        }
    }
    
    for (auto clip : clips) {
        bool found = false;
        for (auto* comp : clipComponents) {
            if (comp->getClip() == clip) {
                found = true;
                break;
            }
        }
        if (!found) {
            auto* clipComp = new ClipComponent(clip, engine);
            clipComponents.add(clipComp);
            addAndMakeVisible(clipComp);
        }
    }
    
    resized();
}

bool TrackLaneComponent::isInterestedInFileDrag(const juce::StringArray& files) {
    if (trackIndex >= engine.getTimelineProject().getNumTracks()) return false;
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    
    for (const auto& file : files) {
        if (file.endsWithIgnoreCase(".wav") || file.endsWithIgnoreCase(".aiff") || 
            file.endsWithIgnoreCase(".mp3") || file.endsWithIgnoreCase(".flac")) {
            return !track.isMidi;
        }
        if (file.endsWithIgnoreCase(".mid") || file.endsWithIgnoreCase(".midi")) {
            return track.isMidi;
        }
    }
    return false;
}

void TrackLaneComponent::filesDropped(const juce::StringArray& files, int x, int y) {
    if (trackIndex >= engine.getTimelineProject().getNumTracks()) return;
    const auto& track = engine.getTimelineProject().getTrack(trackIndex);
    
    double pixelsPerSecond = timeline.getPixelsPerSecond();
    if (pixelsPerSecond <= 0) return;
    
    double dropSeconds = (x + timeline.getScrollOffsetX()) / pixelsPerSecond;
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double startSamples = dropSeconds * sampleRate;
    
    for (const auto& filePath : files) {
        juce::File file(filePath);
        
        if (!track.isMidi && (file.hasFileExtension("wav") || file.hasFileExtension("aiff") || 
                              file.hasFileExtension("mp3") || file.hasFileExtension("flac"))) {
            
            auto reader = std::unique_ptr<juce::AudioFormatReader>(
                engine.getFormatManager().createReaderFor(file));
                
            if (reader) {
                int numSamples = static_cast<int>(reader->lengthInSamples);
                int numChannels = reader->numChannels;
                reader.reset(); // Close the file handle to prevent Windows file locking
                
                auto audioClip = std::make_shared<AudioClip>(file, static_cast<int>(startSamples), numSamples);
                audioClip->setNumChannels(numChannels);
                audioClip->setName(file.getFileNameWithoutExtension());
                engine.getTimelineProject().setTrackName(trackIndex, file.getFileNameWithoutExtension());
                engine.getTimelineProject().addClipToTrack(trackIndex, audioClip);
                return; // Only process the first valid file
            }
        }
        else if (track.isMidi && (file.hasFileExtension("mid") || file.hasFileExtension("midi"))) {
            juce::FileInputStream fis(file);
            juce::MidiFile midiFile;
            if (midiFile.readFrom(fis)) {
                midiFile.convertTimestampTicksToSeconds();
                double lengthInSeconds = 0.0;
                for (int i = 0; i < midiFile.getNumTracks(); ++i) {
                    auto* midiTrack = midiFile.getTrack(i);
                    if (midiTrack->getNumEvents() > 0) {
                        lengthInSeconds = std::max(lengthInSeconds, midiTrack->getEventPointer(midiTrack->getNumEvents() - 1)->message.getTimeStamp());
                    }
                }
                
                int numSamples = static_cast<int>(lengthInSeconds * sampleRate);
                if (numSamples <= 0) numSamples = static_cast<int>(sampleRate);
                
                auto midiClip = std::make_shared<MidiClip>(static_cast<int>(startSamples), numSamples);
                midiClip->setName(file.getFileNameWithoutExtension());
                engine.getTimelineProject().setTrackName(trackIndex, file.getFileNameWithoutExtension());
                
                for (int i = 0; i < midiFile.getNumTracks(); ++i) {
                    auto* midiTrack = midiFile.getTrack(i);
                    for (int j = 0; j < midiTrack->getNumEvents(); ++j) {
                        auto msg = midiTrack->getEventPointer(j)->message;
                        if (msg.isNoteOn()) {
                            double noteOffTime = msg.getTimeStamp() + 0.5;
                            int noteNum = msg.getNoteNumber();
                            for (int k = j + 1; k < midiTrack->getNumEvents(); ++k) {
                                auto msg2 = midiTrack->getEventPointer(k)->message;
                                if (msg2.isNoteOff() && msg2.getNoteNumber() == noteNum) {
                                    noteOffTime = msg2.getTimeStamp();
                                    break;
                                }
                            }
                            
                            midiClip->addNote(1, noteNum, msg.getFloatVelocity(), msg.getTimeStamp() * sampleRate, (noteOffTime - msg.getTimeStamp()) * sampleRate);
                        }
                    }
                }
                
                engine.getTimelineProject().addClipToTrack(trackIndex, midiClip);
                return; // Only process the first valid file
            }
        }
    }
}

void TrackLaneComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
}

void TrackLaneComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        auto& project = engine.getTimelineProject();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        double clickSeconds = (event.x + timeline.getScrollOffsetX()) / timeline.getPixelsPerSecond();
        double startSamples = clickSeconds * sampleRate;
        
        double selStart = project.getTimeSelectionStart();
        double selEnd = project.getTimeSelectionEnd();
        if (selStart >= 0 && selEnd >= 0 && selStart != selEnd) {
            double minSamples = std::min(selStart, selEnd);
            double maxSamples = std::max(selStart, selEnd);
            if (startSamples >= minSamples && startSamples <= maxSamples && project.getTimeSelectedTracks().contains(trackIndex)) {
                showContextMenuForTimeSelection(event);
                return;
            }
        }
        showContextMenuForLane(event);
    } else {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        double pixelsPerSecond = timeline.getPixelsPerSecond();
        double scrollX = timeline.getScrollOffsetX();
        
        double clickSeconds = (event.x + scrollX) / pixelsPerSecond;
        double startSamples = clickSeconds * sampleRate;
        
        auto& project = engine.getTimelineProject();
        project.setSelectedClip(AnyClipPtr{}); // Clear clip selection
        
        if (!event.mods.isShiftDown()) {
            project.clearTimeSelection();
        }
        project.setTimeSelection(startSamples, startSamples);
        project.addTimeSelectedTrack(trackIndex);
        isDraggingSelection = true;
    }
}

void TrackLaneComponent::mouseDrag(const juce::MouseEvent& event) {
    if (isDraggingSelection) {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        double pixelsPerSecond = timeline.getPixelsPerSecond();
        double scrollX = timeline.getScrollOffsetX();
        
        double currentSeconds = (event.x + scrollX) / pixelsPerSecond;
        double endSamples = currentSeconds * sampleRate;
        
        auto& project = engine.getTimelineProject();
        project.setTimeSelection(project.getTimeSelectionStart(), endSamples);
        
        // Multi-track highlighting logic
        if (auto* parent = getParentComponent()) {
            auto parentPos = event.getEventRelativeTo(parent).getPosition();
            int draggedTrackIndex = -1;
            
            for (int i = 0; i < parent->getNumChildComponents(); ++i) {
                if (auto* lane = dynamic_cast<TrackLaneComponent*>(parent->getChildComponent(i))) {
                    if (parentPos.y >= lane->getY() && parentPos.y < lane->getBottom()) {
                        draggedTrackIndex = lane->trackIndex;
                        break;
                    }
                }
            }
            
            if (draggedTrackIndex != -1) {
                int startTrack = std::min(trackIndex, draggedTrackIndex);
                int endTrack = std::max(trackIndex, draggedTrackIndex);
                
                juce::SparseSet<int> selectedTracks;
                selectedTracks.addRange(juce::Range<int>(startTrack, endTrack + 1));
                project.setTimeSelectedTracks(selectedTracks);
            }
        }
    }
}

void TrackLaneComponent::mouseUp(const juce::MouseEvent& event) {
    isDraggingSelection = false;
}

void TrackLaneComponent::showContextMenuForClip(const juce::MouseEvent& event, AnyClipPtr clip) {
    juce::PopupMenu menu;
    menu.addItem(1, "Cut", true, false);
    menu.addItem(2, "Copy", true, false);
    menu.addItem(3, "Duplicate", true, false);
    menu.addItem(4, "Delete", true, false);
    menu.addSeparator();
    menu.addItem(5, "Rename", true, false);
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMousePosition(),
        [this, clip](int result) {
            if (result == 1) { // Cut
                engine.getTimelineProject().setSelectedClip(clip);
                engine.getTimelineProject().copySelectedClips();
                engine.getTimelineProject().removeClip(clip);
            } else if (result == 2) { // Copy
                engine.getTimelineProject().setSelectedClip(clip);
                engine.getTimelineProject().copySelectedClips();
            } else if (result == 3) { // Duplicate
                // Currently just copy + paste right after clip end
                engine.getTimelineProject().setSelectedClip(clip);
                engine.getTimelineProject().copySelectedClips();
                double endSample = 0;
                std::visit([&](auto&& c) { endSample = c->getStartSample() + c->getLengthSamples(); }, clip);
                engine.getTimelineProject().pasteClips(trackIndex, endSample);
            } else if (result == 4) { // Delete
                engine.getTimelineProject().removeClip(clip);
            }
        });
}

void TrackLaneComponent::showContextMenuForTimeSelection(const juce::MouseEvent& event) {
    juce::PopupMenu menu;
    menu.addItem(1, "Cut", true, false);
    menu.addItem(2, "Copy", true, false);
    menu.addItem(3, "Duplicate", true, false);
    menu.addItem(4, "Delete", true, false);
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMousePosition(),
        [this](int result) {
            auto& project = engine.getTimelineProject();
            double selStart = project.getTimeSelectionStart();
            double selEnd = project.getTimeSelectionEnd();
            
            if (selStart >= 0 && selEnd >= 0 && selStart != selEnd) {
                if (result == 2 || result == 1 || result == 3) { // Copy / Cut / Duplicate
                    project.copySelectedClips();
                }
                
                if (result == 1 || result == 4) { // Cut / Delete
                    double minSamples = std::min(selStart, selEnd);
                    double maxSamples = std::max(selStart, selEnd);
                    auto clips = project.getClipsOnTrack(trackIndex);
                    for (auto clip : clips) {
                        double clipStart = 0;
                        double clipLength = 0;
                        std::visit([&](auto&& c) { clipStart = c->getStartSample(); clipLength = c->getLengthSamples(); }, clip);
                        double clipEnd = clipStart + clipLength;
                        
                        if (clipStart < maxSamples && clipEnd > minSamples) {
                            if (clipStart >= minSamples && clipEnd <= maxSamples) {
                                project.removeClip(clip);
                            } else if (clipStart < minSamples && clipEnd > maxSamples) {
                                std::visit([&](auto&& c) { c->setLengthSamples(minSamples - clipStart); }, clip);
                            } else if (clipStart < minSamples) {
                                std::visit([&](auto&& c) { c->setLengthSamples(minSamples - clipStart); }, clip);
                            } else if (clipEnd > maxSamples) {
                                std::visit([&](auto&& c) {
                                    double cutAmount = maxSamples - clipStart;
                                    c->setStartSample(maxSamples);
                                    c->setLengthSamples(clipLength - cutAmount);
                                    c->setSourceOffsetSamples(c->getSourceOffsetSamples() + cutAmount);
                                }, clip);
                            }
                        }
                    }
                    project.clearTimeSelection();
                    project.notifyClipModified();
                }
                
                if (result == 3) { // Duplicate
                    double maxSamples = std::max(selStart, selEnd);
                    project.pasteClips(trackIndex, maxSamples);
                }
            }
        });
}

void TrackLaneComponent::showContextMenuForLane(const juce::MouseEvent& event) {
    juce::PopupMenu menu;
    menu.addItem(28, "Paste", true, false);
    menu.addSeparator();
    menu.addItem(9, "Insert Empty MIDI Clip(s)", true, false);
    
    float clickX = event.position.x;
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMousePosition(),
        [this, clickX](int result) {
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            double pixelsPerSecond = timeline.getPixelsPerSecond();
            double scrollX = timeline.getScrollOffsetX();
            double clickSeconds = (clickX + scrollX) / pixelsPerSecond;
            double pasteStartSamples = clickSeconds * sampleRate;
            
            if (result == 28) { // Paste
                engine.getTimelineProject().pasteClips(trackIndex, pasteStartSamples);
            } else if (result == 9) { // Insert Empty MIDI Clip
                double tempo = engine.getTransport().getTempo();
                double secondsPerBeat = 60.0 / tempo;
                double snappedSeconds = std::round(clickSeconds / secondsPerBeat) * secondsPerBeat;
                double startSamples = snappedSeconds * sampleRate;
                double lengthSamples = (secondsPerBeat * 4.0) * sampleRate; // 1 bar
                
                auto clip = std::make_shared<MidiClip>(startSamples, lengthSamples);
                engine.getTimelineProject().addClipToTrack(trackIndex, clip);
            }
        });
}

} // namespace Nimbus::Timeline
