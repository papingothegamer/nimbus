#pragma once

#include <JuceHeader.h>
#include "AudioEngine/PluginNode.h"

namespace Nimbus {

class PluginWindow : public juce::DocumentWindow {
public:
    PluginWindow(const juce::String& name, PluginNode* node);
    ~PluginWindow() override;

    void closeButtonPressed() override;

private:
    PluginNode* pluginNode = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};

} // namespace Nimbus
