#include "TempoSequence.h"
#include <cmath>

namespace Nimbus {

TempoSequence::TempoSequence() = default;
TempoSequence::~TempoSequence() = default;

void TempoSequence::setTempo(double bpm) {
    if (bpm > 0.0) currentBpm.store(bpm, std::memory_order_relaxed);
}

double TempoSequence::getTempo() const {
    return currentBpm.load(std::memory_order_relaxed);
}

void TempoSequence::setTimeSignature(int numerator, int denominator) {
    if (numerator > 0) sigNumerator.store(numerator, std::memory_order_relaxed);
    if (denominator > 0) sigDenominator.store(denominator, std::memory_order_relaxed);
}

int TempoSequence::getTimeSigNumerator() const {
    return sigNumerator.load(std::memory_order_relaxed);
}

int TempoSequence::getTimeSigDenominator() const {
    return sigDenominator.load(std::memory_order_relaxed);
}

double TempoSequence::timeToBeats(double timeInSeconds) const {
    double secondsPerBeat = 60.0 / currentBpm.load(std::memory_order_relaxed);
    return timeInSeconds / secondsPerBeat;
}

double TempoSequence::beatsToTime(double beats) const {
    double secondsPerBeat = 60.0 / currentBpm.load(std::memory_order_relaxed);
    return beats * secondsPerBeat;
}

BarsAndBeats TempoSequence::timeToBarsAndBeats(double timeInSeconds) const {
    double totalBeats = timeToBeats(timeInSeconds);
    
    BarsAndBeats bnb;
    double beatsPerBar = static_cast<double>(sigNumerator.load(std::memory_order_relaxed)); 
    
    bnb.bars = static_cast<int>(std::floor(totalBeats / beatsPerBar));
    bnb.beats = std::fmod(totalBeats, beatsPerBar);
    
    return bnb;
}

double TempoSequence::barsAndBeatsToTime(const BarsAndBeats& bnb) const {
    double beatsPerBar = static_cast<double>(sigNumerator.load(std::memory_order_relaxed));
    double totalBeats = (bnb.bars * beatsPerBar) + bnb.beats;
    return beatsToTime(totalBeats);
}

} // namespace Nimbus
