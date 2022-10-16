/*
  ==============================================================================

    ResponseCurve.h
    Created: 25 May 2022 12:56:52pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PathProducer.h"
#include "PluginProcessor.h"

    struct ResponseCurve : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
    {
    public:
        ResponseCurve(DistortionProjAudioProcessor&);
        ~ResponseCurve();

        //==============================================================================
        void timerCallback() override;
        void paint (juce::Graphics&) override;
        
        void parameterValueChanged (int parameterIndex, float newValue) override;

        void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {}
        
        void toggleAnalysisEnablement(bool enabled)
        {
            shouldShowFFT = enabled;
        }
        
    private:
        using Filter = dsp::IIR::Filter<float>;
        using MonoChain = dsp::ProcessorChain<Filter, Filter>;
        
        DistortionProjAudioProcessor& audioProcessor;
        juce::Atomic<bool> parametersChanged {false};
        
        MonoChain monoChain;
        
        juce::Rectangle<int> getAnalysisArea();
        juce::Rectangle<int> getRenderArea();
        
        PathProducer leftPathProducer, rightPathProducer;
        
        bool shouldShowFFT = true;
        
    };

