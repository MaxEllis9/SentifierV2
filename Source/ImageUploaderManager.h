/*
  ==============================================================================

    ImageUploaderManager.h
    Created: 25 Aug 2022 11:44:21am
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace ImageUploadBackend
{

    class ImageUploadManager
    {
    public:
        
        static File parentDirectory;
        
        ImageUploadManager();
        ~ImageUploadManager();
        
        int loadNextImage();
        int loadPrevImage();
        void uploadImage(File imageToLoad);
        void getAllImages();
        String getCurrentImagePath();

        Array<File> allImages;
        File currentImage;

        
    private:
        
        void setParentDirectory();
        
    };
}
