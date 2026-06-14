#pragma once

#include "AudioClip.h"
#include <vector>
#include <memory>

namespace Nimbus {

/**
 * The data model representing the tracks and clips in the arrangement view.
 */
class TimelineProject {
public:
    TimelineProject() = default;
    ~TimelineProject() = default;

    /**
     * Add a clip to a specific track index. (For now we just store clips linearly and associate them by some ID, 
     * but we'll do a simple list per track).
     */
    void addClipToTrack(int trackIndex, std::shared_ptr<AudioClip> clip);

    /**
     * Get all clips on a specific track.
     */
    std::vector<std::shared_ptr<AudioClip>> getClipsOnTrack(int trackIndex) const;

private:
    // Basic model: track index -> list of clips
    // In a real DAW this would be more complex, but this suffices for Phase 4
    std::vector<std::vector<std::shared_ptr<AudioClip>>> trackClips;
};

} // namespace Nimbus
