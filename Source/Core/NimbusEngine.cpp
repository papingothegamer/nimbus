#include "NimbusEngine.h"
#include "AudioEngine/TestToneNode.h"
#include "AudioEngine/Track.h"
#include "AudioEngine/AudioClipNode.h"
#include "AudioEngine/PluginNode.h"

namespace Nimbus {

NimbusEngine::NimbusEngine() : deviceManagerWrapper(mainGraph, transport), thumbnailCache(5) {}

NimbusEngine::~NimbusEngine() = default;

void NimbusEngine::initialise() {
    // Register WAV/AIFF formats
    formatManager.registerBasicFormats();

    // 1. Create the Mixer and hold a raw pointer for UI access
    auto mixerPtr = std::make_unique<Mixer>();
    mixer = mixerPtr.get();
    mainGraph.addNode(std::move(mixerPtr));

    // 2. Create the main streamer for Phase 4 (test file)
    juce::File testFile(R"(C:\Users\Laptop\Desktop\X26\EAGLP\export_1726306721135.wav)");
    mainStreamer = std::make_shared<DiskStreamer>(testFile, formatManager);

    // 3. Create a clip and the AudioClipNode
    auto clip = std::make_shared<AudioClip>(testFile, 0, 48000 * 60); // 60 seconds long max
    auto clipNode = std::make_unique<AudioClipNode>(clip, mainStreamer, transport);

    // 4. Create a track and add the AudioClipNode as source
    auto track = std::make_unique<Track>();
    track->setSourceNode(std::move(clipNode));
    
    // Add the track to the mixer
    mixer->addTrack(std::move(track));

    // Add clip to the data model
    timelineProject.addClipToTrack(0, clip);

    // 5. Load Phase 5 Test Plugin
    juce::String pluginError;
    auto pluginInstance = pluginManager.loadPlugin(R"(C:\Program Files\Common Files\VST3\uaudio_pultec_eqp-1a.vst3)", pluginError);
    if (pluginInstance) {
        testPluginNode = std::make_shared<PluginNode>(std::move(pluginInstance));
        // We will just hold onto testPluginNode so the MainWindow can show its UI.
    }

    // 6. Initialize the audio device manager, which will start pulling audio from the graph
    deviceManagerWrapper.initialise();

    // 7. Start transport immediately for testing Phase 4
    transport.play();
}

} // namespace Nimbus
