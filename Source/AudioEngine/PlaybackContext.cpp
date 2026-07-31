#include "PlaybackContext.h"
#include "Nodes/Node.h"
#include "Nodes/TrackNode.h"
#include "../Core/Plugins/Plugin.h"
#include <juce_core/juce_core.h>

#include "../Core/NimbusEngine.h"
#include "AudioClipNode.h"

namespace Nimbus {

PlaybackContext::PlaybackContext(NimbusEngine& e)
    : engine(e), timelineProject(e.getTimelineProject()), transport(e.getTransport())
{
    for (int i = 0; i < 128; ++i) {
        trackVolumes[i].store(1.0f);
        trackPans[i].store(0.0f);
        trackMutes[i].store(false);
        trackSolos[i].store(false);
    }
    timelineProject.addListener(this);
    rebuildGraph();
}

PlaybackContext::~PlaybackContext()
{
    timelineProject.removeListener(this);
}

void PlaybackContext::rebuildGraph()
{
    // Build new graph on the message thread
    auto newGraph = createGraphFromProject();
    if (newGraph)
        newGraph->prepare(sampleRate, blockSize);
        
    // Atomically swap it in
    pendingGraph = newGraph;
    graphNeedsSwap.store(true, std::memory_order_release);
}

std::shared_ptr<Node> PlaybackContext::createGraphFromProject()
{
    auto rootMixer = std::make_shared<MixerNode>();
    
    int numTracks = timelineProject.getNumTracks();
    for (int i = 0; i < numTracks; ++i) {
        auto track = timelineProject.getTrack(i);
        auto trackNode = std::make_unique<TrackNode>();
        
        if (i < 128) {
            trackVolumes[i].store(track.volume, std::memory_order_relaxed);
            trackPans[i].store(track.pan, std::memory_order_relaxed);
            trackMutes[i].store(track.isMuted, std::memory_order_relaxed);
            trackSolos[i].store(track.isSoloed, std::memory_order_relaxed);
            anySolo.store(hasAnySolo(timelineProject), std::memory_order_relaxed);
            
            trackNode->bindState(&trackVolumes[i], &trackPans[i], &trackMutes[i], &trackSolos[i], &anySolo, &trackLevelMeasurers[i]);
        }
        
        // Add plugins
        if (track.instrumentPlugin) {
            trackNode->addPlugin(track.instrumentPlugin->createNode());
        }
        for (auto& plugin : track.plugins) {
            if (plugin) {
                trackNode->addPlugin(plugin->createNode());
            }
        }
        
        // Add Audio Clips
        for (auto& clipPtr : timelineProject.getClipsOnTrack(i)) {
            if (auto audioClip = std::dynamic_pointer_cast<AudioClip>(clipPtr)) {
                auto streamer = std::make_shared<DiskStreamer>(audioClip->getSourceFile(), engine.getFormatManager());
                auto clipNode = std::make_unique<AudioClipNode>(audioClip, streamer, transport);
                trackNode->addInput(std::move(clipNode));
            }
        }
        
        rootMixer->addInput(std::move(trackNode));
    }
    
    return rootMixer;
}

void PlaybackContext::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                                       float* const* outputChannelData, int numOutputChannels,
                                                       int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    // Check if we need to swap the graph (lock-free)
    if (graphNeedsSwap.exchange(false, std::memory_order_acquire)) {
        activeGraph = pendingGraph;
        pendingGraph.reset();
    }
    
    // Clear outputs
    for (int i = 0; i < numOutputChannels; ++i) {
        if (outputChannelData[i]) {
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
        }
    }
    
    if (activeGraph) {
        juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
        juce::MidiBuffer midiMessages;
        
        ProcessContext processContext;
        processContext.buffer = &buffer;
        processContext.midiMessages = &midiMessages;
        processContext.isPlaying = transport.isPlaying();
        processContext.currentPositionSamples = transport.getCurrentPosition();
        processContext.tempo = transport.getTempo();
        
        activeGraph->process(processContext);
        
        // Simple master volume (could be smoothed in future)
        float currentMasterVolume = engine.getTimelineProject().getMasterVolume();
        if (currentMasterVolume != 1.0f) {
            buffer.applyGain(currentMasterVolume);
        }
        
        // Track master peaks
        masterLevelMeasurer.processBuffer(buffer);
        
        transport.advancePosition(numSamples);
    }
}

void PlaybackContext::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    blockSize = device->getCurrentBufferSizeSamples();
    rebuildGraph();
}

void PlaybackContext::audioDeviceStopped()
{
    activeGraph.reset();
    pendingGraph.reset();
}

void PlaybackContext::trackAdded(int, const TrackModel&) { rebuildGraph(); }
void PlaybackContext::trackRemoved(int) { rebuildGraph(); }
void PlaybackContext::trackClipsChanged(int) { rebuildGraph(); }

void PlaybackContext::trackMuteChanged(int trackIndex, bool isMuted) {
    if (trackIndex >= 0 && trackIndex < 128) trackMutes[trackIndex].store(isMuted, std::memory_order_relaxed);
}

void PlaybackContext::trackSoloChanged(int trackIndex, bool isSoloed) {
    if (trackIndex >= 0 && trackIndex < 128) {
        trackSolos[trackIndex].store(isSoloed, std::memory_order_relaxed);
        anySolo.store(hasAnySolo(timelineProject), std::memory_order_relaxed);
    }
}

void PlaybackContext::trackVolumeChanged(int trackIndex, float volume) {
    if (trackIndex >= 0 && trackIndex < 128) trackVolumes[trackIndex].store(volume, std::memory_order_relaxed);
}

void PlaybackContext::trackPanChanged(int trackIndex, float pan) {
    if (trackIndex >= 0 && trackIndex < 128) trackPans[trackIndex].store(pan, std::memory_order_relaxed);
}

bool PlaybackContext::hasAnySolo(const TimelineProject& project) const {
    for (int i = 0; i < project.getNumTracks(); ++i) {
        if (project.getTrack(i).isSoloed) return true;
    }
    return false;
}

std::pair<float, float> PlaybackContext::getTrackPeakLevel(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < 128) {
        return trackLevelMeasurers[trackIndex].getDecayedPeak(0.85f);
    }
    return {0.0f, 0.0f};
}

std::pair<float, float> PlaybackContext::getMasterPeakLevel() const {
    return masterLevelMeasurer.getDecayedPeak(0.85f);
}

} // namespace Nimbus
