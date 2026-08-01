#include "NimbusEngine.h"
#include "../DataModel/TimelineProject.h"
#include "../AudioEngine/DiskStreaming/DiskStreamer.h"
#include "../AudioEngine/AudioRecorder.h"
#include "../AudioEngine/MidiRecorder.h"
#include "AudioEngine/PluginNode.h"
#include "Plugins/StockPluginFactory.h"
#include "Plugins/IStockPlugin.h"
#include "AudioEngine/ComputerMidiController.h"

namespace Nimbus {

NimbusEngine::NimbusEngine() 
    : thumbnailCache(5)
{
    formatManager.registerBasicFormats();
    
    // Create computer midi controller
    computerMidiController = std::make_unique<ComputerMidiController>(*this);
    
    juce::Logger::writeToLog("NimbusEngine constructed");
}

NimbusEngine::~NimbusEngine() {
    playbackContext.reset();
}

void NimbusEngine::initialise() {
    juce::Logger::writeToLog("Engine: Registering formats");
    // Register WAV/AIFF formats
    formatManager.registerBasicFormats();

    juce::Logger::writeToLog("Engine: Creating Playback Context");
    playbackContext = std::make_unique<PlaybackContext>(*this);

    juce::Logger::writeToLog("Engine: Initialising Device Manager");
    // Initialize the audio device manager, and set it to call into our PlaybackContext
    deviceManagerWrapper.initialise();
    deviceManagerWrapper.getDeviceManager().addAudioCallback(playbackContext.get());

    juce::Logger::writeToLog("Engine: Initialise Complete");
}

void NimbusEngine::addTrack(bool isMidi, bool isStereo) {
    TrackModel model;
    model.name = isMidi ? "MIDI Track" : (isStereo ? "Stereo Audio Track" : "Mono Audio Track");
    model.isMidi = isMidi;
    model.isStereo = isMidi ? true : isStereo; 
    
    timelineProject.addTrack(model);
}

void NimbusEngine::duplicateTrack(int trackIndex) {
    // Legacy logic for duplicating track plugins will be re-written
    // when we rebuild the ValueTree track serialization.
    timelineProject.duplicateTrack(trackIndex);
}

std::pair<float, float> NimbusEngine::getMasterPeakLevel() const {
    if (playbackContext) return playbackContext->getMasterPeakLevel();
    return {0.0f, 0.0f};
}

std::pair<float, float> NimbusEngine::getTrackPeakLevel(int trackIndex) const {
    if (playbackContext) return playbackContext->getTrackPeakLevel(trackIndex);
    return {0.0f, 0.0f};
}

MidiRecorder* NimbusEngine::getMidiRecorder(int trackIndex) {
    if (midiRecorders.find(trackIndex) == midiRecorders.end()) {
        midiRecorders[trackIndex] = std::make_unique<MidiRecorder>();
    }
    return midiRecorders[trackIndex].get();
}

void NimbusEngine::startRecording() {
    double currentPos = transport.getCurrentPosition();
    for (int i = 0; i < timelineProject.getNumTracks(); ++i) {
        if (timelineProject.getTrack(i).isArmed) {
            getMidiRecorder(i)->startRecording(currentPos);
        }
    }
    transport.record();
}

void NimbusEngine::stopRecording() {
    transport.stop();
    double currentPos = transport.getCurrentPosition();
    for (int i = 0; i < timelineProject.getNumTracks(); ++i) {
        if (timelineProject.getTrack(i).isArmed) {
            auto clip = getMidiRecorder(i)->stopRecordingAndGetClip(currentPos);
            if (clip) {
                timelineProject.addClipToTrack(i, clip);
            }
        }
    }
}

} // namespace Nimbus
