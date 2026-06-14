#include "PluginManager.h"

namespace Nimbus {

PluginManager::PluginManager() {
    formatManager.addDefaultFormats();
}

PluginManager::~PluginManager() = default;

std::unique_ptr<juce::AudioPluginInstance> PluginManager::loadPlugin(const juce::String& pluginPath, juce::String& errorMessage) {
    juce::File pluginFile(pluginPath);
    if (!pluginFile.existsAsFile() && !pluginFile.isDirectory()) {
        errorMessage = "Plugin file not found.";
        return nullptr;
    }

    for (int i = 0; i < formatManager.getNumFormats(); ++i) {
        if (auto* format = formatManager.getFormat(i)) {
            if (format->fileMightContainThisPluginType(pluginPath)) {
                juce::OwnedArray<juce::PluginDescription> types;
                format->findAllTypesForFile(types, pluginPath);
                
                if (types.size() > 0) {
                    std::unique_ptr<juce::AudioPluginInstance> instance = formatManager.createPluginInstance(
                        *types[0],
                        44100.0,
                        512,
                        errorMessage
                    );
                    if (instance != nullptr) {
                        return instance;
                    }
                }
            }
        }
    }

    if (errorMessage.isEmpty()) {
        errorMessage = "Failed to find a valid plugin description in the file.";
    }
    return nullptr;
}

} // namespace Nimbus
