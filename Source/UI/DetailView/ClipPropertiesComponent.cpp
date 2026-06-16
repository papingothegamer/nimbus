#include "ClipPropertiesComponent.h"
#include "UI/DesignSystem/Colors.h"
#include "UI/DesignSystem/Typography.h"

namespace Nimbus::DetailView {

ClipPropertiesComponent::ClipPropertiesComponent(NimbusEngine& e) : engine(e) {
    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextPrimary);
    titleLabel.setText("Clip", juce::dontSendNotification);

    addAndMakeVisible(nameLabel);
    nameLabel.setFont(DesignSystem::Typography::getPrimaryFont());
    nameLabel.setColour(juce::Label::textColourId, DesignSystem::Colors::TextSecondary);

    addAndMakeVisible(startLabel);
    addAndMakeVisible(lengthLabel);

    // Audio controls
    addChildComponent(warpButton);
    addChildComponent(transposeSlider);
    addChildComponent(transposeLabel);
    transposeLabel.setText("Transpose", juce::dontSendNotification);
    addChildComponent(gainSlider);
    addChildComponent(gainLabel);
    gainLabel.setText("Gain", juce::dontSendNotification);

    // MIDI controls
    addChildComponent(loopButton);
    addChildComponent(signatureLabel);
    signatureLabel.setText("Sig: 4/4", juce::dontSendNotification);
    addChildComponent(grooveBox);
    grooveBox.addItem("None", 1);
    grooveBox.setSelectedId(1);

    setMidiMode(true);
}

void ClipPropertiesComponent::paint(juce::Graphics& g) {
    g.fillAll(DesignSystem::Colors::ModuleBackground);
    
    // Right border separator
    g.setColour(DesignSystem::Colors::Divider);
    g.fillRect(getWidth() - 1, 0, 1, getHeight());
}

void ClipPropertiesComponent::resized() {
    auto bounds = getLocalBounds().reduced(10);
    
    titleLabel.setBounds(bounds.removeFromTop(20));
    nameLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);

    startLabel.setBounds(bounds.removeFromTop(20));
    lengthLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);

    if (isMidiMode) {
        loopButton.setBounds(bounds.removeFromTop(24));
        signatureLabel.setBounds(bounds.removeFromTop(20));
        grooveBox.setBounds(bounds.removeFromTop(24));
    } else {
        warpButton.setBounds(bounds.removeFromTop(24));
        transposeLabel.setBounds(bounds.removeFromTop(20));
        transposeSlider.setBounds(bounds.removeFromTop(24));
        gainLabel.setBounds(bounds.removeFromTop(20));
        gainSlider.setBounds(bounds.removeFromTop(24));
    }
}

void ClipPropertiesComponent::setMidiMode(bool isMidi) {
    isMidiMode = isMidi;
    
    warpButton.setVisible(!isMidi);
    transposeSlider.setVisible(!isMidi);
    transposeLabel.setVisible(!isMidi);
    gainSlider.setVisible(!isMidi);
    gainLabel.setVisible(!isMidi);
    
    loopButton.setVisible(isMidi);
    signatureLabel.setVisible(isMidi);
    grooveBox.setVisible(isMidi);
    
    resized();
}

void ClipPropertiesComponent::updateClipInfo(const juce::String& name, double startSamples, double lengthSamples) {
    nameLabel.setText(name, juce::dontSendNotification);
    
    double sampleRate = engine.getTransport().getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    
    double startSeconds = startSamples / sampleRate;
    double lengthSeconds = lengthSamples / sampleRate;
    
    startLabel.setText("Start: " + juce::String(startSeconds, 2) + "s", juce::dontSendNotification);
    lengthLabel.setText("Len: " + juce::String(lengthSeconds, 2) + "s", juce::dontSendNotification);
}

} // namespace Nimbus::DetailView
