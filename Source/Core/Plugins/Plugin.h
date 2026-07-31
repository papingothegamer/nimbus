#pragma once

#include <JuceHeader.h>
#include "../../AudioEngine/PluginNode.h"
#include "IStockPlugin.h"

namespace Nimbus {

class Plugin {
public:
    Plugin(const juce::PluginDescription& desc) 
        : description(desc) 
    {
    }

    virtual ~Plugin() = default;

    virtual juce::String getName() const = 0;
    virtual juce::Component* createEditor() = 0;
    virtual bool isBypassed() const = 0;
    virtual void setBypassed(bool b) = 0;

    // Creates the processing node for the audio graph
    virtual std::unique_ptr<Node> createNode() = 0;

    const juce::PluginDescription& getDescription() const { return description; }

    juce::ValueTree state { "PLUGIN" };
protected:
    juce::PluginDescription description;
};

// Wraps an external VST3/AU juce::AudioPluginInstance
class ExternalPlugin : public Plugin {
public:
    ExternalPlugin(std::unique_ptr<juce::AudioPluginInstance> instance, const juce::PluginDescription& desc)
        : Plugin(desc), pluginInstance(std::move(instance)) 
    {
        state.setProperty("type", "external", nullptr);
        state.setProperty("identifier", desc.fileOrIdentifier, nullptr);
    }

    juce::String getName() const override { return pluginInstance ? pluginInstance->getName() : "Unknown"; }
    
    juce::Component* createEditor() override {
        if (pluginInstance && pluginInstance->hasEditor())
            return pluginInstance->createEditorIfNeeded();
        return nullptr;
    }

    bool isBypassed() const override { return bypassed; }
    void setBypassed(bool b) override { bypassed = b; }

    std::unique_ptr<Node> createNode() override {
        if (!pluginInstance) return nullptr;
        auto node = std::make_unique<PluginNode>(pluginInstance.get());
        node->setBypassed(bypassed);
        return node;
    }

private:
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    bool bypassed = false;
};

// Wraps a stock plugin
class InternalPlugin : public Plugin {
public:
    InternalPlugin(std::unique_ptr<IStockPlugin> instance, const juce::PluginDescription& desc)
        : Plugin(desc), pluginInstance(std::move(instance)) 
    {
        state.setProperty("type", "internal", nullptr);
        state.setProperty("identifier", desc.fileOrIdentifier, nullptr);
    }

    juce::String getName() const override { return pluginInstance ? pluginInstance->getName() : "Unknown"; }
    
    juce::Component* createEditor() override {
        if (pluginInstance)
            return pluginInstance->createEditor();
        return nullptr;
    }

    bool isBypassed() const override { return bypassed; }
    void setBypassed(bool b) override { 
        bypassed = b; 
        if (pluginInstance) pluginInstance->setBypassed(b);
    }

    std::unique_ptr<Node> createNode() override {
        // Wait, IStockPlugin inherits from Node! 
        // But we can't hand over ownership of the pluginInstance to the graph!
        // We need a wrapper node that just points to it.
        return std::make_unique<InternalPluginWrapperNode>(pluginInstance.get());
    }

private:
    class InternalPluginWrapperNode : public Node {
    public:
        InternalPluginWrapperNode(IStockPlugin* p) : plugin(p) {}
        void prepare(double sampleRate, int blockSize) override {
            Node::prepare(sampleRate, blockSize);
            if (plugin) plugin->prepare(sampleRate, blockSize);
        }
        void process(const ProcessContext& context) override {
            if (plugin && !plugin->isBypassed()) plugin->process(context);
        }
        int getLatencySamples() const override {
            return plugin ? plugin->getLatencySamples() : 0;
        }
    private:
        IStockPlugin* plugin;
    };

    std::unique_ptr<IStockPlugin> pluginInstance;
    bool bypassed = false;
};

} // namespace Nimbus
