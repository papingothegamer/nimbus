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
        // Draw the full waveform dimmed
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.3f));
        thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
        
        // Highlight the active cropped region
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        double startSecs = currentClip->getSourceOffsetSamples() / sampleRate;
        double endSecs = startSecs + (currentClip->getLengthSamples() / sampleRate);
        
        float x1 = static_cast<float>((startSecs / thumbnail.getTotalLength()) * getWidth());
        float x2 = static_cast<float>((endSecs / thumbnail.getTotalLength()) * getWidth());
        
        // Draw the active portion fully opaque
        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(static_cast<int>(x1), 0, static_cast<int>(x2 - x1), getHeight()));
        g.setColour(DesignSystem::Colors::PrimaryAction);
        thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
        g.restoreState();
        
        // Draw region borders
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawVerticalLine(static_cast<int>(x1), 0.0f, static_cast<float>(getHeight()));
        g.drawVerticalLine(static_cast<int>(x2), 0.0f, static_cast<float>(getHeight()));
        
        // Draw warp markers
        if (currentClip->isWarpEnabled()) {
            g.setColour(juce::Colours::yellow);
            for (double markerSample : currentClip->getWarpMarkers()) {
                double markerSecs = markerSample / sampleRate;
                float mx = static_cast<float>((markerSecs / thumbnail.getTotalLength()) * getWidth());
                
                // Draw vertical line
                g.drawVerticalLine(static_cast<int>(mx), 0.0f, static_cast<float>(getHeight()));
                
                // Draw triangle marker at the top
                juce::Path p;
                p.addTriangle(mx - 4.0f, 0.0f, mx + 4.0f, 0.0f, mx, 8.0f);
                g.fillPath(p);
            }
        }
    } else {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.drawText("Loading waveform...", getLocalBounds(), juce::Justification::centred, true);
    }
    
    // Draw playhead
    if (engine.getTransport().isPlaying()) {
        double positionSamples = engine.getTransport().getCurrentPosition();
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0.0) sampleRate = 48000.0;
        
        double clipGlobalStart = currentClip->getStartSample();
        double clipGlobalEnd = clipGlobalStart + currentClip->getLengthSamples();
        
        if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
            double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
            double sourceSecs = (currentClip->getSourceOffsetSamples() / sampleRate) + timeIntoClip;
            
            float px = 0.0f;
            if (thumbnail.getTotalLength() > 0.0) {
                px = static_cast<float>((sourceSecs / thumbnail.getTotalLength()) * getWidth());
            }
            
            g.setColour(DesignSystem::Colors::PrimaryAction);
            g.drawVerticalLine(static_cast<int>(px), 0.0f, static_cast<float>(getHeight()));
            
            juce::Path p;
            p.addTriangle(px - 5.0f, 0.0f, px + 5.0f, 0.0f, px, 8.0f);
            g.fillPath(p);
        }
    }
}

void AudioClipContent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &thumbnail) {
        if (auto* parent = getParentComponent()) {
            parent->resized(); // Trigger re-layout when thumbnail loads
        }
        repaint();
    }
}

void AudioClipContent::mouseDoubleClick(const juce::MouseEvent& e) {
    if (!currentClip || !currentClip->isWarpEnabled() || thumbnail.getTotalLength() <= 0.0) return;
    
    // Only allow inserting markers in the top region
    if (e.y <= 20) {
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        float proportion = e.x / static_cast<float>(getWidth());
        double markerSecs = proportion * thumbnail.getTotalLength();
        double markerSamples = markerSecs * sampleRate;
        
        currentClip->addWarpMarker(markerSamples);
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
    resized();
}

void AudioClipViewComponent::resized() {
    viewport.setBounds(getLocalBounds());
    
    // Scale width based on length, 100 pixels per second
    int desiredWidth = getWidth();
    if (content.getTotalLength() > 0.0) {
        desiredWidth = juce::jmax(getWidth(), static_cast<int>(content.getTotalLength() * 100.0));
    }
    
    content.setBounds(0, 0, desiredWidth, getHeight() - viewport.getScrollBarThickness());
}

} // namespace Nimbus::DetailView
