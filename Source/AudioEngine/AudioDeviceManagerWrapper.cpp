#include "AudioDeviceManagerWrapper.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace Nimbus {

AudioDeviceManagerWrapper::AudioDeviceManagerWrapper(AudioGraph& mainGraph)
    : graph(mainGraph)
{
}

AudioDeviceManagerWrapper::~AudioDeviceManagerWrapper() {
    deviceManager.removeAudioCallback(this);
}

void AudioDeviceManagerWrapper::initialise() {
    // Initialise the device manager with 2 input channels and 2 output channels
    auto errorInfo = deviceManager.initialiseWithDefaultDevices(2, 2);
    if (errorInfo.isNotEmpty()) {
        juce::Logger::writeToLog("AudioDeviceManager Error: " + errorInfo);
    }
    
    deviceManager.addAudioCallback(this);
}

void AudioDeviceManagerWrapper::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device) {
        auto sampleRate = device->getCurrentSampleRate();
        auto blockSize = device->getCurrentBufferSizeSamples();
        
        // Ensure our pre-allocated process buffer is large enough
        processBuffer.setSize(2, blockSize);
        
        graph.prepareToPlay(sampleRate, blockSize);
    }
}

void AudioDeviceManagerWrapper::audioDeviceStopped() {
    graph.releaseResources();
}

void AudioDeviceManagerWrapper::audioDeviceIOCallbackWithContext(const float* const* /*inputChannelData*/,
                                                                 int /*numInputChannels*/,
                                                                 float* const* outputChannelData,
                                                                 int numOutputChannels,
                                                                 int numSamples,
                                                                 const juce::AudioIODeviceCallbackContext& /*context*/) 
{
    // Clear our intermediate process buffer
    processBuffer.clear();

    // The graph processes and writes into processBuffer
    graph.processBlock(processBuffer, dummyMidiBuffer);

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
