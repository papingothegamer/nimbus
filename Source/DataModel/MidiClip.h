#pragma once

#include "Clip.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace Nimbus {

class MidiClip : public Clip {
public:
    MidiClip(double startSample, double lengthSamples);
    ~MidiClip() override = default;

    std::shared_ptr<Clip> clone() const override;

    // MIDI Data (Kept outside ValueTree for performance, but could be serialized)
    juce::MidiMessageSequence& getSequence() { return sequence; }
    const juce::MidiMessageSequence& getSequence() const { return sequence; }

    void addNote(int channel, int noteNumber, float velocity, double noteStartSample, double noteLengthSamples);

    // Thread-safe CachedValues mapped directly to the base class ValueTree
    juce::CachedValue<bool> isLooped;

private:
    juce::MidiMessageSequence sequence;
};

} // namespace Nimbus
