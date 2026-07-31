#pragma once

#include "AudioClip.h"
#include "MidiClip.h"
#include <vector>
#include <memory>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>

#include "Models.h"

namespace Nimbus {

class Plugin;

#include "Clip.h"

using AnyClipPtr = std::shared_ptr<Clip>;

struct TrackModel {
    TrackID id;
    juce::String name;
    TrackType type = TrackType::Audio;
    bool isMidi = false; // Legacy field, eventually remove
    bool isMuted = false;
    bool isSoloed = false;
    bool isArmed = false;
    bool isStereo = false;
    // Routing
    InputSource inputSource;
    TrackID outputBus; // Empty UUID means master
    
    // Grouping
    bool isGroup = false;
    bool isFolded = false;
    TrackID parentGroupId;
    
    float volume = 1.0f; // Linear gain (0 to X)
    float pan = 0.0f;
    
    int inputChannelIndex = -1; // -1 = All active, 0 = Ch1, 1 = Ch2, etc. (Legacy)
    
    // Plugins
    std::vector<std::shared_ptr<Plugin>> plugins;
    std::shared_ptr<Plugin> instrumentPlugin;
    bool hasInstrument = false;
};

struct MarkerModel {
    double positionSamples = 0.0;
    juce::String name;
    juce::Colour color = juce::Colours::white;
};

/**
 * The data model representing the tracks and clips in the arrangement view.
 * Now backed natively by juce::ValueTree for tracktion-style reactivity and thread safety.
 */
class TimelineProject : private juce::ValueTree::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void trackAdded(int trackIndex, const TrackModel& track) {}
        virtual void trackRemoved(int trackIndex) {}
        virtual void trackMuteChanged(int trackIndex, bool isMuted) {}
        virtual void trackArmChanged(int trackIndex, bool isArmed) {}
        virtual void trackStereoChanged(int trackIndex, bool isStereo) {}
        virtual void trackNameChanged(int trackIndex, const juce::String& newName) {}
        virtual void trackMoved(int sourceIndex, int targetIndex) {}
        virtual void trackSelectionChanged() {}
        virtual void timeSelectionChanged() {}
        virtual void trackFoldStateChanged(int trackIndex, bool isFolded) {}
        virtual void tracksGrouped() {}
        virtual void projectNameChanged(const juce::String& newName) {}
        virtual void timeSignatureChanged(int num, int den) {}
        virtual void trackClipsChanged(int trackIndex) {}
        virtual void selectedClipChanged() {}
        virtual void trackSoloChanged(int trackIndex, bool isSoloed) {}
        virtual void trackVolumeChanged(int trackIndex, float volume) {}
        virtual void trackPanChanged(int trackIndex, float pan) {}
        virtual void trackInputChannelChanged(int trackIndex, int inputChannel) {}
        virtual void masterVolumeChanged(float volume) {}
        virtual void markerAdded(int markerIndex, const MarkerModel& marker) {}
        virtual void markerRemoved(int markerIndex) {}
        virtual void markerMoved(int markerIndex, double newPositionSamples) {}
    };

    TimelineProject();
    ~TimelineProject() override;

    juce::ValueTree& getState() { return state; }

    void addListener(Listener* listener) { listeners.add(listener); }
    void removeListener(Listener* listener) { listeners.remove(listener); }

    void addTrack(const TrackModel& track);
    void insertTrack(int index, const TrackModel& track);
    TrackModel getTrack(int index) const; // Return by value now, constructed from ValueTree
    int getNumTracks() const;
    
    void removeTrack(int index);
    void moveTrack(int sourceIndex, int targetIndex);
    
    void setTrackName(int trackIndex, const juce::String& newName);

    void groupTracks(const juce::SparseSet<int>& trackIndices);
    void ungroupTracks(int groupTrackIndex);
    void setTrackFolded(int trackIndex, bool isFolded);

    void setTrackMuted(int trackIndex, bool isMuted);
    bool isTrackMuted(int trackIndex) const;
    
    void setTrackArmed(int trackIndex, bool isArmed);
    bool isTrackArmed(int trackIndex) const;

    void setTrackStereo(int trackIndex, bool isStereo);
    bool isTrackStereo(int trackIndex) const;

    void setTrackSoloed(int trackIndex, bool isSoloed);
    bool isTrackSoloed(int trackIndex) const;

    void setTrackVolume(int trackIndex, float volume);
    float getTrackVolume(int trackIndex) const;

    void setTrackPan(int trackIndex, float pan);
    float getTrackPan(int trackIndex) const;

    void setTrackInputChannel(int trackIndex, int inputChannel);
    int getTrackInputChannel(int trackIndex) const;

    void setMasterVolume(float volume);
    float getMasterVolume() const;

    bool isMultiArmingEnabled() const { return multiArmingEnabled; }
    void setMultiArmingEnabled(bool enabled) { multiArmingEnabled = enabled; }


    void setTrackSelected(int trackIndex, bool clearExisting = true);
    void toggleTrackSelection(int trackIndex);
    void selectTrackRange(int fromIndex, int toIndex);
    bool isTrackSelected(int trackIndex) const;
    void deselectAllTracks();
    const juce::SparseSet<int>& getSelectedTracks() const { return selectedTracks; }
    
    // Time Selection
    void setTimeSelection(double startSamples, double endSamples);
    void setTimeSelectedTracks(const juce::SparseSet<int>& tracks);
    void addTimeSelectedTrack(int trackIndex);
    void clearTimeSelection();
    
    double getTimeSelectionStart() const { return timeSelectionStartSamples; }
    double getTimeSelectionEnd() const { return timeSelectionEndSamples; }
    const juce::SparseSet<int>& getTimeSelectedTracks() const { return timeSelectedTracks; }
    
    // Project Metadata
    juce::String getProjectName() const;
    void setProjectName(const juce::String& name);
    
    juce::String getKeySignature() const;
    void setKeySignature(const juce::String& key);
    
    int getTimeSigNumerator() const;
    void setTimeSigNumerator(int num);
    
    int getTimeSigDenominator() const;
    void setTimeSigDenominator(int den);
    
    int getLastSelectedTrack() const { return lastSelectedTrack; }

    void addClipToTrack(int trackIndex, AnyClipPtr clip);
    void removeClip(AnyClipPtr clip);
    std::vector<AnyClipPtr> getClipsOnTrack(int trackIndex) const;
    void resolveCrossfades(int trackIndex);
    double getTotalDurationSamples() const;
    
    // Plugins
    void addPluginToTrack(int trackIndex, std::shared_ptr<Plugin> plugin);
    void setInstrumentForTrack(int trackIndex, std::shared_ptr<Plugin> plugin);
    
    // Clipboard & Duplication
    void copySelectedClips();
    void pasteClips(int trackIndex, double startSample);
    void duplicateTrack(int trackIndex);

    void notifyClipModified();

    void setSelectedClip(AnyClipPtr clip);
    AnyClipPtr getSelectedClip() const;

    // Markers
    void addMarker(const MarkerModel& marker);
    void removeMarker(int index);
    int getNumMarkers() const;
    MarkerModel getMarker(int index) const;
    void moveMarker(int index, double newPositionSamples);
    void setMarkerName(int index, const juce::String& newName);

private:
    juce::ValueTree state;
    juce::UndoManager undoManager; // Internal for state changes, or passed in

    juce::ListenerList<Listener> listeners;
    juce::SparseSet<int> selectedTracks;
    int lastSelectedTrack = -1; // For shift-select logic
    AnyClipPtr currentSelectedClip;
    std::vector<AnyClipPtr> clipboardClips;
    
    double timeSelectionStartSamples = -1.0;
    double timeSelectionEndSamples = -1.0;
    juce::SparseSet<int> timeSelectedTracks;

    bool multiArmingEnabled = false;

    // We maintain a cache of clips to avoid recreating Clip objects constantly, mimicking Tracktion's ValueTreeObjectList
    std::vector<std::vector<AnyClipPtr>> cachedClips;
    void refreshCachedClips();
    
    std::vector<std::vector<std::shared_ptr<Plugin>>> cachedPlugins;
    std::vector<std::shared_ptr<Plugin>> cachedInstruments;

    // ValueTree::Listener
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;
    void valueTreeParentChanged(juce::ValueTree& treeWhoseParentHasChanged) override {}

    juce::ValueTree getTrackTree(int index) const;
    TrackModel trackModelFromTree(const juce::ValueTree& vt) const;
};

} // namespace Nimbus
