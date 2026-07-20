#pragma once

#include "AudioClip.h"
#include "MidiClip.h"
#include <vector>
#include <memory>
#include <variant>
#include <juce_events/juce_events.h>

#include "Models.h"

namespace Nimbus {

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
    
    float volume = 0.75f;
    float pan = 0.0f;
    
    int inputChannelIndex = -1; // -1 = All active, 0 = Ch1, 1 = Ch2, etc. (Legacy)
    
    // Plugins
    std::vector<PluginSlot> pluginSlots;
    PluginSlot instrumentSlot;
    bool hasInstrument = false;
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
        virtual void trackArmChanged(int trackIndex, bool isArmed) {}
        virtual void trackStereoChanged(int trackIndex, bool isStereo) {}
        virtual void trackNameChanged(int trackIndex, const juce::String& newName) {}
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
    const juce::String& getProjectName() const { return projectName; }
    void setProjectName(const juce::String& name) { 
        projectName = name; 
        listeners.call(&Listener::projectNameChanged, name);
    }
    
    const juce::String& getKeySignature() const { return keySignature; }
    void setKeySignature(const juce::String& key) { keySignature = key; }
    
    int getTimeSigNumerator() const { return timeSigNumerator; }
    void setTimeSigNumerator(int num) { 
        timeSigNumerator = num; 
        listeners.call(&Listener::timeSignatureChanged, num, timeSigDenominator);
    }
    
    int getTimeSigDenominator() const { return timeSigDenominator; }
    void setTimeSigDenominator(int den) { 
        timeSigDenominator = den; 
        listeners.call(&Listener::timeSignatureChanged, timeSigNumerator, den);
    }
    
    int getLastSelectedTrack() const { return lastSelectedTrack; }

    void addClipToTrack(int trackIndex, AnyClipPtr clip);
    void removeClip(AnyClipPtr clip);
    std::vector<AnyClipPtr> getClipsOnTrack(int trackIndex) const;
    double getTotalDurationSamples() const;
    
    // Clipboard & Duplication
    void copySelectedClips();
    void pasteClips(int trackIndex, double startSample);
    void duplicateTrack(int trackIndex);

    void notifyClipModified();

    void setSelectedClip(AnyClipPtr clip);
    AnyClipPtr getSelectedClip() const;

private:
    std::vector<TrackModel> tracks;
    std::vector<std::vector<AnyClipPtr>> trackClips;
    juce::ListenerList<Listener> listeners;
    juce::SparseSet<int> selectedTracks;
    int lastSelectedTrack = -1; // For shift-select logic
    AnyClipPtr currentSelectedClip;
    std::vector<AnyClipPtr> clipboardClips;
    
    double timeSelectionStartSamples = -1.0;
    double timeSelectionEndSamples = -1.0;
    juce::SparseSet<int> timeSelectedTracks;

    juce::String projectName = "Untitled Project";
    juce::String keySignature = "C MAJ";
    int timeSigNumerator = 4;
    int timeSigDenominator = 4;
};

} // namespace Nimbus
