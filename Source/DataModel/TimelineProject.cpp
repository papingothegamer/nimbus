#include "TimelineProject.h"
#include "../Core/Plugins/Plugin.h"

namespace Nimbus {

TimelineProject::TimelineProject()
    : state("PROJECT")
{
    state.addListener(this);
}

TimelineProject::~TimelineProject()
{
    state.removeListener(this);
}

juce::ValueTree TimelineProject::getTrackTree(int index) const
{
    if (index >= 0 && index < state.getNumChildren()) {
        auto child = state.getChild(index);
        if (child.hasType("TRACK")) return child;
        // Need to skip MARKERS node if it's there
        int trackCount = 0;
        for (auto c : state) {
            if (c.hasType("TRACK")) {
                if (trackCount == index) return c;
                trackCount++;
            }
        }
    }
    return juce::ValueTree();
}

TrackModel TimelineProject::trackModelFromTree(const juce::ValueTree& vt) const
{
    TrackModel track;
    if (!vt.isValid()) return track;

    track.name = vt.getProperty("name", "Track").toString();
    track.isMuted = vt.getProperty("mute", false);
    track.isSoloed = vt.getProperty("solo", false);
    track.isArmed = vt.getProperty("arm", false);
    track.isStereo = vt.getProperty("stereo", true);
    track.isGroup = vt.getProperty("group", false);
    track.isFolded = vt.getProperty("folded", false);
    track.volume = vt.getProperty("volume", 1.0f);
    track.pan = vt.getProperty("pan", 0.0f);
    track.inputChannelIndex = vt.getProperty("inputChannel", -1);
    
    // Legacy mapping
    if (vt.getProperty("midi", false)) {
        track.type = TrackType::Midi;
        track.isMidi = true;
    } else {
        track.type = TrackType::Audio;
    }

    return track;
}

void TimelineProject::addTrack(const TrackModel& trackModel)
{
    juce::ValueTree vt("TRACK");
    vt.setProperty("name", trackModel.name, &undoManager);
    vt.setProperty("mute", trackModel.isMuted, &undoManager);
    vt.setProperty("solo", trackModel.isSoloed, &undoManager);
    vt.setProperty("arm", trackModel.isArmed, &undoManager);
    vt.setProperty("stereo", trackModel.isStereo, &undoManager);
    vt.setProperty("group", trackModel.isGroup, &undoManager);
    vt.setProperty("folded", trackModel.isFolded, &undoManager);
    vt.setProperty("volume", trackModel.volume, &undoManager);
    vt.setProperty("pan", trackModel.pan, &undoManager);
    vt.setProperty("inputChannel", trackModel.inputChannelIndex, &undoManager);
    vt.setProperty("midi", trackModel.isMidi || trackModel.type == TrackType::Midi, &undoManager);
    
    state.appendChild(vt, &undoManager);
}

void TimelineProject::insertTrack(int index, const TrackModel& trackModel)
{
    juce::ValueTree vt("TRACK");
    vt.setProperty("name", trackModel.name, &undoManager);
    vt.setProperty("midi", trackModel.isMidi || trackModel.type == TrackType::Midi, &undoManager);
    vt.setProperty("stereo", trackModel.isStereo, &undoManager);
    
    state.addChild(vt, index, &undoManager);
}

TrackModel TimelineProject::getTrack(int index) const
{
    TrackModel m = trackModelFromTree(getTrackTree(index));
    if (index < cachedPlugins.size()) m.plugins = cachedPlugins[index];
    if (index < cachedInstruments.size()) m.instrumentPlugin = cachedInstruments[index];
    return m;
}

int TimelineProject::getNumTracks() const
{
    int count = 0;
    for (auto c : state) {
        if (c.hasType("TRACK")) count++;
    }
    return count;
}

void TimelineProject::removeTrack(int index)
{
    state.removeChild(index, &undoManager);
}

void TimelineProject::moveTrack(int sourceIndex, int targetIndex)
{
    state.moveChild(sourceIndex, targetIndex, &undoManager);
}

void TimelineProject::setTrackName(int trackIndex, const juce::String& newName)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("name", newName, &undoManager);
}

void TimelineProject::setTrackMuted(int trackIndex, bool isMuted)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("mute", isMuted, &undoManager);
}

bool TimelineProject::isTrackMuted(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("mute", false);
}

void TimelineProject::setTrackArmed(int trackIndex, bool isArmed)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("arm", isArmed, &undoManager);
}

bool TimelineProject::isTrackArmed(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("arm", false);
}

void TimelineProject::setTrackStereo(int trackIndex, bool isStereo)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("stereo", isStereo, &undoManager);
}

bool TimelineProject::isTrackStereo(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("stereo", true);
}

void TimelineProject::setTrackSoloed(int trackIndex, bool isSoloed)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("solo", isSoloed, &undoManager);
}

bool TimelineProject::isTrackSoloed(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("solo", false);
}

void TimelineProject::setTrackVolume(int trackIndex, float volume)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("volume", volume, &undoManager);
}

float TimelineProject::getTrackVolume(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("volume", 1.0f);
}

void TimelineProject::setTrackPan(int trackIndex, float pan)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("pan", pan, &undoManager);
}

float TimelineProject::getTrackPan(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("pan", 0.0f);
}

void TimelineProject::setTrackInputChannel(int trackIndex, int inputChannel)
{
    if (auto vt = getTrackTree(trackIndex); vt.isValid())
        vt.setProperty("inputChannel", inputChannel, &undoManager);
}

int TimelineProject::getTrackInputChannel(int trackIndex) const
{
    return getTrackTree(trackIndex).getProperty("inputChannel", -1);
}

void TimelineProject::setMasterVolume(float volume)
{
    state.setProperty("masterVolume", volume, &undoManager);
}

float TimelineProject::getMasterVolume() const
{
    return state.getProperty("masterVolume", 1.0f);
}

void TimelineProject::setTrackSelected(int trackIndex, bool clearExisting)
{
    if (clearExisting) selectedTracks.clear();
    selectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
    lastSelectedTrack = trackIndex;
    listeners.call(&Listener::trackSelectionChanged);
}

void TimelineProject::toggleTrackSelection(int trackIndex)
{
    if (selectedTracks.contains(trackIndex))
        selectedTracks.removeRange(juce::Range<int>(trackIndex, trackIndex + 1));
    else
        selectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
    lastSelectedTrack = trackIndex;
    listeners.call(&Listener::trackSelectionChanged);
}

void TimelineProject::selectTrackRange(int fromIndex, int toIndex)
{
    selectedTracks.addRange(juce::Range<int>(std::min(fromIndex, toIndex), std::max(fromIndex, toIndex) + 1));
    listeners.call(&Listener::trackSelectionChanged);
}

bool TimelineProject::isTrackSelected(int trackIndex) const
{
    return selectedTracks.contains(trackIndex);
}

void TimelineProject::deselectAllTracks()
{
    selectedTracks.clear();
    listeners.call(&Listener::trackSelectionChanged);
}

void TimelineProject::setTimeSelection(double startSamples, double endSamples)
{
    timeSelectionStartSamples = startSamples;
    timeSelectionEndSamples = endSamples;
    listeners.call(&Listener::timeSelectionChanged);
}

void TimelineProject::setTimeSelectedTracks(const juce::SparseSet<int>& tracks)
{
    timeSelectedTracks = tracks;
    listeners.call(&Listener::timeSelectionChanged);
}

void TimelineProject::addTimeSelectedTrack(int trackIndex)
{
    timeSelectedTracks.addRange(juce::Range<int>(trackIndex, trackIndex + 1));
    listeners.call(&Listener::timeSelectionChanged);
}

void TimelineProject::clearTimeSelection()
{
    timeSelectionStartSamples = -1.0;
    timeSelectionEndSamples = -1.0;
    timeSelectedTracks.clear();
    listeners.call(&Listener::timeSelectionChanged);
}

void TimelineProject::groupTracks(const juce::SparseSet<int>& trackIndices) {}
void TimelineProject::ungroupTracks(int groupTrackIndex) {}
void TimelineProject::setTrackFolded(int trackIndex, bool isFolded) {}

juce::String TimelineProject::getProjectName() const { return state.getProperty("projectName", "Untitled Project"); }
void TimelineProject::setProjectName(const juce::String& name) { state.setProperty("projectName", name, &undoManager); }
juce::String TimelineProject::getKeySignature() const { return state.getProperty("keySignature", "C MAJ"); }
void TimelineProject::setKeySignature(const juce::String& key) { state.setProperty("keySignature", key, &undoManager); }
int TimelineProject::getTimeSigNumerator() const { return state.getProperty("timeSigNum", 4); }
void TimelineProject::setTimeSigNumerator(int num) { state.setProperty("timeSigNum", num, &undoManager); }
int TimelineProject::getTimeSigDenominator() const { return state.getProperty("timeSigDen", 4); }
void TimelineProject::setTimeSigDenominator(int den) { state.setProperty("timeSigDen", den, &undoManager); }

void TimelineProject::addClipToTrack(int trackIndex, AnyClipPtr clip)
{
    // For now we don't fully serialize clips in this pass, we just cache them.
    if (trackIndex >= cachedClips.size()) cachedClips.resize(trackIndex + 1);
    cachedClips[trackIndex].push_back(clip);
    listeners.call(&Listener::trackClipsChanged, trackIndex);
}

void TimelineProject::removeClip(AnyClipPtr clip) {
    if (!clip) return;
    
    int trackIndex = -1;
    for (size_t t = 0; t < cachedClips.size(); ++t) {
        auto& trackClips = cachedClips[t];
        auto it = std::find(trackClips.begin(), trackClips.end(), clip);
        if (it != trackClips.end()) {
            trackClips.erase(it);
            trackIndex = static_cast<int>(t);
            break;
        }
    }
    
    if (trackIndex != -1) {
        if (currentSelectedClip == clip) {
            setSelectedClip(AnyClipPtr{});
        }
        listeners.call(&Listener::trackClipsChanged, trackIndex);
    }
}
std::vector<AnyClipPtr> TimelineProject::getClipsOnTrack(int trackIndex) const
{
    if (trackIndex < cachedClips.size()) return cachedClips[trackIndex];
    return {};
}

void TimelineProject::resolveCrossfades(int trackIndex) {}

double TimelineProject::getTotalDurationSamples() const {
    double maxSample = 0.0;
    for (const auto& trackClips : cachedClips) {
        for (const auto& clip : trackClips) {
            if (clip) {
                double endSample = clip->getEndSample();
                if (endSample > maxSample) {
                    maxSample = endSample;
                }
            }
        }
    }
    return maxSample;
}

void TimelineProject::addPluginToTrack(int trackIndex, std::shared_ptr<Plugin> plugin) {
    if (trackIndex >= cachedPlugins.size()) cachedPlugins.resize(trackIndex + 1);
    cachedPlugins[trackIndex].push_back(plugin);
    if (auto vt = getTrackTree(trackIndex); vt.isValid()) {
        vt.appendChild(plugin->state, &undoManager);
    }
    listeners.call(&Listener::trackPluginsChanged, trackIndex);
}

void TimelineProject::removePluginFromTrack(int trackIndex, int pluginIndex) {
    if (trackIndex >= 0 && trackIndex < cachedPlugins.size()) {
        auto& plugins = cachedPlugins[trackIndex];
        if (pluginIndex >= 0 && pluginIndex < plugins.size()) {
            auto plugin = plugins[pluginIndex];
            plugins.erase(plugins.begin() + pluginIndex);
            
            if (auto vt = getTrackTree(trackIndex); vt.isValid()) {
                vt.removeChild(plugin->state, &undoManager);
            }
            listeners.call(&Listener::trackPluginsChanged, trackIndex);
        }
    }
}

void TimelineProject::setInstrumentForTrack(int trackIndex, std::shared_ptr<Plugin> plugin) {
    if (trackIndex >= cachedInstruments.size()) cachedInstruments.resize(trackIndex + 1);
    cachedInstruments[trackIndex] = plugin;
    if (auto vt = getTrackTree(trackIndex); vt.isValid()) {
        vt.appendChild(plugin->state, &undoManager);
    }
}

void TimelineProject::copySelectedClips() {}
void TimelineProject::pasteClips(int trackIndex, double startSample) {}
void TimelineProject::duplicateTrack(int trackIndex) {}
void TimelineProject::notifyClipModified() {}

void TimelineProject::setSelectedClip(AnyClipPtr clip)
{
    currentSelectedClip = clip;
    listeners.call(&Listener::selectedClipChanged);
}

AnyClipPtr TimelineProject::getSelectedClip() const
{
    return currentSelectedClip;
}

// Markers
void TimelineProject::addMarker(const MarkerModel& marker) {
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (!markersTree.isValid()) {
        markersTree = juce::ValueTree("MARKERS");
        state.appendChild(markersTree, &undoManager);
    }
    juce::ValueTree m("MARKER");
    m.setProperty("position", marker.positionSamples, &undoManager);
    m.setProperty("name", marker.name, &undoManager);
    m.setProperty("color", marker.color.toString(), &undoManager);
    markersTree.appendChild(m, &undoManager);
}

void TimelineProject::removeMarker(int index) {
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (markersTree.isValid()) {
        markersTree.removeChild(index, &undoManager);
    }
}

int TimelineProject::getNumMarkers() const {
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (markersTree.isValid()) {
        return markersTree.getNumChildren();
    }
    return 0;
}

MarkerModel TimelineProject::getMarker(int index) const {
    MarkerModel model;
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (markersTree.isValid()) {
        auto m = markersTree.getChild(index);
        if (m.isValid()) {
            model.positionSamples = m.getProperty("position", 0.0);
            model.name = m.getProperty("name", "Marker").toString();
            model.color = juce::Colour::fromString(m.getProperty("color", juce::Colours::white.toString()).toString());
        }
    }
    return model;
}

void TimelineProject::moveMarker(int index, double newPositionSamples) {
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (markersTree.isValid()) {
        auto m = markersTree.getChild(index);
        if (m.isValid()) {
            m.setProperty("position", newPositionSamples, &undoManager);
        }
    }
}

void TimelineProject::setMarkerName(int index, const juce::String& newName) {
    juce::ValueTree markersTree = state.getChildWithName("MARKERS");
    if (markersTree.isValid()) {
        auto m = markersTree.getChild(index);
        if (m.isValid()) {
            m.setProperty("name", newName, &undoManager);
        }
    }
}

// ValueTree::Listener
void TimelineProject::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged.hasType("TRACK")) {
        int index = state.indexOf(treeWhosePropertyHasChanged);
        if (index != -1) {
            if (property == juce::Identifier("name")) listeners.call(&Listener::trackNameChanged, index, treeWhosePropertyHasChanged.getProperty("name").toString());
            else if (property == juce::Identifier("mute")) listeners.call(&Listener::trackMuteChanged, index, treeWhosePropertyHasChanged.getProperty("mute"));
            else if (property == juce::Identifier("arm")) listeners.call(&Listener::trackArmChanged, index, treeWhosePropertyHasChanged.getProperty("arm"));
            else if (property == juce::Identifier("solo")) listeners.call(&Listener::trackSoloChanged, index, treeWhosePropertyHasChanged.getProperty("solo"));
            else if (property == juce::Identifier("volume")) listeners.call(&Listener::trackVolumeChanged, index, treeWhosePropertyHasChanged.getProperty("volume"));
            else if (property == juce::Identifier("pan")) listeners.call(&Listener::trackPanChanged, index, treeWhosePropertyHasChanged.getProperty("pan"));
            else if (property == juce::Identifier("stereo")) listeners.call(&Listener::trackStereoChanged, index, treeWhosePropertyHasChanged.getProperty("stereo"));
            else if (property == juce::Identifier("inputChannel")) listeners.call(&Listener::trackInputChannelChanged, index, treeWhosePropertyHasChanged.getProperty("inputChannel"));
        }
    } else if (treeWhosePropertyHasChanged.hasType("MARKER")) {
        juce::ValueTree markersTree = state.getChildWithName("MARKERS");
        if (markersTree.isValid()) {
            int index = markersTree.indexOf(treeWhosePropertyHasChanged);
            if (index != -1) {
                if (property == juce::Identifier("position")) listeners.call(&Listener::markerMoved, index, (double)treeWhosePropertyHasChanged.getProperty("position"));
            }
        }
    } else if (treeWhosePropertyHasChanged == state) {
        if (property == juce::Identifier("projectName")) listeners.call(&Listener::projectNameChanged, getProjectName());
        else if (property == juce::Identifier("timeSigNum") || property == juce::Identifier("timeSigDen")) listeners.call(&Listener::timeSignatureChanged, getTimeSigNumerator(), getTimeSigDenominator());
        else if (property == juce::Identifier("masterVolume")) listeners.call(&Listener::masterVolumeChanged, getMasterVolume());
    }
}

void TimelineProject::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (parentTree == state && childWhichHasBeenAdded.hasType("TRACK")) {
        int trackIndex = 0;
        for (auto c : state) {
            if (c == childWhichHasBeenAdded) break;
            if (c.hasType("TRACK")) trackIndex++;
        }
        cachedClips.insert(cachedClips.begin() + trackIndex, std::vector<AnyClipPtr>());
        cachedPlugins.insert(cachedPlugins.begin() + trackIndex, std::vector<std::shared_ptr<Plugin>>());
        cachedInstruments.insert(cachedInstruments.begin() + trackIndex, nullptr);
        listeners.call(&Listener::trackAdded, trackIndex, trackModelFromTree(childWhichHasBeenAdded));
    } else if (parentTree.hasType("MARKERS") && childWhichHasBeenAdded.hasType("MARKER")) {
        int index = parentTree.indexOf(childWhichHasBeenAdded);
        MarkerModel m;
        m.positionSamples = childWhichHasBeenAdded.getProperty("position", 0.0);
        m.name = childWhichHasBeenAdded.getProperty("name", "Marker").toString();
        m.color = juce::Colour::fromString(childWhichHasBeenAdded.getProperty("color", juce::Colours::white.toString()).toString());
        listeners.call(&Listener::markerAdded, index, m);
    }
}

void TimelineProject::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
{
    if (parentTree == state && childWhichHasBeenRemoved.hasType("TRACK")) {
        // Find the index among tracks, but childWhichHasBeenRemoved is already gone.
        // Wait, how do we know its track index? 
        // We can't easily recalculate it from the tree because it's gone.
        // Actually, we can use the cache size vs getNumTracks or just pass a generic fix.
        // A better approach is to not rely on indexFromWhichChildWasRemoved directly if MARKERS are there.
        // Wait, I will just iterate cachedClips and we assume trackRemoved passes the same index.
        // Actually, I'll fix this properly by counting TRACK nodes up to indexFromWhichChildWasRemoved.
        int trackIndex = 0;
        int i = 0;
        for (auto c : state) {
            if (i >= indexFromWhichChildWasRemoved) break;
            if (c.hasType("TRACK")) trackIndex++;
            i++;
        }
        if (trackIndex < cachedClips.size()) {
            cachedClips.erase(cachedClips.begin() + trackIndex);
        }
        if (trackIndex < cachedPlugins.size()) {
            cachedPlugins.erase(cachedPlugins.begin() + trackIndex);
        }
        if (trackIndex < cachedInstruments.size()) {
            cachedInstruments.erase(cachedInstruments.begin() + trackIndex);
        }
        listeners.call(&Listener::trackRemoved, trackIndex);
    } else if (parentTree.hasType("MARKERS")) {
        listeners.call(&Listener::markerRemoved, indexFromWhichChildWasRemoved);
    }
}

void TimelineProject::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
{
    // Need to handle reordering logic for UI listeners and clip caches
    listeners.call(&Listener::trackMoved, oldIndex, newIndex);
}

} // namespace Nimbus
