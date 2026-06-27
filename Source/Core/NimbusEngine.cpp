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
    model.isStereo = !isMidi; // Audio track defaults to stereo
    
    timelineProject.addTrack(model);

    if (mixer != nullptr) {
        auto track = std::make_unique<Track>(&transport);
        track->setInputChannelIndex(model.inputChannelIndex);
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

void NimbusEngine::trackSoloChanged(int trackIndex, bool isSoloed) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setSoloed(isSoloed);
        }
    }
}

void NimbusEngine::trackVolumeChanged(int trackIndex, float volume) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setVolume(volume);
        }
    }
}

void NimbusEngine::trackPanChanged(int trackIndex, float pan) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setPan(pan);
        }
    }
}

void NimbusEngine::trackInputChannelChanged(int trackIndex, int inputChannel) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setInputChannelIndex(inputChannel);
        }
    }
}

void NimbusEngine::trackArmChanged(int trackIndex, bool isArmed) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            track->setArmed(isArmed);
        }
    }
}

void NimbusEngine::trackClipsChanged(int trackIndex) {
    if (mixer) {
        if (auto* track = mixer->getTrack(trackIndex)) {
            const auto& trackModel = timelineProject.getTrack(trackIndex);
            
            // For now, we just grab the first clip and assign it to the track's source node
            // A full implementation would use a TrackClipManagerNode
            if (timelineProject.getClipsOnTrack(trackIndex).size() > 0) {
                const auto& anyClip = timelineProject.getClipsOnTrack(trackIndex).front();
                
                if (std::holds_alternative<std::shared_ptr<MidiClip>>(anyClip)) {
                    auto midiClip = std::get<std::shared_ptr<MidiClip>>(anyClip);
                    auto clipNode = std::make_unique<MidiClipNode>(midiClip, transport);
                    track->setSourceNode(std::move(clipNode));
                } else if (std::holds_alternative<std::shared_ptr<AudioClip>>(anyClip)) {
                    auto audioClip = std::get<std::shared_ptr<AudioClip>>(anyClip);
                    auto streamer = std::make_shared<DiskStreamer>(audioClip->getSourceFile(), formatManager);
                    auto clipNode = std::make_unique<AudioClipNode>(audioClip, streamer, transport);
                    track->setSourceNode(std::move(clipNode));
                }
            } else {
                track->setSourceNode(nullptr); // No clips
            }
        }
    }
}

void NimbusEngine::trackRemoved(int trackIndex) {
    if (mixer) {
        mixer->removeTrack(trackIndex);
    }
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
