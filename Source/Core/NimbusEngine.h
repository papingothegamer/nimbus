#pragma once

#include "AudioEngine/PlaybackContext.h"
#include "AudioEngine/AudioDeviceManagerWrapper.h"
#include "AudioEngine/Transport.h"
#include "DataModel/TimelineProject.h"
#include "AudioEngine/DiskStreaming/DiskStreamer.h"
#include "PluginHost/PluginManager.h"
#include "AudioEngine/PluginNode.h"
#include "AudioEngine/AudioRecorder.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

namespace Nimbus {

class DiskStreamer;
class PluginNode;
class MidiRecorder;
class PlaybackEngine;
class ComputerMidiController;

/**
 * The root service container for the Nimbus DAW.
 * Owns the core audio engine components and manages their lifecycles.
 */
class NimbusEngine {
public:
    NimbusEngine();
    ~NimbusEngine();

    void initialise();

    PlaybackContext* getPlaybackContext() const { return playbackContext.get(); }
    Transport& getTransport() { return transport; }
    AudioDeviceManagerWrapper& getAudioDeviceManager() { return deviceManagerWrapper; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }
    juce::AudioThumbnailCache& getThumbnailCache() { return thumbnailCache; }
    TimelineProject& getTimelineProject() { return timelineProject; }
    PluginManager& getPluginManager() { return pluginManager; }
    ComputerMidiController* getComputerMidiController() const { return computerMidiController.get(); }
    PluginNode* getTestPluginNode() const { return testPluginNode.get(); }
    juce::UndoManager& getUndoManager() { return undoManager; }

    bool isFollowPlayheadEnabled() const { return followPlayhead; }
    void setFollowPlayheadEnabled(bool enabled) { followPlayhead = enabled; }
    
    int getSidebarLocation() const { return sidebarLocation; }
    void setSidebarLocation(int loc) {
        sidebarLocation = loc;
        if (onSidebarLocationChanged) onSidebarLocationChanged();
    } // 0 = left, 1 = right

    std::function<void()> onSidebarLocationChanged;

    void addTrack(bool isMidi, bool isStereo = true);
    void duplicateTrack(int trackIndex);
    
    std::pair<float, float> getMasterPeakLevel() const;
    std::pair<float, float> getTrackPeakLevel(int trackIndex) const;

    struct PluginClipboard {
        juce::PluginDescription description;
        juce::ValueTree state;
        bool hasData = false;
    };
    PluginClipboard& getPluginClipboard() { return clipboard; }

    void startRecording();
    void stopRecording();

private:
    std::unique_ptr<PlaybackContext> playbackContext;
    Transport transport;
    AudioDeviceManagerWrapper deviceManagerWrapper;
    bool followPlayhead = true;
    int sidebarLocation = 1; // Default to right
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    PluginManager pluginManager;
    TimelineProject timelineProject;
    juce::UndoManager undoManager;

    juce::TimeSliceThread recorderThread { "RecorderThread" };
    std::map<int, std::unique_ptr<AudioRecorder>> trackRecorders;
    std::map<int, std::unique_ptr<MidiRecorder>> midiRecorders;

    // Temporary storage for our single disk streamer for Phase 4
    std::shared_ptr<DiskStreamer> mainStreamer;

    std::unique_ptr<ComputerMidiController> computerMidiController;

    // Temporary storage for our Phase 5 test plugin
    std::shared_ptr<PluginNode> testPluginNode;
    PluginClipboard clipboard;
};

} // namespace Nimbus
