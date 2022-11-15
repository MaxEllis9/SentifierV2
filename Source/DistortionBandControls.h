/*
  ==============================================================================

    DistortionBandControls.h
    Created: 23 Sep 2022 1:47:46pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KnobComponent.h"

namespace Gui
{
    class DistBandControlPanel : public juce::Component
    {
    public:
        DistBandControlPanel(const String nameString,
                             const String driveParam,
                             const String soloParam,
                             const String muteParam,
                             const String bypassParam,
                             const String distortionSelector,
                             DistortionProjAudioProcessor& p) : audioProcessor(p),
        
        driveKnob(2, "dB", 0.0, 20.0),
        solo(0, true, "S", false),
        mute(1, true, "M", false),
        bypass(2, true, "B", false),
    
        driveKnobAttachment(audioProcessor.apvts, driveParam, driveKnob),
        distortionTypeAttachment(audioProcessor.apvts, distortionSelector, distortionType),
        soloAttachment(audioProcessor.apvts, soloParam, solo),
        muteAttachment(audioProcessor.apvts, muteParam, mute),
        bypassAttachment(audioProcessor.apvts, bypassParam, bypass)
        
        {
            configureLabel(nameString, bandLabel);
            configureLabel("Drive", driveLabel);
            
            distortionType.addItem("Distortion Type", 1);
            distortionType.addItem("Soft Clip", 2);
            distortionType.addItem("Hard Clip", 3);
            distortionType.addItem("Saturation", 4);
            distortionType.addItem("LoFi Distortion", 5);
            distortionType.addItem("Tube Distortion", 6);
            distortionType.addItem("Fuzz Distortion", 7);
            distortionType.setSelectedId(1);
            distortionType.setTextWhenNothingSelected("Distortion type");
            distortionType.setMouseCursor(MouseCursor::PointingHandCursor);
            distortionType.setColour(ComboBox::ColourIds::backgroundColourId, Colour(102, 25, 30));
            distortionType.setColour(ComboBox::ColourIds::outlineColourId, Colour(61, 15, 18));
            distortionType.setColour(ComboBox::ColourIds::textColourId, Colours::gainsboro);
            distortionType.setJustificationType(juce::Justification::centred);
            
            comboBoxLnF.setColour(PopupMenu::ColourIds::backgroundColourId, Colours::black);
            comboBoxLnF.setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, Colour(102, 25, 30));
            comboBoxLnF.setColour(PopupMenu::ColourIds::textColourId, Colours::gainsboro);
            
            distortionType.setLookAndFeel(&comboBoxLnF);
            
            addAndMakeVisible(distortionType);
            addAndMakeVisible(bandLabel);
            addAndMakeVisible(driveLabel);
            addAndMakeVisible(driveKnob);
            addAndMakeVisible(solo);
            addAndMakeVisible(bypass);
            addAndMakeVisible(mute);

        }
        
        ~DistBandControlPanel()
        {
            distortionType.setLookAndFeel(nullptr);
        }
        
        void resized() override
        {
            
            const auto container = getLocalBounds();
            auto bounds = container;
            
            bandLabel.setBounds(bounds.removeFromTop(container.proportionOfHeight(0.1)));
            distortionType.setBounds(bounds.removeFromTop(container.proportionOfHeight(0.2)).reduced(10.f, 5.f));
            
            auto smbButtons = bounds.removeFromLeft(container.proportionOfWidth(0.4));
            
            solo.setBounds(smbButtons.removeFromTop(smbButtons.getHeight() * 0.33).reduced(0, 5.f));
            mute.setBounds(smbButtons.removeFromTop(smbButtons.getHeight() * 0.5).reduced(0, 5.f));
            bypass.setBounds(smbButtons.reduced(0, 5.f));
            
            driveLabel.setBounds(bounds.removeFromTop(bounds.proportionOfHeight(0.15)));
            driveKnob.setBounds(bounds);

            
        }
        
        void configureLabel(const String& labelString, Label& labelComp)
        {
            labelComp.setText(labelString, dontSendNotification);
            labelComp.setJustificationType(juce::Justification::centred);
            labelComp.setColour(Label::ColourIds::textColourId, Colours::gainsboro);
        }
        

        
    private:
        
        using APVTS = juce::AudioProcessorValueTreeState;
        
        DistortionProjAudioProcessor& audioProcessor;
        
        Label bandLabel, driveLabel;
        
        viator_gui::FilmStripKnob driveKnob;
                
        viator_gui::ToggleButton solo, mute, bypass;
        
        APVTS::SliderAttachment driveKnobAttachment;
        
        ComboBox distortionType;
        
        APVTS::ComboBoxAttachment distortionTypeAttachment;
                
        APVTS::ButtonAttachment soloAttachment,
        muteAttachment,
        bypassAttachment;
        
        LookAndFeel_V4 comboBoxLnF;
        
    };
}
