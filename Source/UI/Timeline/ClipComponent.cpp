#include "ClipComponent.h"
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
}

} // namespace Nimbus::Timeline
