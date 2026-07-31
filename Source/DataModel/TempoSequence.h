#pragma once

#include <JuceHeader.h>
#include <atomic>

namespace Nimbus {

struct BarsAndBeats {
    int bars = 0;
    double beats = 0.0; // Fractional beats
};

/**
 * Native implementation of a Tempo Sequence.
 * Handles conversions between Time, Samples, Beats, and Bars.
 * Designed to mirror tracktion's tempo sequence methodology.
 */
class TempoSequence {
public:
    TempoSequence();
    ~TempoSequence();

    void setTempo(double bpm);
    double getTempo() const;

    void setTimeSignature(int numerator, int denominator);
    int getTimeSigNumerator() const;
    int getTimeSigDenominator() const;

    // Conversions
    double timeToBeats(double timeInSeconds) const;
    double beatsToTime(double beats) const;

    BarsAndBeats timeToBarsAndBeats(double timeInSeconds) const;
    double barsAndBeatsToTime(const BarsAndBeats& bnb) const;

private:
    std::atomic<double> currentBpm{120.0};
    std::atomic<int> sigNumerator{4};
    std::atomic<int> sigDenominator{4};
};

} // namespace Nimbus
