/*
  ==============================================================================

    PresetPanel.h
    Created: 21 Aug 2022 2:19:18pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui
{
    class PresetPanel : public Component, Button::Listener, ComboBox::Listener
    {
    public:
        PresetPanel(PresetBackend::PresetManager& pm, Gui::ImageUploadPanel& iup) : presetManager(pm), imageUploadPanel(iup)
        {
            configureButton(saveButton, "Save");
            configureButton(deleteButton, "Delete");
            configureButton(prevButton, "<");
            configureButton(nextButton, ">");
            configureButton(initButton, "init");
            
            presetList.setTextWhenNothingSelected("No preset selected");
            presetList.setMouseCursor(MouseCursor::PointingHandCursor);
            presetList.setColour(ComboBox::ColourIds::backgroundColourId, Colours::black);
            presetList.setColour(ComboBox::ColourIds::textColourId, Colours::gainsboro);
            
            comboBoxLnF.setColour(PopupMenu::ColourIds::backgroundColourId, Colours::black);
            comboBoxLnF.setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, Colour(102, 25, 30));
            comboBoxLnF.setColour(PopupMenu::ColourIds::textColourId, Colours::gainsboro);
            
            presetList.setLookAndFeel(&comboBoxLnF);
            
            addAndMakeVisible(presetList);
            presetList.addListener(this);
            
            loadPresetList();
            
        }
        
        ~PresetPanel()
        {
            saveButton.removeListener(this);
            deleteButton.removeListener(this);
            prevButton.removeListener(this);
            nextButton.removeListener(this);
            presetList.removeListener(this);
            initButton.removeListener(this);
            presetList.setLookAndFeel(nullptr);

        }
        
        void resized() override
        {
            const auto container = getLocalBounds();
            auto bounds = container.reduced(4.f, 0.f);
            
            initButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.165f)).reduced(2.f, 5.f));
            saveButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.165f)).reduced(2.f, 5.f));
            prevButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.085f)).reduced(2.f, 5.f));
            presetList.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.335f)).reduced(2.f, 5.f));
            nextButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.085f)).reduced(2.f, 5.f));
            deleteButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.165f)).reduced(2.f, 5.f));
        }
        
        PresetBackend::PresetManager getPresetManager()
        {
            return presetManager;
        }
        
        
    private:
        
        void buttonClicked(Button* button) override
        {
            if(button == &saveButton)
            {
                fileChooser =  std::make_unique<FileChooser>(
                    "Please enter the name of the preset to save",
                    PresetBackend::PresetManager::defaultDirectory,
                    "*." + PresetBackend::PresetManager::fileExtension
                );
                fileChooser->launchAsync(FileBrowserComponent::saveMode, [&](const FileChooser& chooser)
                {
                    const auto result = chooser.getResult();
                    presetManager.savePreset(result.getFileNameWithoutExtension());
                    loadPresetList();
                });
            }
            if(button == &prevButton)
            {
                const auto index = presetManager.loadPrevPreset();
                presetList.setSelectedItemIndex(index, dontSendNotification);
                updateImage(presetManager.getCurrentImageString());
            }
            if(button == &nextButton)
            {
                const auto index = presetManager.loadNextPreset();
                presetList.setSelectedItemIndex(index, dontSendNotification);
                updateImage(presetManager.getCurrentImageString());
            }
            if(button == &deleteButton)
            {
                if(presetManager.getCurrentPreset() != "init")
                {
                    if(AlertWindow::showOkCancelBox(MessageBoxIconType::QuestionIcon, "Delete Preset?", "Are you sure you want to delete " + presetManager.getCurrentPreset() + "?"))
                    {
                        presetManager.deletePreset(presetManager.getCurrentPreset());
                        loadPresetList();
                    }
                }
            }
            if(button == &initButton)
            {
                presetManager.loadPreset("init");
                loadPresetList();
                updateImage(presetManager.getCurrentImageString());
            }
        }
        
        void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override
        {
            if (comboBoxThatHasChanged == &presetList)
                {
                    presetManager.loadPreset(presetList.getItemText(presetList.getSelectedItemIndex()));
                    updateImage(presetManager.getCurrentImageString());
                }
        }
        
        
        void configureButton(Button& button, const String& buttonText)
        {
            button.setButtonText(buttonText);
            button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
            button.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
            button.setColour(TextButton::ColourIds::textColourOffId, Colours::gainsboro);

            addAndMakeVisible(button);
            button.addListener(this);
            
        }
        
        void loadPresetList()
        {
            presetList.clear(dontSendNotification);
            const auto allPresets =  presetManager.getAllPresets();
            const auto currentPreset = presetManager.getCurrentPreset();
            presetList.addItemList(allPresets, 1);
            presetList.setSelectedItemIndex(allPresets.indexOf(currentPreset), dontSendNotification);
        }
        
        void updateImage(String imageFilePath)
        {
            File newImageFile = File(imageFilePath);
            auto newImage = ImageCache::getFromFile(newImageFile);
            imageUploadPanel.setImage(newImage);
        }
        
        PresetBackend::PresetManager& presetManager;
        
        Gui::ImageUploadPanel& imageUploadPanel;
        
        TextButton saveButton, deleteButton, prevButton, nextButton, initButton;
        ComboBox presetList;
        
        std::unique_ptr<FileChooser> fileChooser;
        
        LookAndFeel_V4 comboBoxLnF;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
        
    };
}
