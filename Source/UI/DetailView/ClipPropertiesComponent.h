#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "ExpandableSection.h"

namespace Nimbus::DetailView {

class ClipPropertiesComponent : public juce::Component, 
                                 public juce::Slider::Listener,
                                 public juce::ComboBox::Listener,
                                 public juce::Label::Listener {
public:
    ClipPropertiesComponent(NimbusEngine& engine);
    ~ClipPropertiesComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMidiMode(bool isMidi);
    void updateClipInfo(const juce::String& name, double startSamples, double lengthSamples);
    void setMidiClip(std::shared_ptr<MidiClip> clip);
    void setAudioClip(std::shared_ptr<AudioClip> clip);

    // Listeners
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void labelTextChanged(juce::Label* label) override;

private:
    void layoutSections();
    
    NimbusEngine& engine;
    bool isMidiMode = true;
    std::shared_ptr<MidiClip> currentMidiClip;
    std::shared_ptr<AudioClip> currentAudioClip;
    
    juce::Viewport scrollView;
    juce::Component contentContainer;

    // Clip header
    juce::Label clipNameLabel;
    
    // === Shared Sections ===
    ExpandableSection positionSection{"Position"};
    juce::Label startLabel{"start", "Start: 0.00s"};
    juce::Label lengthLabel{"length", "Length: 0.00s"};
    juce::Label endLabel{"end", "End: 0.00s"};
    
    // === MIDI Sections ===
    ExpandableSection midiNotesSection{"Notes"};
    juce::Slider quantizeSlider;      // Quantize strength 0-100%
    juce::ComboBox quantizeGridBox;   // 1/4, 1/8, 1/16, 1/32
    juce::TextButton quantizeButton{"Quantize"};
    juce::Slider transposeSlider;     // -24 to +24 semitones
    juce::Label transposeValueLabel;
    juce::Slider velocityScaleSlider; // 0-200% velocity scaling
    juce::Slider humanizeSlider;      // 0-100% timing randomization
    
    ExpandableSection midiTimingSection{"Timing"};
    juce::ToggleButton loopButton{"Loop"};
    juce::ComboBox grooveBox;
    juce::Label signatureLabel{"sig", "4/4"};
    
    ExpandableSection midiTransformSection{"Transform"};
    juce::TextButton reverseButton{"Reverse"};
    juce::TextButton invertButton{"Invert"};
    juce::TextButton legatoButton{"Legato"};
    juce::TextButton duplicateButton{"Duplicate"};
    
    // === Audio Sections ===
    ExpandableSection audioSampleSection{"Sample"};
    juce::ToggleButton warpButton{"Warp"};
    juce::ToggleButton preservePitchButton{"Preserve Pitch"};
    juce::Slider pitchShiftSlider;    // -24 to +24 semitones
    juce::Label pitchShiftValueLabel;
    juce::Slider timeStretchSlider;   // 0.25x to 4.0x
    juce::Label timeStretchValueLabel;
    
    ExpandableSection audioGainSection{"Gain & Mix"};
    juce::Slider gainSlider;          // 0.0 to 2.0 linear
    juce::Label gainValueLabel;
    
    ExpandableSection audioFadesSection{"Fades"};
    juce::Slider fadeInSlider;        // 0 to 10000ms
    juce::Label fadeInValueLabel;
    juce::ComboBox fadeInCurveBox;    // Linear, Log, Exp, S-Curve
    juce::Slider fadeOutSlider;       // 0 to 10000ms
    juce::Label fadeOutValueLabel;
    juce::ComboBox fadeOutCurveBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipPropertiesComponent)
};

} // namespace Nimbus::DetailView
