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

void TopToolbarComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        juce::String svgStr(data, (size_t)size);
        svgStr = svgStr.replace("fill=\"#000000\"", "fill=\"#ffffff\"")
                       .replace("fill=\"#212121\"", "fill=\"#ffffff\"")
                       .replace("fill=\"currentColor\"", "fill=\"#ffffff\"")
                       .replace("<svg ", "<svg fill=\"#ffffff\" color=\"#ffffff\" ");

        if (auto xml = juce::XmlDocument::parse(svgStr)) {
            if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
            }
        }
    }
}

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    auto setupIconBtn = [](juce::DrawableButton& btn) {
        btn.getProperties().set("transparentBackground", true);
        btn.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::DrawableButton::backgroundOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        btn.setWantsKeyboardFocus(false);
    };

    addAndMakeVisible(transportGroupContainer);
    addAndMakeVisible(toolsGroupContainer);
    addAndMakeVisible(actionGroupContainer);

    // --- Action Group: Undo, Redo, Save, Cut, Trim ---
    setupIconBtn(undoButton);
    setupIconBtn(redoButton);
    setupIconBtn(saveProjectButton);
    setupIconBtn(cutButton);
    setupIconBtn(trimButton);
    loadSvgIcon(undoButton, DesignSystem::Iconography::Undo);
    loadSvgIcon(redoButton, DesignSystem::Iconography::Redo);
    loadSvgIcon(saveProjectButton, DesignSystem::Iconography::Save);
    loadSvgIcon(cutButton, DesignSystem::Iconography::Cut);
    loadSvgIcon(trimButton, DesignSystem::Iconography::Trim);
    actionGroupContainer.addAndMakeVisible(undoButton);
    actionGroupContainer.addAndMakeVisible(redoButton);
    actionGroupContainer.addAndMakeVisible(saveProjectButton);
    actionGroupContainer.addAndMakeVisible(cutButton);
    actionGroupContainer.addAndMakeVisible(trimButton);
    
    projectNameLabel.setJustificationType(juce::Justification::centredLeft);
    projectNameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f).boldened());
    projectNameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    projectNameLabel.setEditable(true);
    projectNameLabel.onTextChange = [this]() {
        engine.getTimelineProject().setProjectName(projectNameLabel.getText());
    };
    addAndMakeVisible(projectNameLabel);

    // --- Zoom controls ---
    setupIconBtn(zoomOutButton);
    setupIconBtn(zoomInButton);
    loadSvgIcon(zoomOutButton, DesignSystem::Iconography::ZoomOut);
    loadSvgIcon(zoomInButton, DesignSystem::Iconography::ZoomIn);
    toolsGroupContainer.addAndMakeVisible(zoomOutButton);
    toolsGroupContainer.addAndMakeVisible(zoomInButton);
    
    zoomLevelLabel.setText("100%", juce::dontSendNotification);
    zoomLevelLabel.setJustificationType(juce::Justification::centred);
    zoomLevelLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
    zoomLevelLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    zoomLevelLabel.setEditable(true);
    zoomLevelLabel.onTextChange = [this]() {
        juce::String text = zoomLevelLabel.getText().retainCharacters("0123456789");
        int zoom = text.getIntValue();
        if (zoom < 10) zoom = 10;
        if (zoom > 150) zoom = 150;
        currentZoom = zoom;
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
    };
    toolsGroupContainer.addAndMakeVisible(zoomLevelLabel);

    // --- Transport controls ---
    setupIconBtn(pauseButton);
    setupIconBtn(playButton);
    setupIconBtn(stopButton);
    setupIconBtn(jumpStartButton);
    setupIconBtn(rewindButton);
    setupIconBtn(jumpEndButton);
    setupIconBtn(recordButton);
    setupIconBtn(loopButton);
    setupIconBtn(metronomeToggle);

    loadSvgIcon(pauseButton, DesignSystem::Iconography::Pause);
    loadSvgIcon(playButton, DesignSystem::Iconography::Play);
    loadSvgIcon(stopButton, DesignSystem::Iconography::Stop);
    loadSvgIcon(jumpStartButton, DesignSystem::Iconography::JumpStart);
    loadSvgIcon(rewindButton, DesignSystem::Iconography::Rewind);
    loadSvgIcon(jumpEndButton, DesignSystem::Iconography::FastForward);
    loadSvgIcon(recordButton, DesignSystem::Iconography::RecordGlobal);
    loadSvgIcon(loopButton, DesignSystem::Iconography::Loop);
    loadSvgIcon(metronomeToggle, DesignSystem::Iconography::Metronome);

    transportGroupContainer.addAndMakeVisible(pauseButton);
    transportGroupContainer.addAndMakeVisible(playButton);
    transportGroupContainer.addAndMakeVisible(stopButton);
    transportGroupContainer.addAndMakeVisible(jumpStartButton);
    transportGroupContainer.addAndMakeVisible(rewindButton);
    transportGroupContainer.addAndMakeVisible(jumpEndButton);
    transportGroupContainer.addAndMakeVisible(recordButton);
    transportGroupContainer.addAndMakeVisible(loopButton);
    transportGroupContainer.addAndMakeVisible(metronomeToggle);
    
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

    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this]() {
        bool looping = loopButton.getToggleState();
        engine.getTransport().setLooping(looping);
        loadSvgIcon(loopButton, looping ? DesignSystem::Iconography::Loop : DesignSystem::Iconography::LoopOff);
    };
    
    metronomeToggle.setClickingTogglesState(true);

    // --- Follow playhead toggle ---
    setupIconBtn(followButton);
    loadSvgIcon(followButton, DesignSystem::Iconography::Follow);
    followButton.setClickingTogglesState(true);
    followButton.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    followButton.onClick = [this]() {
        engine.setFollowPlayheadEnabled(followButton.getToggleState());
    };
    addAndMakeVisible(followButton);

    // --- Panel toggles ---
    setupIconBtn(pianoRollToggle);
    setupIconBtn(mixerToggle);
    setupIconBtn(settingsButton);
    
    loadSvgIcon(pianoRollToggle, DesignSystem::Iconography::PianoOff);
    loadSvgIcon(mixerToggle, DesignSystem::Iconography::Tune);
    loadSvgIcon(settingsButton, DesignSystem::Iconography::Settings);

    pianoRollToggle.setClickingTogglesState(true);
    pianoRollToggle.onClick = [this]() {
        loadSvgIcon(pianoRollToggle, pianoRollToggle.getToggleState() ? DesignSystem::Iconography::PianoOn : DesignSystem::Iconography::PianoOff);
        if (onDetailToggle) onDetailToggle();
    };
    mixerToggle.setClickingTogglesState(true);
    
    addAndMakeVisible(pianoRollToggle);
    addAndMakeVisible(mixerToggle);
    addAndMakeVisible(settingsButton);

    // --- CPU label ---
    cpuLabel.setText("CPU: 0%", juce::dontSendNotification);
    cpuLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(10.0f));
    cpuLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    addAndMakeVisible(cpuLabel);

    // --- Button callbacks ---
    zoomOutButton.onClick = [this]() {
        currentZoom = juce::jlimit(10, 150, currentZoom - 10);
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
        if (onZoomOut) onZoomOut();
    };
    zoomInButton.onClick = [this]() {
        currentZoom = juce::jlimit(10, 150, currentZoom + 10);
        zoomLevelLabel.setText(juce::String(currentZoom) + "%", juce::dontSendNotification);
        if (onZoomIn) onZoomIn();
    };
    saveProjectButton.onClick = [this]() {
        juce::FileChooser chooser("Select directory to save project", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        chooser.launchAsync(flags, [this](const juce::FileChooser& fc) {
            juce::File result = fc.getResult();
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
        if (onBottomPanelToggle) onBottomPanelToggle();
    };
    pauseButton.onClick = [this]() {
        if (engine.getTransport().isPlaying()) engine.getTransport().stop();
    };
    playButton.onClick = [this]() {
        if (!engine.getTransport().isPlaying()) engine.getTransport().play();
    };
    stopButton.onClick = [this]() {
        engine.getTransport().stop();
        engine.getTransport().setPosition(0.0);
    };
    recordButton.onClick = [this]() {
        if (engine.getTransport().isRecording()) engine.stopRecording();
        else engine.startRecording();
    };
    jumpStartButton.onClick = [this]() {
        engine.getTransport().setPosition(0.0);
    };
    rewindButton.onClick = [this]() {
        double pos = engine.getTransport().getCurrentPosition();
        double sr = engine.getTransport().getSampleRate();
        if (sr <= 0.0) sr = 48000.0;
        double rewindAmount = sr * 5.0; // rewind 5 seconds
        engine.getTransport().setPosition(juce::jmax(0.0, pos - rewindAmount));
    };
    jumpEndButton.onClick = [this]() {
        engine.getTransport().setPosition(engine.getTimelineProject().getTotalDurationSamples());
    };
    
    startTimerHz(30);
}

TopToolbarComponent::~TopToolbarComponent() {}

void TopToolbarComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    g.setColour(DesignSystem::Colors::ModuleBackground);
    g.fillRect(0, 0, getWidth(), 28);
    
    g.setColour(DesignSystem::Colors::TextPrimary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
    g.drawText("File     Edit     Select     View     Record     Tracks     Generate     Effect     Analyze     Tools     Extra     Help",
               12, 0, getWidth() - 300, 28, juce::Justification::centredLeft, false);

    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.7f));
    g.fillRect(0, 28, getWidth(), 1);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TopToolbarComponent::resized() {
    auto bounds = getLocalBounds();
    auto topStrip = bounds.removeFromTop(28);
    workspaceLabel.setBounds(topStrip.removeFromRight(140).reduced(4, 0));
    shareButton.setBounds(topStrip.removeFromRight(70).reduced(2, 3));
    audioSetupButton.setBounds(topStrip.removeFromRight(95).reduced(2, 3));

    bounds.removeFromTop(6);
    auto lowerRow = bounds.reduced(8, 2);

    constexpr auto targetButtonSize = 30;

    // Action group: Undo, Redo, Save, Cut, Trim
    juce::FlexBox actionLayout;
    actionGroupContainer.setBounds(lowerRow.removeFromLeft(5 * targetButtonSize).withHeight(targetButtonSize));
    actionLayout.flexDirection = juce::FlexBox::Direction::row;
    actionLayout.items.add(juce::FlexItem(undoButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(redoButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(saveProjectButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(cutButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(trimButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.performLayout(actionGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);

    // Transport group: Pause, Play, Stop, JumpStart, Rewind, FastForward, Record, Loop, Metronome
    juce::FlexBox transportLayout;
    transportGroupContainer.setBounds(lowerRow.removeFromLeft(9 * targetButtonSize).withHeight(targetButtonSize));
    transportLayout.flexDirection = juce::FlexBox::Direction::row;
    for (auto* button : { &pauseButton, &playButton, &stopButton, &jumpStartButton, &rewindButton, &jumpEndButton, &recordButton, &loopButton, &metronomeToggle })
        transportLayout.items.add(juce::FlexItem(*button).withWidth(targetButtonSize).withHeight(targetButtonSize));
    transportLayout.performLayout(transportGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);

    // Zoom group
    juce::FlexBox toolsLayout;
    toolsGroupContainer.setBounds(lowerRow.removeFromLeft(120).withHeight(targetButtonSize));
    toolsLayout.flexDirection = juce::FlexBox::Direction::row;
    toolsLayout.items.add(juce::FlexItem(zoomOutButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    toolsLayout.items.add(juce::FlexItem(zoomLevelLabel).withWidth(60).withHeight(targetButtonSize));
    toolsLayout.items.add(juce::FlexItem(zoomInButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    toolsLayout.performLayout(toolsGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);
    projectNameLabel.setBounds(lowerRow.removeFromLeft(140).withHeight(targetButtonSize));

    // Right-side displays
    barsDisplay.setBounds(lowerRow.removeFromRight(82).withHeight(36).withY(30));
    timeDisplay.setBounds(lowerRow.removeFromRight(138).withHeight(36).withY(30));
    sigDisplay.setBounds(lowerRow.removeFromRight(52).withHeight(36).withY(30));
    bpmDisplay.setBounds(lowerRow.removeFromRight(68).withHeight(36).withY(30));
    
    cpuLabel.setBounds(lowerRow.removeFromRight(86).withHeight(targetButtonSize));
    settingsButton.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    mixerToggle.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    pianoRollToggle.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    followButton.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
}

void TopToolbarComponent::timerCallback() {
    playButton.setToggleState(engine.getTransport().isPlaying(), juce::dontSendNotification);
    recordButton.setToggleState(engine.getTransport().isRecording(), juce::dontSendNotification);
    
    double posSamples = engine.getTransport().getCurrentPosition();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    double posSeconds = posSamples / sampleRate;
    
    const auto totalSeconds = static_cast<int>(posSeconds);
    juce::String timeStr = juce::String::formatted("%02d h %02d m %02d.%02d s", totalSeconds / 3600, (totalSeconds / 60) % 60, totalSeconds % 60, static_cast<int>((posSeconds - std::floor(posSeconds)) * 100.0));
    timeDisplay.setValue(timeStr);
    
    double tempo = engine.getTransport().getTempo();
    if (tempo <= 0.0) tempo = 120.0;
    
    auto& project = engine.getTimelineProject();
    int num = project.getTimeSigNumerator();
    
    double totalBeats = posSeconds * (tempo / 60.0);
    juce::String barStr = juce::String::formatted("%d.%d.%02d", static_cast<int>(totalBeats / num) + 1, static_cast<int>(std::fmod(totalBeats, static_cast<double>(num))) + 1, static_cast<int>(std::fmod(totalBeats * 100.0, 100.0)));
    barsDisplay.setValue(barStr);
    
    bpmDisplay.setValue(juce::String(tempo, 1));
    if (!sigDisplay.valueLabel.isBeingEdited()) sigDisplay.setValue(juce::String::formatted("%d/%d", num, project.getTimeSigDenominator()));
    if (!projectNameLabel.isBeingEdited()) projectNameLabel.setText(project.getProjectName(), juce::dontSendNotification);
    
    followButton.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);
    loopButton.setToggleState(engine.getTransport().isLooping(), juce::dontSendNotification);

    // Real CPU meter from JUCE AudioDeviceManager
    cpuLoad = static_cast<float>(engine.getAudioDeviceManager().getJuceAudioDeviceManager().getCpuUsage()) * 100.0f;
    cpuLabel.setText("CPU: " + juce::String(static_cast<int>(cpuLoad)) + "%", juce::dontSendNotification);
    
    // Color the CPU label based on load
    if (cpuLoad > 80.0f)
        cpuLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    else if (cpuLoad > 50.0f)
        cpuLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    else
        cpuLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
}

} // namespace Nimbus::MainLayout