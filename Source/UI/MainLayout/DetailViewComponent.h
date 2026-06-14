#pragma once

#include <JuceHeader.h>
#include "Core/NimbusEngine.h"

namespace Nimbus::MainLayout {

class DetailViewComponent : public juce::Component {
public:
    DetailViewComponent(NimbusEngine& engine);
    ~DetailViewComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    NimbusEngine& engine;
    juce::Label placeholderLabel{"Detail", "Select a track to view devices or clips."};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetailViewComponent)
};

} // namespace Nimbus::MainLayout
