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

void TopToolbarComponent::CpuMeterDisplay::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(DesignSystem::Colors::ComponentBackground.brighter(0.05f));
    g.fillRoundedRectangle(bounds, 3.0f);
    
    float fillWidth = bounds.getWidth() * (currentLoad / 100.0f);
    
    juce::Colour meterColor = DesignSystem::Colors::PrimaryAction;
    if (currentLoad > 80.0f) meterColor = juce::Colours::red;
    else if (currentLoad > 50.0f) meterColor = juce::Colours::orange;
    
    g.setColour(meterColor.withAlpha(0.8f));
    g.fillRoundedRectangle(bounds.withWidth(fillWidth), 3.0f);
    
    g.setColour(DesignSystem::Colors::TextPrimary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(9.0f).boldened());
    g.drawText("CPU " + juce::String(static_cast<int>(currentLoad)) + "%", bounds, juce::Justification::centred, false);
}

void TopToolbarComponent::CpuMeterDisplay::updateLoad(float newLoad) {
    if (std::abs(currentLoad - newLoad) > 0.5f) {
        currentLoad = newLoad;
        repaint();
    }
}

void TopToolbarComponent::loadSvgIcon(juce::DrawableButton& btn, const juce::String& iconName, juce::Colour color) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        juce::String svgStr(data, (size_t)size);
        juce::String colorStr = color.toDisplayString(false); // Fix: use RGB format without alpha
        svgStr = svgStr.replace("fill=\"#000000\"", "fill=\"#" + colorStr + "\"")
                       .replace("fill=\"#212121\"", "fill=\"#" + colorStr + "\"")
                       .replace("fill=\"currentColor\"", "fill=\"#" + colorStr + "\"")
                       .replace("<svg ", "<svg fill=\"#" + colorStr + "\" color=\"#" + colorStr + "\" ");

        if (auto xml = juce::XmlDocument::parse(svgStr)) {
            if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
            }
        }
    }
}

void TopToolbarComponent::loadToggleSvgIcon(juce::DrawableButton& btn, const juce::String& iconName, juce::Colour offColor, juce::Colour onColor) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        auto createSvg = [&](juce::Colour c) -> std::unique_ptr<juce::Drawable> {
            juce::String svgStr(data, (size_t)size);
            juce::String colorStr = c.toDisplayString(false); // Fix: use RGB format
            svgStr = svgStr.replace("fill=\"#000000\"", "fill=\"#" + colorStr + "\"")
                           .replace("fill=\"#212121\"", "fill=\"#" + colorStr + "\"")
                           .replace("fill=\"currentColor\"", "fill=\"#" + colorStr + "\"")
                           .replace("<svg ", "<svg fill=\"#" + colorStr + "\" color=\"#" + colorStr + "\" ");
            if (auto xml = juce::XmlDocument::parse(svgStr))
                return juce::Drawable::createFromSVG(*xml);
            return nullptr;
        };

        auto offSvg = createSvg(offColor);
        auto onSvg = createSvg(onColor);
        if (offSvg && onSvg) {
            btn.setImages(offSvg.get(), nullptr, nullptr, nullptr, onSvg.get(), nullptr, nullptr, nullptr);
        }
    }
}

void TopToolbarComponent::setZoomLevel(int zoomPercentage) {
    zoomLevelLabel.setText(juce::String(zoomPercentage) + "%", juce::dontSendNotification);
}

TopToolbarComponent::TopToolbarComponent(NimbusEngine& e) : engine(e) {
    engine.getTransport().addListener(this);
    engine.getTimelineProject().addListener(this);
    auto setupIconBtn = [](juce::DrawableButton& btn) {
        btn.getProperties().set("transparentBackground", true);
        btn.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        btn.setColour(juce::DrawableButton::backgroundOnColourId, DesignSystem::Colors::PrimaryAction.withAlpha(0.2f));
        btn.setWantsKeyboardFocus(false);
    };

    addAndMakeVisible(transportGroupContainer);
    addAndMakeVisible(toolsGroupContainer);
    addAndMakeVisible(actionGroupContainer);

    // --- Action Group: Undo, Redo, Copy, Trim, Paste ---
    setupIconBtn(undoButton);
    setupIconBtn(redoButton);
    setupIconBtn(saveProjectButton);
    setupIconBtn(copyButton);
    setupIconBtn(trimButton);
    setupIconBtn(pasteButton);
    loadSvgIcon(undoButton, DesignSystem::Iconography::Undo);
    loadSvgIcon(redoButton, DesignSystem::Iconography::Redo);
    loadSvgIcon(saveProjectButton, DesignSystem::Iconography::Save);
    loadSvgIcon(copyButton, DesignSystem::Iconography::Copy);
    loadSvgIcon(trimButton, DesignSystem::Iconography::Trim);
    loadSvgIcon(pasteButton, DesignSystem::Iconography::Paste);
    actionGroupContainer.addAndMakeVisible(undoButton);
    actionGroupContainer.addAndMakeVisible(redoButton);
    actionGroupContainer.addAndMakeVisible(copyButton);
    actionGroupContainer.addAndMakeVisible(trimButton);
    actionGroupContainer.addAndMakeVisible(pasteButton);
    
    addAndMakeVisible(saveProjectButton);
    
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
        if (zoom > 200) zoom = 200;
        
        if (onZoomLevelRequested) onZoomLevelRequested(zoom);
    };
    toolsGroupContainer.addAndMakeVisible(zoomLevelLabel);

    // --- Transport controls ---
    setupIconBtn(playButton);
    setupIconBtn(stopButton);
    setupIconBtn(recordButton);
    setupIconBtn(jumpStartButton);
    setupIconBtn(jumpEndButton);
    setupIconBtn(loopButton);
    setupIconBtn(metronomeToggle);

    loadSvgIcon(playButton, DesignSystem::Iconography::Play);
    loadSvgIcon(stopButton, DesignSystem::Iconography::Stop);
    loadToggleSvgIcon(recordButton, DesignSystem::Iconography::RecordGlobal, juce::Colours::white, juce::Colours::red);
    loadSvgIcon(jumpStartButton, DesignSystem::Iconography::JumpStart);
    loadSvgIcon(jumpEndButton, DesignSystem::Iconography::FastForward);
    loadToggleSvgIcon(loopButton, DesignSystem::Iconography::Loop, DesignSystem::Colors::TextSecondary, juce::Colours::white);
    loadToggleSvgIcon(metronomeToggle, DesignSystem::Iconography::Metronome, DesignSystem::Colors::TextSecondary, juce::Colours::white);

    transportGroupContainer.addAndMakeVisible(playButton);
    transportGroupContainer.addAndMakeVisible(stopButton);
    transportGroupContainer.addAndMakeVisible(recordButton);
    transportGroupContainer.addAndMakeVisible(jumpStartButton);
    transportGroupContainer.addAndMakeVisible(jumpEndButton);
    transportGroupContainer.addAndMakeVisible(loopButton);
    addAndMakeVisible(metronomeToggle);
    
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
    };
    
    metronomeToggle.setClickingTogglesState(true);

    // --- Follow playhead toggle ---
    setupIconBtn(followButton);
    loadToggleSvgIcon(followButton, DesignSystem::Iconography::Follow, DesignSystem::Colors::TextSecondary, juce::Colours::white);
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

    // --- CPU meter ---
    addAndMakeVisible(cpuMeter);

    // --- Button callbacks ---
    undoButton.onClick = [this]() { engine.getUndoManager().undo(); };
    redoButton.onClick = [this]() { engine.getUndoManager().redo(); };
    copyButton.onClick = [this]() { engine.getTimelineProject().copySelectedClips(); };
    trimButton.onClick = [this]() { /* Placeholder for Trim */ };
    pasteButton.onClick = [this]() {
        auto& project = engine.getTimelineProject();
        if (project.getSelectedTracks().size() > 0) {
            int track = project.getSelectedTracks().operator[](0);
            project.pasteClips(track, engine.getTransport().getCurrentPosition());
        }
    };
    
    zoomOutButton.onClick = [this]() {
        if (onZoomOut) onZoomOut();
    };
    
    zoomInButton.onClick = [this]() {
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
    playButton.onClick = [this]() {
        if (!engine.getTransport().isPlaying()) engine.getTransport().play();
        else engine.getTransport().stop();
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
    jumpEndButton.onClick = [this]() {
        engine.getTransport().setPosition(engine.getTimelineProject().getTotalDurationSamples());
    };
    
    // Initialize state
    transportStateChanged();
    transportTempoChanged(engine.getTransport().getTempo());
    transportLoopingChanged(engine.getTransport().isLooping());
    projectNameChanged(engine.getTimelineProject().getProjectName());
    timeSignatureChanged(engine.getTimelineProject().getTimeSigNumerator(), engine.getTimelineProject().getTimeSigDenominator());
    
    startTimerHz(30);
}

TopToolbarComponent::~TopToolbarComponent() {
    engine.getTransport().removeListener(this);
    engine.getTimelineProject().removeListener(this);
}

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

    // Action group: Undo, Redo, Copy, Trim, Paste
    juce::FlexBox actionLayout;
    actionGroupContainer.setBounds(lowerRow.removeFromLeft(5 * targetButtonSize).withHeight(targetButtonSize));
    actionLayout.flexDirection = juce::FlexBox::Direction::row;
    actionLayout.items.add(juce::FlexItem(undoButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(redoButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(copyButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(trimButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.items.add(juce::FlexItem(pasteButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    actionLayout.performLayout(actionGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);

    // Transport group: SkipStart, Play, Stop, SkipEnd, Record, Loop
    juce::FlexBox transportLayout;
    transportGroupContainer.setBounds(lowerRow.removeFromLeft(6 * targetButtonSize).withHeight(targetButtonSize));
    transportLayout.flexDirection = juce::FlexBox::Direction::row;
    for (auto* button : { &jumpStartButton, &playButton, &stopButton, &jumpEndButton, &recordButton, &loopButton })
        transportLayout.items.add(juce::FlexItem(*button).withWidth(targetButtonSize).withHeight(targetButtonSize));
    transportLayout.performLayout(transportGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);

    // Zoom group
    juce::FlexBox toolsLayout;
    toolsGroupContainer.setBounds(lowerRow.removeFromLeft(100).withHeight(targetButtonSize));
    toolsLayout.flexDirection = juce::FlexBox::Direction::row;
    toolsLayout.items.add(juce::FlexItem(zoomOutButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    toolsLayout.items.add(juce::FlexItem(zoomLevelLabel).withWidth(40).withHeight(targetButtonSize));
    toolsLayout.items.add(juce::FlexItem(zoomInButton).withWidth(targetButtonSize).withHeight(targetButtonSize));
    toolsLayout.performLayout(toolsGroupContainer.getLocalBounds().toFloat());

    lowerRow.removeFromLeft(8);
    projectNameLabel.setBounds(lowerRow.removeFromLeft(140).withHeight(targetButtonSize));
    saveProjectButton.setBounds(lowerRow.removeFromLeft(targetButtonSize).withHeight(targetButtonSize));

    // Right-side displays
    barsDisplay.setBounds(lowerRow.removeFromRight(82).withHeight(34).withY(26));
    timeDisplay.setBounds(lowerRow.removeFromRight(138).withHeight(34).withY(26));
    sigDisplay.setBounds(lowerRow.removeFromRight(52).withHeight(34).withY(26));
    bpmDisplay.setBounds(lowerRow.removeFromRight(68).withHeight(34).withY(26));
    
    int displaysWidth = 82 + 138 + 52 + 68;
    int cpuWidth = 64;
    cpuMeter.setBounds(getWidth() - 8 - (displaysWidth / 2) - (cpuWidth / 2), 62, cpuWidth, 12);
    
    settingsButton.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    mixerToggle.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    pianoRollToggle.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    followButton.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
    metronomeToggle.setBounds(lowerRow.removeFromRight(targetButtonSize).withHeight(targetButtonSize));
}

void TopToolbarComponent::timerCallback() {
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
    
    followButton.setToggleState(engine.isFollowPlayheadEnabled(), juce::dontSendNotification);

    // Real CPU meter from JUCE AudioDeviceManager
    cpuLoad = static_cast<float>(engine.getAudioDeviceManager().getJuceAudioDeviceManager().getCpuUsage()) * 100.0f;
    cpuMeter.updateLoad(cpuLoad);
}

void TopToolbarComponent::transportStateChanged() {
    bool isPlaying = engine.getTransport().isPlaying();
    playButton.setToggleState(isPlaying, juce::dontSendNotification);
    loadSvgIcon(playButton, isPlaying ? DesignSystem::Iconography::Pause : DesignSystem::Iconography::Play);
    recordButton.setToggleState(engine.getTransport().isRecording(), juce::dontSendNotification);
}

void TopToolbarComponent::transportTempoChanged(double newTempo) {
    if (newTempo <= 0.0) newTempo = 120.0;
    bpmDisplay.setValue(juce::String(newTempo, 1));
}

void TopToolbarComponent::transportLoopingChanged(bool isLooping) {
    loopButton.setToggleState(isLooping, juce::dontSendNotification);
}

void TopToolbarComponent::projectNameChanged(const juce::String& newName) {
    if (!projectNameLabel.isBeingEdited()) {
        projectNameLabel.setText(newName, juce::dontSendNotification);
    }
}

void TopToolbarComponent::timeSignatureChanged(int num, int den) {
    if (!sigDisplay.valueLabel.isBeingEdited()) {
        sigDisplay.setValue(juce::String::formatted("%d/%d", num, den));
    }
}

} // namespace Nimbus::MainLayout