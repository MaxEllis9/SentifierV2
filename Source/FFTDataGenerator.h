/*
  ==============================================================================

    FFTDataGenerator.h
    Created: 22 Aug 2022 12:26:22pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

template<typename BlockType>
struct FFTDataGenerator
{
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();
        
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());
        
        //first apply a windowing function to the data
        window->multiplyWithWindowingTable (fftData.data(), fftSize);
        
        //then render FFt data
        forwardFFT->performFrequencyOnlyForwardTransform (fftData.data());
        
        int numBins = (int)fftSize / 2;
        
        //normalise fft values
        for(int i=0; i<numBins; ++i)
        {
            fftData[i] /= (float)numBins;
        }
        
        //convert to dBs
        for(int i=0; i<numBins; ++i)
        {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }
        
        fftDataFifo.push(fftData);
    }
    
    void changeOrder(FFTOrder newOrder)
    {
        //when you change order, recreate the window, forwardfft, fifo, fftdata
        //also reset Fifo index
        //things that need recreating should be created on the heap using std::make_unique<>
        
        order = newOrder;
        auto fftSize = getFFTSize();
        
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
        
        fftData.clear();
        fftData.resize(fftSize * 2, 0);
        
        fftDataFifo.prepare(fftData.size());
    }
    
    //==============================================================================
    int getFFTSize() const {return 1 << order;}
    int getNumAvailableFFTDataBlocks() const {return fftDataFifo.getNumAvailableForReading();}
    //==============================================================================
    bool getFFTData(BlockType& fftData) {return fftDataFifo.pull(fftData);}
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    
    Fifo<BlockType> fftDataFifo;
};
