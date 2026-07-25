#include "SideBrowserComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"
#include "UI/DesignSystem/Iconography.h"
#include <algorithm>
#include "Core/Plugins/StockPluginFactory.h"

namespace Nimbus::MainLayout {

class SideBrowserComponent::CategoriesModel : public juce::ListBoxModel {
public:
    CategoriesModel() {
        categories = {
            "Audio Devices", "MIDI Devices", 
            "Plugins"
        };
    }
    
    int getNumRows() override { return categories.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(6, 2);
        if (rowIsSelected) {
            g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.12f));
            g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        }
        
        g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f).boldened());
        g.drawText(categories[rowNumber], bounds.withTrimmedLeft(12), juce::Justification::centredLeft, true);
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (onCategorySelected) onCategorySelected(categories[row]);
    }

    std::vector<juce::String> categories;
    std::function<void(const juce::String&)> onCategorySelected;
};

class SideBrowserComponent::PluginItemsModel : public juce::ListBoxModel {
public:
    PluginItemsModel(NimbusEngine& e) : engine(e) {}
    
    struct ListItem {
        bool isHeader = false;
        juce::String headerText;
        juce::PluginDescription desc;
    };
    
    void updateList(bool instrumentsOnly) {
        items.clear();
        std::vector<juce::PluginDescription> rawPlugins;
        auto& list = engine.getPluginManager().getKnownPluginList();
        for (int i = 0; i < list.getNumTypes(); ++i) {
            auto* type = list.getType(i);
            rawPlugins.push_back(*type);
        }
        // Sort by manufacturer then name
        std::sort(rawPlugins.begin(), rawPlugins.end(), [](const auto& a, const auto& b) {
            if (a.manufacturerName != b.manufacturerName)
                return a.manufacturerName.compareIgnoreCase(b.manufacturerName) < 0;
            return a.name.compareIgnoreCase(b.name) < 0;
        });
        
        juce::String currentMfg;
        for (auto& p : rawPlugins) {
            if (p.manufacturerName != currentMfg) {
                currentMfg = p.manufacturerName;
                items.push_back({true, currentMfg, {}});
            }
            if (expandedMakers.count(currentMfg) > 0) {
                items.push_back({false, "", p});
            }
        }
    }

    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto& item = items[rowNumber];
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4, 1);
        
        if (item.isHeader) {
            g.setColour(DesignSystem::Colors::TextSecondary.darker(0.2f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f).boldened());
            
            bool isExpanded = expandedMakers.count(item.headerText) > 0;
            g.drawText(" " + item.headerText.toUpperCase(), 16, 0, width - 16, height, juce::Justification::centredLeft, true);
            
            // Draw chevron manually using lines for a cleaner look
            float chevronX = 8.0f;
            float chevronY = height / 2.0f;
            juce::Path p;
            if (isExpanded) {
                p.addTriangle(chevronX - 3, chevronY - 2, chevronX + 3, chevronY - 2, chevronX, chevronY + 2);
            } else {
                p.addTriangle(chevronX - 2, chevronY - 3, chevronX - 2, chevronY + 3, chevronX + 2, chevronY);
            }
            g.fillPath(p);
        } else {
            if (rowIsSelected) {
                g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.15f));
                g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
            }
            g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
            
            // Draw bullet point
            g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.3f));
            g.fillEllipse(16.0f, height / 2.0f - 2.0f, 4.0f, 4.0f);
            
            g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
            g.drawText(item.desc.name, 28, 0, width - 28, height, juce::Justification::centredLeft, true);
        }
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) {
            if (expandedMakers.count(item.headerText) > 0) {
                expandedMakers.erase(item.headerText);
            } else {
                expandedMakers.insert(item.headerText);
            }
            // Need to notify component to update list. 
            // The cleanest way is to call updateList and updateContent, but we don't have a direct reference to the ListBox here.
            // We can just rely on a callback or pass the listbox in.
            if (onModelChanged) onModelChanged();
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) return;
        
        auto& desc = item.desc;
        
        auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
        if (!selectedTracks.isEmpty()) {
            int trackIndex = selectedTracks.getRange(0).getStart();
            juce::String err;
            auto instance = engine.getPluginManager().loadPlugin(desc.fileOrIdentifier, err);
            if (instance != nullptr) {
                auto node = std::make_unique<PluginNode>(std::move(instance));
                auto track = engine.getMixer()->getTrack(trackIndex);
                if (track) {
                    if (desc.isInstrument) {
                        track->setInstrumentPlugin(std::move(node));
                    } else {
                        track->addInsertPlugin(std::move(node));
                    }
                }
            } else {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Plugin Load Error", err);
            }
        }
    }

    std::function<void()> onModelChanged;

private:
    NimbusEngine& engine;
    std::vector<ListItem> items;
    std::set<juce::String> expandedMakers;
};

class SideBrowserComponent::FoldersModel : public juce::ListBoxModel {
public:
    FoldersModel() {
        int size = 0;
        if (auto* d = BinaryData::getNamedResource(DesignSystem::Iconography::Folder.toRawUTF8(), size)) {
            folderIcon = juce::Drawable::createFromImageData(d, size);
            if (folderIcon) folderIcon->replaceColour(juce::Colours::white, DesignSystem::Colors::TextSecondary);
        }
    }
    
    int getNumRows() override { return folders.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(6, 2);
        if (rowIsSelected) {
            g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.12f));
            g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
        }
        
        if (folderIcon) {
            auto iconBounds = bounds.removeFromLeft(16).withSizeKeepingCentre(14, 14);
            folderIcon->drawWithin(g, iconBounds.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        
        g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
        g.drawText(" " + folders[rowNumber].getFileName(), bounds.withTrimmedLeft(4), juce::Justification::centredLeft, true);
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row >= 0 && row < folders.size() && onFolderSelected) {
            onFolderSelected(folders[row]);
        }
    }
    
    std::vector<juce::File> folders;
    std::function<void(const juce::File&)> onFolderSelected;
    std::unique_ptr<juce::Drawable> folderIcon;
};

class SideBrowserComponent::FileItemsModel : public juce::ListBoxModel {
public:
    FileItemsModel(NimbusEngine& e) : engine(e) {
        int size = 0;
        if (auto* d = BinaryData::getNamedResource(DesignSystem::Iconography::Folder.toRawUTF8(), size)) {
            folderIcon = juce::Drawable::createFromImageData(d, size);
            if (folderIcon) folderIcon->replaceColour(juce::Colours::white, DesignSystem::Colors::TextSecondary);
        }
        if (auto* d = BinaryData::getNamedResource(DesignSystem::Iconography::Audio.toRawUTF8(), size)) {
            waveformIcon = juce::Drawable::createFromImageData(d, size);
            if (waveformIcon) waveformIcon->replaceColour(juce::Colours::white, DesignSystem::Colors::TextSecondary);
        }
    }
    
    void setDirectory(const juce::File& dir) {
        currentDir = dir;
        items.clear();
        
        // Add ".." if not root
        if (currentDir.getParentDirectory() != currentDir) {
            items.push_back({true, currentDir.getParentDirectory(), ".."});
        }
        
        if (currentDir.isDirectory()) {
            auto children = currentDir.findChildFiles(juce::File::findFilesAndDirectories, false);
            
            // Sort: directories first, then files alphabetically
            std::sort(children.begin(), children.end(), [](const juce::File& a, const juce::File& b) {
                if (a.isDirectory() != b.isDirectory()) return a.isDirectory();
                return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
            });
            
            for (const auto& f : children) {
                if (f.isDirectory()) {
                    items.push_back({true, f, f.getFileName()});
                } else {
                    auto ext = f.getFileExtension().toLowerCase();
                    if (ext == ".wav" || ext == ".mp3" || ext == ".aif" || ext == ".aiff" || ext == ".ogg" || ext == ".flac") {
                        items.push_back({false, f, f.getFileName()});
                    }
                }
            }
        }
        
        if (onModelChanged) onModelChanged();
    }
    
    struct FileItem {
        bool isDirectory;
        juce::File file;
        juce::String displayName;
    };
    
    int getNumRows() override { return items.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        if (rowNumber < 0 || rowNumber >= items.size()) return;
        auto& item = items[rowNumber];
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4, 1);
        
        if (rowIsSelected) {
            g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.15f));
            g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        }
        
        auto iconBounds = bounds.removeFromLeft(16).withSizeKeepingCentre(14, 14);
        
        if (item.isDirectory) {
            if (folderIcon) folderIcon->drawWithin(g, iconBounds.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        } else {
            if (waveformIcon) waveformIcon->drawWithin(g, iconBounds.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }
        
        g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
        g.drawText(item.displayName, bounds.withTrimmedLeft(8), juce::Justification::centredLeft, true);
    }
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= items.size()) return;
        auto& item = items[row];
        if (!item.isDirectory && onFileSelected) {
            onFileSelected(item.file);
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= items.size()) return;
        auto& item = items[row];
        if (item.isDirectory) {
            setDirectory(item.file);
        }
    }
    
    juce::var getDragSourceDescription(const juce::SparseSet<int>& selectedRows) override {
        if (selectedRows.isEmpty()) return {};
        int row = selectedRows[0];
        if (row < 0 || row >= items.size()) return {};
        if (items[row].isDirectory) return {}; // ONLY drag files
        return juce::var("AUDIO_FILE:" + items[row].file.getFullPathName());
    }
    
    std::function<void()> onModelChanged;
    std::function<void(const juce::File&)> onFileSelected;
    juce::File currentDir;
    
private:
    NimbusEngine& engine;
    std::vector<FileItem> items;
    std::unique_ptr<juce::Drawable> folderIcon, waveformIcon;
};

class SideBrowserComponent::StockEffectsModel : public juce::ListBoxModel {
public:
    StockEffectsModel(NimbusEngine& e) : engine(e) {}
    
    struct ListItem {
        bool isHeader = false;
        juce::String headerText;
        juce::String pluginName;
    };
    
    void updateList(bool isMidi) {
        items.clear();
        auto categories = StockPluginFactory::getCategories();
        for (const auto& cat : categories) {
            bool isMidiCat = (cat == "MIDI Effects");
            if (isMidi != isMidiCat) continue;
            
            if (isMidi) {
                auto plugins = StockPluginFactory::getPluginsInCategory(cat);
                for (const auto& p : plugins) {
                    items.push_back({false, "", p});
                }
            } else {
                items.push_back({true, cat, ""});
                if (expandedCats.count(cat) > 0) {
                    auto plugins = StockPluginFactory::getPluginsInCategory(cat);
                    for (const auto& p : plugins) {
                        items.push_back({false, "", p});
                    }
                }
            }
        }
    }

    int getNumRows() override { return items.size(); }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override {
        auto& item = items[rowNumber];
        auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4, 1);
        
        if (item.isHeader) {
            g.setColour(DesignSystem::Colors::TextSecondary.darker(0.2f));
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(11.0f).boldened());
            
            bool isExpanded = expandedCats.count(item.headerText) > 0;
            g.drawText(" " + item.headerText.toUpperCase(), 16, 0, width - 16, height, juce::Justification::centredLeft, true);
            
            float chevronX = 8.0f;
            float chevronY = height / 2.0f;
            juce::Path p;
            if (isExpanded) {
                p.addTriangle(chevronX - 3, chevronY - 2, chevronX + 3, chevronY - 2, chevronX, chevronY + 2);
            } else {
                p.addTriangle(chevronX - 2, chevronY - 3, chevronX - 2, chevronY + 3, chevronX + 2, chevronY);
            }
            g.fillPath(p);
        } else {
            if (rowIsSelected) {
                g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.15f));
                g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
            }
            g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
            g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
            
            g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.3f));
            g.fillEllipse(16.0f, height / 2.0f - 2.0f, 4.0f, 4.0f);
            
            g.setColour(rowIsSelected ? DesignSystem::Colors::TextPrimary : DesignSystem::Colors::TextSecondary);
            g.drawText(item.pluginName, 28, 0, width - 28, height, juce::Justification::centredLeft, true);
        }
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) {
            if (expandedCats.count(item.headerText) > 0) {
                expandedCats.erase(item.headerText);
            } else {
                expandedCats.insert(item.headerText);
            }
            if (onModelChanged) onModelChanged();
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= (int)items.size()) return;
        auto& item = items[row];
        if (item.isHeader) return;
        
        auto selectedTracks = engine.getTimelineProject().getSelectedTracks();
        if (!selectedTracks.isEmpty()) {
            int trackIndex = selectedTracks.getRange(0).getStart();
            auto track = engine.getMixer()->getTrack(trackIndex);
            if (track) {
                auto node = StockPluginFactory::createPlugin(item.pluginName);
                if (node) {
                    if (node->isMidiEffect()) track->addMidiInsertPlugin(std::move(node));
                    else track->addInsertPlugin(std::move(node));
                }
            }
        }
    }

    std::function<void()> onModelChanged;

private:
    NimbusEngine& engine;
    std::vector<ListItem> items;
    std::set<juce::String> expandedCats;
};
// --- PreviewPlayerComponent Implementation ---
PreviewPlayerComponent::PreviewPlayerComponent(NimbusEngine& e)
    : engine(e), thumbnail(512, engine.getFormatManager(), engine.getThumbnailCache()) {
    
    audioSourcePlayer.setSource(&transportSource);
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().addAudioCallback(&audioSourcePlayer);
    
    // Use solo (headphones) icon for the preview button
    int size = 0;
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Solo.toRawUTF8(), size)) {
        playIcon = juce::Drawable::createFromImageData(data, size);
        if (playIcon) playIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
    }
    // Active state icon (same icon, brighter)
    if (auto* data = BinaryData::getNamedResource(DesignSystem::Iconography::Solo.toRawUTF8(), size)) {
        pauseIcon = juce::Drawable::createFromImageData(data, size);
        if (pauseIcon) pauseIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::PrimaryAction);
    }

    playButton.setImages(playIcon.get());
    playButton.onClick = [this]() {
        if (transportSource.isPlaying()) stop();
        else play();
    };
    addAndMakeVisible(playButton);

    thumbnail.addChangeListener(this);
    transportSource.addChangeListener(this);
}

PreviewPlayerComponent::~PreviewPlayerComponent() {
    engine.getAudioDeviceManager().getJuceAudioDeviceManager().removeAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(nullptr);
    transportSource.setSource(nullptr);
}

void PreviewPlayerComponent::loadFile(const juce::File& file) {
    if (auto reader = engine.getFormatManager().createReaderFor(file)) {
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource = std::move(newSource);
        thumbnail.setSource(new juce::FileInputSource(file));
        play();
    }
}

void PreviewPlayerComponent::play() {
    transportSource.setPosition(0.0);
    transportSource.start();
    startTimerHz(30);
    if (pauseIcon) {
        playButton.setImages(pauseIcon.get());
    }
}

void PreviewPlayerComponent::stop() {
    transportSource.stop();
    stopTimer();
    if (playIcon) {
        playButton.setImages(playIcon.get());
    }
}

void PreviewPlayerComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &thumbnail) {
        repaint();
    } else if (source == &transportSource) {
        if (!transportSource.isPlaying() && playIcon) {
            playButton.setImages(playIcon.get());
            stopTimer();
        }
    }
}

void PreviewPlayerComponent::timerCallback() {
    repaint();
    if (transportSource.isPlaying()) {
        double pos = transportSource.getCurrentPosition();
        double limit = 0.0;
        if (readerSource && readerSource->getAudioFormatReader()) {
            limit = readerSource->getAudioFormatReader()->lengthInSamples / readerSource->getAudioFormatReader()->sampleRate;
        }
        
        double maxDuration = (limit > 60.0) ? 30.0 : 60.0;
        if (pos >= maxDuration) {
            stop();
        }
    }
}

void PreviewPlayerComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
    
    // Subtle top separator
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.4f));
    g.fillRect(4, 0, getWidth() - 8, 1);
    
    auto bounds = getLocalBounds().withTrimmedTop(1).reduced(4, 2);
    auto thumbBounds = bounds.withTrimmedLeft(22); // compact space for headphones button

    // Draw waveform background
    g.setColour(DesignSystem::Colors::ComponentBackground.withAlpha(0.5f));
    g.fillRoundedRectangle(thumbBounds.toFloat(), 3.0f);
    
    // Draw waveform
    if (thumbnail.getTotalLength() > 0.0) {
        g.setColour(DesignSystem::Colors::PrimaryAction.withAlpha(0.7f));
        thumbnail.drawChannels(g, thumbBounds.reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);
        
        // Playhead
        if (transportSource.isPlaying() || transportSource.getCurrentPosition() > 0.0) {
            float prop = (float)(transportSource.getCurrentPosition() / thumbnail.getTotalLength());
            float x = thumbBounds.getX() + prop * thumbBounds.getWidth();
            g.setColour(DesignSystem::Colors::TextPrimary);
            g.drawLine(x, (float)thumbBounds.getY(), x, (float)thumbBounds.getBottom(), 1.0f);
        }
    }
}

void PreviewPlayerComponent::resized() {
    auto bounds = getLocalBounds().withTrimmedTop(1).reduced(4, 2);
    playButton.setBounds(bounds.removeFromLeft(18).withSizeKeepingCentre(16, 16));
}

SideBrowserComponent::SideBrowserComponent(NimbusEngine& e) : engine(e), previewPlayer(e) {
    catModel = std::make_unique<CategoriesModel>();
    pluginModel = std::make_unique<PluginItemsModel>(engine);
    foldersModel = std::make_unique<FoldersModel>();
    fileModel = std::make_unique<FileItemsModel>(engine);
    fileModel->onFileSelected = [this](const juce::File& file) {
        previewPlayer.loadFile(file);
    };
    
    fileModel->onModelChanged = [this]() {
        itemsList.updateContent();
    };
    
    foldersModel->onFolderSelected = [this](const juce::File& f) {
        categoriesList.deselectAllRows();
        itemsList.setVisible(true);
        previewPlayer.setVisible(true); // Show preview when browsing folders
        fileModel->setDirectory(f);
        itemsList.setModel(fileModel.get());
        itemsList.updateContent();
        resized(); // Re-layout to accommodate preview player
    };
    
    pluginModel->onModelChanged = [this]() {
        pluginModel->updateList(false);
        itemsList.updateContent();
    };
    
    stockEffectsModel = std::make_unique<StockEffectsModel>(engine);
    stockEffectsModel->onModelChanged = [this]() {
        // Find which mode we are in based on category list selection
        int selectedRow = categoriesList.getSelectedRow();
        bool isMidi = false;
        if (selectedRow >= 0 && selectedRow < catModel->categories.size()) {
            if (catModel->categories[selectedRow] == "MIDI Devices") {
                isMidi = true;
            }
        }
        stockEffectsModel->updateList(isMidi);
        itemsList.updateContent();
    };
    
    catModel->onCategorySelected = [this](const juce::String& cat) {
        foldersList.deselectAllRows();
        
        // Hide preview player when browsing device/plugin categories
        previewPlayer.setVisible(false);
        previewPlayer.stop();
        
        itemsList.setVisible(true);
        if (cat == "Plugins") {
            pluginModel->updateList(false);
            itemsList.setModel(pluginModel.get());
        } else if (cat == "Audio Devices") {
            stockEffectsModel->updateList(false);
            itemsList.setModel(stockEffectsModel.get());
        } else if (cat == "MIDI Devices") {
            stockEffectsModel->updateList(true);
            itemsList.setModel(stockEffectsModel.get());
        } else {
            itemsList.setModel(nullptr);
        }
        itemsList.updateContent();
    };

    categoriesList.setModel(catModel.get());
    categoriesList.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    categoriesList.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    categoriesList.setRowHeight(28);
    addAndMakeVisible(categoriesList);
    
    foldersList.setModel(foldersModel.get());
    foldersList.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    foldersList.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    foldersList.setRowHeight(24);
    addAndMakeVisible(foldersList);
    
    addFolderButton.setButtonText("+ Add Folder");
    addFolderButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    addFolderButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addFolderButton.setColour(juce::TextButton::textColourOffId, DesignSystem::Colors::TextSecondary.withAlpha(0.7f));
    addFolderButton.getProperties().set("transparentBackground", true);
    addFolderButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>("Select a folder to add", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "");
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc) {
            auto result = fc.getResult();
            if (result.exists() && result.isDirectory()) {
                foldersModel->folders.push_back(result);
                foldersList.updateContent();
            }
        });
    };
    addAndMakeVisible(addFolderButton);
    
    itemsList.setModel(pluginModel.get());
    itemsList.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    itemsList.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    itemsList.setRowHeight(24);
    addAndMakeVisible(itemsList);
    
    // Resizer bar between the two columns
    columnResizer.currentWidth = leftColumnWidth;
    columnResizer.setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    columnResizer.onWidthChanged = [this](int newWidth) {
        leftColumnWidth = newWidth;
        resized();
    };
    addAndMakeVisible(columnResizer);
    
    itemsList.setVisible(false);
    
    searchBox.setTextToShowWhenEmpty("Search...", DesignSystem::Colors::TextSecondary.withAlpha(0.5f));
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    searchBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour(juce::TextEditor::textColourId, DesignSystem::Colors::TextPrimary);
    addAndMakeVisible(searchBox);
    searchBox.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(13.0f));
    addAndMakeVisible(previewPlayer);
    previewPlayer.setVisible(false); // Hidden by default, shown only when browsing folders
    
    startTimer(500); // Polling for scan completion

    int dataSize = 0;
    if (auto* data = BinaryData::getNamedResource("search_svg", dataSize)) {
        searchIcon = juce::Drawable::createFromImageData(data, dataSize);
        if (searchIcon) searchIcon->replaceColour(juce::Colours::black, DesignSystem::Colors::TextSecondary);
    }
}

SideBrowserComponent::~SideBrowserComponent() {
    stopTimer();
    itemsList.setModel(nullptr);
    categoriesList.setModel(nullptr);
}

void SideBrowserComponent::timerCallback() {
    if (!engine.getPluginManager().isScanning()) {
        stopTimer();
        // Force refresh
        pluginModel->updateList(false);
        itemsList.updateContent();
    }
}

void SideBrowserComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::AppBackground);
    
    // Left border
    g.setColour(DesignSystem::Colors::Divider.withAlpha(0.3f));
    g.fillRect(0, 0, 1, getHeight());
    
    // Draw search bar background
    auto searchBarBounds = getLocalBounds().removeFromTop(36).reduced(8, 5);
    g.setColour(DesignSystem::Colors::ComponentBackground.withAlpha(0.6f));
    g.fillRoundedRectangle(searchBarBounds.toFloat(), 4.0f);
    
    // Search icon
    if (searchIcon) {
        auto iconBounds = searchBarBounds.removeFromLeft(22).withSizeKeepingCentre(14, 14).toFloat();
        searchIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
    }
    
    if (!itemsList.isVisible()) {
        auto welcomeBounds = getLocalBounds().withTrimmedLeft(leftColumnWidth + 4).withTrimmedTop(36).reduced(16);
        g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.5f));
        g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(12.0f));
        
        auto descBounds = welcomeBounds.withSizeKeepingCentre(welcomeBounds.getWidth(), 60);
        g.drawFittedText("Select a category or folder\nto browse content.", descBounds, juce::Justification::centred, 3);
    }
    
    // Draw "PLACES" header
    int placesY = 36 + 4 + (28 * 3) + 4;
    g.setColour(DesignSystem::Colors::TextSecondary.withAlpha(0.4f));
    g.setFont(DesignSystem::Typography::getPrimaryFont().withHeight(10.0f).boldened());
    g.drawText("PLACES", 14, placesY, leftColumnWidth - 28, 14, juce::Justification::centredLeft, true);
}

void SideBrowserComponent::resized() {
    auto bounds = getLocalBounds();
    auto topBar = bounds.removeFromTop(36).reduced(8, 5);
    
    searchBox.setBounds(topBar.withTrimmedLeft(22).reduced(2, 0));
    
    bounds.removeFromTop(4); // spacing
    
    auto leftCol = bounds.removeFromLeft(leftColumnWidth);
    categoriesList.setBounds(leftCol.removeFromTop(28 * 3)); // Compact height for 3 categories
    
    leftCol.removeFromTop(22); // Space for PLACES header
    
    addFolderButton.setBounds(leftCol.removeFromTop(22).reduced(8, 0));
    foldersList.setBounds(leftCol);
    
    // Resizer bar
    columnResizer.setBounds(bounds.removeFromLeft(3));
    
    // Bottom preview player - only allocate space if visible
    if (previewPlayer.isVisible()) {
        previewPlayer.setBounds(bounds.removeFromBottom(36));
    }
    
    itemsList.setBounds(bounds);
}

} // namespace Nimbus::MainLayout
