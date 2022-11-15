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
    currentImage = imageToLoad;
    parentDirectory = imageToLoad.getParentDirectory();
    getAllImages();
    //    imageClassifier.createPatch();
    //analyse current image
}

int ImageUploadManager::loadNextImage()
{
    if(allImages.isEmpty())
    {
        return -1;
    }
    const auto currentIndex = allImages.indexOf(currentImage);
    const auto nextIndex = currentIndex + 1 > allImages.size() - 1 ? 0 : currentIndex + 1;
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






using namespace std;

const std::string ImageClassifier::sentimentOutputFilename = "/Users/max/Downloads/visual-sentiment-analysis-main/predictions.csv";


ImageClassifier::ImageClassifier()
{
    char pythonFileName[] = "/Users/max/Downloads/visual-sentiment-analysis-main/predict.py";
    FILE* fp;

    Py_Initialize();
    fp = fopen(pythonFileName, "r");
    if(fp != NULL)
    {
        PyRun_SimpleFile(fp, pythonFileName);
    }
    fclose(fp);
    Py_Finalize();
    }

    ImageClassifier::~ImageClassifier()
    {
    //    Py_Finalize();

}

vector<string> ImageClassifier::getSentimentPredictions(string fname)
{
    vector<string> row;
    string line, word;

    fstream file (fname, ios::in);
    if(file.is_open())
    {
        getline(file, line);
        row.clear();
        stringstream str(line);
        while(getline(str, word, ','))
        {
            row.push_back(word);
        }
    }
    else
    {
        cout << "Could not open the file\n";
    }

    for(int j=0; j<row.size(); j++)
    {
        cout << row[j] << " ";
    }

    return row;
}


void ImageClassifier::createPatch()
{
    auto sentimentValues = getSentimentPredictions(sentimentOutputFilename);
    float negativeValue = stof(sentimentValues[0]);
    float neutralValue = stof(sentimentValues[1]);
    float positiveValue = stof(sentimentValues[2]);

//    auto settiings = getChainSettings();

    if(negativeValue >= 0.5)
    {
        //set low dist type to fuzzz
        //set mid dist type to tube
    }
    else{
        // set low dist to lofi
    }

    if(neutralValue >= 0.5)
    {

    }
    else{

    }

    if(positiveValue >= 0.5)
    {

    }
    else{

    }

    //        set low drive getDriveMapping(negativeValue);
    //        set low drive getDriveMapping(neutralValue);
    //        set low drive getDriveMapping(positiveValue);

}

float ImageClassifier::getDriveMapping(float sentimentValue)
    {
    auto mapping = jmap(sentimentValue, 0.f, 1.f, 0.f, 20.f);

    return mapping;

}
