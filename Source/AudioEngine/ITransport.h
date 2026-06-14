#pragma once

namespace Nimbus {

/**
 * Abstracts the playback state.
 */
class ITransport {
public:
    virtual ~ITransport() = default;

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void record() = 0;
    
    virtual void setPosition(double samplePosition) = 0;
    virtual double getCurrentPosition() const = 0;
    
    virtual bool isPlaying() const = 0;
    virtual bool isRecording() const = 0;

    virtual double getSampleRate() const = 0;
    virtual double getTempo() const = 0;
};

} // namespace Nimbus
