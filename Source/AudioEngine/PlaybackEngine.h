#pragma once

#include "DataModel/TimelineProject.h"
#include "AudioEngine/Mixer.h"
#include "AudioEngine/Track.h"
#include "PluginHost/PluginManager.h"
#include <map>
#include <memory>

namespace Nimbus {

/**
 * PlaybackEngine bridges the pure-data TimelineProject with the real-time AudioGraph.
 * It listens to the project and instantiates/routes the IAudioNodes (Mixer, Tracks, Plugins).
 */
class NimbusEngine;

class PlaybackEngine : public TimelineProject::Listener {
public:
    PlaybackEngine(NimbusEngine& engine, TimelineProject& project, Mixer& mixer, PluginManager& pluginManager);
    ~PlaybackEngine() override;

    // TimelineProject::Listener
    void trackAdded(int trackIndex, const TrackModel& track) override;
    void trackRemoved(int trackIndex) override;
    void trackMuteChanged(int trackIndex, bool isMuted) override;
    void trackSoloChanged(int trackIndex, bool isSoloed) override;
    void trackArmChanged(int trackIndex, bool isArmed) override;
    void trackVolumeChanged(int trackIndex, float volume) override;
    void trackPanChanged(int trackIndex, float pan) override;
    void trackInputChannelChanged(int trackIndex, int inputChannel) override;
    void trackClipsChanged(int trackIndex) override;

    Track* getAudioTrack(const TrackID& trackId);

private:
    NimbusEngine& engine;
    TimelineProject& project;
    Mixer& mixer;
    PluginManager& pluginManager;

    // Maps the stable TrackID (as string) to the real-time Track node in the mixer
    std::map<juce::String, Track*> trackNodes;
};

} // namespace Nimbus
