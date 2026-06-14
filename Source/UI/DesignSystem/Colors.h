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

inline const juce::Colour Colors::AppBackground       = juce::Colour::fromString("#FF111318");
inline const juce::Colour Colors::PanelBackground     = juce::Colour::fromString("#FF181A20");
inline const juce::Colour Colors::ModuleBackground    = juce::Colour::fromString("#FF22252D");
inline const juce::Colour Colors::ComponentBackground = juce::Colour::fromString("#FF2A2D35");

inline const juce::Colour Colors::Divider             = juce::Colour::fromString("#FF333742");
inline const juce::Colour Colors::ComponentBorder     = juce::Colour::fromString("#FF4A4E5A");

inline const juce::Colour Colors::PrimaryAction       = juce::Colour::fromString("#FF4A90E2");
inline const juce::Colour Colors::RecordDanger        = juce::Colour::fromString("#FFE04A4A");
inline const juce::Colour Colors::Solo                = juce::Colour::fromString("#FFE0C04A");
inline const juce::Colour Colors::Mute                = juce::Colour::fromString("#FFD35400");
inline const juce::Colour Colors::Success             = juce::Colour::fromString("#FF2ECC71");

inline const juce::Colour Colors::TextPrimary         = juce::Colour::fromString("#FFE2E4E9");
inline const juce::Colour Colors::TextSecondary       = juce::Colour::fromString("#FFA0A4B0");
inline const juce::Colour Colors::TextTitle           = juce::Colour::fromString("#FF888C96");
inline const juce::Colour Colors::TextMicro           = juce::Colour::fromString("#FF6B6F7D");

} // namespace Nimbus::DesignSystem
