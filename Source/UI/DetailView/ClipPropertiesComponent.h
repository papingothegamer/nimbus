#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "AbletonWidgets.h"
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

    // --- Clip Panel ---
    UI::AbletonPanel clipPanel{"CLIP"};
    
    juce::Label startLabel{"", "Start"};
    UI::AbletonNumberBox startBox;
    juce::Label endLabel{"", "End"};
    UI::AbletonNumberBox endBox;
    
    UI::AbletonToggleButton loopButton{"Loop"};
    
    juce::Label positionLabel{"", "Position"};
    UI::AbletonNumberBox positionBox;
    juce::Label lengthLabel{"", "Length"};
    UI::AbletonNumberBox lengthBox;
    
    juce::Label signatureLabel{"", "Signature"};
    juce::Label signatureBox{"", "4 / 4"};
    juce::Label grooveLabel{"", "Groove"};
    juce::ComboBox grooveBox;
    
    // === Audio Panel ===
    UI::AbletonPanel audioPanel{"Audio"};
    UI::AbletonToggleButton matchTempoButton{"Match Tempo"}; // Shortened to fit panel
    UI::AbletonToggleButton followButton{"Follow"};
    juce::ComboBox algorithmBox;
    UI::AbletonToggleButton preservePitchButton{"Preserve Pitch"};
    
    // Time sub-column placeholders
    juce::ComboBox transientBox;
    juce::Label bpmLabel{"", "BPM"};
    juce::Label bpmBox;
    juce::TextButton halfSpeedBtn{"/2"};
    juce::TextButton doubleSpeedBtn{"*2"};
    
    // Gain & Pitch sub-column
    UI::AbletonVerticalGainSlider gainSlider;
    juce::Label gainLabel{"", "0.00 dB"};
    
    Nimbus::PluginDial pitchSlider{"Pitch", -24.0, 24.0, 0.0, " st", nullptr};
    juce::Label pitchLabel{"", "st"};
    juce::Label pitchBox{"", "0"};
    
    Nimbus::PluginDial panSlider{"Pan", -1.0, 1.0, 0.0, "", nullptr};
    
    UI::AbletonToggleButton reverseButton{"Rev"};
    juce::TextButton editButton{"Edit"};
    
    // === Notes Panel ===
    UI::AbletonPanel notesPanel{"Notes"};
    UI::AbletonNumberBox quantizeBox;
    UI::AbletonToggleButton quantizeButton{"Quantize"};
    UI::AbletonNumberBox transposeBox;
    UI::AbletonNumberBox velocityScaleBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipPropertiesComponent)
};

} // namespace Nimbus::DetailView