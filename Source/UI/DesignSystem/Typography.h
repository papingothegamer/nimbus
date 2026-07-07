#pragma once

#include <JuceHeader.h>

namespace Nimbus::DesignSystem {

struct Typography {
    static juce::Typeface::Ptr getChakraPetchRegular() {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::ChakraPetchRegular_ttf, BinaryData::ChakraPetchRegular_ttfSize);
        return typeface;
    }

    static juce::Typeface::Ptr getChakraPetchBold() {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::ChakraPetchBold_ttf, BinaryData::ChakraPetchBold_ttfSize);
        return typeface;
    }

    static juce::Typeface::Ptr getInterRegular() {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
        return typeface;
    }

    static juce::Typeface::Ptr getInterBold() {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::InterBold_ttf, BinaryData::InterBold_ttfSize);
        return typeface;
    }
    
    static juce::Typeface::Ptr getMaterialIcons() {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::MaterialSymbolsRounded_ttf, BinaryData::MaterialSymbolsRounded_ttfSize);
        return typeface;
    }

    static juce::Font getGiantFont() {
        return juce::Font(getInterBold()).withHeight(28.0f);
    }

    static juce::Font getTitleFont() {
        return juce::Font(getInterBold()).withHeight(14.0f);
    }

    static juce::Font getPrimaryFont() {
        return juce::Font(getInterRegular()).withHeight(13.0f);
    }

    static juce::Font getSecondaryFont() {
        return juce::Font(getInterRegular()).withHeight(11.0f);
    }

    static juce::Font getMicroFont() {
        return juce::Font(getInterRegular()).withHeight(9.0f);
    }
    
    static juce::Font getMonospacedFont(float size) {
        return juce::Font(getInterRegular()).withHeight(size); 
    }
    
    static juce::Font getIconFont(float size = 16.0f) {
        return juce::Font(getMaterialIcons()).withHeight(size);
    }
};

} // namespace Nimbus::DesignSystem
