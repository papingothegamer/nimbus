#include "TopToolbarComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include "BinaryData.h"
#include "UI/Settings/SettingsMenuComponent.h"

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
    arrRecordButton.setClickingTogglesState(true);
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
    
    playButton.setWantsKeyboardFocus(false);
    stopButton.setWantsKeyboardFocus(false);
    arrRecordButton.setWantsKeyboardFocus(false);
    loopButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(loopStartLabel);
    loopStartLabel.setText("1. 1. 1", juce::dontSendNotification);
    addAndMakeVisible(loopLengthLabel);
    loopLengthLabel.setText("4. 0. 0", juce::dontSendNotification);

    addAndMakeVisible(drawModeToggle);
    addAndMakeVisible(compMidiToggle);
    addAndMakeVisible(keyMapToggle);
    addAndMakeVisible(midiMapToggle);
    
    drawModeToggle.setButtonText(DesignSystem::Iconography::Pencil);
    compMidiToggle.setButtonText(DesignSystem::Iconography::Flat);
    keyMapToggle.setButtonText(DesignSystem::Iconography::Piano);
    midiMapToggle.setButtonText(DesignSystem::Iconography::Midi);
    
    settingsButton.setButtonText(DesignSystem::Iconography::Settings);
    addAndMakeVisible(settingsButton);

    drawModeToggle.setClickingTogglesState(true);
    compMidiToggle.setClickingTogglesState(true);
    keyMapToggle.setClickingTogglesState(true);
    midiMapToggle.setClickingTogglesState(true);

    setupTextButton(drawModeToggle);
    setupTextButton(compMidiToggle);
    setupTextButton(keyMapToggle);
    setupTextButton(midiMapToggle);
    setupTextButton(settingsButton);
    
    addAndMakeVisible(cpuLabel);
    cpuLabel.setText("0%", juce::dontSendNotification);

    addAndMakeVisible(browserToggleButton);
    addAndMakeVisible(detailToggleButton);

    browserToggleButton.onClick = [this] { if (onBrowserToggle) onBrowserToggle(); };
    detailToggleButton.onClick = [this] { if (onDetailToggle) onDetailToggle(); };

    playButton.onClick = [this] { 
        if (engine.getTransport().isPlaying()) {
            engine.getTransport().stop();
        } else {
            engine.getTransport().play();
        }
    };
    stopButton.onClick = [this] { 
        engine.getTransport().stop(); 
    };
    
    followPlayheadToggle.onClick = [this] {
        engine.setFollowPlayheadEnabled(followPlayheadToggle.getToggleState());
    };
    
    arrRecordButton.onClick = [this] {
        if (engine.getTransport().isRecording()) {
            engine.stopRecording(); // Stop playback and recording
        } else {
            engine.startRecording();
        }
    };

    settingsButton.onClick = [this] {
        auto menu = std::make_unique<UI::Settings::SettingsMenuComponent>(engine);
        juce::CallOutBox::launchAsynchronously(std::move(menu), settingsButton.getScreenBounds(), nullptr);
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
    bool isPlaying = engine.getTransport().isPlaying();
    playButton.setToggleState(isPlaying, juce::dontSendNotification);
    playButton.setButtonText(isPlaying ? DesignSystem::Iconography::Pause : DesignSystem::Iconography::Play);
    followPlayheadToggle.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    
    bool isRecording = engine.getTransport().isRecording();
    arrRecordButton.setToggleState(isRecording, juce::dontSendNotification);
}

void TopToolbarComponent::resized() {
    auto bounds = getLocalBounds().reduced(4, 2);
    int h = bounds.getHeight();
    int btnW = 28;
    int smallBtnW = 20;
    int gap = 4;

    // Far-left: browser toggle
    browserToggleButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(gap);

    // Left Block: Tap, Tempo, Nudge, TimeSig, Metronome, Quantize
    tapTempoButton.setBounds(bounds.removeFromLeft(32).withHeight(h));
    bounds.removeFromLeft(gap);
    tempoLabel.setBounds(bounds.removeFromLeft(50).withHeight(h));
    bounds.removeFromLeft(2);
    nudgeDownButton.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    nudgeUpButton.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    bounds.removeFromLeft(gap);
    timeSigNumLabel.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    timeSigDenLabel.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    bounds.removeFromLeft(gap);
    metronomeToggle.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(gap);
    quantizeBox.setBounds(bounds.removeFromLeft(60).withHeight(h));

    bounds.removeFromLeft(gap * 3);

    // Center Block: Transport
    playButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(2);
    stopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(2);
    arrRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(gap);
    autoArmButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(2);
    reenableAutoButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(gap);
    sessionRecordButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(2);
    captureMidiButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));

    bounds.removeFromLeft(gap * 3);

    // Loop Block
    loopButton.setBounds(bounds.removeFromLeft(btnW).withHeight(h));
    bounds.removeFromLeft(2);
    punchInButton.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    loopStartLabel.setBounds(bounds.removeFromLeft(36).withHeight(h));
    loopLengthLabel.setBounds(bounds.removeFromLeft(36).withHeight(h));
    punchOutButton.setBounds(bounds.removeFromLeft(smallBtnW).withHeight(h));
    bounds.removeFromLeft(gap);
    followPlayheadToggle.setBounds(bounds.removeFromLeft(btnW).withHeight(h));

    // Far-right: detail toggle
    detailToggleButton.setBounds(bounds.removeFromRight(btnW).withHeight(h));
    bounds.removeFromRight(gap);

    // Right block: CPU, Settings, MIDI map, Key map, Comp MIDI, Draw mode
    cpuLabel.setBounds(bounds.removeFromRight(40).withHeight(h));
    bounds.removeFromRight(gap);
    settingsButton.setBounds(bounds.removeFromRight(btnW).withHeight(h));
    bounds.removeFromRight(gap);
    midiMapToggle.setBounds(bounds.removeFromRight(btnW).withHeight(h));
    bounds.removeFromRight(2);
    keyMapToggle.setBounds(bounds.removeFromRight(btnW).withHeight(h));
    bounds.removeFromRight(2);
    compMidiToggle.setBounds(bounds.removeFromRight(btnW).withHeight(h));
    bounds.removeFromRight(2);
    drawModeToggle.setBounds(bounds.removeFromRight(btnW).withHeight(h));
}

} // namespace Nimbus::MainLayout
