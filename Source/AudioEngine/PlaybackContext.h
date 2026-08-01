#pragma once

#include <JuceHeader.h>
#include "DataModel/TimelineProject.h"
#include "Nodes/Node.h"
#include "LevelMeasurer.h"
#include <atomic>
#include <memory>
#include <array>
#include <map>

#include "Transport.h"

namespace Nimbus {

/**
 * Compiles and manages the lock-free audio graph for playback.
 * Mirrors Tracktion's te::EditPlaybackContext methodology natively.
 */
class NimbusEngine;

class PlaybackContext : public TimelineProject::Listener, 
                        public juce::AudioIODeviceCallback
{
public:
    PlaybackContext(NimbusEngine& engine);
    ~PlaybackContext() override;

    // Triggered when the graph needs to be rebuilt (e.g. tracks added/removed)
    void rebuildGraph();
    
    std::pair<float, float> getTrackPeakLevel(int trackIndex) const;
    std::pair<float, float> getMasterPeakLevel() const;

    // juce::AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                          float* const* outputChannelData, int numOutputChannels,
                                          int numSamples, const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override {}

    // TimelineProject::Listener overrides to trigger rebuilds
    void trackAdded(int trackIndex, const TrackModel& track) override;
    void trackRemoved(int trackIndex) override;
    void trackMuteChanged(int trackIndex, bool isMuted) override;
    void trackSoloChanged(int trackIndex, bool isSoloed) override;
    void trackArmChanged(int trackIndex, bool isArmed) override;
    void trackPluginsChanged(int trackIndex) override;
    void trackVolumeChanged(int trackIndex, float volume) override;
    void trackPanChanged(int trackIndex, float pan) override;
    void trackClipsChanged(int trackIndex) override;

private:
    NimbusEngine& engine;
    TimelineProject& timelineProject;
    Transport& transport;
    
    // The active lock-free node graph currently being processed by the audio thread
    std::shared_ptr<Node> activeGraph;
    
    // Pending graph to be swapped in
    std::shared_ptr<Node> pendingGraph;
    std::atomic<bool> graphNeedsSwap { false };
    
    // Cache for clip nodes to prevent dropping stream states during graph rebuilds
    std::map<AnyClipPtr, std::shared_ptr<Node>> cachedClipNodes;
    
    double sampleRate = 44100.0;
    int blockSize = 512;
    // Max 128 tracks for simplicity, can be dynamic later
    mutable std::array<LevelMeasurer, 128> trackLevelMeasurers;
    mutable LevelMeasurer masterLevelMeasurer;
    
    std::array<std::atomic<float>, 128> trackVolumes;
    std::array<std::atomic<float>, 128> trackPans;
    std::array<std::atomic<bool>, 128> trackMutes;
    std::array<std::atomic<bool>, 128> trackSolos;
    std::atomic<bool> anySolo { false };
    
    bool hasAnySolo(const TimelineProject& project) const;
    std::shared_ptr<Node> createGraphFromProject();
};

} // namespace Nimbus
