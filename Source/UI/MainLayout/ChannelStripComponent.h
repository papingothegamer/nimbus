#pragma once

#include <JuceHeader.h>

#include "Core/NimbusEngine.h"
#include "UI/Mixer/GroupIndicatorComponent.h"
#include "UI/Mixer/MeteredFader.h"

namespace Nimbus::MainLayout {

class ChannelStripComponent : public juce::Component, public TimelineProject::Listener, public juce::Label::Listener {
public:
    ChannelStripComponent(NimbusEngine& engine, const juce::String& name, bool isStereo, bool isMaster);
    ~ChannelStripComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setTrackIndex(int newIndex);
    void updateMeters();

    void setLevelProvider(std::function<float()> provider);

    // Callbacks for interactivity
    std::function<void()> onSelected;

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

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
    UI::Mixer::MeteredFader fader;

    juce::DrawableButton muteButton{"Mute", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton soloButton{"Solo", juce::DrawableButton::ImageOnButtonBackground};
    juce::DrawableButton armButton{"Arm", juce::DrawableButton::ImageOnButtonBackground};

    int trackIndex = -1;

    std::function<float()> levelProvider;
    void mouseDown(const juce::MouseEvent& event) override;

    // TimelineProject::Listener
    void trackMuteChanged(int track, bool isMuted) override;
    void trackArmChanged(int track, bool isArmed) override;
    void trackSelectionChanged() override;
    void trackSoloChanged(int track, bool isSoloed) override;
    void trackVolumeChanged(int track, float volume) override;
    void trackPanChanged(int track, float pan) override;
    void trackNameChanged(int track, const juce::String& newName) override;
    void trackInputChannelChanged(int track, int inputChannel) override;
    
    float currentLevel = 0.0f;
    
    UI::GroupIndicatorComponent groupIndicator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripComponent)
};

} // namespace Nimbus::MainLayout
