#pragma once

#include "Clip.h"
#include <juce_core/juce_core.h>

namespace Nimbus {

class AudioClip : public Clip {
public:
    enum class StretchAlgorithm {
        Percussive = 0,
        Melodic = 1
    };

    struct WarpMarker {
        double sourceSample = 0.0;
        double targetSample = 0.0;
        bool isSelected = false;
        
        bool operator==(const WarpMarker& other) const {
            return sourceSample == other.sourceSample && targetSample == other.targetSample;
        }
    };

    AudioClip(const juce::File& file, double startSample, double lengthSamples, double sourceLengthSamples = -1.0);
    ~AudioClip() override = default;

    void updateWarpedLength(double currentDawTempo);

    juce::File getFile() const { return sourceFile; }

    std::shared_ptr<Clip> clone() const override;

    const juce::File& getSourceFile() const { return sourceFile; }
    
    int getNumChannels() const { return numChannels; }
    void setNumChannels(int c) { numChannels = c; }

    // Thread-safe CachedValues mapped directly to the base class ValueTree
    juce::CachedValue<float> gain;
    juce::CachedValue<float> pan;
    juce::CachedValue<bool> reverse;
    juce::CachedValue<double> sourceLengthSamples;
    juce::CachedValue<double> speedMultiplier;
    juce::CachedValue<float> fadeOutCurve;
    juce::CachedValue<bool> isCrossfadingIn;
    juce::CachedValue<bool> isCrossfadingOut;
    juce::CachedValue<float> pitchShift;
    
    juce::CachedValue<int> fadeInSamples;
    juce::CachedValue<int> fadeOutSamples;
    juce::CachedValue<float> fadeInCurve;
    juce::CachedValue<int> pitchShiftSemitones;
    juce::CachedValue<bool> matchDawTempo;
    juce::CachedValue<double> originalBpm;
    juce::CachedValue<bool> preservePitch;
    juce::CachedValue<int> algorithmInt; // Cast to/from StretchAlgorithm

    StretchAlgorithm getAlgorithm() const { return static_cast<StretchAlgorithm>(algorithmInt.get()); }
    void setAlgorithm(StretchAlgorithm a) { algorithmInt = static_cast<int>(a); }
    
    // Transients (in source samples)
    std::vector<double> detectedTransients;
    
    // Warp Markers
    std::vector<WarpMarker> warpMarkers;
    
    void addWarpMarker(double source, double target);
    void removeWarpMarker(int index);
    
    // Generate some fake transients for UI testing
    void generateMockTransients(double sampleRate);

private:
    juce::File sourceFile;
    int numChannels = 2;
};

} // namespace Nimbus
