#include "TimelineProject.h"

namespace Nimbus {

void TimelineProject::addTrack(const TrackModel& track) {
    tracks.push_back(track);
    trackClips.push_back({}); // Empty clips list for the new track
    listeners.call(&Listener::trackAdded, tracks.size() - 1, track);
}

const TrackModel& TimelineProject::getTrack(int index) const {
    return tracks[index];
}

int TimelineProject::getNumTracks() const {
    return tracks.size();
}

void TimelineProject::addClipToTrack(int trackIndex, std::shared_ptr<AudioClip> clip) {
    if (trackIndex >= trackClips.size()) {
        trackClips.resize(trackIndex + 1);
        tracks.resize(trackIndex + 1);
    }
    trackClips[trackIndex].push_back(std::move(clip));
}

std::vector<std::shared_ptr<AudioClip>> TimelineProject::getClipsOnTrack(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < trackClips.size()) {
        return trackClips[trackIndex];
    }
    return {};
}

} // namespace Nimbus
