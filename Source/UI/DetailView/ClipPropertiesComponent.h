#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "AbletonWidgets.h"
#include "Core/Plugins/Stock/StockPluginUI.h"

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

    // === Clip Panel ===
    UI::AbletonPanel clipPanel{"Clip"};
    juce::Label clipNameLabel;
    UI::AbletonToggleButton loopButton{"Loop"};
    juce::Label signatureLabel{"sig", "4/4"};
    
    // === Audio Panel ===
    UI::AbletonPanel audioPanel{"Audio"};
    UI::AbletonToggleButton warpButton{"Warp"};
    UI::AbletonToggleButton matchTempoButton{"Match"};
    juce::ComboBox warpModeBox;
    std::unique_ptr<NimbusRotaryDial> pitchDial;
    UI::AbletonNumberBox gainBox;
    
    // === Notes Panel ===
    UI::AbletonPanel notesPanel{"Notes"};
    UI::AbletonNumberBox quantizeBox;
    UI::AbletonToggleButton quantizeButton{"Quantize"};
    UI::AbletonNumberBox transposeBox;
    UI::AbletonNumberBox velocityScaleBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipPropertiesComponent)
};

} // namespace Nimbus::DetailView
