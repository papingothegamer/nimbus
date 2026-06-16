#include "TopToolbarComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include "BinaryData.h"

namespace Nimbus::MainLayout {

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    auto setupButton = [](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
    };

    setupButton(playButton);
    setupButton(stopButton);
    setupButton(arrRecordButton);
    setupButton(loopButton);
    setupButton(metronomeToggle);
    setupButton(followPlayheadToggle);

    playButton.setButtonText(DesignSystem::Iconography::Play);
    stopButton.setButtonText(DesignSystem::Iconography::Stop);
    arrRecordButton.setButtonText(DesignSystem::Iconography::RecordGlobal);
    loopButton.setButtonText(DesignSystem::Iconography::Loop);
    metronomeToggle.setButtonText(DesignSystem::Iconography::Metronome);
    metronomeToggle.setClickingTogglesState(true);
    followPlayheadToggle.setButtonText(DesignSystem::Iconography::Follow);
    followPlayheadToggle.setClickingTogglesState(true);

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

    autoArmButton.setButtonText(DesignSystem::Iconography::Tune);
    reenableAutoButton.setButtonText(DesignSystem::Iconography::RightFold);
    sessionRecordButton.setButtonText(DesignSystem::Iconography::Save);
    captureMidiButton.setButtonText(DesignSystem::Iconography::PianoOff);
    punchInButton.setButtonText(DesignSystem::Iconography::Fold);
    punchOutButton.setButtonText(DesignSystem::Iconography::Unfold);

    browserToggleButton.setButtonText(DesignSystem::Iconography::Sidebar);
    detailToggleButton.setButtonText(DesignSystem::Iconography::DetailView);

    nudgeDownButton.setButtonText("<");
    nudgeUpButton.setButtonText(">");
    tapTempoButton.setButtonText("TAP");
    
    playButton.setClickingTogglesState(true);
    playButton.setColour(juce::DrawableButton::backgroundOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.3f));

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
    addAndMakeVisible(autoArmButton);
    addAndMakeVisible(reenableAutoButton);
    addAndMakeVisible(sessionRecordButton);
    addAndMakeVisible(captureMidiButton);

    setupButton(browserToggleButton);
    setupButton(detailToggleButton);
    browserToggleButton.setClickingTogglesState(true);
    detailToggleButton.setClickingTogglesState(true);

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(arrRecordButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(punchInButton);
    addAndMakeVisible(loopStartLabel);
    loopStartLabel.setText("1. 1. 1", juce::dontSendNotification);
    addAndMakeVisible(loopLengthLabel);
    loopLengthLabel.setText("4. 0. 0", juce::dontSendNotification);

    addAndMakeVisible(drawModeToggle);
    addAndMakeVisible(compMidiToggle);
    addAndMakeVisible(keyMapToggle);
    addAndMakeVisible(midiMapToggle);
    
    linkToggle.setButtonText(DesignSystem::Iconography::Stereo);
    drawModeToggle.setButtonText(DesignSystem::Iconography::Pencil);
    compMidiToggle.setButtonText(DesignSystem::Iconography::Flat);
    keyMapToggle.setButtonText(DesignSystem::Iconography::Piano);
    midiMapToggle.setButtonText(DesignSystem::Iconography::Midi);

    linkToggle.setClickingTogglesState(true);
    drawModeToggle.setClickingTogglesState(true);
    compMidiToggle.setClickingTogglesState(true);
    keyMapToggle.setClickingTogglesState(true);
    midiMapToggle.setClickingTogglesState(true);

    setupTextButton(linkToggle);
    setupTextButton(drawModeToggle);
    setupTextButton(compMidiToggle);
    setupTextButton(keyMapToggle);
    setupTextButton(midiMapToggle);
    
    addAndMakeVisible(cpuLabel);
    cpuLabel.setText("0%", juce::dontSendNotification);

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
    int btnW = 24;
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
    playButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    stopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    arrRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(5);
    autoArmButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    reenableAutoButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(5);
    sessionRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    captureMidiButton.setBounds(bounds.removeFromLeft(30).withHeight(h));

    bounds.removeFromLeft(blockSpacing);

    // Right Block 1 (Loop)
    loopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    punchInButton.setBounds(bounds.removeFromLeft(15).withHeight(h));    
    loopStartLabel.setBounds(bounds.removeFromLeft(30));
    loopLengthLabel.setBounds(bounds.removeFromLeft(30));
    
    followPlayheadToggle.setBounds(bounds.removeFromLeft(40));

    // Far Right Block (Misc)
    auto rightBounds = bounds.removeFromRight(200);
    cpuLabel.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    midiMapToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    keyMapToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    compMidiToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));
    drawModeToggle.setBounds(rightBounds.removeFromRight(40).withHeight(h));

    // Place toggles at the far edges
    browserToggleButton.setBounds(getLocalBounds().removeFromLeft(20).withHeight(20));
    detailToggleButton.setBounds(getLocalBounds().removeFromRight(20).withHeight(20));
}

} // namespace Nimbus::MainLayout
