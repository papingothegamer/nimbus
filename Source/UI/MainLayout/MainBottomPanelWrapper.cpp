#include "MainBottomPanelWrapper.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

MainBottomPanelWrapper::MainBottomPanelWrapper(NimbusEngine& engine)
    : mixer(engine), detail(engine) {
    for (auto* tab : { &mixerTab, &deviceTab }) {
        tab->setClickingTogglesState(true);
        tab->setRadioGroupId(0x4e425057);
        tab->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        tab->setColour(juce::TextButton::buttonOnColourId, DesignSystem::Colors::ComponentBackground);
        tab->setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary);
        tab->setColour(juce::TextButton::textColourOnId, DesignSystem::Colors::TextPrimary);
        addAndMakeVisible(*tab);
    }
    mixerTab.setToggleState(true, juce::dontSendNotification);
    mixerTab.onClick = [this] { showMixerView(); };
    deviceTab.onClick = [this] { showDeviceView(); };

    addAndMakeVisible(mixer);
    addChildComponent(detail);

    // The device pane claims half the available lower workspace when enabled.
    splitLayout.setItemLayout(0, -0.75, -0.25, -0.50);
    splitLayout.setItemLayout(1, -0.75, -0.25, -0.50);
}

void MainBottomPanelWrapper::showDeviceView() {
    deviceViewActive = true;
    deviceTab.setToggleState(true, juce::dontSendNotification);
    detail.setVisible(true);
    resized();
}

void MainBottomPanelWrapper::showMixerView() {
    deviceViewActive = false;
    mixerTab.setToggleState(true, juce::dontSendNotification);
    detail.setVisible(false);
    resized();
}

void MainBottomPanelWrapper::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 29, getWidth(), 1);
}

void MainBottomPanelWrapper::resized() {
    auto area = getLocalBounds();
    auto tabArea = area.removeFromTop(30).reduced(6, 3);
    mixerTab.setBounds(tabArea.removeFromLeft(72));
    deviceTab.setBounds(tabArea.removeFromLeft(148));

    if (!deviceViewActive) {
        mixer.setBounds(area);
        detail.setBounds({});
        return;
    }

    juce::Component* panes[] { &detail, &mixer };
    int widths[] { area.getWidth() / 2, area.getWidth() - area.getWidth() / 2 };
    splitLayout.layOutComponents(panes, 2, 0, area.getY(), area.getWidth(), area.getHeight(), false, true);
    // StretchableLayoutManager uses the current component sizes as an input;
    // establish the exact initial 50/50 geometry requested by the workspace.
    detail.setBounds(area.withWidth(widths[0]));
    mixer.setBounds(area.withTrimmedLeft(widths[0]));
}

} // namespace Nimbus::MainLayout
