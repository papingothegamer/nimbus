#include "AudioClipNode.h"

namespace Nimbus {

AudioClipNode::AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStreamer> streamer, Transport& transport)
    : clipModel(std::move(clip)), diskStreamer(std::move(streamer)), globalTransport(transport) {
    if (diskStreamer) {
        timeStretchReader = std::make_unique<TimeStretchReader>(diskStreamer);
    }
}

void AudioClipNode::prepare(double sampleRate, int blockSize) {
    Node::prepare(sampleRate, blockSize);
    if (diskStreamer && !diskStreamer->isThreadRunning()) {
        diskStreamer->requestSeek(clipModel->sourceOffsetSamples.get());
        diskStreamer->startStreaming();
    }
    if (timeStretchReader) {
        timeStretchReader->prepare(sampleRate, blockSize);
    }
}

void AudioClipNode::releaseResources() {
    if (diskStreamer) {
        diskStreamer->stopStreaming();
    }
}

void AudioClipNode::process(const ProcessContext& context) {
    auto* bufferPtr = context.buffer;
    if (!bufferPtr) return;
    auto& buffer = *bufferPtr;
    
    if (!diskStreamer || !diskStreamer->isReady() || !globalTransport.isPlaying()) {
        buffer.clear();
        lastProcessedTransportPos = -1;
        return;
    }

    int currentTransportPos = globalTransport.getCurrentPositionSamples();
    int numSamples = buffer.getNumSamples();

    // Did the transport jump or just start?
    if (lastProcessedTransportPos == -1 || currentTransportPos != lastProcessedTransportPos) {
        // Calculate speed ratio to determine correct seek position in file
        double tempSpeedRatio = clipModel->speedMultiplier.get();
        if (clipModel->isWarped.get()) {
            double dawTempo = globalTransport.getTempo();
            double originalTempo = clipModel->originalBpm.get();
            if (originalTempo <= 0.0) originalTempo = 120.0;
            if (dawTempo > 0.0) {
                tempSpeedRatio = dawTempo / originalTempo;
            }
        }
        tempSpeedRatio = juce::jlimit(0.1, 10.0, tempSpeedRatio);
        
        // Transport seeked!
        if (currentTransportPos >= clipModel->startSample.get()) {
            int relativeFilePos = static_cast<int>(clipModel->sourceOffsetSamples.get() + ((currentTransportPos - clipModel->startSample.get()) * tempSpeedRatio));
            diskStreamer->requestSeek(relativeFilePos);
        } else {
            // If we seeked before the clip, prime the disk streamer to the clip's start
            diskStreamer->requestSeek(clipModel->sourceOffsetSamples.get());
        }
    }

    int clipStart = clipModel->startSample.get();
    int clipEnd = clipStart + clipModel->lengthSamples.get();

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

        double speedRatio = clipModel->speedMultiplier.get();
        if (clipModel->isWarped.get()) {
            double dawTempo = globalTransport.getTempo();
            double originalTempo = clipModel->originalBpm.get();
            if (originalTempo <= 0.0) originalTempo = 120.0;
            if (dawTempo > 0.0) {
                speedRatio = dawTempo / originalTempo;
            }
        }
        
        speedRatio = juce::jlimit(0.1, 10.0, speedRatio);
        
        double pitchRatio = 1.0;
        // Advanced warp MUST only affect clip BPM, NOT clip pitch
        if (!clipModel->isWarped.get() || clipModel->getWarpMode() != AudioClip::WarpMode::Advanced) {
            pitchRatio = std::pow(2.0, static_cast<double>(clipModel->pitchShiftSemitones.get()) / 12.0);
            pitchRatio = juce::jlimit(0.1, 10.0, pitchRatio);
        }
        
        // The position inside the audio file corresponding to the first sample we need to render
        double timeIntoClip = (currentTransportPos + renderStartOffset) - clipStart;
        int filePosition = static_cast<int>(clipModel->sourceOffsetSamples.get() + (timeIntoClip * speedRatio));

        // Clear regions before and after
        if (renderStartOffset > 0) {
            buffer.clear(0, renderStartOffset);
        }
        if (renderStartOffset + renderLength < numSamples) {
            buffer.clear(renderStartOffset + renderLength, numSamples - (renderStartOffset + renderLength));
        }

        if (speedRatio == 1.0 && pitchRatio == 1.0) {
            // Create an alias buffer just for the region we want to render
            juce::AudioBuffer<float> subBuffer(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), renderStartOffset, renderLength);
            diskStreamer->processBlock(subBuffer, filePosition, renderLength);
        } else {
            bool useStretcher = false;
            if (clipModel->isWarped.get()) {
                if (clipModel->preservePitch.get() || clipModel->getWarpMode() == AudioClip::WarpMode::Advanced) {
                    useStretcher = true;
                }
            }
            
            juce::AudioBuffer<float> subBuffer(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), renderStartOffset, renderLength);
            
            if (useStretcher && timeStretchReader) {
                timeStretchReader->setTimeStretchRatio(speedRatio);
                
                double pitchSemis = 0.0;
                if (clipModel->getWarpMode() != AudioClip::WarpMode::Advanced) {
                    pitchSemis = clipModel->pitchShiftSemitones.get();
                }
                timeStretchReader->setPitchShift(pitchSemis);
                
                timeStretchReader->readBlock(subBuffer, filePosition, renderLength, false, 0.0, 0.0);
            } else {
                // Fall back to standard resampling
                double playbackRatio = speedRatio * pitchRatio;
                
                if (playbackRatio <= 0.0) {
                    subBuffer.clear();
                } else {
                    int samplesToRead = static_cast<int>(std::ceil(renderLength * playbackRatio)) + 4; // Add padding for interpolation
                    if (readBuffer.getNumSamples() < samplesToRead || readBuffer.getNumChannels() < buffer.getNumChannels()) {
                        readBuffer.setSize(buffer.getNumChannels(), samplesToRead, false, false, true);
                    }
                    readBuffer.clear();
                    
                    juce::AudioBuffer<float> subReadBuffer(readBuffer.getArrayOfWritePointers(), readBuffer.getNumChannels(), 0, samplesToRead);
                    diskStreamer->processBlock(subReadBuffer, filePosition, samplesToRead);
                    
                    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                        auto* outPtr = subBuffer.getWritePointer(ch);
                        auto* inPtr = readBuffer.getReadPointer(ch);
                        
                        if (ch == 0) interpolatorLeft.process(playbackRatio, inPtr, outPtr, renderLength);
                        else if (ch == 1) interpolatorRight.process(playbackRatio, inPtr, outPtr, renderLength);
                    }
                }
            }
        }

        // Apply fades and gain
        int fadeInSamples = clipModel->fadeInSamples.get();
        int fadeOutSamples = clipModel->fadeOutSamples.get();
        float inCurve = clipModel->fadeInCurve.get();
        float outCurve = clipModel->fadeOutCurve.get();
        bool isXFadeIn = clipModel->isCrossfadingIn.get();
        bool isXFadeOut = clipModel->isCrossfadingOut.get();
        float clipGain = clipModel->gain.get();
        int clipLength = clipModel->lengthSamples.get();

        for (int i = 0; i < renderLength; ++i) {
            int timeIntoClip = (currentTransportPos + renderStartOffset + i) - clipStart;
            float sampleGain = clipGain;
            
            if (timeIntoClip < fadeInSamples && fadeInSamples > 0) {
                float t_fade = static_cast<float>(timeIntoClip) / static_cast<float>(fadeInSamples);
                if (isXFadeIn) {
                    sampleGain *= std::sin(juce::MathConstants<float>::halfPi * t_fade);
                } else {
                    sampleGain *= std::pow(t_fade, inCurve);
                }
            } else if (timeIntoClip > clipLength - fadeOutSamples && fadeOutSamples > 0) {
                float t_fade = static_cast<float>(timeIntoClip - (clipLength - fadeOutSamples)) / static_cast<float>(fadeOutSamples);
                if (isXFadeOut) {
                    sampleGain *= std::cos(juce::MathConstants<float>::halfPi * t_fade);
                } else {
                    sampleGain *= std::pow(1.0f - t_fade, outCurve);
                }
            }
            
            if (sampleGain != 1.0f) {
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                    buffer.getWritePointer(ch)[renderStartOffset + i] *= sampleGain;
                }
            }
        }

    }

    lastProcessedTransportPos = currentTransportPos + numSamples;
}

} // namespace Nimbus
