/*
  ==============================================================================

    PathProducer.h
    Created: 22 Aug 2022 12:26:46pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SingleChannelSampleFifo.h"
#include "SpectrumPathGenerator.h"
#include "FFTDataGenerator.h"
#include "PluginProcessor.h"

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<DistortionProjAudioProcessor::BlockType>& scsf) :
    leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order8192);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    
//    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    
    void process(juce::Rectangle<float> fftBounds, double sampleRate)
    {
        juce::AudioBuffer<float> tempIncomingBuffer;
        
        while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
        {
            if(leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
            {
                auto size = tempIncomingBuffer.getNumSamples();
                            
                jassert(size <= monoBuffer.getNumSamples());
                size = juce::jmin(size, monoBuffer.getNumSamples());
                
                auto writePointer = monoBuffer.getWritePointer(0, 0);
                auto readPointer = monoBuffer.getReadPointer(0, size);
                
                
//                std::copy(readPointer,
//                          readPointer + (monoBuffer.getNumSamples() - size),
//                          writePointer);

    //            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
    //                                              monoBuffer.getReadPointer(0, size),
    //                                              monoBuffer.getNumSamples() - size);
                
                juce::FloatVectorOperations::copy(writePointer,
                                                  readPointer,
                                                  monoBuffer.getNumSamples() - size);

                juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                                  tempIncomingBuffer.getReadPointer(0, 0),
                                                  size);
                
                leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
            }
        }
        
        const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
        const auto binWidth = sampleRate / (double)fftSize;
        
        while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
        {
            std::vector<float> fftData;
            if(leftChannelFFTDataGenerator.getFFTData(fftData))
            {
                pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
            }
        }
        
        while(pathProducer.getNumPathsAvailable())
        {
            pathProducer.getPath(leftChannelFFTPath);
        }
        
    }
    
    
    juce::Path getPath() {return leftChannelFFTPath;}
    
private:
    SingleChannelSampleFifo<DistortionProjAudioProcessor::BlockType>* leftChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

    SpectrumPathGenerator<juce::Path> pathProducer;

    juce::Path leftChannelFFTPath;
};
