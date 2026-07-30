#include "TempoSequence.h"
#include <algorithm>

namespace Nimbus {

TempoSequence::TempoSequence() {
    // Add default segment
    addSegment(0.0, 120.0, 4, 4);
}

void TempoSequence::addSegment(double startBeat, double bpm, int sigNum, int sigDen) {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    
    // Check if segment exists at startBeat
    auto it = std::find_if(segments.begin(), segments.end(),
        [startBeat](const TempoSegment& seg) { return seg.startBeat == startBeat; });

    if (it != segments.end()) {
        it->bpm = bpm;
        it->timeSigNumerator = sigNum;
        it->timeSigDenominator = sigDen;
    } else {
        TempoSegment newSeg;
        newSeg.startBeat = startBeat;
        newSeg.bpm = bpm > 0.0 ? bpm : 120.0;
        newSeg.timeSigNumerator = sigNum;
        newSeg.timeSigDenominator = sigDen;
        
        segments.push_back(newSeg);
        std::sort(segments.begin(), segments.end(),
            [](const TempoSegment& a, const TempoSegment& b) {
                return a.startBeat < b.startBeat;
            });
    }
    
    updatePrecalculatedTimes();
}

void TempoSequence::clear() {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    segments.clear();
    addSegment(0.0, 120.0, 4, 4);
}

void TempoSequence::updatePrecalculatedTimes() {
    double currentTime = 0.0;
    
    for (size_t i = 0; i < segments.size(); ++i) {
        if (i == 0) {
            segments[i].startTime = 0.0;
        } else {
            double beatDiff = segments[i].startBeat - segments[i-1].startBeat;
            double secondsPerBeat = 60.0 / segments[i-1].bpm;
            currentTime += beatDiff * secondsPerBeat;
            segments[i].startTime = currentTime;
        }
    }
}

double TempoSequence::beatsToTime(double beats) const {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    
    if (segments.empty()) return 0.0;
    
    if (beats <= segments.front().startBeat) {
        double secondsPerBeat = 60.0 / segments.front().bpm;
        return segments.front().startTime + (beats - segments.front().startBeat) * secondsPerBeat;
    }

    for (int i = static_cast<int>(segments.size()) - 1; i >= 0; --i) {
        if (beats >= segments[i].startBeat) {
            double secondsPerBeat = 60.0 / segments[i].bpm;
            return segments[i].startTime + (beats - segments[i].startBeat) * secondsPerBeat;
        }
    }
    
    return 0.0;
}

double TempoSequence::timeToBeats(double time) const {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    
    if (segments.empty()) return 0.0;
    
    if (time <= segments.front().startTime) {
        double beatsPerSecond = segments.front().bpm / 60.0;
        return segments.front().startBeat + (time - segments.front().startTime) * beatsPerSecond;
    }

    for (int i = static_cast<int>(segments.size()) - 1; i >= 0; --i) {
        if (time >= segments[i].startTime) {
            double beatsPerSecond = segments[i].bpm / 60.0;
            return segments[i].startBeat + (time - segments[i].startTime) * beatsPerSecond;
        }
    }
    
    return 0.0;
}

double TempoSequence::getTempoAtTime(double time) const {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    if (segments.empty()) return 120.0;
    
    for (int i = static_cast<int>(segments.size()) - 1; i >= 0; --i) {
        if (time >= segments[i].startTime) {
            return segments[i].bpm;
        }
    }
    return segments.front().bpm;
}

double TempoSequence::getTempoAtBeat(double beat) const {
    std::lock_guard<std::mutex> lock(sequenceMutex);
    if (segments.empty()) return 120.0;
    
    for (int i = static_cast<int>(segments.size()) - 1; i >= 0; --i) {
        if (beat >= segments[i].startBeat) {
            return segments[i].bpm;
        }
    }
    return segments.front().bpm;
}

} // namespace Nimbus
