/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GainMeter.h"
#include "ImageAnalyser.h"
#include "PresetPanel.h"
#include "ResponseCurve.h"
#include "ImageUploadPanel.h"
#include "DistortionBandControls.h"
#include "KnobComponent.h"

struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& toggleButton,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
};

struct PowerButton : juce::ToggleButton {};


//==============================================================================

class DistortionProjAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionProjAudioProcessorEditor (DistortionProjAudioProcessor&);
    ~DistortionProjAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void resetImage();
    
    void configureLabel(const String& labelString, Label& labelComp);
    
    void configureOnOffSwitch(ImageButton& onOffSwitch);

    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DistortionProjAudioProcessor& audioProcessor;
        
    ResponseCurve responseCurveComponent;
    
    Label nameLabel;
    
    Gui::GainMeter GainMeterInL,
    GainMeterInR,
    GainMeterOutL,
    GainMeterOutR;
    
    Gui::Bulb onOffBulb;
    
    Gui::PresetPanel presetPanel;
    
    Gui::ImageUploadPanel imageUploadPanel;
    
    viator_gui::FilmStripKnob lowCutKnob,
    highCutKnob;
    
    PowerButton
    highCutBypass,
    lowCutBypass;
    
    ImageButton onOffSwitch;
        
    ImageComponent lowPassSymbolImg, highPassSymbolImg;
        
    TextEditor imageAnalysisOutput;
    
    std::vector<juce::Component*> getComponents();
    std::vector<juce::Button*> getButtons();

    using APVTS = juce::AudioProcessorValueTreeState;
    
    APVTS::SliderAttachment
    highCutKnobAttachment,
    lowCutKnobAttachment;
    
    APVTS::ButtonAttachment onOffButtonAttachment,
    highCutBypassAttachment,
    lowCutBypassAttachment;
    

    Gui::DistBandControlPanel lowControls, midControls, highControls;
    
    CustomLookAndFeel lnf;

    ImageAnalyser imageAnalyser;
    
    Gui::KnobWithLabel mixKnob,
    lowMidCrossKnob,
    midHighCrossKnob,
    inputGainKnob,
    outputGainKnob;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionProjAudioProcessorEditor)
};
