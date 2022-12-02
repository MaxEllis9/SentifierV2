/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SingleChannelSampleFifo.h"
#include "PresetManager.h"
#include "DistortionBand.h"
#include "ImageUploaderManager.h"
#include "ChainSettings.h"


ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts);

using Filter = dsp::IIR::Filter<float>;

using MonoChain = dsp::ProcessorChain<Filter, Filter>;

using Coefficients = Filter::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacement);

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.coefficients, coefficients[0]);
//        chain.template setBypassed<Index>(false);
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 1) ;
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 1);
}

//==============================================================================
/**
*/
class DistortionProjAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DistortionProjAudioProcessor();
    ~DistortionProjAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts {
        *this,
        nullptr,
        "Parameters",
        createParameterLayout()
    };
    
    float getInRmsLevel(int channel){
        if(channel == 0){
            return rmsInLevelLeft;
        }
        return rmsInLevelRight;
    }
    
    float getOutRmsLevel(int channel){
        if(channel == 0){
            return rmsOutLevelLeft;
        }
        return rmsOutLevelRight;
    }
        
    using BlockType = juce::AudioBuffer<float>;
    
    SingleChannelSampleFifo<BlockType> leftChannelFifo {0};
    SingleChannelSampleFifo<BlockType> rightChannelFifo {1};
    
    PresetManager& getPresetManager() {return *presetManager;};

    ImageUploadManager& getImageUploadManager() {return imageUploadManager;};
    
private:
    //==============================================================================
    
    using Gain = dsp::Gain<float>;
    using Distortion = viator_dsp::Distortion<float>;
    
    MonoChain leftChain, rightChain;

    Gain outputGain, inputGain;
    
    AudioParameterChoice* distortionTypeParam {nullptr};

    float rmsInLevelLeft, rmsInLevelRight, rmsOutLevelLeft, rmsOutLevelRight;
    
    template<typename T, typename U>
    void applyGain(T& buffer, U& gain)
    {
        auto block = dsp::AudioBlock<float>(buffer);
        auto ctx = dsp::ProcessContextReplacing<float>(block);
        gain.process(ctx);
    }
        
    void updateHighCutFilters(const ChainSettings& chainSettings);
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    
    void updateFilters();
    
    
    juce::dsp::LinkwitzRileyFilter<float> LP1, AP1,
                                          HP1, LP2,
                                               HP2;
    
    juce::AudioParameterFloat* lowMidCrossoverFreq { nullptr };
    juce::AudioParameterFloat* midHighCrossoverFreq { nullptr };

    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    
    std::array<DistBand::DistortionBand, 3> distortionBands;
    DistBand::DistortionBand& lowDistBand = distortionBands[0];
    DistBand::DistortionBand& midDistBand = distortionBands[1];
    DistBand::DistortionBand& highDistBand = distortionBands[2];
    
    ImageUploadManager imageUploadManager;

    std::unique_ptr<PresetManager> presetManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionProjAudioProcessor)
};
