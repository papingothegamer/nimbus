#include "ChannelStripComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"

namespace {
    class AbletonRotaryLAF : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                              const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override {
            float radius = juce::jmin(width / 2.0f, height / 2.0f) - 2.0f;
            float centreX = x + width * 0.5f;
            float centreY = y + height * 0.5f;
            float rx = centreX - radius;
            float ry = centreY - radius;
            float rw = radius * 2.0f;
            float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour(juce::Colour(0xff121212));
            g.fillEllipse(rx, ry, rw, rw);
            g.setColour(juce::Colour(0xff888888));
            g.drawEllipse(rx, ry, rw, rw, 1.5f);

            float centerAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * 0.5f;
            juce::Path arc;
            arc.addCentredArc(centreX, centreY, radius - 1.5f, radius - 1.5f, 0.0f, centerAngle, angle, true);
            g.setColour(juce::Colour(0xfffdb913)); // Bright Ableton-style orange/yellow
            g.strokePath(arc, juce::PathStrokeType(2.0f));

            juce::Path p;
            p.addRectangle(-1.0f, -radius, 2.0f, radius * 0.8f);
            p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
            g.setColour(juce::Colours::white);
            g.fillPath(p);
        }
    };
    AbletonRotaryLAF rotaryLAF;
}

namespace Nimbus::MainLayout {

static void applySvgToButton(juce::DrawableButton& btn, const juce::String& normalIcon, juce::Colour normalColor, const juce::String& activeIcon, juce::Colour activeColor) {
    auto createTinted = [](const juce::String& name, juce::Colour tint) -> std::unique_ptr<juce::Drawable> {
        int size = 0;
        if (const char* data = BinaryData::getNamedResource(name.toUTF8(), size)) {
            juce::String str(data, (size_t)size);
            str = str.replace("currentColor", "#000000").replace("#212121", "#000000");
            if (auto xml = juce::XmlDocument::parse(str)) {
                if (auto svg = juce::Drawable::createFromSVG(*xml)) {
                    svg->replaceColour(juce::Colours::black, tint);
                    return svg;
                }
            }
        }
        return nullptr;
    };

    auto normalSvg = createTinted(normalIcon, normalColor);
    auto activeSvg = createTinted(activeIcon, activeColor);
    if (normalSvg && activeSvg) {
        btn.setImages(normalSvg.get(), nullptr, nullptr, nullptr, activeSvg.get(), nullptr, nullptr, nullptr);
    }
}

ChannelStripComponent::ChannelStripComponent(NimbusEngine& e, const juce::String& name, bool isStereo, bool isMaster)
    : engine(e), channelName(name), stereo(isStereo), master(isMaster)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(name, juce::dontSendNotification);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f));
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
            if (activeChannels[i]) {
                juce::String name = channelNames[i];
                if (name.length() > 10) name = name.substring(0, 10) + "...";
                inputComboBox.addItem(name, i + 10);
            }
        }
    }
    inputComboBox.setSelectedId(1, juce::dontSendNotification);
    inputComboBox.setJustificationType(juce::Justification::centred);
    inputComboBox.onChange = [this]() {
        if (trackIndex != -1) {
            int selectedId = inputComboBox.getSelectedId();
            engine.getTimelineProject().setTrackInputChannel(trackIndex, selectedId == 1 ? -1 : selectedId - 10);
        }
    };

    addAndMakeVisible(routingComboBox);
    routingComboBox.addItem("Master", 1);
    routingComboBox.addItem(stereo ? "Ext. Out" : "Synth Plugin", 2);
    routingComboBox.setSelectedId(1, juce::dontSendNotification);
    routingComboBox.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(meteredFader);
    addChildComponent(midiMeter);

    if (stereo) meteredFader.setTrackType(UI::MeteredFader::TrackType::StereoAudio);
    else meteredFader.setTrackType(UI::MeteredFader::TrackType::MonoAudio);
    
    addAndMakeVisible(pan);
    pan.setLookAndFeel(&rotaryLAF);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setDoubleClickReturnValue(true, 0.0);
    pan.onValueChange = [this] { engine.getTimelineProject().setTrackPan(trackIndex, static_cast<float>(pan.getValue())); };
    
    meteredFader.getSlider().setDoubleClickReturnValue(true, 0.0);
    meteredFader.getSlider().onValueChange = [this]() {
        if (trackIndex != -1) {
            float db = static_cast<float>(meteredFader.getSlider().getValue());
            float vol = juce::Decibels::decibelsToGain(db, -60.0f);
            engine.getTimelineProject().setTrackVolume(trackIndex, vol);
        }
    };

    if (!master) {
        addAndMakeVisible(muteButton);
        addAndMakeVisible(soloButton);
        addAndMakeVisible(armButton);
        
        muteButton.setClickingTogglesState(true);
        soloButton.setClickingTogglesState(true);
        armButton.setClickingTogglesState(true);
        
        muteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
        applySvgToButton(muteButton, DesignSystem::Iconography::VolumeOff, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::Unmute, juce::Colours::white);
        
        soloButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
        applySvgToButton(soloButton, DesignSystem::Iconography::Solo, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::Solo, juce::Colours::black);
        
        armButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        armButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
        applySvgToButton(armButton, DesignSystem::Iconography::RecordArm, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::RecordArm, juce::Colours::white);
        
        muteButton.onClick = [this] { engine.getTimelineProject().setTrackMuted(trackIndex, muteButton.getToggleState()); };
        soloButton.onClick = [this] { 
            if (master) {
                // Future: toggle master monitoring 
            } else {
                engine.getTimelineProject().setTrackSoloed(trackIndex, soloButton.getToggleState()); 
            }
        };
        armButton.onClick = [this] { engine.getTimelineProject().setTrackArmed(trackIndex, armButton.getToggleState()); };
        
        if (master) {
            muteButton.setVisible(false);
            soloButton.setVisible(true);
            armButton.setVisible(false);
            pan.setVisible(false);
            inputComboBox.setVisible(false);
            meteredFader.setTrackType(UI::MeteredFader::TrackType::StereoAudio);
            meteredFader.setVisible(true);
            midiMeter.setVisible(false);
        }
    } else {
        addAndMakeVisible(soloButton);
        soloButton.setClickingTogglesState(true);
        soloButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
        applySvgToButton(soloButton, DesignSystem::Iconography::Solo, juce::Colours::white.withAlpha(0.6f), DesignSystem::Iconography::Solo, juce::Colours::black);
    }

    engine.getTimelineProject().addListener(this);
    addAndMakeVisible(groupIndicator);
}

ChannelStripComponent::~ChannelStripComponent() {
    engine.getTimelineProject().removeListener(this);
    pan.setLookAndFeel(nullptr);
}

void ChannelStripComponent::setTrackIndex(int index) {
    trackIndex = index;
    if (trackIndex != -1 && !master) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        
        int inputChannel = engine.getTimelineProject().getTrackInputChannel(trackIndex);
        inputComboBox.setSelectedId(inputChannel == -1 ? 1 : inputChannel + 10, juce::dontSendNotification);
        
        juce::String defaultName = "Mono Audio Track";
        if (track.isGroup) defaultName = "Group Track";
        else if (track.isMidi) defaultName = "MIDI Track";
        else if (track.isStereo) defaultName = "Stereo Audio Track";

        if (nameLabel.getText().isEmpty() || nameLabel.getText() == channelName) {
            nameLabel.setText(track.name.isNotEmpty() ? track.name : defaultName, juce::dontSendNotification);
        }

        muteButton.setToggleState(track.isMuted, juce::dontSendNotification);
        soloButton.setToggleState(track.isSoloed, juce::dontSendNotification);
        armButton.setToggleState(engine.getTimelineProject().isTrackArmed(trackIndex), juce::dontSendNotification);

        float db = juce::Decibels::gainToDecibels(track.volume, -60.0f);
        meteredFader.getSlider().setValue(db, juce::dontSendNotification);
        pan.setValue(track.pan, juce::dontSendNotification);
        
        if (track.isMidi) {
            meteredFader.setVisible(false);
            midiMeter.setVisible(true);
        } else {
            midiMeter.setVisible(false);
            meteredFader.setVisible(true);
            meteredFader.setTrackType((track.isStereo || track.isGroup) ? UI::MeteredFader::TrackType::StereoAudio : UI::MeteredFader::TrackType::MonoAudio);
        }
    }
}

void ChannelStripComponent::labelTextChanged(juce::Label* labelThatHasChanged) {
    if (labelThatHasChanged == &nameLabel && trackIndex != -1 && !master) {
        auto newText = nameLabel.getText();
        if (newText.trim().isEmpty()) {
            const auto& track = engine.getTimelineProject().getTrack(trackIndex);
            juce::String defaultName = "Mono Audio Track";
            if (track.isGroup) defaultName = "Group Track";
            else if (track.isMidi) defaultName = "MIDI Track";
            else if (track.isStereo) defaultName = "Stereo Audio Track";
            nameLabel.setText(defaultName, juce::dontSendNotification);
            engine.getTimelineProject().setTrackName(trackIndex, defaultName);
        } else {
            engine.getTimelineProject().setTrackName(trackIndex, newText);
        }
    }
}

void ChannelStripComponent::trackNameChanged(int track, const juce::String& newName) {
    if (track == trackIndex && !master && !nameLabel.isBeingEdited()) nameLabel.setText(newName, juce::dontSendNotification);
}

void ChannelStripComponent::setLevelProvider(std::function<float()> provider) { levelProvider = std::move(provider); }

void ChannelStripComponent::updateMeters() {
    if (levelProvider) {
        float currentDb = juce::Decibels::gainToDecibels(levelProvider(), -60.0f);
        float normalized = (currentDb + 60.0f) / 70.0f;
        normalized = juce::jlimit(0.0f, 1.0f, normalized);
        
        if (midiMeter.isVisible()) midiMeter.setLevel(normalized);
        else meteredFader.setLevel(normalized, normalized);
    }
}

void ChannelStripComponent::paint(juce::Graphics& g) {
    bool isGroup = false;
    if (trackIndex != -1) {
        isGroup = engine.getTimelineProject().getTrack(trackIndex).isGroup;
    }
    
    g.fillAll(selected ? DesignSystem::Colors::PanelBackground.brighter(0.1f) : (isGroup ? DesignSystem::Colors::PanelBackground.brighter(0.05f) : DesignSystem::Colors::ModuleBackground));
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(getLocalBounds(), 1);
}

void ChannelStripComponent::resized() {
    auto bounds = getLocalBounds().reduced(2);
    
    if (trackIndex != -1) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        groupIndicator.setVisible(!track.parentGroupId.isNull());
        if (groupIndicator.isVisible()) groupIndicator.setBounds(bounds.removeFromBottom(8));
    } else groupIndicator.setVisible(false);
    
    nameLabel.setBounds(bounds.removeFromTop(18));
    if (!master) {
        inputComboBox.setBounds(bounds.removeFromTop(18).reduced(2, 0));
        bounds.removeFromTop(2);
    }
    routingComboBox.setBounds(bounds.removeFromBottom(18).reduced(2, 0));
    
    pan.setBounds(bounds.removeFromTop(36).reduced(8, 0));
    bounds.removeFromTop(8); 
    
    auto faderArea = bounds.removeFromRight(28).reduced(0, 4);
    if (midiMeter.isVisible()) midiMeter.setBounds(faderArea);
    else meteredFader.setBounds(faderArea);

    if (!master) {
        auto buttonCol = bounds.removeFromLeft(24).reduced(0, 4);
        armButton.setBounds(buttonCol.removeFromBottom(24).reduced(1));
        soloButton.setBounds(buttonCol.removeFromBottom(24).reduced(1));
        muteButton.setBounds(buttonCol.removeFromBottom(24).reduced(1));
    } else {
        auto buttonCol = bounds.removeFromLeft(24).reduced(0, 4);
        soloButton.setBounds(buttonCol.removeFromBottom(24).reduced(1));
    }
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
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) engine.getTimelineProject().removeTrack(trackIndex);
        });
    }
}

void ChannelStripComponent::trackMuteChanged(int track, bool isMuted) {
    if (track == trackIndex && !master) {
        muteButton.setToggleState(isMuted, juce::dontSendNotification);
    }
}

void ChannelStripComponent::trackArmChanged(int track, bool isArmed) {
    if (track == trackIndex) {
        armButton.setToggleState(isArmed, juce::dontSendNotification);
    }
}

void ChannelStripComponent::trackSoloChanged(int track, bool isSoloed) {
    if (track == trackIndex) {
        soloButton.setToggleState(isSoloed, juce::dontSendNotification);
    }
}

void ChannelStripComponent::trackVolumeChanged(int track, float volume) {
    if (track == trackIndex) {
        float db = juce::Decibels::gainToDecibels(volume, -60.0f);
        meteredFader.getSlider().setValue(db, juce::dontSendNotification);
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