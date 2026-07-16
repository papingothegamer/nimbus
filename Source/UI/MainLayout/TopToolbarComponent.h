#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Iconography.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h" // Added missing include

namespace Nimbus::MainLayout {

class TopToolbarComponent : public juce::Component, public juce::Timer {
public:
    TopToolbarComponent(NimbusEngine& engine);
    ~TopToolbarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    std::function<void()> onBrowserToggle;
    std::function<void()> onBottomPanelToggle;
    std::function<void()> onDetailToggle;
    std::function<void()> onZoomIn;
    std::function<void()> onZoomOut;

private:
    NimbusEngine& engine;
    int currentZoom = 100;

    juce::TextButton audioSetupButton { "Audio Setup" };
    juce::TextButton shareButton { "Share" };
    juce::Label workspaceLabel;

    juce::Component transportGroupContainer;
    juce::Component toolsGroupContainer;
    juce::Component actionGroupContainer;

    juce::DrawableButton undoButton{"Undo", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton redoButton{"Redo", juce::DrawableButton::ImageOnButtonBackground};
    juce::Label projectNameLabel;

    juce::DrawableButton zoomOutButton{"ZoomOut", juce::DrawableButton::ImageOnButtonBackground};
    juce::Label zoomLevelLabel;
    juce::DrawableButton zoomInButton{"ZoomIn", juce::DrawableButton::ImageOnButtonBackground};

    juce::DrawableButton pauseButton{"Pause", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton playButton{"Play", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton stopButton{"Stop", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton jumpStartButton{"JumpStart", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton jumpEndButton{"JumpEnd", juce::DrawableButton::ImageOnButtonBackground}; 
    juce::DrawableButton recordButton{"Record", juce::DrawableButton::ImageOnButtonBackground};

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
    DisplayBox timeDisplay{"TIME", "00:00:00", juce::Colour(0xff00ff00)}; 
    DisplayBox bpmDisplay{"BPM", "120.0", juce::Colour(0xffff9900)}; 
    DisplayBox sigDisplay{"SIG.", "4/4", DesignSystem::Colors::TextSecondary};

    juce::DrawableButton loopButton{"Loop", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton metronomeToggle{"Metronome", juce::DrawableButton::ImageOnButtonBackground};
    
    juce::DrawableButton followButton{"Follow", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton pianoRollToggle{"Piano", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton mixerToggle{"Mixer", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton settingsButton{"Settings", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton saveProjectButton{"Save", juce::DrawableButton::ImageOnButtonBackground};

    juce::Label cpuLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopToolbarComponent)
};

} // namespace Nimbus::MainLayout