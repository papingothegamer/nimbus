#pragma once

#include <JuceHeader.h>
#include "DataModel/MidiClip.h"

namespace Nimbus {
class NimbusEngine;

namespace DetailView {

class PianoRollContent : public juce::Component {
public:
    PianoRollContent(NimbusEngine& engine);
    ~PianoRollContent() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);

private:
    NimbusEngine& engine;
    std::shared_ptr<MidiClip> currentClip;
    int keyWidth = 60;
    int keyHeight = 16;
    int totalKeys = 128;
    
    int draggedEventIndex = -1;
    bool isResizing = false;
    double dragStartNoteTime = 0.0;
    double dragStartNoteLength = 0.0;
    int dragStartMouseX = 0;
    int dragStartMouseY = 0;
    int dragStartNoteNumber = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollContent)
};


class PianoRollComponent : public juce::Component {
public:
    PianoRollComponent(NimbusEngine& engine);
    ~PianoRollComponent() override;

    void resized() override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);

private:
    juce::Viewport viewport;
    PianoRollContent content;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};

} // namespace DetailView
} // namespace Nimbus
