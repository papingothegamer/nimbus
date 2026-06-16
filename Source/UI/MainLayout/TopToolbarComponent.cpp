#include "TopToolbarComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "BinaryData.h"

namespace Nimbus::MainLayout {

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    auto setupButton = [](juce::DrawableButton& btn, const char* svgData, int svgSize) {
        if (svgData != nullptr) {
            juce::String svgStr(svgData, svgSize);
            svgStr = svgStr.replace("currentColor", "#000000");
            std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(svgStr);
            if (xml != nullptr) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
                    btn.setImages(svg.get());
                }
            }
        }
        btn.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    };

    auto setupTextButton = [](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    };

    setupTextButton(autoArmButton);
    setupTextButton(reenableAutoButton);
    setupTextButton(sessionRecordButton);
    setupTextButton(captureMidiButton);
    setupTextButton(punchInButton);
    setupTextButton(punchOutButton);
    setupTextButton(tapTempoButton);
    setupTextButton(nudgeDownButton);
    setupTextButton(nudgeUpButton);
    setupTextButton(followPlayheadToggle);
    followPlayheadToggle.setClickingTogglesState(true);

    setupButton(playButton, BinaryData::play_svg, BinaryData::play_svgSize);
    playButton.setClickingTogglesState(true);
    playButton.setColour(juce::DrawableButton::backgroundOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.3f));
    
    setupButton(stopButton, BinaryData::box_svg, BinaryData::box_svgSize);
    setupButton(arrRecordButton, BinaryData::circle_svg, BinaryData::circle_svgSize);
    setupButton(loopButton, BinaryData::repeat_svg, BinaryData::repeat_svgSize);

    addAndMakeVisible(linkToggle);
    addAndMakeVisible(tapTempoButton);
    addAndMakeVisible(tempoLabel);
    tempoLabel.setText("120.00", juce::dontSendNotification);
    addAndMakeVisible(nudgeDownButton);
    addAndMakeVisible(nudgeUpButton);
    addAndMakeVisible(timeSigNumLabel);
    timeSigNumLabel.setText("4", juce::dontSendNotification);
    addAndMakeVisible(timeSigDenLabel);
    timeSigDenLabel.setText("4", juce::dontSendNotification);
    addAndMakeVisible(metronomeToggle);
    addAndMakeVisible(quantizeBox);
    quantizeBox.addItem("1 Bar", 1);
    quantizeBox.setSelectedId(1);

    addAndMakeVisible(punchOutButton);
    
    addAndMakeVisible(followPlayheadToggle);

    addAndMakeVisible(linkToggle);
    addAndMakeVisible(tapTempoButton);
    addAndMakeVisible(autoArmButton);
    addAndMakeVisible(reenableAutoButton);
    addAndMakeVisible(sessionRecordButton);
    addAndMakeVisible(captureMidiButton);

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(arrRecordButton);
    addAndMakeVisible(autoArmButton);
    addAndMakeVisible(reenableAutoButton);
    addAndMakeVisible(sessionRecordButton);
    addAndMakeVisible(captureMidiButton);

    addAndMakeVisible(loopButton);
    addAndMakeVisible(punchInButton);
    addAndMakeVisible(punchOutButton);
    addAndMakeVisible(loopStartLabel);
    loopStartLabel.setText("1. 1. 1", juce::dontSendNotification);
    addAndMakeVisible(loopLengthLabel);
    loopLengthLabel.setText("4. 0. 0", juce::dontSendNotification);

    addAndMakeVisible(drawModeToggle);
    addAndMakeVisible(compMidiToggle);
    addAndMakeVisible(keyMapToggle);
    addAndMakeVisible(midiMapToggle);
    
    auto setupToggle = [](juce::ToggleButton& t) {
        t.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
        t.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
    };
    setupToggle(linkToggle);
    setupToggle(metronomeToggle);
    setupToggle(drawModeToggle);
    setupToggle(compMidiToggle);
    setupToggle(keyMapToggle);
    setupToggle(midiMapToggle);
    addAndMakeVisible(cpuLabel);
    cpuLabel.setText("0%", juce::dontSendNotification);

    setupButton(browserToggleButton, BinaryData::panelleft_svg, BinaryData::panelleft_svgSize);
    setupButton(detailToggleButton, BinaryData::panelbottom_svg, BinaryData::panelbottom_svgSize);
    addAndMakeVisible(browserToggleButton);
    addAndMakeVisible(detailToggleButton);

    browserToggleButton.onClick = [this] { if (onBrowserToggle) onBrowserToggle(); };
    detailToggleButton.onClick = [this] { if (onDetailToggle) onDetailToggle(); };

    playButton.onClick = [this] { 
        engine.getTransport().play(); 
        playButton.setToggleState(engine.getTransport().isPlaying(), juce::dontSendNotification);
    };
    stopButton.onClick = [this] { 
        engine.getTransport().stop(); 
        playButton.setToggleState(false, juce::dontSendNotification);
    };
    
    followPlayheadToggle.onClick = [this] {
        engine.setFollowPlayheadEnabled(followPlayheadToggle.getToggleState());
    };
    
    startTimerHz(20);
}

TopToolbarComponent::~TopToolbarComponent() = default;

void TopToolbarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Bottom border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TopToolbarComponent::timerCallback() {
    playButton.setToggleState(engine.getTransport().isPlaying(), juce::dontSendNotification);
    followPlayheadToggle.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
}

void TopToolbarComponent::resized() {
    auto bounds = getLocalBounds().reduced(5);
    
    int blockSpacing = 20;
    int btnW = 20;
    int h = 24;

    // Left Block
    linkToggle.setBounds(bounds.removeFromLeft(40).withHeight(h));
    bounds.removeFromLeft(5);
    tapTempoButton.setBounds(bounds.removeFromLeft(30).withHeight(h));
    bounds.removeFromLeft(5);
    tempoLabel.setBounds(bounds.removeFromLeft(50).withHeight(h));
    bounds.removeFromLeft(5);
    nudgeDownButton.setBounds(bounds.removeFromLeft(15).withHeight(h));
    nudgeUpButton.setBounds(bounds.removeFromLeft(15).withHeight(h));
    bounds.removeFromLeft(5);
    timeSigNumLabel.setBounds(bounds.removeFromLeft(20).withHeight(h));
    timeSigDenLabel.setBounds(bounds.removeFromLeft(20).withHeight(h));
    bounds.removeFromLeft(5);
    metronomeToggle.setBounds(bounds.removeFromLeft(30).withHeight(h));
    bounds.removeFromLeft(5);
    quantizeBox.setBounds(bounds.removeFromLeft(60).withHeight(h));

    bounds.removeFromLeft(blockSpacing);

    // Center Block (Transport)
    playButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h).reduced(2));
    stopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h).reduced(2));
    arrRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h).reduced(2));
    bounds.removeFromLeft(5);
    autoArmButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    reenableAutoButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(5);
    sessionRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    captureMidiButton.setBounds(bounds.removeFromLeft(30).withHeight(h));

    bounds.removeFromLeft(blockSpacing);

    // Right Block 1 (Loop)
    loopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h).reduced(2));
    punchInButton.setBounds(bounds.removeFromLeft(15).withHeight(h));    
    loopStartLabel.setBounds(bounds.removeFromLeft(30).reduced(2));
    loopLengthLabel.setBounds(bounds.removeFromLeft(30).reduced(2));
    
    followPlayheadToggle.setBounds(bounds.removeFromLeft(40).reduced(2));

    // Far Right Block (Misc)
    auto rightBounds = bounds.removeFromRight(200);
    cpuLabel.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    midiMapToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    keyMapToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    compMidiToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    drawModeToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));

    // Place toggles at the far edges
    browserToggleButton.setBounds(getLocalBounds().removeFromLeft(20).withHeight(20).reduced(2));
    detailToggleButton.setBounds(getLocalBounds().removeFromRight(20).withHeight(20).reduced(2));
}

} // namespace Nimbus::MainLayout
