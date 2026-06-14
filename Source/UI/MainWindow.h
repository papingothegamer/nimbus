#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus {

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(juce::String name, NimbusEngine& engine);
    ~MainWindow() override;

    void closeButtonPressed() override;

    void resized() override;

private:
    NimbusEngine& engine;
    juce::Slider volumeSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace Nimbus
