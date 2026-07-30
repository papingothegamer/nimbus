#include "ClipComponent.h"
#include "TimelineComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::Timeline {

juce::Colour ClipComponent::getClipColor(int index) {
    if (index < 0) return juce::Colour(0xff0a84ff);
    float hue = std::fmod(index * 0.381966f, 1.0f);
    return juce::Colour::fromHSV(hue, 0.6f, 0.95f, 1.0f); // Bright enough to contrast with black, no grey/black
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
    : engine(e), clipData(clip), thumbnail(engine.getFormatManager(), engine.getThumbnailCache(), engine.getBackgroundThread()) {
    
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

    // Header bottom border
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawHorizontalLine(headerBounds.getBottom() - 1, headerBounds.getX(), headerBounds.getRight());

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
    
    auto waveBounds = bounds; // Copy bounds so we don't mutate the original
    
    if (isAudio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
        if (thumbnail.getTotalLength() > 0.0) {
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0) sampleRate = 48000.0;
            
            double startSecs = audioClip->sourceOffsetSamples.get() / sampleRate;
            double endSecs = startSecs + (audioClip->lengthSamples.get() / sampleRate);
            
            int numChannels = thumbnail.getNumChannels();
            if (numChannels == 2) {
                auto topHalf = waveBounds.removeFromTop(waveBounds.getHeight() / 2);
                thumbnail.drawChannel(g, topHalf.reduced(0, 1), startSecs, endSecs, 0, 1.0f);
                g.setColour(juce::Colours::black.withAlpha(0.2f));
                g.fillRect(waveBounds.getX(), waveBounds.getY() - 1, waveBounds.getWidth(), 1); // Divider
                g.setColour(juce::Colour(0xff212121));
                thumbnail.drawChannel(g, waveBounds.reduced(0, 1), startSecs, endSecs, 1, 1.0f);
            } else {
                thumbnail.drawChannels(g, waveBounds.reduced(0, 2), startSecs, endSecs, 1.0f);
            }
        } else {
            g.setFont(DesignSystem::Typography::getPrimaryFont());
            g.setColour(juce::Colours::white);
            g.drawText("Loading...", bounds, juce::Justification::centred, false);
        }
        
        // --- Draw Fades ---
        double sampleRate = engine.getTransport().getSampleRate();
        if (sampleRate <= 0) sampleRate = 48000.0;
        
        double lengthSamples = audioClip->lengthSamples.get();
        if (lengthSamples > 0) {
            float inWidth = static_cast<float>((audioClip->fadeInSamples.get() / lengthSamples) * bounds.getWidth());
            float outWidth = static_cast<float>((audioClip->fadeOutSamples.get() / lengthSamples) * bounds.getWidth());
            float inCurve = audioClip->fadeInCurve.get();
            float outCurve = audioClip->fadeOutCurve.get();
            float height = bounds.getHeight();
            
            float handleSize = 6.0f;
            float handleOffset = handleSize / 2.0f;
            
            // --- Fade In ---
            if (inWidth > 0.0f) {
                juce::Path inPath;
                inPath.startNewSubPath(bounds.getX(), bounds.getY());
                inPath.lineTo(bounds.getX() + inWidth, bounds.getY());
                for (float x = inWidth; x >= 0.0f; x -= 1.0f) {
                    float t = x / inWidth;
                    float y = bounds.getBottom() - (std::pow(t, inCurve) * height);
                    inPath.lineTo(bounds.getX() + x, y);
                }
                inPath.closeSubPath();
                
                g.setColour(juce::Colours::black.withAlpha(0.4f));
                g.fillPath(inPath);
                
                if (inWidth > 2.0f) {
                    juce::Path strokePath;
                    strokePath.startNewSubPath(bounds.getX(), bounds.getBottom());
                    for (float x = 0; x <= inWidth; x += 1.0f) {
                        float t = x / inWidth;
                        float y = bounds.getBottom() - (std::pow(t, inCurve) * height);
                        strokePath.lineTo(bounds.getX() + x, y);
                    }
                    g.setColour(juce::Colours::black);
                    g.strokePath(strokePath, juce::PathStrokeType(1.0f));
                }
            }
            
            // --- Fade Out ---
            if (outWidth > 0.0f) {
                float startX = bounds.getRight() - outWidth;
                juce::Path outPath;
                outPath.startNewSubPath(bounds.getRight(), bounds.getY());
                outPath.lineTo(startX, bounds.getY());
                for (float x = 0; x <= outWidth; x += 1.0f) {
                    float t = x / outWidth;
                    float y = bounds.getBottom() - (std::pow(1.0f - t, outCurve) * height);
                    outPath.lineTo(startX + x, y);
                }
                outPath.lineTo(bounds.getRight(), bounds.getBottom());
                outPath.closeSubPath();
                
                g.setColour(juce::Colours::black.withAlpha(0.4f));
                g.fillPath(outPath);
                
                if (outWidth > 2.0f) {
                    juce::Path strokePath;
                    strokePath.startNewSubPath(startX, bounds.getY());
                    for (float x = 0; x <= outWidth; x += 1.0f) {
                        float t = x / outWidth;
                        float y = bounds.getBottom() - (std::pow(1.0f - t, outCurve) * height);
                        strokePath.lineTo(startX + x, y);
                    }
                    g.setColour(juce::Colours::black);
                    g.strokePath(strokePath, juce::PathStrokeType(1.0f));
                }
            }
                
            // --- Handles ---
            bool showHandles = isMouseOver(true) || isMouseButtonDown();
            if (showHandles) {
                // Determine hovers (approximation based on current state)
                auto mousePos = getMouseXYRelative();
                bool hoverInDur = isDraggingFadeIn || (std::abs(mousePos.x - inWidth) <= 12 && std::abs(mousePos.y - 18.0f) <= 12);
                float startXOut = bounds.getRight() - outWidth;
                bool hoverOutDur = isDraggingFadeOut || (std::abs(mousePos.x - startXOut) <= 12 && std::abs(mousePos.y - 18.0f) <= 12);

                // Fade In Duration Handle (Top Left)
                float durXIn = juce::jlimit(0.0f, (float)bounds.getWidth() - handleSize, bounds.getX() + inWidth - handleOffset);
                float durYIn = bounds.getY();
                
                g.setColour(hoverInDur ? juce::Colours::white : juce::Colours::black);
                g.fillRect(durXIn, durYIn, handleSize, handleSize);
                g.setColour(juce::Colours::white);
                g.drawRect(durXIn, durYIn, handleSize, handleSize, hoverInDur ? 2.0f : 1.0f);
                
                // Fade In Curve Handle
                if (inWidth >= 10.0f) {
                    float tMid = 0.5f;
                    float cx = inWidth * tMid;
                    float cy = bounds.getBottom() - (std::pow(tMid, inCurve) * height);
                    float curveX = juce::jlimit(0.0f, (float)bounds.getWidth() - handleSize, bounds.getX() + cx - handleOffset);
                    float curveY = juce::jlimit((float)bounds.getY(), bounds.getBottom() - handleSize, cy - handleOffset);
                    bool hoverInCurve = isDraggingFadeInCurve || (std::abs(mousePos.x - cx) <= 12 && std::abs(mousePos.y - (cy + 18.0f)) <= 12);
                    
                    g.setColour(hoverInCurve ? juce::Colours::white : juce::Colours::black);
                    g.fillRect(curveX, curveY, handleSize, handleSize);
                    g.setColour(juce::Colours::white);
                    g.drawRect(curveX, curveY, handleSize, handleSize, hoverInCurve ? 2.0f : 1.0f);
                }
                
                // Fade Out Duration Handle (Top Right)
                float durXOut = juce::jlimit(0.0f, (float)bounds.getWidth() - handleSize, startXOut - handleOffset);
                float durYOut = bounds.getY();
                
                g.setColour(hoverOutDur ? juce::Colours::white : juce::Colours::black);
                g.fillRect(durXOut, durYOut, handleSize, handleSize);
                g.setColour(juce::Colours::white);
                g.drawRect(durXOut, durYOut, handleSize, handleSize, hoverOutDur ? 2.0f : 1.0f);
                
                // Fade Out Curve Handle
                if (outWidth >= 10.0f) {
                    float tMid = 0.5f;
                    float cx = outWidth * tMid;
                    float cy = bounds.getBottom() - (std::pow(1.0f - tMid, outCurve) * height);
                    float curveX = juce::jlimit(0.0f, (float)bounds.getWidth() - handleSize, bounds.getX() + startXOut + cx - handleOffset);
                    float curveY = juce::jlimit((float)bounds.getY(), bounds.getBottom() - handleSize, cy - handleOffset);
                    bool hoverOutCurve = isDraggingFadeOutCurve || (std::abs(mousePos.x - (startXOut + cx)) <= 12 && std::abs(mousePos.y - (cy + 18.0f)) <= 12);
                    
                    g.setColour(hoverOutCurve ? juce::Colours::white : juce::Colours::black);
                    g.fillRect(curveX, curveY, handleSize, handleSize);
                    g.setColour(juce::Colours::white);
                    g.drawRect(curveX, curveY, handleSize, handleSize, hoverOutCurve ? 2.0f : 1.0f);
                }
            }
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
                    double noteStart = event->message.getTimeStamp();
                    double noteLength = 48000.0 * 0.25;
                    
                    for (int j = i + 1; j < midiClip->getSequence().getNumEvents(); ++j) {
                        auto* offEvent = midiClip->getSequence().getEventPointer(j);
                        if (offEvent->message.isNoteOff() && offEvent->message.getNoteNumber() == event->message.getNoteNumber()) {
                            noteLength = offEvent->message.getTimeStamp() - noteStart;
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
    dragStartY = event.getScreenY();
    
    originalStartSamples = clipData->startSample.get();
    originalLengthSamples = clipData->lengthSamples.get();
    originalSourceOffsetSamples = clipData->sourceOffsetSamples.get();
    
    isDraggingFadeIn = false;
    isDraggingFadeOut = false;
    isDraggingFadeInCurve = false;
    isDraggingFadeOutCurve = false;

    if (clipData->getType() == Clip::Type::Audio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
        double lengthSamples = audioClip->lengthSamples.get();
        if (lengthSamples > 0) {
            float inWidth = static_cast<float>((audioClip->fadeInSamples.get() / lengthSamples) * getWidth());
            float outWidth = static_cast<float>((audioClip->fadeOutSamples.get() / lengthSamples) * getWidth());
            float waveHeight = getHeight() - 18.0f;
            float waveBottom = getHeight();
            
            // --- Fade In ---
            // Duration Handle Hover
            if (std::abs(event.x - inWidth) <= 12 && std::abs(event.y - 18.0f) <= 12) {
                isDraggingFadeIn = true;
                dragStartFadeInSamples = audioClip->fadeInSamples.get();
                return;
            }
            if (inWidth >= 10.0f) {
                // Curve Handle Hover
                float inCurve = audioClip->fadeInCurve.get();
                float xMidIn = inWidth * 0.5f;
                float yMidIn = waveBottom - (std::pow(0.5f, inCurve) * waveHeight);
                if (std::abs(event.x - xMidIn) <= 12 && std::abs(event.y - yMidIn) <= 12) {
                    isDraggingFadeInCurve = true;
                    dragStartFadeInCurve = inCurve;
                    return;
                }
            }
            
            // --- Fade Out ---
            // Duration Handle Hover
            float startXOut = getWidth() - outWidth;
            if (std::abs(event.x - startXOut) <= 12 && std::abs(event.y - 18.0f) <= 12) {
                isDraggingFadeOut = true;
                dragStartFadeOutSamples = audioClip->fadeOutSamples.get();
                return;
            }
            if (outWidth >= 10.0f) {
                // Curve Handle Hover
                float outCurve = audioClip->fadeOutCurve.get();
                float xMidOut = outWidth * 0.5f;
                float yMidOut = waveBottom - (std::pow(0.5f, outCurve) * waveHeight);
                if (std::abs(event.x - (startXOut + xMidOut)) <= 12 && std::abs(event.y - yMidOut) <= 12) {
                    isDraggingFadeOutCurve = true;
                    dragStartFadeOutCurve = outCurve;
                    return;
                }
            }
        }
    }
    
    bool isHeaderClick = event.y <= 18;
    isResizingLeft = (event.x <= 5);
    isResizingRight = (event.x >= getWidth() - 5);
    isDragging = !isResizingLeft && !isResizingRight;
    isSelectingTime = false;
    
    if (isSelectingTime) {
        if (auto* parent = getParentComponent()) {
            parent->mouseDown(event.getEventRelativeTo(parent));
        }
    }
}

void ClipComponent::mouseEnter(const juce::MouseEvent& event) {
    if (clipData->getType() == Clip::Type::Audio) {
        isHoveringEdges = true;
        repaint();
    }
}

void ClipComponent::mouseExit(const juce::MouseEvent& event) {
    if (isHoveringEdges) {
        isHoveringEdges = false;
        repaint();
    }
    if (clipData->getType() == Clip::Type::Audio) {
        repaint();
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void ClipComponent::mouseMove(const juce::MouseEvent& event) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
    
    if (clipData->getType() == Clip::Type::Audio) {
        auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
        double lengthSamples = audioClip->lengthSamples.get();
        if (lengthSamples > 0) {
            float inWidth = static_cast<float>((audioClip->fadeInSamples.get() / lengthSamples) * getWidth());
            float outWidth = static_cast<float>((audioClip->fadeOutSamples.get() / lengthSamples) * getWidth());
            float waveHeight = getHeight() - 18.0f;
            float waveBottom = getHeight();
            
            // --- Fade In Hit Testing ---
            if (std::abs(event.x - inWidth) <= 12 && std::abs(event.y - 18.0f) <= 12) {
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                return;
            }
            if (inWidth >= 10.0f) {
                float inCurve = audioClip->fadeInCurve.get();
                float xMidIn = inWidth * 0.5f;
                float yMidIn = waveBottom - (std::pow(0.5f, inCurve) * waveHeight);
                if (std::abs(event.x - xMidIn) <= 12 && std::abs(event.y - yMidIn) <= 12) {
                    setMouseCursor(juce::MouseCursor::PointingHandCursor);
                    return;
                }
            }
            
            // --- Fade Out Hit Testing ---
            float startXOut = getWidth() - outWidth;
            if (std::abs(event.x - startXOut) <= 12 && std::abs(event.y - 18.0f) <= 12) {
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
                return;
            }
            if (outWidth >= 10.0f) {
                float outCurve = audioClip->fadeOutCurve.get();
                float xMidOut = outWidth * 0.5f;
                float yMidOut = waveBottom - (std::pow(0.5f, outCurve) * waveHeight);
                if (std::abs(event.x - (startXOut + xMidOut)) <= 12 && std::abs(event.y - yMidOut) <= 12) {
                    setMouseCursor(juce::MouseCursor::PointingHandCursor);
                    return;
                }
            }
        }
    }
    
    bool isHeaderHover = event.y <= 18;
    bool isLeftEdge = (event.x <= 5);
    bool isRightEdge = (event.x >= getWidth() - 5);
    
    if (!isHeaderHover && (isLeftEdge || isRightEdge)) {
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
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
    
    if (isDraggingFadeIn || isDraggingFadeOut || isDraggingFadeInCurve || isDraggingFadeOutCurve) {
        if (clipData->getType() == Clip::Type::Audio) {
            auto audioClip = std::static_pointer_cast<AudioClip>(clipData);
            
            if (isDraggingFadeIn) {
                double newSamples = dragStartFadeInSamples + deltaSamples;
                newSamples = juce::jlimit(192.0, static_cast<double>(audioClip->lengthSamples.get()), newSamples);
                audioClip->fadeInSamples = static_cast<int>(newSamples);
            } else if (isDraggingFadeOut) {
                double newSamples = dragStartFadeOutSamples - deltaSamples;
                newSamples = juce::jlimit(192.0, static_cast<double>(audioClip->lengthSamples.get()), newSamples);
                audioClip->fadeOutSamples = static_cast<int>(newSamples);
            } else if (isDraggingFadeInCurve) {
                int deltaY = event.getScreenY() - dragStartY;
                float newCurve = dragStartFadeInCurve + (deltaY * 0.05f); // drag UP = negative deltaY = lower exponent = handle moves UP
                audioClip->fadeInCurve = juce::jlimit(0.1f, 4.0f, newCurve);
            } else if (isDraggingFadeOutCurve) {
                int deltaY = event.getScreenY() - dragStartY;
                float newCurve = dragStartFadeOutCurve + (deltaY * 0.05f);
                audioClip->fadeOutCurve = juce::jlimit(0.1f, 4.0f, newCurve);
            }
            repaint();
        }
        return;
    }
    
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

    bool wasFadeDrag = isDraggingFadeIn || isDraggingFadeOut || isDraggingFadeInCurve || isDraggingFadeOutCurve;

    isDragging = false;
    isResizingLeft = false;
    isResizingRight = false;
    isDraggingFadeIn = false;
    isDraggingFadeOut = false;
    isDraggingFadeInCurve = false;
    isDraggingFadeOutCurve = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    
    // Notify listeners after fade changes are complete (NOT during drag)
    if (wasFadeDrag) {
        repaint();
    }

    // Resolve crossfades on mouse drop
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        auto clips = engine.getTimelineProject().getClipsOnTrack(i);
        if (std::find(clips.begin(), clips.end(), clipData) != clips.end()) {
            engine.getTimelineProject().resolveCrossfades(i);
            break;
        }
    }
}

void ClipComponent::mouseDoubleClick(const juce::MouseEvent& event) {}

} // namespace Nimbus::Timeline
