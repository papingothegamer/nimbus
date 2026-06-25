#include "Mixer.h"

namespace Nimbus {

Mixer::Mixer() : trackAddQueue(128), trackRemoveQueue(128) {
}

void Mixer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    for (auto& track : tracks) {
        track->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    }
    masterFader.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    inputBufferCopy.setSize(2, maximumExpectedSamplesPerBlock);
    inputBufferCopy.clear();
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

    int removeIndex;
    while (trackRemoveQueue.pop(removeIndex)) {
        if (removeIndex >= 0 && removeIndex < (int)tracks.size()) {
            tracks[removeIndex]->releaseResources();
            tracks.erase(tracks.begin() + removeIndex);
        }
    }

    // 2. Copy the live input from buffer before clearing it
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        inputBufferCopy.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            inputBufferCopy.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
        }
    }

    // 3. Clear master buffer
    buffer.clear();

    // 4. Process and sum all tracks
    bool anySoloed = false;
    for (auto& t : tracks) if (t->isSoloed()) { anySoloed = true; break; }

    for (auto& track : tracks) {
        track->setInputBuffer(&inputBufferCopy);
        if (anySoloed && !track->isSoloed()) continue;
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

void Mixer::removeTrack(int index) {
    trackRemoveQueue.push(index);
}

void Mixer::setMasterVolume(float gainLinear) {
    masterFader.setGainLinear(gainLinear);
}

void Mixer::setMasterPan(float panValue) {
    masterFader.setPan(panValue);
}

} // namespace Nimbus
