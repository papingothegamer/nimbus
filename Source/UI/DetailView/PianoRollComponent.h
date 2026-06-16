#pragma once

#include <JuceHeader.h>

namespace Nimbus::DetailView {

class PianoRollComponent : public juce::Component {
public:
    PianoRollComponent() = default;
    ~PianoRollComponent() override = default;

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.drawText("Piano Roll / MIDI Editor (WIP)", getLocalBounds(), juce::Justification::centred, true);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};

} // namespace Nimbus::DetailView
