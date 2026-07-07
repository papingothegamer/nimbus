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

inline const juce::Colour Colors::AppBackground       = juce::Colour::fromString("#FF131313");
inline const juce::Colour Colors::PanelBackground     = juce::Colour::fromString("#FF1C1C1E");
inline const juce::Colour Colors::ModuleBackground    = juce::Colour::fromString("#FF252528");
inline const juce::Colour Colors::ComponentBackground = juce::Colour::fromString("#FF2E2E32");

inline const juce::Colour Colors::Divider             = juce::Colour::fromString("#FF35353A");
inline const juce::Colour Colors::ComponentBorder     = juce::Colour::fromString("#FF424248");

inline const juce::Colour Colors::PrimaryAction       = juce::Colour::fromString("#FF0A84FF"); // Swift blue
inline const juce::Colour Colors::RecordDanger        = juce::Colour::fromString("#FFFF453A"); // Swift red
inline const juce::Colour Colors::Solo                = juce::Colour::fromString("#FFFFD60A"); // Swift yellow
inline const juce::Colour Colors::Mute                = juce::Colour::fromString("#FFFF9F0A"); // Swift orange
inline const juce::Colour Colors::Success             = juce::Colour::fromString("#FF32D74B"); // Swift green

inline const juce::Colour Colors::TextPrimary         = juce::Colour::fromString("#FFFFFFFF");
inline const juce::Colour Colors::TextSecondary       = juce::Colour::fromString("#FF98989D");
inline const juce::Colour Colors::TextTitle           = juce::Colour::fromString("#FFC7C7CC");
inline const juce::Colour Colors::TextMicro           = juce::Colour::fromString("#FF8E8E93");

} // namespace Nimbus::DesignSystem
