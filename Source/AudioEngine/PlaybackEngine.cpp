#include "PlaybackEngine.h"
#include "TrackSourceNode.h"

#include "Core/NimbusEngine.h"

namespace Nimbus {

PlaybackEngine::PlaybackEngine(NimbusEngine& eng, TimelineProject& proj, Mixer& mix, PluginManager& pm)
    : engine(eng), project(proj), mixer(mix), pluginManager(pm) {
    project.addListener(this);
}

PlaybackEngine::~PlaybackEngine() {
    project.removeListener(this);
}

void PlaybackEngine::trackAdded(int trackIndex, const TrackModel& track) {
    if (track.isGroup) return; // Group tracks don't have audio nodes currently
    
    auto newTrack = std::make_unique<Track>(track.id, track.isStereo, &engine.getTransport());
    newTrack->setSourceNode(std::make_unique<TrackSourceNode>(engine.getTransport(), engine.getFormatManager()));
    // newTrack->setMuted(track.isMuted); // etc
    trackNodes[track.id.toString()] = newTrack.get();
    
    mixer.addTrack(std::move(newTrack));
}

void PlaybackEngine::trackRemoved(int trackIndex) {
    if (auto* mixerTrack = mixer.getTrack(trackIndex)) {
        trackNodes.erase(mixerTrack->getId().toString());
    }
    mixer.removeTrack(trackIndex);
}

void PlaybackEngine::trackMuteChanged(int trackIndex, bool isMuted) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setMuted(isMuted);
}

void PlaybackEngine::trackSoloChanged(int trackIndex, bool isSoloed) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setSoloed(isSoloed);
}

void PlaybackEngine::trackArmChanged(int trackIndex, bool isArmed) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setArmed(isArmed);
}

void PlaybackEngine::trackVolumeChanged(int trackIndex, float volume) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setVolume(volume);
}

void PlaybackEngine::trackPanChanged(int trackIndex, float pan) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setPan(pan);
}

void PlaybackEngine::trackInputChannelChanged(int trackIndex, int inputChannel) {
    if (auto* t = mixer.getTrack(trackIndex)) t->setInputChannelIndex(inputChannel);
}

void PlaybackEngine::trackClipsChanged(int trackIndex) {
    const auto& track = project.getTrack(trackIndex);
    if (auto* audioTrack = getAudioTrack(track.id)) {
        if (auto* sourceNode = dynamic_cast<TrackSourceNode*>(audioTrack->getSourceNode())) {
            sourceNode->updateClips(project.getClipsOnTrack(trackIndex));
        }
    }
}

Track* PlaybackEngine::getAudioTrack(const TrackID& trackId) {
    auto it = trackNodes.find(trackId.toString());
    if (it != trackNodes.end()) return it->second;
    return nullptr;
}

} // namespace Nimbus
