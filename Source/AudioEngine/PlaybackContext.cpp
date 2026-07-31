#include "PlaybackContext.h"
#include "Nodes/Node.h"
#include "Nodes/TrackNode.h"
#include "../Core/Plugins/Plugin.h"
#include <juce_core/juce_core.h>

namespace Nimbus {

PlaybackContext::PlaybackContext(TimelineProject& project, Transport& t)
    : timelineProject(project), transport(t)
{
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
        trackNode->setVolume(track.volume);
        trackNode->setPan(track.pan);
        trackNode->setMuted(track.isMuted || (!track.isSoloed && hasAnySolo(timelineProject)));
        
        // Add plugins
        if (track.instrumentPlugin) {
            trackNode->addPlugin(track.instrumentPlugin->createNode());
        }
        for (auto& plugin : track.plugins) {
            if (plugin) {
                trackNode->addPlugin(plugin->createNode());
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
void PlaybackContext::trackMuteChanged(int, bool) { rebuildGraph(); }
void PlaybackContext::trackSoloChanged(int, bool) { rebuildGraph(); }
void PlaybackContext::trackVolumeChanged(int, float) { rebuildGraph(); }
void PlaybackContext::trackPanChanged(int, float) { rebuildGraph(); }

bool PlaybackContext::hasAnySolo(const TimelineProject& project) const {
    for (int i = 0; i < project.getNumTracks(); ++i) {
        if (project.getTrack(i).isSoloed) return true;
    }
    return false;
}

} // namespace Nimbus
