#include "TimelineProject.h"

namespace Nimbus {

void TimelineProject::addClipToTrack(int trackIndex, std::shared_ptr<AudioClip> clip) {
    if (trackIndex >= trackClips.size()) {
        trackClips.resize(trackIndex + 1);
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
