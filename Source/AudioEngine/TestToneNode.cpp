#include "TestToneNode.h"
#include <juce_core/juce_core.h>

namespace Nimbus {

TestToneNode::TestToneNode() = default;

void TestToneNode::prepareToPlay(double sampleRate, int /*maximumExpectedSamplesPerBlock*/) {
    currentSampleRate = sampleRate;
    auto cyclesPerSample = frequency / currentSampleRate;
    angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
}

void TestToneNode::releaseResources() {
}

void TestToneNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // Generate a simple sine wave and mix it into the buffer
    for (int sample = 0; sample < numSamples; ++sample) {
        auto currentSample = static_cast<float>(std::sin(currentAngle) * level);
        currentAngle += angleDelta;
        
        for (int channel = 0; channel < numChannels; ++channel) {
            // Add to the existing buffer contents
            buffer.addSample(channel, sample, currentSample);
        }
    }
}

} // namespace Nimbus
