#include "AudioClipNode.h"

namespace Nimbus {

AudioClipNode::AudioClipNode(std::shared_ptr<AudioClip> clip, std::shared_ptr<DiskStream> streamer, Transport& transport)
    : audioClip(std::move(clip)), diskStream(std::move(streamer)), transport(transport) {
    syncStateFromModel(); // Initial sync
    startTimerHz(30);     // 30 times a second, sync model safely to atomics
}

AudioClipNode::~AudioClipNode() {
    stopTimer();
}

void AudioClipNode::syncStateFromModel() {
    if (!audioClip) return;
    renderState.startSample.store(audioClip->startSample.get());
    renderState.lengthSamples.store(audioClip->lengthSamples.get());
    renderState.sourceOffsetSamples.store(audioClip->sourceOffsetSamples.get());
    renderState.speedMultiplier.store(audioClip->speedMultiplier.get());
    renderState.pitchShiftSemitones.store(audioClip->pitchShiftSemitones.get());
    renderState.gain.store(audioClip->gain.get());
    renderState.fadeInSamples.store(audioClip->fadeInSamples.get());
    renderState.fadeOutSamples.store(audioClip->fadeOutSamples.get());
    renderState.fadeInCurve.store(audioClip->fadeInCurve.get());
    renderState.fadeOutCurve.store(audioClip->fadeOutCurve.get());
    renderState.isCrossfadingIn.store(audioClip->isCrossfadingIn.get());
    renderState.isCrossfadingOut.store(audioClip->isCrossfadingOut.get());
    renderState.preservePitch.store(audioClip->preservePitch.get());
    renderState.matchDawTempo.store(audioClip->matchDawTempo.get());
    renderState.originalBpm.store(audioClip->originalBpm.get());
}

void AudioClipNode::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    if (diskStream) {
        diskStream->requestSeek(renderState.sourceOffsetSamples.load());
    }
    
    // Default channels to 2 for our typical audio clips
    granularStretcher.prepare(sampleRate, maximumExpectedSamplesPerBlock, 2);
    
    // Pre-allocate readBuffer for up to 10x speed stretch to avoid setSize on audio thread
    int maxExpectedRead = static_cast<int>(std::ceil(maximumExpectedSamplesPerBlock * 10.0)) + 4096;
    readBuffer.setSize(2, maxExpectedRead, false, false, false);
}

void AudioClipNode::releaseResources() {
    // BufferedFileReader stops its thread automatically on destruction
}

void AudioClipNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) {
    if (!diskStream || !diskStream->isReady() || !transport.isPlaying()) {
        buffer.clear();
        lastProcessedTransportPos = -1;
        return;
    }

    int currentTransportPos = transport.getCurrentPositionSamples();
    int numSamples = buffer.getNumSamples();

    // Read atomics locally for consistency across the block
    int clipStart = renderState.startSample.load();
    int clipLength = renderState.lengthSamples.load();
    int clipSourceOffset = renderState.sourceOffsetSamples.load();
    int clipEnd = clipStart + clipLength;

    // Did the transport jump or just start?
    if (lastProcessedTransportPos == -1 || currentTransportPos != lastProcessedTransportPos) {
        // Transport seeked!
        int relativeFilePos = currentTransportPos - clipStart + clipSourceOffset;
        if (relativeFilePos >= 0) {
            diskStream->requestSeek(relativeFilePos);
        } else {
            // If we seeked before the clip, prime the disk streamer to the clip's start
            diskStream->requestSeek(clipSourceOffset);
        }
        
        granularStretcher.reset();
        interpolatorLeft.reset();
        interpolatorRight.reset();
        lastFilePosition = -1;
    }

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

        double speedRatio = renderState.speedMultiplier.load();
        if (renderState.matchDawTempo.load()) {
            double dawTempo = transport.getTempo();
            double originalTempo = renderState.originalBpm.load();
            if (originalTempo > 0.0 && dawTempo > 0.0) {
                speedRatio = dawTempo / originalTempo;
            }
        }
        
        if (lastSpeedRatio != -1.0 && speedRatio != lastSpeedRatio) {
            granularStretcher.reset();
            interpolatorLeft.reset();
            interpolatorRight.reset();
            lastFilePosition = -1;
        }
        lastSpeedRatio = speedRatio;
        
        double pitchRatio = std::pow(2.0, renderState.pitchShiftSemitones.load() / 12.0);
        
        // The position inside the audio file corresponding to the first sample we need to render
        double timeIntoClip = (currentTransportPos + renderStartOffset) - clipStart;
        int filePosition = static_cast<int>(clipSourceOffset + (timeIntoClip * speedRatio));

        int samplesAdvanced = 0;
        if (lastFilePosition != -1) {
            samplesAdvanced = filePosition - lastFilePosition;
        }
        lastFilePosition = filePosition;

        // Clear regions before and after
        if (renderStartOffset > 0) {
            buffer.clear(0, renderStartOffset);
        }
        if (renderStartOffset + renderLength < numSamples) {
            buffer.clear(renderStartOffset + renderLength, numSamples - (renderStartOffset + renderLength));
        }

        if (speedRatio == 1.0 && pitchRatio == 1.0) {
            // Read directly into the region we want to render
            diskStream->readSamples(buffer, renderStartOffset, filePosition, renderLength);
        } else {
            bool preservePitch = renderState.preservePitch.load();
            double playbackRatio = speedRatio * pitchRatio; // Combined ratio
            
            if (preservePitch) {
                // If preserving pitch, the granular stretcher handles speedRatio.
                // It does NOT handle pitchRatio natively yet (we would need to resample the grains for pitchShift),
                // but for now, we will let the granular stretcher handle the speed ratio without pitch change.
                // Assuming pitchShift is 0.0 when matchDawTempo is used (which is our primary goal).
                int samplesToRead = granularStretcher.getNumSourceSamplesRequired(renderLength, speedRatio) + 4;
                if (samplesToRead > readBuffer.getNumSamples()) {
                    samplesToRead = readBuffer.getNumSamples(); // Clamp to avoid reallocation
                }
                readBuffer.clear(); // Could optimize to only clear the used portion, but clear() is fast enough
                
                diskStream->readSamples(readBuffer, 0, filePosition, samplesToRead);
                
                granularStretcher.process(buffer, renderStartOffset, renderLength, readBuffer, 0, samplesToRead, speedRatio, samplesAdvanced);
                
            } else {
                // Not preserving pitch -> standard resampling (speed shift = pitch shift)
                int samplesToRead = static_cast<int>(std::ceil(renderLength * playbackRatio)) + 4; // Add padding for interpolation
                if (samplesToRead > readBuffer.getNumSamples()) {
                    samplesToRead = readBuffer.getNumSamples(); // Clamp to avoid reallocation
                }
                readBuffer.clear();
                
                diskStream->readSamples(readBuffer, 0, filePosition, samplesToRead);
                
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                    auto* outPtr = buffer.getWritePointer(ch) + renderStartOffset;
                    auto* inPtr = readBuffer.getReadPointer(ch);
                    
                    if (ch == 0) interpolatorLeft.process(playbackRatio, inPtr, outPtr, renderLength);
                    else if (ch == 1) interpolatorRight.process(playbackRatio, inPtr, outPtr, renderLength);
                }
            }
        }

        // Apply fades and gain
        int fadeInSamples = renderState.fadeInSamples.load();
        int fadeOutSamples = renderState.fadeOutSamples.load();
        float inCurve = renderState.fadeInCurve.load();
        float outCurve = renderState.fadeOutCurve.load();
        bool isXFadeIn = renderState.isCrossfadingIn.load();
        bool isXFadeOut = renderState.isCrossfadingOut.load();
        float clipGain = renderState.gain.load();

        for (int i = 0; i < renderLength; ++i) {
            int timeIntoClipRel = (currentTransportPos + renderStartOffset + i) - clipStart;
            float sampleGain = clipGain;
            
            if (timeIntoClipRel < fadeInSamples && fadeInSamples > 0) {
                float t_fade = static_cast<float>(timeIntoClipRel) / static_cast<float>(fadeInSamples);
                if (isXFadeIn) {
                    sampleGain *= std::sin(juce::MathConstants<float>::halfPi * t_fade);
                } else {
                    sampleGain *= std::pow(t_fade, inCurve);
                }
            } else if (timeIntoClipRel > clipLength - fadeOutSamples && fadeOutSamples > 0) {
                float t_fade = static_cast<float>(timeIntoClipRel - (clipLength - fadeOutSamples)) / static_cast<float>(fadeOutSamples);
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
