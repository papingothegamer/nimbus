#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

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

    double getSourceOffsetSamples() const { return sourceOffsetSamples; }
    void setSourceOffsetSamples(double offset) { sourceOffsetSamples = offset; }

    // MIDI Data
    juce::MidiMessageSequence& getSequence() { return sequence; }
    const juce::MidiMessageSequence& getSequence() const { return sequence; }

    void addNote(int channel, int noteNumber, float velocity, double startSample, double lengthSamples);
    
    // Extended properties
    const juce::String& getName() const { return name; }
    void setName(const juce::String& n) { name = n; }
    
    bool getIsLooped() const { return isLooped; }
    void setIsLooped(bool l) { isLooped = l; }
    
    int getColorIndex() const { return colorIndex; }
    void setColorIndex(int c) { colorIndex = c; }
    
private:
    double startSample{0.0};
    double lengthSamples{0.0};
    double sourceOffsetSamples{0.0};

    // We store the raw MIDI events here.
    // Time is in samples relative to the start of the clip.
    juce::MidiMessageSequence sequence;
    juce::String name{"MIDI Clip"};
    bool isLooped = false;
    int colorIndex = -1;     // -1 = inherit from track
};

} // namespace Nimbus
