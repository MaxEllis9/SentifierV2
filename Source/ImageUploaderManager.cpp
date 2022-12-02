/*
==============================================================================

ImageUploaderManager.cpp
Created: 25 Aug 2022 11:44:21am
Author:  Max Ellis

==============================================================================
*/

#include "ImageUploaderManager.h"

File ImageUploadManager::parentDirectory(File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));

ImageUploadManager::ImageUploadManager()
{
    currentImage = File("/Users/max/plugInDev/distortionPlugIn/images/imageUpload.png");
}

ImageUploadManager::~ImageUploadManager()
{
}


void ImageUploadManager::uploadImage(File imageToLoad)
{
    imageClassifier.classifierTreeP = imageManagerTreeP;
    currentImage = imageToLoad;
    parentDirectory = imageToLoad.getParentDirectory();
    getAllImages();
    analysisOutput = imageClassifier.createPatch(currentImage);
}

int ImageUploadManager::loadNextImage()
{
    if(allImages.isEmpty())
    {
        return -1;
    }
    const auto currentIndex = allImages.indexOf(currentImage);
    const auto nextIndex = currentIndex + 1 > allImages.size() - 1 ? 0 : currentIndex + 1;
    analysisOutput = imageClassifier.createPatch(allImages[nextIndex]);
    return nextIndex;
}

int ImageUploadManager::loadPrevImage()
{
    if(allImages.isEmpty())
    {
        return -1;
    }
    const auto currentIndex = allImages.indexOf(currentImage);
    const auto prevIndex = currentIndex - 1 < 0 ? allImages.size() - 1 : currentIndex - 1;
    analysisOutput = imageClassifier.createPatch(allImages[prevIndex]);
    return prevIndex;
}

void ImageUploadManager::getAllImages()
{
    const auto tempImgArr1 = parentDirectory.findChildFiles(File::TypesOfFileToFind::findFiles, true, "*.jpg");
    const auto tempImgArr2 = parentDirectory.findChildFiles(File::TypesOfFileToFind::findFiles, true, "*.jpeg");
    const auto tempImgArr3 = parentDirectory.findChildFiles(File::TypesOfFileToFind::findFiles, true, "*.png");
    allImages = tempImgArr1;
    allImages.addArray(tempImgArr2);
    allImages.addArray(tempImgArr3);
}

void ImageUploadManager::setParentDirectory()
{
    parentDirectory = currentImage.getParentDirectory();
}

String ImageUploadManager::getCurrentImagePath()
{
    return currentImage.getFullPathName();
}
