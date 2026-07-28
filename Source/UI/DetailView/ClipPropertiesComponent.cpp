#include "ClipPropertiesComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

ClipPropertiesComponent::ClipPropertiesComponent(NimbusEngine& e) : engine(e) {
    // --- Viewport & Container ---
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contentContainer, false);
    viewport.setScrollBarsShown(true, false); // vertical only
    
    contentContainer.addAndMakeVisible(clipNameLabel);
    clipNameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(14.0f).boldened());
    clipNameLabel.setJustificationType(juce::Justification::centred);
    clipNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    contentContainer.addAndMakeVisible(clipPanel);
    
    // Layout helpers
    auto setupLabel = [](juce::Label& lbl, const juce::String& text, float height = 11.0f) {
        lbl.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(height));
        lbl.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
        lbl.setJustificationType(juce::Justification::centredLeft);
        lbl.setText(text, juce::dontSendNotification);
        lbl.setBorderSize(juce::BorderSize<int>(0)); // Kill default padding to save space
    };
    
    auto setupBox = [](juce::Label& box, const juce::String& text) {
        box.setText(text, juce::dontSendNotification);
        box.setJustificationType(juce::Justification::centred);
        box.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        box.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        box.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
        box.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f).boldened());
        box.setEditable(true, false, false);
        box.setBorderSize(juce::BorderSize<int>(0)); // Crucial to stop text truncation
    };
    
    auto setupTimeBox = [](UI::InspectorNumberBox& box) {
        box.setRange(0.0, 1000.0, 0.01);
        box.textFormatter = [](double value) {
            int bars = static_cast<int>(value) + 1;
            int beats = static_cast<int>((value - (bars - 1)) * 4) + 1;
            int sixteenths = static_cast<int>(((value - (bars - 1)) * 4 - (beats - 1)) * 4) + 1;
            return juce::String(bars) + ". " + juce::String(beats) + ". " + juce::String(sixteenths);
        };
    };
    
    setupLabel(startLabel, "Start");
    setupTimeBox(startBox);
    setupLabel(endLabel, "End");
    setupTimeBox(endBox);
    
    setupLabel(positionLabel, "Pos.");
    setupTimeBox(positionBox);
    setupLabel(lengthLabel, "Len.");
    setupTimeBox(lengthBox);
    
    setupLabel(signatureLabel, "Sig.");
    setupBox(signatureBox, "4 / 4");
    setupLabel(grooveLabel, "Groove");
    grooveBox.addItem("None", 1);
    grooveBox.setSelectedId(1);
    grooveBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    grooveBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    
    clipPanel.addContent(&startLabel);
    clipPanel.addContent(&startBox);
    clipPanel.addContent(&endLabel);
    clipPanel.addContent(&endBox);
    clipPanel.addContent(&loopButton);
    clipPanel.addContent(&positionLabel);
    clipPanel.addContent(&positionBox);
    clipPanel.addContent(&lengthLabel);
    clipPanel.addContent(&lengthBox);
    clipPanel.addContent(&signatureLabel);
    clipPanel.addContent(&signatureBox);
    clipPanel.addContent(&grooveLabel);
    clipPanel.addContent(&grooveBox);
    
    // --- Audio Panel ---
    contentContainer.addAndMakeVisible(audioPanel);
    audioPanel.addContent(&matchTempoButton);
    matchTempoButton.setButtonText("Match"); // Shortened
    audioPanel.addContent(&followButton);
    audioPanel.addContent(&algorithmBox);
    preservePitchButton.setButtonText("Preserve"); // Shortened
    audioPanel.addContent(&preservePitchButton);
    
    algorithmBox.addItem("Beats", 1);
    algorithmBox.addItem("Tones", 2);
    algorithmBox.addItem("Texture", 3);
    algorithmBox.addItem("Re-Pitch", 4);
    algorithmBox.addItem("Complex", 5);
    algorithmBox.setSelectedId(5);
    algorithmBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    algorithmBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    algorithmBox.onChange = [this] {
        if (currentAudioClip) {
            currentAudioClip->setAlgorithm(static_cast<AudioClip::StretchAlgorithm>(algorithmBox.getSelectedId() - 1));
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    audioPanel.addContent(&transientBox);
    transientBox.addItem("Trans.", 1);
    transientBox.setSelectedId(1);
    transientBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    transientBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    
    setupLabel(bpmLabel, "BPM");
    setupBox(bpmBox, "120.0");
    
    halfSpeedBtn.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::ComponentBackground);
    doubleSpeedBtn.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::ComponentBackground);
    
    audioPanel.addContent(&bpmLabel);
    audioPanel.addContent(&bpmBox);
    audioPanel.addContent(&halfSpeedBtn);
    audioPanel.addContent(&doubleSpeedBtn);
    
    audioPanel.addContent(&gainSlider);
    setupLabel(gainLabel, "0.00 dB");
    gainLabel.setJustificationType(juce::Justification::centred);
    audioPanel.addContent(&gainLabel);
    
    gainSlider.onValueChange = [this] {
        if (currentAudioClip) {
            currentAudioClip->gain = static_cast<float>(gainSlider.getValue());
            gainLabel.setText(juce::String(gainSlider.getValue(), 2) + " dB", juce::dontSendNotification);
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    audioPanel.addContent(&pitchSlider);
    setupBox(pitchBox, "0");
    pitchSlider.getSlider().onValueChange = [this] {
        if (currentAudioClip) {
            currentAudioClip->pitchShiftSemitones = static_cast<int>(pitchSlider.getSlider().getValue());
            pitchBox.setText(juce::String(currentAudioClip->pitchShiftSemitones.get()), juce::dontSendNotification);
            engine.getTimelineProject().notifyClipModified();
        }
    };
    audioPanel.addContent(&pitchLabel);
    audioPanel.addContent(&pitchBox);
    
    audioPanel.addContent(&formantSlider);
    setupBox(formantBox, "0");
    formantSlider.getSlider().onValueChange = [this] {
        if (currentAudioClip) {
            // Note: AudioClip needs formantShiftSemitones later, this is a placeholder
            formantBox.setText(juce::String(formantSlider.getSlider().getValue()), juce::dontSendNotification);
            engine.getTimelineProject().notifyClipModified();
        }
    };
    audioPanel.addContent(&formantBox);
    
    audioPanel.addContent(&reverseButton);
    editButton.setColour(juce::TextButton::buttonColourId, DesignSystem::Colors::ComponentBackground);
    audioPanel.addContent(&editButton);
    
    matchTempoButton.onClick = [this] {
        if (currentAudioClip) {
            currentAudioClip->matchDawTempo = matchTempoButton.getToggleState();
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    preservePitchButton.onClick = [this] {
        if (currentAudioClip) {
            currentAudioClip->preservePitch = preservePitchButton.getToggleState();
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    reverseButton.onClick = [this] {
        if (currentAudioClip) {
            currentAudioClip->reverse = reverseButton.getToggleState();
            engine.getTimelineProject().notifyClipModified();
        }
    };
    
    // Lock out audio panel components
    audioPanel.setEnabled(false);
    
    // --- Notes Panel ---
    contentContainer.addAndMakeVisible(notesPanel);
    
    scaleKeyBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    scaleKeyBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    const char* keys[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 12; ++i) scaleKeyBox.addItem(keys[i], i + 1);
    scaleKeyBox.setSelectedId(1, juce::dontSendNotification);
    
    scaleTypeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    scaleTypeBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    scaleTypeBox.addItem("Major", 1);
    scaleTypeBox.addItem("Minor", 2);
    scaleTypeBox.addItem("Dorian", 3);
    scaleTypeBox.addItem("Phrygian", 4);
    scaleTypeBox.addItem("Lydian", 5);
    scaleTypeBox.addItem("Mixolydian", 6);
    scaleTypeBox.addItem("Locrian", 7);
    scaleTypeBox.setSelectedId(1, juce::dontSendNotification);
    
    notesPanel.addContent(&scaleKeyBox);
    notesPanel.addContent(&scaleTypeBox);
    notesPanel.addContent(&foldButton);
    
    notesPanel.addContent(&quantizeButton);
    notesPanel.addContent(&legatoButton);
    
    notesPanel.addContent(&quantizeStrengthDial);
    notesPanel.addContent(&quantizeSwingDial);
    
    notesPanel.addContent(&transposeLabel);
    notesPanel.addContent(&transposeBox);
    notesPanel.addContent(&velocityScaleLabel);
    notesPanel.addContent(&velocityScaleBox);
    
    transposeLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    transposeLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    transposeLabel.setJustificationType(juce::Justification::centredLeft);
    transposeLabel.setBorderSize(juce::BorderSize<int>(0));
    
    velocityScaleLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    velocityScaleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    velocityScaleLabel.setJustificationType(juce::Justification::centredLeft);
    velocityScaleLabel.setBorderSize(juce::BorderSize<int>(0));
    
    transposeBox.setRange(-24.0, 24.0, 1.0);
    transposeBox.setSuffix(" st");
    transposeBox.setNumDecimalPlaces(0);
    
    velocityScaleBox.setRange(0.0, 200.0, 1.0);
    velocityScaleBox.setSuffix(" %");
    velocityScaleBox.setNumDecimalPlaces(0);
    velocityScaleBox.setValue(100.0, juce::dontSendNotification);
    
    quantizeButton.onClick = [this] { quantizeAction(); };
    
    clipPanel.onFoldStateChanged = [this] { resized(); };
    audioPanel.onFoldStateChanged = [this] { resized(); };
    notesPanel.onFoldStateChanged = [this] { resized(); };
}

void ClipPropertiesComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.darker(0.2f));
    
    if (clipNameLabel.isVisible()) {
        auto bounds = clipNameLabel.getBounds();
        g.setColour(currentClipColor);
        g.fillRect(bounds);
        
        // Add a subtle bottom divider
        g.setColour(DesignSystem::Colors::Divider);
        g.drawHorizontalLine(bounds.getBottom(), bounds.getX(), bounds.getRight());
    }
}

void ClipPropertiesComponent::resized() {
    viewport.setBounds(getLocalBounds());
    layoutPanels();
}

void ClipPropertiesComponent::layoutPanels() {
    auto area = getLocalBounds().reduced(2);
    area.setPosition(0, 0);
    area.setWidth(viewport.getMaximumVisibleWidth() > 0 ? viewport.getMaximumVisibleWidth() - 4 : area.getWidth());
    
    int currentY = 0;
    const int spacing = 6;
    
    // --- CLIP HEADER ---
    if (clipNameLabel.getText().isNotEmpty()) {
        clipNameLabel.setVisible(true);
        clipNameLabel.setBounds(0, currentY, area.getWidth() + 4, 28);
        currentY += 28 + spacing;
    } else {
        clipNameLabel.setVisible(false);
    }
    
    // Helper to layout a row with label and component
    auto layoutRow = [](juce::Rectangle<int>& container, juce::Component& label, juce::Component& comp, int height = 20) {
        auto row = container.removeFromTop(height);
        label.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(2, 0));
        comp.setBounds(row.reduced(2, 0));
        container.removeFromTop(4); // padding
    };
    
    // --- CLIP PANEL ---
    if (clipPanel.isVisible()) {
        int height = clipPanel.isFolded() ? 18 : 170;
        clipPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!clipPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, clipPanel.getWidth() - 12, height - 23);
            layoutRow(content, startLabel, startBox);
            layoutRow(content, endLabel, endBox);
            
            auto loopRow = content.removeFromTop(20);
            loopButton.setBounds(loopRow.reduced(20, 0));
            content.removeFromTop(4);
            
            layoutRow(content, positionLabel, positionBox);
            layoutRow(content, lengthLabel, lengthBox);
            layoutRow(content, signatureLabel, signatureBox);
            layoutRow(content, grooveLabel, grooveBox);
        }
    }
    
    // --- AUDIO PANEL ---
    if (audioPanel.isVisible()) {
        int height = audioPanel.isFolded() ? 18 : 250;
        audioPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!audioPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, audioPanel.getWidth() - 12, height - 23);
            
            // Match and Follow on one row
            auto row1 = content.removeFromTop(20);
            matchTempoButton.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(2, 0));
            followButton.setBounds(row1.reduced(2, 0));
            content.removeFromTop(4);
            
            layoutRow(content, bpmLabel, bpmBox);
            
            auto row2 = content.removeFromTop(20);
            halfSpeedBtn.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2, 0));
            doubleSpeedBtn.setBounds(row2.reduced(2, 0));
            content.removeFromTop(4);
            
            auto row3 = content.removeFromTop(20);
            preservePitchButton.setBounds(row3.removeFromLeft(row3.getWidth() / 2).reduced(2, 0));
            reverseButton.setBounds(row3.reduced(2, 0));
            content.removeFromTop(4);
            
            // Full width dropdown for Algorithm
            algorithmBox.setBounds(content.removeFromTop(20).reduced(2, 0));
            content.removeFromTop(4);
            
            // Transients if Beats mode
            transientBox.setBounds(content.removeFromTop(20).reduced(2, 0));
            content.removeFromTop(10);
            
            // Dials row
            auto dialRow = content.removeFromTop(60);
            int third = dialRow.getWidth() / 3;
            gainSlider.setBounds(dialRow.removeFromLeft(third));
            pitchSlider.setBounds(dialRow.removeFromLeft(third));
            formantSlider.setBounds(dialRow);
        }
    }
    
    // --- NOTES PANEL ---
    if (notesPanel.isVisible()) {
        int height = notesPanel.isFolded() ? 18 : 200;
        notesPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!notesPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, notesPanel.getWidth() - 12, height - 23);
            
            auto row1 = content.removeFromTop(20);
            scaleKeyBox.setBounds(row1.removeFromLeft(row1.getWidth() / 3).reduced(2, 0));
            scaleTypeBox.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(2, 0));
            foldButton.setBounds(row1.reduced(2, 0));
            content.removeFromTop(4);
            
            auto row2 = content.removeFromTop(20);
            quantizeButton.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2, 0));
            legatoButton.setBounds(row2.reduced(2, 0));
            content.removeFromTop(10);
            
            auto dialRow = content.removeFromTop(60);
            quantizeStrengthDial.setBounds(dialRow.removeFromLeft(dialRow.getWidth() / 2));
            quantizeSwingDial.setBounds(dialRow);
            content.removeFromTop(10);
            
            auto tRow = content.removeFromTop(20);
            transposeLabel.setBounds(tRow.removeFromLeft(tRow.getWidth() / 2).reduced(2, 0));
            transposeBox.setBounds(tRow.reduced(2, 0));
            content.removeFromTop(4);
            
            auto vRow = content.removeFromTop(20);
            velocityScaleLabel.setBounds(vRow.removeFromLeft(vRow.getWidth() / 2).reduced(2, 0));
            velocityScaleBox.setBounds(vRow.reduced(2, 0));
        }
    }
    
    contentContainer.setSize(area.getWidth(), currentY);
}

void ClipPropertiesComponent::setMidiMode(bool isMidi) {
    isMidiMode = isMidi;
    audioPanel.setVisible(!isMidiMode);
    notesPanel.setVisible(isMidiMode);
    layoutPanels();
}

void ClipPropertiesComponent::updateClipInfo(const juce::String& name, double startSamples, double lengthSamples) {
    clipNameLabel.setText(name, juce::dontSendNotification);
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double tempo = engine.getTransport().getTempo();
    if (tempo <= 0.0) tempo = 120.0;
    
    double secondsPerBeat = 60.0 / tempo;
    double samplesPerBeat = secondsPerBeat * sampleRate;
    double samplesPerBar = samplesPerBeat * 4.0; // Assuming 4/4
    
    auto getBars = [&](double samples) {
        return samples / samplesPerBar;
    };
    
    startBox.setValue(getBars(startSamples), juce::dontSendNotification);
    endBox.setValue(getBars(startSamples + lengthSamples), juce::dontSendNotification);
    
    positionBox.setValue(getBars(startSamples), juce::dontSendNotification);
    lengthBox.setValue(getBars(lengthSamples), juce::dontSendNotification);
    
    bpmBox.setText(juce::String(tempo, 2), juce::dontSendNotification);
}

void ClipPropertiesComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentMidiClip = clip;
    if (clip) {
        int index = clip->colorIndex.get();
        if (index >= 0) {
            float hue = std::fmod(index * 0.381966f, 1.0f);
            currentClipColor = juce::Colour::fromHSV(hue, 0.6f, 0.95f, 1.0f);
        }
        repaint();
    }
}

void ClipPropertiesComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    currentAudioClip = clip;
    if (clip) {
        int index = clip->colorIndex.get();
        if (index >= 0) {
            float hue = std::fmod(index * 0.381966f, 1.0f);
            currentClipColor = juce::Colour::fromHSV(hue, 0.6f, 0.95f, 1.0f);
        }
        repaint();
        
        matchTempoButton.setToggleState(clip->matchDawTempo.get(), juce::dontSendNotification);
        preservePitchButton.setToggleState(clip->preservePitch.get(), juce::dontSendNotification);
        reverseButton.setToggleState(clip->reverse.get(), juce::dontSendNotification);
        algorithmBox.setSelectedId(static_cast<int>(clip->getAlgorithm()) + 1, juce::dontSendNotification);
        pitchSlider.setValue((float)clip->pitchShiftSemitones.get());
        pitchBox.setText(juce::String(clip->pitchShiftSemitones.get()), juce::dontSendNotification);
        panSlider.setValue((float)clip->pan.get());
        gainSlider.setValue(clip->gain.get(), juce::dontSendNotification);
        gainLabel.setText(juce::String(clip->gain.get(), 2) + " dB", juce::dontSendNotification);
    }
}

void ClipPropertiesComponent::quantizeAction() {
    if (!currentMidiClip) return;
    auto& seq = currentMidiClip->getSequence();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    
    double gridSamples = 0.25 * secondsPerBeat * sampleRate;
    double strength = quantizeStrengthDial.getSlider().getValue() / 100.0;
    
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
