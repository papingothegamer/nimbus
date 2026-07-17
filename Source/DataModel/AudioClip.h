#pragma once

#include <juce_core/juce_core.h>

namespace Nimbus {

/**
 * Represents a region of audio placed on a track's timeline.
 */
class AudioClip {
public:
    enum class WarpMode {
        Beats,
        Tones,
        Complex,
        ComplexPro
    };

    AudioClip(const juce::File& file, int startSample, int lengthSamples, int sourceOffsetSamples = 0);
    ~AudioClip() = default;

    const juce::File& getSourceFile() const { return sourceFile; }
    
    // Timeline position (start time in samples relative to the project start)
    int getStartSample() const { return startSample; }
    void setStartSample(int sample) { startSample = sample; }

    // How many samples long this clip is on the timeline
    int getLengthSamples() const { return lengthSamples; }
    void setLengthSamples(int length) { lengthSamples = length; }

    // Offset into the source file (for when the user trims the start of the clip)
    int getSourceOffsetSamples() const { return sourceOffsetSamples; }
    void setSourceOffsetSamples(int offset) { sourceOffsetSamples = offset; }

    // Timeline End Sample
    int getEndSample() const { return startSample + lengthSamples; }

    // Extended properties
    const juce::String& getName() const { return name; }
    void setName(const juce::String& n) { name = n; }
    
    int getColorIndex() const { return colorIndex; }
    void setColorIndex(int c) { colorIndex = c; }
    
    int getNumChannels() const { return numChannels; }
    void setNumChannels(int c) { numChannels = c; }
    
    float getGain() const { return gain; }
    void setGain(float g) { gain = juce::jlimit(0.0f, 2.0f, g); }
    
    int getFadeInSamples() const { return fadeInSamples; }
    void setFadeInSamples(int s) { fadeInSamples = juce::jmax(0, s); }
    int getFadeOutSamples() const { return fadeOutSamples; }
    void setFadeOutSamples(int s) { fadeOutSamples = juce::jmax(0, s); }
    
    int getFadeInCurve() const { return fadeInCurve; }
    void setFadeInCurve(int c) { fadeInCurve = juce::jlimit(0, 3, c); }
    int getFadeOutCurve() const { return fadeOutCurve; }
    void setFadeOutCurve(int c) { fadeOutCurve = juce::jlimit(0, 3, c); }
    
    float getPitchShift() const { return pitchShift; }
    void setPitchShift(float p) { pitchShift = juce::jlimit(-24.0f, 24.0f, p); }
    
    float getTimeStretch() const { return timeStretch; }
    void setTimeStretch(float t) { timeStretch = juce::jlimit(0.1f, 10.0f, t); }
    
    bool isWarpEnabled() const { return warpEnabled; }
    void setWarpEnabled(bool w) { warpEnabled = w; }
    
    WarpMode getWarpMode() const { return warpMode; }
    void setWarpMode(WarpMode m) { warpMode = m; }
    
    double getOriginalBpm() const { return originalBpm; }
    void setOriginalBpm(double bpm) { originalBpm = juce::jlimit(20.0, 999.0, bpm); }
    
    bool getMatchDawTempo() const { return matchDawTempo; }
    void setMatchDawTempo(bool m) { matchDawTempo = m; }
    
    bool isFollowEnabled() const { return followEnabled; }
    void setFollowEnabled(bool f) { followEnabled = f; }
    
    bool isPreservePitch() const { return preservePitch; }
    void setPreservePitch(bool p) { preservePitch = p; }
    
    const std::vector<double>& getWarpMarkers() const { return warpMarkers; }
    void addWarpMarker(double samplePos) { warpMarkers.push_back(samplePos); }
    void setWarpMarker(size_t index, double samplePos) {
        if (index < warpMarkers.size()) warpMarkers[index] = samplePos;
    }
    void clearWarpMarkers() { warpMarkers.clear(); }

private:
    juce::File sourceFile;
    int startSample = 0;
    int lengthSamples = 0;
    int sourceOffsetSamples = 0;
    
    // Extended properties
    juce::String name{"Audio Clip"};
    float gain = 1.0f;            // Linear gain (0.0 - 2.0)
    int fadeInSamples = 0;
    int fadeOutSamples = 0;
    int fadeInCurve = 0;          // 0=Linear, 1=Log, 2=Exp, 3=S-Curve
    int fadeOutCurve = 0;
    float pitchShift = 0.0f;      // In semitones (-24 to +24)
    float timeStretch = 1.0f;     // 1.0 = original speed
    bool warpEnabled = true;
    WarpMode warpMode = WarpMode::Beats;
    bool matchDawTempo = true;
    bool followEnabled = true;
    double originalBpm = 120.0;
    bool preservePitch = true;
    
    // Warp markers (positions in source samples)
    std::vector<double> warpMarkers;
    
    int colorIndex = -1;
    int numChannels = 2;
};

} // namespace Nimbus
