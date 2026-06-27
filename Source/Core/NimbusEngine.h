#pragma once

#include "AudioEngine/AudioGraph.h"
#include "AudioEngine/AudioDeviceManagerWrapper.h"
#include "AudioEngine/Transport.h"
#include "AudioEngine/Mixer.h"
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

/**
 * The root service container for the Nimbus DAW.
 * Owns the core audio engine components and manages their lifecycles.
 */
class NimbusEngine : public TimelineProject::Listener {
public:
    NimbusEngine();
    ~NimbusEngine();

    void initialise();

    Mixer* getMixer() const { return mixer; }
    Transport& getTransport() { return transport; }
    AudioDeviceManagerWrapper& getAudioDeviceManager() { return deviceManagerWrapper; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }
    juce::AudioThumbnailCache& getThumbnailCache() { return thumbnailCache; }
    TimelineProject& getTimelineProject() { return timelineProject; }
    PluginManager& getPluginManager() { return pluginManager; }
    PluginNode* getTestPluginNode() const { return testPluginNode.get(); }

    bool isFollowPlayheadEnabled() const { return followPlayhead; }
    void setFollowPlayheadEnabled(bool enabled) { followPlayhead = enabled; }

    void addTrack(bool isMidi);

    // TimelineProject::Listener overrides
    void trackMuteChanged(int trackIndex, bool isMuted) override;
    void trackSoloChanged(int trackIndex, bool isSoloed) override;
    void trackArmChanged(int trackIndex, bool isArmed) override;
    void trackVolumeChanged(int trackIndex, float volume) override;
    void trackPanChanged(int trackIndex, float pan) override;
    void trackRemoved(int trackIndex) override;
    void trackClipsChanged(int trackIndex) override;

    float getMasterPeakLevel() const;
    float getTrackPeakLevel(int trackIndex) const;

    struct PluginClipboard {
        juce::PluginDescription description;
        juce::MemoryBlock state;
        bool hasData = false;
    };
    PluginClipboard& getPluginClipboard() { return clipboard; }

    void startRecording();
    void stopRecording();

private:
    AudioGraph mainGraph; // The root graph executing on the audio thread
    Mixer* mixer = nullptr; // Raw pointer to the mixer owned by mainGraph
    Transport transport;
    AudioDeviceManagerWrapper deviceManagerWrapper;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    PluginManager pluginManager;
    TimelineProject timelineProject;

    juce::TimeSliceThread recorderThread { "RecorderThread" };
    std::vector<std::unique_ptr<AudioRecorder>> trackRecorders;
    std::vector<std::unique_ptr<MidiRecorder>> midiRecorders;

    // Temporary storage for our single disk streamer for Phase 4
    std::shared_ptr<DiskStreamer> mainStreamer;

    // Temporary storage for our Phase 5 test plugin
    std::shared_ptr<PluginNode> testPluginNode;
    bool followPlayhead = true;
    PluginClipboard clipboard;
};

} // namespace Nimbus
