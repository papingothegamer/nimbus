#include "PluginWindow.h"

namespace Nimbus {

PluginWindow::PluginWindow(const juce::String& name, std::shared_ptr<Plugin> p)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons),
      plugin(p)
{
    setUsingNativeTitleBar(true);

    if (plugin) {
        if (auto* editor = plugin->createEditor()) {
            setContentOwned(editor, true);
        }
    }

    setResizable(true, true);
    setTopLeftPosition(100, 100);
    setVisible(true);
}

PluginWindow::~PluginWindow() {
    clearContentComponent();
}

void PluginWindow::closeButtonPressed() {
    delete this; // Normally managed by a desktop manager or unique_ptr, but fine for testing.
}

} // namespace Nimbus
