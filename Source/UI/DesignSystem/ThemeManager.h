#pragma once

#include <JuceHeader.h>

namespace Nimbus::DesignSystem {

/**
 * Audacity-inspired dark workspace tokens.  Components consume semantic tokens
 * rather than embedding palette literals, keeping the UI themeable without
 * leaking another framework's rendering model into Nimbus.
 */
struct ThemeManager final {
    enum class Token { workspace, panel, raisedPanel, control, border, text, mutedText, accent, record, selection };

    static juce::Colour colour(Token token) {
        switch (token) {
            case Token::workspace: return juce::Colour(0xff20252a);
            case Token::panel: return juce::Colour(0xff2b3238);
            case Token::raisedPanel: return juce::Colour(0xff394149);
            case Token::control: return juce::Colour(0xff56616b);
            case Token::border: return juce::Colour(0xff46515a);
            case Token::text: return juce::Colour(0xfff1f4f6);
            case Token::mutedText: return juce::Colour(0xffb6c0c9);
            case Token::accent: return juce::Colour(0xff9b8219);
            case Token::record: return juce::Colour(0xffd34c52);
            case Token::selection: return juce::Colour(0xff5794e6);
        }
        return juce::Colours::black;
    }
};

} // namespace Nimbus::DesignSystem
