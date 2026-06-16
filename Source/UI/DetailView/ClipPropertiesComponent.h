#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::DetailView {

class ClipPropertiesComponent : public juce::Component {
public:
    ClipPropertiesComponent(NimbusEngine& engine);
    ~ClipPropertiesComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMidiMode(bool isMidi);
    void updateClipInfo(const juce::String& name, double startSamples, double lengthSamples);

private:
    NimbusEngine& engine;
    bool isMidiMode = true;

    juce::Label titleLabel;
    juce::Label nameLabel;
    
    // Properties shared
    juce::Label startLabel;
    juce::Label lengthLabel;

    // Audio-specific
    juce::ToggleButton warpButton{"Warp"};
    juce::Slider transposeSlider;
    juce::Label transposeLabel;
    juce::Slider gainSlider;
    juce::Label gainLabel;

    // MIDI-specific
    juce::ToggleButton loopButton{"Loop"};
    juce::Label signatureLabel;
    juce::ComboBox grooveBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipPropertiesComponent)
};

} // namespace Nimbus::DetailView
