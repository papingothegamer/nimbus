#include "Transport.h"
#include <cmath>

namespace Nimbus {

Transport::Transport() = default;

void Transport::play() {
    playing.store(true, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this]() { listeners.call(&Listener::transportStateChanged); });
}

void Transport::stop() {
    playing.store(false, std::memory_order_relaxed);
    recording.store(false, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this]() { listeners.call(&Listener::transportStateChanged); });
}

void Transport::record() {
    recording.store(true, std::memory_order_relaxed);
    playing.store(true, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this]() { listeners.call(&Listener::transportStateChanged); });
}

void Transport::stopRecording() {
    recording.store(false, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this]() { listeners.call(&Listener::transportStateChanged); });
}

void Transport::setPosition(double samplePosition) {
    double sr = sampleRate.load(std::memory_order_relaxed);
    if (sr > 0.0) {
        playHead.setPositionSeconds(samplePosition / sr);
    }
}

double Transport::getCurrentPosition() const {
    double sr = sampleRate.load(std::memory_order_relaxed);
    if (sr <= 0.0) sr = 48000.0;
    return playHead.getLivePositionSeconds() * sr;
}

int Transport::getCurrentPositionSamples() const {
    return static_cast<int>(getCurrentPosition());
}

bool Transport::isPlaying() const {
    return playing.load(std::memory_order_relaxed);
}

bool Transport::isRecording() const {
    return recording.load(std::memory_order_relaxed);
}

bool Transport::isLooping() const {
    return looping.load(std::memory_order_relaxed);
}

void Transport::setLooping(bool shouldLoop) {
    looping.store(shouldLoop, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this, shouldLoop]() { listeners.call(&Listener::transportLoopingChanged, shouldLoop); });
}

void Transport::setLoopRegion(double startSamples, double endSamples) {
    loopStartSamples.store(startSamples, std::memory_order_relaxed);
    loopEndSamples.store(endSamples, std::memory_order_relaxed);
}

double Transport::getLoopStartSamples() const {
    return loopStartSamples.load(std::memory_order_relaxed);
}

double Transport::getLoopEndSamples() const {
    return loopEndSamples.load(std::memory_order_relaxed);
}

double Transport::getSampleRate() const {
    return sampleRate.load(std::memory_order_relaxed);
}

double Transport::getTempo() const {
    return tempo.load(std::memory_order_relaxed);
}

void Transport::setTempo(double newTempo) {
    tempo.store(newTempo, std::memory_order_relaxed);
    juce::MessageManager::callAsync([this, newTempo]() { listeners.call(&Listener::transportTempoChanged, newTempo); });
}

int Transport::getTimeSignatureNumerator() const {
    return timeSigNumerator.load(std::memory_order_relaxed);
}

int Transport::getTimeSignatureDenominator() const {
    return timeSigDenominator.load(std::memory_order_relaxed);
}

void Transport::setTimeSignature(int numerator, int denominator) {
    timeSigNumerator.store(numerator, std::memory_order_relaxed);
    timeSigDenominator.store(denominator, std::memory_order_relaxed);
}

void Transport::setLatencySamples(int latencySamples) {
    playHead.setLatencySamples(latencySamples);
}

void Transport::advancePosition(int numSamples) {
    if (isPlaying()) {
        double sr = sampleRate.load(std::memory_order_relaxed);
        if (sr <= 0.0) return;
        bool isLoop = looping.load(std::memory_order_relaxed);
        double loopStart = loopStartSamples.load(std::memory_order_relaxed) / sr;
        double loopEnd = loopEndSamples.load(std::memory_order_relaxed) / sr;
        playHead.advanceAudioTime(numSamples, sr, isLoop, loopStart, loopEnd);
    }
}

void Transport::setSampleRate(double newSampleRate) {
    sampleRate.store(newSampleRate, std::memory_order_relaxed);
}

} // namespace Nimbus
