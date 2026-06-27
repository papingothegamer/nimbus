#include "MidiClipNode.h"

namespace Nimbus {

MidiClipNode::MidiClipNode(std::shared_ptr<MidiClip> clip, ITransport& t)
    : midiClip(std::move(clip)), transport(t) {
}

void MidiClipNode::prepareToPlay(double sampleRate, int /*maximumExpectedSamplesPerBlock*/) {
    sampleRate_ = sampleRate;
}

void MidiClipNode::releaseResources() {
    // Nothing to release
}

void MidiClipNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    if (!transport.isPlaying() || !midiClip)
        return;

    double currentPos = transport.getCurrentPosition();
    double clipStart = midiClip->getStartSample();
    double clipEnd = clipStart + midiClip->getLengthSamples();
    int numSamples = buffer.getNumSamples();

    if (currentPos + numSamples <= clipStart || currentPos >= clipEnd)
        return;

    auto& sequence = midiClip->getSequence();

    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        if (event == nullptr)
            continue;

        auto& msg = event->message;

        // Event timestamp is relative to clip start (in samples)
        double absoluteTime = clipStart + msg.getTimeStamp();
        int sampleOffset = juce::roundToInt(absoluteTime - currentPos);

        if (sampleOffset >= 0 && sampleOffset < numSamples) {
            midiMessages.addEvent(msg, sampleOffset);
        }
    }
}

} // namespace Nimbus
