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
    audioPanel.addContent(&warpButton);
    audioPanel.addContent(&preservePitchButton);
    audioPanel.addContent(&pitchShiftBox);
    audioPanel.addContent(&gainBox);
    
    pitchShiftBox.setRange(-24.0, 24.0, 1.0);
    pitchShiftBox.setSuffix(" st");
    pitchShiftBox.setNumDecimalPlaces(0);
    
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
    auto bounds = getLocalBounds();
    
    int panelWidth = 125;
    int margin = 5;
    
    // Layout Clip Panel
    auto clipBounds = bounds.removeFromLeft(panelWidth).reduced(margin);
    clipPanel.setBounds(clipBounds);
    
    // Position inside clipPanel
    clipNameLabel.setBounds(5, 5, panelWidth - 20, 20);
    signatureLabel.setBounds(5, 30, panelWidth - 20, 15);
    loopButton.setBounds(5, 55, 60, 20);
    
    // Layout Audio or Notes Panel
    if (isMidiMode) {
        audioPanel.setVisible(false);
        notesPanel.setVisible(true);
        
        auto notesBounds = bounds.removeFromLeft(panelWidth).reduced(margin);
        notesPanel.setBounds(notesBounds);
        
        quantizeButton.setBounds(5, 5, 60, 20);
        quantizeBox.setBounds(70, 5, 45, 20);
        
        transposeBox.setBounds(5, 30, 110, 20);
        velocityScaleBox.setBounds(5, 55, 110, 20);
        
    } else {
        notesPanel.setVisible(false);
        audioPanel.setVisible(true);
        
        auto audioBounds = bounds.removeFromLeft(panelWidth).reduced(margin);
        audioPanel.setBounds(audioBounds);
        
        warpButton.setBounds(5, 5, 50, 20);
        preservePitchButton.setBounds(60, 5, 55, 20);
        
        pitchShiftBox.setBounds(5, 30, 110, 20);
        gainBox.setBounds(5, 55, 110, 20);
    }
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
