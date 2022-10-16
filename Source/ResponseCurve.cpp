/*
  ==============================================================================

    ResponseCurve.cpp
    Created: 25 May 2022 12:57:00pm
    Author:  Max Ellis

  ==============================================================================
*/

#include "ResponseCurve.h"

enum ChainPositions
{
    lowCut,
    highCut
};

    ResponseCurve::ResponseCurve(DistortionProjAudioProcessor& p) : audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)
    {
        const auto& params = audioProcessor.getParameters();
        
        for(auto param : params)
        {
            param->addListener(this);
        }
        
        startTimerHz(120);
    }


    ResponseCurve::~ResponseCurve()
    {
        const auto& params = audioProcessor.getParameters();
        
        for(auto param : params)
        {
            param->removeListener(this);
        }
    }

    void ResponseCurve::parameterValueChanged(int parameterIndex, float newValue)
    {
        parametersChanged.set(true);
    }



    void ResponseCurve::timerCallback()
    {
        if(shouldShowFFT)
        {
            auto fftBounds = getLocalBounds().toFloat();
            auto sampleRate = audioProcessor.getSampleRate();
            
            
            leftPathProducer.process(fftBounds, sampleRate);
            rightPathProducer.process(fftBounds, sampleRate);
        }
        
        if(parametersChanged.compareAndSetBool(false, true))
        {
            auto chainSettings = getChainSettings(audioProcessor.apvts);
            
            monoChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);
            monoChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);

            auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
            auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
            
            updateCutFilter(monoChain.get<ChainPositions::lowCut>(), lowCutCoefficients);
            updateCutFilter(monoChain.get<ChainPositions::highCut>(), highCutCoefficients);
            
        }
        
        repaint();
    }

    void ResponseCurve::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().reduced(2.5f, 0.f);
        
        auto& lowcut = monoChain.get<ChainPositions::lowCut>();
        auto& highcut = monoChain.get<ChainPositions::highCut>();
        
        auto sampleRate = audioProcessor.getSampleRate();
        auto w = bounds.getWidth();
        
        std::vector<double> mags;
        
        mags.resize(w);
        
        for(int i = 0; i < w; ++i)
        {
            double mag = 1.f;
            auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
            
            if(!monoChain.isBypassed<ChainPositions::lowCut>()){
                mag *= lowcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
            }
            if(!monoChain.isBypassed<ChainPositions::highCut>()){
                mag *= highcut.coefficients->getMagnitudeForFrequency(freq, sampleRate);
            }
        
            
            mags[i] = Decibels::gainToDecibels(mag);
        }
        
        Path responseCurve;
        
        const double outputMin = bounds.getBottom();
        const double outputMax = bounds.getY();
        auto map = [outputMin, outputMax](double input)
        {
            return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };
        
        responseCurve.startNewSubPath(bounds.getX(), map(mags.front()));
        
        for(size_t i = 1; i<mags.size(); ++i)
        {
            responseCurve.lineTo(bounds.getX() + i, map(mags[i]));
        }
        
        if(shouldShowFFT)
        {
            auto leftChannelFFTPath = leftPathProducer.getPath();
            leftChannelFFTPath.applyTransform(AffineTransform().translation(bounds.getX(), bounds.getY()));
            
            g.setColour(Colours::skyblue);
            g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
            
            auto rightChannelFFTPath = rightPathProducer.getPath();
            rightChannelFFTPath.applyTransform(AffineTransform().translation(bounds.getX(), bounds.getY()));
            
            g.setColour(Colours::blueviolet);
            g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
        }
        
        g.setColour(Colours::white);
        g.strokePath(responseCurve, PathStrokeType(2.f));
    }

    juce::Rectangle<int> ResponseCurve::getRenderArea()
    {
        auto bounds = getLocalBounds();
        auto titleBarArea = bounds.removeFromTop(bounds.getHeight() * 0.1);
        auto dropDownArea = titleBarArea.removeFromLeft(titleBarArea.getWidth() * 0.33);
        auto metersArea = bounds.removeFromRight(bounds.getWidth() * 0.15);
        auto thumbnailArea = metersArea.removeFromTop(metersArea.getHeight() * 0.33);
        
        return bounds = bounds.removeFromTop(bounds.getHeight() * 0.5);
    }

    juce::Rectangle<int> ResponseCurve::getAnalysisArea()
    {
        auto bounds = getRenderArea();
        return bounds = bounds.reduced(22.5f, 15.f);
        
    }

   
