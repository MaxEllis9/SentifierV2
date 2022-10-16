/*
  ==============================================================================

    DistortionBandPanel.h
    Created: 25 Aug 2022 4:18:50pm
    Author:  Max Ellis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace DistBand
{

    struct DistortionBand
    {
        juce::AudioParameterBool* bypass { nullptr };
        juce::AudioParameterBool* solo { nullptr };
        juce::AudioParameterBool* mute { nullptr };
        juce::AudioParameterFloat* drive { nullptr };
        juce::AudioParameterFloat* mix { nullptr };
        juce::AudioParameterChoice* distortionType { nullptr };

        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            softClipper.setClipperType(viator_dsp::Distortion<float>::ClipType::kSoft);
            softClipper.prepare(spec);
            
            hardClipper.setClipperType(viator_dsp::Distortion<float>::ClipType::kHard);
            hardClipper.prepare(spec);
            
            fuzzDistortion.setClipperType(viator_dsp::Distortion<float>::ClipType::kFuzz);
            fuzzDistortion.prepare(spec);
            
            saturation.setClipperType(viator_dsp::Distortion<float>::ClipType::kSaturation);
            saturation.prepare(spec);
            
            tubeDistortion.setClipperType(viator_dsp::Distortion<float>::ClipType::kTube);
            tubeDistortion.prepare(spec);
            
            lofiDistortion.setClipperType(viator_dsp::Distortion<float>::ClipType::kLofi);
            lofiDistortion.prepare(spec);
            
        }

        void updateDistortionSettings()
        {
            softClipper.setDrive(drive->get());
            hardClipper.setDrive(drive->get());
            fuzzDistortion.setDrive(drive->get());
            saturation.setDrive(drive->get());
            tubeDistortion.setDrive(drive->get());
            lofiDistortion.setDrive(drive->get());
            
            softClipper.setMix(mix->get()/100);
            hardClipper.setMix(mix->get()/100);
            fuzzDistortion.setMix(mix->get()/100);
            saturation.setMix(mix->get()/100);
            tubeDistortion.setMix(mix->get()/100);
            lofiDistortion.setMix(mix->get()/100);
            
            softClipper.setEnabled(!bypass->get());
            hardClipper.setEnabled(!bypass->get());
            fuzzDistortion.setEnabled(!bypass->get());
            saturation.setEnabled(!bypass->get());
            tubeDistortion.setEnabled(!bypass->get());
            lofiDistortion.setEnabled(!bypass->get());
        }

        void process(juce::AudioBuffer<float>& buffer)
        {
            auto block = juce::dsp::AudioBlock<float>(buffer);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            
            //might need to be notted
            context.isBypassed = bypass->get();
            
            switch (distortionType->getIndex()) {
                case 1:
                    softClipper.process(context);
                    break;
                case 2:
                    hardClipper.process(context);
                    break;
                case 3:
                    saturation.process(context);
                    break;
                case 4:
                    lofiDistortion.process(context);
                    break;
                case 5:
                    tubeDistortion.process(context);
                    break;
                case 6:
                    fuzzDistortion.process(context);
                    break;
                    
                default:
                    break;
            }
        }
        
        private:
            viator_dsp::Distortion<float> softClipper,
                                        hardClipper,
                                        fuzzDistortion,
                                        saturation,
                                        tubeDistortion,
                                        lofiDistortion;
            
    };
}
