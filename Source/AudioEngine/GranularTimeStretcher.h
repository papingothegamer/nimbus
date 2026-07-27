#pragma once
#include <JuceHeader.h>
#include <vector>

namespace Nimbus {

class GranularTimeStretcher {
public:
    GranularTimeStretcher();
    ~GranularTimeStretcher() = default;

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    /**
     * Processes audio to change speed without changing pitch.
     * speedRatio > 1.0 means faster playback (shorter duration).
     * speedRatio < 1.0 means slower playback (longer duration).
     * 
     * The outputBuffer is filled with `outputBuffer.getNumSamples()` samples.
     * The input buffer is expected to have enough samples to satisfy the demand.
     */
    void process(juce::AudioBuffer<float>& outputBuffer, const juce::AudioBuffer<float>& sourceBuffer, double speedRatio);

    /**
     * Helper to determine how many source samples to read to produce `numOutputSamples`
     * given the current internal phase and speedRatio.
     */
    int getNumSourceSamplesRequired(int numOutputSamples, double speedRatio) const;

private:
    double currentSampleRate = 44100.0;
    int channels = 2;
    
    // Grain parameters
    int grainSizeSamples = 2048; 
    
    // Internal state
    double sourcePhase = 0.0;
    int outputPhase = 0;
    
    juce::AudioBuffer<float> overlapBuffer;
    std::vector<float> window;
};

} // namespace Nimbus
