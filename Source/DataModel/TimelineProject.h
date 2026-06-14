#pragma once

#include "AudioClip.h"
#include <vector>
#include <memory>
#include <juce_events/juce_events.h>

namespace Nimbus {

struct TrackModel {
    juce::String name;
    bool isMidi;
    bool isMuted = false;
    bool isSoloed = false;
};

/**
 * The data model representing the tracks and clips in the arrangement view.
 */
class TimelineProject {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void trackAdded(int trackIndex, const TrackModel& track) {}
        virtual void trackRemoved(int trackIndex) {}
    };

    TimelineProject() = default;
    ~TimelineProject() = default;

    void addListener(Listener* listener) { listeners.add(listener); }
    void removeListener(Listener* listener) { listeners.remove(listener); }

    void addTrack(const TrackModel& track);
    const TrackModel& getTrack(int index) const;
    int getNumTracks() const;

    void addClipToTrack(int trackIndex, std::shared_ptr<AudioClip> clip);
    std::vector<std::shared_ptr<AudioClip>> getClipsOnTrack(int trackIndex) const;

private:
    std::vector<TrackModel> tracks;
    std::vector<std::vector<std::shared_ptr<AudioClip>>> trackClips;
    juce::ListenerList<Listener> listeners;
};

} // namespace Nimbus
