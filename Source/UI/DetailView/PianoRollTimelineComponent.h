#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::DetailView {

class PianoRollTimelineComponent : public juce::Component {
public:
    PianoRollTimelineComponent(NimbusEngine& engine);
    ~PianoRollTimelineComponent() override;

    void paint(juce::Graphics& g) override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);

private:
    NimbusEngine& engine;
    std::shared_ptr<MidiClip> currentClip;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollTimelineComponent)
};

} // namespace Nimbus::DetailView
