#include "NotesPanelComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

NotesPanelComponent::NotesPanelComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(DesignSystem::Typography::getSecondaryFont().withStyle(juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    
    addAndMakeVisible(reverseButton);
    addAndMakeVisible(invertButton);
    addAndMakeVisible(legatoButton);
    addAndMakeVisible(duplicateButton);
    
    reverseButton.onClick = [this] {
        if (!currentClip) return;
        auto& seq = currentClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        
        // Simple reverse: invert the start times of notes relative to the clip center
        double maxTime = 0.0;
        double minTime = 1e10;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                if (t > maxTime) maxTime = t;
                if (t < minTime) minTime = t;
            }
        }
        
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double t = evt->message.getTimeStamp();
                double newTime = maxTime - (t - minTime);
                evt->message.setTimeStamp(newTime);
                if (evt->noteOffObject) {
                    double length = evt->noteOffObject->message.getTimeStamp() - t;
                    evt->noteOffObject->message.setTimeStamp(newTime + length);
                }
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    invertButton.onClick = [this] {
        if (!currentClip) return;
        auto& seq = currentClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        
        int minPitch = 127;
        int maxPitch = 0;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                int p = evt->message.getNoteNumber();
                if (p < minPitch) minPitch = p;
                if (p > maxPitch) maxPitch = p;
            }
        }
        int center = minPitch + (maxPitch - minPitch) / 2;
        
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                int p = evt->message.getNoteNumber();
                int newPitch = center - (p - center);
                newPitch = juce::jlimit(0, 127, newPitch);
                evt->message.setNoteNumber(newPitch);
                if (evt->noteOffObject) {
                    evt->noteOffObject->message.setNoteNumber(newPitch);
                }
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    legatoButton.onClick = [this] {
        if (!currentClip) return;
        auto& seq = currentClip->getSequence();
        
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                double nextTime = currentClip->getLengthSamples();
                // Find next note
                for (int j = 0; j < seq.getNumEvents(); ++j) {
                    auto* nextEvt = seq.getEventPointer(j);
                    if (nextEvt->message.isNoteOn() && nextEvt->message.getTimeStamp() > evt->message.getTimeStamp()) {
                        nextTime = std::min(nextTime, nextEvt->message.getTimeStamp());
                    }
                }
                if (evt->noteOffObject) {
                    evt->noteOffObject->message.setTimeStamp(nextTime);
                }
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
    
    duplicateButton.onClick = [this] {
        if (!currentClip) return;
        auto& seq = currentClip->getSequence();
        if (seq.getNumEvents() == 0) return;
        
        double maxTime = 0.0;
        for (int i = 0; i < seq.getNumEvents(); ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOff()) {
                maxTime = std::max(maxTime, evt->message.getTimeStamp());
            }
        }
        
        int originalEvents = seq.getNumEvents();
        for (int i = 0; i < originalEvents; ++i) {
            auto* evt = seq.getEventPointer(i);
            if (evt->message.isNoteOn()) {
                juce::MidiMessage newOn = evt->message;
                newOn.setTimeStamp(newOn.getTimeStamp() + maxTime);
                
                juce::MidiMessage newOff = juce::MidiMessage::noteOff(newOn.getChannel(), newOn.getNoteNumber(), (juce::uint8)0);
                if (evt->noteOffObject) {
                    newOff.setTimeStamp(evt->noteOffObject->message.getTimeStamp() + maxTime);
                } else {
                    newOff.setTimeStamp(newOn.getTimeStamp() + 48000.0 * 0.25);
                }
                
                seq.addEvent(newOn);
                seq.addEvent(newOff);
            }
        }
        seq.updateMatchedPairs();
        engine.getTimelineProject().notifyClipModified();
    };
}

void NotesPanelComponent::setMidiClip(std::shared_ptr<MidiClip> clip) {
    currentClip = clip;
}

NotesPanelComponent::~NotesPanelComponent() = default;

void NotesPanelComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground.brighter(0.02f));
    
    g.setColour(DesignSystem::Colors::Divider);
    g.drawRect(getLocalBounds(), 1);
}

void NotesPanelComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    
    titleLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(8); // Spacer
    
    int btnHeight = 22;
    
    auto row1 = area.removeFromTop(btnHeight);
    reverseButton.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(2));
    invertButton.setBounds(row1.reduced(2));
    
    auto row2 = area.removeFromTop(btnHeight);
    legatoButton.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2));
    duplicateButton.setBounds(row2.reduced(2));
}

} // namespace Nimbus::DetailView
