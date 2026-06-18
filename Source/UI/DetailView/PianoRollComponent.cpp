#include "PianoRollComponent.h"
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"

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
            
            int gridWidth = getWidth() - keyWidth;
            int num16ths = static_cast<int>(clipSeconds / secondsPer16th);
            
            g.setColour(DesignSystem::Colors::Divider.withAlpha(0.3f));
            for (int i = 0; i <= num16ths; ++i) {
                float x = keyWidth + static_cast<float>((i * secondsPer16th / clipSeconds) * gridWidth);
                g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
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
                        
                        float x = keyWidth + static_cast<float>((noteStart / clipSamples) * gridWidth);
                        float w = static_cast<float>((noteLength / clipSamples) * gridWidth);
                        
                        int noteNumber = event->message.getNoteNumber();
                        int row = 127 - noteNumber;
                        int y = row * keyHeight;
                        
                        g.setColour(DesignSystem::Colors::PrimaryAction);
                        g.fillRect(x, static_cast<float>(y), w, static_cast<float>(keyHeight));
                        
                        g.setColour(juce::Colours::black.withAlpha(0.8f));
                        g.drawRect(x, static_cast<float>(y), w, static_cast<float>(keyHeight), 1.0f);
                    }
                }
            }
        }
        
        // Draw keyboard on the left, sticky!
        for (int note = 0; note < totalKeys; ++note) {
            int y = note * keyHeight;
            int midiNote = 127 - note;
            bool isBlack = juce::MidiMessage::isMidiNoteBlack(midiNote);
            
            g.setColour(isBlack ? juce::Colours::black : juce::Colours::white);
            g.fillRect(vx, y, keyWidth, keyHeight);
            
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.drawRect(vx, y, keyWidth, keyHeight, 1);
            
            // Draw C note labels
            if (midiNote % 12 == 0 && !isBlack) {
                g.setColour(juce::Colours::black);
                g.drawText("C" + juce::String((midiNote / 12) - 2), vx + 2, y, keyWidth - 4, keyHeight, juce::Justification::centredRight, false);
            }
            
            // Draw horizontal grid lines
            g.setColour(DesignSystem::Colors::Divider.withAlpha(isBlack ? 0.3f : 0.6f));
            g.drawHorizontalLine(y + keyHeight, vx + keyWidth, static_cast<float>(getWidth()));
        }
    }
}

void PianoRollContent::mouseDown(const juce::MouseEvent& event) {
    if (!currentClip) return;
    
    draggedEventIndex = -1;
    isResizing = false;
    
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
            double timeInClip = (static_cast<double>(x) / gridWidth) * clipSamples;
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
                    double noteRightEdgeX = (noteRightEdgeTime / clipSamples) * gridWidth;
                    if (x >= noteRightEdgeX - 5.0) {
                        isResizing = true;
                    }
                }
                return;
            }
            
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
                
                engine.getTimelineProject().notifyClipModified();
                repaint();
                
                // Set as dragged so user can resize immediately
                for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                    auto* evt = currentClip->getSequence().getEventPointer(i);
                    if (evt->message.isNoteOn() && evt->message.getNoteNumber() == noteNumber && evt->message.getTimeStamp() == snappedTime) {
                        draggedEventIndex = i;
                        dragStartMouseX = event.getPosition().x;
                        dragStartMouseY = event.getPosition().y;
                        dragStartNoteTime = snappedTime;
                        dragStartNoteLength = samplesPer16th;
                        dragStartNoteNumber = noteNumber;
                        isResizing = false;
                        break;
                    }
                }
            }
        }
    }
}

void PianoRollContent::mouseDrag(const juce::MouseEvent& event) {
    if (!currentClip || draggedEventIndex == -1) return;
    
    auto* evt = currentClip->getSequence().getEventPointer(draggedEventIndex);
    if (!evt || !evt->message.isNoteOn()) return;
    
    double clipSamples = currentClip->getLengthSamples();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    double secondsPer16th = secondsPerBeat / 4.0;
    double samplesPer16th = secondsPer16th * sampleRate;
    
    int gridWidth = getWidth() - keyWidth;
    
    int dx = event.getPosition().x - dragStartMouseX;
    int dy = event.getPosition().y - dragStartMouseY;
    
    double dtSamples = (static_cast<double>(dx) / gridWidth) * clipSamples;
    
    if (isResizing) {
        double newLength = dragStartNoteLength + dtSamples;
        newLength = std::max(newLength, samplesPer16th); // Min length 1/16th
        double snappedLength = std::round(newLength / samplesPer16th) * samplesPer16th;
        
        if (evt->noteOffObject) {
            evt->noteOffObject->message.setTimeStamp(dragStartNoteTime + snappedLength);
            currentClip->getSequence().updateMatchedPairs();
            engine.getTimelineProject().notifyClipModified();
            repaint();
        }
    } else {
        double newTime = dragStartNoteTime + dtSamples;
        double snappedTime = std::round(newTime / samplesPer16th) * samplesPer16th;
        snappedTime = std::max(0.0, snappedTime);
        
        int dRow = std::round(static_cast<float>(dy) / keyHeight);
        int newNoteNumber = dragStartNoteNumber - dRow;
        newNoteNumber = juce::jlimit(0, 127, newNoteNumber);
        
        if (snappedTime != evt->message.getTimeStamp() || newNoteNumber != evt->message.getNoteNumber()) {
            evt->message.setTimeStamp(snappedTime);
            evt->message.setNoteNumber(newNoteNumber);
            if (evt->noteOffObject) {
                evt->noteOffObject->message.setTimeStamp(snappedTime + dragStartNoteLength);
                evt->noteOffObject->message.setNoteNumber(newNoteNumber);
            }
            // re-sorting might change indices, but for dragging it might glitch if draggedEventIndex changes.
            // For now, sorting happens in updateMatchedPairs which sorts the events.
            currentClip->getSequence().updateMatchedPairs();
            
            // update draggedEventIndex to the new index after sort
            for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                auto* e = currentClip->getSequence().getEventPointer(i);
                if (e->message.isNoteOn() && e->message.getTimeStamp() == snappedTime && e->message.getNoteNumber() == newNoteNumber) {
                    draggedEventIndex = i;
                    break;
                }
            }
            
            engine.getTimelineProject().notifyClipModified();
            repaint();
        }
    }
}

// ==============================================================================
PianoRollComponent::PianoRollComponent(NimbusEngine& e) : content(e) {
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
    viewport.setBounds(getLocalBounds());
    content.setBounds(0, 0, juce::jmax(1000, getWidth()), 128 * 16);
}

} // namespace Nimbus::DetailView
