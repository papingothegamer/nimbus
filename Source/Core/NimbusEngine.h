#pragma once

#include "AudioEngine/AudioGraph.h"
#include "AudioEngine/AudioDeviceManagerWrapper.h"
#include "AudioEngine/Transport.h"
#include "AudioEngine/Mixer.h"
#include "DataModel/TimelineProject.h"
#include "AudioEngine/DiskStreaming/DiskStreamer.h"
#include "PluginHost/PluginManager.h"
#include "AudioEngine/PluginNode.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>

namespace Nimbus {

class PluginNode;

/**
 * The root service container for the Nimbus DAW.
 * Owns the core audio engine components and manages their lifecycles.
 */
class NimbusEngine {
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
    std::shared_ptr<PluginNode> getTestPluginNode() const { return testPluginNode; }

private:
    AudioGraph mainGraph; // The root graph executing on the audio thread
    Mixer* mixer = nullptr; // Raw pointer to the mixer owned by mainGraph
    Transport transport;
    AudioDeviceManagerWrapper deviceManagerWrapper;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    PluginManager pluginManager;
    TimelineProject timelineProject;

    // Temporary storage for our single disk streamer for Phase 4
    std::shared_ptr<DiskStreamer> mainStreamer;

    // Temporary storage for our Phase 5 test plugin
    std::shared_ptr<PluginNode> testPluginNode;
};

} // namespace Nimbus
