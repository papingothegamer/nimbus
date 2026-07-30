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
     * Processes audio with granular stretching.
     * outputStartSample/outputNumSamples dictate where to write in outputBuffer.
     * sourceStartSample/sourceNumSamples dictate where to read in sourceBuffer.
     */
    void process(juce::AudioBuffer<float>& outputBuffer, int outputStartSample, int outputNumSamples,
                 const juce::AudioBuffer<float>& sourceBuffer, int sourceStartSample, int sourceNumSamples,
                 double speedRatio, int samplesAdvancedByCaller);

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
