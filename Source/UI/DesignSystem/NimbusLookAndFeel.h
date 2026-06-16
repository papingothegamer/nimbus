#pragma once

#include <JuceHeader.h>
#include <map>
#include <memory>

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
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

private:
    juce::Drawable* getOrCacheSvg(const juce::String& resourceName);
    std::map<juce::String, std::unique_ptr<juce::Drawable>> svgCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NimbusLookAndFeel)
};

} // namespace Nimbus::DesignSystem
