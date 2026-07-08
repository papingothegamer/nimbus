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
    
    void setMidiClip(std::shared_ptr<MidiClip> clip);
    
    enum class Tool { Pointer, Pencil, Eraser };
    void setTool(Tool t) { currentTool = t; }
    
    enum class Snap { Off, Bar, Beat, Eighth, Sixteenth, ThirtySecond };
    void setSnap(Snap s) { currentSnap = s; }
    
    void setVelocityVisible(bool v) { velocityVisible = v; resized(); repaint(); }

private:
    NimbusEngine& engine;
    std::shared_ptr<MidiClip> currentClip;
    int keyWidth = 60;
    int keyHeight = 16;
    int totalKeys = 128;
    int velocityLaneHeight = 80;
    
    Tool currentTool = Tool::Pointer;
    Snap currentSnap = Snap::Sixteenth;
    bool velocityVisible = false;
    
    int draggedEventIndex = -1;
    bool isResizing = false;
    bool isDraggingVelocity = false;
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

private:
    NimbusEngine& engine;
    
    // Toolbar
    juce::Component toolbar;
    juce::DrawableButton pointerButton{"Pointer", juce::DrawableButton::ImageFitted};
    juce::DrawableButton pencilButton{"Pencil", juce::DrawableButton::ImageFitted};
    juce::DrawableButton eraserButton{"Eraser", juce::DrawableButton::ImageFitted};
    juce::ComboBox snapBox;
    juce::ToggleButton velocityToggle{"Velocity"};
    
    std::unique_ptr<juce::Drawable> pointerIcon;
    std::unique_ptr<juce::Drawable> pencilIcon;
    std::unique_ptr<juce::Drawable> eraserIcon;
    
    juce::Viewport viewport;
    PianoRollContent content;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};

} // namespace DetailView
} // namespace Nimbus
