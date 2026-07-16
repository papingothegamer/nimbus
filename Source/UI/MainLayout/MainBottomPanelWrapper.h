#pragma once

#include <JuceHeader.h>
#include "BottomMixerComponent.h"
#include "DetailViewComponent.h"

namespace Nimbus::MainLayout {

/** Owns the lower workspace frame and the two explicit Audacity-like views. */
class MainBottomPanelWrapper final : public juce::Component {
public:
    explicit MainBottomPanelWrapper(NimbusEngine& engine);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void showDeviceView();
    void showMixerView();

private:
    BottomMixerComponent mixer;
    DetailViewComponent detail;
    juce::TextButton mixerTab { "MIXER" };
    juce::TextButton deviceTab { "CLIP / DEVICE VIEW" };
    juce::StretchableLayoutManager splitLayout;
    bool deviceViewActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainBottomPanelWrapper)
};

} // namespace Nimbus::MainLayout
