#pragma once
#include <JuceHeader.h>
#include "../../Core/NimbusEngine.h"

namespace Nimbus::UI::Settings {

class SettingsMenuComponent : public juce::Component {
public:
    SettingsMenuComponent(NimbusEngine& engine);
    ~SettingsMenuComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NimbusEngine& engine;
    juce::AudioDeviceSelectorComponent audioSetupComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsMenuComponent)
};

} // namespace Nimbus::UI::Settings
