#include "TopToolbarComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include "UI/Settings/SettingsMenuComponent.h"

namespace Nimbus::MainLayout {

void TopToolbarComponent::DisplayBox::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(DesignSystem::Colors::ModuleBackground);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(DesignSystem::Colors::TextSecondary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(10.0f).boldened());
    g.drawText(header, 0, 2, getWidth(), 12, juce::Justification::centredTop, false);
}

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    auto setupIconBtn = [](juce::TextButton& btn) {
        btn.getProperties().set("transparentBackground", true);
        btn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::PrimaryAction);
        btn.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
        btn.setColour(juce::TextButton::textColourOnId, DesignSystem::Colors::TextPrimary);
        btn.setWantsKeyboardFocus(false);
    };

    // Setup left section
    setupIconBtn(undoButton);
    setupIconBtn(redoButton);
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    
    projectNameLabel.setJustificationType(juce::Justification::centredLeft);
    projectNameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f).boldened());
    projectNameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    projectNameLabel.setEditable(true);
    projectNameLabel.onTextChange = [this]() {
        engine.getTimelineProject().setProjectName(projectNameLabel.getText());
    };
    addAndMakeVisible(projectNameLabel);

    setupIconBtn(saveProjectButton);
    addAndMakeVisible(saveProjectButton);

    // Setup Zoom
    setupIconBtn(zoomOutButton);
    setupIconBtn(zoomInButton);
    addAndMakeVisible(zoomOutButton);
    addAndMakeVisible(zoomInButton);
    
    zoomLevelLabel.setText("100%", juce::dontSendNotification);
    zoomLevelLabel.setJustificationType(juce::Justification::centred);
    zoomLevelLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
    zoomLevelLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    zoomLevelLabel.setEditable(true);
    zoomLevelLabel.onTextChange = [this]() {
        juce::String text = zoomLevelLabel.getText().retainCharacters("0123456789");
        int zoom = text.getIntValue();
        if (zoom < 10) zoom = 10;
        if (zoom > 500) zoom = 500;
        currentZoom = zoom;
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
    };
    addAndMakeVisible(zoomLevelLabel);

    // Setup Transport
    setupIconBtn(jumpStartButton);
    setupIconBtn(rewindButton);
    setupIconBtn(playButton);
    setupIconBtn(recordButton);
    setupIconBtn(fastForwardButton);
    
    addAndMakeVisible(jumpStartButton);
    addAndMakeVisible(rewindButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(fastForwardButton);
    
    // Setup Displays
    addAndMakeVisible(barsDisplay);
    addAndMakeVisible(timeDisplay);
    addAndMakeVisible(bpmDisplay);
    addAndMakeVisible(sigDisplay);

    bpmDisplay.valueLabel.setEditable(true);
    bpmDisplay.valueLabel.onTextChange = [this]() {
        float newBpm = bpmDisplay.valueLabel.getText().getFloatValue();
        if (newBpm >= 20.0f && newBpm <= 300.0f) {
            engine.getTransport().setTempo(newBpm);
        }
    };

    sigDisplay.valueLabel.setEditable(true);
    sigDisplay.valueLabel.onTextChange = [this]() {
        juce::String text = sigDisplay.valueLabel.getText();
        if (text.containsChar('/')) {
            auto parts = juce::StringArray::fromTokens(text, "/", "");
            if (parts.size() >= 2) {
                int num = parts[0].getIntValue();
                int den = parts[1].getIntValue();
                if (num > 0 && den > 0) {
                    engine.getTimelineProject().setTimeSigNumerator(num);
                    engine.getTimelineProject().setTimeSigDenominator(den);
                }
            }
        }
    };



    setupIconBtn(loopButton);
    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this]() {
        engine.getTransport().setLooping(loopButton.getToggleState());
    };
    
    setupIconBtn(metronomeToggle);
    metronomeToggle.setClickingTogglesState(true);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(metronomeToggle);

    setupIconBtn(followButton);
    followButton.setClickingTogglesState(true);
    followButton.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    followButton.onClick = [this]() {
        engine.setFollowPlayheadEnabled(followButton.getToggleState());
    };
    addAndMakeVisible(followButton);

    setupIconBtn(pianoRollToggle);
    setupIconBtn(mixerToggle);
    setupIconBtn(settingsButton);
    
    pianoRollToggle.setClickingTogglesState(true);
    pianoRollToggle.onClick = [this]() {
        pianoRollToggle.setButtonText(pianoRollToggle.getToggleState() ? DesignSystem::Iconography::PianoOn : DesignSystem::Iconography::PianoOff);
        // We'll also fire a callback if it was doing something else (the user will hook this up later)
    };
    mixerToggle.setClickingTogglesState(true);
    // settingsButton doesn't toggle state, it probably opens a menu
    
    addAndMakeVisible(pianoRollToggle);
    addAndMakeVisible(mixerToggle);
    addAndMakeVisible(settingsButton);

    cpuLabel.setText("CPU [||||      ]", juce::dontSendNotification);
    cpuLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(10.0f));
    cpuLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    addAndMakeVisible(cpuLabel);

    // Actions
    zoomOutButton.onClick = [this]() {
        currentZoom = juce::jlimit(10, 500, currentZoom - 10);
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
        if (onZoomOut) onZoomOut();
    };
    zoomInButton.onClick = [this]() {
        currentZoom = juce::jlimit(10, 500, currentZoom + 10);
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
        if (onZoomIn) onZoomIn();
    };
    saveProjectButton.onClick = [this]() {
        juce::FileChooser chooser("Select directory to save project", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        chooser.launchAsync(flags, [this](const juce::FileChooser& fc) {
            juce::File result = fc.getResult();
            if (result.isDirectory()) {
                // Future implementation of save logic
            }
        });
    };
    settingsButton.onClick = [this]() {
        juce::DialogWindow::LaunchOptions options;
        options.content.setOwned(new UI::Settings::SettingsMenuComponent(engine));
        options.dialogTitle = "Preferences";
        options.dialogBackgroundColour = DesignSystem::Colors::AppBackground;
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = false;
        options.resizable = false;
        options.launchAsync();
    };
    mixerToggle.onClick = [this]() {
        if (onDetailToggle) onDetailToggle();
    };
    playButton.onClick = [this]() {
        if (engine.getTransport().isPlaying()) {
            engine.getTransport().stop();
        } else {
            engine.getTransport().play();
        }
    };
    recordButton.onClick = [this]() {
        if (engine.getTransport().isRecording()) {
            engine.getTransport().stopRecording();
        } else {
            engine.getTransport().record();
        }
    };
    jumpStartButton.onClick = [this]() {
        engine.getTransport().setPosition(0.0);
    };
    
    startTimerHz(30);
}

TopToolbarComponent::~TopToolbarComponent() {}

void TopToolbarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Bottom border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);

    // Group separators
    int centerX = getWidth() / 2;
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.5f));
    
    // Zoom / Transport separator
    g.fillRect(centerX - 260, 8, 1, getHeight() - 16);
    
    // Transport / Time Displays separator
    g.fillRect(centerX - 105, 8, 1, getHeight() - 16);
    
    // Time Displays / Tempo separator
    g.fillRect(centerX + 65, 8, 1, getHeight() - 16);
    
    // Tempo / Toggles separator
    g.fillRect(centerX + 235, 8, 1, getHeight() - 16);
}

void TopToolbarComponent::resized() {
    auto bounds = getLocalBounds().reduced(8, 4);
    int groupHeight = bounds.getHeight();
    
    // Left section
    undoButton.setBounds(10, 4, 24, groupHeight);
    redoButton.setBounds(38, 4, 24, groupHeight);
    projectNameLabel.setBounds(70, 0, 150, getHeight());
    saveProjectButton.setBounds(225, 8, 24, 24);
    
    int centerX = getWidth() / 2;
    
    // Center-Left: Zoom
    zoomOutButton.setBounds(centerX - 335, 4, 24, groupHeight);
    zoomLevelLabel.setBounds(centerX - 311, 4, 40, groupHeight);
    zoomInButton.setBounds(centerX - 271, 4, 24, groupHeight);
    
    // Center: Transport
    jumpStartButton.setBounds(centerX - 245, 4, 24, groupHeight);
    rewindButton.setBounds(centerX - 217, 4, 24, groupHeight);
    playButton.setBounds(centerX - 189, 4, 24, groupHeight);
    recordButton.setBounds(centerX - 161, 4, 24, groupHeight);
    fastForwardButton.setBounds(centerX - 133, 4, 24, groupHeight);

    // Center: Displays
    barsDisplay.setBounds(centerX - 95, 2, 75, 36);
    timeDisplay.setBounds(centerX - 15, 2, 70, 36);

    // Center-Right: Tempo and Sig
    bpmDisplay.setBounds(centerX + 65, 2, 60, 36);
    sigDisplay.setBounds(centerX + 135, 2, 45, 36);

    // Far-Right: Toggles
    loopButton.setBounds(centerX + 195, 4, 24, groupHeight);
    metronomeToggle.setBounds(centerX + 223, 4, 24, groupHeight);
    
    // Status right side
    int rightEdge = getWidth() - 10;
    cpuLabel.setBounds(rightEdge - 80, 10, 80, 20);
    settingsButton.setBounds(rightEdge - 110, 8, 24, 24);
    mixerToggle.setBounds(rightEdge - 138, 8, 24, 24);
    pianoRollToggle.setBounds(rightEdge - 166, 8, 24, 24);
    
    followButton.setBounds(centerX + 305, 4, 24, groupHeight);
}

void TopToolbarComponent::timerCallback() {
    bool isPlaying = engine.getTransport().isPlaying();
    playButton.setToggleState(isPlaying, juce::dontSendNotification);
    playButton.setButtonText(isPlaying ? DesignSystem::Iconography::Pause : DesignSystem::Iconography::Play);
    
    bool isRecording = engine.getTransport().isRecording();
    recordButton.setToggleState(isRecording, juce::dontSendNotification);
    
    double posSamples = engine.getTransport().getCurrentPosition();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    double posSeconds = posSamples / sampleRate;
    
    // Time formatting: MM:SS:MS
    int mins = static_cast<int>(posSeconds) / 60;
    int secs = static_cast<int>(posSeconds) % 60;
    int ms = static_cast<int>((posSeconds - std::floor(posSeconds)) * 100);
    juce::String timeStr = juce::String::formatted("%02d:%02d:%02d", mins, secs, ms);
    timeDisplay.setValue(timeStr);
    
    // Bars formatting: Bar.Beat.Tick (assuming current tempo & sig)
    double tempo = engine.getTransport().getTempo();
    if (tempo <= 0.0) tempo = 120.0;
    
    auto& project = engine.getTimelineProject();
    int num = project.getTimeSigNumerator();
    
    double totalBeats = posSeconds * (tempo / 60.0);
    int bar = static_cast<int>(totalBeats / num) + 1;
    int beat = static_cast<int>(std::fmod(totalBeats, static_cast<double>(num))) + 1;
    int ticks = static_cast<int>(std::fmod(totalBeats * 100.0, 100.0));
    juce::String barStr = juce::String::formatted("%d.%d.%02d", bar, beat, ticks);
    barsDisplay.setValue(barStr);
    
    bpmDisplay.setValue(juce::String(tempo, 1));
    if (!sigDisplay.valueLabel.isBeingEdited()) {
        sigDisplay.setValue(juce::String::formatted("%d/%d", num, project.getTimeSigDenominator()));
    }
    
    if (!projectNameLabel.isBeingEdited()) {
        projectNameLabel.setText(project.getProjectName(), juce::dontSendNotification);
    }
    
    followButton.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    
    bool isLooping = engine.getTransport().isLooping();
    loopButton.setToggleState(isLooping, juce::dontSendNotification);
    loopButton.setButtonText(DesignSystem::Iconography::Loop);
}

} // namespace Nimbus::MainLayout
