#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <algorithm>

namespace Nimbus {

/**
 * Lock-free level measurer for VU metering, inspired by Tracktion Engine.
 * Extracts peak levels from audio buffers and provides thread-safe readout with decay.
 */
class LevelMeasurer {
public:
    LevelMeasurer() {
        peakL.store(0.0f, std::memory_order_relaxed);
        peakR.store(0.0f, std::memory_order_relaxed);
    }

    /**
     * Process an incoming audio block on the audio thread.
     * Accumulates maximum peaks using a lock-free compare-and-swap loop.
     */
    void processBuffer(const juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumSamples() == 0) return;

        float blockPeakL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
        float blockPeakR = buffer.getNumChannels() > 1 ? buffer.getMagnitude(1, 0, buffer.getNumSamples()) : blockPeakL;

        // Atomically accumulate max peak
        float currentL = peakL.load(std::memory_order_relaxed);
        while (blockPeakL > currentL && !peakL.compare_exchange_weak(currentL, blockPeakL, std::memory_order_relaxed)) {}

        float currentR = peakR.load(std::memory_order_relaxed);
        while (blockPeakR > currentR && !peakR.compare_exchange_weak(currentR, blockPeakR, std::memory_order_relaxed)) {}
    }

    /**
     * Get the decayed peak on the UI thread.
     * This resets the atomic accumulator but preserves a decayed peak state to prevent flashing.
     * @param decayFactor e.g., 0.85f for 30Hz UI polling.
     */
    std::pair<float, float> getDecayedPeak(float decayFactor = 0.85f) {
        // Exchange with 0 to get the max peak since last poll
        float newPeakL = peakL.exchange(0.0f, std::memory_order_relaxed);
        float newPeakR = peakR.exchange(0.0f, std::memory_order_relaxed);

        // Apply decay
        currentDisplayL = std::max(newPeakL, currentDisplayL * decayFactor);
        currentDisplayR = std::max(newPeakR, currentDisplayR * decayFactor);

        return {currentDisplayL, currentDisplayR};
    }

private:
    std::atomic<float> peakL { 0.0f };
    std::atomic<float> peakR { 0.0f };
    float currentDisplayL = 0.0f;
    float currentDisplayR = 0.0f;
};

} // namespace Nimbus
