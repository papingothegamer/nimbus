#pragma once

#include "IAudioNode.h"
#include <cmath>

namespace Nimbus {

class TestToneNode : public IAudioNode {
public:
    TestToneNode();
    ~TestToneNode() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

private:
    double currentSampleRate = 44100.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    
    const double frequency = 440.0; // A4
    const double level = 0.1;       // -20dBFS approx
};

} // namespace Nimbus
