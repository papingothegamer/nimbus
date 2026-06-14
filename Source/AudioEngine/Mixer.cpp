#include "Mixer.h"

namespace Nimbus {

Mixer::Mixer() : trackAddQueue(128) {
}

void Mixer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    for (auto& track : tracks) {
        track->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    }
    masterFader.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void Mixer::releaseResources() {
    for (auto& track : tracks) {
        track->releaseResources();
    }
    masterFader.releaseResources();
}

void Mixer::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // 1. Pick up any new tracks added from the UI
    std::unique_ptr<Track> pendingTrack;
    while (trackAddQueue.pop(pendingTrack)) {
        if (pendingTrack) {
            pendingTrack->prepareToPlay(currentSampleRate, currentBlockSize);
            tracks.push_back(std::move(pendingTrack));
        }
    }

    // 2. Clear master buffer
    buffer.clear();

    // 3. Process and sum all tracks
    for (auto& track : tracks) {
        track->processBlock(buffer, midiMessages);
    }

    // 4. Apply master fader and panning
    masterFader.processBlock(buffer, midiMessages);

    // 5. Update master level meter
    meter.processBlock(buffer);
}

int Mixer::getLatencySamples() const {
    // In a real DAW with PDC (Plugin Delay Compensation), 
    // the mixer latency is the maximum latency of all tracks.
    int maxLatency = 0;
    for (const auto& track : tracks) {
        int trackLatency = track->getLatencySamples();
        if (trackLatency > maxLatency) {
            maxLatency = trackLatency;
        }
    }
    return maxLatency;
}

void Mixer::addTrack(std::unique_ptr<Track> track) {
    bool success = trackAddQueue.push(std::move(track));
    jassert(success && "Track add queue is full!");
}

void Mixer::setMasterVolume(float gainLinear) {
    masterFader.setGainLinear(gainLinear);
}

void Mixer::setMasterPan(float panValue) {
    masterFader.setPan(panValue);
}

} // namespace Nimbus
