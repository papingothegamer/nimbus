#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::DetailView {

class NotesPanelComponent : public juce::Component {
public:
    NotesPanelComponent(NimbusEngine& engine);
    ~NotesPanelComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);

private:
    NimbusEngine& engine;
    std::shared_ptr<MidiClip> currentClip;
    
    juce::Label titleLabel{"", "Notes"};
    juce::TextButton reverseButton{"Reverse"};
    juce::TextButton invertButton{"Invert"};
    juce::TextButton legatoButton{"Legato"};
    juce::TextButton duplicateButton{"Duplicate"};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotesPanelComponent)
};

} // namespace Nimbus::DetailView
