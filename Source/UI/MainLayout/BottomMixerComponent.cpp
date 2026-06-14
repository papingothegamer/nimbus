#include "BottomMixerComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

BottomMixerComponent::BottomMixerComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(DesignSystem::Typography::getTitleFont());
    titleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextTitle);

    engine.getTimelineProject().addListener(this);

    masterStrip = std::make_unique<ChannelStripComponent>("MASTER", true, true);
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
    auto* strip = new ChannelStripComponent(track.name, false, !track.isMidi);
    strip->setLevelProvider([this, trackIndex]() { return engine.getTrackPeakLevel(trackIndex); });
    trackStrips.add(strip);
    addAndMakeVisible(strip);
    resized();
}

void BottomMixerComponent::trackRemoved(int trackIndex) {
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
    
    for (auto* strip : trackStrips) {
        strip->setBounds(bounds.removeFromLeft(stripWidth));
    }
    
    // Put master at the far right
    auto rightBounds = getLocalBounds().withTrimmedTop(30).withTrimmedRight(10).removeFromRight(80);
    masterStrip->setBounds(rightBounds);
}

} // namespace Nimbus::MainLayout
