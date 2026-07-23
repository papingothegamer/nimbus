#include "MidiActivityMeter.h"

namespace Nimbus::UI {

MidiActivityMeter::MidiActivityMeter() {}
MidiActivityMeter::~MidiActivityMeter() {}

void MidiActivityMeter::setLevel(float level) {
    if (std::abs(level - currentLevel) > 0.01f) {
        currentLevel = level;
        repaint();
    }
}

void MidiActivityMeter::resized() {}

void MidiActivityMeter::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float meterWidth = bounds.getWidth() - 12.0f; 
    juce::Rectangle<float> meterBounds(0, 0, meterWidth, bounds.getHeight());

    // 1. Draw Meter Background
    g.setColour(juce::Colour(0xff111111));
    g.fillRect(meterBounds);

    if (currentLevel <= 0.0f) return;

    // 2. Draw Active Meter (Midi squares)
    g.setColour(juce::Colours::white);
    int numDots = 15;
    float dotSpacing = bounds.getHeight() / numDots;
    int activeDots = juce::roundToInt(currentLevel * numDots);
    
    for (int i = 0; i < activeDots; ++i) {
        float yPos = bounds.getHeight() - (i * dotSpacing) - dotSpacing;
        g.fillRect(meterWidth * 0.5f - 2.5f, yPos + 1.0f, 5.0f, 5.0f);
    }
}

} // namespace Nimbus::UI
