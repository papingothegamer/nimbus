#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "InspectorWidgets.h"
#include "Core/Plugins/Stock/StockPluginUI.h"
#include "DataModel/AudioClip.h"
#include "DataModel/MidiClip.h"

namespace Nimbus::DetailView {

class ClipPropertiesComponent : public juce::Component {
public:
    ClipPropertiesComponent(NimbusEngine& engine);
    ~ClipPropertiesComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMidiMode(bool isMidi);
    void updateClipInfo(const juce::String& name, double startSamples, double lengthSamples);
    void setMidiClip(std::shared_ptr<MidiClip> clip);
    void setAudioClip(std::shared_ptr<AudioClip> clip);

private:
    void layoutPanels();
    void quantizeAction();
    
    NimbusEngine& engine;
    bool isMidiMode = true;
    std::shared_ptr<MidiClip> currentMidiClip;
    std::shared_ptr<AudioClip> currentAudioClip;

    // Scrolling
    juce::Viewport viewport;
    juce::Component contentContainer;

    // --- Clip Header ---
    juce::Label clipNameLabel;
    juce::Colour currentClipColor = juce::Colour(0xff333333);

    // --- Clip Panel ---
    UI::InspectorPanel clipPanel{"CLIP"};
    
    juce::Label startLabel{"", "Start"};
    UI::InspectorNumberBox startBox;
    juce::Label endLabel{"", "End"};
    UI::InspectorNumberBox endBox;
    
    UI::InspectorToggleButton loopButton{"Loop"};
    
    juce::Label positionLabel{"", "Position"};
    UI::InspectorNumberBox positionBox;
    juce::Label lengthLabel{"", "Length"};
    UI::InspectorNumberBox lengthBox;
    
    juce::Label signatureLabel{"", "Signature"};
    juce::Label signatureBox{"", "4 / 4"};
    juce::Label grooveLabel{"", "Groove"};
    juce::ComboBox grooveBox;
    
    // === Audio Panel ===
    UI::InspectorPanel audioPanel{"Audio"};
    UI::InspectorToggleButton matchTempoButton{"Match Tempo"}; // Shortened to fit panel
    UI::InspectorToggleButton followButton{"Follow"};
    juce::ComboBox algorithmBox;
    UI::InspectorToggleButton preservePitchButton{"Preserve Pitch"};
    
    // Time sub-column placeholders
    juce::ComboBox transientBox;
    juce::Label bpmLabel{"", "BPM"};
    juce::Label bpmBox;
    juce::TextButton halfSpeedBtn{"/2"};
    juce::TextButton doubleSpeedBtn{"*2"};
    
    // Gain & Pitch sub-column
    UI::InspectorVerticalGainSlider gainSlider;
    juce::Label gainLabel{"", "0.00 dB"};
    
    Nimbus::PluginDial pitchSlider{"Pitch", -24.0, 24.0, 0.0, " st", nullptr};
    juce::Label pitchLabel{"", "st"};
    juce::Label pitchBox{"", "0"};
    
    Nimbus::PluginDial formantSlider{"Formant", -24.0, 24.0, 0.0, " st", nullptr};
    juce::Label formantBox{"", "0"};
    
    Nimbus::PluginDial panSlider{"Pan", -1.0, 1.0, 0.0, "", nullptr};
    
    UI::InspectorToggleButton reverseButton{"Rev"};
    juce::TextButton editButton{"Edit"};
    
    // === Notes Panel ===
    UI::InspectorPanel notesPanel{"Notes"};
    
    // Scale section
    juce::ComboBox scaleKeyBox;
    juce::ComboBox scaleTypeBox;
    UI::InspectorToggleButton foldButton{"Fold"};
    
    // Quantize / Legato section
    UI::InspectorToggleButton quantizeButton{"Quantize"};
    UI::InspectorToggleButton legatoButton{"Legato"};
    
    Nimbus::PluginDial quantizeStrengthDial{"Strength", 0.0, 100.0, 100.0, "%", nullptr};
    Nimbus::PluginDial quantizeSwingDial{"Swing", 0.0, 100.0, 0.0, "%", nullptr};
    
    juce::Label transposeLabel{"", "Trans."};
    UI::InspectorNumberBox transposeBox;
    juce::Label velocityScaleLabel{"", "Velocity"};
    UI::InspectorNumberBox velocityScaleBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipPropertiesComponent)
};

} // namespace Nimbus::DetailView
