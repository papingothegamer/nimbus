#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>

namespace Nimbus {

class NimbusEngine;

class ComputerMidiController : public juce::KeyListener, public juce::Timer {
public:
    ComputerMidiController(NimbusEngine& engine);
    ~ComputerMidiController() override;

    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

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

    int getNoteFromKeyCode(int keyCode);
    void sendPitchBend(int value);
    void sendModulation(int value);
};

} // namespace Nimbus
