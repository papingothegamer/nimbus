#include "ClipPropertiesComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

ClipPropertiesComponent::ClipPropertiesComponent(NimbusEngine& e) : engine(e) {
    // --- Viewport & Container ---
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contentContainer, false);
    viewport.setScrollBarsShown(true, false); // vertical only
    
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
    
    auto setupTimeBox = [](UI::AbletonNumberBox& box) {
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
    
    clipPanel.onFoldStateChanged = [this] { resized(); };
    audioPanel.onFoldStateChanged = [this] { resized(); };
    notesPanel.onFoldStateChanged = [this] { resized(); };
}

void ClipPropertiesComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.darker(0.2f));
}

void ClipPropertiesComponent::resized() {
    viewport.setBounds(getLocalBounds());
    layoutPanels();
}

void ClipPropertiesComponent::layoutPanels() {
    auto area = getLocalBounds().reduced(2);
    area.setPosition(0, 0);
    area.setWidth(viewport.getMaximumVisibleWidth() > 0 ? viewport.getMaximumVisibleWidth() - 4 : area.getWidth());
    
    int currentY = 2;
    const int spacing = 4;
    
    // --- CLIP PANEL ---
    if (clipPanel.isVisible()) {
        int height = clipPanel.isFolded() ? 18 : 155;
        clipPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!clipPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, clipPanel.getWidth() - 12, height - 23);
            int half = content.getWidth() / 2;
            
            auto r1_lbl = content.removeFromTop(12);
            startLabel.setBounds(r1_lbl.removeFromLeft(half));
            endLabel.setBounds(r1_lbl);
            
            auto r1_box = content.removeFromTop(18);
            startBox.setBounds(r1_box.removeFromLeft(half).reduced(2, 0));
            endBox.setBounds(r1_box.reduced(2, 0));
            
            content.removeFromTop(spacing);
            
            loopButton.setBounds(content.removeFromTop(18));
            
            content.removeFromTop(spacing);
            
            auto r2_lbl = content.removeFromTop(12);
            positionLabel.setBounds(r2_lbl.removeFromLeft(half));
            lengthLabel.setBounds(r2_lbl);
            
            auto r2_box = content.removeFromTop(18);
            positionBox.setBounds(r2_box.removeFromLeft(half).reduced(2, 0));
            lengthBox.setBounds(r2_box.reduced(2, 0));
            
            content.removeFromTop(spacing);
            
            auto r3_lbl = content.removeFromTop(12);
            signatureLabel.setBounds(r3_lbl.removeFromLeft(half));
            grooveLabel.setBounds(r3_lbl);
            
            auto r3_box = content.removeFromTop(18);
            signatureBox.setBounds(r3_box.removeFromLeft(half).reduced(2, 0));
            grooveBox.setBounds(r3_box.reduced(2, 0));
        }
    }
    
    // --- AUDIO PANEL ---
    if (audioPanel.isVisible()) {
        int height = audioPanel.isFolded() ? 18 : 185;
        audioPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!audioPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, audioPanel.getWidth() - 12, height - 23);
            auto leftCol = content.removeFromLeft(100);
            auto rightCol = content.withTrimmedLeft(4);
            
            // Left Col Setup
            auto lRow1 = leftCol.removeFromTop(18);
            matchTempoButton.setBounds(lRow1.removeFromLeft(48).reduced(1,0));
            followButton.setBounds(lRow1.reduced(1,0));
            
            leftCol.removeFromTop(spacing);
            
            bpmLabel.setBounds(leftCol.removeFromTop(12));
            auto lRow2 = leftCol.removeFromTop(18);
            bpmBox.setBounds(lRow2.removeFromLeft(48).reduced(1,0));
            halfSpeedBtn.setBounds(lRow2.removeFromLeft(25).reduced(1,0));
            doubleSpeedBtn.setBounds(lRow2.reduced(1,0));
            
            leftCol.removeFromTop(spacing);
            preservePitchButton.setBounds(leftCol.removeFromTop(18));
            
            leftCol.removeFromTop(spacing);
            algorithmBox.setBounds(leftCol.removeFromTop(18));
            
            leftCol.removeFromTop(spacing);
            transientBox.setBounds(leftCol.removeFromTop(18));
            
            leftCol.removeFromTop(spacing);
            auto revRow = leftCol.removeFromTop(18);
            reverseButton.setBounds(revRow.removeFromLeft(40));
            
            // Right Col Setup (Gain Slider & Pitch Knob)
            gainSlider.setBounds(rightCol.removeFromTop(65).reduced(4, 0));
            gainLabel.setBounds(rightCol.removeFromTop(16));
            gainLabel.setJustificationType(juce::Justification::centred);
            
            rightCol.removeFromTop(spacing);
            pitchSlider.setBounds(rightCol.removeFromTop(50));
            pitchBox.setBounds(rightCol.removeFromTop(18));
        }
    }
    
    // --- NOTES PANEL ---
    if (notesPanel.isVisible()) {
        int height = notesPanel.isFolded() ? 18 : 120;
        notesPanel.setBounds(2, currentY, area.getWidth(), height);
        currentY += height + spacing;
        
        if (!notesPanel.isFolded()) {
            auto content = juce::Rectangle<int>(0, 0, notesPanel.getWidth() - 20, height - 23);
            quantizeButton.setBounds(content.removeFromTop(20));
            content.removeFromTop(spacing);
            quantizeBox.setBounds(content.removeFromTop(20));
            content.removeFromTop(spacing);
            transposeBox.setBounds(content.removeFromTop(20));
            content.removeFromTop(spacing);
            velocityScaleBox.setBounds(content.removeFromTop(20));
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
}

void ClipPropertiesComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    currentAudioClip = clip;
    if (clip) {
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