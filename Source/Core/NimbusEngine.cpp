#include "NimbusEngine.h"
#include "../DataModel/TimelineProject.h"
#include "../AudioEngine/DiskStreaming/DiskStreamer.h"
#include "../AudioEngine/AudioRecorder.h"
#include "../AudioEngine/MidiRecorder.h"
#include "AudioEngine/TestToneNode.h"
#include "AudioEngine/Track.h"
#include "AudioEngine/AudioClipNode.h"
#include "AudioEngine/MidiClipNode.h"
#include "AudioEngine/PluginNode.h"

#include "AudioEngine/PlaybackEngine.h"

namespace Nimbus {

NimbusEngine::NimbusEngine() : deviceManagerWrapper(mainGraph, transport), thumbnailCache(5) {
    juce::Logger::writeToLog("NimbusEngine constructed");
}

NimbusEngine::~NimbusEngine() {
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

    juce::Logger::writeToLog("Engine: Initialising Device Manager");
    // 6. Initialize the audio device manager, which will start pulling audio from the graph
    deviceManagerWrapper.initialise();

    playbackEngine = std::make_unique<PlaybackEngine>(*this, timelineProject, *mixer, pluginManager);

    juce::Logger::writeToLog("Engine: Initialise Complete");
}

void NimbusEngine::addTrack(bool isMidi, bool isStereo) {
    TrackModel model;
    model.name = isMidi ? "MIDI Track" : (isStereo ? "Stereo Audio Track" : "Mono Audio Track");
    model.isMidi = isMidi;
    model.isStereo = isMidi ? true : isStereo; // MIDI tracks generate stereo internally
    
    timelineProject.addTrack(model);
    // Track node is now created by PlaybackEngine observing timelineProject
}

float NimbusEngine::getMasterPeakLevel() const {
    return mixer ? mixer->getMasterPeakLevel() : 0.0f;
}

float NimbusEngine::getTrackPeakLevel(int trackIndex) const {
    return mixer ? mixer->getTrackPeakLevel(trackIndex) : 0.0f;
}

void NimbusEngine::startRecording() {
    recorderThread.startThread();
    
    double sampleRate = transport.getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    for (int i = 0; i < timelineProject.getNumTracks(); ++i) {
        if (timelineProject.isTrackArmed(i)) {
            if (!timelineProject.getTrack(i).isMidi) {
                // Create AudioRecorder
                auto recorder = std::make_unique<AudioRecorder>(recorderThread);
                juce::File tempFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                        .getChildFile("NimbusRecord_" + juce::String(juce::Time::currentTimeMillis()) + ".wav");
                
                // Audio engine output tracks are implicitly 2 channels
                if (recorder->startRecording(tempFile, sampleRate, 2)) {
                    if (auto* mixerTrack = mixer->getTrack(i)) {
                        mixerTrack->setRecorder(recorder.get());
                    }
                    trackRecorders[i] = std::move(recorder);
                }
            } else {
                // Create MidiRecorder
                auto recorder = std::make_unique<MidiRecorder>();
                recorder->startRecording(transport.getCurrentPosition());
                
                if (auto* mixerTrack = mixer->getTrack(i)) {
                    mixerTrack->setMidiRecorder(recorder.get());
                }
                midiRecorders[i] = std::move(recorder);
            }
        }
    }
    
    transport.record();
}

void NimbusEngine::stopRecording() {
    transport.stop();
    
    for (auto& pair : trackRecorders) {
        int trackIdx = pair.first;
        auto& recorder = pair.second;
        
        if (mixer && mixer->getTrack(trackIdx)) {
            mixer->getTrack(trackIdx)->setRecorder(nullptr);
        }
        
        juce::File recordedFile = recorder->stopRecording();
        
        if (recordedFile.existsAsFile()) {
            double sampleRate = transport.getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
            double numSamples = recorder->getNumSamplesRecorded();
            double startPos = transport.getCurrentPosition() - numSamples;
            int startPosInt = static_cast<int>(startPos);
            if (startPosInt < 0) startPosInt = 0;
            
            auto audioClip = std::make_shared<AudioClip>(recordedFile, startPosInt, static_cast<int>(numSamples));
            timelineProject.addClipToTrack(trackIdx, audioClip);
        }
    }
    
    double endPosition = transport.getCurrentPosition();
    for (auto& pair : midiRecorders) {
        int trackIdx = pair.first;
        auto& recorder = pair.second;
        
        if (mixer && mixer->getTrack(trackIdx)) {
            mixer->getTrack(trackIdx)->setMidiRecorder(nullptr);
        }
        
        auto clip = recorder->stopRecordingAndGetClip(endPosition);
        if (clip) {
            timelineProject.addClipToTrack(trackIdx, clip);
        }
    }

    // Ensure all tracks have recorder set to null
    for (int i = 0; i < timelineProject.getNumTracks(); ++i) {
        if (mixer && mixer->getTrack(i)) {
            mixer->getTrack(i)->setRecorder(nullptr);
            mixer->getTrack(i)->setMidiRecorder(nullptr);
        }
    }
    
    trackRecorders.clear();
    midiRecorders.clear();
}

} // namespace Nimbus
