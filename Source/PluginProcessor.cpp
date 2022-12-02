/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "math.h"

enum ChainPositions
{
    lowCut,
    highCut
};


//==============================================================================
DistortionProjAudioProcessor::DistortionProjAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    auto boolHelper = [&apvts = this->apvts](auto& param, const auto paramNameString)
    {
        param = dynamic_cast<AudioParameterBool*>(apvts.getParameter(paramNameString));
        jassert(param != nullptr);
    };
    
    auto floatHelper = [&apvts = this->apvts](auto& param, const auto paramNameString)
    {
        param = dynamic_cast<AudioParameterFloat*>(apvts.getParameter(paramNameString));
        jassert(param != nullptr);
    };
    
    auto choiceHelper = [&apvts = this->apvts](auto& param, const auto paramNameString)
    {
        param = dynamic_cast<AudioParameterChoice*>(apvts.getParameter(paramNameString));
        jassert(param != nullptr);
    };
    
    apvts.state.setProperty(PresetManager::presetNameProperty, "", nullptr);
    apvts.state.setProperty("version", ProjectInfo::versionString, nullptr);
    apvts.state.setProperty(PresetManager::imageFilePathProperty, imageUploadManager.getCurrentImagePath(), nullptr);
    
    presetManager = std::make_unique<PresetManager>(apvts, imageUploadManager);
    
    
    boolHelper(lowDistBand.bypass, "lowBand Bypass");
    boolHelper(midDistBand.bypass, "midBand Bypass");
    boolHelper(highDistBand.bypass, "highBand Bypass");
    
    boolHelper(lowDistBand.mute, "lowBand Mute");
    boolHelper(midDistBand.mute, "midBand Mute");
    boolHelper(highDistBand.mute, "highBand Mute");
    
    boolHelper(lowDistBand.solo, "lowBand Solo");
    boolHelper(midDistBand.solo, "midBand Solo");
    boolHelper(highDistBand.solo, "highBand Solo");
    
    floatHelper(lowDistBand.drive, "lowdrive");
    floatHelper(midDistBand.drive, "middrive");
    floatHelper(highDistBand.drive, "highdrive");
    
    floatHelper(lowDistBand.mix, "mix");
    floatHelper(midDistBand.mix, "mix");
    floatHelper(highDistBand.mix, "mix");
    
    choiceHelper(lowDistBand.distortionType, "lowDistortion mode");
    choiceHelper(midDistBand.distortionType, "midDistortion mode");
    choiceHelper(highDistBand.distortionType, "highDistortion mode");
    
    imageUploadManager.imageManagerTreeP = &apvts;

    

//    initial multibnand stuff from yt vid
    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    AP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    
}

DistortionProjAudioProcessor::~DistortionProjAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionProjAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionProjAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionProjAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionProjAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionProjAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionProjAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionProjAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionProjAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionProjAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionProjAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionProjAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;
    
    outputGain.prepare(spec);
    outputGain.setRampDurationSeconds(0.05);
    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateFilters();
    
    for(auto& dist : distortionBands)
    {
        dist.prepare(spec);
    }
    
    //intial multiband stuff
    LP1.prepare(spec);
    HP1.prepare(spec);
    LP2.prepare(spec);
    HP2.prepare(spec);
    AP1.prepare(spec);

    for(auto& buffer : filterBuffers)
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }

}

void DistortionProjAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionProjAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DistortionProjAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto settings = getChainSettings(apvts);
    
    if(settings.powerSwitch==true){
    
        auto block = dsp::AudioBlock<float>(buffer);
        auto context = dsp::ProcessContextReplacing<float>(block);
        
        inputGain.setGainDecibels(settings.inputgain);
        outputGain.setGainDecibels(settings.outputgain);
        
        applyGain(buffer, inputGain);
        
        rmsInLevelLeft = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        rmsInLevelRight = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        
        ////////////////////////////// multiband testing
        
        for(auto& dist : distortionBands)
        {
            dist.updateDistortionSettings();
        }
        
        for( auto& fb : filterBuffers)
        {
            fb = buffer;
        }

        auto lowMidCutoff = settings.lowBandFreq;
        LP1.setCutoffFrequency(lowMidCutoff);
        HP1.setCutoffFrequency(lowMidCutoff);

        auto midHighCutoff = settings.highBandFreq;
        AP1.setCutoffFrequency(midHighCutoff);
        LP2.setCutoffFrequency(midHighCutoff);
        HP2.setCutoffFrequency(midHighCutoff);

        auto fb0block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
        auto fb1block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
        auto fb2block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

        auto fb0ctxt = juce::dsp::ProcessContextReplacing<float>(fb0block);
        auto fb1ctxt = juce::dsp::ProcessContextReplacing<float>(fb1block);
        auto fb2ctxt = juce::dsp::ProcessContextReplacing<float>(fb2block);

        LP1.process(fb0ctxt);
        AP1.process(fb0ctxt);

        HP1.process(fb1ctxt);
        filterBuffers[2] = filterBuffers[1];
        LP2.process(fb1ctxt);

        HP2.process(fb2ctxt);
        
        for(size_t i = 0; i < filterBuffers.size(); ++i)
        {
            distortionBands[i].process(filterBuffers[i]);
        }


        auto numSamples = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        buffer.clear();

        auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
        {
            for (auto i = 0; i < nc; ++i)
            {
                inputBuffer.addFrom(i, 0, source, i, 0, ns);
            }
        };
        
        auto bandsSoloed = false;
        for(auto& dist : distortionBands)
        {
            if(dist.solo->get())
            {
                bandsSoloed = true;
            }
        }
        
        if(bandsSoloed)
        {
            for(size_t i = 0; i < distortionBands.size(); ++i)
            {
                auto& dist = distortionBands[i];
                if(dist.solo->get())
                {
                    addFilterBand(buffer, filterBuffers[i]);
                }
                
            }
            
        }
        
        else
        {
            for(size_t i = 0; i < distortionBands.size(); ++i)
            {
                auto& dist = distortionBands[i];
                if(!dist.mute->get())
                {
                    addFilterBand(buffer, filterBuffers[i]);
                }
                
            }
            
        }
        
        updateFilters();

        auto leftBlock = block.getSingleChannelBlock(0);
        auto rightBlock = block.getSingleChannelBlock(1);
        
        dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        
        leftChain.process(leftContext);
        rightChain.process(rightContext);
        
        applyGain(buffer, outputGain);
        
        rmsOutLevelLeft = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
        rmsOutLevelRight = Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));
        
        leftChannelFifo.update(buffer);
        rightChannelFifo.update(buffer);
        
    
    }

    
}


//==============================================================================
bool DistortionProjAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionProjAudioProcessor::createEditor()
{
    return new DistortionProjAudioProcessorEditor (*this);
//    return new GenericAudioProcessorEditor(*this);
    
}

//==============================================================================
void DistortionProjAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    const auto state = apvts.copyState();
    const auto xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    
}

void DistortionProjAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    const auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if(xmlState == nullptr)
    {
        return;
    }
    const auto newTree = ValueTree::fromXml(*xmlState);
    apvts.replaceState(newTree);

}

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.highCutFreq = apvts.getRawParameterValue("highCut Freq")->load();
    settings.lowCutFreq = apvts.getRawParameterValue("lowCut Freq")->load();
    settings.lowDrive = apvts.getRawParameterValue("lowdrive")->load();
    settings.midDrive = apvts.getRawParameterValue("middrive")->load();
    settings.highDrive = apvts.getRawParameterValue("highdrive")->load();
    settings.inputgain = apvts.getRawParameterValue("inputgain")->load();
    settings.outputgain = apvts.getRawParameterValue("outputgain")->load();
    settings.mix = apvts.getRawParameterValue("mix")->load();
    settings.lowBandFreq = apvts.getRawParameterValue("crossoverLowMid")->load();
    settings.highBandFreq = apvts.getRawParameterValue("crossoverMidHigh")->load();
    settings.powerSwitch = apvts.getRawParameterValue("power switch")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("highCut Bypass")->load() > 0.5f;
    settings.lowCutBypassed = apvts.getRawParameterValue("lowCut Bypass")->load() > 0.5f;
    settings.lowBandBypassed = apvts.getRawParameterValue("lowBand Bypass")->load() > 0.5f;
    settings.midBandBypassed = apvts.getRawParameterValue("midBand Bypass")->load() > 0.5f;
    settings.highBandBypassed = apvts.getRawParameterValue("highBand Bypass")->load() > 0.5f;
    settings.lowBandMute = apvts.getRawParameterValue("lowBand Mute")->load() > 0.5f;
    settings.midBandMute = apvts.getRawParameterValue("midBand Mute")->load() > 0.5f;
    settings.highBandMute = apvts.getRawParameterValue("highBand Mute")->load() > 0.5f;
    settings.lowBandSolo = apvts.getRawParameterValue("lowBand Solo")->load() > 0.5f;
    settings.midBandSolo = apvts.getRawParameterValue("midBand Solo")->load() > 0.5f;
    settings.highBandSolo = apvts.getRawParameterValue("highBand Solo")->load() > 0.5f;
    settings.lowDistortionMode = apvts.getRawParameterValue("lowDistortion mode")->load();
    settings.midDistortionMode = apvts.getRawParameterValue("midDistortion mode")->load();
    settings.highDistortionMode = apvts.getRawParameterValue("highDistortion mode")->load();
    
    return settings;
}


void updateCoefficients(Coefficients &old, const Coefficients &replacement)
{
    *old = *replacement;
}

void DistortionProjAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<ChainPositions::lowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::lowCut>();
    
    leftChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::lowCut>(chainSettings.lowCutBypassed);

    
    updateCutFilter(leftLowCut, lowCutCoefficients);
    updateCutFilter(rightLowCut, lowCutCoefficients);
}

void DistortionProjAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<ChainPositions::highCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::highCut>();
    
    leftChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::highCut>(chainSettings.highCutBypassed);

    updateCutFilter(leftHighCut, highCutCoefficients);
    updateCutFilter(rightHighCut, highCutCoefficients);
}

void DistortionProjAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    
    updateLowCutFilters(chainSettings);
    updateHighCutFilters(chainSettings);
}



AudioProcessorValueTreeState::ParameterLayout DistortionProjAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto freqRange = NormalisableRange<float>(1.f, 22000.f, 1.f, 0.25f);

    auto gainRange = NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);
    
    auto driveRange = NormalisableRange<float>(0.f, 20.f, 0.5f, 1.f);
    
    layout.add(std::make_unique<AudioParameterFloat>("lowCut Freq",
                                                     "lowCut Freq",
                                                     freqRange,
                                                     1.f));
    layout.add(std::make_unique<AudioParameterFloat>("highCut Freq",
                                                     "highCut Freq",
                                                     freqRange,
                                                     22000.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("outputgain",
                                                     "outputGain",
                                                     gainRange,
                                                     0.f));
    layout.add(std::make_unique<AudioParameterFloat>("inputgain",
                                                     "inputGain",
                                                     gainRange,
                                                     0.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("mix",
                                                     "Mix",
                                                     NormalisableRange<float>(0.f, 100.f, 1.f, 1.f),
                                                     100.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("lowdrive",
                                                     "lowDrive",
                                                     driveRange,
                                                     0.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("middrive",
                                                     "midDrive",
                                                     driveRange,
                                                     0.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("highdrive",
                                                     "highDrive",
                                                     driveRange,
                                                     0.f));
    
    StringArray distortionType;
    distortionType.add("None");
    distortionType.add("Soft Clip");
    distortionType.add("Hard Clip");
    distortionType.add("Saturation");
    distortionType.add("Fuzz Distortion");
    distortionType.add("Tube Distortion");
    distortionType.add("Lofi Distortion");
    
    layout.add(std::make_unique<AudioParameterChoice>("lowDistortion mode",
                                                      "Low Distortion Mode",
                                                      distortionType,
                                                      0
                                                      ));
    
    layout.add(std::make_unique<AudioParameterChoice>("midDistortion mode",
                                                      "Mid Distortion Mode",
                                                      distortionType,
                                                      0
                                                      ));
    
    layout.add(std::make_unique<AudioParameterChoice>("highDistortion mode",
                                                      "High Distortion Mode",
                                                      distortionType,
                                                      0
                                                      ));
        
    layout.add(std::make_unique<AudioParameterBool>("highCut Bypass",
                                                    "highCut Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("lowCut Bypass",
                                                    "lowCut Bypass",
                                                    false
                                                    ));
    
    layout.add(std::make_unique<AudioParameterBool>("lowBand Bypass",
                                                    "lowBand Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("midBand Bypass",
                                                    "midBand Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("highBand Bypass",
                                                    "highBand Bypass",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("lowBand Solo",
                                                    "lowBand Solo",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("midBand Solo",
                                                    "midBand Solo",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("highBand Solo",
                                                    "highBand Solo",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("lowBand Mute",
                                                    "lowBand Mute",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("midBand Mute",
                                                    "midBand Mute",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("highBand Mute",
                                                    "highBand Mute",
                                                    false
                                                    ));
    layout.add(std::make_unique<AudioParameterBool>("power switch",
                                                    "Power switch",
                                                    true
                                                    ));
    
    layout.add(std::make_unique<AudioParameterFloat>("crossoverLowMid",
                                                     "crossoverLowMid",
                                                     NormalisableRange<float>(1.f, 999.f, 1, 1),
                                                     200.f));
    
    layout.add(std::make_unique<AudioParameterFloat>("crossoverMidHigh",
                                                     "crossoverMidHigh",
                                                     NormalisableRange<float>(1000.f, 22000.f, 1, 1),
                                                     2000.f));
            
    return layout;
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionProjAudioProcessor();
}
