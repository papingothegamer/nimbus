#include "MidiClip.h"

namespace Nimbus {

MidiClip::MidiClip(double startSampleValue, double lengthSamplesValue)
    : Clip(Type::Midi, startSampleValue, lengthSamplesValue),
      isLooped(state, "isLooped", nullptr, false)
{
}

void MidiClip::addNote(int channel, int noteNumber, float velocity, double noteStartSample, double noteLengthSamples) {
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
    noteOn.setTimeStamp(noteStartSample);
    
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(channel, noteNumber, 0.0f);
    noteOff.setTimeStamp(noteStartSample + noteLengthSamples);

    sequence.addEvent(noteOn);
    sequence.addEvent(noteOff);
    sequence.updateMatchedPairs();
}

std::shared_ptr<Clip> MidiClip::clone() const {
    auto c = std::make_shared<MidiClip>(startSample.get(), lengthSamples.get());
    c->state.copyPropertiesFrom(this->state, nullptr);
    c->sequence = this->sequence;
    return c;
}

} // namespace Nimbus
