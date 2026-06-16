#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Iconography.h"

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
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::TextButton arrRecordButton{"ArrRecord"};
    juce::TextButton autoArmButton{"A"};
    juce::TextButton reenableAutoButton{"<-"};
    juce::TextButton sessionRecordButton{"O"};
    juce::TextButton captureMidiButton{"[C]"};
    
    juce::TextButton loopButton{"Loop"};
    juce::TextButton punchInButton{"<"};
    juce::TextButton punchOutButton{">"};
    
    juce::Label loopStartLabel;
    juce::Label loopLengthLabel;
    
    juce::TextButton linkToggle{"Link"};
    juce::TextButton followPlayheadToggle{"Follow"};
    juce::TextButton tapTempoButton{"Tap"};
    juce::Label tempoLabel;
    juce::TextButton nudgeDownButton{"<"};
    juce::TextButton nudgeUpButton{">"};
    juce::Label timeSigNumLabel;
    juce::Label timeSigDenLabel;
    
    juce::TextButton metronomeToggle{"O O"};
    juce::ComboBox quantizeBox;

    juce::TextButton drawModeToggle{DesignSystem::Iconography::Pencil};
    juce::TextButton compMidiToggle{DesignSystem::Iconography::Piano};
    juce::TextButton keyMapToggle{"KEY"};
    juce::TextButton midiMapToggle{DesignSystem::Iconography::Midi};
    juce::Label cpuLabel;

    juce::TextButton browserToggleButton{DesignSystem::Iconography::Sidebar};
    juce::TextButton detailToggleButton{DesignSystem::Iconography::DetailView};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopToolbarComponent)
};

} // namespace Nimbus::MainLayout
