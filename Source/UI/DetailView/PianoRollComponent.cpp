#include "PianoRollComponent.h"
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::DetailView {

// ==============================================================================
PianoRollContent::PianoRollContent(NimbusEngine& e) : engine(e) {}
PianoRollContent::~PianoRollContent() = default;

void PianoRollContent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentClip = clip;
    repaint();
}

void PianoRollContent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        int vx = vp->getViewPositionX();
        
        // Draw vertical grid and notes first
        if (currentClip) {
            double clipSamples = currentClip->getLengthSamples();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            double clipSeconds = clipSamples / sampleRate;
            
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double secondsPer16th = secondsPerBeat / 4.0;
            
            double pixelsPerSecond = 100.0;
            int num16ths = static_cast<int>(clipSeconds / secondsPer16th);
            
            g.setColour(DesignSystem::Colors::Divider.withAlpha(0.3f));
            for (int i = 0; i <= num16ths; ++i) {
                float x = keyWidth + static_cast<float>((i * secondsPer16th) * pixelsPerSecond);
                g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
            }
            
            // Draw notes
            if (clipSamples > 0) {
                double pixelsPerSecond = 100.0;
                for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                    auto* event = currentClip->getSequence().getEventPointer(i);
                    if (event->message.isNoteOn()) {
                        double noteStart = event->message.getTimeStamp();
                        double noteLength = 0.0;
                        
                        if (event->noteOffObject != nullptr) {
                            noteLength = event->noteOffObject->message.getTimeStamp() - noteStart;
                        }
                        if (noteLength == 0.0) noteLength = 48000.0 * 0.25;
                        
                        float x = keyWidth + static_cast<float>((noteStart / sampleRate) * pixelsPerSecond);
                        float w = static_cast<float>((noteLength / sampleRate) * pixelsPerSecond);
                        
                        int noteNumber = event->message.getNoteNumber();
                        int row = 127 - noteNumber;
                        int y = row * keyHeight;
                        
                        g.setColour(DesignSystem::Colors::PrimaryAction);
                        g.fillRect(x, static_cast<float>(y), w, static_cast<float>(keyHeight));
                        
                        g.setColour(juce::Colours::black.withAlpha(0.8f));
                        g.drawRect(x, static_cast<float>(y), w, static_cast<float>(keyHeight), 1.0f);
                        
                        // Wait, velocity is handled differently now.
                    }
                }
            }
        }
            
        // Draw keyboard on the left, sticky!
        for (int note = 0; note < totalKeys; ++note) {
            int y = note * keyHeight;
            int midiNote = 127 - note;
            bool isBlack = juce::MidiMessage::isMidiNoteBlack(midiNote);
            
            juce::Rectangle<float> keyRect(vx, y, keyWidth, keyHeight);
            
            if (isBlack) {
                g.setColour(juce::Colour::fromString("#FF2A2A2E")); // Dark grey
                g.fillRoundedRectangle(keyRect.reduced(0, 1).withTrimmedRight(2), 3.0f);
            } else {
                g.setColour(juce::Colour::fromString("#FFF4F4F4")); // Off-white
                g.fillRoundedRectangle(keyRect.reduced(0, 0.5f).withTrimmedRight(2), 2.0f);
            }
            
            // Draw horizontal grid lines extended from white keys
            if (!isBlack) {
                g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
                g.drawHorizontalLine(y + keyHeight, vx + keyWidth, static_cast<float>(getWidth()));
            }
            
            // Draw C note labels
            if (midiNote % 12 == 0 && !isBlack) {
                g.setColour(juce::Colours::black.withAlpha(0.7f));
                g.setFont(juce::Font(10.0f).boldened());
                g.drawText("C" + juce::String((midiNote / 12) - 2), vx + 2, y, keyWidth - 8, keyHeight, juce::Justification::centredRight, false);
            }
        }

        // Draw velocity lane at the bottom of the viewport if visible
        if (velocityVisible) {
            int contentHeight = vp->getViewPositionY() + vp->getHeight() - velocityLaneHeight;
            g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.1f));
            g.fillRect(0.0f, static_cast<float>(contentHeight), static_cast<float>(getWidth()), static_cast<float>(velocityLaneHeight));
            g.setColour(DesignSystem::Colors::Divider);
            g.drawHorizontalLine(contentHeight, 0.0f, static_cast<float>(getWidth()));
            
            if (currentClip) {
                double sampleRate = engine.getTransport().getSampleRate();
                if (sampleRate <= 0) sampleRate = 48000.0;
                double pixelsPerSecond = 100.0;
                
                for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                    auto* event = currentClip->getSequence().getEventPointer(i);
                    if (event->message.isNoteOn()) {
                        double noteStart = event->message.getTimeStamp();
                        float x = keyWidth + static_cast<float>((noteStart / sampleRate) * pixelsPerSecond);
                        
                        float vel = event->message.getVelocity() / 127.0f;
                        float vh = vel * (velocityLaneHeight - 4);
                        float vy = contentHeight + velocityLaneHeight - vh;
                        
                        g.setColour(selectedEventIndices.contains(i) ? DesignSystem::Colors::PrimaryAction : DesignSystem::Colors::PrimaryAction.withAlpha(0.6f));
                        g.fillRect(x - 2.0f, vy, 4.0f, vh);
                        g.setColour(juce::Colours::black.withAlpha(0.5f));
                        g.drawRect(x - 2.0f, vy, 4.0f, vh, 1.0f);
                    }
                }
            }
            
            // Draw velocity label on the left
            g.setColour(DesignSystem::Colors::TextSecondary);
            g.setFont(juce::Font(12.0f));
            g.drawText("Velocity", vx + 5, contentHeight + 5, keyWidth - 10, 20, juce::Justification::centredLeft, false);
        }
    
        // Draw marquee selection
        if (isMarqueeSelecting) {
            g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
            g.fillRect(marqueeRect);
            g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.5f));
            g.drawRect(marqueeRect, 1.0f);
        }
        
        // Draw playhead
        if (currentClip && engine.getTransport().isPlaying()) {
            double positionSamples = engine.getTransport().getCurrentPosition();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0.0) sampleRate = 48000.0;
            
            double clipGlobalStart = currentClip->getStartSample();
            double clipGlobalEnd = clipGlobalStart + currentClip->getLengthSamples();
            
            if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
                double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
                double pixelsPerSecond = 100.0;
                float px = keyWidth + static_cast<float>(timeIntoClip * pixelsPerSecond);
                
                g.setColour(DesignSystem::Colors::PrimaryAction);
                g.drawVerticalLine(static_cast<int>(px), 0.0f, static_cast<float>(getHeight()));
            }
        }
    }
}

void PianoRollContent::mouseDown(const juce::MouseEvent& event) {
    if (!currentClip) return;
    
    draggedEventIndex = -1;
    isResizing = false;
    isDraggingVelocity = false;
    
    int contentHeight = 128 * keyHeight;
    int vx = 0;
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        contentHeight = vp->getViewPositionY() + vp->getHeight() - velocityLaneHeight;
        vx = vp->getViewPositionX();
    }
    
    if (velocityVisible && event.y > contentHeight) {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        double pixelsPerSecond = 100.0;
        
        for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
            auto* evt = currentClip->getSequence().getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double noteStart = evt->message.getTimeStamp();
                float x = keyWidth + static_cast<float>((noteStart / sampleRate) * pixelsPerSecond);
                if (std::abs(event.x - x) <= 4.0f) {
                    draggedEventIndex = i;
                    isDraggingVelocity = true;
                    if (!event.mods.isShiftDown() && !event.mods.isCommandDown()) {
                        selectedEventIndices.clear();
                    }
                    selectedEventIndices.addIfNotAlreadyThere(i);
                    break;
                }
            }
        }
        
        if (draggedEventIndex != -1) {
            float vel = juce::jlimit(0.0f, 1.0f, 1.0f - static_cast<float>(event.y - contentHeight) / velocityLaneHeight);
            for (int idx : selectedEventIndices) {
                auto* evt = currentClip->getSequence().getEventPointer(idx);
                if (evt && evt->message.isNoteOn()) {
                    evt->message.setVelocity(vel);
                }
            }
            engine.getTimelineProject().notifyClipModified();
            repaint();
        }
        return;
    }
    
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        int vx = vp->getViewPositionX();
        if (event.getPosition().x > vx + keyWidth) {
            int x = event.getPosition().x - keyWidth;
            int y = event.getPosition().y;
            
            int row = y / keyHeight;
            if (row < 0 || row > 127) return;
            int noteNumber = 127 - row;
            
            double clipSamples = currentClip->getLengthSamples();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double secondsPer16th = secondsPerBeat / 4.0;
            double samplesPer16th = secondsPer16th * sampleRate;
            int gridWidth = getWidth() - keyWidth;
            double pixelsPerSecond = 100.0;
            double timeInClip = (static_cast<double>(x) / pixelsPerSecond) * sampleRate;
            if (timeInClip < 0.0) timeInClip = 0.0;
            
            // Look for existing note under mouse
            int foundNoteIndex = -1;
            double foundNoteStart = 0.0;
            double foundNoteLength = 0.0;
            
            for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                auto* evt = currentClip->getSequence().getEventPointer(i);
                if (evt->message.isNoteOn() && evt->message.getNoteNumber() == noteNumber) {
                    double noteStart = evt->message.getTimeStamp();
                    double noteLength = samplesPer16th;
                    if (evt->noteOffObject != nullptr) {
                        noteLength = evt->noteOffObject->message.getTimeStamp() - noteStart;
                    }
                    if (timeInClip >= noteStart && timeInClip <= noteStart + noteLength) {
                        foundNoteIndex = i;
                        foundNoteStart = noteStart;
                        foundNoteLength = noteLength;
                        break;
                    }
                }
            }
            
            if (foundNoteIndex != -1) {
                if (event.mods.isPopupMenu() || event.mods.isCommandDown() || event.mods.isCtrlDown()) {
                    // Delete note
                    currentClip->getSequence().deleteEvent(foundNoteIndex, true);
                    currentClip->getSequence().updateMatchedPairs();
                    engine.getTimelineProject().notifyClipModified();
                    repaint();
                } else {
                    // Prepare to drag or resize
                    draggedEventIndex = foundNoteIndex;
                    dragStartMouseX = event.getPosition().x;
                    dragStartMouseY = event.getPosition().y;
                    dragStartNoteTime = foundNoteStart;
                    dragStartNoteLength = foundNoteLength;
                    dragStartNoteNumber = noteNumber;
                    
                    double noteRightEdgeTime = foundNoteStart + foundNoteLength;
                    double noteRightEdgeX = (noteRightEdgeTime / sampleRate) * pixelsPerSecond;
                    if (x >= noteRightEdgeX - 5.0) {
                        isResizing = true;
                    }
                }
                return;
            }
            
            // Clicked empty space
            if (!event.mods.isShiftDown() && !event.mods.isCommandDown()) {
                selectedEventIndices.clear();
            }
            isMarqueeSelecting = true;
            dragStartMouseX = event.getPosition().x;
            dragStartMouseY = event.getPosition().y;
            marqueeRect = juce::Rectangle<float>(static_cast<float>(event.getPosition().x), static_cast<float>(event.getPosition().y), 0.0f, 0.0f);
            repaint();
            
            // Add snapped note if not right-clicking
            if (!event.mods.isPopupMenu()) {
                double snappedTime = std::floor(timeInClip / samplesPer16th) * samplesPer16th;
                
                juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
                noteOn.setTimeStamp(snappedTime);
                
                juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, (juce::uint8)0);
                noteOff.setTimeStamp(snappedTime + samplesPer16th);
                
                currentClip->getSequence().addEvent(noteOn);
                currentClip->getSequence().addEvent(noteOff);
                currentClip->getSequence().updateMatchedPairs();
                
                // Audition the note
                int trackIndex = -1;
                auto& sel = engine.getTimelineProject().getSelectedTracks();
                if (sel.getNumRanges() > 0) {
                    trackIndex = sel.getRange(0).getStart();
                }
                if (trackIndex >= 0) {
                    if (auto* track = engine.getMixer()->getTrack(trackIndex)) {
                        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
                        track->addLiveMidiMessage(noteOn);
                    }
                }

                engine.getTimelineProject().notifyClipModified();
                repaint();
                
                // Set as dragged so user can resize immediately
                for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                    auto* evt = currentClip->getSequence().getEventPointer(i);
                    if (evt->message.isNoteOn() && evt->message.getNoteNumber() == noteNumber && evt->message.getTimeStamp() == snappedTime) {
                        draggedEventIndex = i;
                        dragStartMouseX = event.getPosition().x;
                        dragStartMouseY = event.getPosition().y;
                        break;
                    }
                }
            }
            return;
        }
    }

    if (currentTool == Tool::Pointer) {
        if (event.x < keyWidth) return;
        
        auto& seq = currentClip->getSequence();
        draggedEventIndex = -1;
        isResizing = false;
        
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        double pixelsPerSecond = 100.0;
        
        for (int i = seq.getNumEvents() - 1; i >= 0; --i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                double len = 0.25 * sampleRate;
                if (evt->noteOffObject) {
                    len = evt->noteOffObject->message.getTimeStamp() - t;
                }
                
                float x = static_cast<float>(keyWidth + (t / sampleRate * pixelsPerSecond));
                float w = static_cast<float>(len / sampleRate * pixelsPerSecond);
                
                int noteNum = evt->message.getNoteNumber();
                int row = 127 - noteNum;
                float y = static_cast<float>(row * keyHeight);
                
                juce::Rectangle<float> rect(x, y, w, static_cast<float>(keyHeight));
                
                if (rect.contains(event.position)) {
                    draggedEventIndex = i;
                    
                    if (event.mods.isRightButtonDown() || event.mods.isCtrlDown()) {
                        if (evt->noteOffObject) seq.deleteEvent(seq.getIndexOf(evt->noteOffObject), true);
                        seq.deleteEvent(i, true);
                        draggedEventIndex = -1;
                        selectedEventIndices.removeFirstMatchingValue(i);
                        engine.getTimelineProject().notifyClipModified();
                        repaint();
                        return;
                    }
                    
                    if (event.x > rect.getRight() - 5.0f) {
                        isResizing = true;
                    }
                    
                    dragStartNoteTime = t;
                    dragStartNoteLength = len;
                    dragStartMouseX = event.x;
                    dragStartMouseY = event.y;
                    dragStartNoteNumber = noteNum;
                    
                    if (!selectedEventIndices.contains(draggedEventIndex)) {
                        if (!event.mods.isShiftDown() && !event.mods.isCommandDown()) {
                            selectedEventIndices.clear();
                        }
                        selectedEventIndices.addIfNotAlreadyThere(draggedEventIndex);
                    } else if (event.mods.isCommandDown() || event.mods.isShiftDown()) {
                        selectedEventIndices.removeFirstMatchingValue(draggedEventIndex);
                        draggedEventIndex = -1;
                    }
                    
                    repaint();
                    
                    if (draggedEventIndex != -1 && !isResizing) {
                        int trackIndex = engine.getTimelineProject().getSelectedTracks().getRange(0).getStart();
                        if (auto* track = engine.getMixer()->getTrack(trackIndex)) {
                            juce::MidiMessage shortOn = juce::MidiMessage::noteOn(1, noteNum, (juce::uint8)100);
                            track->addLiveMidiMessage(shortOn);
                        }
                    }
                    return;
                }
            }
        }
        
        // Clicked empty space
        if (!event.mods.isShiftDown() && !event.mods.isCommandDown()) {
            selectedEventIndices.clear();
        }
        isMarqueeSelecting = true;
        dragStartMouseX = event.x;
        dragStartMouseY = event.y;
        marqueeRect = juce::Rectangle<float>(static_cast<float>(event.x), static_cast<float>(event.y), 0.0f, 0.0f);
        repaint();
    } else if (currentTool == Tool::Pencil) {
        if (event.x < keyWidth) return;
        
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        double tempo = engine.getTransport().getTempo();
        double secondsPerBeat = 60.0 / tempo;
        
        double snapBeats = 0.25;
        if (currentSnap == Snap::Bar) snapBeats = 4.0;
        else if (currentSnap == Snap::Beat) snapBeats = 1.0;
        else if (currentSnap == Snap::Eighth) snapBeats = 0.5;
        else if (currentSnap == Snap::Sixteenth) snapBeats = 0.25;
        else if (currentSnap == Snap::ThirtySecond) snapBeats = 0.125;
        
        double snapSeconds = snapBeats * secondsPerBeat;
        double snapSamples = snapSeconds * sampleRate;
        double pixelsPerSecond = 100.0;
        
        double clickedSeconds = (event.x - keyWidth) / pixelsPerSecond;
        double snappedSeconds = currentSnap == Snap::Off ? clickedSeconds : std::floor(clickedSeconds / snapSeconds) * snapSeconds;
        double startSample = snappedSeconds * sampleRate;
        
        int row = event.y / keyHeight;
        int noteNum = 127 - row;
        
        currentClip->addNote(1, noteNum, 0.8f, startSample, snapSamples > 0 ? snapSamples : sampleRate * 0.25);
        currentClip->getSequence().updateMatchedPairs();
        
        engine.getTimelineProject().notifyClipModified();
        repaint();
    } else if (currentTool == Tool::Eraser) {
        if (event.x < keyWidth) return;
        auto& seq = currentClip->getSequence();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        double pixelsPerSecond = 100.0;
        
        for (int i = seq.getNumEvents() - 1; i >= 0; --i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                double len = 0.25 * sampleRate;
                if (evt->noteOffObject) len = evt->noteOffObject->message.getTimeStamp() - t;
                
                float x = static_cast<float>(keyWidth + (t / sampleRate * pixelsPerSecond));
                float w = static_cast<float>(len / sampleRate * pixelsPerSecond);
                int noteNum = evt->message.getNoteNumber();
                int row = 127 - noteNum;
                float y = static_cast<float>(row * keyHeight);
                
                juce::Rectangle<float> rect(x, y, w, static_cast<float>(keyHeight));
                
                if (rect.contains(event.position)) {
                    if (evt->noteOffObject) seq.deleteEvent(seq.getIndexOf(evt->noteOffObject), true);
                    seq.deleteEvent(i, true);
                    selectedEventIndices.removeFirstMatchingValue(i);
                    engine.getTimelineProject().notifyClipModified();
                    repaint();
                    return;
                }
            }
        }
    }
}

void PianoRollContent::mouseDrag(const juce::MouseEvent& event) {
    if (!currentClip) return;
    
    if (isMarqueeSelecting) {
        marqueeRect.setSize(event.x - dragStartMouseX, event.y - dragStartMouseY);
        if (marqueeRect.getWidth() < 0) {
            marqueeRect.setX(static_cast<float>(event.x));
            marqueeRect.setWidth(static_cast<float>(dragStartMouseX - event.x));
        }
        if (marqueeRect.getHeight() < 0) {
            marqueeRect.setY(static_cast<float>(event.y));
            marqueeRect.setHeight(static_cast<float>(dragStartMouseY - event.y));
        }
        
        // Select notes in marquee
        if (!event.mods.isShiftDown() && !event.mods.isCommandDown()) {
            selectedEventIndices.clear();
        }
        
        auto& seq = currentClip->getSequence();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        double pixelsPerSecond = 100.0;
        
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                double len = 0.25 * sampleRate;
                if (evt->noteOffObject) len = evt->noteOffObject->message.getTimeStamp() - t;
                
                float x = static_cast<float>(keyWidth + (t / sampleRate * pixelsPerSecond));
                float w = static_cast<float>(len / sampleRate * pixelsPerSecond);
                int row = 127 - evt->message.getNoteNumber();
                float y = static_cast<float>(row * keyHeight);
                
                juce::Rectangle<float> rect(x, y, w, static_cast<float>(keyHeight));
                if (marqueeRect.intersects(rect)) {
                    selectedEventIndices.addIfNotAlreadyThere(i);
                }
            }
        }
        repaint();
        return;
    }
    
    if (draggedEventIndex == -1) return;
    
    if (isDraggingVelocity) {
        int contentHeight = 128 * keyHeight;
        if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
            contentHeight = vp->getViewPositionY() + vp->getHeight() - velocityLaneHeight;
        }
        float vel = juce::jlimit(0.0f, 1.0f, 1.0f - static_cast<float>(event.y - contentHeight) / velocityLaneHeight);
        for (int idx : selectedEventIndices) {
            auto* evt = currentClip->getSequence().getEventPointer(idx);
            if (evt && evt->message.isNoteOn()) {
                evt->message.setVelocity(vel);
            }
        }
        engine.getTimelineProject().notifyClipModified();
        repaint();
        return;
    }
    
    auto& seq = currentClip->getSequence();
    auto* draggedEvt = seq.getEventPointer(draggedEventIndex);
    if (!draggedEvt || !draggedEvt->message.isNoteOn()) return;

    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    
    double snapBeats = 0.25;
    if (currentSnap == Snap::Bar) snapBeats = 4.0;
    else if (currentSnap == Snap::Beat) snapBeats = 1.0;
    else if (currentSnap == Snap::Eighth) snapBeats = 0.5;
    else if (currentSnap == Snap::Sixteenth) snapBeats = 0.25;
    else if (currentSnap == Snap::ThirtySecond) snapBeats = 0.125;
    
    double snapSeconds = snapBeats * secondsPerBeat;
    double snapSamples = snapSeconds * sampleRate;
    double pixelsPerSecond = 100.0;
    
    double xDeltaSeconds = (event.x - dragStartMouseX) / pixelsPerSecond;
    double newTimeSeconds = (dragStartNoteTime / sampleRate) + xDeltaSeconds;
    
    if (currentSnap != Snap::Off) {
        newTimeSeconds = std::round(newTimeSeconds / snapSeconds) * snapSeconds;
    }
    
    if (newTimeSeconds < 0) newTimeSeconds = 0;
    double newTimeSamples = newTimeSeconds * sampleRate;
    
    if (isResizing) {
        double newLengthSeconds = (dragStartNoteLength / sampleRate) + xDeltaSeconds;
        if (currentSnap != Snap::Off) {
            newLengthSeconds = std::round(newLengthSeconds / snapSeconds) * snapSeconds;
        }
        if (newLengthSeconds < snapSeconds && currentSnap != Snap::Off) newLengthSeconds = snapSeconds;
        if (newLengthSeconds < 0.01) newLengthSeconds = 0.01;
        
        double newLengthSamples = newLengthSeconds * sampleRate;
        
        if (draggedEvt->noteOffObject) {
            draggedEvt->noteOffObject->message.setTimeStamp(draggedEvt->message.getTimeStamp() + newLengthSamples);
        }
    } else {
        int yDeltaRows = (event.y - dragStartMouseY) / keyHeight;
        int newNoteNumber = dragStartNoteNumber - yDeltaRows;
        newNoteNumber = juce::jlimit(0, 127, newNoteNumber);
        
        double deltaSamples = newTimeSamples - dragStartNoteTime;
        int deltaNoteNumber = newNoteNumber - dragStartNoteNumber;
        
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt && evt->message.isNoteOn()) {
                evt->message.setTimeStamp(evt->message.getTimeStamp() + deltaSamples);
                int p = juce::jlimit(0, 127, evt->message.getNoteNumber() + deltaNoteNumber);
                evt->message.setNoteNumber(p);
                
                if (evt->noteOffObject) {
                    evt->noteOffObject->message.setTimeStamp(evt->noteOffObject->message.getTimeStamp() + deltaSamples);
                    evt->noteOffObject->message.setNoteNumber(p);
                }
            }
        }
        dragStartNoteTime += deltaSamples;
        dragStartNoteNumber += deltaNoteNumber;
        dragStartMouseY += yDeltaRows * keyHeight;
    }
    
    seq.updateMatchedPairs();
    engine.getTimelineProject().notifyClipModified();
    
    draggedEventIndex = seq.getIndexOf(draggedEvt);
    repaint();
}

void PianoRollContent::mouseUp(const juce::MouseEvent& event) {
    if (isMarqueeSelecting) {
        isMarqueeSelecting = false;
        repaint();
        return;
    }
    if (draggedEventIndex != -1 && !isResizing && !isDraggingVelocity) {
        int trackIndex = engine.getTimelineProject().getSelectedTracks().getRange(0).getStart();
        if (auto* track = engine.getMixer()->getTrack(trackIndex)) {
            juce::MidiMessage shortOff = juce::MidiMessage::noteOff(1, dragStartNoteNumber, (juce::uint8)0);
            track->addLiveMidiMessage(shortOff);
        }
    }
    isDraggingVelocity = false;
    draggedEventIndex = -1;
}

// ==============================================================================
PianoRollComponent::PianoRollComponent(NimbusEngine& e) : engine(e), content(e) {
    addAndMakeVisible(toolbar);
    
    int iconSize = 0;
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Pointer.toUTF8(), iconSize)) {
        pointerIcon = juce::Drawable::createFromImageData(data, iconSize);
        if (pointerIcon) pointerIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
    }
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Pencil.toUTF8(), iconSize)) {
        pencilIcon = juce::Drawable::createFromImageData(data, iconSize);
        if (pencilIcon) pencilIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
    }
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Eraser.toUTF8(), iconSize)) {
        eraserIcon = juce::Drawable::createFromImageData(data, iconSize);
        if (eraserIcon) eraserIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
    }
    
    if (pointerIcon) pointerButton.setImages(pointerIcon.get());
    if (pencilIcon) pencilButton.setImages(pencilIcon.get());
    if (eraserIcon) eraserButton.setImages(eraserIcon.get());
    
    pointerButton.onClick = [this] { content.setTool(PianoRollContent::Tool::Pointer); };
    pencilButton.onClick = [this] { content.setTool(PianoRollContent::Tool::Pencil); };
    eraserButton.onClick = [this] { content.setTool(PianoRollContent::Tool::Eraser); };
    
    toolbar.addAndMakeVisible(pointerButton);
    toolbar.addAndMakeVisible(pencilButton);
    toolbar.addAndMakeVisible(eraserButton);
    
    snapBox.addItem("Off", 1);
    snapBox.addItem("Bar", 2);
    snapBox.addItem("Beat", 3);
    snapBox.addItem("1/8", 4);
    snapBox.addItem("1/16", 5);
    snapBox.addItem("1/32", 6);
    snapBox.setSelectedId(5);
    snapBox.onChange = [this] {
        int id = snapBox.getSelectedId();
        PianoRollContent::Snap s = PianoRollContent::Snap::Sixteenth;
        if (id == 1) s = PianoRollContent::Snap::Off;
        else if (id == 2) s = PianoRollContent::Snap::Bar;
        else if (id == 3) s = PianoRollContent::Snap::Beat;
        else if (id == 4) s = PianoRollContent::Snap::Eighth;
        else if (id == 5) s = PianoRollContent::Snap::Sixteenth;
        else if (id == 6) s = PianoRollContent::Snap::ThirtySecond;
        content.setSnap(s);
    };
    toolbar.addAndMakeVisible(snapBox);
    
    velocityToggle.setToggleState(true, juce::dontSendNotification);
    content.setVelocityVisible(true);
    velocityToggle.onClick = [this] {
        content.setVelocityVisible(velocityToggle.getToggleState());
        content.setBounds(0, 0, juce::jmax(1000, getWidth()), content.getDesiredHeight());
        resized();
    };
    toolbar.addAndMakeVisible(velocityToggle);
    
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
}

PianoRollComponent::~PianoRollComponent() = default;

void PianoRollComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    content.setMidiClip(clip);
    // Center around C3 (Note 60)
    int c3Row = 127 - 60;
    viewport.setViewPosition(0, (c3Row * 16) - (viewport.getHeight() / 2));
}

void PianoRollComponent::resized() {
    auto bounds = getLocalBounds();
    auto toolBounds = bounds.removeFromTop(30);
    toolbar.setBounds(toolBounds);
    
    int x = 10;
    pointerButton.setBounds(x, 3, 24, 24); x += 30;
    pencilButton.setBounds(x, 3, 24, 24); x += 30;
    eraserButton.setBounds(x, 3, 24, 24); x += 40;
    
    snapBox.setBounds(x, 3, 80, 24); x += 90;
    velocityToggle.setBounds(x, 3, 80, 24);
    
    viewport.setBounds(bounds);
    content.setBounds(0, 0, juce::jmax(1000, getWidth()), content.getDesiredHeight());
}

void PianoRollComponent::paint(juce::Graphics& g) {
    g.setColour(DesignSystem::Colors::ComponentBackground);
    g.fillRect(toolbar.getBounds());
    g.setColour(DesignSystem::Colors::Divider);
    g.drawHorizontalLine(toolbar.getBottom() - 1, 0, static_cast<float>(getWidth()));
}

} // namespace Nimbus::DetailView
