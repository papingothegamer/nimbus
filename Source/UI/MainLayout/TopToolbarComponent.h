#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Iconography.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::MainLayout {

class TopToolbarComponent : public juce::Component, public juce::Timer {
public:
    TopToolbarComponent(NimbusEngine& engine);
    ~TopToolbarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    std::function<void()> onBrowserToggle;
    std::function<void()> onDetailToggle;
    std::function<void()> onZoomIn;
    std::function<void()> onZoomOut;

private:
    NimbusEngine& engine;
    int currentZoom = 100;

    // --- Left Section ---
    juce::TextButton undoButton{DesignSystem::Iconography::Undo};
    juce::TextButton redoButton{DesignSystem::Iconography::Redo};
    juce::Label projectNameLabel;

    // --- Center-Left Section (Zoom) ---
    juce::TextButton zoomOutButton{DesignSystem::Iconography::ZoomOut};
    juce::Label zoomLevelLabel;
    juce::TextButton zoomInButton{DesignSystem::Iconography::ZoomIn};

    // --- Center Section (Transport) ---
    juce::TextButton jumpStartButton{DesignSystem::Iconography::JumpStart};
    juce::TextButton rewindButton{DesignSystem::Iconography::Rewind};
    juce::TextButton playButton{DesignSystem::Iconography::Play};
    juce::TextButton recordButton{DesignSystem::Iconography::RecordGlobal};
    juce::TextButton fastForwardButton{DesignSystem::Iconography::FastForward};

    // --- Time Displays ---
    class DisplayBox : public juce::Component {
    public:
        DisplayBox(const juce::String& h, const juce::String& val, juce::Colour valColor) 
            : header(h) {
            valueLabel.setText(val, juce::dontSendNotification);
            valueLabel.setColour(juce::Label::textColourId, valColor);
            valueLabel.setColour(juce::Label::textWhenEditingColourId, valColor);
            valueLabel.setColour(juce::Label::backgroundWhenEditingColourId, DesignSystem::Colors::ModuleBackground.brighter(0.1f));
            valueLabel.setJustificationType(juce::Justification::centred);
            valueLabel.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(16.0f).boldened());
            addAndMakeVisible(valueLabel);
        }
            
        void paint(juce::Graphics& g) override;
        void resized() override {
            auto bounds = getLocalBounds();
            valueLabel.setBounds(bounds.withTrimmedTop(14));
        }
        
        void setValue(const juce::String& newVal) { 
            if (!valueLabel.isBeingEdited()) {
                valueLabel.setText(newVal, juce::dontSendNotification); 
            }
        }
        
        juce::Label valueLabel;
        
    private:
        juce::String header;
    };
    
    DisplayBox barsDisplay{"BARS", "1.1.00", DesignSystem::Colors::TextPrimary};
    DisplayBox timeDisplay{"TIME", "00:00:00", juce::Colour(0xff00ff00)}; // Neon Green
    DisplayBox bpmDisplay{"BPM", "120.0", juce::Colour(0xffff9900)}; // Orange
    DisplayBox sigDisplay{"SIG.", "4/4", DesignSystem::Colors::TextSecondary};

    // --- Toggles ---
    juce::TextButton loopButton{DesignSystem::Iconography::Loop};
    juce::TextButton metronomeToggle{DesignSystem::Iconography::Metronome};
    
    // --- Right Section ---
    juce::TextButton followButton{DesignSystem::Iconography::Follow};
    juce::TextButton pianoRollToggle{DesignSystem::Iconography::PianoOff};
    juce::TextButton mixerToggle{DesignSystem::Iconography::Tune};
    juce::TextButton settingsButton{DesignSystem::Iconography::Settings};
    juce::TextButton saveProjectButton{DesignSystem::Iconography::Save};

    // --- Far Right ---
    juce::Label cpuLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopToolbarComponent)
};

} // namespace Nimbus::MainLayout
