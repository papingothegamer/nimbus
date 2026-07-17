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

void TimelineProject::setTrackName(int trackIndex, const juce::String& newName) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        tracks[trackIndex].name = newName;
        listeners.call(&Listener::trackNameChanged, trackIndex, newName);
    }
}

void TimelineProject::removeTrack(int index)
{
    if (index >= 0 && index < tracks.size())
    {
        bool isGroup = tracks[index].isGroup;
        
        // CRITICAL FIX: Use 'auto' so it correctly identifies as Nimbus::TrackID
        auto groupId = tracks[index].id; 

        // 1. Remove the target track (the group folder itself, or a standard track)
        tracks.erase(tracks.begin() + index);
        
        // Broadcast removal so the UI components shift their indices down
        listeners.call([index](Listener& l) { l.trackRemoved(index); });

        // 2. Cascading Delete for Group Children
        if (isGroup)
        {
            // Because we erased the group, its first child is now sitting exactly at 'index'.
            // We keep erasing at 'index' as long as the track belongs to the deleted group.
            while (index < tracks.size() && tracks[index].parentGroupId == groupId)
            {
                tracks.erase(tracks.begin() + index);
                listeners.call([index](Listener& l) { l.trackRemoved(index); });
            }
        }

        // 3. Broadcast a selection update
        listeners.call([](Listener& l) { l.trackSelectionChanged(); });
    }
}
void TimelineProject::groupTracks(const juce::SparseSet<int>& trackIndices) {
    if (trackIndices.isEmpty()) return;
    
    int firstIndex = trackIndices.getRange(0).getStart();
    
    TrackModel groupTrack;
    groupTrack.id = TrackID();
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
        TrackID groupId = tracks[groupTrackIndex].id;
        for (auto& track : tracks) {
            if (track.parentGroupId == groupId) {
                track.parentGroupId = TrackID();
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



void TimelineProject::setTrackSelected(int trackIndex, bool clearExisting) {
    if (clearExisting) selectedTracks.clear();
    
    if (trackIndex >= 0) {
        selectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
        lastSelectedTrack = trackIndex;
        
        if (clearExisting) { // Only auto-select clip on single-click (not multi-select)
            if (trackIndex < trackClips.size() && !trackClips[trackIndex].empty()) {
                currentSelectedClip = trackClips[trackIndex][0];
                listeners.call(&Listener::selectedClipChanged);
            } else {
                currentSelectedClip = std::shared_ptr<AudioClip>{nullptr};
                listeners.call(&Listener::selectedClipChanged);
            }
        }
    }
    listeners.call(&Listener::trackSelectionChanged);
}

void TimelineProject::setTimeSelection(double startSamples, double endSamples) {
    if (timeSelectionStartSamples != startSamples || timeSelectionEndSamples != endSamples) {
        timeSelectionStartSamples = startSamples;
        timeSelectionEndSamples = endSamples;
        listeners.call(&Listener::timeSelectionChanged);
    }
}

void TimelineProject::setTimeSelectedTracks(const juce::SparseSet<int>& tracks) {
    if (timeSelectedTracks != tracks) {
        timeSelectedTracks = tracks;
        listeners.call(&Listener::timeSelectionChanged);
    }
}

void TimelineProject::addTimeSelectedTrack(int trackIndex) {
    if (!timeSelectedTracks.contains(trackIndex)) {
        timeSelectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
        listeners.call(&Listener::timeSelectionChanged);
    }
}

void TimelineProject::clearTimeSelection() {
    if (timeSelectionStartSamples >= 0 || timeSelectionEndSamples >= 0 || !timeSelectedTracks.isEmpty()) {
        timeSelectionStartSamples = -1.0;
        timeSelectionEndSamples = -1.0;
        timeSelectedTracks.clear();
        listeners.call(&Listener::timeSelectionChanged);
    }
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
    
    std::visit([](auto&& c) {
        if (c->getColorIndex() == -1) {
            c->setColorIndex(juce::Random::getSystemRandom().nextInt(70));
        }
    }, clip);
    
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

void TimelineProject::notifyClipModified() {
    for (size_t trackIndex = 0; trackIndex < trackClips.size(); ++trackIndex) {
        listeners.call(&Listener::trackClipsChanged, static_cast<int>(trackIndex));
    }
}

void TimelineProject::setSelectedClip(AnyClipPtr clip) {
    currentSelectedClip = clip;
    listeners.call(&Listener::selectedClipChanged);
}

AnyClipPtr TimelineProject::getSelectedClip() const {
    return currentSelectedClip;
}

double TimelineProject::getTotalDurationSamples() const {
    double maxDuration = 0.0;
    for (const auto& trackClipList : trackClips) {
        for (const auto& clip : trackClipList) {
            std::visit([&](auto&& c) {
                // c can be shared_ptr<AudioClip> or shared_ptr<MidiClip>
                double clipEnd = static_cast<double>(c->getStartSample()) + static_cast<double>(c->getLengthSamples());
                if (clipEnd > maxDuration) {
                    maxDuration = clipEnd;
                }
            }, clip);
        }
    }
    // Return at least some minimal duration so UI doesn't break, e.g. 5 seconds (5 * 44100 = 220500) if project is empty
    return maxDuration > 0.0 ? maxDuration : 220500.0; 
}

void TimelineProject::copySelectedClips() {
    clipboardClips.clear();
    
    if (currentSelectedClip != AnyClipPtr{}) {
        auto clonedClip = std::visit([](auto&& c) -> AnyClipPtr {
            using T = std::decay_t<decltype(c)>;
            return std::make_shared<typename T::element_type>(*c);
        }, currentSelectedClip);
        clipboardClips.push_back(clonedClip);
    } else if (timeSelectionStartSamples >= 0 && timeSelectionEndSamples >= 0 && timeSelectionStartSamples != timeSelectionEndSamples) {
        // Copy all clips within the time selection for selected tracks
        double minSamples = std::min(timeSelectionStartSamples, timeSelectionEndSamples);
        double maxSamples = std::max(timeSelectionStartSamples, timeSelectionEndSamples);
        
        for (int trackIndex = 0; trackIndex < getNumTracks(); ++trackIndex) {
            if (timeSelectedTracks.contains(trackIndex) || selectedTracks.contains(trackIndex)) {
                for (const auto& clip : trackClips[trackIndex]) {
                    double clipStart = 0;
                    double clipLength = 0;
                    std::visit([&](auto&& c) { clipStart = c->getStartSample(); clipLength = c->getLengthSamples(); }, clip);
                    double clipEnd = clipStart + clipLength;
                    
                    if (clipStart < maxSamples && clipEnd > minSamples) {
                        auto clonedClip = std::visit([](auto&& c) -> AnyClipPtr {
                            using T = std::decay_t<decltype(c)>;
                            return std::make_shared<typename T::element_type>(*c);
                        }, clip);
                        
                        // Adjust the cloned clip to start relative to the selection if we wanted complex pasting,
                        // but for now just copy the exact clip properties
                        clipboardClips.push_back(clonedClip);
                    }
                }
            }
        }
    }
}

void TimelineProject::pasteClips(int trackIndex, double startSample) {
    if (clipboardClips.empty() || trackIndex < 0 || trackIndex >= tracks.size()) return;
    
    // For simplicity, paste the first clip exactly at startSample.
    // If there were multiple clips, we would need to calculate relative offsets.
    double offset = startSample;
    if (clipboardClips.size() > 0) {
        double originalFirstStart = 0;
        std::visit([&](auto&& c) { originalFirstStart = c->getStartSample(); }, clipboardClips[0]);
        
        for (const auto& clip : clipboardClips) {
            auto clonedClip = std::visit([](auto&& c) -> AnyClipPtr {
                using T = std::decay_t<decltype(c)>;
                return std::make_shared<typename T::element_type>(*c);
            }, clip);
            
            double originalStart = 0;
            std::visit([&](auto&& c) { originalStart = c->getStartSample(); }, clonedClip);
            
            double relativeStart = originalStart - originalFirstStart;
            std::visit([&](auto&& c) { c->setStartSample(startSample + relativeStart); }, clonedClip);
            
            addClipToTrack(trackIndex, clonedClip);
        }
    }
}

void TimelineProject::duplicateTrack(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < tracks.size()) {
        TrackModel newTrack = tracks[trackIndex];
        newTrack.id = TrackID(); // Generate new UUID
        newTrack.name = newTrack.name + " (Copy)";
        
        insertTrack(trackIndex + 1, newTrack);
        
        // Copy all clips from the original track to the new track
        auto originalClips = getClipsOnTrack(trackIndex);
        for (const auto& clip : originalClips) {
            auto clonedClip = std::visit([](auto&& c) -> AnyClipPtr {
                using T = std::decay_t<decltype(c)>;
                return std::make_shared<typename T::element_type>(*c);
            }, clip);
            addClipToTrack(trackIndex + 1, clonedClip);
        }
    }
}

// Force rebuild
} // namespace Nimbus
