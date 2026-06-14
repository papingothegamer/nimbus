#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::MainLayout {

class SideBrowserComponent : public juce::Component {
public:
    SideBrowserComponent(NimbusEngine& engine);
    ~SideBrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NimbusEngine& engine;
    
    juce::TextEditor searchBox;
    juce::ListBox categoriesList;
    juce::ListBox itemsList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideBrowserComponent)
};

} // namespace Nimbus::MainLayout
