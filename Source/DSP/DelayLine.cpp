#include "DelayLine.h"

namespace Nimbus {

DelayLine::DelayLine() {}

void DelayLine::prepare(double sampleRate, int maximumExpectedSamplesPerBlock, int maxDelayInSamples) {
    currentSampleRate = sampleRate;
    delayBuffer.setSize(2, maxDelayInSamples); // Force stereo internally for safety
    delayBuffer.clear();
    writePosition = 0;
}

void DelayLine::setDelaySamples(int delay) {
    // Clamp to valid range
    if (delay < 0) delay = 0;
    if (delay >= delayBuffer.getNumSamples()) delay = delayBuffer.getNumSamples() - 1;
    delaySamples = delay;
}

void DelayLine::process(juce::AudioBuffer<float>& buffer) {
    if (delaySamples == 0) {
        return; // Zero latency, bypass directly
    }

    int numChannels = std::min(buffer.getNumChannels(), delayBuffer.getNumChannels());
    int numSamples = buffer.getNumSamples();
    int bufferLength = delayBuffer.getNumSamples();
    
    if (bufferLength == 0) return;

    for (int channel = 0; channel < numChannels; ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        float* delayData = delayBuffer.getWritePointer(channel);

        int dPos = writePosition;

        for (int i = 0; i < numSamples; ++i) {
            float inputSample = channelData[i];
            
            // Write to delay line
            delayData[dPos] = inputSample;

            // Read from delay line
            int readPos = dPos - delaySamples;
            if (readPos < 0) readPos += bufferLength;

            channelData[i] = delayData[readPos];

            // Advance pointer
            dPos++;
            if (dPos >= bufferLength) dPos = 0;
        }
    }
    
    // Update write position once for all channels
    writePosition += numSamples;
    while (writePosition >= bufferLength) {
        writePosition -= bufferLength;
    }
}

} // namespace Nimbus
