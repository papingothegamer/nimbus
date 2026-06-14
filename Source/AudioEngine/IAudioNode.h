#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace Nimbus {

/**
 * The foundational interface for any object in the processing graph.
 * This includes Tracks, Plugins, Busses, and Instruments.
 */
class IAudioNode {
public:
    virtual ~IAudioNode() = default;

    /**
     * Called from the message thread when sample rate or block size changes.
     * Implementations can allocate memory here.
     */
    virtual void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) = 0;

    /**
     * Called from the message thread when playback stops and resources can be freed.
     */
    virtual void releaseResources() = 0;

    /**
     * Called exclusively on the high-priority real-time audio thread.
     * MUST NOT ALLOCATE MEMORY. MUST NOT BLOCK OR ACQUIRE LOCKS.
     */
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;

    /**
     * Returns the processing latency in samples. Used for Delay Compensation.
     */
    virtual int getLatencySamples() const { return 0; }
};

} // namespace Nimbus
