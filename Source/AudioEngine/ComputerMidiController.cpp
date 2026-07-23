#include "ComputerMidiController.h"
#include "../Core/NimbusEngine.h"

namespace Nimbus {

ComputerMidiController::ComputerMidiController(NimbusEngine& e) : engine(e) {
    // Run a high-frequency timer for smooth modulation
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
    // Lower row: A=C, W=C#, S=D, E=D#, D=E, F=F, T=F#, G=G, Y=G#, H=A, U=A#, J=B, K=C
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

bool ComputerMidiController::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/) {
    if (!enabled) return false;

    int keyCode = std::toupper(key.getKeyCode());

    // Octave controls
    if (keyCode == 'Z') {
        currentOctave = juce::jmax(0, currentOctave - 1);
        return true;
    }
    if (keyCode == 'X') {
        currentOctave = juce::jmin(8, currentOctave + 1);
        return true;
    }

    // Velocity controls
    if (keyCode == 'C') {
        currentVelocity = juce::jmax(1, currentVelocity - 20);
        return true;
    }
    if (keyCode == 'V') {
        currentVelocity = juce::jmin(127, currentVelocity + 20);
        return true;
    }

    // Note input
    int noteOffset = getNoteFromKeyCode(keyCode);
    if (noteOffset != -1) {
        int midiNote = (currentOctave * 12) + noteOffset;
        midiNote = juce::jlimit(0, 127, midiNote);
        
        if (activeKeyCodesToNotes.find(keyCode) == activeKeyCodesToNotes.end()) {
            activeKeyCodesToNotes[keyCode] = midiNote;
            auto msg = juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)currentVelocity);
            msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            engine.getAudioDeviceManager().injectMidiMessage(msg);
        }
        return true;
    }

    // Arrow keys are handled by keyStateChanged to support holding down
    if (key.isKeyCode(juce::KeyPress::upKey) || 
        key.isKeyCode(juce::KeyPress::downKey) || 
        key.isKeyCode(juce::KeyPress::leftKey) || 
        key.isKeyCode(juce::KeyPress::rightKey)) {
        return true;
    }

    return false;
}

bool ComputerMidiController::keyStateChanged(bool isKeyDown, juce::Component* /*originatingComponent*/) {
    if (!enabled) return false;

    bool handled = false;

    // Pitch Bend keys
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
    }

    // Modulation keys
    leftArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::leftKey);
    rightArrowDown = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::rightKey);
    
    if (leftArrowDown || rightArrowDown) handled = true;

    // Handle Note Offs
    for (auto it = activeKeyCodesToNotes.begin(); it != activeKeyCodesToNotes.end(); ) {
        int kc = it->first;
        // isKeyCurrentlyDown expects lower case for letters? No, it expects the exact keycode.
        // For letters, it might be safer to just check both lower and upper.
        bool isDown = juce::KeyPress::isKeyCurrentlyDown(kc) || juce::KeyPress::isKeyCurrentlyDown(std::tolower(kc));
        if (!isDown) {
            auto msg = juce::MidiMessage::noteOff(1, it->second, (juce::uint8)0);
            msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            engine.getAudioDeviceManager().injectMidiMessage(msg);
            it = activeKeyCodesToNotes.erase(it);
            handled = true;
        } else {
            ++it;
        }
    }

    return handled;
}

void ComputerMidiController::timerCallback() {
    if (!enabled) return;

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
