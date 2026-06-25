#include "TimelineComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::Timeline {

TrackLaneComponent::TrackLaneComponent(NimbusEngine& e, Nimbus::TimelineComponent& t, int tIndex) 
    : engine(e), timeline(t), trackIndex(tIndex) {
    engine.getTimelineProject().addListener(this);
    updateClips();
}

TrackLaneComponent::~TrackLaneComponent() {
    engine.getTimelineProject().removeListener(this);
}

void TrackLaneComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    // If selected, highlight
    if (engine.getTimelineProject().isTrackSelected(trackIndex)) {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRect(getLocalBounds());
    }
    
    // Recording indicator
    if (engine.getTransport().isRecording() && engine.getTimelineProject().isTrackArmed(trackIndex)) {
        g.setColour(juce::Colours::red.withAlpha(0.15f));
        g.fillRect(getLocalBounds());
    }
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}


void TrackLaneComponent::mouseDoubleClick(const juce::MouseEvent& event) {
    if (trackIndex < engine.getTimelineProject().getNumTracks()) {
        const auto& track = engine.getTimelineProject().getTrack(trackIndex);
        if (track.isMidi) {
            // Adaptive grid 1/16th note default
            // Snap double-click position to nearest 1/16th beat or beat
            // For now, let's create a 1-bar clip (4 beats in 4/4)
            double tempo = engine.getTransport().getTempo();
            double secondsPerBeat = 60.0 / tempo;
            double sampleRate = engine.getTransport().getSampleRate();
            if (sampleRate <= 0.0) sampleRate = 48000.0;
            double clipLengthSamples = (secondsPerBeat * 4.0) * sampleRate;

            double pixelsPerSecond = timeline.getPixelsPerSecond();
            double scrollX = timeline.getScrollOffsetX();
            double clickSeconds = (event.position.x + scrollX) / pixelsPerSecond;
            double snappedSeconds = std::round(clickSeconds / secondsPerBeat) * secondsPerBeat;
            double startSamples = snappedSeconds * sampleRate;

            auto midiClip = std::make_shared<MidiClip>(startSamples, clipLengthSamples);
            // Empty clip, no notes!
            
            engine.getTimelineProject().addClipToTrack(trackIndex, midiClip);
        }
    }
}

void TrackLaneComponent::trackClipsChanged(int changedTrackIndex) {
    if (changedTrackIndex == trackIndex) {
        updateClips();
    }
}

void TrackLaneComponent::resized() {
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0) sampleRate = 48000.0;
    
    double pixelsPerSecond = timeline.getPixelsPerSecond();
    double scrollX = timeline.getScrollOffsetX();
    
    for (auto* clipComp : clipComponents) {
        auto clip = clipComp->getClip();
        
        double startSamples = 0.0;
        double lengthSamples = 0.0;
        
        if (std::holds_alternative<std::shared_ptr<MidiClip>>(clip)) {
            if (auto mc = std::get<std::shared_ptr<MidiClip>>(clip)) {
                startSamples = mc->getStartSample();
                lengthSamples = mc->getLengthSamples();
            }
        } else if (std::holds_alternative<std::shared_ptr<AudioClip>>(clip)) {
            if (auto ac = std::get<std::shared_ptr<AudioClip>>(clip)) {
                startSamples = ac->getStartSample();
                lengthSamples = ac->getLengthSamples();
            }
        }
        
        double startSeconds = startSamples / sampleRate;
        double lengthSeconds = lengthSamples / sampleRate;
        
        int x = static_cast<int>(startSeconds * pixelsPerSecond - scrollX);
        int w = static_cast<int>(lengthSeconds * pixelsPerSecond);
        
        clipComp->setBounds(x, 0, w, getHeight());
    }
}

void TrackLaneComponent::updateClips() {
    clipComponents.clear();
    
    // Get clips for this track from the timeline project
    auto clips = engine.getTimelineProject().getClipsOnTrack(trackIndex);
    for (auto clip : clips) {
        auto* clipComp = new ClipComponent(clip, engine);
        clipComponents.add(clipComp);
        addAndMakeVisible(clipComp);
    }
    
    resized();
}

void TrackLaneComponent::setTrackIndex(int newIndex) {
    trackIndex = newIndex;
}

void TrackLaneComponent::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        showContextMenu(event);
    } else {
        // Handle normal click (e.g. selection)
    }
}

void TrackLaneComponent::showContextMenu(const juce::MouseEvent& event) {
    juce::PopupMenu menu;
    
    menu.addItem(1, "Cut", true, false);
    menu.addItem(2, "Copy", true, false);
    menu.addItem(3, "Duplicate", true, false);
    menu.addItem(4, "Delete", true, false);
    menu.addSeparator();
    menu.addItem(5, "Zoom to Time Selection");
    menu.addItem(6, "Zoom Back from Time Selection");
    menu.addSeparator();
    menu.addItem(7, "Consolidate");
    menu.addItem(8, "Consolidate Time to New Scene");
    menu.addItem(9, "Insert Empty MIDI Clip(s)");
    menu.addItem(10, "Loop Selection");
    menu.addItem(11, "Freeze Track");
    menu.addSeparator();
    
    juce::PopupMenu gridMenu;
    gridMenu.addItem(12, "Widest");
    gridMenu.addItem(13, "Wide");
    gridMenu.addItem(14, "Medium");
    gridMenu.addItem(15, "Narrow");
    gridMenu.addItem(16, "Narrowest");
    menu.addSubMenu("Adaptive Grid", gridMenu);
    
    juce::PopupMenu fixedGridMenu;
    fixedGridMenu.addItem(17, "8 Bars");
    fixedGridMenu.addItem(18, "4 Bars");
    fixedGridMenu.addItem(19, "2 Bars");
    fixedGridMenu.addItem(20, "1 Bar");
    fixedGridMenu.addItem(21, "1/2");
    fixedGridMenu.addItem(22, "1/4");
    fixedGridMenu.addItem(23, "1/8");
    fixedGridMenu.addItem(24, "1/16");
    fixedGridMenu.addItem(25, "1/32");
    menu.addSubMenu("Fixed Grid", fixedGridMenu);
    
    menu.addItem(26, "Off");
    menu.addItem(27, "Triplet Grid");
    
    float clickX = event.position.x;
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMousePosition(),
        [this, clickX](int result) {
            if (result == 9) { // Insert Empty MIDI Clip
                double tempo = engine.getTransport().getTempo();
                double secondsPerBeat = 60.0 / tempo;
                double sampleRate = engine.getTransport().getSampleRate();
                if (sampleRate <= 0) sampleRate = 48000.0;
                
                double pixelsPerSecond = timeline.getPixelsPerSecond();
                double scrollX = timeline.getScrollOffsetX();
                
                double clickSeconds = (clickX + scrollX) / pixelsPerSecond;
                double snappedSeconds = std::round(clickSeconds / secondsPerBeat) * secondsPerBeat;
                
                double startSamples = snappedSeconds * sampleRate;
                double lengthSamples = (secondsPerBeat * 4.0) * sampleRate; // 1 bar
                
                auto clip = std::make_shared<MidiClip>(startSamples, lengthSamples);
                engine.getTimelineProject().addClipToTrack(trackIndex, clip);
            }
        });
}

} // namespace Nimbus::Timeline
