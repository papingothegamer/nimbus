#include "DiskStreamManager.h"

namespace Nimbus {

DiskStreamManager::DiskStreamManager()
    : juce::Thread("DiskStreamManager")
{
    startThread(juce::Thread::Priority::normal);
}

DiskStreamManager::~DiskStreamManager() {
    stopThread(4000);
}

void DiskStreamManager::addStream(std::shared_ptr<DiskStream> stream) {
    if (!stream) return;
    const juce::ScopedLock sl(streamsLock);
    activeStreams.push_back(stream);
    notify(); // Wake up thread to start processing the new stream
}

void DiskStreamManager::removeStream(std::shared_ptr<DiskStream> stream) {
    const juce::ScopedLock sl(streamsLock);
    activeStreams.erase(
        std::remove_if(activeStreams.begin(), activeStreams.end(),
            [&](const std::weak_ptr<DiskStream>& wp) {
                auto sp = wp.lock();
                return !sp || sp == stream;
            }),
        activeStreams.end());
}

void DiskStreamManager::run() {
    while (!threadShouldExit()) {
        bool processedAny = false;

        {
            const juce::ScopedLock sl(streamsLock);
            
            // Clean up dead streams
            activeStreams.erase(
                std::remove_if(activeStreams.begin(), activeStreams.end(),
                    [](const std::weak_ptr<DiskStream>& wp) { return wp.expired(); }),
                activeStreams.end());

            // Process active streams
            for (auto& wp : activeStreams) {
                if (auto stream = wp.lock()) {
                    stream->processStream();
                    processedAny = true;
                }
            }
        }

        // Wait a bit before checking again to avoid burning CPU
        wait(10);
    }
}

} // namespace Nimbus
