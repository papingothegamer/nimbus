#include "ClipComponent.h"
#include "TimelineComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::Timeline {

juce::Colour ClipComponent::getClipColor(int index) {
    if (index < 0) return juce::Colour(0xff0a84ff);
    float hue = std::fmod(index * 0.381966f, 1.0f);
    return juce::Colour::fromHSV(hue, 0.75f, 0.75f, 1.0f);
}

class ColorGridComponent : public juce::PopupMenu::CustomComponent {
public:
    ColorGridComponent(int cols, int rows, std::function<void(int)> onSelect) 
        : juce::PopupMenu::CustomComponent(false), columns(cols), numRows(rows), onColorSelected(std::move(onSelect)) {
    }
    
    void getIdealSize(int& idealWidth, int& idealHeight) override {
        idealWidth = columns * 16 + 4;
        idealHeight = numRows * 16 + 4;
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff1e1e1e));
        for (int i = 0; i < columns * numRows; ++i) {
            int x = 2 + (i % columns) * 16;
            int y = 2 + (i / columns) * 16;
            g.setColour(ClipComponent::getClipColor(i));
            g.fillRect(x + 1, y + 1, 14, 14);
            if (hoverIndex == i) {
                g.setColour(juce::Colours::white);
                g.drawRect(x, y, 16, 16, 2);
            }
        }
    }
    void mouseMove(const juce::MouseEvent& event) override {
        int col = (event.x - 2) / 16;
        int row = (event.y - 2) / 16;
        int idx = (col >= 0 && col < columns && row >= 0 && row < numRows) ? row * columns + col : -1;
        if (idx != hoverIndex) { hoverIndex = idx; repaint(); }
    }
    void mouseExit(const juce::MouseEvent&) override { hoverIndex = -1; repaint(); }
    void mouseDown(const juce::MouseEvent& event) override {
        if (hoverIndex != -1) {
            if (onColorSelected) onColorSelected(hoverIndex);
            juce::PopupMenu::dismissAllActiveMenus();
        }
    }
private:
    int columns, numRows;
    int hoverIndex = -1;
    std::function<void(int)> onColorSelected;
};

class UniqueFileInputSource : public juce::FileInputSource {
public:
    UniqueFileInputSource(const juce::File& file, juce::int64 uniqueId)
        : juce::FileInputSource(file), hash(file.hashCode64() ^ uniqueId) {}

    juce::int64 hashCode() const override { return hash; }

private:
    juce::int64 hash;
};

ClipComponent::ClipComponent(AnyClipPtr clip, NimbusEngine& e)
    : engine(e), clipData(clip), thumbnail(512, engine.getFormatManager(), engine.getThumbnailCache()) {
    
    if (clipData->getType() == Clip::Type::Audio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
        if (audioClip) {
            juce::int64 uniqueId = reinterpret_cast<juce::int64>(audioClip.get());
            thumbnail.setSource(new UniqueFileInputSource(audioClip->getSourceFile(), uniqueId));
        }
    }
    
    thumbnail.addChangeListener(this);
    
    // No label needed, we draw the text in paint
}

void ClipComponent::showPropertiesMenu() {
    juce::PopupMenu menu;
    
    juce::PopupMenu colorMenu;
    auto* grid = new ColorGridComponent(10, 7, [this](int newColor) {
        clipData->colorIndex = newColor;
        repaint();
        engine.getTimelineProject().notifyClipModified();
    });
    colorMenu.addCustomItem(1, std::unique_ptr<juce::PopupMenu::CustomComponent>(grid));
    menu.addSubMenu("Clip Color", colorMenu);
    
    menu.addSeparator();
    menu.addItem(2, "Cut");
    menu.addItem(3, "Copy");
    menu.addItem(4, "Delete");
    
    // Position menu explicitly under the ellipsis
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(18);
    auto ellipsisBounds = headerBounds.withTrimmedLeft(headerBounds.getWidth() - 20);
    
    juce::PopupMenu::Options options;
    options = options.withTargetScreenArea(localAreaToGlobal(ellipsisBounds));
    
    menu.showMenuAsync(options, [this](int result) {
        if (result == 4) {
            engine.getTimelineProject().removeClip(clipData);
        }
    });
}

ClipComponent::~ClipComponent() {
    thumbnail.removeChangeListener(this);
}

void ClipComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &thumbnail) {
        repaint();
    }
}

void ClipComponent::paint(juce::Graphics& g) {
    bool isAudio = clipData->getType() == Clip::Type::Audio;
    
    int colorIndex = clipData->colorIndex.get();
    juce::String name = clipData->name.get();
    
    juce::Colour baseColour = getClipColor(colorIndex);
    bool isDark = baseColour.getPerceivedBrightness() < 0.5f;
    juce::Colour textColour = isDark ? juce::Colours::white : juce::Colours::black;
    
    auto bounds = getLocalBounds();
    
    auto headerBounds = bounds.removeFromTop(18);

    // Draw header
    g.setColour(baseColour.darker(0.2f));
    g.fillRect(headerBounds);
    
    // Draw Text
    g.setColour(textColour);
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
    g.drawText(" " + name, headerBounds.reduced(2, 0).withTrimmedRight(20), juce::Justification::centredLeft, true);
    
    // Draw ellipsis
    float cx = headerBounds.getRight() - 10.0f;
    float cy = headerBounds.getCentreY();
    g.setColour(textColour);
    g.fillEllipse(cx - 5.0f, cy - 1.5f, 3.0f, 3.0f);
    g.fillEllipse(cx, cy - 1.5f, 3.0f, 3.0f);
    g.fillEllipse(cx + 5.0f, cy - 1.5f, 3.0f, 3.0f);

    // Draw body
    g.setColour(baseColour.withAlpha(0.85f));
    g.fillRect(bounds);
    
    // Selection highlight
    if (engine.getTimelineProject().getSelectedClip() == clipData) {
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds(), 1);
    }

    // Draw waveform or MIDI
    g.setColour(juce::Colour(0xff212121)); // Audacity dark waveform
    
    if (isAudio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
        if (thumbnail.getTotalLength() > 0.0) {
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
            double startSecs = audioClip->sourceOffsetSamples.get() / sampleRate;
            double endSecs = startSecs + (audioClip->lengthSamples.get() / sampleRate);
            
            int numChannels = thumbnail.getNumChannels();
            if (numChannels == 2) {
                auto topHalf = bounds.removeFromTop(bounds.getHeight() / 2);
                thumbnail.drawChannel(g, topHalf.reduced(0, 1), startSecs, endSecs, 0, 1.0f);
                g.setColour(juce::Colours::black.withAlpha(0.2f));
                g.fillRect(bounds.getX(), bounds.getY() - 1, bounds.getWidth(), 1); // Divider
                g.setColour(juce::Colour(0xff212121));
                thumbnail.drawChannel(g, bounds.reduced(0, 1), startSecs, endSecs, 1, 1.0f);
            } else {
                thumbnail.drawChannels(g, bounds.reduced(0, 2), startSecs, endSecs, 1.0f);
            }
        } else {
            g.setFont(DesignSystem::Typography::getPrimaryFont());
            g.setColour(juce::Colours::white);
            g.drawText("Loading...", bounds, juce::Justification::centred, false);
        }
    } else if (clipData->getType() == Clip::Type::Midi) {
        auto midiClip = std::static_pointer_cast<MidiClip>(clipData);
        if (midiClip) {
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
            int range = juce::jmax(1, maxNote - minNote + 2);
            
            double clipSamples = midiClip->lengthSamples.get();
            double offsetSamples = midiClip->sourceOffsetSamples.get();
            
            for (int i = 0; i < midiClip->getSequence().getNumEvents(); ++i) {
                auto* event = midiClip->getSequence().getEventPointer(i);
                if (event->message.isNoteOn()) {
                    double noteStart = event->message.getTimeStamp() * 48000.0;
                    double noteLength = 48000.0 * 0.25;
                    
                    for (int j = i + 1; j < midiClip->getSequence().getNumEvents(); ++j) {
                        auto* offEvent = midiClip->getSequence().getEventPointer(j);
                        if (offEvent->message.isNoteOff() && offEvent->message.getNoteNumber() == event->message.getNoteNumber()) {
                            noteLength = offEvent->message.getTimeStamp() * 48000.0 - noteStart;
                            break;
                        }
                    }
                    
                    double drawStart = noteStart - offsetSamples;
                    if (drawStart + noteLength > 0 && drawStart < clipSamples && clipSamples > 0.0) {
                        float x = static_cast<float>((drawStart / clipSamples) * bounds.getWidth());
                        float w = static_cast<float>((noteLength / clipSamples) * bounds.getWidth());
                        float y = bounds.getY() + static_cast<float>(maxNote + 1 - event->message.getNoteNumber()) / static_cast<float>(range) * bounds.getHeight();
                        float h = juce::jmax(2.0f, bounds.getHeight() / static_cast<float>(range));
                        
                        float vel = event->message.getFloatVelocity();
                        g.setColour(juce::Colour(0xff212121).withAlpha(0.4f + 0.6f * vel));
                        
                        // Clip the drawing rect to bounds
                        juce::Rectangle<float> rect(x, y, juce::jmax(1.0f, w), h);
                        rect = rect.getIntersection(bounds.toFloat());
                        g.fillRect(rect);
                    }
                }
            }
        }
    }
}

void ClipComponent::resized() {
}

void ClipComponent::mouseDown(const juce::MouseEvent& event) {
    engine.getTimelineProject().setSelectedClip(clipData);
    
    if (event.y <= 18 && event.x >= getWidth() - 20) {
        showPropertiesMenu();
        return;
    }
    
    if (event.mods.isPopupMenu()) {
        if (auto* parent = dynamic_cast<Timeline::TrackLaneComponent*>(getParentComponent())) {
            parent->showContextMenuForClip(event.getEventRelativeTo(parent), clipData);
        }
        return;
    }

    dragStartX = event.getScreenX();
    
    originalStartSamples = clipData->startSample.get();
    originalLengthSamples = clipData->lengthSamples.get();
    originalSourceOffsetSamples = clipData->sourceOffsetSamples.get();
    
    bool isHeaderClick = event.y <= 18;
    isResizingLeft = (event.x <= 5);
    isResizingRight = (event.x >= getWidth() - 5);
    isDragging = isHeaderClick && !isResizingLeft && !isResizingRight;
    isSelectingTime = !isHeaderClick && !isResizingLeft && !isResizingRight;
    
    if (isSelectingTime) {
        if (auto* parent = getParentComponent()) {
            parent->mouseDown(event.getEventRelativeTo(parent));
        }
    }
}

void ClipComponent::mouseMove(const juce::MouseEvent& event) {
    if (event.x <= 5 || event.x >= getWidth() - 5) {
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    } else {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void ClipComponent::mouseDrag(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) return;
    
    if (isSelectingTime) {
        if (auto* parent = getParentComponent()) {
            parent->mouseDrag(event.getEventRelativeTo(parent));
        }
        return;
    }
    
    int deltaX = event.getScreenX() - dragStartX;
    
    double pixelsPerSecond = 100.0;
    auto* comp = getParentComponent();
    while (comp != nullptr) {
        if (auto* tc = dynamic_cast<TimelineComponent*>(comp)) {
            pixelsPerSecond = tc->getPixelsPerSecond();
            break;
        }
        comp = comp->getParentComponent();
    }
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double deltaSeconds = static_cast<double>(deltaX) / pixelsPerSecond;
    double deltaSamples = deltaSeconds * sampleRate;
    
    double tempo = engine.getTransport().getTempo();
    double secondsPerBeat = 60.0 / tempo;
    double snapSeconds = secondsPerBeat / 4.0;
    
    if (isResizingRight) {
        double newLengthSamples = originalLengthSamples + deltaSamples;
        double newLengthSeconds = newLengthSamples / sampleRate;
        double snappedLengthSeconds = std::round(newLengthSeconds / snapSeconds) * snapSeconds;
        newLengthSamples = juce::jmax(100.0, snappedLengthSeconds * sampleRate);
        
        clipData->lengthSamples = newLengthSamples;
        
    } else if (isResizingLeft) {
        double newStartSamples = originalStartSamples + deltaSamples;
        double newStartSeconds = newStartSamples / sampleRate;
        double snappedStartSeconds = std::round(newStartSeconds / snapSeconds) * snapSeconds;
        newStartSamples = juce::jmax(0.0, snappedStartSeconds * sampleRate);
        
        double actualDeltaSamples = newStartSamples - originalStartSamples;
        double newLengthSamples = originalLengthSamples - actualDeltaSamples;
        double newOffsetSamples = originalSourceOffsetSamples + actualDeltaSamples;
        
        if (newLengthSamples > 100.0 && newOffsetSamples >= 0.0) {
            clipData->startSample = newStartSamples;
            clipData->lengthSamples = newLengthSamples;
            clipData->sourceOffsetSamples = newOffsetSamples;
        }
    } else if (isDragging) {
        double newStartSamples = originalStartSamples + deltaSamples;
        double newStartSeconds = newStartSamples / sampleRate;
        double snappedStartSeconds = std::round(newStartSeconds / snapSeconds) * snapSeconds;
        newStartSamples = juce::jmax(0.0, snappedStartSeconds * sampleRate);
        
        clipData->startSample = newStartSamples;
    }
    
    if (auto* parent = getParentComponent()) {
        parent->resized();
    }
    repaint();
}

void ClipComponent::mouseUp(const juce::MouseEvent& event) {
    if (isSelectingTime) {
        if (auto* parent = getParentComponent()) {
            parent->mouseUp(event.getEventRelativeTo(parent));
        }
        isSelectingTime = false;
        return;
    }

    isDragging = false;
    isResizingLeft = false;
    isResizingRight = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void ClipComponent::mouseDoubleClick(const juce::MouseEvent& event) {}

} // namespace Nimbus::Timeline
