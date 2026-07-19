#pragma once

#include <JuceHeader.h>

namespace Nimbus::UI {

// A sleek, flat toggle button resembling Ableton's toggle switches (e.g., Warp, Loop)
class AbletonToggleButton : public juce::Button {
public:
    AbletonToggleButton(const juce::String& name);
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

// A sleek number box that allows clicking and dragging to change values (like Ableton's transpose/gain boxes)
class AbletonNumberBox : public juce::Component {
public:
    AbletonNumberBox();
    ~AbletonNumberBox() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setRange(double min, double max, double step);
    void setValue(double newValue, juce::NotificationType notification = juce::sendNotificationAsync);
    double getValue() const { return value; }
    
    void setSuffix(const juce::String& suffixText) { suffix = suffixText; repaint(); }
    void setNumDecimalPlaces(int places) { decimalPlaces = places; repaint(); }
    
    std::function<void(double)> onValueChanged;
    std::function<juce::String(double)> textFormatter;

private:
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    double value = 0.0;
    double minValue = 0.0;
    double maxValue = 1.0;
    double stepSize = 0.1;
    double defaultValue = 0.0;
    
    float dragStartValue = 0.0f;
    juce::String suffix = "";
    int decimalPlaces = 2;
    
    juce::Label editLabel;
};

// A panel representing one of the property boxes (e.g., "Clip", "Audio", "Notes")
class AbletonPanel : public juce::Component {
public:
    AbletonPanel(const juce::String& title);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void addContent(juce::Component* comp);
    void clearContent();

    bool isFolded() const { return folded; }
    void setFolded(bool shouldBeFolded);

    std::function<void()> onFoldStateChanged;

private:
    void mouseDown(const juce::MouseEvent& e) override;

    juce::String panelTitle;
    bool folded = false;
    juce::Component contentContainer;
    juce::Array<juce::Component*> contents;
};

// A sleek vertical slider for gain, based on the meter fader but without the VU
class AbletonVerticalGainSlider : public juce::Slider {
public:
    AbletonVerticalGainSlider();
    ~AbletonVerticalGainSlider() override;

private:
    class GainSliderLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    } customLaf;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbletonVerticalGainSlider)
};

} // namespace Nimbus::UI
