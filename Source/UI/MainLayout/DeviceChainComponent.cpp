#include "DeviceChainComponent.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/PluginWindow.h"
#include "UI/DesignSystem/Iconography.h"
#include "Core/Plugins/IStockPlugin.h"
#include "Core/Plugins/StockPluginFactory.h"

namespace Nimbus::MainLayout {

class DeviceChainComponent::PluginBox : public juce::Component {
public:
    PluginBox(IAudioNode* pNode, Track* pTrack, NimbusEngine& e) : node(pNode), track(pTrack), engine(e) {
        if (auto* vst = dynamic_cast<PluginNode*>(node)) {
            if (vst->getPluginInstance()) {
                name = vst->getPluginInstance()->getName();
            }
        } else if (auto* stock = dynamic_cast<IStockPlugin*>(node)) {
            name = stock->getName();
            embeddedEditor.reset(stock->createEditor());
            if (embeddedEditor) {
                addAndMakeVisible(embeddedEditor.get());
                editorWidth = embeddedEditor->getWidth();
            }
        }

        
        int iconDataSize = 0;
        if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Device.toUTF8(), iconDataSize)) {
            deviceIcon = juce::Drawable::createFromImageData(data, iconDataSize);
            if (deviceIcon) deviceIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
        }
        
        if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Settings.toUTF8(), iconDataSize)) {
            settingsIcon = juce::Drawable::createFromImageData(data, iconDataSize);
            if (settingsIcon) settingsIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
        }
        
        if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Delete.toUTF8(), iconDataSize)) {
            deleteIcon = juce::Drawable::createFromImageData(data, iconDataSize);
            if (deleteIcon) deleteIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextPrimary);
        }
    }
    
    void paint(juce::Graphics& g) override {
        if (node == nullptr) return;
        auto bounds = getLocalBounds().reduced(2).toFloat();
        bool isBypassed = false;
        if (auto* vst = dynamic_cast<PluginNode*>(node)) isBypassed = vst->isBypassed();
        else if (auto* stock = dynamic_cast<IStockPlugin*>(node)) isBypassed = stock->isBypassed();
        
        // Main Device Background
        g.setColour(DesignSystem::Colors::ModuleBackground.darker(0.05f));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Header Background (Darker/tinted title bar)
        auto headerBounds = bounds.removeFromTop(22.0f);
        g.setColour(isBypassed ? DesignSystem::Colors::ComponentBackground.darker(0.1f) : DesignSystem::Colors::ComponentBackground.brighter(0.1f));
        g.fillRoundedRectangle(headerBounds.getX(), headerBounds.getY(), headerBounds.getWidth(), headerBounds.getHeight(), 4.0f);
        g.fillRect(headerBounds.getX(), headerBounds.getBottom() - 2.0f, headerBounds.getWidth(), 2.0f); // Square bottom corners
        
        // Outer Border
        g.setColour(DesignSystem::Colors::ComponentBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Bypass toggle (Ableton style small square in top left)
        juce::Rectangle<float> bypassRect(headerBounds.getX() + 6.0f, headerBounds.getY() + 5.0f, 12.0f, 12.0f);
        if (!isBypassed) {
            g.setColour(juce::Colours::yellow.withAlpha(0.9f));
            g.fillRoundedRectangle(bypassRect, 2.0f);
        } else {
            g.setColour(DesignSystem::Colors::ComponentBackground.darker(0.2f));
            g.fillRoundedRectangle(bypassRect, 2.0f);
            g.setColour(DesignSystem::Colors::ComponentBorder);
            g.drawRoundedRectangle(bypassRect, 2.0f, 1.0f);
        }

        // Header Content - Plugin Name
        g.setColour(isBypassed ? DesignSystem::Colors::TextSecondary : DesignSystem::Colors::TextPrimary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f).withStyle(juce::Font::bold));
        g.drawText(name.toUpperCase(), headerBounds.withTrimmedLeft(24).withTrimmedRight(40), juce::Justification::centredLeft, true);
        
        // Settings Icon (Wrench/Gear equivalent)
        juce::Rectangle<float> settingsRect(headerBounds.getRight() - 36.0f, headerBounds.getY() + 4.0f, 14.0f, 14.0f);
        if (settingsIcon) {
            settingsIcon->setAlpha(isBypassed ? 0.5f : 0.8f);
            settingsIcon->drawWithin(g, settingsRect, juce::RectanglePlacement::centred, 1.0f);
        }
        
        // Delete Icon (X)
        juce::Rectangle<float> deleteRect(headerBounds.getRight() - 18.0f, headerBounds.getY() + 4.0f, 14.0f, 14.0f);
        if (deleteIcon) {
            deleteIcon->setAlpha(isBypassed ? 0.5f : 0.8f);
            deleteIcon->drawWithin(g, deleteRect, juce::RectanglePlacement::centred, 1.0f);
        }
        
        // Body Design
        auto bodyBounds = bounds; // already has header removed
        
        if (!isBypassed) {
            if (embeddedEditor) {
                // If it's a stock plugin, we don't draw the generic macro area.
                // The editor is already positioned in resized().
            } else {
                // Ableton style generic macro area
            auto iconBounds = bodyBounds.removeFromTop(40).reduced(10);
            if (deviceIcon) {
                deviceIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
                deviceIcon->setAlpha(0.3f);
                deviceIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
            }

            // Draw some generic minimalist "XY pad" or blank parameter space
            g.setColour(DesignSystem::Colors::ComponentBackground.darker(0.1f));
            auto paramArea = bodyBounds.reduced(8);
            g.fillRoundedRectangle(paramArea, 3.0f);
            g.setColour(DesignSystem::Colors::ComponentBorder.withAlpha(0.5f));
            g.drawRoundedRectangle(paramArea, 3.0f, 1.0f);
            
            g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.4f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(10.0f));
            g.drawText("Open GUI to edit", paramArea, juce::Justification::centred, false);
            
            }
        } else {
            g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.4f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f).withStyle(juce::Font::italic));
            g.drawText("Bypassed", bodyBounds, juce::Justification::centred, false);
        }
    }
    
    void resized() override {
        if (embeddedEditor) {
            auto bounds = getLocalBounds().reduced(2);
            bounds.removeFromTop(24); // header
            embeddedEditor->setBounds(bounds);
        }
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        if (node == nullptr) return;
        auto bounds = getLocalBounds().reduced(2);
        auto headerBounds = bounds.removeFromTop(24);
        
        juce::Rectangle<int> bypassRect(headerBounds.getX() + 6, headerBounds.getY() + 5, 12, 12);
        juce::Rectangle<int> settingsRect(headerBounds.getRight() - 36, headerBounds.getY() + 4, 14, 14);
        juce::Rectangle<int> deleteRect(headerBounds.getRight() - 18, headerBounds.getY() + 4, 14, 14);
        
        if (bypassRect.contains(e.getPosition())) {
            if (auto* vst = dynamic_cast<PluginNode*>(node)) vst->setBypassed(!vst->isBypassed());
            else if (auto* stock = dynamic_cast<IStockPlugin*>(node)) stock->setBypassed(!stock->isBypassed());
            repaint();
            return;
        }
        
        if (settingsRect.contains(e.getPosition())) {
            openPluginWindow();
            return;
        }
        
        if (deleteRect.contains(e.getPosition())) {
            deletePlugin();
            return;
        }
        
        if (e.mods.isPopupMenu()) {
            juce::PopupMenu menu;
            menu.addItem(1, "Copy");
            menu.addItem(2, "Paste", engine.getPluginClipboard().hasData);
            menu.addItem(3, "Duplicate");
            menu.addSeparator();
            menu.addItem(4, "Delete");
            
            menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
                if (result == 1) { // Copy
                    if (auto* vst = dynamic_cast<PluginNode*>(node)) {
                        if (vst->getPluginInstance()) {
                            auto& cb = engine.getPluginClipboard();
                            cb.description = vst->getPluginInstance()->getPluginDescription();
                            vst->getPluginInstance()->getStateInformation(cb.state);
                            cb.hasData = true;
                        }
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
                    if (auto* vst = dynamic_cast<PluginNode*>(node)) {
                        if (vst->getPluginInstance() && track) {
                            juce::MemoryBlock state;
                            vst->getPluginInstance()->getStateInformation(state);
                            juce::String err;
                            auto newInstance = engine.getPluginManager().loadPlugin(vst->getPluginInstance()->getPluginDescription().fileOrIdentifier, err);
                            if (newInstance) {
                                newInstance->setStateInformation(state.getData(), (int)state.getSize());
                                auto newNode = std::make_unique<PluginNode>(std::move(newInstance));
                                track->addInsertPlugin(std::move(newNode));
                            }
                        }
                    }
                } else if (result == 4) {
                    deletePlugin();
                }
            });
        }
    }
    
    void mouseDoubleClick(const juce::MouseEvent& e) override {
        openPluginWindow();
    }

public:
    int getEditorWidth() const { return editorWidth; }

private:
    void openPluginWindow() {
        if (auto* vst = dynamic_cast<PluginNode*>(node)) {
            if (window == nullptr) {
                window = new PluginWindow(name, vst);
            } else {
                window->toFront(true);
            }
        }
    }
    
    void deletePlugin() {
        if (track != nullptr && node != nullptr) {
            // CRITICAL: Destroy the editor synchronously BEFORE destroying the plugin node.
            // This stops any timers in the editor from firing and accessing a freed plugin.
            embeddedEditor.reset();
            
            auto* nodeToDelete = node;
            node = nullptr; // PREVENT DANGLING POINTER ACCESS!
            
            if (track->getInstrumentPlugin() == nodeToDelete) {
                track->setInstrumentPlugin(nullptr);
            } else {
                track->removeInsertPlugin(nodeToDelete);
            }
            if (window != nullptr) {
                window->closeButtonPressed();
            }
        }
    }

    IAudioNode* node = nullptr;
    Track* track;
    NimbusEngine& engine;
    juce::String name;
    std::unique_ptr<juce::Component> embeddedEditor;
    int editorWidth = 0;
    juce::Component::SafePointer<PluginWindow> window;
    std::unique_ptr<juce::Drawable> deviceIcon;
    std::unique_ptr<juce::Drawable> settingsIcon;
    std::unique_ptr<juce::Drawable> deleteIcon;
};

DeviceChainComponent::DeviceChainComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, true, false, false);
    
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
        
        juce::PopupMenu stockMenu;
        auto categories = StockPluginFactory::getCategories();
        int stockId = 1000;
        for (const auto& cat : categories) {
            juce::PopupMenu catMenu;
            auto plugins = StockPluginFactory::getPluginsInCategory(cat);
            for (const auto& plug : plugins) {
                catMenu.addItem(stockId++, plug);
            }
            stockMenu.addSubMenu(cat, catMenu);
        }
        menu.addSubMenu("Add Audio Effect", stockMenu);
        menu.addSeparator();
        
        menu.addItem(1, "Paste", engine.getPluginClipboard().hasData);
        menu.showMenuAsync(juce::PopupMenu::Options(), [this, categories](int result) {
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
            } else if (result >= 1000) {
                // Find which stock plugin was selected
                int id = 1000;
                juce::String selectedPlugin;
                for (const auto& cat : categories) {
                    auto plugins = StockPluginFactory::getPluginsInCategory(cat);
                    for (const auto& plug : plugins) {
                        if (id == result) {
                            selectedPlugin = plug;
                            break;
                        }
                        id++;
                    }
                }
                
                if (selectedPlugin.isNotEmpty()) {
                    auto track = engine.getMixer()->getTrack(currentTrackIndex);
                    if (track) {
                        auto stockPlug = StockPluginFactory::createPlugin(selectedPlugin);
                        if (stockPlug) {
                            track->addInsertPlugin(std::move(stockPlug));
                        }
                    }
                }
            }
        });
    }
}

void DeviceChainComponent::resized() {
    viewport.setBounds(getLocalBounds());
    
    int x = 5;
    for (auto& box : pluginBoxes) {
        int w = box->getEditorWidth() > 0 ? box->getEditorWidth() + 4 : 140;
        box->setBounds(x, 5, w, viewport.getHeight() - 10 - viewport.getScrollBarThickness());
        x += w + 5;
    }
    
    content.setBounds(0, 0, x, viewport.getHeight() - viewport.getScrollBarThickness());
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
    
    // Count total expected nodes
    int expectedNodes = (track->getInstrumentPlugin() != nullptr ? 1 : 0) + track->getInsertGraph().getNodes().size();
    
    // Quick check if it's the same
    if (currentTrackIndex == trackIndex && pluginBoxes.size() == expectedNodes) {
        return; // Assume no change for now to avoid flickering
    }
    
    currentTrackIndex = trackIndex;
    pluginBoxes.clear();
    
    // Add instrument first
    if (auto* instr = dynamic_cast<PluginNode*>(track->getInstrumentPlugin())) {
        auto box = std::make_unique<PluginBox>(instr, track, engine);
        content.addAndMakeVisible(box.get());
        pluginBoxes.push_back(std::move(box));
    }
    
    const auto& nodes = track->getInsertGraph().getNodes();
    for (auto& n : nodes) {
        auto box = std::make_unique<PluginBox>(n.get(), track, engine);
        content.addAndMakeVisible(box.get());
        pluginBoxes.push_back(std::move(box));
    }
    
    resized();
    repaint();
}

} // namespace Nimbus::MainLayout
