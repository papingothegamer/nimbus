#pragma once

#include "AudioEngine/AudioGraph.h"
#include "AudioEngine/AudioDeviceManagerWrapper.h"
#include "AudioEngine/Transport.h"
#include "AudioEngine/Mixer.h"
#include <memory>

namespace Nimbus {

/**
 * The root service container for the Nimbus DAW.
 * Owns the core audio engine components and manages their lifecycles.
 */
class NimbusEngine {
public:
    NimbusEngine();
    ~NimbusEngine();

    void initialise();

    Mixer* getMixer() const { return mixer; }
    Transport& getTransport() { return transport; }
    AudioDeviceManagerWrapper& getAudioDeviceManager() { return deviceManagerWrapper; }

private:
    AudioGraph mainGraph; // The root graph executing on the audio thread
    Mixer* mixer = nullptr; // Raw pointer to the mixer owned by mainGraph
    AudioDeviceManagerWrapper deviceManagerWrapper;
    Transport transport;
};

} // namespace Nimbus
