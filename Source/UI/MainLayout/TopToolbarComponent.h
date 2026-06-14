#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::MainLayout {

class TopToolbarComponent : public juce::Component {
public:
    TopToolbarComponent(NimbusEngine& engine);
    ~TopToolbarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onBrowserToggle;
    std::function<void()> onDetailToggle;

private:
    NimbusEngine& engine;

    // Transport controls
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageFitted};
    juce::DrawableButton pauseButton{"Pause", juce::DrawableButton::ImageFitted};
    juce::DrawableButton recordButton{"Record", juce::DrawableButton::ImageFitted};
    juce::DrawableButton loopButton{"Loop", juce::DrawableButton::ImageFitted};
    
    juce::DrawableButton browserToggleButton{"Browser", juce::DrawableButton::ImageFitted};
    juce::DrawableButton detailToggleButton{"Detail", juce::DrawableButton::ImageFitted};

    juce::Label tempoLabel;
    juce::TextButton tapTempoButton{"Tap"};
    juce::Label timeSignatureLabel;
    juce::ToggleButton metronomeToggle{"Metronome"};
    juce::ToggleButton snapToggle{"Snap"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopToolbarComponent)
};

} // namespace Nimbus::MainLayout
