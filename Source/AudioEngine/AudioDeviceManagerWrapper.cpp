#include "AudioDeviceManagerWrapper.h"

namespace Nimbus {

AudioDeviceManagerWrapper::AudioDeviceManagerWrapper()
{
}

AudioDeviceManagerWrapper::~AudioDeviceManagerWrapper()
{
    deviceManager.removeChangeListener(this);
    deviceManager.removeMidiInputDeviceCallback(juce::String(), this);
}

void AudioDeviceManagerWrapper::initialise()
{
    juce::Logger::writeToLog("Initializing AudioDeviceManager...");

    // Try to find ASIO first
    const auto& types = deviceManager.getAvailableDeviceTypes();
    juce::AudioIODeviceType* asioType = nullptr;
    for (auto* type : types) {
        if (type->getTypeName() == "ASIO") {
            asioType = type;
            break;
        }
    }
    
    juce::String error;
    bool asioSuccess = false;
    
    if (asioType != nullptr) {
        deviceManager.setCurrentAudioDeviceType("ASIO", true);
        error = deviceManager.initialise(2, 2, nullptr, true);
        if (error.isEmpty()) {
            asioSuccess = true;
            juce::Logger::writeToLog("Audio device initialized successfully with ASIO.");
        } else {
            juce::Logger::writeToLog("ASIO initialization failed: " + error + ". Falling back...");
        }
    }
    
    if (!asioSuccess) {
        error = deviceManager.initialiseWithDefaultDevices(2, 2);
        if (error.isNotEmpty()) {
            juce::Logger::writeToLog("Audio device initialization error: " + error);
        } else {
            juce::Logger::writeToLog("Audio device initialized successfully (Fallback).");
        }
    }

    deviceManager.addChangeListener(this);
    deviceManager.addMidiInputDeviceCallback(juce::String(), this);
    
    // reset collector
    midiCollector.reset(44100.0);
}

void AudioDeviceManagerWrapper::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    midiCollector.addMessageToQueue(message);
}

void AudioDeviceManagerWrapper::injectMidiMessage(const juce::MidiMessage& msg)
{
    midiCollector.addMessageToQueue(msg);
}

void AudioDeviceManagerWrapper::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager) {
        auto setup = deviceManager.getAudioDeviceSetup();
        double sr = setup.sampleRate;
        if (sr > 0.0) {
            midiCollector.reset(sr);
        }
    }
}

} // namespace Nimbus
