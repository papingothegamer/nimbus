#include "ChannelStripComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

ChannelStripComponent::ChannelStripComponent(const juce::String& name, bool isStereo, bool isMaster)
    : channelName(name), stereo(isStereo), master(isMaster)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(fader);
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    fader.setRange(0.0, 1.0, 0.01);
    fader.setValue(0.75);
    fader.setColour(juce::Slider::thumbColourId, DesignSystem::Colors::PrimaryAction);
    fader.setColour(juce::Slider::trackColourId, DesignSystem::Colors::ModuleBackground);

    addAndMakeVisible(pan);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setValue(0.0);
    pan.setColour(juce::Slider::rotarySliderFillColourId, DesignSystem::Colors::PrimaryAction);

    if (!master) {
        addAndMakeVisible(muteButton);
        muteButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Mute);
        muteButton.setClickingTogglesState(true);

        addAndMakeVisible(soloButton);
        soloButton.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::Solo);
        soloButton.setClickingTogglesState(true);

        addAndMakeVisible(stereoButton);
        stereoButton.setClickingTogglesState(true);
        stereoButton.setToggleState(stereo, juce::dontSendNotification);
        stereoButton.onClick = [this]() {
            stereo = stereoButton.getToggleState();
            repaint();
        };
    }

    startTimerHz(30); // 30fps meter updates
}

ChannelStripComponent::~ChannelStripComponent() {
    stopTimer();
}

void ChannelStripComponent::setLevelProvider(std::function<float()> provider) {
    levelProvider = std::move(provider);
}

void ChannelStripComponent::timerCallback() {
    if (levelProvider) {
        float newLevel = levelProvider();
        if (std::abs(newLevel - currentLevel) > 0.01f) {
            currentLevel = newLevel;
            repaint();
        }
    }
}

void ChannelStripComponent::paint(juce::Graphics& g) {
    if (selected) {
        g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    }

    // Border
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(getLocalBounds(), 1);

    // Draw meters
    auto bounds = getLocalBounds();
    auto meterArea = bounds.removeFromRight(15).reduced(2, 40).withTrimmedBottom(20);
    
    if (stereo) {
        auto leftMeter = meterArea.removeFromLeft(meterArea.getWidth() / 2).reduced(1, 0);
        auto rightMeter = meterArea.reduced(1, 0);
        drawMeter(g, leftMeter, currentLevel); 
        drawMeter(g, rightMeter, currentLevel);
    } else {
        drawMeter(g, meterArea.reduced(2, 0), currentLevel);
    }
}

void ChannelStripComponent::drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level) {
    g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.2f));
    g.fillRect(bounds);

    juce::Colour gradientTop = juce::Colours::red;
    juce::Colour gradientMid = juce::Colours::yellow;
    juce::Colour gradientBottom = juce::Colours::lime;

    juce::ColourGradient cg(gradientBottom, bounds.getBottomLeft().toFloat(),
                            gradientTop, bounds.getTopLeft().toFloat(), false);
    cg.addColour(0.7f, gradientMid);

    int height = juce::roundToInt(bounds.getHeight() * level);
    auto fillBounds = bounds.withTrimmedTop(bounds.getHeight() - height);
    
    g.setGradientFill(cg);
    g.fillRect(fillBounds);
}

void ChannelStripComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    
    nameLabel.setBounds(bounds.removeFromTop(20));
    
    if (!master) {
        auto btnArea = bounds.removeFromTop(20);
        stereoButton.setBounds(btnArea.removeFromRight(20).reduced(2));
        muteButton.setBounds(btnArea.removeFromLeft(bounds.getWidth() / 2).reduced(2));
        soloButton.setBounds(btnArea.reduced(2));
    } else {
        bounds.removeFromTop(20); // spacer
    }

    pan.setBounds(bounds.removeFromTop(50).reduced(4));
    
    // Fader gets the rest, but keep right edge clear for meters
    fader.setBounds(bounds.withTrimmedRight(15).reduced(4));
}

void ChannelStripComponent::mouseDown(const juce::MouseEvent&) {
    selected = true;
    repaint();
    if (onSelected) {
        onSelected();
    }
}

} // namespace Nimbus::MainLayout
