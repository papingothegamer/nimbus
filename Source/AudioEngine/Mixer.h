#pragma once

#include "IAudioNode.h"
#include "Track.h"
#include "DSP/GainNode.h"
#include "DSP/LevelMeter.h"
#include <vector>
#include <memory>
#include <juce_core/juce_core.h>
#include <memory>

namespace Nimbus {

/**
 * The Master summing bus.
 * Manages all Tracks in the project and sums them into the final stereo output.
 */
class Mixer : public IAudioNode {
public:
    Mixer();
    ~Mixer() override = default;

    // IAudioNode
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    int getLatencySamples() const override;

    /**
     * Add a track to the mixer. Thread-safe (called from UI).
     */
    void addTrack(std::unique_ptr<Track> track, int insertIndex = -1);

    /**
     * Remove a track from the mixer by index. Thread-safe (called from UI).
     */
    void removeTrack(int index);

    // Master Fader Controls
    void setMasterVolume(float gainLinear);
    void setMasterPan(float panValue);

    // Level Metering
    float getMasterPeakLevel() const { return meter.getPeakLevel(); }
    float getMasterRMSLevel() const { return meter.getRMSLevel(); }
    float getTrackPeakLevel(int index) const {
        const juce::SpinLock::ScopedLockType sl(processLock);
        if (index >= 0 && index < (int)tracks.size()) return tracks[index]->getPeakLevel();
        return 0.0f;
    }
    
    Track* getTrack(int index) const {
        const juce::SpinLock::ScopedLockType sl(processLock);
        if (index >= 0 && index < (int)tracks.size()) return tracks[index].get();
        return nullptr;
    }
    
    juce::SpinLock& getProcessLock() const { return processLock; }

private:
    std::vector<std::unique_ptr<Track>> tracks;
    mutable juce::SpinLock processLock;

    Track* getTrackById(const TrackID& id) const {
        const juce::SpinLock::ScopedLockType sl(processLock);
        for (const auto& t : tracks) {
            if (t->getId() == id) return t.get();
        }
        return nullptr;
    }

    bool isTrackOrAncestorSoloed(Track* track) const;
    bool isAnyDescendantSoloed(Track* track) const;

    GainNode masterFader;
    LevelMeter meter;
    juce::AudioBuffer<float> inputBufferCopy;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};

} // namespace Nimbus
