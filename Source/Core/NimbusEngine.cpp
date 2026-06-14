#include "NimbusEngine.h"
#include "AudioEngine/TestToneNode.h"

namespace Nimbus {

NimbusEngine::NimbusEngine()
    : deviceManagerWrapper(mainGraph)
{
}

NimbusEngine::~NimbusEngine() = default;

void NimbusEngine::initialise() {
    // 1. Create the Mixer and hold a raw pointer for UI access
    auto mixerPtr = std::make_unique<Mixer>();
    mixer = mixerPtr.get();
    mainGraph.addNode(std::move(mixerPtr));

    // 2. Create a track with a TestToneNode as its source
    auto track = std::make_unique<Track>();
    track->setSourceNode(std::make_unique<TestToneNode>());
    
    // 3. Add the track to the mixer
    mixer->addTrack(std::move(track));

    // 4. Initialize the audio device manager, which will start pulling audio from the graph
    deviceManagerWrapper.initialise();
}

} // namespace Nimbus
