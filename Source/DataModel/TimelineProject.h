#pragma once

#include "AudioClip.h"
#include "MidiClip.h"
#include <vector>
#include <memory>
#include <variant>
#include <juce_events/juce_events.h>

namespace Nimbus {

using AnyClipPtr = std::variant<std::shared_ptr<AudioClip>, std::shared_ptr<MidiClip>>;

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
        virtual void trackClipsChanged(int trackIndex) {}
        virtual void selectedClipChanged() {}
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

    void addClipToTrack(int trackIndex, AnyClipPtr clip);
    std::vector<AnyClipPtr> getClipsOnTrack(int trackIndex) const;
    
    void setSelectedClip(AnyClipPtr clip);
    AnyClipPtr getSelectedClip() const;
    
    void notifyClipModified();

private:
    std::vector<TrackModel> tracks;
    std::vector<std::vector<AnyClipPtr>> trackClips;
    juce::ListenerList<Listener> listeners;
    juce::SparseSet<int> selectedTracks;
    int lastSelectedTrack = -1; // For shift-select logic
    AnyClipPtr currentSelectedClip;
};

} // namespace Nimbus
