#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <algorithm>
#include <cmath>

namespace Nimbus {

/**
 * A lock-free level meter to safely transfer peak and RMS values
 * from the audio thread to the UI thread.
 */
class LevelMeter {
public:
    LevelMeter() = default;
    ~LevelMeter() = default;

    /**
     * Called by the audio thread to update the meter with a new block of audio.
     */
    void processBlock(const juce::AudioBuffer<float>& buffer) {
        float maxPeak = 0.0f;
        float sumSquares = 0.0f;
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        if (numSamples == 0 || numChannels == 0) return;

        for (int ch = 0; ch < numChannels; ++ch) {
            auto* channelData = buffer.getReadPointer(ch);
            for (int s = 0; s < numSamples; ++s) {
                float sample = channelData[s];
                float absSample = std::abs(sample);
                
                if (absSample > maxPeak) maxPeak = absSample;
                sumSquares += (sample * sample);
            }
        }

        float currentPeak = maxPeak;
        float currentRms = std::sqrt(sumSquares / (numSamples * numChannels));

        // Use a decay envelope on the audio thread so the UI can catch peaks
        // even if it polls slowly (e.g. 60Hz vs audio rate).
        float oldPeak = peakLevel.load(std::memory_order_relaxed);
        if (currentPeak > oldPeak) {
            peakLevel.store(currentPeak, std::memory_order_relaxed);
        } else {
            // Simple decay (can be improved with time constants later)
            peakLevel.store(oldPeak * 0.95f, std::memory_order_relaxed);
        }

        float oldRms = rmsLevel.load(std::memory_order_relaxed);
        if (currentRms > oldRms) {
            rmsLevel.store(currentRms, std::memory_order_relaxed);
        } else {
            rmsLevel.store(oldRms * 0.95f, std::memory_order_relaxed);
        }
    }

    /**
     * Called by the UI thread to read the current peak level (0.0 to 1.0)
     */
    float getPeakLevel() const {
        return peakLevel.load(std::memory_order_relaxed);
    }

    /**
     * Called by the UI thread to read the current RMS level (0.0 to 1.0)
     */
    float getRMSLevel() const {
        return rmsLevel.load(std::memory_order_relaxed);
    }

private:
    std::atomic<float> peakLevel{0.0f};
    std::atomic<float> rmsLevel{0.0f};
};

} // namespace Nimbus
