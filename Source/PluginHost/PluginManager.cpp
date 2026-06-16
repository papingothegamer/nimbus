#include "PluginManager.h"

namespace Nimbus {

PluginManager::PluginManager() {
    formatManager.addDefaultFormats();
    
    // Load cached plugins
    juce::File appData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Nimbus").getChildFile("Plugins.xml");
    if (appData.existsAsFile()) {
        if (auto xml = juce::XmlDocument::parse(appData)) {
            knownPluginList.recreateFromXml(*xml);
        }
    }
}

PluginManager::~PluginManager() {
    stopScanning();
}

PluginManager::ScannerThread::ScannerThread(PluginManager& owner) 
    : juce::Thread("PluginScannerThread"), owner(owner) {
}

PluginManager::ScannerThread::~ScannerThread() {
    stopThread(2000);
}

void PluginManager::ScannerThread::run() {
#if JUCE_WINDOWS
    juce::ScopedJuceInitialiser_GUI guiInit; // Many Windows VSTs require GUI thread environment
#endif

    for (int i = 0; i < owner.formatManager.getNumFormats(); ++i) {
        if (threadShouldExit()) break;
        
        auto* format = owner.formatManager.getFormat(i);
        if (format) {
            juce::PluginDirectoryScanner scanner(
                owner.knownPluginList, 
                *format,
                format->getDefaultLocationsToSearch(),
                true,
                juce::File(),
                true
            );
            
            juce::String name;
            while (scanner.scanNextFile(true, name)) {
                if (threadShouldExit()) break;
            }
        }
    }
    
    // Save cached plugins
    if (!threadShouldExit()) {
        if (auto xml = owner.knownPluginList.createXml()) {
            juce::File appData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Nimbus");
            appData.createDirectory();
            xml->writeTo(appData.getChildFile("Plugins.xml"));
        }
    }
}

void PluginManager::startScanning() {
    if (scannerThread != nullptr && scannerThread->isThreadRunning()) return;
    scannerThread = std::make_unique<ScannerThread>(*this);
    scannerThread->startThread();
}

void PluginManager::stopScanning() {
    if (scannerThread != nullptr) {
        scannerThread->stopThread(2000);
        scannerThread.reset();
    }
}

bool PluginManager::isScanning() const {
    return scannerThread != nullptr && scannerThread->isThreadRunning();
}

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
