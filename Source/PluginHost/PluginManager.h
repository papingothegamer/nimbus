#pragma once

#include <JuceHeader.h>
#include <memory>

namespace Nimbus {

/**
 * Manages plugin formats and instantiates audio plugin instances.
 */
class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    // Returns a new plugin instance from a given file path.
    // If it fails, returns nullptr and sets errorMessage.
    std::unique_ptr<juce::AudioPluginInstance> loadPlugin(const juce::String& pluginPath, juce::String& errorMessage);

    juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }
    juce::KnownPluginList& getKnownPluginList() { return knownPluginList; }

    void startScanning();
    void stopScanning();
    bool isScanning() const;

private:
    class ScannerThread : public juce::Thread {
    public:
        ScannerThread(PluginManager& owner);
        ~ScannerThread() override;
        void run() override;
    private:
        PluginManager& owner;
        std::unique_ptr<juce::PluginDirectoryScanner> scanner;
    };

    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList knownPluginList;
    std::unique_ptr<ScannerThread> scannerThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager)
};

} // namespace Nimbus
