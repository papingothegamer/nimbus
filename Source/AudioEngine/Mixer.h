#pragma once

#include "IAudioNode.h"
#include "Track.h"
#include "DSP/GainNode.h"
#include "DSP/LevelMeter.h"
#include "Concurrency/LockFreeQueue.h"
#include <vector>
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
    void addTrack(std::unique_ptr<Track> track);

    // Master Fader Controls
    void setMasterVolume(float gainLinear);
    void setMasterPan(float panValue);

    // Level Metering
    float getMasterPeakLevel() const { return meter.getPeakLevel(); }
    float getMasterRMSLevel() const { return meter.getRMSLevel(); }
    float getTrackPeakLevel(int index) const {
        if (index >= 0 && index < tracks.size()) return tracks[index]->getPeakLevel();
        return 0.0f;
    }
    
    Track* getTrack(int index) const {
        if (index >= 0 && index < tracks.size()) return tracks[index].get();
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<Track>> tracks;
    LockFreeQueue<std::unique_ptr<Track>> trackAddQueue;

    GainNode masterFader;
    LevelMeter meter;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};

} // namespace Nimbus
