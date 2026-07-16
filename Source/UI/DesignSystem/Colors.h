#pragma once

#include <JuceHeader.h>

namespace Nimbus::DesignSystem {

struct Colors {
    // Base Canvas (Matte Dark Theme)
    static const juce::Colour AppBackground;
    static const juce::Colour PanelBackground;
    static const juce::Colour ModuleBackground;
    static const juce::Colour ComponentBackground;
    
    // Borders and Separators
    static const juce::Colour Divider;
    static const juce::Colour ComponentBorder;

    // Accent Colors
    static const juce::Colour PrimaryAction;
    static const juce::Colour RecordDanger;
    static const juce::Colour Solo;
    static const juce::Colour Mute;
    static const juce::Colour Success;

    // Typography Colors
    static const juce::Colour TextPrimary;
    static const juce::Colour TextSecondary;
    static const juce::Colour TextTitle;
    static const juce::Colour TextMicro;
};

inline const juce::Colour Colors::AppBackground       = juce::Colour::fromString("#FF1E1E1E"); // Audacity dark background
inline const juce::Colour Colors::PanelBackground     = juce::Colour::fromString("#FF2D2D30"); // Audacity panels
inline const juce::Colour Colors::ModuleBackground    = juce::Colour::fromString("#FF252526"); // Audacity track/module
inline const juce::Colour Colors::ComponentBackground = juce::Colour::fromString("#FF3F3F46"); // Audacity component

inline const juce::Colour Colors::Divider             = juce::Colour::fromString("#FF121212"); // Darker separator
inline const juce::Colour Colors::ComponentBorder     = juce::Colour::fromString("#FF555555"); // Lighter border

inline const juce::Colour Colors::PrimaryAction       = juce::Colour::fromString("#FF1C69D4"); // Audacity primary blue
inline const juce::Colour Colors::RecordDanger        = juce::Colour::fromString("#FFD43232"); // Audacity record red
inline const juce::Colour Colors::Solo                = juce::Colour::fromString("#FFD4B81C"); // Audacity yellow
inline const juce::Colour Colors::Mute                = juce::Colour::fromString("#FFD46A1C"); // Audacity orange
inline const juce::Colour Colors::Success             = juce::Colour::fromString("#FF2E8A38"); // Audacity green

inline const juce::Colour Colors::TextPrimary         = juce::Colour::fromString("#FFE0E0E0");
inline const juce::Colour Colors::TextSecondary       = juce::Colour::fromString("#FFA0A0A0");
inline const juce::Colour Colors::TextTitle           = juce::Colour::fromString("#FFFFFFFF");
inline const juce::Colour Colors::TextMicro           = juce::Colour::fromString("#FF808080");

} // namespace Nimbus::DesignSystem
