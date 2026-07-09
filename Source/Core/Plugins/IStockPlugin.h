#pragma once

#include <JuceHeader.h>
#include "AudioEngine/IAudioNode.h"

namespace Nimbus {

class IStockPlugin : public IAudioNode {
public:
    virtual ~IStockPlugin() = default;

    virtual juce::String getName() const = 0;
    virtual juce::String getCategory() const = 0;
    
    // Creates the embedded UI editor for this plugin. 
    // The caller takes ownership of the returned component.
    virtual juce::Component* createEditor() = 0;
    
    virtual bool isBypassed() const = 0;
    virtual void setBypassed(bool b) = 0;
};

} // namespace Nimbus
