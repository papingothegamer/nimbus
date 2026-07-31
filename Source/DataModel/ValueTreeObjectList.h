#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

namespace Nimbus {

/**
 * A utility class to automatically maintain a list of C++ objects that are
 * built from the children of a specific juce::ValueTree.
 *
 * This mirrors Tracktion Engine's ValueTreeObjectList methodology natively.
 */
template <typename ObjectType>
class ValueTreeObjectList : public juce::ValueTree::Listener
{
public:
    ValueTreeObjectList(juce::ValueTree parentTree)
        : parent(parentTree)
    {
    }

    virtual ~ValueTreeObjectList()
    {
        if (parent.isValid())
            parent.removeListener(this);
    }

    void initialise()
    {
        if (parent.isValid())
        {
            parent.addListener(this);
            for (auto child : parent)
                if (isSuitableType(child))
                    objects.push_back(std::unique_ptr<ObjectType>(createNewObject(child)));
        }
    }

    // You must implement these in your subclass:
    virtual bool isSuitableType(const juce::ValueTree& v) const = 0;
    virtual ObjectType* createNewObject(const juce::ValueTree& v) = 0;
    virtual void deleteObject(ObjectType* obj) { delete obj; }
    virtual void newObjectAdded(ObjectType*) {}
    virtual void objectRemoved(ObjectType*) {}
    virtual void objectOrderChanged() {}

    const std::vector<std::unique_ptr<ObjectType>>& getObjects() const { return objects; }

    ObjectType* getObjectFor(const juce::ValueTree& v) const
    {
        for (auto& obj : objects)
            if (obj->getValueTree() == v)
                return obj.get();
        return nullptr;
    }

protected:
    juce::ValueTree parent;
    std::vector<std::unique_ptr<ObjectType>> objects;

    void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& child) override
    {
        if (parentTree == parent && isSuitableType(child))
        {
            int index = parent.indexOf(child);
            // Count how many suitable objects exist before this index
            int insertIndex = 0;
            for (int i = 0; i < index; ++i)
                if (isSuitableType(parent.getChild(i)))
                    insertIndex++;

            auto* newObj = createNewObject(child);
            objects.insert(objects.begin() + insertIndex, std::unique_ptr<ObjectType>(newObj));
            newObjectAdded(newObj);
        }
    }

    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& child, int) override
    {
        if (parentTree == parent && isSuitableType(child))
        {
            for (auto it = objects.begin(); it != objects.end(); ++it)
            {
                if ((*it)->getValueTree() == child)
                {
                    auto* obj = it->release();
                    objects.erase(it);
                    objectRemoved(obj);
                    deleteObject(obj);
                    break;
                }
            }
        }
    }

    void valueTreeChildOrderChanged(juce::ValueTree& parentTree, int, int) override
    {
        if (parentTree == parent)
        {
            std::vector<std::unique_ptr<ObjectType>> newOrder;
            for (auto child : parent)
            {
                if (isSuitableType(child))
                {
                    for (auto it = objects.begin(); it != objects.end(); ++it)
                    {
                        if (*it && (*it)->getValueTree() == child)
                        {
                            newOrder.push_back(std::move(*it));
                            break;
                        }
                    }
                }
            }
            objects = std::move(newOrder);
            objectOrderChanged();
        }
    }

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {}
    void valueTreeParentChanged(juce::ValueTree&) override {}
};

} // namespace Nimbus
