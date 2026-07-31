#pragma once

#include <JuceHeader.h>
#include "DataModel/TimelineProject.h"
#include "Nodes/Node.h"
#include <atomic>
#include <memory>

#include "Transport.h"

namespace Nimbus {

/**
 * Compiles and manages the lock-free audio graph for playback.
 * Mirrors Tracktion's te::EditPlaybackContext methodology natively.
 */
class PlaybackContext : public TimelineProject::Listener, 
                        public juce::AudioIODeviceCallback
{
public:
    PlaybackContext(TimelineProject& project, Transport& transport);
    ~PlaybackContext() override;

    // Triggered when the graph needs to be rebuilt (e.g. tracks added/removed)
    void rebuildGraph();

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
    void trackVolumeChanged(int trackIndex, float volume) override;
    void trackPanChanged(int trackIndex, float pan) override;

private:
    TimelineProject& timelineProject;
    Transport& transport;
    
    // The active lock-free node graph currently being processed by the audio thread
    std::shared_ptr<Node> activeGraph;
    
    // Pending graph to be swapped in
    std::shared_ptr<Node> pendingGraph;
    std::atomic<bool> graphNeedsSwap { false };
    
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    bool hasAnySolo(const TimelineProject& project) const;
    std::shared_ptr<Node> createGraphFromProject();
};

} // namespace Nimbus
