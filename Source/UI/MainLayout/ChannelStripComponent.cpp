#include "ChannelStripComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::MainLayout {

// Define the static SVG helper here so we don't have to change your Header file!
static void applySvgToButton(juce::DrawableButton& btn, const juce::String& iconName) {
    int size = 0;
    if (const char* data = BinaryData::getNamedResource(iconName.toUTF8(), size)) {
        if (auto svg = juce::Drawable::createFromImageData(data, size)) {
            // MAKE ICONS WHITE
            svg->replaceColour(juce::Colours::black, juce::Colours::white);
            btn.setImages(svg.get(), nullptr, nullptr, nullptr, svg.get(), nullptr, nullptr, nullptr);
        }
    }
}

ChannelStripComponent::ChannelStripComponent(NimbusEngine& e, const juce::String& name, bool isStereo, bool isMaster)
    : engine(e), channelName(name), stereo(isStereo), master(isMaster)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setEditable(true, false, false);
    nameLabel.addListener(this);

    addAndMakeVisible(inputComboBox);
    int itemIndex = 1;
    inputComboBox.addItem("No Input", itemIndex++);
    if (auto* device = engine.getAudioDeviceManager().getJuceAudioDeviceManager().getCurrentAudioDevice()) {
        auto activeChannels = device->getActiveInputChannels();
        auto channelNames = device->getInputChannelNames();
        for (int i = 0; i < channelNames.size(); ++i) {
            if (activeChannels[i]) inputComboBox.addItem(channelNames[i], i + 10);
        }
    }
    inputComboBox.addItem("Resampling", 100);
    inputComboBox.setSelectedId(1, juce::dontSendNotification);
    inputComboBox.setJustificationType(juce::Justification::centred);
    inputComboBox.onChange = [this]() {
        if (trackIndex != -1) {
            int selectedId = inputComboBox.getSelectedId();
            int inputIndex = -1;
            if (selectedId >= 10 && selectedId < 100) inputIndex = selectedId - 10;
            engine.getTimelineProject().setTrackInputChannel(trackIndex, inputIndex);
        }
    };

    addAndMakeVisible(routingComboBox);
    routingComboBox.addItem("Master", 1);
    routingComboBox.addItem(stereo ? "Ext. Out" : "Synth Plugin", 2);
    routingComboBox.setSelectedId(1, juce::dontSendNotification);
    routingComboBox.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(volumeLabel);
    volumeLabel.setText("-Inf", juce::dontSendNotification);
    volumeLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
    volumeLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);

    addAndMakeVisible(fader);
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    fader.setRange(0.0, 1.0, 0.01);
    fader.setValue(0.75);
    fader.setColour(juce::Slider::thumbColourId, DesignSystem::Colors::PrimaryAction);
    fader.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);

    addAndMakeVisible(pan);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setValue(0.0);
    pan.getProperties().set("isPan", true);
    pan.setDoubleClickReturnValue(true, 0.0);
    pan.onValueChange = [this]() {
        if (trackIndex != -1) engine.getTimelineProject().setTrackPan(trackIndex, static_cast<float>(pan.getValue()));
    };

    fader.onValueChange = [this]() {
        if (trackIndex != -1) {
            float vol = static_cast<float>(fader.getValue());
            engine.getTimelineProject().setTrackVolume(trackIndex, vol);
            float db = juce::Decibels::gainToDecibels(vol, -100.0f);
            if (db <= -60.0f) volumeLabel.setText("-Inf", juce::dontSendNotification);
            else volumeLabel.setText(juce::String(db, 1) + " dB", juce::dontSendNotification);
        }
    };

    if (!master) {
        addAndMakeVisible(muteButton);
        addAndMakeVisible(soloButton);
        addAndMakeVisible(armButton);
        
        muteButton.setClickingTogglesState(true);
        soloButton.setClickingTogglesState(true);
        armButton.setClickingTogglesState(true);
        
        applySvgToButton(soloButton, DesignSystem::Iconography::Solo);
        applySvgToButton(armButton, DesignSystem::Iconography::RecordArm);
        
        muteButton.onClick = [this] {
            bool isMuted = !muteButton.getToggleState();
            engine.getTimelineProject().setTrackMuted(trackIndex, isMuted);
            applySvgToButton(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
        };
        soloButton.onClick = [this]() { engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState()); };
        armButton.onClick = [this]() { engine.getTimelineProject().setTrackArmed(trackIndex, armButton.getToggleState()); };
    }

    engine.getTimelineProject().addListener(this);
    addAndMakeVisible(groupIndicator);
}

ChannelStripComponent::~ChannelStripComponent() {
    engine.getTimelineProject().removeListener(this);
}

void ChannelStripComponent::setTrackIndex(int index) {
    trackIndex = index;
    if (trackIndex != -1 && !master) {
        int inputChannel = engine.getTimelineProject().getTrackInputChannel(trackIndex);
        inputComboBox.setSelectedId(inputChannel == -1 ? 1 : inputChannel + 10, juce::dontSendNotification);
        
        bool isMuted = engine.getTimelineProject().getTrack(trackIndex).isMuted;
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
        applySvgToButton(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
        
        soloButton.setToggleState(engine.getTimelineProject().getTrack(trackIndex).isSoloed, juce::dontSendNotification);
        armButton.setToggleState(engine.getTimelineProject().isTrackArmed(trackIndex), juce::dontSendNotification);
    }
}

void ChannelStripComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel && trackIndex != -1 && !master) {
        engine.getTimelineProject().setTrackName(trackIndex, nameLabel.getText());
    }
}

void ChannelStripComponent::trackNameChanged(int track, const juce::String& newName) {
    if (track == trackIndex && !master && !nameLabel.isBeingEdited()) {
        nameLabel.setText(newName, juce::dontSendNotification);
    }
}

void ChannelStripComponent::setLevelProvider(std::function<float()> provider) { levelProvider = std::move(provider); }

void ChannelStripComponent::updateMeters() {
    if (levelProvider) {
        float newLevel = levelProvider();
        if (std::abs(newLevel - currentLevel) > 0.01f) {
            currentLevel = newLevel;
            auto bounds = getLocalBounds();
            repaint(bounds.removeFromRight(15).reduced(2, 40).withTrimmedBottom(20));
        }
    }
}

void ChannelStripComponent::paint(juce::Graphics& g) {
    bool isSelected = false;
    bool isGroup = false;
    if (trackIndex != -1) {
        isSelected = engine.getTimelineProject().isTrackSelected(trackIndex);
        isGroup = engine.getTimelineProject().getTrack(trackIndex).isGroup;
    }
    
    g.fillAll(isSelected ? DesignSystem::Colors::PanelBackground.brighter(0.1f) : (isGroup ? DesignSystem::Colors::PanelBackground.brighter(0.05f) : DesignSystem::Colors::ModuleBackground));
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(getLocalBounds(), 1);

    auto bounds = getLocalBounds();
    auto meterArea = bounds.removeFromRight(15).reduced(2, 40).withTrimmedBottom(20);
    
    if (master) {
        drawMeter(g, meterArea.removeFromLeft(meterArea.getWidth() / 2).reduced(1, 0), currentLevel); 
        drawMeter(g, meterArea.reduced(1, 0), currentLevel);
    } else {
        drawMeter(g, meterArea.reduced(2, 0), currentLevel);
    }
}

void ChannelStripComponent::drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level) {
    g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.2f));
    g.fillRect(bounds);
    juce::ColourGradient cg(juce::Colours::lime, bounds.getBottomLeft().toFloat(), juce::Colours::red, bounds.getTopLeft().toFloat(), false);
    cg.addColour(0.7f, juce::Colours::yellow);
    int height = juce::roundToInt(bounds.getHeight() * level);
    g.setGradientFill(cg);
    g.fillRect(bounds.withTrimmedTop(bounds.getHeight() - height));
}

void ChannelStripComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    
    if (trackIndex != -1) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        groupIndicator.setVisible(!track.parentGroupId.isNull());
        if (groupIndicator.isVisible()) groupIndicator.setBounds(bounds.removeFromBottom(10));
    } else groupIndicator.setVisible(false);
    
    auto contentBounds = bounds.withTrimmedRight(15);
    nameLabel.setBounds(contentBounds.removeFromTop(20));
    contentBounds.removeFromTop(2);
    
    if (!master) {
        inputComboBox.setBounds(contentBounds.removeFromTop(20).reduced(2, 0));
        contentBounds.removeFromTop(2);
    }
    
    pan.setBounds(contentBounds.removeFromTop(40).reduced(8, 0));
    contentBounds.removeFromTop(2);
    
    if (!master) {
        auto buttonRow = contentBounds.removeFromTop(20).reduced(4, 0);
        int bw = buttonRow.getWidth() / 3;
        muteButton.setBounds(buttonRow.removeFromLeft(bw).reduced(1, 0));
        soloButton.setBounds(buttonRow.removeFromLeft(bw).reduced(1, 0));
        armButton.setBounds(buttonRow.reduced(1, 0));
        contentBounds.removeFromTop(2);
    }
    
    volumeLabel.setBounds(contentBounds.removeFromTop(20).reduced(4, 0));
    contentBounds.removeFromTop(2);
    routingComboBox.setBounds(contentBounds.removeFromBottom(20).reduced(2, 0));
    contentBounds.removeFromBottom(2);
    fader.setBounds(contentBounds.reduced(4, 0));
}

void ChannelStripComponent::mouseDown(const juce::MouseEvent& event) {
    if (!master) {
        if (event.mods.isShiftDown()) {
            int lastSelected = engine.getTimelineProject().getLastSelectedTrack();
            if (lastSelected != -1) engine.getTimelineProject().selectTrackRange(lastSelected, trackIndex);
            else engine.getTimelineProject().setTrackSelected(trackIndex, true);
        } else if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
            engine.getTimelineProject().toggleTrackSelection(trackIndex);
        } else if (!event.mods.isPopupMenu()) {
            engine.getTimelineProject().setTrackSelected(trackIndex, true);
        }
    }
    if (onSelected) onSelected();

    if (event.mods.isPopupMenu() && !master) {
        juce::PopupMenu m;
        m.addItem(1, "Delete Track");
        m.addItem(2, "Group Tracks");
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) engine.getTimelineProject().removeTrack(trackIndex);
            else if (result == 2) engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks());
        });
    }
}

void ChannelStripComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex && !master) {
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
        applySvgToButton(muteButton, isMuted ? DesignSystem::Iconography::Mute : DesignSystem::Iconography::Unmute);
    }
}

void ChannelStripComponent::trackArmChanged(int track, bool isArmed) {
    if (track == trackIndex) armButton.setToggleState(isArmed, juce::dontSendNotification);
}

void ChannelStripComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) soloButton.setToggleState(isSoloed, juce::dontSendNotification);
}

void ChannelStripComponent::trackVolumeChanged(int track, float volume) {
    if (track == trackIndex) {
        fader.setValue(volume, juce::dontSendNotification);
        float db = juce::Decibels::gainToDecibels(volume, -100.0f);
        volumeLabel.setText(db <= -60.0f ? "-Inf" : juce::String(db, 1) + " dB", juce::dontSendNotification);
    }
}

void ChannelStripComponent::trackPanChanged(int track, float panValue) {
    if (track == trackIndex) pan.setValue(panValue, juce::dontSendNotification);
}

void ChannelStripComponent::trackInputChannelChanged(int track, int inputChannel) {
    if (track == trackIndex) inputComboBox.setSelectedId(inputChannel == -1 ? 1 : inputChannel + 10, juce::dontSendNotification);
}

void ChannelStripComponent::trackSelectionChanged() {
    if (!master) selected = engine.getTimelineProject().isTrackSelected(trackIndex);
    repaint();
}

} // namespace Nimbus::MainLayout