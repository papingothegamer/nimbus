#pragma once

#include "Nodes/Node.h"
#include "DataModel/MidiClip.h"
#include "Transport.h"
#include <JuceHeader.h>
#include <memory>

namespace Nimbus {

/**
 * An audio node that renders a MidiClip by generating MIDI messages.
 * Position-aware via the global Transport.
 */
class MidiClipNode : public Node {
public:
    MidiClipNode(std::shared_ptr<MidiClip> clip);
    ~MidiClipNode() override = default;

    // Node
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void process(const ProcessContext& context) override;

private:
    std::shared_ptr<MidiClip> midiClip;
    int lastProcessedTransportPos = -1;
    double sampleRate_{44100.0};
    float currentPeak{0.0f};

    juce::Array<int> activeNotes;
    double lastTransportPos{-1.0};
    
    void turnOffActiveNotes(juce::MidiBuffer& midiMessages, int sampleOffset);

    // Framework for bouncing:
    // If a synth plugin is attached to this node or track, the MIDI buffer
    // is passed to the synth. To bounce to audio seamlessly, we can render 
    // the synth output offline and replace this node with an AudioClipNode.
};

} // namespace Nimbus
