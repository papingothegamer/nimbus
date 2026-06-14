#include "TrackLaneComponent.h"
#include "UI/DesignSystem/Colors.h"

namespace Nimbus::Timeline {

TrackLaneComponent::TrackLaneComponent(NimbusEngine& e, int tIndex) : engine(e), trackIndex(tIndex) {
    updateClips();
}

TrackLaneComponent::~TrackLaneComponent() = default;

void TrackLaneComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::PanelBackground);
    
    // Bottom separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
}

void TrackLaneComponent::resized() {
    // Basic layout for demo: just place the clip filling the track
    if (clipComponents.size() > 0) {
        clipComponents[0]->setBounds(0, 0, getWidth(), getHeight());
    }
}

void TrackLaneComponent::updateClips() {
    clipComponents.clear();
    
    // Get clips for this track from the timeline project
    auto clips = engine.getTimelineProject().getClipsOnTrack(trackIndex);
    for (auto clip : clips) {
        auto* clipComp = new ClipComponent(clip, engine.getFormatManager());
        clipComponents.add(clipComp);
        addAndMakeVisible(clipComp);
    }
    
    resized();
}

} // namespace Nimbus::Timeline
