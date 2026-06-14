#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "AudioGraph.h"

namespace Nimbus {

/**
 * Wraps juce::AudioDeviceManager and feeds the AudioGraph.
 */
class AudioDeviceManagerWrapper : public juce::AudioIODeviceCallback {
public:
    AudioDeviceManagerWrapper(AudioGraph& mainGraph);
    ~AudioDeviceManagerWrapper() override;

    void initialise();

    // juce::AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    juce::AudioDeviceManager deviceManager;
    AudioGraph& graph;
    juce::AudioBuffer<float> processBuffer;
    juce::MidiBuffer dummyMidiBuffer;
};

} // namespace Nimbus
