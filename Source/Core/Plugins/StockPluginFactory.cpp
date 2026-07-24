#include "StockPluginFactory.h"
#include "Stock/FilterPlugin.h"
#include "Stock/CompressorPlugin.h"
#include "Stock/ChorusPlugin.h"
#include "Stock/DelayPlugin.h"
#include "Stock/ReverbPlugin.h"
#include "Stock/GainPlugin.h"
#include "Stock/CloudEQPlugin.h"
#include "Stock/MidiArpeggiatorPlugin.h"
#include "Stock/MidiMonitorPlugin.h"
#include "Stock/MidiPitchPlugin.h"
#include "Stock/MidiScalePlugin.h"
#include "Stock/MidiChordPlugin.h"

namespace Nimbus {

juce::StringArray StockPluginFactory::getCategories() {
    return {
        "EQ & Filters",
        "Dynamics",
        "Modulation",
        "Delay & Loop",
        "Reverb",
        "Utility",
        "MIDI Effects"
    };
}

juce::StringArray StockPluginFactory::getPluginsInCategory(const juce::String& category) {
    if (category == "EQ & Filters") return { "Filter", "Cloud EQ" };
    if (category == "Dynamics") return { "Compressor" };
    if (category == "Modulation") return { "Chorus" };
    if (category == "Delay & Loop") return { "Delay" };
    if (category == "Reverb") return { "Reverb" };
    if (category == "Utility") return { "Gain" };
    if (category == "MIDI Effects") return { "Arpeggiator", "MIDI Monitor", "Pitch", "Scale", "Chord" };
    return {};
}

std::unique_ptr<IStockPlugin> StockPluginFactory::createPlugin(const juce::String& name) {
    if (name == "Filter") return std::make_unique<FilterPlugin>();
    if (name == "Compressor") return std::make_unique<CompressorPlugin>();
    if (name == "Chorus") return std::make_unique<ChorusPlugin>();
    if (name == "Delay") return std::make_unique<DelayPlugin>();
    if (name == "Reverb") return std::make_unique<ReverbPlugin>();
    if (name == "Gain") return std::make_unique<GainPlugin>();
    if (name == "Cloud EQ") return std::make_unique<CloudEQPlugin>();
    
    // MIDI Effects
    if (name == "Arpeggiator") return std::make_unique<MidiArpeggiatorPlugin>();
    if (name == "MIDI Monitor") return std::make_unique<MidiMonitorPlugin>();
    if (name == "Pitch") return std::make_unique<MidiPitchPlugin>();
    if (name == "Scale") return std::make_unique<MidiScalePlugin>();
    if (name == "Chord") return std::make_unique<MidiChordPlugin>();
    
    return nullptr;
}

} // namespace Nimbus
