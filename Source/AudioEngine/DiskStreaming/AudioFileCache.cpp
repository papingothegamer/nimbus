#include "AudioFileCache.h"

namespace Nimbus {

AudioFileCache::AudioFileCache(juce::AudioFormatManager& fmtMgr)
    : formatManager(fmtMgr)
{
}

std::unique_ptr<juce::AudioFormatReader> AudioFileCache::createReader(const juce::File& file) {
    return std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
}

std::shared_ptr<DiskStream> AudioFileCache::createStream(const juce::File& file) {
    auto stream = std::make_shared<DiskStream>(file, formatManager);
    streamManager.addStream(stream);
    return stream;
}

} // namespace Nimbus
