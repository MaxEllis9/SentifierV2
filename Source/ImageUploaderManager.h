/*
  ==============================================================================

    ImageUploaderManager.h
    Created: 25 Aug 2022 11:44:21am
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <fstream>


// python predict.py images_list.txt --model vgg19_finetuned_all --batch-size 64 > predictions.csv

class ImageClassifier
{
public:
    static const std::string sentimentOutputFilename;

    ImageClassifier();

    ~ImageClassifier();

    std::vector<std::string> getSentimentPredictions(std::string fname);

    void createPatch();

    float getDriveMapping(float sentimentValue);

    //    const char pythonFileName[];
    //
    //    FILE* pythonFile;
private:


};




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
    
    //        ImageClassifier imageClassifier;
    
private:
    
    void setParentDirectory();
    
};

