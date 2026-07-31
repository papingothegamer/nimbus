#pragma once

#include "AudioEngine/Nodes/Node.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <atomic>

namespace Nimbus {

/**
 * A basic DSP node that applies a linear gain to the audio signal.
 * Uses juce::SmoothedValue to prevent zipper noise when the gain changes.
 */
class GainNode : public Node {
public:
    GainNode();
    ~GainNode() override = default;

    // Node
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void process(const ProcessContext& context) override;

    /**
     * Sets the target linear gain multiplier (e.g. 1.0 = 0dB, 0.5 = -6dB).
     * The node will smoothly ramp to this value over the block to prevent clicking.
     */
    void setGainLinear(float newGainLinear);

    /**
     * Set the pan (-1.0 = left, 1.0 = right, 0.0 = center).
     */
    void setPan(float newPan);

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pan;
};

} // namespace Nimbus
