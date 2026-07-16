#include "MeteredFader.h"

namespace Nimbus::UI::Mixer {

MeteredFader::FaderLookAndFeel::FaderLookAndFeel() {
}

void MeteredFader::FaderLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                      float sliderPos, float minSliderPos, float maxSliderPos,
                                                      const juce::Slider::SliderStyle style, juce::Slider& slider) {
    // Draw the track line
    float trackWidth = 2.0f;
    float trackX = x + (width * 0.5f) - (trackWidth * 0.5f);
    g.setColour(juce::Colour(0xff1e1e1e));
    g.fillRect(trackX, (float)y, trackWidth, (float)height);

    // Draw tick marks (Ableton-style)
    g.setColour(juce::Colour(0xff3f3f3f));
    for (int i = 0; i <= 6; ++i) {
        float ty = y + (height * (i / 6.0f));
        g.fillRect(x + width * 0.5f, ty - 0.5f, width * 0.4f, 1.0f);
    }

    // Draw the sleek rectangular thumb
    float thumbHeight = 16.0f;
    float thumbWidth = width * 0.8f;
    float thumbX = x + (width * 0.5f) - (thumbWidth * 0.5f);
    float thumbY = sliderPos - (thumbHeight * 0.5f);
    
    // Thumb background
    g.setColour(juce::Colour(0xff4a4a4a));
    g.fillRect(thumbX, thumbY, thumbWidth, thumbHeight);
    
    // Thumb highlight/border
    g.setColour(juce::Colour(0xff5f5f5f));
    g.drawRect(thumbX, thumbY, thumbWidth, thumbHeight, 1.0f);

    // Thumb center indicator line
    g.setColour(juce::Colour(0xff00ffff)); // Cyan-ish or white line for visibility
    g.fillRect(thumbX + 2.0f, sliderPos - 1.0f, thumbWidth - 4.0f, 2.0f);
}

MeteredFader::MeteredFader() {
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(0.0, 1.0, 0.001);
    slider.setValue(0.75);
    slider.setLookAndFeel(&customLookAndFeel);
}

MeteredFader::~MeteredFader() {
    slider.setLookAndFeel(nullptr);
}

void MeteredFader::setTrackInfo(TrackType type, bool isStereo) {
    if (trackType != type || stereo != isStereo) {
        trackType = type;
        stereo = isStereo;
        repaint();
    }
}

void MeteredFader::setLevel(float peakLevel) {
    if (std::abs(peakLevel - currentLevel) > 0.005f) {
        currentLevel = peakLevel;
        // Only repaint the meter section to save CPU
        int meterWidth = 8;
        if (stereo && trackType == TrackType::Audio) meterWidth = 12;
        repaint(getWidth() - meterWidth - 2, 0, meterWidth + 2, getHeight());
    }
}

void MeteredFader::paint(juce::Graphics& g) {
    // Draw background
    g.fillAll(DesignSystem::Colors::ModuleBackground.darker(0.1f));

    // Meter positioning
    auto bounds = getLocalBounds();
    
    int meterWidth = 8; // Default for mono or MIDI
    if (stereo && trackType == TrackType::Audio) {
        meterWidth = 12; // Stereo needs more space for 2 bars
    }

    auto meterBounds = bounds.removeFromRight(meterWidth).reduced(0, 4).withTrimmedRight(2).toFloat();

    // Draw meter background
    g.setColour(juce::Colour(0xff000000));
    g.fillRect(meterBounds);

    if (currentLevel > 0.0f) {
        float fillHeight = meterBounds.getHeight() * currentLevel;
        auto fillBounds = meterBounds.withTrimmedTop(meterBounds.getHeight() - fillHeight);

        if (trackType == TrackType::Midi || trackType == TrackType::Instrument) {
            // distinct velocity indicator / color accent (e.g. orange)
            g.setColour(juce::Colour(0xffff9900));
            g.fillRect(fillBounds);
        } else {
            // Audio gradient
            juce::ColourGradient cg(juce::Colours::lime, meterBounds.getBottomLeft(),
                                    juce::Colours::red, meterBounds.getTopLeft(), false);
            cg.addColour(0.7f, juce::Colours::yellow);
            g.setGradientFill(cg);

            if (stereo) {
                // Split VU meter for stereo
                float halfWidth = (meterBounds.getWidth() - 1.0f) * 0.5f;
                g.fillRect(fillBounds.getX(), fillBounds.getY(), halfWidth, fillBounds.getHeight());
                g.fillRect(fillBounds.getX() + halfWidth + 1.0f, fillBounds.getY(), halfWidth, fillBounds.getHeight());
            } else {
                // Mono single bar
                g.fillRect(fillBounds);
            }
        }
    }
}

void MeteredFader::resized() {
    auto bounds = getLocalBounds();
    int meterWidth = 8;
    if (stereo && trackType == TrackType::Audio) meterWidth = 12;
    
    // Slider takes the remaining space on the left
    slider.setBounds(bounds.withTrimmedRight(meterWidth + 2));
}

} // namespace Nimbus::UI::Mixer
