#include "AudioClipViewComponent.h"
#include "Core/NimbusEngine.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Iconography.h"
#include "UI/DesignSystem/NimbusLookAndFeel.h"

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
    // Premium dark background #111111
    g.fillAll(juce::Colour(0xff111111));
    
    // Draw subtle 1px grid lines
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    double beatPixels = 100.0; // Estimate
    for (float x = 0; x < getWidth(); x += beatPixels) {
        g.drawVerticalLine(static_cast<int>(x), 0, static_cast<float>(getHeight()));
    }
    
    if (!currentClip) {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.drawText("No audio clip selected", getLocalBounds(), juce::Justification::centred, true);
        return;
    }
    
    if (thumbnail.getTotalLength() > 0.0) {
        auto clipBounds = g.getClipBounds();
        double visibleStartSecs = (clipBounds.getX() / (double)getWidth()) * thumbnail.getTotalLength();
        double visibleEndSecs = (clipBounds.getRight() / (double)getWidth()) * thumbnail.getTotalLength();
        
        visibleStartSecs = juce::jlimit(0.0, thumbnail.getTotalLength(), visibleStartSecs);
        visibleEndSecs = juce::jlimit(0.0, thumbnail.getTotalLength(), visibleEndSecs);

        // Draw the full waveform dimmed (only for visible bounds)
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.3f));
        thumbnail.drawChannels(g, clipBounds.reduced(2), visibleStartSecs, visibleEndSecs, 1.0f);
        
        // Highlight the active cropped region
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        double startSecs = currentClip->sourceOffsetSamples.get() / sampleRate;
        double endSecs = startSecs + (currentClip->lengthSamples.get() / sampleRate);
        
        float x1 = static_cast<float>((startSecs / thumbnail.getTotalLength()) * getWidth());
        float x2 = static_cast<float>((endSecs / thumbnail.getTotalLength()) * getWidth());
        
        // Draw the active portion fully opaque with gradient
        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(static_cast<int>(x1), 0, static_cast<int>(x2 - x1), getHeight()));
        
        juce::ColourGradient gradient(juce::Colour(0xff00b4db), 0.0f, static_cast<float>(getHeight() / 2),
                                      juce::Colour(0xff0083b0), 0.0f, static_cast<float>(getHeight()), false);
        gradient.addColour(0.0f, juce::Colour(0xff0083b0)); // deep blue
        gradient.addColour(0.5f, juce::Colour(0xff00ffff)); // bright cyan
        gradient.addColour(1.0f, juce::Colour(0xff0083b0)); // deep blue
        g.setGradientFill(gradient);
        
        thumbnail.drawChannels(g, clipBounds.reduced(2), visibleStartSecs, visibleEndSecs, 1.0f);
        g.restoreState();
        
        // Draw region borders
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        if (x1 >= clipBounds.getX() && x1 <= clipBounds.getRight()) {
            g.drawVerticalLine(static_cast<int>(x1), 0.0f, static_cast<float>(getHeight()));
        }
        if (x2 >= clipBounds.getX() && x2 <= clipBounds.getRight()) {
            g.drawVerticalLine(static_cast<int>(x2), 0.0f, static_cast<float>(getHeight()));
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
        
        double clipGlobalStart = currentClip->startSample.get();
        double clipGlobalEnd = clipGlobalStart + currentClip->lengthSamples.get();
        
        if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
            double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
            double sourceSecs = (currentClip->sourceOffsetSamples.get() / sampleRate) + timeIntoClip;
            
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

void AudioClipContent::mouseDown(const juce::MouseEvent& e) {
    if (!currentClip || thumbnail.getTotalLength() <= 0.0) return;
    
    // To implement slip-stretch editing, we can check if we clicked near the edges of the active region
    // while holding Alt.
}

void AudioClipContent::mouseDrag(const juce::MouseEvent& e) {
    if (!currentClip || thumbnail.getTotalLength() <= 0.0) return;
    
    // Alt-drag stretching logic to go here
}

void AudioClipContent::mouseUp(const juce::MouseEvent& e) {
}

void AudioClipContent::mouseDoubleClick(const juce::MouseEvent& e) {
}

// ==============================================================================

AudioClipViewComponent::AudioClipViewComponent(NimbusEngine& e) : engine(e), content(e) {
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
    
    addAndMakeVisible(zoomInButton);
    addAndMakeVisible(zoomOutButton);
    addAndMakeVisible(followButton);
    
    // Properties for icon buttons so they look correct
    zoomInButton.getProperties().set("transparentBackground", true);
    zoomOutButton.getProperties().set("transparentBackground", true);
    followButton.getProperties().set("transparentBackground", true);
    
    followButton.setClickingTogglesState(true);
    followButton.setToggleState(autoScrollEnabled, juce::dontSendNotification);
    
    zoomInButton.onClick = [this] {
        zoomFactor = juce::jlimit(0.001, 1000.0, zoomFactor * 1.5);
        resized();
    };
    
    zoomOutButton.onClick = [this] {
        zoomFactor = juce::jlimit(0.001, 1000.0, zoomFactor / 1.5);
        resized();
    };
    
    followButton.onClick = [this] {
        autoScrollEnabled = followButton.getToggleState();
    };
    
    startTimerHz(30);
}

AudioClipViewComponent::~AudioClipViewComponent() {
    stopTimer();
}

void AudioClipViewComponent::setAudioClip(std::shared_ptr<AudioClip> clip) {
    bool isNewClip = (currentClip != clip);
    currentClip = clip;
    content.setAudioClip(clip);
    
    if (isNewClip && clip && content.getTotalLength() > 0.0 && getWidth() > 0) {
        zoomFactor = static_cast<double>(getWidth()) / (content.getTotalLength() * 100.0);
        zoomFactor = juce::jlimit(0.001, 1000.0, zoomFactor);
    }
    
    resized();
}

void AudioClipViewComponent::lookAndFeelChanged() {
    // Buttons are juce::TextButton now, so NimbusLookAndFeel handles the SVGs automatically.
}

void AudioClipViewComponent::resized() {
    viewport.setBounds(getLocalBounds());
    
    int btnSize = 24;
    int padding = 10;
    
    zoomInButton.setBounds(getWidth() - btnSize - padding, padding, btnSize, btnSize);
    zoomOutButton.setBounds(getWidth() - btnSize * 2 - padding * 2, padding, btnSize, btnSize);
    followButton.setBounds(getWidth() - btnSize * 3 - padding * 3, padding, btnSize, btnSize);
    
    zoomInButton.toFront(false);
    zoomOutButton.toFront(false);
    followButton.toFront(false);
    
    // Scale width based on length, 100 pixels per second
    int desiredWidth = getWidth();
    if (content.getTotalLength() > 0.0) {
        desiredWidth = juce::jmax(getWidth(), static_cast<int>(content.getTotalLength() * 100.0 * zoomFactor));
    }
    
    content.setBounds(0, 0, desiredWidth, getHeight() - viewport.getScrollBarThickness());
}

void AudioClipViewComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (event.mods.isCtrlDown() || event.mods.isAltDown()) {
        double oldZoom = zoomFactor;
        if (wheel.deltaY > 0) zoomFactor *= 1.1;
        else if (wheel.deltaY < 0) zoomFactor /= 1.1;
        
        zoomFactor = juce::jlimit(0.001, 1000.0, zoomFactor);
        
        if (oldZoom != zoomFactor) {
            double ratio = zoomFactor / oldZoom;
            int currentX = viewport.getViewPositionX() + event.x;
            int newX = static_cast<int>(currentX * ratio) - event.x;
            
            resized();
            viewport.setViewPosition(newX, viewport.getViewPositionY());
        }
    } else {
        juce::Component::mouseWheelMove(event, wheel);
    }
}

void AudioClipViewComponent::timerCallback() {
    if (!currentClip || !engine.getTransport().isPlaying()) return;
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double positionSamples = engine.getTransport().getCurrentPosition();
    double clipGlobalStart = currentClip->startSample.get();
    double clipGlobalEnd = clipGlobalStart + currentClip->lengthSamples.get();
    
    if (positionSamples >= clipGlobalStart && positionSamples <= clipGlobalEnd) {
        double timeIntoClip = (positionSamples - clipGlobalStart) / sampleRate;
        double sourceSecs = (currentClip->sourceOffsetSamples.get() / sampleRate) + timeIntoClip;
        
        if (content.getTotalLength() > 0.0) {
            float px = static_cast<float>((sourceSecs / content.getTotalLength()) * content.getWidth());
            
            int viewWidth = viewport.getViewWidth();
            int viewX = viewport.getViewPositionX();
            
            if (autoScrollEnabled) {
                if (px > viewX + viewWidth * 0.8f) {
                    viewport.setViewPosition(static_cast<int>(px - viewWidth * 0.8f), viewport.getViewPositionY());
                } else if (px < viewX + viewWidth * 0.2f && px > viewWidth * 0.2f) {
                    viewport.setViewPosition(static_cast<int>(px - viewWidth * 0.2f), viewport.getViewPositionY());
                }
            }
        }
        
        content.repaint();
    }
}

} // namespace Nimbus::DetailView
