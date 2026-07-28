#include "PianoRollComponent.h"
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include "UI/Timeline/ClipComponent.h"
#include "DataModel/TimelineProject.h"
#include "AudioEngine/Track.h"

namespace Nimbus::DetailView {

// ==============================================================================
PianoRollContent::PianoRollContent(NimbusEngine& e) : engine(e) {
    setWantsKeyboardFocus(true);
}
PianoRollContent::~PianoRollContent() = default;

void PianoRollContent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentClip = clip;
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        setBounds(0, 0, juce::jmax(1000, getDesiredWidth()), getDesiredHeight());
    }
    repaint();
}

int PianoRollContent::getDesiredWidth() const {
    if (!currentClip) return 1000;
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    double clipSeconds = currentClip->lengthSamples.get() / sampleRate;
    return keyWidth + static_cast<int>(clipSeconds * 100.0 * timeZoom) + 500;
}


void PianoRollContent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff181818)); // Slightly lighter background
    
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        int vx = vp->getViewPositionX();
        
        // Draw vertical grid and notes first
        if (currentClip) {
            double clipSamples = currentClip->lengthSamples.get();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            double clipSeconds = clipSamples / sampleRate;
            
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double secondsPer16th = secondsPerBeat / 4.0;
            
            double pixelsPerSecond = 100.0 * timeZoom;
            int num16ths = static_cast<int>(clipSeconds / secondsPer16th);
            
            g.setColour(DesignSystem::Colors::Divider.withAlpha(0.3f));
            for (int i = 0; i <= num16ths; ++i) {
                float x = keyWidth + static_cast<float>((i * secondsPer16th) * pixelsPerSecond);
                g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
            }
            
            // Draw ghost notes from other MIDI clips
            auto& project = engine.getTimelineProject();
            for (int trackIdx = 0; trackIdx < project.getNumTracks(); ++trackIdx) {
            for (auto& clipBase : project.getClipsOnTrack(trackIdx)) {
                if (clipBase->getType() == Clip::Type::Midi && clipBase.get() != currentClip.get()) {
                    auto otherClip = std::static_pointer_cast<MidiClip>(clipBase);
                    double otherGlobalStart = otherClip->startSample.get();
                        double otherOffset = otherClip->sourceOffsetSamples.get();
                        
                        double currentGlobalStart = currentClip->startSample.get();
                        double currentOffset = currentClip->sourceOffsetSamples.get();
                        
                        for (int i = 0; i < otherClip->getSequence().getNumEvents(); ++i) {
                            auto* event = otherClip->getSequence().getEventPointer(i);
                            if (event->message.isNoteOn()) {
                                double noteStart = event->message.getTimeStamp();
                                double noteLength = 0.0;
                                if (event->noteOffObject != nullptr) {
                                    noteLength = event->noteOffObject->message.getTimeStamp() - noteStart;
                                }
                                if (noteLength == 0.0) noteLength = 48000.0 * 0.25;
                                
                                double globalNoteStart = otherGlobalStart + noteStart - otherOffset;
                                double relativeNoteStart = globalNoteStart - currentGlobalStart + currentOffset;
                                
                                float x = keyWidth + static_cast<float>((relativeNoteStart / sampleRate) * pixelsPerSecond);
                                float w = static_cast<float>((noteLength / sampleRate) * pixelsPerSecond);
                                
                                int noteNumber = event->message.getNoteNumber();
                                int row = 127 - noteNumber;
                                int y = row * keyHeight;
                                
                                // Draw faintly
                                juce::Colour clipColor = Nimbus::Timeline::ClipComponent::getClipColor(otherClip->colorIndex.get());
                                g.setColour(clipColor.withAlpha(0.2f));
                                g.fillRect(x, static_cast<float>(y) + 1.0f, w, static_cast<float>(keyHeight) - 2.0f);
                            }
                        }
                    }
                }
            }
            
            // Draw notes
            if (clipSamples > 0) {
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
                        
                        float vel = event->message.getVelocity() / 127.0f;
                        juce::Colour clipColor = Nimbus::Timeline::ClipComponent::getClipColor(currentClip->colorIndex.get());
                        juce::Colour noteColor = clipColor.interpolatedWith(juce::Colours::white, vel * 0.5f);
                        
                        if (selectedEventIndices.contains(i)) {
                            noteColor = noteColor.brighter(0.4f);
                            g.setColour(noteColor.withAlpha(0.3f));
                            g.fillRoundedRectangle(x - 2, static_cast<float>(y) - 1, w + 4, static_cast<float>(keyHeight) + 2, 4.0f);
                        }
                        
                        g.setColour(noteColor);
                        g.fillRoundedRectangle(x, static_cast<float>(y) + 1.0f, w, static_cast<float>(keyHeight) - 2.0f, 2.0f);
                        
                        g.setColour(juce::Colours::black.withAlpha(0.6f));
                        g.drawRoundedRectangle(x, static_cast<float>(y) + 1.0f, w, static_cast<float>(keyHeight) - 2.0f, 2.0f, 1.0f);
                        
                        // Wait, velocity is handled differently now.
                    }
                }
            }
        }
            
        // Draw keyboard on the left, sticky!
        // First pass: Draw white keys
        for (int note = 0; note < totalKeys; ++note) {
            int midiNote = 127 - note;
            if (!juce::MidiMessage::isMidiNoteBlack(midiNote)) {
                int y = note * keyHeight;
                juce::Rectangle<float> keyRect(vx, y, keyWidth, keyHeight);
                g.setColour(juce::Colour::fromString("#FFF4F4F4"));
                g.fillRect(keyRect);
                g.setColour(DesignSystem::Colors::Divider.withAlpha(0.2f));
                g.drawHorizontalLine(y + keyHeight, vx, vx + keyWidth);
            }
        }
        
        // Second pass: Draw black keys on top
        for (int note = 0; note < totalKeys; ++note) {
            int midiNote = 127 - note;
            if (juce::MidiMessage::isMidiNoteBlack(midiNote)) {
                int y = note * keyHeight;
                float blackKeyWidth = keyWidth * 0.65f;
                juce::Rectangle<float> keyRect(vx, y, blackKeyWidth, keyHeight);
                g.setColour(juce::Colour::fromString("#FF2A2A2E"));
                g.fillRoundedRectangle(keyRect.reduced(0, 1).withTrimmedRight(2), 3.0f);
            }
        }
        
        // Third pass: Draw grid lines and labels
        for (int note = 0; note < totalKeys; ++note) {
            int y = note * keyHeight;
            int midiNote = 127 - note;
            bool isBlack = juce::MidiMessage::isMidiNoteBlack(midiNote);
            
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
                double pixelsPerSecond = 100.0 * timeZoom;
                
                for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                    auto* event = currentClip->getSequence().getEventPointer(i);
                    if (event->message.isNoteOn()) {
                        double noteStart = event->message.getTimeStamp();
                        float x = keyWidth + static_cast<float>((noteStart / sampleRate) * pixelsPerSecond);
                        
                        float vel = event->message.getVelocity() / 127.0f;
                        float vh = vel * (velocityLaneHeight - 4);
                        float vy = contentHeight + velocityLaneHeight - vh;
                        
                        juce::Colour stalkColor = selectedEventIndices.contains(i) ? DesignSystem::Colors::PrimaryAction : DesignSystem::Colors::TextSecondary.withAlpha(0.6f);
                        g.setColour(stalkColor);
                        
                        // Thin stalk (1px wide)
                        g.drawLine(x, vy, x, contentHeight + velocityLaneHeight, 1.0f);
                        
                        // Top handle (small circle)
                        float handleRadius = 3.0f;
                        g.setColour(selectedEventIndices.contains(i) ? juce::Colours::white : stalkColor);
                        g.fillEllipse(x - handleRadius, vy - handleRadius, handleRadius * 2, handleRadius * 2);
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
            
            double clipGlobalStart = currentClip->startSample.get();
            double clipGlobalEnd = clipGlobalStart + currentClip->lengthSamples.get();
            
            if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
                double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
                double pixelsPerSecond = 100.0 * timeZoom;
                float px = keyWidth + static_cast<float>(timeIntoClip * pixelsPerSecond);
                
                g.setColour(juce::Colours::white);
                g.drawVerticalLine(static_cast<int>(px), 0.0f, static_cast<float>(getHeight()));
            }
        }
    }
}

void PianoRollContent::mouseDown(const juce::MouseEvent& event) {
    grabKeyboardFocus();
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
        double pixelsPerSecond = 100.0 * timeZoom;
        
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
            
            double clipSamples = currentClip->lengthSamples.get();
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double secondsPer16th = secondsPerBeat / 4.0;
            double samplesPer16th = secondsPer16th * sampleRate;
            int gridWidth = getWidth() - keyWidth;
            double pixelsPerSecond = 100.0 * timeZoom;
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
                    selectedEventIndices.remove(foundNoteIndex);
                    engine.getTimelineProject().notifyClipModified();
                    repaint();
                } else {
                    // Prepare to drag or resize
                    draggedEventIndex = foundNoteIndex;
                    if (!selectedEventIndices.contains(foundNoteIndex)) {
                        if (!event.mods.isShiftDown()) selectedEventIndices.clear();
                        selectedEventIndices.add(foundNoteIndex);
                    }
                    dragStartMouseX = event.getPosition().x;
                    dragStartMouseY = event.getPosition().y;
                    dragStartNoteTime = foundNoteStart;
                    dragStartNoteLength = foundNoteLength;
                    dragStartNoteNumber = noteNumber;
                    hasDuplicatedForDrag = false;
                    
                    double noteRightEdgeTime = foundNoteStart + foundNoteLength;
                    double noteRightEdgeX = keyWidth + (noteRightEdgeTime / sampleRate) * pixelsPerSecond;
                    if (event.getPosition().x >= noteRightEdgeX - 5.0) {
                        isResizing = true;
                    }
                    repaint(); // To immediately highlight the selected note
                }
                return;
            }
            
            if (event.mods.isPopupMenu()) {
                // Ignore right clicks on empty space
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
            return;
        }
    }
}

void PianoRollContent::mouseDoubleClick(const juce::MouseEvent& event) {
    if (!currentClip) return;
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        int vx = vp->getViewPositionX();
        if (event.getPosition().x > vx + keyWidth) {
            int x = event.getPosition().x - keyWidth;
            int y = event.getPosition().y;
            
            int row = y / keyHeight;
            if (row < 0 || row > 127) return;
            int noteNumber = 127 - row;
            
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
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
            double pixelsPerSecond = 100.0 * timeZoom;
            
            double timeInClip = (static_cast<double>(x) / pixelsPerSecond) * sampleRate;
            if (timeInClip < 0.0) timeInClip = 0.0;
            
            // Hit test for existing note first to delete it
            int foundNoteIndex = -1;
            for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                auto* evt = currentClip->getSequence().getEventPointer(i);
                if (evt->message.isNoteOn() && evt->message.getNoteNumber() == noteNumber) {
                    double noteStart = evt->message.getTimeStamp();
                    double noteLength = sampleRate * 0.25;
                    if (evt->noteOffObject != nullptr) {
                        noteLength = evt->noteOffObject->message.getTimeStamp() - noteStart;
                    }
                    if (timeInClip >= noteStart && timeInClip <= noteStart + noteLength) {
                        foundNoteIndex = i;
                        break;
                    }
                }
            }
            if (foundNoteIndex != -1) {
                currentClip->getSequence().deleteEvent(foundNoteIndex, true);
                currentClip->getSequence().updateMatchedPairs();
                selectedEventIndices.remove(foundNoteIndex);
                engine.getTimelineProject().notifyClipModified();
                repaint();
                return;
            }
            
            double snappedTime = std::floor(timeInClip / snapSamples) * snapSamples;
            if (currentSnap == Snap::Off) snappedTime = timeInClip;
            
            juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
            noteOn.setTimeStamp(snappedTime);
            
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, (juce::uint8)0);
            noteOff.setTimeStamp(snappedTime + (snapSamples > 0 ? snapSamples : sampleRate * 0.25));
            
            currentClip->getSequence().addEvent(noteOn);
            currentClip->getSequence().addEvent(noteOff);
            currentClip->getSequence().updateMatchedPairs();
            
            engine.getTimelineProject().notifyClipModified();
            repaint();
        }
    }
}

void PianoRollContent::mouseMove(const juce::MouseEvent& event) {
    if (!currentClip) return;
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        int vx = vp->getViewPositionX();
        if (event.x > vx + keyWidth) {
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            double pixelsPerSecond = 100.0 * timeZoom;
            
            bool hoveringEdge = false;
            auto& seq = currentClip->getSequence();
            for (int i = 0; i < seq.getNumEvents(); ++i) {
                auto* evt = seq.getEventPointer(i);
                if (evt && evt->message.isNoteOn()) {
                    double noteStart = evt->message.getTimeStamp();
                    double noteLength = 0.25 * sampleRate;
                    if (evt->noteOffObject) noteLength = evt->noteOffObject->message.getTimeStamp() - noteStart;
                    
                    double noteRightEdgeTime = noteStart + noteLength;
                    double noteRightEdgeX = keyWidth + (noteRightEdgeTime / sampleRate) * pixelsPerSecond;
                    
                    int noteNum = evt->message.getNoteNumber();
                    int row = 127 - noteNum;
                    float y = static_cast<float>(row * keyHeight);
                    
                    if (event.y >= y && event.y <= y + keyHeight) {
                        if (std::abs(event.x - noteRightEdgeX) <= 5.0) {
                            hoveringEdge = true;
                            break;
                        }
                    }
                }
            }
            if (hoveringEdge) {
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            } else {
                setMouseCursor(juce::MouseCursor::NormalCursor);
            }
            return;
        }
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void PianoRollContent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (event.mods.isCommandDown() || event.mods.isCtrlDown()) {
        double factor = (wheel.deltaY > 0) ? 1.1 : 0.9;
        
        double newZoom = juce::jlimit(0.1, 10.0, timeZoom * factor);
        if (newZoom == timeZoom) return;
        
        if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
            double ratio = newZoom / timeZoom;
            int mouseX = event.x;
            int relativeToGrid = mouseX - keyWidth;
            int oldVy = vp->getViewPositionY();
            
            timeZoom = newZoom;
            setBounds(0, 0, juce::jmax(1000, getDesiredWidth()), getDesiredHeight());
            
            int newRelativeToGrid = static_cast<int>(relativeToGrid * ratio);
            int newMouseX = keyWidth + newRelativeToGrid;
            
            vp->setViewPosition(vp->getViewPositionX() + (newMouseX - mouseX), oldVy);
        }
        repaint();
    } else {
        juce::Component::mouseWheelMove(event, wheel);
    }
}

void PianoRollContent::mouseMagnify(const juce::MouseEvent& event, float scaleFactor) {
    double newZoom = juce::jlimit(0.1, 10.0, timeZoom * scaleFactor);
    if (newZoom == timeZoom) return;
    
    if (auto* vp = findParentComponentOfClass<juce::Viewport>()) {
        double ratio = newZoom / timeZoom;
        int mouseX = event.x;
        int relativeToGrid = mouseX - keyWidth;
        int oldVy = vp->getViewPositionY();
        
        timeZoom = newZoom;
        setBounds(0, 0, juce::jmax(1000, getDesiredWidth()), getDesiredHeight());
        
        int newRelativeToGrid = static_cast<int>(relativeToGrid * ratio);
        int newMouseX = keyWidth + newRelativeToGrid;
        
        vp->setViewPosition(vp->getViewPositionX() + (newMouseX - mouseX), oldVy);
    }
    repaint();
}

bool PianoRollContent::keyPressed(const juce::KeyPress& key) {
    if (!currentClip) return false;
    
    if (key.isKeyCode('a') || key.isKeyCode('A')) {
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()) {
            selectedEventIndices.clear();
            auto& seq = currentClip->getSequence();
            for (int i = 0; i < seq.getNumEvents(); ++i) {
                if (seq.getEventPointer(i)->message.isNoteOn()) {
                    selectedEventIndices.addIfNotAlreadyThere(i);
                }
            }
            repaint();
            return true;
        }
    }
    
    if (key.isKeyCode(juce::KeyPress::upKey) || key.isKeyCode(juce::KeyPress::downKey)) {
        if (selectedEventIndices.isEmpty()) return false;
        
        int delta = key.getModifiers().isShiftDown() ? 12 : 1;
        if (key.isKeyCode(juce::KeyPress::downKey)) delta = -delta;
        
        auto& seq = currentClip->getSequence();
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt && evt->message.isNoteOn()) {
                int newNote = juce::jlimit(0, 127, evt->message.getNoteNumber() + delta);
                evt->message.setNoteNumber(newNote);
                if (evt->noteOffObject) evt->noteOffObject->message.setNoteNumber(newNote);
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
        repaint();
        return true;
    }
    
    if (key.isKeyCode(juce::KeyPress::leftKey) || key.isKeyCode(juce::KeyPress::rightKey)) {
        if (selectedEventIndices.isEmpty()) return false;
        
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
        else if (currentSnap == Snap::Off) snapBeats = 0.0625; // Nudge by 1/64th if snap is off
        
        double snapSamples = snapBeats * secondsPerBeat * sampleRate;
        double deltaSamples = key.isKeyCode(juce::KeyPress::leftKey) ? -snapSamples : snapSamples;
        
        auto& seq = currentClip->getSequence();
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt && evt->message.isNoteOn()) {
                double newTime = juce::jmax(0.0, evt->message.getTimeStamp() + deltaSamples);
                evt->message.setTimeStamp(newTime);
                if (evt->noteOffObject) {
                    double offTime = juce::jmax(newTime + 10.0, evt->noteOffObject->message.getTimeStamp() + deltaSamples);
                    evt->noteOffObject->message.setTimeStamp(offTime);
                }
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
        repaint();
        return true;
    }
    
    if (key.isKeyCode(juce::KeyPress::backspaceKey) || key.isKeyCode(juce::KeyPress::deleteKey)) {
        if (selectedEventIndices.isEmpty()) return false;
        
        auto& seq = currentClip->getSequence();
        juce::Array<juce::MidiMessageSequence::MidiEventHolder*> toDelete;
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt) {
                toDelete.add(evt);
                if (evt->noteOffObject) toDelete.add(evt->noteOffObject);
            }
        }
        for (auto* evt : toDelete) {
            seq.deleteEvent(seq.getIndexOf(evt), true);
        }
        selectedEventIndices.clear();
        engine.getTimelineProject().notifyClipModified();
        repaint();
        return true;
    }
    
    return false;
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
        double pixelsPerSecond = 100.0 * timeZoom;
        
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
    
    if (event.mods.isAltDown() && !hasDuplicatedForDrag && !isResizing && !isDraggingVelocity) {
        hasDuplicatedForDrag = true;
        juce::Array<juce::MidiMessageSequence::MidiEventHolder*> newNoteOns;
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt && evt->message.isNoteOn()) {
                auto* newEvt = seq.addEvent(evt->message);
                newNoteOns.add(newEvt);
                if (evt->noteOffObject) {
                    seq.addEvent(evt->noteOffObject->message);
                }
            }
        }
        seq.updateMatchedPairs();
        selectedEventIndices.clear();
        for (auto* newEvt : newNoteOns) {
            selectedEventIndices.add(seq.getIndexOf(newEvt));
        }
        if (!selectedEventIndices.isEmpty()) {
            draggedEventIndex = selectedEventIndices.getFirst();
        }
    }
    
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
    double pixelsPerSecond = 100.0 * timeZoom;
    
    double xDeltaSeconds = (event.x - dragStartMouseX) / pixelsPerSecond;
    double newTimeSeconds = (dragStartNoteTime / sampleRate) + xDeltaSeconds;
    
    if (currentSnap != Snap::Off && !event.mods.isCommandDown() && !event.mods.isCtrlDown()) {
        newTimeSeconds = std::round(newTimeSeconds / snapSeconds) * snapSeconds;
    }
    
    if (newTimeSeconds < 0) newTimeSeconds = 0;
    double newTimeSamples = newTimeSeconds * sampleRate;
    
    if (isResizing) {
        double newLengthSeconds = (dragStartNoteLength / sampleRate) + xDeltaSeconds;
        if (currentSnap != Snap::Off && !event.mods.isCommandDown() && !event.mods.isCtrlDown()) {
            newLengthSeconds = std::round(newLengthSeconds / snapSeconds) * snapSeconds;
        }
        if (newLengthSeconds < snapSeconds && currentSnap != Snap::Off) newLengthSeconds = snapSeconds;
        if (newLengthSeconds < 0.01) newLengthSeconds = 0.01;
        
        double newLengthSamples = newLengthSeconds * sampleRate;
        double deltaLengthSamples = newLengthSamples - dragStartNoteLength;
        
        for (int idx : selectedEventIndices) {
            auto* evt = seq.getEventPointer(idx);
            if (evt && evt->message.isNoteOn() && evt->noteOffObject) {
                double currentStart = evt->message.getTimeStamp();
                double currentLength = evt->noteOffObject->message.getTimeStamp() - currentStart;
                double targetLength = juce::jmax(10.0, currentLength + deltaLengthSamples);
                evt->noteOffObject->message.setTimeStamp(currentStart + targetLength);
            }
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
        int vy = viewport.getViewPositionY();
        content.setVelocityVisible(velocityToggle.getToggleState());
        content.setBounds(0, 0, juce::jmax(1000, content.getDesiredWidth()), content.getDesiredHeight());
        resized();
        viewport.setViewPosition(viewport.getViewPositionX(), vy);
    };
    toolbar.addAndMakeVisible(velocityToggle);
    
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
}

PianoRollComponent::~PianoRollComponent() = default;

void PianoRollComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    content.setMidiClip(clip);
    int centerNote = 60; // Default C3
    if (clip && clip->getSequence().getNumEvents() > 0) {
        long sum = 0;
        int count = 0;
        for (int i = 0; i < clip->getSequence().getNumEvents(); ++i) {
            auto* evt = clip->getSequence().getEventPointer(i);
            if (evt->message.isNoteOn()) {
                sum += evt->message.getNoteNumber();
                count++;
            }
        }
        if (count > 0) {
            centerNote = static_cast<int>(sum / count);
        }
    }
    int centerRow = 127 - centerNote;
    viewport.setViewPosition(0, (centerRow * 16) - (viewport.getHeight() / 2));
}

void PianoRollComponent::resized() {
    auto bounds = getLocalBounds();
    auto toolBounds = bounds.removeFromTop(30);
    toolbar.setBounds(toolBounds);
    
    int x = 10;
    snapBox.setBounds(x, 3, 80, 24); x += 90;
    velocityToggle.setBounds(x, 3, 80, 24);
    
    viewport.setBounds(bounds);
    content.setBounds(0, 0, juce::jmax(1000, content.getDesiredWidth()), content.getDesiredHeight());
}

void PianoRollComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Premium dark background overlaid with clip tint
    juce::Colour bgColor = DesignSystem::Colors::ComponentBackground;
    juce::Colour clipColor = juce::Colour(0xff0a84ff);
    juce::String clipName = "MIDI Clip";
    
    if (auto currentClip = getCurrentClip()) {
        int index = currentClip->colorIndex.get();
        if (index >= 0) {
            float hue = std::fmod(index * 0.381966f, 1.0f);
            clipColor = juce::Colour::fromHSV(hue, 0.6f, 0.95f, 1.0f);
        }
        bgColor = clipColor.withAlpha(0.2f).overlaidWith(DesignSystem::Colors::ComponentBackground);
        clipName = currentClip->name.get();
    }
    
    g.setColour(bgColor);
    g.fillRect(toolbar.getBounds());
    
    // Draw clip name in toolbar
    bool isDark = bgColor.getPerceivedBrightness() < 0.5f;
    g.setColour(isDark ? juce::Colours::white : juce::Colours::black);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f).boldened());
    
    // Position text to the right of the velocity toggle
    int textX = velocityToggle.getRight() + 10;
    g.drawText(clipName, toolbar.getBounds().withTrimmedLeft(textX), juce::Justification::centredLeft, true);
    
    g.setColour(DesignSystem::Colors::Divider);
    g.drawHorizontalLine(toolbar.getBottom() - 1, 0, static_cast<float>(getWidth()));
}

} // namespace Nimbus::DetailView
