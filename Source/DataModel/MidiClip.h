#pragma once

#include <JuceHeader.h>

namespace Nimbus {

/**
 * A clip containing MIDI data to be played back or edited.
 */
class MidiClip {
public:
    MidiClip(double startSample, double clipLengthSamples);
    ~MidiClip() = default;

    // Time info
    double getStartSample() const { return startSample; }
    void setStartSample(double newStart) { startSample = newStart; }
    
    double getLengthSamples() const { return lengthSamples; }
    void setLengthSamples(double newLength) { lengthSamples = newLength; }

    // MIDI Data
    juce::MidiMessageSequence& getSequence() { return sequence; }
    const juce::MidiMessageSequence& getSequence() const { return sequence; }

    void addNote(int channel, int noteNumber, float velocity, double startSample, double lengthSamples);
    
private:
    double startSample{0.0};
    double lengthSamples{0.0};

    // We store the raw MIDI events here.
    // Time is in samples relative to the start of the clip.
    juce::MidiMessageSequence sequence;
};

} // namespace Nimbus
