#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace Nimbus {

class Clip {
public:
    enum class Type {
        Audio,
        Midi
    };

    Clip(Type type, double start, double length)
        : clipType(type),
          state("ClipState"),
          startSample(state, "startSample", nullptr, start),
          lengthSamples(state, "lengthSamples", nullptr, length),
          sourceOffsetSamples(state, "sourceOffsetSamples", nullptr, 0.0),
          name(state, "name", nullptr, type == Type::Audio ? "Audio Clip" : "MIDI Clip"),
          colorIndex(state, "colorIndex", nullptr, -1),
          muted(state, "muted", nullptr, false)
    {
    }

    virtual ~Clip() = default;

    virtual std::shared_ptr<Clip> clone() const = 0;

    Type getType() const { return clipType; }
    
    // The central source of truth for thread-safe property binding
    juce::ValueTree state;

    // Common properties stored via CachedValue for lock-free read access
    juce::CachedValue<double> startSample;
    juce::CachedValue<double> lengthSamples;
    juce::CachedValue<double> sourceOffsetSamples;
    juce::CachedValue<juce::String> name;
    juce::CachedValue<int> colorIndex;
    juce::CachedValue<bool> muted;

    double getEndSample() const { return startSample.get() + lengthSamples.get(); }

private:
    Type clipType;
};

} // namespace Nimbus
