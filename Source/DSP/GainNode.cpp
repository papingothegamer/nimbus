#include "GainNode.h"
#include <cmath>

namespace Nimbus {

GainNode::GainNode() {
    // Default to 0dB (1.0 multiplier)
    gain.setCurrentAndTargetValue(1.0f);
    pan.setCurrentAndTargetValue(0.0f);
}

void GainNode::prepareToPlay(double sampleRate, int /*maximumExpectedSamplesPerBlock*/) {
    // 20ms ramp time prevents zipper noise
    gain.reset(sampleRate, 0.02);
    pan.reset(sampleRate, 0.02);
}

void GainNode::releaseResources() {
}

void GainNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (buffer.getNumChannels() == 0) return;

    int numSamples = buffer.getNumSamples();

    // Fast path: if no parameters are actively smoothing, we can just apply a static gain
    if (!gain.isSmoothing() && !pan.isSmoothing()) {
        float currentGain = gain.getCurrentValue();
        float currentPan = pan.getCurrentValue();

        // Hard left = 1.0/0.0, Center = 0.707/0.707, Hard right = 0.0/1.0 (Equal power panning)
        float leftPanLinear = std::cos((currentPan + 1.0f) * 0.25f * juce::MathConstants<float>::pi);
        float rightPanLinear = std::sin((currentPan + 1.0f) * 0.25f * juce::MathConstants<float>::pi);

        if (buffer.getNumChannels() == 1) {
            buffer.applyGain(currentGain);
        } else if (buffer.getNumChannels() >= 2) {
            buffer.applyGain(0, 0, numSamples, currentGain * leftPanLinear);
            buffer.applyGain(1, 0, numSamples, currentGain * rightPanLinear);
        }
    } else {
        // Slow path: Calculate smoothing per-sample
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i) {
            float g = gain.getNextValue();
            float p = pan.getNextValue();

            float leftPanLinear = std::cos((p + 1.0f) * 0.25f * juce::MathConstants<float>::pi);
            float rightPanLinear = std::sin((p + 1.0f) * 0.25f * juce::MathConstants<float>::pi);

            leftChannel[i] *= (g * leftPanLinear);
            if (rightChannel != nullptr) {
                rightChannel[i] *= (g * rightPanLinear);
            }
        }
    }
}

void GainNode::setGainLinear(float newGainLinear) {
    gain.setTargetValue(newGainLinear);
}

void GainNode::setPan(float newPan) {
    pan.setTargetValue(std::clamp(newPan, -1.0f, 1.0f));
}

} // namespace Nimbus
