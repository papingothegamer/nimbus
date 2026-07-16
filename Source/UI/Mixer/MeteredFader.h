#pragma once
#include <JuceHeader.h>

namespace Nimbus::UI {

class MeteredFader : public juce::Component {
public:
    enum class TrackType { MonoAudio, StereoAudio };

    MeteredFader();
    ~MeteredFader() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return volumeSlider; }
    
    void setLevel(float levelLeft, float levelRight = 0.0f);
    void setTrackType(TrackType newType);

private:
    juce::Slider volumeSlider;
    TrackType type = TrackType::MonoAudio;
    float currentLevelL = 0.0f;
    float currentLevelR = 0.0f;

    class FaderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    } customLaf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeteredFader)
};

} // namespace Nimbus::UI