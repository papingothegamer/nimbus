#include "StockPluginFactory.h"
#include "Stock/FilterPlugin.h"
#include "Stock/CompressorPlugin.h"
#include "Stock/ChorusPlugin.h"
#include "Stock/DelayPlugin.h"
#include "Stock/ReverbPlugin.h"
#include "Stock/GainPlugin.h"
#include "Stock/CloudEQPlugin.h"

namespace Nimbus {

juce::StringArray StockPluginFactory::getCategories() {
    return {
        "EQ & Filters",
        "Dynamics",
        "Modulation",
        "Delay & Loop",
        "Reverb",
        "Utility"
    };
}

juce::StringArray StockPluginFactory::getPluginsInCategory(const juce::String& category) {
    if (category == "EQ & Filters") return { "Filter", "Cloud EQ" };
    if (category == "Dynamics") return { "Compressor" };
    if (category == "Modulation") return { "Chorus" };
    if (category == "Delay & Loop") return { "Delay" };
    if (category == "Reverb") return { "Reverb" };
    if (category == "Utility") return { "Gain" };
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
    
    return nullptr;
}

} // namespace Nimbus
