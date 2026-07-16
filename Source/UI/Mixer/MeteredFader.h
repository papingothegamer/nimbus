#pragma once

#include <JuceHeader.h>
#include "DataModel/Models.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::UI::Mixer {

class MeteredFader : public juce::Component {
public:
    MeteredFader();
    ~MeteredFader() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTrackInfo(TrackType type, bool isStereo);
    void setLevel(float peakLevel);
    
    juce::Slider slider;

private:
    class FaderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        FaderLookAndFeel();
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    };

    FaderLookAndFeel customLookAndFeel;
    
    TrackType trackType = TrackType::Audio;
    bool stereo = false;
    float currentLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeteredFader)
};

} // namespace Nimbus::UI::Mixer
