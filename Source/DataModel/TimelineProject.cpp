#include "TimelineProject.h"

namespace Nimbus {

void TimelineProject::addTrack(const TrackModel& track) {
    tracks.push_back(track);
    trackClips.push_back({}); // Empty clips list for the new track
    listeners.call(&Listener::trackAdded, tracks.size() - 1, track);
}

void TimelineProject::insertTrack(int index, const TrackModel& track) {
    if (index < 0 || index > tracks.size()) return;
    tracks.insert(tracks.begin() + index, track);
    trackClips.insert(trackClips.begin() + index, {});
    listeners.call(&Listener::trackAdded, index, track);
}

const TrackModel& TimelineProject::getTrack(int index) const {
    return tracks[index];
}

int TimelineProject::getNumTracks() const {
    return tracks.size();
}

void TimelineProject::removeTrack(int index) {
    if (index >= 0 && index < tracks.size()) {
        // Clear selection if the current selected clip belongs to this track
        bool hasSelection = std::visit([](auto&& ptr) { return ptr != nullptr; }, currentSelectedClip);
        if (hasSelection) {
            bool found = false;
            for (auto& clip : trackClips[index]) {
                if (clip == currentSelectedClip) {
                    found = true;
                    break;
                }
            }
            if (found) {
                setSelectedClip(std::shared_ptr<AudioClip>{nullptr});
            }
        }
        
        tracks.erase(tracks.begin() + index);
        trackClips.erase(trackClips.begin() + index);
        
        juce::SparseSet<int> newSelection;
        for (int i = 0; i < selectedTracks.getNumRanges(); ++i) {
            auto range = selectedTracks.getRange(i);
            for (int r = range.getStart(); r < range.getEnd(); ++r) {
                if (r < index) newSelection.addRange(juce::Range<int>(r, r + 1));
                else if (r > index) newSelection.addRange(juce::Range<int>(r - 1, r));
            }
        }
        selectedTracks = newSelection;
        
        listeners.call(&Listener::trackRemoved, index);
        listeners.call(&Listener::trackSelectionChanged);
    }
}

void TimelineProject::groupTracks(const juce::SparseSet<int>& trackIndices) {
    if (trackIndices.isEmpty()) return;
    
    int firstIndex = trackIndices.getRange(0).getStart();
    
    TrackModel groupTrack;
    groupTrack.id = juce::Uuid();
    groupTrack.name = "Group Track";
    groupTrack.isGroup = true;
    groupTrack.isMidi = false;
    
    // Insert directly into data model without triggering trackAdded
    // (group tracks don't have corresponding audio engine tracks)
    tracks.insert(tracks.begin() + firstIndex, groupTrack);
    trackClips.insert(trackClips.begin() + firstIndex, {});
    
    // Set parent IDs after insertion (since indices shift)
    for (int i = 0; i < trackIndices.getNumRanges(); ++i) {
        auto range = trackIndices.getRange(i);
        for (int r = range.getStart(); r < range.getEnd(); ++r) {
            int newIndex = (r >= firstIndex) ? r + 1 : r;
            tracks[newIndex].parentGroupId = groupTrack.id;
        }
    }
    
    listeners.call(&Listener::tracksGrouped);
}

void TimelineProject::ungroupTracks(int groupTrackIndex) {
    if (groupTrackIndex >= 0 && groupTrackIndex < tracks.size() && tracks[groupTrackIndex].isGroup) {
        juce::Uuid groupId = tracks[groupTrackIndex].id;
        for (auto& track : tracks) {
            if (track.parentGroupId == groupId) {
                track.parentGroupId = juce::Uuid::null();
            }
        }
        removeTrack(groupTrackIndex);
        listeners.call(&Listener::tracksGrouped);
    }
}

void TimelineProject::setTrackFolded(int trackIndex, bool isFolded) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].isFolded = isFolded;
        listeners.call(&Listener::trackFoldStateChanged, trackIndex, isFolded);
    }
}

void TimelineProject::setTrackMuted(int trackIndex, bool isMuted) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].isMuted = isMuted;
        listeners.call(&Listener::trackMuteChanged, trackIndex, isMuted);
    }
}

bool TimelineProject::isTrackMuted(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].isMuted;
    }
    return false;
}

void TimelineProject::setTrackArmed(int trackIndex, bool isArmed) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].isArmed = isArmed;
        listeners.call(&Listener::trackArmChanged, trackIndex, isArmed);
    }
}

bool TimelineProject::isTrackArmed(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].isArmed;
    }
    return false;
}

void TimelineProject::setTrackStereo(int trackIndex, bool isStereo) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].isStereo = isStereo;
        listeners.call(&Listener::trackStereoChanged, trackIndex, isStereo);
    }
}

bool TimelineProject::isTrackStereo(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].isStereo;
    }
    return false;
}

void TimelineProject::setTrackSoloed(int trackIndex, bool isSoloed) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].isSoloed = isSoloed;
        listeners.call(&Listener::trackSoloChanged, trackIndex, isSoloed);
    }
}

bool TimelineProject::isTrackSoloed(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].isSoloed;
    }
    return false;
}

void TimelineProject::setTrackVolume(int trackIndex, float volume) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].volume = volume;
        listeners.call(&Listener::trackVolumeChanged, trackIndex, volume);
    }
}

float TimelineProject::getTrackVolume(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].volume;
    }
    return 0.75f;
}

void TimelineProject::setTrackPan(int trackIndex, float pan) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].pan = pan;
        listeners.call(&Listener::trackPanChanged, trackIndex, pan);
    }
}

float TimelineProject::getTrackPan(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].pan;
    }
    return 0.0f;
}

void TimelineProject::setTrackInputChannel(int trackIndex, int inputChannel) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].inputChannelIndex = inputChannel;
        listeners.call(&Listener::trackInputChannelChanged, trackIndex, inputChannel);
    }
}

int TimelineProject::getTrackInputChannel(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        return tracks[trackIndex].inputChannelIndex;
    }
    return -1;
}

void TimelineProject::linkTracks(int trackIndex1, int trackIndex2) {
    if (trackIndex1 >= 0 && trackIndex1 < tracks.size() &&
        trackIndex2 >= 0 && trackIndex2 < tracks.size() &&
        trackIndex1 != trackIndex2) {
        
        tracks[trackIndex1].linkedTrackId = tracks[trackIndex2].id;
        tracks[trackIndex2].linkedTrackId = tracks[trackIndex1].id;
        
        setTrackStereo(trackIndex1, true);
        setTrackStereo(trackIndex2, true);
        
        listeners.call(&Listener::trackSelectionChanged); // To trigger UI updates if necessary
    }
}

void TimelineProject::unlinkTrack(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        juce::Uuid linkedId = tracks[trackIndex].linkedTrackId;
        if (!linkedId.isNull()) {
            tracks[trackIndex].linkedTrackId = juce::Uuid();
            // Find and unlink the other track
            for (auto& track : tracks) {
                if (track.id == linkedId) {
                    track.linkedTrackId = juce::Uuid();
                    break;
                }
            }
            listeners.call(&Listener::trackSelectionChanged);
        }
    }
}

void TimelineProject::setTrackSelected(int trackIndex, bool clearExisting) {
    if (clearExisting) selectedTracks.clear();
    
    if (trackIndex >= 0) {
        selectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
        lastSelectedTrack = trackIndex;
    }
    listeners.call(&Listener::trackSelectionChanged);
}

void TimelineProject::toggleTrackSelection(int trackIndex) {
    if (trackIndex >= 0) {
        if (selectedTracks.contains(trackIndex)) {
            selectedTracks.removeRange(juce::Range<int>(trackIndex, trackIndex + 1));
        } else {
            selectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
            lastSelectedTrack = trackIndex;
        }
        listeners.call(&Listener::trackSelectionChanged);
    }
}

void TimelineProject::selectTrackRange(int fromIndex, int toIndex) {
    int start = std::min(fromIndex, toIndex);
    int end = std::max(fromIndex, toIndex) + 1;
    selectedTracks.clear();
    selectedTracks.addRange(juce::Range<int>(start, end));
    lastSelectedTrack = toIndex;
    listeners.call(&Listener::trackSelectionChanged);
}

bool TimelineProject::isTrackSelected(int trackIndex) const {
    return selectedTracks.contains(trackIndex);
}

void TimelineProject::addClipToTrack(int trackIndex, AnyClipPtr clip) {
    if (trackIndex >= trackClips.size()) {
        trackClips.resize(trackIndex + 1);
        tracks.resize(trackIndex + 1);
    }
    trackClips[trackIndex].push_back(std::move(clip));
    listeners.call(&Listener::trackClipsChanged, trackIndex);
}

void TimelineProject::removeClip(AnyClipPtr clip) {
    for (int i = 0; i < trackClips.size(); ++i) {
        auto& clips = trackClips[i];
        auto it = std::remove(clips.begin(), clips.end(), clip);
        if (it != clips.end()) {
            clips.erase(it, clips.end());
            listeners.call(&Listener::trackClipsChanged, i);
            
            if (currentSelectedClip == clip) {
                setSelectedClip(AnyClipPtr{});
            }
            break;
        }
    }
}

std::vector<AnyClipPtr> TimelineProject::getClipsOnTrack(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < trackClips.size()) {
        return trackClips[trackIndex];
    }
    return {};
}

void TimelineProject::setSelectedClip(AnyClipPtr clip) {
    currentSelectedClip = clip;
    listeners.call(&Listener::selectedClipChanged);
}

AnyClipPtr TimelineProject::getSelectedClip() const {
    return currentSelectedClip;
}

void TimelineProject::notifyClipModified() {
    listeners.call(&Listener::trackClipsChanged, -1);
}

// Force rebuild
} // namespace Nimbus
