#include "MidiClip.h"

namespace Nimbus {

MidiClip::MidiClip(double start, double length)
    : startSample(start), lengthSamples(length) {
}

void MidiClip::addNote(int channel, int noteNumber, float velocity, double noteStart, double noteLength) {
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
    noteOn.setTimeStamp(noteStart);
    
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(channel, noteNumber, 0.0f);
    noteOff.setTimeStamp(noteStart + noteLength);

    sequence.addEvent(noteOn);
    sequence.addEvent(noteOff);
    sequence.updateMatchedPairs();
}

} // namespace Nimbus
