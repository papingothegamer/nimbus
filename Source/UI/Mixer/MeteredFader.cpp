#include "MeteredFader.h"

namespace Nimbus::UI {

void MeteredFader::FaderLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                      float sliderPos, float minSliderPos, float maxSliderPos,
                                                      const juce::Slider::SliderStyle style, juce::Slider& slider) {
    float meterWidth = width - 12.0f; 

    // Horizontal line overlaid perfectly on top of the meter
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.fillRect((float)x, sliderPos - 1.0f, meterWidth, 2.0f);

    // Ableton-style triangle pointer thumb on the right
    juce::Path p;
    float pW = 6.0f;
    float pH = 8.0f;
    float pX = meterWidth + 1.0f; 
    
    p.addTriangle(pX, sliderPos, pX + pW, sliderPos - pH * 0.5f, pX + pW, sliderPos + pH * 0.5f);
    g.setColour(juce::Colour(0xff888888));
    g.fillPath(p);
    g.setColour(juce::Colour(0xff222222));
    
    // FIX: JUCE uses strokePath, not drawPath!
    g.strokePath(p, juce::PathStrokeType(1.0f)); 
}

MeteredFader::MeteredFader() {
    volumeSlider.setLookAndFeel(&customLaf);
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    // Range maps -60dB to +10dB directly
    volumeSlider.setRange(-60.0, 10.0, 0.1); 
    volumeSlider.setValue(0.0);
    addAndMakeVisible(volumeSlider);
}

MeteredFader::~MeteredFader() {
    volumeSlider.setLookAndFeel(nullptr);
}

void MeteredFader::setLevel(float levelLeft, float levelRight) {
    if (std::abs(levelLeft - currentLevelL) > 0.01f || std::abs(levelRight - currentLevelR) > 0.01f) {
        currentLevelL = levelLeft;
        currentLevelR = levelRight;
        repaint();
    }
}

void MeteredFader::setTrackType(TrackType newType) {
    if (type != newType) {
        type = newType;
        repaint();
    }
}

void MeteredFader::resized() {
    volumeSlider.setBounds(getLocalBounds());
}

void MeteredFader::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float meterWidth = bounds.getWidth() - 12.0f; 
    juce::Rectangle<float> meterBounds(0, 0, meterWidth, bounds.getHeight());

    // 1. Draw Meter Background
    g.setColour(juce::Colour(0xff111111));
    if (type == TrackType::StereoAudio) {
        float halfW = (meterWidth / 2.0f) - 0.5f;
        g.fillRect(0.0f, 0.0f, halfW, bounds.getHeight());
        g.fillRect(halfW + 1.0f, 0.0f, halfW, bounds.getHeight());
    } else {
        g.fillRect(meterBounds);
    }

    // 2. Draw Ticks beside the VU meter (-60 to +10 in steps of 5)
    g.setColour(juce::Colour(0xff555555));
    for (int db = -60; db <= 10; db += 5) {
        float proportion = 1.0f - ((db + 60.0f) / 70.0f);
        float y = proportion * bounds.getHeight();
        g.fillRect(meterWidth + 2.0f, y, 4.0f, 1.0f);
    }

    if (currentLevelL <= 0.0f && currentLevelR <= 0.0f) return;

    // 3. Draw Active Meter
    juce::ColourGradient cg(juce::Colours::lime, meterBounds.getBottomLeft(), juce::Colours::red, meterBounds.getTopLeft(), false);
    cg.addColour(0.7f, juce::Colours::yellow);

    if (type == TrackType::MonoAudio) {
        int fillHeight = juce::roundToInt(meterBounds.getHeight() * currentLevelL);
        g.setGradientFill(cg);
        g.fillRect(meterBounds.withTrimmedTop(meterBounds.getHeight() - fillHeight));
    } 
    else if (type == TrackType::StereoAudio) {
        float halfW = (meterWidth / 2.0f) - 0.5f;
        int fillHL = juce::roundToInt(meterBounds.getHeight() * currentLevelL);
        int fillHR = juce::roundToInt(meterBounds.getHeight() * currentLevelR);
        
        g.setGradientFill(cg);
        g.fillRect(0.0f, meterBounds.getHeight() - fillHL, halfW, (float)fillHL);
        g.fillRect(halfW + 1.0f, meterBounds.getHeight() - fillHR, halfW, (float)fillHR);
    }
}

} // namespace Nimbus::UI