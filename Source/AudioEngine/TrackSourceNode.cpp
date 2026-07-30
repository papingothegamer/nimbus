#include "TrackSourceNode.h"

namespace Nimbus {

TrackSourceNode::TrackSourceNode(Transport& t, AudioFileCache& cache)
    : transport(t), audioFileCache(cache) {
}

TrackSourceNode::~TrackSourceNode() {
    releaseResources();
}

void TrackSourceNode::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;

    const juce::SpinLock::ScopedLockType sl(processLock);
    
    // Pre-allocate mixBuffer for at least 16 channels to support multi-channel/surround outputs without reallocating.
    mixBuffer.setSize(16, maximumExpectedSamplesPerBlock, false, false, false);

    for (auto& node : clipNodes) {
        node->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    }
}

void TrackSourceNode::releaseResources() {
    const juce::SpinLock::ScopedLockType sl(processLock);
    for (auto& node : clipNodes) {
        node->releaseResources();
    }
}

void TrackSourceNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    const juce::SpinLock::ScopedLockType sl(processLock);
    
    // We mix all clip nodes into the track buffer. 
    // They are internally position-aware and will only render if the transport overlaps them.
    for (auto& node : clipNodes) {
        // We use a pre-allocated mix buffer because we are mixing multiple nodes 
        // that might overwrite each other if they share the exact buffer.
        
        mixBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
        mixBuffer.clear();
        
        juce::MidiBuffer tempMidi;
        
        node->processBlock(mixBuffer, tempMidi);
        
        // Mix into the main track buffer
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            if (ch < mixBuffer.getNumChannels()) {
                buffer.addFrom(ch, 0, mixBuffer, ch, 0, buffer.getNumSamples());
            }
        }
        
        midiMessages.addEvents(tempMidi, 0, buffer.getNumSamples(), 0);
    }
}

void TrackSourceNode::updateClips(const std::vector<AnyClipPtr>& newClips) {
    std::vector<std::unique_ptr<IAudioNode>> newNodes;
    
    for (const auto& clipPtr : newClips) {
        if (clipPtr->getType() == Clip::Type::Audio) {
            auto audioClip = std::static_pointer_cast<AudioClip>(clipPtr);
            if (audioClip) {
                auto streamer = audioFileCache.createStream(audioClip->getSourceFile());
                newNodes.push_back(std::make_unique<AudioClipNode>(audioClip, streamer, transport));
            }
        } else if (clipPtr->getType() == Clip::Type::Midi) {
            auto midiClip = std::static_pointer_cast<MidiClip>(clipPtr);
            if (midiClip) {
                newNodes.push_back(std::make_unique<MidiClipNode>(midiClip, transport));
            }
        }
    }

    if (currentSampleRate > 0) {
        for (auto& node : newNodes) {
            node->prepareToPlay(currentSampleRate, currentBlockSize);
        }
    }

    const juce::SpinLock::ScopedLockType sl(processLock);
    clipNodes = std::move(newNodes);
}

} // namespace Nimbus
