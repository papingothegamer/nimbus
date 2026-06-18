#include "NimbusEngine.h"
#include "AudioEngine/TestToneNode.h"
#include "AudioEngine/Track.h"
#include "AudioEngine/AudioClipNode.h"
#include "AudioEngine/MidiClipNode.h"
#include "AudioEngine/PluginNode.h"

namespace Nimbus {

NimbusEngine::NimbusEngine() : deviceManagerWrapper(mainGraph, transport), thumbnailCache(5) {
    juce::Logger::writeToLog("NimbusEngine constructed");
}

NimbusEngine::~NimbusEngine() {
    timelineProject.removeListener(this);
}

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

    juce::Logger::writeToLog("Engine: Initialising Device Manager");
    // 6. Initialize the audio device manager, which will start pulling audio from the graph
    deviceManagerWrapper.initialise();

    timelineProject.addListener(this);

    juce::Logger::writeToLog("Engine: Initialise Complete");
}

void NimbusEngine::addTrack(bool isMidi) {
    TrackModel model;
    model.name = isMidi ? "MIDI Track" : "Audio Track";
    model.isMidi = isMidi;
    
    timelineProject.addTrack(model);

    if (mixer != nullptr) {
        auto track = std::make_unique<Track>();
        // MIDI tracks don't get a source node here - they get one when a clip is added
        mixer->addTrack(std::move(track));
    }
}

float NimbusEngine::getMasterPeakLevel() const {
    return mixer ? mixer->getMasterPeakLevel() : 0.0f;
}

float NimbusEngine::getTrackPeakLevel(int trackIndex) const {
    return mixer ? mixer->getTrackPeakLevel(trackIndex) : 0.0f;
}

void NimbusEngine::trackMuteChanged(int trackIndex, bool isMuted) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setMuted(isMuted);
        }
    }
}

void NimbusEngine::trackRemoved(int trackIndex) {
    if (mixer) {
        mixer->removeTrack(trackIndex);
    }
}

} // namespace Nimbus
