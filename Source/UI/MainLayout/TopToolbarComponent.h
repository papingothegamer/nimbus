#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::MainLayout {

class TopToolbarComponent : public juce::Component, public juce::Timer {
public:
    TopToolbarComponent(NimbusEngine& engine);
    ~TopToolbarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    std::function<void()> onBrowserToggle;
    std::function<void()> onDetailToggle;

private:
    NimbusEngine& engine;

    // Transport controls
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopButton{"Stop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton arrRecordButton{"ArrRecord", juce::DrawableButton::ImageFitted};
    juce::TextButton autoArmButton{"A"};
    juce::TextButton reenableAutoButton{"<-"};
    juce::TextButton sessionRecordButton{"O"};
    juce::TextButton captureMidiButton{"[C]"};
    
    juce::DrawableButton loopButton{"Loop", juce::DrawableButton::ImageFitted};
    juce::TextButton punchInButton{"<"};
    juce::TextButton punchOutButton{">"};
    
    juce::Label loopStartLabel;
    juce::Label loopLengthLabel;
    
    juce::ToggleButton linkToggle{"Link"};
    juce::TextButton followPlayheadToggle{"Follow"};
    juce::TextButton tapTempoButton{"Tap"};
    juce::Label tempoLabel;
    juce::TextButton nudgeDownButton{"<"};
    juce::TextButton nudgeUpButton{">"};
    juce::Label timeSigNumLabel;
    juce::Label timeSigDenLabel;
    
    juce::ToggleButton metronomeToggle{"O O"};
    juce::ComboBox quantizeBox;

    juce::ToggleButton drawModeToggle{"Draw"};
    juce::ToggleButton compMidiToggle{"Keyb"};
    juce::ToggleButton keyMapToggle{"KEY"};
    juce::ToggleButton midiMapToggle{"MIDI"};
    juce::Label cpuLabel;

    juce::DrawableButton browserToggleButton{"Browser", juce::DrawableButton::ImageFitted};
    juce::DrawableButton detailToggleButton{"Detail", juce::DrawableButton::ImageFitted};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopToolbarComponent)
};

} // namespace Nimbus::MainLayout
