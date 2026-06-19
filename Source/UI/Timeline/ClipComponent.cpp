#include "ClipComponent.h"
#include "TimelineComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::Timeline {

ClipComponent::ClipComponent(AnyClipPtr clip, NimbusEngine& e)
    : engine(e), clipData(clip), thumbnail(512, engine.getFormatManager(), thumbnailCache) {
    if (std::holds_alternative<std::shared_ptr<AudioClip>>(clipData)) {
        auto audioClip = std::get<std::shared_ptr<AudioClip>>(clipData);
        if (audioClip) {
            thumbnail.setSource(new juce::FileInputSource(audioClip->getSourceFile()));
        }
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
    if (std::holds_alternative<std::shared_ptr<AudioClip>>(clipData)) {
        g.setColour(DesignSystem::Colors::PrimaryAction);
        if (thumbnail.getTotalLength() > 0.0) {
            thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
        } else {
            g.setFont(10.0f);
            g.drawText("Loading...", getLocalBounds(), juce::Justification::centred, false);
        }
    } else if (std::holds_alternative<std::shared_ptr<MidiClip>>(clipData)) {
        auto midiClip = std::get<std::shared_ptr<MidiClip>>(clipData);
        if (midiClip) {
            g.setColour(juce::Colours::yellow.withAlpha(0.8f));
            int minNote = 127;
            int maxNote = 0;
            for (int i = 0; i < midiClip->getSequence().getNumEvents(); ++i) {
                auto* evt = midiClip->getSequence().getEventPointer(i);
                if (evt->message.isNoteOn()) {
                    minNote = juce::jmin(minNote, evt->message.getNoteNumber());
                    maxNote = juce::jmax(maxNote, evt->message.getNoteNumber());
                }
            }
            if (maxNote < minNote) { minNote = 60; maxNote = 72; }
            int range = juce::jmax(1, maxNote - minNote + 2); // padding
            
            double clipSamples = midiClip->getLengthSamples();
            for (int i = 0; i < midiClip->getSequence().getNumEvents(); ++i) {
                auto* event = midiClip->getSequence().getEventPointer(i);
                if (event->message.isNoteOn()) {
                    double noteStart = event->message.getTimeStamp();
                    double noteLength = 48000.0 * 0.25; // default
                    
                    for (int j = i + 1; j < midiClip->getSequence().getNumEvents(); ++j) {
                        auto* offEvent = midiClip->getSequence().getEventPointer(j);
                        if (offEvent->message.isNoteOff() && offEvent->message.getNoteNumber() == event->message.getNoteNumber()) {
                            noteLength = offEvent->message.getTimeStamp() - noteStart;
                            break;
                        }
                    }
                    
                    if (clipSamples > 0.0) {
                        float x = static_cast<float>((noteStart / clipSamples) * getWidth());
                        float w = static_cast<float>((noteLength / clipSamples) * getWidth());
                        float y = static_cast<float>(maxNote + 1 - event->message.getNoteNumber()) / static_cast<float>(range) * getHeight();
                        float h = juce::jmax(2.0f, getHeight() / static_cast<float>(range));
                        g.fillRect(x, y, juce::jmax(1.0f, w), h);
                    }
                }
            }
        }
    }
}

void ClipComponent::resized() {
    //
}

void ClipComponent::mouseDown(const juce::MouseEvent& event) {
    engine.getTimelineProject().setSelectedClip(clipData);
    
    if (event.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Cut");
        menu.addItem(2, "Copy");
        menu.addSeparator();
        menu.addItem(3, "Delete");
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 3) {
                // Delete clip logic (needs parent to remove it, or via project)
                // We'll leave it as a placeholder or call project remove
            }
        });
        return;
    }

    dragStartX = event.getScreenX();
    
    if (std::holds_alternative<std::shared_ptr<AudioClip>>(clipData)) {
        if (auto ac = std::get<std::shared_ptr<AudioClip>>(clipData)) {
            originalStartSamples = ac->getStartSample();
            originalLengthSamples = ac->getLengthSamples();
        }
    } else if (std::holds_alternative<std::shared_ptr<MidiClip>>(clipData)) {
        if (auto mc = std::get<std::shared_ptr<MidiClip>>(clipData)) {
            originalStartSamples = mc->getStartSample();
            originalLengthSamples = mc->getLengthSamples();
        }
    }
    
    isResizing = (event.x >= getWidth() - 5);
    isDragging = !isResizing;
}

void ClipComponent::mouseMove(const juce::MouseEvent& event) {
    if (event.x >= getWidth() - 5) {
        setMouseCursor(juce::MouseCursor::RightEdgeResizeCursor);
    } else {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void ClipComponent::mouseDrag(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) return;
    
    int deltaX = event.getScreenX() - dragStartX;
    
    // We need to know pixels per second to convert deltaX to samples
    // For now, we'll try to find the timeline component parent to get pixelsPerSecond
    double pixelsPerSecond = 100.0; // fallback
    if (auto* parent = getParentComponent()) {
        // TrackLaneComponent -> tracksContainer -> viewport -> TimelineComponent
        // Simplest way is to ask the engine or assume a fixed pixel rate for now
        // TimelineComponent has getPixelsPerSecond(), but we don't have a direct ref here.
        // We'll traverse parents to find it.
        auto* comp = getParentComponent();
        while (comp != nullptr) {
            if (auto* tc = dynamic_cast<TimelineComponent*>(comp)) {
                pixelsPerSecond = tc->getPixelsPerSecond();
                break;
            }
            comp = comp->getParentComponent();
        }
    }
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double deltaSeconds = static_cast<double>(deltaX) / pixelsPerSecond;
    double deltaSamples = deltaSeconds * sampleRate;
    
    // Also snap to grid
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    double snapSeconds = secondsPerBeat / 4.0; // 1/16th note snap
    
    if (isResizing) {
        double newLengthSamples = originalLengthSamples + deltaSamples;
        double newLengthSeconds = newLengthSamples / sampleRate;
        double snappedLengthSeconds = std::round(newLengthSeconds / snapSeconds) * snapSeconds;
        newLengthSamples = juce::jmax(100.0, snappedLengthSeconds * sampleRate);
        
        if (std::holds_alternative<std::shared_ptr<AudioClip>>(clipData)) {
            if (auto ac = std::get<std::shared_ptr<AudioClip>>(clipData)) {
                ac->setLengthSamples(newLengthSamples);
            }
        } else if (std::holds_alternative<std::shared_ptr<MidiClip>>(clipData)) {
            if (auto mc = std::get<std::shared_ptr<MidiClip>>(clipData)) {
                mc->setLengthSamples(newLengthSamples);
            }
        }
    } else if (isDragging) {
        double newStartSamples = originalStartSamples + deltaSamples;
        double newStartSeconds = newStartSamples / sampleRate;
        double snappedStartSeconds = std::round(newStartSeconds / snapSeconds) * snapSeconds;
        newStartSamples = juce::jmax(0.0, snappedStartSeconds * sampleRate);
        
        if (std::holds_alternative<std::shared_ptr<AudioClip>>(clipData)) {
            if (auto ac = std::get<std::shared_ptr<AudioClip>>(clipData)) {
                ac->setStartSample(newStartSamples);
            }
        } else if (std::holds_alternative<std::shared_ptr<MidiClip>>(clipData)) {
            if (auto mc = std::get<std::shared_ptr<MidiClip>>(clipData)) {
                mc->setStartSample(newStartSamples);
            }
        }
    }
    
    if (auto* parent = getParentComponent()) {
        parent->resized();
    }
}

void ClipComponent::mouseUp(const juce::MouseEvent& event) {
    isDragging = false;
    isResizing = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

} // namespace Nimbus::Timeline
