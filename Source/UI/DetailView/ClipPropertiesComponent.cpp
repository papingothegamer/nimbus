#include "ClipPropertiesComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

ClipPropertiesComponent::ClipPropertiesComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(scrollView);
    scrollView.setViewedComponent(&contentContainer, false);
    scrollView.setScrollBarsShown(true, false);
    
    // Clip name
    contentContainer.addAndMakeVisible(clipNameLabel);
    clipNameLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f).withStyle(juce::Font::bold));
    clipNameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    clipNameLabel.setEditable(true, false, false);
    clipNameLabel.addListener(this);
    clipNameLabel.setText("Clip", juce::dontSendNotification);
    
    // Position section (shared)
    contentContainer.addAndMakeVisible(positionSection);
    positionSection.addContentComponent(&startLabel);
    positionSection.addContentComponent(&lengthLabel);
    positionSection.addContentComponent(&endLabel);
    startLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    startLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    lengthLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    lengthLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    endLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    endLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    positionSection.onExpandChanged = [this] { layoutSections(); };
    
    // === MIDI Notes Section ===
    contentContainer.addAndMakeVisible(midiNotesSection);
    
    quantizeGridBox.addItem("1/4", 1);
    quantizeGridBox.addItem("1/8", 2);
    quantizeGridBox.addItem("1/16", 3);
    quantizeGridBox.addItem("1/32", 4);
    quantizeGridBox.setSelectedId(3);
    quantizeGridBox.addListener(this);
    midiNotesSection.addContentComponent(&quantizeGridBox);
    
    quantizeSlider.setRange(0.0, 100.0, 1.0);
    quantizeSlider.setValue(100.0);
    quantizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    quantizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    quantizeSlider.addListener(this);
    midiNotesSection.addContentComponent(&quantizeSlider);
    
    quantizeButton.onClick = [this] {
        if (!currentMidiClip) return;
        auto& seq = currentMidiClip->getSequence();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        double tempo = engine.getTransport().getTempo();
        double secondsPerBeat = 60.0 / tempo;
        
        // Grid sizes: 1/4=1beat, 1/8=0.5, 1/16=0.25, 1/32=0.125
        double gridBeats[] = {1.0, 0.5, 0.25, 0.125};
        int gridIdx = quantizeGridBox.getSelectedId() - 1;
        if (gridIdx < 0 || gridIdx > 3) gridIdx = 2;
        double gridSamples = gridBeats[gridIdx] * secondsPerBeat * sampleRate;
        double strength = quantizeSlider.getValue() / 100.0;
        
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
    };
    midiNotesSection.addContentComponent(&quantizeButton);
    
    transposeSlider.setRange(-24.0, 24.0, 1.0);
    transposeSlider.setValue(0.0);
    transposeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 18);
    transposeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    transposeSlider.addListener(this);
    midiNotesSection.addContentComponent(&transposeSlider);
    
    transposeValueLabel.setText("Transpose: 0 st", juce::dontSendNotification);
    transposeValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    transposeValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    midiNotesSection.addContentComponent(&transposeValueLabel);
    
    velocityScaleSlider.setRange(0.0, 200.0, 1.0);
    velocityScaleSlider.setValue(100.0);
    velocityScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    velocityScaleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    velocityScaleSlider.addListener(this);
    midiNotesSection.addContentComponent(&velocityScaleSlider);
    
    humanizeSlider.setRange(0.0, 100.0, 1.0);
    humanizeSlider.setValue(0.0);
    humanizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    humanizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    humanizeSlider.addListener(this);
    midiNotesSection.addContentComponent(&humanizeSlider);
    
    midiNotesSection.onExpandChanged = [this] { layoutSections(); };
    
    // === MIDI Timing Section ===
    contentContainer.addAndMakeVisible(midiTimingSection);
    midiTimingSection.addContentComponent(&loopButton);
    midiTimingSection.addContentComponent(&signatureLabel);
    signatureLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(11.0f));
    signatureLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    
    grooveBox.addItem("None", 1);
    grooveBox.addItem("Swing 8th", 2);
    grooveBox.addItem("Swing 16th", 3);
    grooveBox.addItem("MPC 54%", 4);
    grooveBox.addItem("MPC 58%", 5);
    grooveBox.addItem("MPC 62%", 6);
    grooveBox.setSelectedId(1);
    grooveBox.addListener(this);
    midiTimingSection.addContentComponent(&grooveBox);
    midiTimingSection.onExpandChanged = [this] { layoutSections(); };
    
    loopButton.onClick = [this] {
        if (currentMidiClip) {
            currentMidiClip->setIsLooped(loopButton.getToggleState());
        }
    };
    
    // === MIDI Transform Section ===
    contentContainer.addAndMakeVisible(midiTransformSection);
    midiTransformSection.addContentComponent(&reverseButton);
    midiTransformSection.addContentComponent(&invertButton);
    midiTransformSection.addContentComponent(&legatoButton);
    midiTransformSection.addContentComponent(&duplicateButton);
    midiTransformSection.onExpandChanged = [this] { layoutSections(); };
    
    reverseButton.onClick = [this] {
        if (!currentMidiClip) return;
        auto& seq = currentMidiClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        double maxTime = 0.0, minTime = 1e10;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                if (t > maxTime) maxTime = t;
                if (t < minTime) minTime = t;
            }
        }
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                double newTime = maxTime - (t - minTime);
                evt->message.setTimeStamp(newTime);
                if (evt->noteOffObject) {
                    double length = evt->noteOffObject->message.getTimeStamp() - t;
                    evt->noteOffObject->message.setTimeStamp(newTime + length);
                }
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    invertButton.onClick = [this] {
        if (!currentMidiClip) return;
        auto& seq = currentMidiClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        int minPitch = 127, maxPitch = 0;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                int p = evt->message.getNoteNumber();
                if (p < minPitch) minPitch = p;
                if (p > maxPitch) maxPitch = p;
            }
        }
        int center = minPitch + (maxPitch - minPitch) / 2;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                int p = evt->message.getNoteNumber();
                int newPitch = juce::jlimit(0, 127, center - (p - center));
                evt->message.setNoteNumber(newPitch);
                if (evt->noteOffObject) evt->noteOffObject->message.setNoteNumber(newPitch);
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    legatoButton.onClick = [this] {
        if (!currentMidiClip) return;
        auto& seq = currentMidiClip->getSequence();
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double nextTime = currentMidiClip->getLengthSamples();
                for (int j = 0; j < seq.getNumEvents(); ++j) {
                    auto* nextEvt = seq.getEventPointer(j);
                    if (nextEvt->message.isNoteOn() && nextEvt->message.getTimeStamp() > evt->message.getTimeStamp()) {
                        nextTime = std::min(nextTime, nextEvt->message.getTimeStamp());
                    }
                }
                if (evt->noteOffObject) evt->noteOffObject->message.setTimeStamp(nextTime);
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    duplicateButton.onClick = [this] {
        if (!currentMidiClip) return;
        auto& seq = currentMidiClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        double maxTime = 0.0;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOff()) maxTime = std::max(maxTime, evt->message.getTimeStamp());
        }
        int originalEvents = seq.getNumEvents();
        for (int i = 0; i < originalEvents; ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                juce::MidiMessage newOn = evt->message;
                newOn.setTimeStamp(newOn.getTimeStamp() + maxTime);
                juce::MidiMessage newOff = juce::MidiMessage::noteOff(newOn.getChannel(), newOn.getNoteNumber(), (juce::uint8)0);
                if (evt->noteOffObject) newOff.setTimeStamp(evt->noteOffObject->message.getTimeStamp() + maxTime);
                else newOff.setTimeStamp(newOn.getTimeStamp() + 48000.0 * 0.25);
                seq.addEvent(newOn);
                seq.addEvent(newOff);
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    // === Audio Sample Section ===
    contentContainer.addAndMakeVisible(audioSampleSection);
    audioSampleSection.addContentComponent(&warpButton);
    audioSampleSection.addContentComponent(&preservePitchButton);
    
    pitchShiftSlider.setRange(-24.0, 24.0, 0.1);
    pitchShiftSlider.setValue(0.0);
    pitchShiftSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    pitchShiftSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchShiftSlider.addListener(this);
    audioSampleSection.addContentComponent(&pitchShiftSlider);
    
    pitchShiftValueLabel.setText("Pitch: 0.0 st", juce::dontSendNotification);
    pitchShiftValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    pitchShiftValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    audioSampleSection.addContentComponent(&pitchShiftValueLabel);
    
    timeStretchSlider.setRange(0.25, 4.0, 0.01);
    timeStretchSlider.setValue(1.0);
    timeStretchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 18);
    timeStretchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    timeStretchSlider.addListener(this);
    audioSampleSection.addContentComponent(&timeStretchSlider);
    
    timeStretchValueLabel.setText("Stretch: 1.00x", juce::dontSendNotification);
    timeStretchValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    timeStretchValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    audioSampleSection.addContentComponent(&timeStretchValueLabel);
    
    audioSampleSection.onExpandChanged = [this] { layoutSections(); };
    
    warpButton.onClick = [this] {
        if (currentAudioClip) currentAudioClip->setWarpEnabled(warpButton.getToggleState());
    };
    preservePitchButton.onClick = [this] {
        if (currentAudioClip) currentAudioClip->setPreservePitch(preservePitchButton.getToggleState());
    };
    
    // === Audio Gain Section ===
    contentContainer.addAndMakeVisible(audioGainSection);
    gainSlider.setRange(0.0, 2.0, 0.01);
    gainSlider.setValue(1.0);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.addListener(this);
    audioGainSection.addContentComponent(&gainSlider);
    
    gainValueLabel.setText("0.0 dB", juce::dontSendNotification);
    gainValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    gainValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    audioGainSection.addContentComponent(&gainValueLabel);
    audioGainSection.onExpandChanged = [this] { layoutSections(); };
    
    // === Audio Fades Section ===
    contentContainer.addAndMakeVisible(audioFadesSection);
    
    fadeInSlider.setRange(0.0, 10000.0, 1.0);
    fadeInSlider.setValue(0.0);
    fadeInSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    fadeInSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeInSlider.addListener(this);
    audioFadesSection.addContentComponent(&fadeInSlider);
    
    fadeInValueLabel.setText("Fade In: 0 ms", juce::dontSendNotification);
    fadeInValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    fadeInValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    audioFadesSection.addContentComponent(&fadeInValueLabel);
    
    fadeInCurveBox.addItem("Linear", 1);
    fadeInCurveBox.addItem("Logarithmic", 2);
    fadeInCurveBox.addItem("Exponential", 3);
    fadeInCurveBox.addItem("S-Curve", 4);
    fadeInCurveBox.setSelectedId(1);
    fadeInCurveBox.addListener(this);
    audioFadesSection.addContentComponent(&fadeInCurveBox);
    
    fadeOutSlider.setRange(0.0, 10000.0, 1.0);
    fadeOutSlider.setValue(0.0);
    fadeOutSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    fadeOutSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeOutSlider.addListener(this);
    audioFadesSection.addContentComponent(&fadeOutSlider);
    
    fadeOutValueLabel.setText("Fade Out: 0 ms", juce::dontSendNotification);
    fadeOutValueLabel.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    fadeOutValueLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);
    audioFadesSection.addContentComponent(&fadeOutValueLabel);
    
    fadeOutCurveBox.addItem("Linear", 1);
    fadeOutCurveBox.addItem("Logarithmic", 2);
    fadeOutCurveBox.addItem("Exponential", 3);
    fadeOutCurveBox.addItem("S-Curve", 4);
    fadeOutCurveBox.setSelectedId(1);
    fadeOutCurveBox.addListener(this);
    audioFadesSection.addContentComponent(&fadeOutCurveBox);
    
    audioFadesSection.onExpandChanged = [this] { layoutSections(); };
    
    setMidiMode(true);
}

void ClipPropertiesComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::ModuleBackground);
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
}

void ClipPropertiesComponent::resized() {
    auto area = getLocalBounds();
    scrollView.setBounds(area);
    layoutSections();
}

void ClipPropertiesComponent::layoutSections() {
    int y = 4;
    int w = juce::jmax(120, scrollView.getWidth() - 10);
    
    clipNameLabel.setBounds(4, y, w, 22);
    y += 26;
    
    positionSection.setBounds(0, y, w + 8, positionSection.getDesiredHeight());
    positionSection.resized();
    y += positionSection.getDesiredHeight();
    
    if (isMidiMode) {
        midiNotesSection.setBounds(0, y, w + 8, midiNotesSection.getDesiredHeight());
        midiNotesSection.resized();
        y += midiNotesSection.getDesiredHeight();
        
        midiTimingSection.setBounds(0, y, w + 8, midiTimingSection.getDesiredHeight());
        midiTimingSection.resized();
        y += midiTimingSection.getDesiredHeight();
        
        midiTransformSection.setBounds(0, y, w + 8, midiTransformSection.getDesiredHeight());
        midiTransformSection.resized();
        y += midiTransformSection.getDesiredHeight();
    } else {
        audioSampleSection.setBounds(0, y, w + 8, audioSampleSection.getDesiredHeight());
        audioSampleSection.resized();
        y += audioSampleSection.getDesiredHeight();
        
        audioGainSection.setBounds(0, y, w + 8, audioGainSection.getDesiredHeight());
        audioGainSection.resized();
        y += audioGainSection.getDesiredHeight();
        
        audioFadesSection.setBounds(0, y, w + 8, audioFadesSection.getDesiredHeight());
        audioFadesSection.resized();
        y += audioFadesSection.getDesiredHeight();
    }
    
    contentContainer.setBounds(0, 0, scrollView.getWidth(), y + 10);
}

void ClipPropertiesComponent::setMidiMode(bool isMidi) {
    isMidiMode = isMidi;
    
    midiNotesSection.setVisible(isMidi);
    midiTimingSection.setVisible(isMidi);
    midiTransformSection.setVisible(isMidi);
    
    audioSampleSection.setVisible(!isMidi);
    audioGainSection.setVisible(!isMidi);
    audioFadesSection.setVisible(!isMidi);
    
    layoutSections();
}

void ClipPropertiesComponent::updateClipInfo(const juce::String& name, double startSamples, double lengthSamples) {
    clipNameLabel.setText(name, juce::dontSendNotification);
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    int timeSigNum = engine.getTransport().getTimeSignatureNumerator();
    double secondsPerBar = secondsPerBeat * timeSigNum;
    
    double startSeconds = startSamples / sampleRate;
    double lengthSeconds = lengthSamples / sampleRate;
    double endSeconds = startSeconds + lengthSeconds;
    
    // Format as bars.beats.ticks
    auto formatTime = [&](double seconds) -> juce::String {
        int bars = static_cast<int>(seconds / secondsPerBar) + 1;
        double remaining = std::fmod(seconds, secondsPerBar);
        int beats = static_cast<int>(remaining / secondsPerBeat) + 1;
        int ticks = static_cast<int>(std::fmod(remaining, secondsPerBeat) / secondsPerBeat * 960);
        return juce::String(bars) + "." + juce::String(beats) + "." + juce::String(ticks).paddedLeft('0', 3);
    };
    
    startLabel.setText("Start: " + formatTime(startSeconds), juce::dontSendNotification);
    lengthLabel.setText("Len: " + formatTime(lengthSeconds), juce::dontSendNotification);
    endLabel.setText("End: " + formatTime(endSeconds), juce::dontSendNotification);
}

void ClipPropertiesComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentMidiClip = clip;
    currentAudioClip = nullptr;
    if (clip) {
        clipNameLabel.setText(clip->getName(), juce::dontSendNotification);
        loopButton.setToggleState(clip->getIsLooped(), juce::dontSendNotification);
    }
}

void ClipPropertiesComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    currentAudioClip = clip;
    currentMidiClip = nullptr;
    if (clip) {
        clipNameLabel.setText(clip->getName(), juce::dontSendNotification);
        gainSlider.setValue(clip->getGain(), juce::dontSendNotification);
        pitchShiftSlider.setValue(clip->getPitchShift(), juce::dontSendNotification);
        timeStretchSlider.setValue(clip->getTimeStretch(), juce::dontSendNotification);
        warpButton.setToggleState(clip->isWarpEnabled(), juce::dontSendNotification);
        preservePitchButton.setToggleState(clip->isPreservePitch(), juce::dontSendNotification);
        
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        fadeInSlider.setValue(clip->getFadeInSamples() / sampleRate * 1000.0, juce::dontSendNotification);
        fadeOutSlider.setValue(clip->getFadeOutSamples() / sampleRate * 1000.0, juce::dontSendNotification);
        fadeInCurveBox.setSelectedId(clip->getFadeInCurve() + 1, juce::dontSendNotification);
        fadeOutCurveBox.setSelectedId(clip->getFadeOutCurve() + 1, juce::dontSendNotification);
        
        // Update value labels
        float gainDb = (clip->getGain() > 0.0f) ? juce::Decibels::gainToDecibels(clip->getGain()) : -100.0f;
        gainValueLabel.setText(juce::String(gainDb, 1) + " dB", juce::dontSendNotification);
        pitchShiftValueLabel.setText("Pitch: " + juce::String(clip->getPitchShift(), 1) + " st", juce::dontSendNotification);
        timeStretchValueLabel.setText("Stretch: " + juce::String(clip->getTimeStretch(), 2) + "x", juce::dontSendNotification);
    }
}

void ClipPropertiesComponent::sliderValueChanged(juce::Slider* slider) {
    if (slider == &transposeSlider && currentMidiClip) {
        int semitones = static_cast<int>(transposeSlider.getValue());
        transposeValueLabel.setText("Transpose: " + juce::String(semitones) + " st", juce::dontSendNotification);
        // Note: Transpose is applied destructively when user changes the value
        // We store the current value but only apply on explicit action
    } else if (slider == &velocityScaleSlider && currentMidiClip) {
        // Velocity scaling is applied as a preview label, destructive on release
    } else if (slider == &gainSlider && currentAudioClip) {
        currentAudioClip->setGain(static_cast<float>(gainSlider.getValue()));
        float gainDb = (currentAudioClip->getGain() > 0.0f) ? juce::Decibels::gainToDecibels(currentAudioClip->getGain()) : -100.0f;
        gainValueLabel.setText(juce::String(gainDb, 1) + " dB", juce::dontSendNotification);
    } else if (slider == &pitchShiftSlider && currentAudioClip) {
        currentAudioClip->setPitchShift(static_cast<float>(pitchShiftSlider.getValue()));
        pitchShiftValueLabel.setText("Pitch: " + juce::String(currentAudioClip->getPitchShift(), 1) + " st", juce::dontSendNotification);
    } else if (slider == &timeStretchSlider && currentAudioClip) {
        currentAudioClip->setTimeStretch(static_cast<float>(timeStretchSlider.getValue()));
        timeStretchValueLabel.setText("Stretch: " + juce::String(currentAudioClip->getTimeStretch(), 2) + "x", juce::dontSendNotification);
    } else if (slider == &fadeInSlider && currentAudioClip) {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        currentAudioClip->setFadeInSamples(static_cast<int>(fadeInSlider.getValue() / 1000.0 * sampleRate));
        fadeInValueLabel.setText("Fade In: " + juce::String(static_cast<int>(fadeInSlider.getValue())) + " ms", juce::dontSendNotification);
    } else if (slider == &fadeOutSlider && currentAudioClip) {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        currentAudioClip->setFadeOutSamples(static_cast<int>(fadeOutSlider.getValue() / 1000.0 * sampleRate));
        fadeOutValueLabel.setText("Fade Out: " + juce::String(static_cast<int>(fadeOutSlider.getValue())) + " ms", juce::dontSendNotification);
    }
}

void ClipPropertiesComponent::comboBoxChanged(juce::ComboBox* comboBox) {
    if (comboBox == &fadeInCurveBox && currentAudioClip) {
        currentAudioClip->setFadeInCurve(fadeInCurveBox.getSelectedId() - 1);
    } else if (comboBox == &fadeOutCurveBox && currentAudioClip) {
        currentAudioClip->setFadeOutCurve(fadeOutCurveBox.getSelectedId() - 1);
    }
}

void ClipPropertiesComponent::labelTextChanged(juce::Label* label) {
    if (label == &clipNameLabel) {
        if (currentMidiClip) {
            currentMidiClip->setName(clipNameLabel.getText());
        } else if (currentAudioClip) {
            currentAudioClip->setName(clipNameLabel.getText());
        }
    }
}

} // namespace Nimbus::DetailView
