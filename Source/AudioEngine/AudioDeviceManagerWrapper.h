#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "Transport.h"

namespace Nimbus {

/**
 * Wraps juce::AudioDeviceManager and handles MIDI inputs.
 */
class AudioDeviceManagerWrapper : public juce::MidiInputCallback,
                                  public juce::ChangeListener {
public:
    AudioDeviceManagerWrapper();
    ~AudioDeviceManagerWrapper() override;

    void initialise();

    // juce::MidiInputCallback
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    
    void injectMidiMessage(const juce::MidiMessage& msg);

    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }

private:
    juce::AudioDeviceManager deviceManager;
    juce::MidiMessageCollector midiCollector;
};

} // namespace Nimbus
