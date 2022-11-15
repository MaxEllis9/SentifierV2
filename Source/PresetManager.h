/*
  ==============================================================================

    PresetManager.h
    Created: 21 Aug 2022 2:43:21pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ImageUploaderManager.h"
#include "ImageUploadPanel.h"

class PresetManager : ValueTree::Listener
{
public:
    
    static const File defaultDirectory;
    static const String fileExtension;
    static const String presetNameProperty;
    static String imageFilePathProperty;
    
    PresetManager(AudioProcessorValueTreeState& apvts, ImageUploadManager& ium);
    
    void savePreset(const String& presetName);
    void deletePreset(const String& presetName);
    void loadPreset(const String& presetName);
    int loadNextPreset();
    int loadPrevPreset();
    StringArray getAllPresets() const;
    String getCurrentPreset() const;
    String getCurrentImageString();
    
private:
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
    
    Value currentPreset;
    Value currentImage;
    AudioProcessorValueTreeState& valueTreeState;
    ImageUploadManager& imageUploadManager;
};
