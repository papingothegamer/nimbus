#include "Transport.h"

namespace Nimbus {

Transport::Transport() = default;

void Transport::play() {
    playing.store(true, std::memory_order_relaxed);
}

void Transport::stop() {
    playing.store(false, std::memory_order_relaxed);
    recording.store(false, std::memory_order_relaxed);
}

void Transport::record() {
    recording.store(true, std::memory_order_relaxed);
    playing.store(true, std::memory_order_relaxed);
}

void Transport::stopRecording() {
    recording.store(false, std::memory_order_relaxed);
}

void Transport::setPosition(double samplePosition) {
    currentPosition.store(samplePosition, std::memory_order_relaxed);
}

double Transport::getCurrentPosition() const {
    return currentPosition.load(std::memory_order_relaxed);
}

int Transport::getCurrentPositionSamples() const {
    return static_cast<int>(currentPosition.load(std::memory_order_relaxed));
}

bool Transport::isPlaying() const {
    return playing.load(std::memory_order_relaxed);
}

bool Transport::isRecording() const {
    return recording.load(std::memory_order_relaxed);
}

double Transport::getSampleRate() const {
    return sampleRate.load(std::memory_order_relaxed);
}

double Transport::getTempo() const {
    return tempo.load(std::memory_order_relaxed);
}

void Transport::advancePosition(int numSamples) {
    if (isPlaying()) {
        // We use fetch_add to safely advance the clock lock-free
        // Note: fetch_add on double is not natively supported in std::atomic before C++20,
        // but C++20 standardizes std::atomic<float> and std::atomic<double>.
        // Since we target C++20, this is perfectly valid.
        currentPosition.fetch_add(static_cast<double>(numSamples), std::memory_order_relaxed);
    }
}

void Transport::setSampleRate(double newSampleRate) {
    sampleRate.store(newSampleRate, std::memory_order_relaxed);
}

} // namespace Nimbus
