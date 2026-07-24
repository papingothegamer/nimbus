#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>

namespace Nimbus {

class NimbusEngine;

class ComputerMidiController : public juce::Timer {
public:
    ComputerMidiController(NimbusEngine& engine);
    ~ComputerMidiController() override;

    void setEnabled(bool shouldBeEnabled);
    bool isEnabled() const { return enabled; }

    void timerCallback() override;

private:
    NimbusEngine& engine;
    bool enabled = false;

    int currentOctave = 4;
    int currentVelocity = 100;
    
    // Modulation tracking
    bool leftArrowDown = false;
    bool rightArrowDown = false;
    float currentModulation = 0.0f; // 0.0 to 127.0
    
    // Pitch bend tracking
    bool upArrowDown = false;
    bool downArrowDown = false;

    // Map of currently pressed characters to MIDI notes
    std::map<int, int> activeKeyCodesToNotes;

    bool zWasDown = false;
    bool xWasDown = false;
    bool cWasDown = false;
    bool vWasDown = false;

    int getNoteFromKeyCode(int keyCode);
    void sendPitchBend(int value);
    void sendModulation(int value);
    bool isKeyCurrentlyDown(int keyCode);
    
    const std::vector<int> mappedKeys = {
        'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K', 'L', 'P', ';', '\''
    };
};

} // namespace Nimbus
