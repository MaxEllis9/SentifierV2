/*
  ==============================================================================

    PresetManager.cpp
    Created: 21 Aug 2022 2:43:21pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "PresetManager.h"

namespace PresetBackend
{

    const File PresetManager::defaultDirectory(File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory)
                                               .getChildFile(ProjectInfo::companyName)
                                               .getChildFile(ProjectInfo::projectName)
    );

    const String PresetManager::fileExtension("preset");
    const String PresetManager::presetNameProperty("presetName");
    String PresetManager::imageFilePathProperty("currentImagePath");

PresetManager::PresetManager(AudioProcessorValueTreeState& apvts, ImageUploadBackend::ImageUploadManager& ium) :
        valueTreeState(apvts), imageUploadManager(ium)
    {
        if(!defaultDirectory.exists())
        {
            const auto result = defaultDirectory.createDirectory();
            if(result.failed())
            {
                DBG("Could not create preset directory: " + result.getErrorMessage());
                jassertfalse;
            }
        }
        
        valueTreeState.state.addListener(this);
        currentPreset.referTo(valueTreeState.state.getPropertyAsValue(presetNameProperty, nullptr));
        currentImage.referTo(valueTreeState.state.getPropertyAsValue(imageFilePathProperty, nullptr));
        
        savePreset("init");
        
    }

    void PresetManager::savePreset(const String& presetName)
    {
        if(presetName.isEmpty())
        {
            return;
        }
        currentPreset.setValue(presetName);
        currentImage.setValue(imageUploadManager.getCurrentImagePath());
        

        const auto xml = valueTreeState.copyState().createXml();
        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + fileExtension);
        if(!xml->writeTo(presetFile))
        {
            DBG("Could not create preset file: " + presetFile.getFullPathName());
            jassertfalse;
        }
    }

    void PresetManager::deletePreset(const String& presetName)
    {
        if(presetName.isEmpty())
        {
            return;
        }
        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + fileExtension);
        if(!presetFile.exists())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
            jassertfalse;
            return;
        }
        if(!presetFile.deleteFile())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " could not be deleted");
            jassertfalse;
            return;
        }
        currentPreset.setValue("");
        currentImage.setValue("");
    }

    void PresetManager::loadPreset(const String& presetName)
    {
        
        if(presetName.isEmpty())
        {
            return;
        }
        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + fileExtension);
        if(!presetFile.existsAsFile())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
            jassertfalse;
            return;
        }
        XmlDocument xmlDocument {presetFile};
        const auto valueTreeToLoad = ValueTree::fromXml(*xmlDocument.getDocumentElement());
        
        valueTreeState.replaceState(valueTreeToLoad);
        currentPreset.setValue(presetName);
        currentImage.setValue(valueTreeToLoad.getProperty("currentImagePath"));
    }

    int PresetManager::loadNextPreset()
    {
        const auto allPresets = getAllPresets();
        if(allPresets.isEmpty())
        {
            return -1;
        }
        const auto currentIndex = allPresets.indexOf(currentPreset.toString());
        const auto nextIndex = currentIndex + 1 > (allPresets.size() - 1) ? 0 : currentIndex + 1;
        loadPreset(allPresets.getReference(nextIndex));
        return nextIndex;
    }

    int PresetManager::loadPrevPreset()
    {
        const auto allPresets = getAllPresets();
        if(allPresets.isEmpty())
        {
            return -1;
        }
        const auto currentIndex = allPresets.indexOf(currentPreset.toString());
        const auto prevIndex = currentIndex - 1 < 0 ? allPresets.size() - 1 : currentIndex - 1;
        loadPreset(allPresets.getReference(prevIndex));
        return prevIndex;
    }

    StringArray PresetManager::getAllPresets() const
    {
        StringArray presets;
        const auto fileArray = defaultDirectory.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*." + fileExtension);
        for(const auto& file : fileArray)
        {
            presets.add(file.getFileNameWithoutExtension());
        }
        return presets;
    }

    String PresetManager::getCurrentPreset() const
    {
        return currentPreset.toString();
    }

    void PresetManager::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
    {
        currentPreset.referTo(treeWhichHasBeenChanged.getPropertyAsValue(presetNameProperty, nullptr));
        currentImage.referTo(treeWhichHasBeenChanged.getPropertyAsValue(imageFilePathProperty, nullptr));
        
    }

    String PresetManager::getCurrentImageString()
    {
        return currentImage.toString();
    }



}
