#include "MidiClipNode.h"

namespace Nimbus {

MidiClipNode::MidiClipNode(std::shared_ptr<MidiClip> clip, ITransport& t)
    : midiClip(std::move(clip)), transport(t) {
}

void MidiClipNode::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    // Initialization code here
}

void MidiClipNode::releaseResources() {
    // Release code here
}

void MidiClipNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (!transport.isPlaying() || !midiClip) {
        return;
    }

    double currentPos = transport.getCurrentPosition();
    double clipStart = midiClip->getStartSample();
    double clipEnd = clipStart + midiClip->getLengthSamples();

    int numSamples = buffer.getNumSamples();

    if (currentPos + numSamples > clipStart && currentPos < clipEnd) {
        // Read MIDI events from sequence and add to midiMessages buffer
    }
}

} // namespace Nimbus
