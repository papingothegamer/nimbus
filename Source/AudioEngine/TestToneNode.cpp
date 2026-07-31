#include "TestToneNode.h"
#include <juce_core/juce_core.h>

namespace Nimbus {

TestToneNode::TestToneNode() {}

void TestToneNode::prepare(double sampleRate, int /*maximumExpectedSamplesPerBlock*/) {
    currentSampleRate = sampleRate;
    
    // Calculate cycles per sample
    auto cyclesPerSample = frequency / currentSampleRate;
    angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
}

void TestToneNode::process(const ProcessContext& context) {
    if (!context.buffer) return;
    
    auto* leftChannel = context.buffer->getWritePointer(0);
    auto* rightChannel = context.buffer->getNumChannels() > 1 ? context.buffer->getWritePointer(1) : nullptr;
    
    for (int i = 0; i < context.buffer->getNumSamples(); ++i) {
        auto currentSample = (float) (std::sin(currentAngle) * level);
        
        currentAngle += angleDelta;
        
        if (leftChannel)  leftChannel[i] = currentSample;
        if (rightChannel) rightChannel[i] = currentSample;
    }
}

} // namespace Nimbus
