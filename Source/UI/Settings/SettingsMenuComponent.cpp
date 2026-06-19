#include "SettingsMenuComponent.h"
#include "../DesignSystem/NimbusLookAndFeel.h"
#include "../DesignSystem/Iconography.h"
#include "../DesignSystem/Colors.h"
#include "../DesignSystem/Typography.h"

namespace Nimbus::UI::Settings {

SettingsMenuComponent::SettingsMenuComponent(NimbusEngine& engineToUse)
    : engine(engineToUse),
      audioSetupComp(engine.getAudioDeviceManager().getJuceAudioDeviceManager(),
                     0, 256, 0, 256, true, true, true, false)
{
    setSize(500, 400);
    addAndMakeVisible(audioSetupComp);
}

SettingsMenuComponent::~SettingsMenuComponent() {
}

void SettingsMenuComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
    
    // Header
    g.setColour(DesignSystem::Colors::ModuleBackground);
    g.fillRect(0, 0, getWidth(), 40);
    
    g.setColour(DesignSystem::Colors::TextPrimary);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(18.0f));
    g.drawText("Settings", 16, 0, getWidth() - 32, 40, juce::Justification::centredLeft);
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 40, getWidth(), 1);
}

void SettingsMenuComponent::resized() {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(41); // header
    
    audioSetupComp.setBounds(bounds.reduced(16));
}

} // namespace Nimbus::UI::Settings
