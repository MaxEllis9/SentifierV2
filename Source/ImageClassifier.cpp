/*
  ==============================================================================

    ImageClassifier.cpp
    Created: 20 Nov 2022 6:30:47pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "ImageClassifier.h"

using namespace std;

const File ImageClassifier::defaultDirectory(File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory)
                                             .getChildFile(ProjectInfo::companyName)
                                             .getChildFile(ProjectInfo::projectName)
                                             );

const File ImageClassifier::sentimentAnalysisDirectory("/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master");

const File ImageClassifier::predictionsFile("/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master/output.txt");

ImageClassifier::ImageClassifier()
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
    
    if(!std::ifstream(predictionsFile.getFullPathName().toStdString()))
    {
        auto directoryPath = defaultDirectory.getFullPathName();
        directoryPath.append("/predictions.csv", 20);
        std::ofstream predictionsFile(directoryPath.toStdString());
        if(!predictionsFile)
        {
            {
                DBG("Could not create predictions file");
                jassertfalse;
            }
        }
        predictionsFile.close();
    }
    
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master')");
    PyRun_SimpleString("sys.argv[0] = 'label_image.py'");
    PyRun_SimpleString("sys.argv.append('')");
}

ImageClassifier::~ImageClassifier()
{
    Py_Finalize();
}

vector<vector<string>> ImageClassifier::getSentimentPredictions(string fname)
{
    vector<vector<string>> content;
    vector<string> row;
    string line, word;
     
    fstream file (fname, ios::in);
    if(file.is_open())
    {
        while(getline(file, line))
        {
            row.clear();
     
            stringstream str(line);
     
            while(getline(str, word, ','))
                row.push_back(word);
            content.push_back(row);
        }
    }
    else
        cout<<"Could not open the file\n";
     
//    for(int i=0;i<content.size();i++)
//    {
//        for(int j=0;j<content[i].size();j++)
//        {
//            cout<<content[i][j]<<" ";
//        }
//        cout<<"\n";
//    }
    file.close();
     
    return content;
}

void ImageClassifier::classifyImage(File image)
{
    string argString = "sys.argv[1] = '" + image.getFullPathName().toStdString() + "'";
    cout << argString << endl;

    const char* cArgString = argString.c_str();
       
    PyRun_SimpleString(cArgString);
    PyRun_SimpleString("exec(open('/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master/src/label_image.py').read())");

}



String ImageClassifier::createPatch(File image)
{
    String output = "Image classification:\n";
    
    classifyImage(image);
//
    auto sentimentValues = getSentimentPredictions(predictionsFile.getFullPathName().toStdString());

    float firstDrive = sentimentValues.empty() ? 0 : stof(sentimentValues[0][1]);
    float secondDrive = sentimentValues.empty() ? 0 : stof(sentimentValues[1][1]);
    float thirdDrive = sentimentValues.empty() ? 0 : stof(sentimentValues[2][1]);
    
    int firstSentiment = 0, secondSentiment = 0, thirdSentiment = 0;
    
    setSentiemnt(2, &thirdSentiment, sentimentValues);
    setSentiemnt(0, &firstSentiment, sentimentValues);
    setSentiemnt(1, &secondSentiment, sentimentValues);


//    0 None
//    1 Soft Clip
//    2 Hard Clip
//    3 Saturation
//    4 Fuzz Distortion
//    5 Tube Distortion
//    6 Lofi Distortion
    
    setDistortionMode("lowDistortion mode", thirdSentiment);
    setDistortionMode("midDistortion mode", firstSentiment);
    setDistortionMode("highDistortion mode", secondSentiment);
    
    classifierTreeP->getParameterAsValue("lowdrive").setValue(getDriveMapping(thirdDrive));
    classifierTreeP->getParameterAsValue("middrive").setValue(getDriveMapping(firstDrive));
    classifierTreeP->getParameterAsValue("highdrive").setValue(getDriveMapping(secondDrive));
    
    fstream humanFile("/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master/humanOutput.txt", ios::in);
    if(humanFile.is_open())
    {
        string line;
        while (getline(humanFile, line)) {
            String newLine = line;
            output.operator+=("    ");
            output.operator+=(newLine);
            output.operator+=("\n");
        }
    }
    
    humanFile.close();
    
    return output;

}

void ImageClassifier::setDistortionMode(string distBandMode, int sentimentClassifier)
{
    switch (sentimentClassifier) {
        case SentimentClassifier::awe :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::hardClip);
            break;
            
        case SentimentClassifier::fear :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::tubeDistortion);
            break;
            
        case SentimentClassifier::sad :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::lofiDistortion);
            break;
            
        case SentimentClassifier::happy :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::softClip);
            break;
            
        case SentimentClassifier::violent :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::fuzzDistortion);
            break;
            
        case SentimentClassifier::love :
            classifierTreeP->getParameterAsValue(distBandMode).setValue(DistType::saturation);
            break;
        
        default:
            break;
    }
}

void ImageClassifier::setSentiemnt(int i, int* sentiment, vector<vector<string>> sentimentData)
{
    if( sentimentData[i][0] == "awe")
    {
        *sentiment = SentimentClassifier::awe;
    }
    else if (sentimentData[i][0] == "fear")
    {
        *sentiment = SentimentClassifier::fear;
    }
    else if (sentimentData[i][0] == "sad")
    {
        *sentiment = SentimentClassifier::sad;
    }
    else if (sentimentData[i][0] == "happy")
    {
        *sentiment = SentimentClassifier::happy;
    }
    else if (sentimentData[i][0] == "violent")
    {
        *sentiment = SentimentClassifier::violent;
    }
    else if (sentimentData[i][0] == "love")
    {
        *sentiment = SentimentClassifier::love;
    }
}

float ImageClassifier::getDriveMapping(float sentimentValue)
{
    auto mapping = jmap(sentimentValue, 0.f, 1.f, 0.f, 20.f);

    return mapping;

}
