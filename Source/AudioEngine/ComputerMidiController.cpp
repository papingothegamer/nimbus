#include "ComputerMidiController.h"
#include "../Core/NimbusEngine.h"

namespace Nimbus {

ComputerMidiController::ComputerMidiController(NimbusEngine& e) : engine(e) {
}

ComputerMidiController::~ComputerMidiController() {
}

void ComputerMidiController::setEnabled(bool shouldBeEnabled) {
    enabled = shouldBeEnabled;
    if (!enabled) {
        // Send Note Off for all active notes
        for (auto const& [keyCode, note] : activeKeyCodesToNotes) {
            auto msg = juce::MidiMessage::noteOff(1, note, (juce::uint8)0);
            msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            engine.getAudioDeviceManager().injectMidiMessage(msg);
        }
        activeKeyCodesToNotes.clear();
        
        // Reset pitch bend
        sendPitchBend(8192);
        upArrowDown = false;
        downArrowDown = false;
        leftArrowDown = false;
        rightArrowDown = false;
    }
}

int ComputerMidiController::getNoteFromKeyCode(int keyCode) {
    // Ableton standard mapping
    switch (keyCode) {
        case 'A': return 0;
        case 'W': return 1;
        case 'S': return 2;
        case 'E': return 3;
        case 'D': return 4;
        case 'F': return 5;
        case 'T': return 6;
        case 'G': return 7;
        case 'Y': return 8;
        case 'H': return 9;
        case 'U': return 10;
        case 'J': return 11;
        case 'K': return 12;
        case 'L': return 14; // D
        case 'P': return 15; // D#
        case ';': return 16; // E
        case '\'': return 17; // F
        default: return -1;
    }
}

bool ComputerMidiController::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) {
    return false; // We handle everything in keyStateChanged
}

bool ComputerMidiController::keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) {
    if (!enabled) return false;

    // Do not capture keypresses if the user is typing in a text editor
    if (dynamic_cast<juce::TextEditor*>(originatingComponent) != nullptr) {
        return false;
    }

    bool handled = false;

    // --- Octave / Velocity Controls (Edge Detection) ---
    bool zDown = juce::KeyPress::isKeyCurrentlyDown('Z') || juce::KeyPress::isKeyCurrentlyDown('z');
    if (zDown && !zWasDown) currentOctave = juce::jmax(0, currentOctave - 1);
    zWasDown = zDown;
    if (zDown) handled = true;

    bool xDown = juce::KeyPress::isKeyCurrentlyDown('X') || juce::KeyPress::isKeyCurrentlyDown('x');
    if (xDown && !xWasDown) currentOctave = juce::jmin(8, currentOctave + 1);
    xWasDown = xDown;
    if (xDown) handled = true;

    bool cDown = juce::KeyPress::isKeyCurrentlyDown('C') || juce::KeyPress::isKeyCurrentlyDown('c');
    if (cDown && !cWasDown) currentVelocity = juce::jmax(1, currentVelocity - 20);
    cWasDown = cDown;
    if (cDown) handled = true;

    bool vDown = juce::KeyPress::isKeyCurrentlyDown('V') || juce::KeyPress::isKeyCurrentlyDown('v');
    if (vDown && !vWasDown) currentVelocity = juce::jmin(127, currentVelocity + 20);
    vWasDown = vDown;
    if (vDown) handled = true;

    // --- Note Input ---
    for (int kc : mappedKeys) {
        bool isDown = juce::KeyPress::isKeyCurrentlyDown(kc) || juce::KeyPress::isKeyCurrentlyDown(std::tolower(kc));
        bool wasDown = (activeKeyCodesToNotes.find(kc) != activeKeyCodesToNotes.end());

        if (isDown && !wasDown) {
            int noteOffset = getNoteFromKeyCode(kc);
            if (noteOffset != -1) {
                int midiNote = (currentOctave * 12) + noteOffset;
                midiNote = juce::jlimit(0, 127, midiNote);
                
                activeKeyCodesToNotes[kc] = midiNote;
                auto msg = juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)currentVelocity);
                msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
                engine.getAudioDeviceManager().injectMidiMessage(msg);
                handled = true;
            }
        } else if (!isDown && wasDown) {
            auto msg = juce::MidiMessage::noteOff(1, activeKeyCodesToNotes[kc], (juce::uint8)0);
            msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            engine.getAudioDeviceManager().injectMidiMessage(msg);
            activeKeyCodesToNotes.erase(kc);
            handled = true;
        } else if (isDown) {
            handled = true; // Consumed
        }
    }

    // --- Pitch Bend ---
    bool newUp = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::upKey);
    bool newDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::downKey);
    
    if (newUp != upArrowDown || newDown != downArrowDown) {
        upArrowDown = newUp;
        downArrowDown = newDown;
        
        int pitchBendVal = 8192; // Center
        if (upArrowDown && !downArrowDown) pitchBendVal = 16383; // Max
        else if (downArrowDown && !upArrowDown) pitchBendVal = 0; // Min
        
        sendPitchBend(pitchBendVal);
        handled = true;
    } else if (newUp || newDown) {
        handled = true;
    }

    // --- Modulation ---
    leftArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey);
    rightArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey);
    
    bool modChanged = false;
    if (rightArrowDown) {
        currentModulation = juce::jmin(127.0f, currentModulation + 2.5f);
        modChanged = true;
        handled = true;
    } else if (leftArrowDown) {
        currentModulation = juce::jmax(0.0f, currentModulation - 2.5f);
        modChanged = true;
        handled = true;
    }

    if (modChanged) {
        sendModulation(juce::roundToInt(currentModulation));
    }

    return handled;
}

void ComputerMidiController::sendPitchBend(int value) {
    auto msg = juce::MidiMessage::pitchWheel(1, value);
    msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    engine.getAudioDeviceManager().injectMidiMessage(msg);
}

void ComputerMidiController::sendModulation(int value) {
    auto msg = juce::MidiMessage::controllerEvent(1, 1, value); // CC1
    msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    engine.getAudioDeviceManager().injectMidiMessage(msg);
}

} // namespace Nimbus
