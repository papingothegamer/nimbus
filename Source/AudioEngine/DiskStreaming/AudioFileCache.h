#pragma once

#include <JuceHeader.h>

#include "DiskStreamManager.h"

namespace Nimbus {

class AudioFileCache {
public:
    AudioFileCache(juce::AudioFormatManager& formatManager);
    ~AudioFileCache() = default;

    std::unique_ptr<juce::AudioFormatReader> createReader(const juce::File& file);
    std::shared_ptr<DiskStream> createStream(const juce::File& file);
    
    juce::AudioFormatManager& getFormatManager() { return formatManager; }
    DiskStreamManager& getStreamManager() { return streamManager; }

private:
    juce::AudioFormatManager& formatManager;
    DiskStreamManager streamManager;
};

} // namespace Nimbus
