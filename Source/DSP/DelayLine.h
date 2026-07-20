#pragma once

#include <JuceHeader.h>

namespace Nimbus {

class DelayLine {
public:
    DelayLine();
    ~DelayLine() = default;

    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock, int maxDelayInSamples = 192000); // Default 192000 = ~4 seconds at 48kHz
    void setDelaySamples(int delayInSamples);
    void process(juce::AudioBuffer<float>& buffer);

    int getDelaySamples() const { return delaySamples; }

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    int delaySamples = 0;
    double currentSampleRate = 44100.0;
};

} // namespace Nimbus
