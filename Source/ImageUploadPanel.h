/*
  ==============================================================================

    ImageUploadPanel.h
    Created: 25 Aug 2022 11:44:06am
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui
{
    class ImageUploadPanel : public juce::Component, Button::Listener

    {
    public:
        
        ImageUploadPanel(ImageUploadManager& ium, TextEditor& to) :
        imageUploadManager(ium),
        textOutput(to)
        {
            configureButton(imageUploadButton, "Upload Image");
            configureButton(scrollLeft, "<");
            configureButton(scrollRight, ">");
            addAndMakeVisible(imageComp);
            
            imageComp.setImage(juce::ImageCache::getFromMemory(BinaryData::imageUpload_png, BinaryData::imageUpload_pngSize));
                imageUploadManager.currentImage = File("/Users/max/plugInDev/distortionPlugInV2/images/imageUpload.png");
            //            std::cout << BinaryData::getNamedResourceOriginalFilename((BinaryData::namedResourceList[5]);
            imageUploaded = false;
            
            //initialise the neural network object
            
        }
        
        ~ImageUploadPanel()
        {
            imageUploadButton.removeListener(this);
            scrollLeft.removeListener(this);
            scrollRight.removeListener(this);
        }
        
        void configureButton(Button& button, const String& buttonText)
        {
            button.setButtonText(buttonText);
            button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
            button.setColour(TextButton::ColourIds::textColourOffId, Colours::gainsboro);
            button.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
            addAndMakeVisible(button);
            button.addListener(this);
        }
        
        void resized() override
        {
            const auto container = getLocalBounds();
            auto bounds = container;
            
            imageUploadButton.setBounds(bounds.removeFromBottom(container.proportionOfHeight(0.25f)).reduced(2.f, 5.f));
            scrollLeft.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.15f)).reduced(2.f, 5.f));
            scrollRight.setBounds(bounds.removeFromRight(container.proportionOfWidth(.15f)).reduced(2.f, 5.f));
            imageComp.setBounds(bounds.reduced(2.f, 10.f));
        }
        
        ImageUploadManager getImageUploadManager()
        {
            return imageUploadManager;
        }
        
        void setImage(Image newImage)
        {
            imageComp.setImage(newImage);
        }
        
        
    private:
        
        void buttonClicked(Button* button) override
        {
            if(button == &imageUploadButton)
            {
                fileChooser =  std::make_unique<FileChooser>(
                                                             "Select an image to upload"
                                                             );
                textOutput.setText("Analysing image and loading all images from folder...");
                
                
                fileChooser->launchAsync(FileBrowserComponent::canSelectFiles, [&](const FileChooser& chooser)
                                         {
                    const auto result = chooser.getResult();
                    uploadedImage = ImageCache::getFromFile(result);
                    imageComp.setImage(uploadedImage);
                    imageUploadManager.uploadImage(result);
                    textOutput.setText("Images loaded and analysis complete.\n\n" + imageUploadManager.analysisOutput);
                });
                imageUploaded = true;
            }
            
            if(button == &scrollLeft && imageUploaded == true)
            {
                const auto index = imageUploadManager.loadPrevImage();
                if(index != -1)
                {
                    Image newImage = ImageCache::getFromFile(imageUploadManager.allImages[index]);
                    imageComp.setImage(newImage);
                    imageUploadManager.currentImage = imageUploadManager.allImages[index];
                    textOutput.setText(imageUploadManager.analysisOutput);
                }
                else
                {
                    textOutput.setText("Could not load all images from parent folder");
                }
            }
            
            if(button == &scrollRight && imageUploaded == true)
            {
                const auto index = imageUploadManager.loadNextImage();
                if(index != -1)
                {
                    Image newImage = ImageCache::getFromFile(imageUploadManager.allImages[index]);
                    imageComp.setImage(newImage);
                    imageUploadManager.currentImage = imageUploadManager.allImages[index];
                    textOutput.setText(imageUploadManager.analysisOutput);
                }
                else
                {
                    textOutput.setText("Could not load all images from parent folder");
                }
            }
            
            //image analysis using neural network
        }
        
        std::unique_ptr<FileChooser> fileChooser;
        
        TextButton imageUploadButton, scrollRight, scrollLeft;
        
        ImageComponent imageComp;
        
        Image uploadedImage;
        
        String initImagePath;
        
        bool imageUploaded;
        
        ImageUploadManager& imageUploadManager;
        
        TextEditor& textOutput;
        
        //neural network object
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageUploadPanel)
    };

}

//


