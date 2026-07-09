#pragma once

#include <JuceHeader.h>
#include "IStockPlugin.h"
#include <memory>
#include <vector>

namespace Nimbus {

class StockPluginFactory {
public:
    static juce::StringArray getCategories();
    static juce::StringArray getPluginsInCategory(const juce::String& category);
    
    // Creates a stock plugin by name (e.g. "Compressor")
    static std::unique_ptr<IStockPlugin> createPlugin(const juce::String& name);
};

} // namespace Nimbus
