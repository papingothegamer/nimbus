#include "GranularTimeStretcher.h"

namespace Nimbus {

GranularTimeStretcher::GranularTimeStretcher() {
    // 50ms grain at 44.1kHz is ~2205 samples. Use 2048 for good power of 2 alignment.
    grainSizeSamples = 2048; 
}

void GranularTimeStretcher::prepare(double sampleRate, int /*maxBlockSize*/, int numChannels) {
    currentSampleRate = sampleRate;
    channels = numChannels;
    
    // Roughly 50ms grain size
    grainSizeSamples = static_cast<int>(0.05 * sampleRate);
    if (grainSizeSamples % 2 != 0) grainSizeSamples++; // Ensure even
    
    overlapBuffer.setSize(channels, grainSizeSamples * 2);
    overlapBuffer.clear();
    
    // Create Hanning window
    window.resize(grainSizeSamples);
    for (int i = 0; i < grainSizeSamples; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * i / (grainSizeSamples - 1)));
    }
    
    reset();
}

void GranularTimeStretcher::reset() {
    sourcePhase = 0.0;
    outputPhase = 0;
    overlapBuffer.clear();
}

int GranularTimeStretcher::getNumSourceSamplesRequired(int numOutputSamples, double speedRatio) const {
    // To produce numOutputSamples, we need to advance outputPhase by numOutputSamples.
    // The sourcePhase will advance by numOutputSamples * speedRatio.
    // We need enough source samples to cover the new sourcePhase + grainSizeSamples.
    return static_cast<int>(std::ceil(numOutputSamples * speedRatio)) + grainSizeSamples;
}

void GranularTimeStretcher::process(juce::AudioBuffer<float>& outputBuffer, int outputStartSample, int outputNumSamples,
                                    const juce::AudioBuffer<float>& sourceBuffer, int sourceStartSample, int sourceNumSamples,
                                    double speedRatio, int samplesAdvancedByCaller) {
    int actualChannels = std::min(channels, std::min(outputBuffer.getNumChannels(), sourceBuffer.getNumChannels()));
    if (speedRatio == 1.0) {
        // Just pass through if no stretching is needed
        for (int ch = 0; ch < actualChannels; ++ch) {
            outputBuffer.copyFrom(ch, outputStartSample, sourceBuffer, ch, sourceStartSample, outputNumSamples);
        }
        sourcePhase = 0.0; // Keep reset so it doesn't drift negative
        return;
    }

    sourcePhase -= samplesAdvancedByCaller;

    int outputWritten = 0;
    int hopSize = grainSizeSamples / 4; // 75% overlap for smoother sound

    while (outputWritten < outputNumSamples) {
        // Check if we need to generate a new grain
        if (outputPhase == 0) {
            int currentSourceInt = static_cast<int>(std::max(0.0, sourcePhase));
            
            // Only generate if we have enough source samples left
            if (currentSourceInt + grainSizeSamples <= sourceNumSamples) {
                // Add new grain to overlap buffer
                for (int ch = 0; ch < actualChannels; ++ch) {
                    auto* overlapPtr = overlapBuffer.getWritePointer(ch);
                    auto* sourcePtr = sourceBuffer.getReadPointer(ch, sourceStartSample + currentSourceInt);
                    
                    for (int i = 0; i < grainSizeSamples; ++i) {
                        overlapPtr[i] += sourcePtr[i] * window[i];
                    }
                }
            }
        }

        // Copy from overlap buffer to output
        int samplesToCopy = std::min(hopSize - outputPhase, outputNumSamples - outputWritten);
        
        for (int ch = 0; ch < actualChannels; ++ch) {
            outputBuffer.copyFrom(ch, outputStartSample + outputWritten, overlapBuffer, ch, outputPhase, samplesToCopy);
        }

        outputWritten += samplesToCopy;
        outputPhase += samplesToCopy;
        
        sourcePhase += (samplesToCopy * speedRatio);

        // If we finished a hop, shift overlap buffer down and advance phases
        if (outputPhase >= hopSize) {
            for (int ch = 0; ch < channels; ++ch) {
                // Shift down
                auto* overlapWrite = overlapBuffer.getWritePointer(ch);
                auto* overlapRead = overlapBuffer.getReadPointer(ch, hopSize);
                int shiftAmount = overlapBuffer.getNumSamples() - hopSize;
                
                for (int i = 0; i < shiftAmount; ++i) {
                    overlapWrite[i] = overlapRead[i];
                }
                
                // Clear the rest
                for (int i = shiftAmount; i < overlapBuffer.getNumSamples(); ++i) {
                    overlapWrite[i] = 0.0f;
                }
            }
            
            outputPhase = 0;
        }
    }
}

} // namespace Nimbus
