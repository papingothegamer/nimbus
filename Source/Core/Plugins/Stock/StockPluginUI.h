#pragma once
#include <JuceHeader.h>
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus {

class AbletonRotaryLAF : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override {
        float radius = juce::jmin(width / 2.0f, height / 2.0f) - 2.0f;
        float centreX = x + width * 0.5f;
        float centreY = y + height * 0.5f;
        float rx = centreX - radius;
        float ry = centreY - radius;
        float rw = radius * 2.0f;
        float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colour(0xff121212));
        g.fillEllipse(rx, ry, rw, rw);
        g.setColour(juce::Colour(0xff888888));
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        float centerAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * 0.5f;
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius - 1.5f, radius - 1.5f, 0.0f, centerAngle, angle, true);
        g.setColour(juce::Colour(0xfffdb913)); // Bright orange/yellow
        g.strokePath(arc, juce::PathStrokeType(2.0f));

        juce::Path p;
        p.addRectangle(-1.0f, -radius, 2.0f, radius * 0.8f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }
};

class NimbusRotaryDial : public juce::Component {
public:
    NimbusRotaryDial(const juce::String& name, float min, float max, float init, const juce::String& suffixStr, std::function<void(float)> cb)
    : callback(cb), suffix(suffixStr) {
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
        label.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
        addAndMakeVisible(label);
        
        slider.setLookAndFeel(&laf);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(min, max, 1.0);
        slider.setValue(init, juce::dontSendNotification);
        slider.setDoubleClickReturnValue(true, init);
        slider.onValueChange = [this] {
            if (callback) callback(slider.getValue());
            valueField.setText(juce::String(slider.getValue(), 0) + suffix, juce::dontSendNotification);
        };
        addAndMakeVisible(slider);
        
        valueField.setText(juce::String(init, 0) + suffix, juce::dontSendNotification);
        valueField.setJustificationType(juce::Justification::centred);
        valueField.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
        valueField.setEditable(true);
        // No bounding box
        valueField.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        valueField.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        valueField.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
        valueField.onTextChange = [this] {
            float v = valueField.getText().retainCharacters("-0123456789.").getFloatValue();
            slider.setValue(v);
        };
        addAndMakeVisible(valueField);
    }
    
    ~NimbusRotaryDial() override { slider.setLookAndFeel(nullptr); }
    
    void resized() override {
        auto bounds = getLocalBounds();
        // Dynamic scaling: Label takes top 20, Field takes bottom 20. Dial takes rest.
        label.setBounds(bounds.removeFromTop(18));
        valueField.setBounds(bounds.removeFromBottom(18));
        
        // Ensure dial is a square
        int dialSize = juce::jmin(bounds.getWidth(), bounds.getHeight());
        slider.setBounds(bounds.getCentreX() - dialSize / 2, bounds.getCentreY() - dialSize / 2, dialSize, dialSize);
    }
    
    void setValue(float v) {
        slider.setValue(v, juce::dontSendNotification);
        valueField.setText(juce::String(v, 2) + suffix, juce::dontSendNotification);
    }
    
    void setDefaultValue(double val) {
        slider.setDoubleClickReturnValue(true, val);
    }
    
    juce::Slider& getSlider() { return slider; }

private:
    AbletonRotaryLAF laf;
    juce::Label label;
    juce::Slider slider;
    juce::Label valueField;
    std::function<void(float)> callback;
    juce::String suffix;
};

class PluginDial : public juce::Component {
public:
    PluginDial(const juce::String& name, double min, double max, double start, std::function<void(float)> cb, const juce::String& suffix = "") 
        : paramName(name), callback(cb) 
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
        slider.setTextValueSuffix(suffix);
        slider.setNumDecimalPlacesToDisplay(2);
        slider.setRange(min, max);
        slider.setValue(start, juce::dontSendNotification);
        slider.setDoubleClickReturnValue(true, start);
        slider.onValueChange = [this]() {
            if (callback) callback((float)slider.getValue());
        };
        addAndMakeVisible(slider);
    }
    
    juce::Slider& getSlider() { return slider; }
    
    void setValue(float val) {
        if (std::abs(slider.getValue() - val) > 0.001) {
            slider.setValue(val, juce::dontSendNotification);
        }
    }
    
    void setDefaultValue(double val) {
        slider.setDoubleClickReturnValue(true, val);
    }
    
    void paint(juce::Graphics& g) override {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f));
        g.drawText(paramName, getLocalBounds().removeFromTop(16), juce::Justification::centred, false);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(16); // label space
        int size = juce::jmin(bounds.getWidth(), bounds.getHeight());
        if (size > 80) size = 80;
        slider.setBounds(bounds.getCentreX() - size/2, bounds.getY() + (bounds.getHeight() - size)/2, size, size);
    }
    
private:
    juce::String paramName;
    juce::Slider slider;
    std::function<void(float)> callback;
};

class PluginHeader : public juce::Component {
public:
    PluginHeader(const juce::String& n) : name(n) {}
    void paint(juce::Graphics& g) override {
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f).boldened());
        g.drawText(name, getLocalBounds(), juce::Justification::centredLeft, false);
    }
private:
    juce::String name;
};

} // namespace Nimbus
