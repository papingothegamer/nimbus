#include "ClipPropertiesComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

ClipPropertiesComponent::ClipPropertiesComponent(NimbusEngine& e) : engine(e) {
    // --- Clip Panel ---
    addAndMakeVisible(clipPanel);
    clipPanel.addContent(&clipNameLabel);
    clipPanel.addContent(&loopButton);
    clipPanel.addContent(&signatureLabel);
    
    clipNameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f).withStyle(juce::Font::bold));
    clipNameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    clipNameLabel.setEditable(true, false, false);
    clipNameLabel.setText("Clip", juce::dontSendNotification);
    
    signatureLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    signatureLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    signatureLabel.setJustificationType(juce::Justification::centredLeft);
    
    // --- Audio Panel ---
    addChildComponent(audioPanel);
    audioPanel.addContent(&matchTempoButton);
    audioPanel.addContent(&preservePitchButton);
    audioPanel.addContent(&algorithmBox);
    
    algorithmBox.addItem("Melodic", 1);
    algorithmBox.addItem("Percussive", 2);
    algorithmBox.setJustificationType(juce::Justification::centredLeft);
    algorithmBox.setColour(juce::ComboBox::backgroundColourId, DesignSystem::Colors::PanelBackground.darker(0.5f));
    algorithmBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    algorithmBox.onChange = [this] {
        if (currentAudioClip) {
            currentAudioClip->setAlgorithm(static_cast<AudioClip::StretchAlgorithm>(algorithmBox.getSelectedId() - 1));
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    matchTempoButton.onClick = [this] {
        if (currentAudioClip) {
            currentAudioClip->setMatchDawTempo(matchTempoButton.getToggleState());
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    preservePitchButton.onClick = [this] {
        if (currentAudioClip) {
            currentAudioClip->setPreservePitch(preservePitchButton.getToggleState());
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    pitchDial = std::make_unique<NimbusRotaryDial>("Pitch", -12.0f, 12.0f, 0.0f, " st", [this](float v) {
        if (currentAudioClip) {
            currentAudioClip->setPitchShiftSemitones(static_cast<int>(v));
            engine.getTimelineProject().notifyClipModified();
        }
    });
    audioPanel.addContent(pitchDial.get());
    
    speedFader = std::make_unique<NimbusHorizontalFader>("Speed", 0.1f, 10.0f, 1.0f, "x", [this](float v) {
        if (currentAudioClip) {
            currentAudioClip->setSpeedMultiplier(v);
            engine.getTimelineProject().notifyClipModified();
        }
    });
    audioPanel.addContent(speedFader.get());
    
    audioPanel.addContent(&gainBox);
    gainBox.setRange(-36.0, 36.0, 0.1);
    gainBox.setSuffix(" dB");
    gainBox.setNumDecimalPlaces(1);
    
    // --- Notes Panel ---
    addChildComponent(notesPanel);
    notesPanel.addContent(&quantizeButton);
    notesPanel.addContent(&quantizeBox);
    notesPanel.addContent(&transposeBox);
    notesPanel.addContent(&velocityScaleBox);
    
    quantizeBox.setRange(0.0, 100.0, 1.0);
    quantizeBox.setSuffix(" %");
    quantizeBox.setNumDecimalPlaces(0);
    quantizeBox.setValue(100.0, juce::dontSendNotification);
    
    transposeBox.setRange(-24.0, 24.0, 1.0);
    transposeBox.setSuffix(" st");
    transposeBox.setNumDecimalPlaces(0);
    
    velocityScaleBox.setRange(0.0, 200.0, 1.0);
    velocityScaleBox.setSuffix(" %");
    velocityScaleBox.setNumDecimalPlaces(0);
    velocityScaleBox.setValue(100.0, juce::dontSendNotification);
    
    quantizeButton.onClick = [this] { quantizeAction(); };
}

void ClipPropertiesComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.darker(0.2f));
}

void ClipPropertiesComponent::resized() {
    layoutPanels();
}

void ClipPropertiesComponent::layoutPanels() {
    clipPanel.setVisible(false);
    audioPanel.setVisible(false);
    notesPanel.setVisible(false);
}

void ClipPropertiesComponent::setMidiMode(bool isMidi) {
    isMidiMode = isMidi;
    layoutPanels();
}

void ClipPropertiesComponent::updateClipInfo(const juce::String& name, double startSamples, double lengthSamples) {
    clipNameLabel.setText(name, juce::dontSendNotification);
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double lengthSecs = lengthSamples / sampleRate;
}

void ClipPropertiesComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentMidiClip = clip;
}

void ClipPropertiesComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    currentAudioClip = clip;
    if (clip) {
        matchTempoButton.setToggleState(clip->getMatchDawTempo(), juce::dontSendNotification);
        preservePitchButton.setToggleState(clip->getPreservePitch(), juce::dontSendNotification);
        algorithmBox.setSelectedId(static_cast<int>(clip->getAlgorithm()) + 1, juce::dontSendNotification);
        pitchDial->setValue(clip->getPitchShiftSemitones());
        speedFader->setValue(clip->getSpeedMultiplier());
        gainBox.setValue(clip->getGain(), juce::dontSendNotification);
    }
}

void ClipPropertiesComponent::quantizeAction() {
    if (!currentMidiClip) return;
    auto& seq = currentMidiClip->getSequence();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    
    // Fixed to 1/16th for now for simplicity, as we removed the GridBox to match Ableton's simple UI in this panel
    double gridSamples = 0.25 * secondsPerBeat * sampleRate;
    double strength = quantizeBox.getValue() / 100.0;
    
    for (int i = 0; i < seq.getNumEvents(); ++i) {
        auto* evt = seq.getEventPointer(i);
        if (evt->message.isNoteOn()) {
            double t = evt->message.getTimeStamp();
            double snapped = std::round(t / gridSamples) * gridSamples;
            double newT = t + (snapped - t) * strength;
            double offset = newT - t;
            evt->message.setTimeStamp(newT);
            if (evt->noteOffObject) {
                evt->noteOffObject->message.setTimeStamp(evt->noteOffObject->message.getTimeStamp() + offset);
            }
        }
    }
    seq.updateMatchedPairs();
    engine.getTimelineProject().notifyClipModified();
}

} // namespace Nimbus::DetailView
