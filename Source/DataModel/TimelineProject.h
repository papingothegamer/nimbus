#pragma once

#include "AudioClip.h"
#include <vector>
#include <memory>
#include <juce_events/juce_events.h>

namespace Nimbus {

struct TrackModel {
    juce::Uuid id;
    juce::String name;
    bool isMidi;
    bool isMuted = false;
    bool isSoloed = false;
    
    // Grouping
    bool isGroup = false;
    bool isFolded = false;
    juce::Uuid parentGroupId;
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
        virtual void trackMuteChanged(int trackIndex, bool isMuted) {}
        virtual void trackSelectionChanged() {}
        virtual void trackFoldStateChanged(int trackIndex, bool isFolded) {}
        virtual void tracksGrouped() {}
    };

    TimelineProject() = default;
    ~TimelineProject() = default;

    void addListener(Listener* listener) { listeners.add(listener); }
    void removeListener(Listener* listener) { listeners.remove(listener); }

    void addTrack(const TrackModel& track);
    void insertTrack(int index, const TrackModel& track);
    const TrackModel& getTrack(int index) const;
    int getNumTracks() const;
    
    void removeTrack(int index);

    void groupTracks(const juce::SparseSet<int>& trackIndices);
    void ungroupTracks(int groupTrackIndex);
    void setTrackFolded(int trackIndex, bool isFolded);

    void setTrackMuted(int trackIndex, bool isMuted);
    bool isTrackMuted(int trackIndex) const;
    
    void setTrackSelected(int trackIndex, bool clearExisting = true);
    void toggleTrackSelection(int trackIndex);
    void selectTrackRange(int fromIndex, int toIndex);
    bool isTrackSelected(int trackIndex) const;
    const juce::SparseSet<int>& getSelectedTracks() const { return selectedTracks; }
    int getLastSelectedTrack() const { return lastSelectedTrack; }

    void addClipToTrack(int trackIndex, std::shared_ptr<AudioClip> clip);
    std::vector<std::shared_ptr<AudioClip>> getClipsOnTrack(int trackIndex) const;

private:
    std::vector<TrackModel> tracks;
    std::vector<std::vector<std::shared_ptr<AudioClip>>> trackClips;
    juce::ListenerList<Listener> listeners;
    juce::SparseSet<int> selectedTracks;
    int lastSelectedTrack = -1; // For shift-select logic
};

} // namespace Nimbus
