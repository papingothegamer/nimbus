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
    
    double clipSamples = currentClip->getLengthSamples();
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double clipSeconds = clipSamples / sampleRate;
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    double secondsPerBar = secondsPerBeat * 4.0;
    
    int numBars = static_cast<int>(clipSeconds / secondsPerBar) + 1;
    
    g.setColour(DesignSystem::Colors::TextSecondary);
    g.setFont(DesignSystem::Typography::getSecondaryFont().withHeight(10.0f));
    
    for (int i = 0; i < numBars; ++i) {
        float x = static_cast<float>((i * secondsPerBar / clipSeconds) * getWidth());
        
        g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
        g.drawText(juce::String(i + 1), static_cast<int>(x) + 2, 2, 30, 12, juce::Justification::topLeft, false);
        
        // Draw beat subdivisions
        for (int b = 1; b < 4; ++b) {
            float beatX = x + static_cast<float>((b * secondsPerBeat / clipSeconds) * getWidth());
            if (beatX < getWidth()) {
                g.drawVerticalLine(static_cast<int>(beatX), static_cast<float>(getHeight()) - 5.0f, static_cast<float>(getHeight()));
                g.drawText("1." + juce::String(b + 1), static_cast<int>(beatX) + 2, 2, 30, 12, juce::Justification::topLeft, false);
            }
        }
    }
}

} // namespace Nimbus::DetailView
