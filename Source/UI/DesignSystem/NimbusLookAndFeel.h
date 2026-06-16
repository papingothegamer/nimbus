#pragma once

#include <JuceHeader.h>

namespace Nimbus::DesignSystem {

class NimbusLookAndFeel : public juce::LookAndFeel_V4 {
public:
    NimbusLookAndFeel();
    ~NimbusLookAndFeel() override;

    // Overrides to make UI flat and matte
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
                          
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                              
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NimbusLookAndFeel)
};

} // namespace Nimbus::DesignSystem
