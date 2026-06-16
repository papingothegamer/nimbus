#include "BottomMixerComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

BottomMixerComponent::BottomMixerComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);

    engine.getTimelineProject().addListener(this);

    // Initial tracks
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        trackAdded(i, engine.getTimelineProject().getTrack(i));
    }

    // Master strip
    masterStrip = std::make_unique<ChannelStripComponent>(engine, "Master", true, true);
    masterStrip->setLevelProvider([this]() { return engine.getMasterPeakLevel(); });
    addAndMakeVisible(masterStrip.get());

    // Load existing tracks
    for (int i = 0; i < engine.getTimelineProject().getNumTracks(); ++i) {
        trackAdded(i, engine.getTimelineProject().getTrack(i));
    }
}

BottomMixerComponent::~BottomMixerComponent() {
    engine.getTimelineProject().removeListener(this);
}

void BottomMixerComponent::trackAdded(int trackIndex, const TrackModel& track) {
    auto* strip = new ChannelStripComponent(engine, track.name, !track.isMidi, false);
    strip->setTrackIndex(trackIndex);
    strip->setLevelProvider([this, trackIndex]() { return engine.getTrackPeakLevel(trackIndex); });
    trackStrips.add(strip);
    addAndMakeVisible(strip);
    resized();
}

void BottomMixerComponent::trackRemoved(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < trackStrips.size()) {
        auto* strip = trackStrips.removeAndReturn(trackIndex);
        delete strip;
        
        for (int i = trackIndex; i < trackStrips.size(); ++i) {
            trackStrips[i]->setTrackIndex(i);
        }
        resized();
    }
}

void BottomMixerComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Top border
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, 0, getWidth(), 1);
}

void BottomMixerComponent::resized() {
    titleLabel.setBounds(4, 4, 100, 20);

    auto bounds = getLocalBounds().withTrimmedTop(30).withTrimmedLeft(10);
    
    int stripWidth = 80;
    
    for (int i = 0; i < trackStrips.size(); ++i) {
        auto* strip = trackStrips[i];
        
        bool isHidden = false;
        if (i < engine.getTimelineProject().getNumTracks()) {
            const auto& track = engine.getTimelineProject().getTrack(i);
            
            // Hide if parent group is folded
            if (!track.parentGroupId.isNull()) {
                for (int j = 0; j < engine.getTimelineProject().getNumTracks(); ++j) {
                    const auto& parentTrack = engine.getTimelineProject().getTrack(j);
                    if (parentTrack.id == track.parentGroupId) {
                        if (parentTrack.isFolded) {
                            isHidden = true;
                        }
                        break;
                    }
                }
            }
        }
        
        if (isHidden) {
            strip->setVisible(false);
            strip->setBounds(0, 0, 0, 0);
        } else {
            strip->setVisible(true);
            strip->setBounds(bounds.removeFromLeft(stripWidth));
        }
    }
    
    // Put master at the far right
    auto rightBounds = getLocalBounds().withTrimmedTop(30).withTrimmedRight(10).removeFromRight(80);
    masterStrip->setBounds(rightBounds);
}

} // namespace Nimbus::MainLayout
