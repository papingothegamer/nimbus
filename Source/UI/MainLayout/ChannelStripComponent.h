#pragma once

#include <JuceHeader.h>

#include "Core/NimbusEngine.h"
#include "UI/Mixer/GroupIndicatorComponent.h"

namespace Nimbus::MainLayout {

class ChannelStripComponent : public juce::Component, private juce::Timer, public TimelineProject::Listener {
public:
    ChannelStripComponent(NimbusEngine& engine, const juce::String& name, bool isStereo, bool isMaster);
    ~ChannelStripComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setTrackIndex(int newIndex);
    void timerCallback() override;

    void setLevelProvider(std::function<float()> provider);

    // Callbacks for interactivity
    std::function<void()> onSelected;

private:
    NimbusEngine& engine;
    juce::String channelName;
    bool stereo;
    bool master;
    bool selected = false;

    juce::Label nameLabel;
    
    juce::ComboBox inputComboBox;
    juce::ComboBox routingComboBox;
    juce::Slider pan;
    juce::Label volumeLabel;
    juce::Slider fader;

    juce::TextButton muteButton{"M"};
    juce::TextButton soloButton{"S"};
    juce::TextButton armButton{"O"};
    juce::TextButton stereoButton{"O"}; // Mock stereo link icon
    juce::TextButton linkIcon{"Link"}; // Mock stereo link icon

    int trackIndex = -1;

    // Helper for rendering meter
    void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level);

    std::function<float()> levelProvider;
    void mouseDown(const juce::MouseEvent& event) override;

    // TimelineProject::Listener
    void trackMuteChanged(int track, bool isMuted) override;
    void trackArmChanged(int track, bool isArmed) override;
    void trackStereoChanged(int track, bool isStereo) override;
    void trackSelectionChanged() override;
    
    float currentLevel = 0.0f;
    
    UI::GroupIndicatorComponent groupIndicator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripComponent)
};

} // namespace Nimbus::MainLayout
