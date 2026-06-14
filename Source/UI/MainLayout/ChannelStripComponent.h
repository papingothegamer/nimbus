#pragma once

#include <JuceHeader.h>

namespace Nimbus::MainLayout {

class ChannelStripComponent : public juce::Component, private juce::Timer {
public:
    ChannelStripComponent(const juce::String& name, bool isStereo = false, bool isMaster = false);
    ~ChannelStripComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void setLevelProvider(std::function<float()> provider);

    // Callbacks for interactivity
    std::function<void()> onSelected;

private:
    juce::String channelName;
    bool stereo;
    bool master;
    bool selected = false;

    juce::Label nameLabel;
    juce::Slider fader{juce::Slider::LinearVertical, juce::Slider::TextBoxBelow};
    juce::Slider pan{juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox};
    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};
    juce::TextButton stereoButton{"OO"}; // Mock stereo link icon

    // Helper for rendering meter
    void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level);

    std::function<float()> levelProvider;
    float currentLevel = 0.0f;

    void mouseDown(const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripComponent)
};

} // namespace Nimbus::MainLayout
