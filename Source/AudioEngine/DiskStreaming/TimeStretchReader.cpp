#include "TimeStretchReader.h"
#include <cmath>

namespace Nimbus {

TimeStretchReader::TimeStretchReader(std::shared_ptr<DiskStreamer> sourceStreamer)
    : streamer(std::move(sourceStreamer))
{
}

void TimeStretchReader::prepare(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    stretcher.presetCheaper(2, currentSampleRate, true); // Assuming stereo for now
    stretcher.reset();
}

void TimeStretchReader::setPitchShift(double semitones) {
    if (pitchShiftSemitones != semitones) {
        pitchShiftSemitones = semitones;
        stretcher.setTransposeSemitones(pitchShiftSemitones);
    }
}

void TimeStretchReader::setTimeStretchRatio(double ratio) {
    timeRatio = ratio;
}

void TimeStretchReader::readBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples, bool shouldLoop, double loopStart, double loopEnd) {
    if (isFirstRead || std::abs(startSample - currentSourcePosition) > 8192) {
        stretcher.reset();
        currentSourcePosition = startSample;
        isFirstRead = false;
    }
    
    int inputSamplesNeeded = static_cast<int>(std::round(numSamples * timeRatio));
    
    // We must ensure the inputBuffer is large enough
    inputBuffer.setSize(buffer.getNumChannels(), std::max(inputSamplesNeeded, 1), false, false, true);
    inputBuffer.clear();
    
    if (inputSamplesNeeded > 0) {
        streamer->processBlock(inputBuffer, static_cast<int>(currentSourcePosition), inputSamplesNeeded);
    }
    
    inputChannels.resize(buffer.getNumChannels());
    outputChannels.resize(buffer.getNumChannels());
    
    for (int i = 0; i < buffer.getNumChannels(); ++i) {
        inputChannels[i] = inputBuffer.getReadPointer(i);
        outputChannels[i] = buffer.getWritePointer(i);
    }
    
    // Process through Signalsmith
    stretcher.process(inputChannels.data(), inputSamplesNeeded, outputChannels.data(), numSamples);
    
    currentSourcePosition += inputSamplesNeeded;
}

} // namespace Nimbus
