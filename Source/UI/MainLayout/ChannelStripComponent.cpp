#include "ChannelStripComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::MainLayout {

ChannelStripComponent::ChannelStripComponent(NimbusEngine& e, const juce::String& name, bool isStereo, bool isMaster)
    : engine(e), channelName(name), stereo(isStereo), master(isMaster)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(inputComboBox);
    inputComboBox.addItem(stereo ? "Ext. In" : "All Ins", 1);
    inputComboBox.addItem("Resampling", 2);
    inputComboBox.setSelectedId(1, juce::dontSendNotification);
    inputComboBox.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(routingComboBox);
    routingComboBox.addItem("Master", 1);
    routingComboBox.addItem(stereo ? "Ext. Out" : "Synth Plugin", 2);
    routingComboBox.setSelectedId(1, juce::dontSendNotification);
    routingComboBox.setJustificationType(juce::Justification::centred);
    routingComboBox.onChange = [this]() {
        // TODO: Update track routing
        juce::Logger::writeToLog("Routing changed to: " + routingComboBox.getText());
    };

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

    if (!master) {
        addAndMakeVisible(muteButton);
        muteButton.setClickingTogglesState(true);
        muteButton.setToggleState(true, juce::dontSendNotification); // Active by default
        muteButton.onClick = [this] {
            bool isMuted = !muteButton.getToggleState();
            engine.getTimelineProject().setTrackMuted(trackIndex, isMuted);
        };
        addAndMakeVisible(soloButton);
        addAndMakeVisible(armButton);
        
        muteButton.setButtonText(DesignSystem::Iconography::Mute);
        soloButton.setButtonText(DesignSystem::Iconography::Solo);
        armButton.setButtonText(DesignSystem::Iconography::RecordArm);
        
        muteButton.setClickingTogglesState(true);
        soloButton.setClickingTogglesState(true);
        armButton.setClickingTogglesState(true);

        if (stereo) {
            addAndMakeVisible(stereoButton);
            stereoButton.setClickingTogglesState(true);
            stereoButton.setToggleState(stereo, juce::dontSendNotification);
            stereoButton.onClick = [this]() {
                stereo = stereoButton.getToggleState();
                repaint();
            };
        }
    }

    startTimerHz(30); // 30fps meter updates
    engine.getTimelineProject().addListener(this);
    
    addAndMakeVisible(groupIndicator);
}

ChannelStripComponent::~ChannelStripComponent() {
    engine.getTimelineProject().removeListener(this);
    stopTimer();
}

void ChannelStripComponent::setTrackIndex(int index) {
    trackIndex = index;
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
    bool isSelected = false;
    bool isGroup = false;
    if (trackIndex != -1) {
        isSelected = engine.getTimelineProject().isTrackSelected(trackIndex);
        isGroup = engine.getTimelineProject().getTrack(trackIndex).isGroup;
    }
    
    if (isSelected) {
        g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.1f));
    } else {
        g.fillAll(isGroup ? DesignSystem::Colors::PanelBackground.brighter(0.05f) : DesignSystem::Colors::ModuleBackground);
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
    
    // Group indicator at bottom if needed
    if (trackIndex != -1) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        if (!track.parentGroupId.isNull()) {
            groupIndicator.setVisible(true);
            groupIndicator.setBounds(bounds.removeFromBottom(10));
        } else {
            groupIndicator.setVisible(false);
        }
    } else {
        groupIndicator.setVisible(false);
    }
    
    // Reserve right edge for meters
    auto contentBounds = bounds.withTrimmedRight(15);
    
    // 1. Name label at top
    nameLabel.setBounds(contentBounds.removeFromTop(20));
    contentBounds.removeFromTop(2);
    
    // 2. Input combo
    if (!master) {
        inputComboBox.setBounds(contentBounds.removeFromTop(20).reduced(2, 0));
        contentBounds.removeFromTop(2);
    }
    
    // 3. Pan knob
    pan.setBounds(contentBounds.removeFromTop(40).reduced(8, 0));
    contentBounds.removeFromTop(2);
    
    // 4. Mute/Solo/Arm buttons
    if (!master) {
        auto buttonRow = contentBounds.removeFromTop(20).reduced(4, 0);
        int bw = buttonRow.getWidth() / 3;
        muteButton.setBounds(buttonRow.removeFromLeft(bw).reduced(1, 0));
        soloButton.setBounds(buttonRow.removeFromLeft(bw).reduced(1, 0));
        armButton.setBounds(buttonRow.reduced(1, 0));
        contentBounds.removeFromTop(2);
    }
    
    // 5. Volume label
    volumeLabel.setBounds(contentBounds.removeFromTop(20).reduced(4, 0));
    contentBounds.removeFromTop(2);
    
    // Bottom: Routing combo and stereo button
    if (!master) {
        if (stereo) {
            stereoButton.setBounds(contentBounds.removeFromBottom(20).reduced(2));
            contentBounds.removeFromBottom(2);
        }
        routingComboBox.setBounds(contentBounds.removeFromBottom(20).reduced(2, 0));
        contentBounds.removeFromBottom(2);
    } else {
        routingComboBox.setBounds(contentBounds.removeFromBottom(20).reduced(2, 0));
        contentBounds.removeFromBottom(2);
    }
    
    // 6. Fader takes remaining space
    fader.setBounds(contentBounds.reduced(4, 0));
}

void ChannelStripComponent::mouseDown(const juce::MouseEvent& event) {
    if (!master) {
        if (event.mods.isShiftDown()) {
            int lastSelected = engine.getTimelineProject().getLastSelectedTrack();
            if (lastSelected != -1) {
                engine.getTimelineProject().selectTrackRange(lastSelected, trackIndex);
            } else {
                engine.getTimelineProject().setTrackSelected(trackIndex, true);
            }
        } else if (event.mods.isCtrlDown() || event.mods.isCommandDown()) {
            engine.getTimelineProject().toggleTrackSelection(trackIndex);
        } else if (!event.mods.isPopupMenu()) {
            engine.getTimelineProject().setTrackSelected(trackIndex, true);
        }
    }
    if (onSelected) {
        onSelected();
    }

    if (event.mods.isPopupMenu() && !master) {
        juce::PopupMenu m;
        m.addItem(1, "Delete Track");
        m.addItem(2, "Group Tracks");
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                engine.getTimelineProject().removeTrack(trackIndex);
            } else if (result == 2) {
                engine.getTimelineProject().groupTracks(engine.getTimelineProject().getSelectedTracks());
            }
        });
    }
}

void ChannelStripComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex) {
        muteButton.setToggleState(!isMuted, juce::dontSendNotification);
    }
}

void ChannelStripComponent::trackSelectionChanged() {
    if (!master) {
        selected = engine.getTimelineProject().isTrackSelected(trackIndex);
        repaint();
    }
}

} // namespace Nimbus::MainLayout
