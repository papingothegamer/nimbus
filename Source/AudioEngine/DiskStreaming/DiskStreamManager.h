#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <mutex>
#include "DiskStream.h"

namespace Nimbus {

class DiskStreamManager : public juce::Thread {
public:
    DiskStreamManager();
    ~DiskStreamManager() override;

    void addStream(std::shared_ptr<DiskStream> stream);
    void removeStream(std::shared_ptr<DiskStream> stream);

    void run() override;

private:
    std::vector<std::weak_ptr<DiskStream>> activeStreams;
    juce::CriticalSection streamsLock;
};

} // namespace Nimbus
