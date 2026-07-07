#pragma once

#include <juce_core/juce_core.h>

namespace Nimbus {

struct TrackID {
    juce::Uuid id;
    TrackID() = default;
    explicit TrackID(juce::Uuid uuid) : id(uuid) {}
    bool operator==(const TrackID& other) const { return id == other.id; }
    bool operator!=(const TrackID& other) const { return id != other.id; }
    bool operator<(const TrackID& other) const { return id.toString() < other.id.toString(); }
    bool isNull() const { return id.isNull(); }
    juce::String toString() const { return id.toString(); }
};

enum class TrackType {
    Audio,
    Midi,
    Instrument,
    Bus,
    Master
};

struct PluginIdentifier {
    juce::String type;
    juce::String manufacturer;
    juce::String name;
    juce::String uniqueID;
};

struct PluginSlot {
    juce::Uuid id;
    PluginIdentifier pluginID;
    bool hasPlugin = false;
    bool isEnabled = true;
    juce::MemoryBlock stateData;
};

enum class InputSourceType {
    AudioDevice,
    MidiDevice,
    VirtualMIDI,
    Sidechain,
    VRackSum,
    None
};

struct InputSource {
    InputSourceType type = InputSourceType::None;
    int channelIndex = -1;
    juce::String deviceID;
    TrackID sidechainTrackID;
};

// Represents time position in the arrangement
struct TimePosition {
    int64_t samples = 0;
    double sampleRate = 44100.0;
    
    double getSeconds() const { return static_cast<double>(samples) / sampleRate; }
    
    bool operator==(const TimePosition& other) const { return samples == other.samples && sampleRate == other.sampleRate; }
    bool operator<(const TimePosition& other) const { return getSeconds() < other.getSeconds(); }
    bool operator>(const TimePosition& other) const { return getSeconds() > other.getSeconds(); }
};

struct TimeRange {
    TimePosition start;
    TimePosition duration;
    
    TimePosition getEnd() const {
        return TimePosition{ start.samples + duration.samples, start.sampleRate };
    }
};

struct Tempo {
    double bpm = 120.0;
};

struct TimeSignature {
    int numerator = 4;
    int denominator = 4;
};

} // namespace Nimbus
