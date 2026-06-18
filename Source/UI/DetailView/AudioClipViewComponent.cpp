#include "AudioClipViewComponent.h"
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::DetailView {

AudioClipContent::AudioClipContent(NimbusEngine& e) 
    : engine(e), 
      thumbnail(512, engine.getFormatManager(), engine.getThumbnailCache()) {
    thumbnail.addChangeListener(this);
}

AudioClipContent::~AudioClipContent() {
    thumbnail.removeChangeListener(this);
}

void AudioClipContent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    currentClip = clip;
    if (currentClip) {
        auto file = currentClip->getSourceFile();
        if (file.existsAsFile()) {
            thumbnail.setSource(new juce::FileInputSource(file));
        } else {
            thumbnail.clear();
        }
    } else {
        thumbnail.clear();
    }
    repaint();
}

void AudioClipContent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.05f));
    
    if (!currentClip) {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.drawText("No audio clip selected", getLocalBounds(), juce::Justification::centred, true);
        return;
    }
    
    if (thumbnail.getTotalLength() > 0.0) {
        g.setColour(DesignSystem::Colors::PrimaryAction);
        thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
    } else {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.drawText("Loading waveform...", getLocalBounds(), juce::Justification::centred, true);
    }
}

void AudioClipContent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &thumbnail) {
        repaint();
    }
}

// ==============================================================================

AudioClipViewComponent::AudioClipViewComponent(NimbusEngine& e) : content(e) {
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
}

AudioClipViewComponent::~AudioClipViewComponent() = default;

void AudioClipViewComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    content.setAudioClip(clip);
}

void AudioClipViewComponent::resized() {
    viewport.setBounds(getLocalBounds());
    content.setBounds(0, 0, juce::jmax(1000, getWidth()), getHeight() - viewport.getScrollBarThickness());
}

} // namespace Nimbus::DetailView
