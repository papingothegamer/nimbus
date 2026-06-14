#pragma once

#include "AudioEngine/IAudioNode.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace Nimbus {

/**
 * A basic DSP node that applies a linear gain to the audio signal.
 * Uses juce::SmoothedValue to prevent zipper noise when the gain changes.
 */
class GainNode : public IAudioNode {
public:
    GainNode();
    ~GainNode() override = default;

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

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
