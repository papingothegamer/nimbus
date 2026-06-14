#pragma once

#include <JuceHeader.h>

namespace Nimbus {

class MainWindow : public juce::DocumentWindow {
public:
    explicit MainWindow(juce::String name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace Nimbus
