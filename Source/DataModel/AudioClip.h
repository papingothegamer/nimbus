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

    AudioClip(const juce::File& file, double startSample, double lengthSamples, double sourceOffsetSamples = 0.0);
    ~AudioClip() override = default;

    std::shared_ptr<Clip> clone() const override;

    const juce::File& getSourceFile() const { return sourceFile; }
    
    int getNumChannels() const { return numChannels; }
    void setNumChannels(int c) { numChannels = c; }

    // Thread-safe CachedValues mapped directly to the base class ValueTree
    juce::CachedValue<float> gain;
    juce::CachedValue<float> pan;
    juce::CachedValue<bool> reverse;
    juce::CachedValue<int> fadeInSamples;
    juce::CachedValue<int> fadeOutSamples;
    juce::CachedValue<int> fadeInCurve;
    juce::CachedValue<int> fadeOutCurve;
    juce::CachedValue<float> pitchShift;
    
    juce::CachedValue<double> speedMultiplier;
    juce::CachedValue<int> pitchShiftSemitones;
    juce::CachedValue<bool> matchDawTempo;
    juce::CachedValue<double> originalBpm;
    juce::CachedValue<bool> preservePitch;
    juce::CachedValue<int> algorithmInt; // Cast to/from StretchAlgorithm

    StretchAlgorithm getAlgorithm() const { return static_cast<StretchAlgorithm>(algorithmInt.get()); }
    void setAlgorithm(StretchAlgorithm a) { algorithmInt = static_cast<int>(a); }

private:
    juce::File sourceFile;
    int numChannels = 2;
};

} // namespace Nimbus
