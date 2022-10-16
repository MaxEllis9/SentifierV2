/*
  ==============================================================================

    KnobWithLabel.h
    Created: 23 Sep 2022 3:56:13pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui {
    class KnobWithLabel : public juce::Component
    {
    public:
        
        KnobWithLabel(const String knobLabelString,
                      int min,
                      int max,
                      const String suffix,
                      const String compName,
                      DistortionProjAudioProcessor& p) : audioProcessor(p),
        
        knob(2, suffix, min, max),
        knobAttachment(audioProcessor.apvts, compName, knob)
        {
            configureLabel(knobLabelString, label);
            addAndMakeVisible(knob);
            addAndMakeVisible(label);
        }
        
        ~KnobWithLabel()
        {
            
        }
        
        void resized() override
        {
            const auto container = getLocalBounds();
            auto bounds = container;
            
            label.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.17).reduced(0.f, 2.f));
            knob.setBounds(bounds.reduced(10.f, 0.f));
            
        }
        
        void configureLabel(const String& labelString, Label& labelComp)
        {
            labelComp.setText(labelString, dontSendNotification);
            labelComp.setJustificationType(juce::Justification::centred);
            labelComp.setColour(Label::ColourIds::textColourId, Colours::gainsboro);
        }
        
        
        viator_gui::FilmStripKnob& getKnobComponent()
        {
            return knob;
        }

    private:
        DistortionProjAudioProcessor& audioProcessor;
        
        viator_gui::FilmStripKnob knob;
        
        Label label;
        
        juce::AudioProcessorValueTreeState::SliderAttachment knobAttachment;
    
    };
}
