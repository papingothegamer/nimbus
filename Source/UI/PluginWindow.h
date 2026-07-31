#pragma once

#include <JuceHeader.h>
#include "Core/Plugins/Plugin.h"

namespace Nimbus {

class PluginWindow : public juce::DocumentWindow {
public:
    PluginWindow(const juce::String& name, std::shared_ptr<Plugin> p);
    ~PluginWindow() override;

    void closeButtonPressed() override;

private:
    std::shared_ptr<Plugin> plugin;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};

} // namespace Nimbus
