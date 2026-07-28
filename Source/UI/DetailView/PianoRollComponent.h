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
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseMagnify(const juce::MouseEvent& event, float scaleFactor) override;
    bool keyPressed(const juce::KeyPress& key) override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);
    std::shared_ptr<MidiClip> getCurrentClip() const { return currentClip; }
    
    enum class Snap { Off, Bar, Beat, Eighth, Sixteenth, ThirtySecond };
    void setSnap(Snap s) { currentSnap = s; }
    
    void setVelocityVisible(bool v) { velocityVisible = v; repaint(); }
    int getDesiredHeight() const { return (128 * keyHeight); }
    int getDesiredWidth() const;

private:
    NimbusEngine& engine;
    std::shared_ptr<MidiClip> currentClip;
    int keyWidth = 60;
    int keyHeight = 16;
    int totalKeys = 128;
    int velocityLaneHeight = 80;
    
    double timeZoom = 1.0;
    
    Snap currentSnap = Snap::Sixteenth;
    bool velocityVisible = false;
    
    int draggedEventIndex = -1;
    bool isResizing = false;
    bool isDraggingVelocity = false;
    bool hasDuplicatedForDrag = false;
    double dragStartNoteTime = 0.0;
    double dragStartNoteLength = 0.0;
    int dragStartMouseX = 0;
    int dragStartMouseY = 0;
    int dragStartNoteNumber = 0;
    
    juce::Array<int> selectedEventIndices;
    
    bool isMarqueeSelecting = false;
    juce::Rectangle<float> marqueeRect;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollContent)
};

class PianoRollComponent : public juce::Component {
public:
    PianoRollComponent(NimbusEngine& engine);
    ~PianoRollComponent() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);
    
    std::shared_ptr<MidiClip> getCurrentClip() const { return content.getCurrentClip(); }

private:
    NimbusEngine& engine;
    
    // Toolbar
    juce::Component toolbar;
    juce::ComboBox snapBox;
    juce::ToggleButton velocityToggle{"Velocity"};
    

    
    juce::Viewport viewport;
    PianoRollContent content;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};

} // namespace DetailView
} // namespace Nimbus
