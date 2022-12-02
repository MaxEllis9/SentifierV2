/*
  ==============================================================================

    ImageClassifier.h
    Created: 20 Nov 2022 6:30:47pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <fstream>

class ImageClassifier
{
public:
    static const std::string sentimentOutputFilename;
    
    static const File defaultDirectory;
            
    static const File sentimentAnalysisDirectory;
    
    static const File predictionsFile;

    ImageClassifier();

    ~ImageClassifier();
        
    std::vector<std::vector<std::string>> getSentimentPredictions(std::string fname);
    
    void classifyImage(File image);
    
    String createPatch(File image);

    float getDriveMapping(float sentimentValue);
    
    void setDistortionMode(std::string distBandMode, int sentimentClassifier);
    
    void setSentiemnt(int i, int* sentiment, std::vector<std::vector<std::string>> sentimentData);
    
    AudioProcessorValueTreeState* classifierTreeP;
    
private:

    enum DistType
    {
        none,
        softClip,
        hardClip,
        saturation,
        fuzzDistortion,
        tubeDistortion,
        lofiDistortion
    };
    
    enum SentimentClassifier
    {
        awe,
        fear,
        love,
        violent,
        happy,
        sad
    };

};


