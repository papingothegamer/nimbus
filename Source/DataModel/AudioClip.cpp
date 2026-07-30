#include "AudioClip.h"

namespace Nimbus {

AudioClip::AudioClip(const juce::File& file, double startSample, double lengthSamples, double srcLengthSamples)
    : Clip(Type::Audio, startSample, lengthSamples),
      sourceFile(file),
      gain(state, "gain", nullptr, 1.0f),
      pan(state, "pan", nullptr, 0.0f),
      reverse(state, "reverse", nullptr, false),
      sourceLengthSamples(state, "sourceLengthSamples", nullptr, srcLengthSamples >= 0 ? srcLengthSamples : lengthSamples),
      speedMultiplier(state, "speedMultiplier", nullptr, 1.0),
      fadeOutCurve(state, "fadeOutCurve", nullptr, 1.0f),
      isCrossfadingIn(state, "isCrossfadingIn", nullptr, false),
      isCrossfadingOut(state, "isCrossfadingOut", nullptr, false),
      pitchShift(state, "pitchShift", nullptr, 0.0f),
      fadeInSamples(state, "fadeInSamples", nullptr, 192),
      fadeOutSamples(state, "fadeOutSamples", nullptr, 192),
      fadeInCurve(state, "fadeInCurve", nullptr, 1.0f),
      pitchShiftSemitones(state, "pitchShiftSemitones", nullptr, 0),
      matchDawTempo(state, "matchDawTempo", nullptr, false),
      originalBpm(state, "originalBpm", nullptr, 120.0),
      preservePitch(state, "preservePitch", nullptr, false),
      algorithmInt(state, "algorithm", nullptr, static_cast<int>(StretchAlgorithm::Melodic))
{
    juce::Logger::writeToLog("Sample Import: " + file.getFullPathName() + " startSample=" + juce::String(startSample) + " lengthSamples=" + juce::String(lengthSamples));
}

std::shared_ptr<Clip> AudioClip::clone() const {
    auto c = std::make_shared<AudioClip>(sourceFile, startSample.get(), lengthSamples.get(), sourceLengthSamples.get());
    c->sourceOffsetSamples = sourceOffsetSamples.get();
    c->name = name.getValue();
    c->colorIndex = colorIndex.get();
    c->muted = muted.get();
    c->gain = gain.get();
    c->pan = pan.get();
    c->reverse = reverse.get();
    c->fadeInSamples = fadeInSamples.get();
    c->fadeOutSamples = fadeOutSamples.get();
    c->fadeInCurve = fadeInCurve.get();
    c->fadeOutCurve = fadeOutCurve.get();
    c->pitchShift = pitchShift.get();
    c->speedMultiplier = speedMultiplier.get();
    c->pitchShiftSemitones = pitchShiftSemitones.get();
    c->matchDawTempo = matchDawTempo.get();
    c->originalBpm = originalBpm.get();
    c->preservePitch = preservePitch.get();
    c->algorithmInt = algorithmInt.get();
    
    c->detectedTransients = detectedTransients;
    c->warpMarkers = warpMarkers;
    
    return c;
}

void AudioClip::addWarpMarker(double source, double target) {
    // Add marker, keeping the list sorted by sourceSample
    WarpMarker m;
    m.sourceSample = source;
    m.targetSample = target;
    
    auto it = std::lower_bound(warpMarkers.begin(), warpMarkers.end(), m, 
        [](const WarpMarker& a, const WarpMarker& b) { return a.sourceSample < b.sourceSample; });
        
    // Don't add if very close to existing
    if (it != warpMarkers.end() && std::abs(it->sourceSample - source) < 10.0) {
        it->targetSample = target;
    } else {
        warpMarkers.insert(it, m);
    }
}

void AudioClip::removeWarpMarker(int index) {
    if (index >= 0 && index < warpMarkers.size()) {
        warpMarkers.erase(warpMarkers.begin() + index);
    }
}

void AudioClip::updateWarpedLength(double dawBpm) {
    juce::Logger::writeToLog("Tempo Match: dawBpm=" + juce::String(dawBpm) + " originalBpm=" + juce::String(originalBpm.get()) + " matchDawTempo=" + juce::String((int)matchDawTempo.get()));
    if (matchDawTempo.get() && originalBpm.get() > 0.0 && dawBpm > 0.0) {
        double ratio = dawBpm / originalBpm.get();
        double originalLength = sourceLengthSamples.get();
        double targetLength = originalLength / ratio;
        juce::Logger::writeToLog("Tempo Match Calculated: ratio=" + juce::String(ratio) + " originalLength=" + juce::String(originalLength) + " targetLength=" + juce::String(targetLength));
        
        lengthSamples = targetLength;
        speedMultiplier = ratio;
        
        warpMarkers.clear();
        addWarpMarker(0.0, 0.0);
        addWarpMarker(originalLength, targetLength);
    }
}

void AudioClip::cropLeft(double deltaSamples) {
    if (deltaSamples == 0.0) return;
    
    double newStart = startSample.get() + deltaSamples;
    double newLength = lengthSamples.get() - deltaSamples;
    
    double currentSpeed = speedMultiplier.get();
    if (currentSpeed <= 0.0) currentSpeed = 1.0;
    
    double sourceDelta = deltaSamples * currentSpeed;
    double newOffset = sourceOffsetSamples.get() + sourceDelta;
    
    startSample = newStart;
    lengthSamples = newLength > 0.0 ? newLength : 0.0;
    sourceOffsetSamples = std::max(0.0, newOffset);
}

void AudioClip::cropRight(double deltaSamples) {
    if (deltaSamples == 0.0) return;
    
    double newLength = lengthSamples.get() + deltaSamples;
    lengthSamples = newLength > 0.0 ? newLength : 0.0;
}

void AudioClip::shiftOffset(double deltaSamples) {
    if (deltaSamples == 0.0) return;
    
    double newOffset = sourceOffsetSamples.get() + deltaSamples;
    sourceOffsetSamples = std::max(0.0, newOffset);
}

void AudioClip::generateMockTransients(double sampleRate) {
    detectedTransients.clear();
    // Generate a transient roughly every 0.5 seconds
    double step = sampleRate * 0.5;
    double maxLen = sourceLengthSamples.get();
    for (double pos = 0.0; pos < maxLen; pos += step) {
        // Add some jitter
        double jitter = (static_cast<double>(rand()) / RAND_MAX - 0.5) * (sampleRate * 0.1);
        double p = juce::jlimit(0.0, maxLen, pos + jitter);
        detectedTransients.push_back(p);
    }
}

} // namespace Nimbus
