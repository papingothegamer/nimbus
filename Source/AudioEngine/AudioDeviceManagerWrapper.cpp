#include "AudioDeviceManagerWrapper.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace Nimbus {

AudioDeviceManagerWrapper::AudioDeviceManagerWrapper(AudioGraph& mainGraph, Transport& transport)
    : graph(mainGraph), globalTransport(transport)
{
}

AudioDeviceManagerWrapper::~AudioDeviceManagerWrapper() {
    deviceManager.removeChangeListener(this);
    deviceManager.removeAudioCallback(this);
}

void AudioDeviceManagerWrapper::initialise() {
    juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Nimbus/AudioDeviceSettings.xml");
    
    std::unique_ptr<juce::XmlElement> savedState;
    if (settingsFile.existsAsFile()) {
        savedState = juce::XmlDocument::parse(settingsFile);
    }
    
    juce::String errorInfo;
    if (savedState != nullptr) {
        errorInfo = deviceManager.initialise(2, 2, savedState.get(), true);
    }
    
    if (savedState == nullptr || errorInfo.isNotEmpty()) {
        // Fallback to defaults
        errorInfo = deviceManager.initialiseWithDefaultDevices(2, 2);
    }

    if (errorInfo.isNotEmpty()) {
        juce::Logger::writeToLog("AudioDeviceManager Error: " + errorInfo);
    } else {
        juce::Logger::writeToLog("AudioDeviceManager: initialised OK");
    }
    
    // Log what device was actually opened
    if (auto* device = deviceManager.getCurrentAudioDevice()) {
        juce::Logger::writeToLog("  Device type: " + device->getTypeName());
        juce::Logger::writeToLog("  Device name: " + device->getName());
        juce::Logger::writeToLog("  Sample rate: " + juce::String(device->getCurrentSampleRate()));
        juce::Logger::writeToLog("  Buffer size: " + juce::String(device->getCurrentBufferSizeSamples()));
        
        auto activeIn = device->getActiveInputChannels();
        auto activeOut = device->getActiveOutputChannels();
        juce::Logger::writeToLog("  Active input channels bitmask: " + juce::String(activeIn.toInteger()));
        juce::Logger::writeToLog("  Active output channels bitmask: " + juce::String(activeOut.toInteger()));
        juce::Logger::writeToLog("  Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
    } else {
        juce::Logger::writeToLog("  WARNING: No audio device opened!");
    }
    
    deviceManager.addAudioCallback(this);
    deviceManager.addMidiInputDeviceCallback(juce::String(), this);
    deviceManager.addChangeListener(this);
}

void AudioDeviceManagerWrapper::changeListenerCallback(juce::ChangeBroadcaster*) {
    // Save settings when they change
    auto state = deviceManager.createStateXml();
    if (state != nullptr) {
        juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Nimbus/AudioDeviceSettings.xml");
        settingsFile.getParentDirectory().createDirectory();
        state->writeTo(settingsFile);
    }
}

void AudioDeviceManagerWrapper::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device) {
        auto sampleRate = device->getCurrentSampleRate();
        auto blockSize = device->getCurrentBufferSizeSamples();
        
        auto activeIn = device->getActiveInputChannels();
        int numActiveInputs = activeIn.countNumberOfSetBits();
        juce::Logger::writeToLog("audioDeviceAboutToStart: SR=" + juce::String(sampleRate) 
            + " BS=" + juce::String(blockSize) 
            + " activeInputs=" + juce::String(numActiveInputs));
        
        midiCollector.reset(sampleRate);

        // Ensure our pre-allocated process buffer is large enough
        processBuffer.setSize(2, blockSize);
        
        globalTransport.setSampleRate(sampleRate);
        graph.prepareToPlay(sampleRate, blockSize);
    }
}

void AudioDeviceManagerWrapper::audioDeviceStopped() {
    graph.releaseResources();
}

void AudioDeviceManagerWrapper::handleIncomingMidiMessage(juce::MidiInput* /*source*/, const juce::MidiMessage& message) {
    midiCollector.addMessageToQueue(message);
}

void AudioDeviceManagerWrapper::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                                 int numInputChannels,
                                                                 float* const* outputChannelData,
                                                                 int numOutputChannels,
                                                                 int numSamples,
                                                                 const juce::AudioIODeviceCallbackContext& /*context*/) 
{
    // Clear our intermediate process buffer
    processBuffer.clear();
    liveMidiBuffer.clear();

    // Copy incoming MIDI events into liveMidiBuffer
    midiCollector.removeNextBlockOfMessages(liveMidiBuffer, numSamples);

    // One-shot diagnostic: log input stats for the first few callbacks
    static int callbackCount = 0;
    if (callbackCount < 5) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numInputChannels; ++ch) {
            if (inputChannelData[ch] != nullptr) {
                for (int s = 0; s < numSamples; ++s) {
                    float v = std::abs(inputChannelData[ch][s]);
                    if (v > maxLevel) maxLevel = v;
                }
            }
        }
        juce::Logger::writeToLog("AudioCallback #" + juce::String(callbackCount)
            + ": numIn=" + juce::String(numInputChannels)
            + " numOut=" + juce::String(numOutputChannels)
            + " samples=" + juce::String(numSamples)
            + " inputPeak=" + juce::String(maxLevel, 6));
        callbackCount++;
    }

    // Copy input hardware data to processBuffer
    int numInputChannelsToCopy = std::min(numInputChannels, processBuffer.getNumChannels());
    for (int ch = 0; ch < numInputChannelsToCopy; ++ch) {
        if (inputChannelData[ch] != nullptr) {
            processBuffer.copyFrom(ch, 0, inputChannelData[ch], numSamples);
        }
    }

    // The graph processes and writes into processBuffer
    graph.processBlock(processBuffer, liveMidiBuffer);
    
    // Advance the transport
    globalTransport.advancePosition(numSamples);

    // Copy the processed audio to the hardware output buffers
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch] != nullptr) {
            // Copy from processBuffer to outputChannelData
            int srcChannel = juce::jmin(ch, processBuffer.getNumChannels() - 1);
            juce::FloatVectorOperations::copy(outputChannelData[ch],
                                              processBuffer.getReadPointer(srcChannel),
                                              numSamples);
        }
    }
}

} // namespace Nimbus

