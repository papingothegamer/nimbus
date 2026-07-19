#include "PianoRollTimelineComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

PianoRollTimelineComponent::PianoRollTimelineComponent(NimbusEngine& e) : engine(e) {}
PianoRollTimelineComponent::~PianoRollTimelineComponent() = default;

void PianoRollTimelineComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentClip = clip;
    repaint();
}

void PianoRollTimelineComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    
    if (!currentClip) return;
    
    double clipSamples = currentClip->lengthSamples.get();
    double clipStartSamples = currentClip->startSample.get();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double clipSeconds = clipSamples / sampleRate;
    double clipStartSeconds = clipStartSamples / sampleRate;
    
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    double secondsPerBar = secondsPerBeat * 4.0;
    
    int firstBar = static_cast<int>(clipStartSeconds / secondsPerBar);
    int lastBar = static_cast<int>((clipStartSeconds + clipSeconds) / secondsPerBar);
    
    int keyWidth = 60; // Must match PianoRollComponent::keyWidth
    int gridWidth = getWidth() - keyWidth;
    
    g.setColour(DesignSystem::Colors::TextSecondary);
    g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    
    for (int i = firstBar; i <= lastBar + 1; ++i) {
        double barGlobalSeconds = i * secondsPerBar;
        double barLocalSeconds = barGlobalSeconds - clipStartSeconds;
        
        if (barLocalSeconds >= 0 && barLocalSeconds <= clipSeconds) {
            float x = keyWidth + static_cast<float>((barLocalSeconds / clipSeconds) * gridWidth);
            
            g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
            g.drawText(juce::String(i + 1), static_cast<int>(x) + 2, 2, 30, 12, juce::Justification::topLeft, false);
        }
        
        // Draw beat subdivisions
        for (int b = 1; b < 4; ++b) {
            double beatLocalSeconds = barLocalSeconds + (b * secondsPerBeat);
            if (beatLocalSeconds >= 0 && beatLocalSeconds <= clipSeconds) {
                float beatX = keyWidth + static_cast<float>((beatLocalSeconds / clipSeconds) * gridWidth);
                g.drawVerticalLine(static_cast<int>(beatX), static_cast<float>(getHeight()) - 5.0f, static_cast<float>(getHeight()));
                g.drawText(juce::String(i + 1) + "." + juce::String(b + 1), static_cast<int>(beatX) + 2, 2, 30, 12, juce::Justification::topLeft, false);
            }
        }
    }
    
    // Draw playhead
    if (engine.getTransport().isPlaying()) {
        double positionSamples = engine.getTransport().getCurrentPosition();
        
        double clipGlobalStart = currentClip->startSample.get();
        double clipGlobalEnd = clipGlobalStart + currentClip->lengthSamples.get();
        
        if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
            double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
            double pixelsPerSecond = 100.0;
            float px = keyWidth + static_cast<float>(timeIntoClip * pixelsPerSecond);
            
            g.setColour(DesignSystem::Colors::PrimaryAction);
            g.drawVerticalLine(static_cast<int>(px), 0.0f, static_cast<float>(getHeight()));
            
            juce::Path p;
            p.addTriangle(px - 5.0f, 0.0f, px + 5.0f, 0.0f, px, 8.0f);
            g.fillPath(p);
        }
    }
}

} // namespace Nimbus::DetailView
