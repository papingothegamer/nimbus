#include "ClipComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::Timeline {

ClipComponent::ClipComponent(std::shared_ptr<AudioClip> clip, juce::AudioFormatManager& formatManager)
    : clipData(clip), thumbnail(512, formatManager, thumbnailCache) {
    if (clipData) {
        thumbnail.setSource(new juce::FileInputSource(clipData->getSourceFile()));
    }
}

ClipComponent::~ClipComponent() = default;

void ClipComponent::paint(juce::Graphics& g) {
    // Draw clip background
    g.fillAll(DesignSystem::Colors::ComponentBackground.brighter(0.1f));
    
    // Draw clip border
    g.setColour(DesignSystem::Colors::ComponentBorder);
    g.drawRect(getLocalBounds(), 1);

    // Draw waveform
    g.setColour(DesignSystem::Colors::PrimaryAction);
    if (thumbnail.getTotalLength() > 0.0) {
        thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
    } else {
        g.setFont(10.0f);
        g.drawText("Loading...", getLocalBounds(), juce::Justification::centred, false);
    }
}

void ClipComponent::resized() {
    //
}

} // namespace Nimbus::Timeline
