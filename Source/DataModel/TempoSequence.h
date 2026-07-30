#pragma once

#include <vector>
#include <mutex>

namespace Nimbus {

struct TempoSegment {
    double startBeat = 0.0;
    double startTime = 0.0; // pre-calculated seconds for fast lookup
    double bpm = 120.0;
    int timeSigNumerator = 4;
    int timeSigDenominator = 4;
};

class TempoSequence {
public:
    TempoSequence();
    ~TempoSequence() = default;

    void addSegment(double startBeat, double bpm, int sigNum = 4, int sigDen = 4);
    void clear();

    double beatsToTime(double beats) const;
    double timeToBeats(double time) const;

    double getTempoAtTime(double time) const;
    double getTempoAtBeat(double beat) const;

private:
    void updatePrecalculatedTimes();
    
    std::vector<TempoSegment> segments;
    mutable std::mutex sequenceMutex;
};

} // namespace Nimbus
