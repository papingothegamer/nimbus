#pragma once
#include <JuceHeader.h>

namespace Nimbus::UI {

class MidiActivityMeter : public juce::Component {
public:
    MidiActivityMeter();
    ~MidiActivityMeter() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void setLevel(float level);

private:
    float currentLevel = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiActivityMeter)
};

} // namespace Nimbus::UI
