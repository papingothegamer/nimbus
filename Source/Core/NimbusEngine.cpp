#include "NimbusEngine.h"
#include "AudioEngine/TestToneNode.h"
#include "AudioEngine/Track.h"
#include "AudioEngine/AudioClipNode.h"
#include "AudioEngine/PluginNode.h"

namespace Nimbus {

NimbusEngine::NimbusEngine() : deviceManagerWrapper(mainGraph, transport), thumbnailCache(5) {
    juce::Logger::writeToLog("NimbusEngine constructed");
}

NimbusEngine::~NimbusEngine() = default;

void NimbusEngine::initialise() {
    juce::Logger::writeToLog("Engine: Registering formats");
    // Register WAV/AIFF formats
    formatManager.registerBasicFormats();

    juce::Logger::writeToLog("Engine: Creating Mixer");
    // 1. Create the Mixer and hold a raw pointer for UI access
    auto mixerPtr = std::make_unique<Mixer>();
    mixer = mixerPtr.get();
    mainGraph.addNode(std::move(mixerPtr));

    juce::Logger::writeToLog("Engine: Creating DiskStreamer");
    // 2. Create the main streamer for Phase 4 (test file)
    juce::File testFile(R"(C:\Users\Laptop\Desktop\X26\EAGLP\export_1726306721135.wav)");
    mainStreamer = std::make_shared<DiskStreamer>(testFile, formatManager);

    juce::Logger::writeToLog("Engine: Creating AudioClipNode");
    // 3. Create a clip and the AudioClipNode
    auto clip = std::make_shared<AudioClip>(testFile, 0, 48000 * 60); // 60 seconds long max
    auto clipNode = std::make_unique<AudioClipNode>(clip, mainStreamer, transport);

    juce::Logger::writeToLog("Engine: Adding Track to Mixer");
    // 4. Create a track and add the AudioClipNode as source
    auto track = std::make_unique<Track>();
    track->setSourceNode(std::move(clipNode));
    
    // Add the track to the mixer
    mixer->addTrack(std::move(track));

    juce::Logger::writeToLog("Engine: Adding Clip to TimelineProject");
    // Add clip to the data model
    timelineProject.addClipToTrack(0, clip);



    juce::Logger::writeToLog("Engine: Initialising Device Manager");
    // 6. Initialize the audio device manager, which will start pulling audio from the graph
    deviceManagerWrapper.initialise();

    juce::Logger::writeToLog("Engine: Starting transport");
    // 7. Start transport immediately for testing Phase 4
    transport.play();
    juce::Logger::writeToLog("Engine: Initialise Complete");
}

void NimbusEngine::addTrack(bool isMidi) {
    TrackModel model;
    model.name = isMidi ? "MIDI Track" : "Audio Track";
    model.isMidi = isMidi;
    
    // 1. Add to data model
    timelineProject.addTrack(model);

    // 2. Add to audio engine
    if (mixer != nullptr) {
        mixer->addTrack(std::make_unique<Track>());
    }
}

float NimbusEngine::getMasterPeakLevel() const {
    return mixer ? mixer->getMasterPeakLevel() : 0.0f;
}

float NimbusEngine::getTrackPeakLevel(int trackIndex) const {
    return mixer ? mixer->getTrackPeakLevel(trackIndex) : 0.0f;
}

} // namespace Nimbus
