#pragma once

#include <JuceHeader.h>

namespace Nimbus::DetailView {

class AudioClipViewComponent : public juce::Component {
public:
    AudioClipViewComponent() = default;
    ~AudioClipViewComponent() override = default;

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.drawText("Audio Clip Settings & Waveform (WIP)", getLocalBounds(), juce::Justification::centred, true);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioClipViewComponent)
};

} // namespace Nimbus::DetailView
