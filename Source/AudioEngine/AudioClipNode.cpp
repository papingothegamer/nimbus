#include "AudioClipNode.h"

namespace Nimbus {

AudioClipNode::AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStreamer> streamer, Transport& transport)
    : clipModel(std::move(clip)), diskStreamer(std::move(streamer)), globalTransport(transport) {
}

void AudioClipNode::prepareToPlay(double /*sampleRate*/, int /*maximumExpectedSamplesPerBlock*/) {
    if (diskStreamer && !diskStreamer->isThreadRunning()) {
        diskStreamer->startStreaming();
    }
}

void AudioClipNode::releaseResources() {
    if (diskStreamer) {
        diskStreamer->stopStreaming();
    }
}

void AudioClipNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (!diskStreamer || !diskStreamer->isReady() || !globalTransport.isPlaying()) {
        buffer.clear();
        lastProcessedTransportPos = -1;
        return;
    }

    int currentTransportPos = globalTransport.getCurrentPositionSamples();
    int numSamples = buffer.getNumSamples();

    // Did the transport jump or just start?
    if (lastProcessedTransportPos == -1 || currentTransportPos != lastProcessedTransportPos) {
        // Transport seeked!
        int relativeFilePos = currentTransportPos - clipModel->getStartSample() + clipModel->getSourceOffsetSamples();
        if (relativeFilePos >= 0) {
            diskStreamer->requestSeek(relativeFilePos);
        }
    }

    int clipStart = clipModel->getStartSample();
    int clipEnd = clipModel->getEndSample();

    // Check if the current block overlaps with the clip
    if (currentTransportPos + numSamples <= clipStart || currentTransportPos >= clipEnd) {
        // Completely outside the clip
        buffer.clear();
    } else {
        // We overlap! We might need to render a partial block if we are entering or leaving the clip
        int renderStartOffset = 0;
        int renderLength = numSamples;

        if (currentTransportPos < clipStart) {
            renderStartOffset = clipStart - currentTransportPos;
            renderLength -= renderStartOffset;
        }

        if (currentTransportPos + numSamples > clipEnd) {
            renderLength -= (currentTransportPos + numSamples - clipEnd);
        }

        double ratio = 1.0;
        if (clipModel->isWarpEnabled() && clipModel->getMatchDawTempo()) {
            double dawTempo = globalTransport.getTempo();
            double originalTempo = clipModel->getOriginalBpm();
            if (originalTempo > 0.0 && dawTempo > 0.0) {
                ratio = dawTempo / originalTempo;
            }
        }

        // The position inside the audio file corresponding to the first sample we need to render
        double timeIntoClip = (currentTransportPos + renderStartOffset) - clipStart;
        int filePosition = static_cast<int>(clipModel->getSourceOffsetSamples() + (timeIntoClip * ratio));

        // Clear regions before and after
        if (renderStartOffset > 0) {
            buffer.clear(0, renderStartOffset);
        }
        if (renderStartOffset + renderLength < numSamples) {
            buffer.clear(renderStartOffset + renderLength, numSamples - (renderStartOffset + renderLength));
        }

        if (ratio == 1.0) {
            // Create an alias buffer just for the region we want to render
            juce::AudioBuffer<float> subBuffer(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), renderStartOffset, renderLength);
            diskStreamer->processBlock(subBuffer, filePosition, renderLength);
        } else {
            int samplesToRead = static_cast<int>(std::ceil(renderLength * ratio)) + 4; // Add padding for interpolation
            if (readBuffer.getNumSamples() < samplesToRead || readBuffer.getNumChannels() < buffer.getNumChannels()) {
                readBuffer.setSize(buffer.getNumChannels(), samplesToRead, false, false, true);
            }
            readBuffer.clear();
            
            juce::AudioBuffer<float> subReadBuffer(readBuffer.getArrayOfWritePointers(), readBuffer.getNumChannels(), 0, samplesToRead);
            diskStreamer->processBlock(subReadBuffer, filePosition, samplesToRead);
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                auto* outPtr = buffer.getWritePointer(ch) + renderStartOffset;
                auto* inPtr = readBuffer.getReadPointer(ch);
                
                if (ch == 0) interpolatorLeft.process(ratio, inPtr, outPtr, renderLength);
                else if (ch == 1) interpolatorRight.process(ratio, inPtr, outPtr, renderLength);
            }
        }

    }

    lastProcessedTransportPos = currentTransportPos + numSamples;
}

} // namespace Nimbus
