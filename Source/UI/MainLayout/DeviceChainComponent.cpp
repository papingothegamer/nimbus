#include "DeviceChainComponent.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/PluginWindow.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::MainLayout {

class DeviceChainComponent::PluginBox : public juce::Component {
public:
    PluginBox(PluginNode* pNode, Track* pTrack) : node(pNode), track(pTrack) {
        if (node && node->getPluginInstance()) {
            name = node->getPluginInstance()->getName();
        }
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().reduced(4);
        g.setColour(DesignSystem::Colors::PanelBackground.brighter(0.1f));
        g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
        
        g.setColour(DesignSystem::Colors::Divider);
        g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 1.0f);
        
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(14.0f);
        g.drawText(name, bounds, juce::Justification::centred, true);
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isPopupMenu()) {
            juce::PopupMenu menu;
            // menu.addItem(1, "Copy"); // Deferred
            // menu.addItem(2, "Duplicate"); // Deferred
            menu.addItem(3, "Delete");
            
            menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
                if (result == 3 && track != nullptr) {
                    track->removeInsertPlugin(node);
                    if (window != nullptr) {
                        window->closeButtonPressed();
                    }
                }
            });
        }
    }
    
    void mouseDoubleClick(const juce::MouseEvent& e) override {
        if (window == nullptr) {
            window = new PluginWindow(name, node);
        } else {
            window->toFront(true);
        }
    }

private:
    PluginNode* node;
    Track* track;
    juce::String name;
    juce::Component::SafePointer<PluginWindow> window;
};

DeviceChainComponent::DeviceChainComponent(NimbusEngine& e) : engine(e) {
    startTimerHz(10); // Check for plugin additions
}

DeviceChainComponent::~DeviceChainComponent() {
    stopTimer();
}

void DeviceChainComponent::timerCallback() {
    updateChain();
}

void DeviceChainComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
    
    if (pluginBoxes.empty()) {
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("Drop Audio Effects Here", getLocalBounds(), juce::Justification::centred, true);
    }
}

void DeviceChainComponent::resized() {
    auto bounds = getLocalBounds();
    int x = 5;
    for (auto& box : pluginBoxes) {
        box->setBounds(x, 5, 120, bounds.getHeight() - 10);
        x += 120 + 5;
    }
}

void DeviceChainComponent::updateChain() {
    auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
    if (selectedTracks.isEmpty()) {
        if (currentTrackIndex != -1) {
            currentTrackIndex = -1;
            pluginBoxes.clear();
            repaint();
        }
        return;
    }
    
    int trackIndex = selectedTracks.getRange(0).getStart();
    auto track = engine.getMixer()->getTrack(trackIndex);
    if (!track) return;
    
    const auto& nodes = track->getInsertGraph().getNodes();
    
    // Quick check if it's the same
    if (currentTrackIndex == trackIndex && pluginBoxes.size() == nodes.size()) {
        return; // Assume no change for now to avoid flickering
    }
    
    currentTrackIndex = trackIndex;
    pluginBoxes.clear();
    
    for (auto& n : nodes) {
        if (auto* pluginNode = dynamic_cast<PluginNode*>(n.get())) {
            auto box = std::make_unique<PluginBox>(pluginNode, track);
            addAndMakeVisible(box.get());
            pluginBoxes.push_back(std::move(box));
        }
    }
    
    resized();
    repaint();
}

} // namespace Nimbus::MainLayout
