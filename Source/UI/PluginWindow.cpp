#include "PluginWindow.h"

namespace Nimbus {

PluginWindow::PluginWindow(const juce::String& name, PluginNode* node)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                 .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons),
      pluginNode(node)
{
    setUsingNativeTitleBar(true);

    if (pluginNode && pluginNode->getPluginInstance()) {
        if (auto* editor = pluginNode->getPluginInstance()->createEditorIfNeeded()) {
            setContentOwned(editor, true);
        }
    }

    setResizable(true, true);
    setTopLeftPosition(100, 100);
    setVisible(true);
}

PluginWindow::~PluginWindow() {
    // If the window is closed/destroyed, we don't destroy the node, just the UI.
    if (pluginNode && pluginNode->getPluginInstance()) {
        pluginNode->getPluginInstance()->editorBeingDeleted(
            dynamic_cast<juce::AudioProcessorEditor*>(getContentComponent())
        );
        clearContentComponent();
    }
}

void PluginWindow::closeButtonPressed() {
    delete this; // Normally managed by a desktop manager or unique_ptr, but fine for testing.
}

} // namespace Nimbus
