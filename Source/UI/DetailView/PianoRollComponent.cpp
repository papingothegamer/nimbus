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
                        
                        for (int j = i + 1; j < currentClip->getSequence().getNumEvents(); ++j) {
                            auto* offEvent = currentClip->getSequence().getEventPointer(j);
                            if (offEvent->message.isNoteOff() && offEvent->message.getNoteNumber() == event->message.getNoteNumber()) {
                                noteLength = offEvent->message.getTimeStamp() - noteStart;
                                break;
                            }
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
            // Delete if exists
            bool deletedNote = false;
            for (int i = 0; i < currentClip->getSequence().getNumEvents(); ++i) {
                auto* evt = currentClip->getSequence().getEventPointer(i);
                if (evt->message.isNoteOn() && evt->message.getNoteNumber() == noteNumber) {
                    double noteStart = evt->message.getTimeStamp();
                    double noteLength = samplesPer16th;
                    for (int j = i + 1; j < currentClip->getSequence().getNumEvents(); ++j) {
                        auto* offEvt = currentClip->getSequence().getEventPointer(j);
                        if (offEvt->message.isNoteOff() && offEvt->message.getNoteNumber() == noteNumber) {
                            noteLength = offEvt->message.getTimeStamp() - noteStart;
                            break;
                        }
                    }
                    if (timeInClip >= noteStart && timeInClip <= noteStart + noteLength) {
                        currentClip->getSequence().deleteEvent(i, false);
                        for (int j = i; j < currentClip->getSequence().getNumEvents(); ++j) {
                            auto* offEvt = currentClip->getSequence().getEventPointer(j);
                            if (offEvt->message.isNoteOff() && offEvt->message.getNoteNumber() == noteNumber) {
                                currentClip->getSequence().deleteEvent(j, false);
                                break;
                            }
                        }
                        currentClip->getSequence().updateMatchedPairs();
                        deletedNote = true;
                        engine.getTimelineProject().notifyClipModified();
                        repaint();
                        break;
                    }
                }
            }
            if (deletedNote) return;
            
            // Add snapped note
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
        }
    }
}

void PianoRollContent::mouseDrag(const juce::MouseEvent& event) {}

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
