#include "DeviceChainComponent.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/PluginWindow.h"
#include "UI/DesignSystem/Iconography.h"

namespace Nimbus::MainLayout {

class DeviceChainComponent::PluginBox : public juce::Component {
public:
    PluginBox(PluginNode* pNode, Track* pTrack, NimbusEngine& e) : node(pNode), track(pTrack), engine(e) {
        if (node && node->getPluginInstance()) {
            name = node->getPluginInstance()->getName();
        }
        
        int iconDataSize = 0;
        if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Device.toUTF8(), iconDataSize)) {
            deviceIcon = juce::Drawable::createFromImageData(data, iconDataSize);
            if (deviceIcon) deviceIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        }
        
        int delDataSize = 0;
        if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Delete.toUTF8(), delDataSize)) {
            deleteIcon = juce::Drawable::createFromImageData(data, delDataSize);
            if (deleteIcon) deleteIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::PrimaryAction);
        }
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().reduced(2);
        
        // Main Background (Module Background)
        g.setColour(DesignSystem::Colors::ModuleBackground);
        g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
        
        // Header Background
        auto headerBounds = bounds.removeFromTop(24);
        g.setColour(DesignSystem::Colors::ComponentBackground);
        g.fillRoundedRectangle(headerBounds.toFloat(), 5.0f);
        g.fillRect(headerBounds.withTop(headerBounds.getBottom() - 5).toFloat()); // Square off the bottom corners
        
        // Border
        g.setColour(DesignSystem::Colors::ComponentBorder);
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 5.0f, 1.0f);

        // Header Content - Plugin Name
        g.setColour(DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f).withStyle(juce::Font::bold));
        g.drawText(name.toUpperCase(), headerBounds.reduced(8, 0), juce::Justification::centredLeft, true);
        
        // Body Design (Device Icon + Generic Parameters)
        auto iconBounds = bounds.removeFromTop(40).reduced(10);
        if (deviceIcon) {
            deviceIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
            deviceIcon->drawWithin(g, iconBounds.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }

        // Generic device graphic (e.g. some knobs or sliders)
        g.setColour(DesignSystem::Colors::Divider);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY() - 10;
        // Two simple "knobs"
        g.drawEllipse(cx - 20, cy - 8, 16, 16, 2.0f);
        g.drawEllipse(cx + 4, cy - 8, 16, 16, 2.0f);

        auto bottomBounds = getLocalBounds().reduced(2).removeFromBottom(24).reduced(4);
        if (deleteIcon) {
            deleteIcon->drawWithin(g, bottomBounds.removeFromRight(16).toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isPopupMenu()) {
            juce::PopupMenu menu;
            menu.addItem(1, "Copy");
            menu.addItem(2, "Paste", engine.getPluginClipboard().hasData);
            menu.addItem(3, "Duplicate");
            menu.addSeparator();
            menu.addItem(4, "Delete");
            
            menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
                if (result == 1) { // Copy
                    if (node && node->getPluginInstance()) {
                        auto& cb = engine.getPluginClipboard();
                        cb.description = node->getPluginInstance()->getPluginDescription();
                        node->getPluginInstance()->getStateInformation(cb.state);
                        cb.hasData = true;
                    }
                } else if (result == 2) { // Paste
                    auto& cb = engine.getPluginClipboard();
                    if (cb.hasData && track) {
                        juce::String err;
                        auto newInstance = engine.getPluginManager().loadPlugin(cb.description.fileOrIdentifier, err);
                        if (newInstance) {
                            newInstance->setStateInformation(cb.state.getData(), (int)cb.state.getSize());
                            auto newNode = std::make_unique<PluginNode>(std::move(newInstance));
                            track->addInsertPlugin(std::move(newNode));
                        }
                    }
                } else if (result == 3) { // Duplicate
                    if (node && node->getPluginInstance() && track) {
                        juce::MemoryBlock state;
                        node->getPluginInstance()->getStateInformation(state);
                        juce::String err;
                        auto newInstance = engine.getPluginManager().loadPlugin(node->getPluginInstance()->getPluginDescription().fileOrIdentifier, err);
                        if (newInstance) {
                            newInstance->setStateInformation(state.getData(), (int)state.getSize());
                            auto newNode = std::make_unique<PluginNode>(std::move(newInstance));
                            track->addInsertPlugin(std::move(newNode));
                        }
                    }
                } else if (result == 4 && track != nullptr) {
                    if (track->getInstrumentPlugin() == node) {
                        track->setInstrumentPlugin(nullptr);
                    } else {
                        track->removeInsertPlugin(node);
                    }
                    if (window != nullptr) {
                        window->closeButtonPressed();
                    }
                }
            });
        }
    }
    
    void mouseUp(const juce::MouseEvent& e) override {
        if (e.mods.isPopupMenu()) return;
        
        auto bottomArea = getLocalBounds().reduced(4).removeFromBottom(30);
        if (bottomArea.contains(e.getPosition())) {
            if (track != nullptr) {
                if (track->getInstrumentPlugin() == node) {
                    track->setInstrumentPlugin(nullptr);
                } else {
                    track->removeInsertPlugin(node);
                }
                if (window != nullptr) {
                    window->closeButtonPressed();
                }
            }
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
    NimbusEngine& engine;
    juce::String name;
    juce::Component::SafePointer<PluginWindow> window;
    std::unique_ptr<juce::Drawable> deviceIcon;
    std::unique_ptr<juce::Drawable> deleteIcon;
};

DeviceChainComponent::DeviceChainComponent(NimbusEngine& e) : engine(e) {
    startTimerHz(10); // Check for plugin additions
    
    int iconDataSize = 0;
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::AddPlugin.toUTF8(), iconDataSize)) {
        addPluginIcon = juce::Drawable::createFromImageData(data, iconDataSize);
        if (addPluginIcon) addPluginIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary.withAlpha(0.5f));
    }
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
        auto bounds = getLocalBounds();
        if (addPluginIcon) {
            auto iconBounds = bounds.withSizeKeepingCentre(48, 48).translated(0, -20);
            addPluginIcon->drawWithin(g, iconBounds.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        
        g.setColour(DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont());
        g.drawText("Drop Audio Effects Here", bounds.translated(0, 20), juce::Justification::centred, true);
    }
}

void DeviceChainComponent::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isPopupMenu() && currentTrackIndex != -1) {
        juce::PopupMenu menu;
        menu.addItem(1, "Paste", engine.getPluginClipboard().hasData);
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                auto& cb = engine.getPluginClipboard();
                if (cb.hasData) {
                    auto track = engine.getMixer()->getTrack(currentTrackIndex);
                    if (track) {
                        juce::String err;
                        auto newInstance = engine.getPluginManager().loadPlugin(cb.description.fileOrIdentifier, err);
                        if (newInstance) {
                            newInstance->setStateInformation(cb.state.getData(), (int)cb.state.getSize());
                            auto newNode = std::make_unique<PluginNode>(std::move(newInstance));
                            track->addInsertPlugin(std::move(newNode));
                        }
                    }
                }
            }
        });
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
    
    // Count total expected nodes
    int expectedNodes = (track->getInstrumentPlugin() != nullptr ? 1 : 0) + nodes.size();
    
    // Quick check if it's the same
    if (currentTrackIndex == trackIndex && pluginBoxes.size() == expectedNodes) {
        return; // Assume no change for now to avoid flickering
    }
    
    currentTrackIndex = trackIndex;
    pluginBoxes.clear();
    
    // Add instrument first
    if (auto* instr = dynamic_cast<PluginNode*>(track->getInstrumentPlugin())) {
        auto box = std::make_unique<PluginBox>(instr, track, engine);
        addAndMakeVisible(box.get());
        pluginBoxes.push_back(std::move(box));
    }
    
    for (auto& n : nodes) {
        if (auto* pluginNode = dynamic_cast<PluginNode*>(n.get())) {
            auto box = std::make_unique<PluginBox>(pluginNode, track, engine);
            addAndMakeVisible(box.get());
            pluginBoxes.push_back(std::move(box));
        }
    }
    
    resized();
    repaint();
}

} // namespace Nimbus::MainLayout
