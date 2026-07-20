#include "Mixer.h"

namespace Nimbus {

Mixer::Mixer() {
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
    // 2. Copy the live input from buffer before clearing it
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        inputBufferCopy.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            inputBufferCopy.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
        }
    }

    // 3. Clear master buffer
    buffer.clear();

    const juce::SpinLock::ScopedLockType sl(processLock);

    // 4. Update Plugin Delay Compensation (PDC)
    int maxLatency = getLatencySamples();
    for (auto& track : tracks) {
        int trackLatency = track->getLatencySamples();
        track->setCompensationDelay(maxLatency - trackLatency);
    }

    // 5. Clear Group Buffers
    for (auto& track : tracks) {
        if (track->isGroup()) track->clearGroupBuffer();
    }

    // 6. Process and sum all tracks (bottom-up traversal for folder routing)
    bool anySoloed = false;
    for (auto& t : tracks) if (t->isSoloed()) { anySoloed = true; break; }

    for (int i = (int)tracks.size() - 1; i >= 0; --i) {
        auto& track = tracks[i];
        
        bool shouldSilence = anySoloed;
        if (anySoloed) {
            if (isTrackOrAncestorSoloed(track.get()) || isAnyDescendantSoloed(track.get())) {
                shouldSilence = false;
            }
        }
        
        juce::AudioBuffer<float>* destBuffer = &buffer;
        if (!track->getParentGroupId().isNull()) {
            // Find parent group directly (we don't use getTrackById because we already hold the lock)
            for (auto& p : tracks) {
                if (p->getId() == track->getParentGroupId()) {
                    destBuffer = &p->getGroupBuffer();
                    break;
                }
            }
        }
        
        track->setInputBuffer(&inputBufferCopy);
        track->setSilencedBySolo(shouldSilence);
        track->processBlock(*destBuffer, midiMessages);
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

void Mixer::addTrack(std::unique_ptr<Track> track, int insertIndex) {
    if (track) {
        if (currentSampleRate > 0) {
            track->prepareToPlay(currentSampleRate, currentBlockSize);
        }
        const juce::SpinLock::ScopedLockType sl(processLock);
        if (insertIndex >= 0 && insertIndex <= (int)tracks.size()) {
            tracks.insert(tracks.begin() + insertIndex, std::move(track));
        } else {
            tracks.push_back(std::move(track));
        }
    }
}

void Mixer::removeTrack(int index) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    if (index >= 0 && index < (int)tracks.size()) {
        tracks[index]->releaseResources();
        tracks.erase(tracks.begin() + index);
    }
}

void Mixer::setMasterVolume(float gainLinear) {
    masterFader.setGainLinear(gainLinear);
}

void Mixer::setMasterPan(float panValue) {
    masterFader.setPan(panValue);
}

bool Mixer::isTrackOrAncestorSoloed(Track* track) const {
    if (!track) return false;
    if (track->isSoloed()) return true;
    
    if (!track->getParentGroupId().isNull()) {
        for (const auto& p : tracks) {
            if (p->getId() == track->getParentGroupId()) {
                return isTrackOrAncestorSoloed(p.get());
            }
        }
    }
    return false;
}

bool Mixer::isAnyDescendantSoloed(Track* track) const {
    if (!track) return false;
    
    // Check immediate children
    for (const auto& child : tracks) {
        if (child->getParentGroupId() == track->getId()) {
            if (child->isSoloed() || isAnyDescendantSoloed(child.get())) {
                return true;
            }
        }
    }
    return false;
}

} // namespace Nimbus
