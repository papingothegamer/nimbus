#pragma once
#include <JuceHeader.h>
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus {

class PluginDial : public juce::Component {
public:
    PluginDial(const juce::String& name, double min, double max, double start, std::function<void(float)> cb, const juce::String& suffix = "") 
        : paramName(name), callback(cb) 
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
        slider.setTextValueSuffix(suffix);
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
    
    void paint(juce::Graphics& g) override {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f));
        g.drawText(paramName, getLocalBounds().removeFromTop(16), juce::Justification::centred, false);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(16); // label space
        slider.setBounds(bounds);
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
