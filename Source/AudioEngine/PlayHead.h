#pragma once

#include <atomic>
#include <algorithm>
#include <memory>

namespace Nimbus {

class PlayHead {
public:
    PlayHead() = default;
    ~PlayHead() = default;

    // Called by the audio thread to advance time
    void advanceAudioTime(int numSamples, double sampleRate, bool isLooping, double loopStartSecs, double loopEndSecs) {
        if (sampleRate <= 0.0) return;

        double oldPos = uncompensatedTimeSeconds.load(std::memory_order_relaxed);
        double deltaSecs = static_cast<double>(numSamples) / sampleRate;
        double newPos = oldPos + deltaSecs;

        if (isLooping && loopEndSecs > loopStartSecs) {
            if (oldPos < loopEndSecs && newPos >= loopEndSecs) {
                newPos = loopStartSecs + (newPos - loopEndSecs);
            }
        }

        uncompensatedTimeSeconds.store(newPos, std::memory_order_release);

        // Latency Compensation Logic
        if (numLatencySamplesToCountDown > 0) {
            int decrement = std::min(numLatencySamplesToCountDown, numSamples);
            numLatencySamplesToCountDown -= decrement;
        }

        double newAudiblePos = 0.0;
        if (numLatencySamplesToCountDown <= 0) {
            // Subtract latency from current position
            double latencyOffset = static_cast<double>(latencyNumSamples.load(std::memory_order_relaxed)) / sampleRate;
            newAudiblePos = std::max(0.0, newPos - latencyOffset);
        } else {
            // Still flushing latency buffers; stay at the jump reference point
            newAudiblePos = referencePositionOnJumpSecs;
        }
        
        audibleTimeSeconds.store(newAudiblePos, std::memory_order_release);
    }

    // Called by UI thread to get the current safe position (Audible time, latency compensated)
    double getLivePositionSeconds() const {
        return audibleTimeSeconds.load(std::memory_order_acquire);
    }

    // Called by UI / Commands to jump position
    void setPositionSeconds(double newPositionSecs) {
        double safePos = std::max(0.0, newPositionSecs);
        uncompensatedTimeSeconds.store(safePos, std::memory_order_release);
        audibleTimeSeconds.store(safePos, std::memory_order_release);
        
        referencePositionOnJumpSecs = safePos;
        numLatencySamplesToCountDown = latencyNumSamples.load(std::memory_order_relaxed);
    }

    // Called by Transport to set latency (e.g. from AudioDeviceManager)
    void setLatencySamples(int latencySamples) {
        latencyNumSamples.store(std::max(0, latencySamples), std::memory_order_relaxed);
    }

private:
    std::atomic<double> uncompensatedTimeSeconds{0.0};
    std::atomic<double> audibleTimeSeconds{0.0};
    std::atomic<int> latencyNumSamples{0};

    // State updated exclusively on the Audio Thread (except setPositionSeconds which acts as an event)
    int numLatencySamplesToCountDown = 0;
    double referencePositionOnJumpSecs = 0.0;
};

} // namespace Nimbus
