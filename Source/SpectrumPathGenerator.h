/*
  ==============================================================================

    SpectrumPathGenerator.h
    Created: 22 Aug 2022 12:26:34pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Fifo.h"

template<typename PathType>
struct SpectrumPathGenerator
{

    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity)
    {
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();

        int numBins = (int)fftSize / 2;

        PathType p;
        p.preallocateSpace(3 * (int)fftBounds.getWidth());

        auto map = [bottom, top, negativeInfinity](float v)
        {
            return juce::jmap(v,
                              negativeInfinity,
                              0.f,
                              (float)bottom,
                              top);
        };

        auto y = map(renderData[0]);
        jassert(!std::isnan(y) && !std::isinf(y));

        p.startNewSubPath(0, y);

        const int pathResolution = 2; //you can draw line-to's every 'pathResolution' pixels

        for(int binNum=1; binNum<numBins; binNum += pathResolution)
        {
            y = map(renderData[binNum]);

            jassert(!std::isnan(y) && !std::isinf(y));

            if(!std::isnan(y) && !std::isinf(y)){
                auto binFreq = binNum * binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX * width);
                p.lineTo(binX, y);
            }
        }

        pathFifo.push(p);
    }

    int getNumPathsAvailable() const
    {
        return pathFifo.getNumAvailableForReading();
    }

    bool getPath(PathType& path)
    {
        return pathFifo.pull(path);
    }

private:
    Fifo<PathType> pathFifo;
};
