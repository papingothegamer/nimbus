#include "ComputerMidiController.h"
#include "../Core/NimbusEngine.h"

namespace Nimbus {

ComputerMidiController::ComputerMidiController(NimbusEngine& e) : engine(e) {
    // Run a high-frequency timer for smooth modulation and key polling
    startTimerHz(60);
}

ComputerMidiController::~ComputerMidiController() {
    stopTimer();
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

bool ComputerMidiController::isKeyCurrentlyDown(int keyCode) {
    return juce::KeyPress::isKeyCurrentlyDown(keyCode) || 
           juce::KeyPress::isKeyCurrentlyDown(std::tolower(keyCode));
}

void ComputerMidiController::timerCallback() {
    if (!enabled) return;

    // Do not capture keypresses if the app is not in the foreground
    if (!juce::Process::isForegroundProcess()) return;
    
    // Do not capture keypresses if the user is typing in a text editor
    if (juce::Component::getCurrentlyFocusedComponent() != nullptr &&
        dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent()) != nullptr) {
        return;
    }

    // --- Octave / Velocity Controls (Edge Detection) ---
    bool zDown = isKeyCurrentlyDown('Z');
    if (zDown && !zWasDown) currentOctave = juce::jmax(0, currentOctave - 1);
    zWasDown = zDown;

    bool xDown = isKeyCurrentlyDown('X');
    if (xDown && !xWasDown) currentOctave = juce::jmin(8, currentOctave + 1);
    xWasDown = xDown;

    bool cDown = isKeyCurrentlyDown('C');
    if (cDown && !cWasDown) currentVelocity = juce::jmax(1, currentVelocity - 20);
    cWasDown = cDown;

    bool vDown = isKeyCurrentlyDown('V');
    if (vDown && !vWasDown) currentVelocity = juce::jmin(127, currentVelocity + 20);
    vWasDown = vDown;

    // --- Note Input ---
    for (int kc : mappedKeys) {
        bool isDown = isKeyCurrentlyDown(kc);
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
            }
        } else if (!isDown && wasDown) {
            auto msg = juce::MidiMessage::noteOff(1, activeKeyCodesToNotes[kc], (juce::uint8)0);
            msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            engine.getAudioDeviceManager().injectMidiMessage(msg);
            activeKeyCodesToNotes.erase(kc);
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
    }

    // --- Modulation ---
    leftArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey);
    rightArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey);
    
    bool modChanged = false;
    if (rightArrowDown) {
        currentModulation = juce::jmin(127.0f, currentModulation + 2.5f);
        modChanged = true;
    } else if (leftArrowDown) {
        currentModulation = juce::jmax(0.0f, currentModulation - 2.5f);
        modChanged = true;
    }

    if (modChanged) {
        sendModulation(juce::roundToInt(currentModulation));
    }
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
