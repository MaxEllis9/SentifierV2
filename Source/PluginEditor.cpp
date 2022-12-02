/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void CustomLookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    if(auto* pb = dynamic_cast<PowerButton*>(&toggleButton)){
        Path powerButton;

        auto bounds = toggleButton.getLocalBounds().removeFromBottom(toggleButton.getLocalBounds().getHeight() * 0.9);
        auto size = jmin(bounds.getWidth(), bounds.getHeight() - 1);
        auto r = Rectangle<float>(bounds.getX() + 15, bounds.getY(), size, size);

        float ang = 35.f;

        size -= 7;

        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);

        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());

        PathStrokeType pst(2, PathStrokeType::JointStyle::curved);

        auto colour = toggleButton.getToggleState() ? Colours::dimgrey : Colours::skyblue;

        g.setColour(colour);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2.f);
    }
//    else if(auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
//    {
//        auto colour = !toggleButton.getToggleState() ? Colours::dimgrey : Colours::green;
//
//        g.setColour(colour);
//
//        auto bounds = toggleButton.getLocalBounds();
//        g.drawRect(bounds);
//
//        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
//    }
    
}

//===================================================================
DistortionProjAudioProcessorEditor::DistortionProjAudioProcessorEditor (DistortionProjAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

responseCurveComponent(audioProcessor),

GainMeterInL([&]() {return audioProcessor.getInRmsLevel(0); } ),
GainMeterInR([&]() {return audioProcessor.getInRmsLevel(1); } ),
GainMeterOutL([&]() {return audioProcessor.getOutRmsLevel(0); } ),
GainMeterOutR([&]() {return audioProcessor.getOutRmsLevel(1); } ),

onOffBulb(Colours::lawngreen),

presetPanel(p.getPresetManager(), imageUploadPanel),

imageUploadPanel(p.getImageUploadManager(), imageAnalysisOutput),

lowCutKnob(2, "Hz", 0, 22000),
highCutKnob(2, "Hz", 0, 22000),

highCutKnobAttachment(audioProcessor.apvts, "highCut Freq", highCutKnob),
lowCutKnobAttachment(audioProcessor.apvts, "lowCut Freq", lowCutKnob),

onOffButtonAttachment(audioProcessor.apvts, "power switch", onOffSwitch),
highCutBypassAttachment(audioProcessor.apvts, "highCut Bypass", highCutBypass),
lowCutBypassAttachment(audioProcessor.apvts, "lowCut Bypass", lowCutBypass),

lowControls("Low Band Control", "lowdrive", "lowBand Solo", "lowBand Mute", "lowBand Bypass", "lowDistortion mode", audioProcessor),
midControls("Mid Band Controls", "middrive", "midBand Solo", "midBand Mute", "midBand Bypass", "midDistortion mode", audioProcessor),
highControls("High Band Controls", "highdrive", "highBand Solo", "highBand Mute", "highBand Bypass", "highDistortion mode", audioProcessor),

mixKnob("Mix", 0, 100, "", "mix", audioProcessor),
lowMidCrossKnob("Low Mid Crossover", 0, 999, "Hz", "crossoverLowMid", audioProcessor),
midHighCrossKnob("Mid High Crossover", 1000, 22000, "Hz", "crossoverMidHigh", audioProcessor),
inputGainKnob("Input Gain", -24, 24, "dB", "inputgain", audioProcessor),
outputGainKnob("Output Gain", -24, 24, "dB", "outputgain", audioProcessor)

{
    highPassSymbolImg.setImage(ImageCache::getFromMemory(BinaryData::lowCutNewWhite_png, BinaryData::lowCutNewWhite_pngSize));

    lowPassSymbolImg.setImage(ImageCache::getFromMemory(BinaryData::highCutNewWhite_png, BinaryData::highCutNewWhite_pngSize));
    
    for(auto* comps : getComponents()){
        addAndMakeVisible(comps);
    }
    
    for(auto* buttons : getButtons()){
        addAndMakeVisible(buttons);
    }
    
    lowCutBypass.setLookAndFeel(&lnf);
    highCutBypass.setLookAndFeel(&lnf);
    
    configureOnOffSwitch(onOffSwitch);
    
    logo.setImage(ImageCache::getFromMemory(BinaryData::logo1_png, BinaryData::logo1_pngSize));
    
//    configureLabel("Insert Name Here", nameLabel);
    
    imageAnalysisOutput.setMultiLine(true, true);
    imageAnalysisOutput.setReadOnly(true);
    imageAnalysisOutput.setColour(TextEditor::ColourIds::backgroundColourId, Colours::black);
    imageAnalysisOutput.setColour(TextEditor::ColourIds::outlineColourId, Colours::black);
    imageAnalysisOutput.setColour(TextEditor::ColourIds::textColourId, Colours::gainsboro);

    imageAnalysisOutput.setScrollbarsShown(true);
    imageAnalysisOutput.setText("Upload an JPEG or PNG image to generate a patch");


    auto safePtr = juce::Component::SafePointer<DistortionProjAudioProcessorEditor>(this);
    
    onOffBulb.setState(true);
    
    onOffSwitch.onClick = [safePtr]()
    {
        if(auto* comp = safePtr.getComponent())
        {
            auto enabled = comp->onOffSwitch.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
            comp->GainMeterInL.toggleMeterEnablement(enabled);
            comp->GainMeterInR.toggleMeterEnablement(enabled);
            comp->GainMeterOutL.toggleMeterEnablement(enabled);
            comp->GainMeterOutR.toggleMeterEnablement(enabled);
            comp->onOffBulb.setState(enabled);
        }
    };
    
    onOffSwitch.setComponentID("onOffSwitch");
        
    setSize (1100, 650);
}

DistortionProjAudioProcessorEditor::~DistortionProjAudioProcessorEditor()
{
    
    lowCutBypass.setLookAndFeel(nullptr);
    highCutBypass.setLookAndFeel(nullptr);

}

void DistortionProjAudioProcessorEditor::configureLabel(const String& labelString, Label& labelComp)
{
    labelComp.setText(labelString, dontSendNotification);
    labelComp.setJustificationType(juce::Justification::centred);
}

void DistortionProjAudioProcessorEditor::configureOnOffSwitch(ImageButton& onOffSwitch)
{
    auto switchOnImg = ImageCache::getFromMemory(BinaryData::switchOnBlue_png, BinaryData::switchOnBlue_pngSize);
    
    auto switchOffImg = ImageCache::getFromMemory(BinaryData::switchOffBlue_png, BinaryData::switchOffBlue_pngSize);
    
    onOffSwitch.setToggleable(true);
    onOffSwitch.setToggleState(true, dontSendNotification);
    onOffSwitch.setClickingTogglesState(true);
    onOffSwitch.setImages(false, true, true, switchOffImg, 1.f, {}, switchOffImg, 1.f, {}, switchOnImg, 1.f, {});
}

//==============================================================================
void DistortionProjAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colour(Colours::black));
//    red colour Colour(158, 39, 46)
    
    auto bounds = getLocalBounds();
    auto titleBar = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto menu = titleBar.removeFromRight(titleBar.getWidth() * 0.5).reduced(4.f);
    titleBar = titleBar.reduced(4.f);
    auto thumbnail = bounds.removeFromRight(bounds.getWidth() * 0.2);
    auto meters = thumbnail.removeFromBottom(thumbnail.getHeight() * 0.66);
    thumbnail = thumbnail.reduced(4.f);
    meters = meters.reduced(4.f);
    auto spectrumArea = bounds.removeFromTop(bounds.getHeight() * 0.45).reduced(4.f);
    auto description = bounds.removeFromRight(bounds.getWidth() * 0.25);
    auto powerButton = description.removeFromBottom(description.getHeight() * 0.33).reduced(4.f);
    description = description.reduced(4.f);
    auto generalKnobs = bounds.removeFromTop(bounds.getHeight() * 0.4).reduced(4.f);
    auto lowControl = bounds.removeFromLeft(bounds.getWidth() * 0.33).reduced(4.f);
    auto midControl = bounds.removeFromLeft(bounds.getWidth() * 0.5).reduced(4.f);
    auto highControl = bounds.reduced(4.f);
    auto spectrumGraph = spectrumArea.reduced(22.f, 15.f);
        
    
    g.setColour(Colour(158, 39, 46));
    g.fillRect(titleBar.toFloat());
    g.fillRect(menu.toFloat());
    g.fillRect(thumbnail.toFloat());
    g.fillRect(meters.toFloat());
    g.fillRect(spectrumArea.toFloat());
    g.fillRect(description.toFloat());
    g.fillRect(generalKnobs.toFloat());
    g.fillRect(lowControl.toFloat());
    g.fillRect(midControl.toFloat());
    g.fillRect(highControl.toFloat());
    g.fillRect(powerButton.toFloat());
    
    
//    draws response graph background
        g.setColour(Colours::black);

    g.fillRect(spectrumGraph.toFloat());

    Array<float> freqs
    {
        20, //30, 40,
        50, 100,
        200, //300, 400,
        500, 1000,
        2000, //3000, 4000,
        5000, 10000,
        20000
    };
    
    auto left = spectrumGraph.getX();
    auto right = spectrumGraph.getRight();
    auto top = spectrumGraph.getY();
    auto bottom = spectrumGraph.getBottom();
    auto width = spectrumGraph.getWidth();
    
    Array<float> xs;
    for(auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }
    
    g.setColour(Colours::dimgrey);
    for(auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }
    
    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };
    
    for(auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
        
    };
    
    g.setColour(Colours::black);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
    for(int i=0; i<freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];
        
        bool addK = false;
        juce::String str;
        if(f > 999.f){
            addK = true;
            f /= 1000.f;
        }
        str << f;
        if(addK){
            str << "k";
        }
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(spectrumArea.getY());
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    
    for(auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        juce::String str;
        if(gDb > 0){
            str << "+";
        }
        str <<gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(spectrumGraph.getRight());
        r.setCentre(r.getCentreX(), y);
        
        
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
        
        str.clear();
        str << (gDb - 24.f);
        r.setX(spectrumArea.getX());
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    //////////crossover vertical lines
    
    auto mapX = [](float frequency, float left, float width)
    {
        auto normX = juce::mapFromLog10(frequency, 20.f, 20000.f);
        return left + width * normX;
    };
    
    auto lowMidX = mapX(lowMidCrossKnob.getKnobComponent().getValue(), left, width);
    g.setColour(Colours::skyblue);
    g.fillRect(Rectangle<float>(lowMidX-2, top, 4.f, bottom-top));
    
    auto midHighX = mapX(midHighCrossKnob.getKnobComponent().getValue(), left, width);
    g.fillRect(Rectangle<float>(midHighX-2, top, 4.f, bottom-top));

    
}

void DistortionProjAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto titleBar = bounds.removeFromTop(bounds.getHeight() * 0.1);
    auto menu = titleBar.removeFromRight(titleBar.getWidth() * 0.50).reduced(4.f);
    titleBar = titleBar.reduced(4.f);
    auto thumbnail = bounds.removeFromRight(bounds.getWidth() * 0.2);
    auto metersArea = thumbnail.removeFromBottom(thumbnail.getHeight() * 0.66);
    metersArea = metersArea.reduced(5.f, 4.f);
    auto meters = metersArea.removeFromBottom(metersArea.getHeight() * 0.75);
    auto gainControls = metersArea;
    auto leftMeter = meters.removeFromLeft(meters.getWidth() * 0.5);
    auto leftMeterSpacer = leftMeter.removeFromLeft(leftMeter.getWidth() * 0.25);
    auto leftLeftMeter = leftMeter.removeFromLeft(leftMeter.proportionOfWidth(0.33));
    auto rightLeftMeter = leftMeter.removeFromLeft(leftMeter.proportionOfWidth(0.5));
    
    auto rightMeter = meters;
    auto rightMeterSpacer = rightMeter.removeFromLeft(rightMeter.getWidth() * 0.25);
    auto leftRightMeter = rightMeter.removeFromLeft(rightMeter.proportionOfWidth(0.33));
    auto rightRightMeter = rightMeter.removeFromLeft(rightMeter.proportionOfWidth(0.5));
    
    thumbnail = thumbnail.reduced(4.f);
    auto spectrumArea = bounds.removeFromTop(bounds.getHeight() * 0.45).reduced(4.f);
    auto description = bounds.removeFromRight(bounds.getWidth() * 0.25);
    auto powerButton = description.removeFromBottom(description.getHeight() * 0.33).reduced(4.f);
    description = description.reduced(4.f);
    auto generalKnobs = bounds.removeFromTop(bounds.getHeight() * 0.4).reduced(4.f);
    auto lowCut = generalKnobs.removeFromLeft(generalKnobs.getWidth() * 0.2);
    auto lowCutLabel = lowCut.removeFromTop(lowCut.getHeight() * 0.17).reduced(0.f, 2.f);
    auto lowCutBypassSect = lowCutLabel.removeFromLeft(lowCutLabel.getWidth() * 0.25);
    
    auto lowMidCross = generalKnobs.removeFromLeft(generalKnobs.getWidth() * 0.25);
    auto mixControl = generalKnobs.removeFromLeft(generalKnobs.getWidth() * 0.33);
    auto midHighCross = generalKnobs.removeFromLeft(generalKnobs.getWidth() * 0.5);
    auto highCut = generalKnobs;
    auto highCutLabel = highCut.removeFromTop(highCut.getHeight() * 0.17).reduced(0.f, 2.f);
    auto highCutBypassSect = highCutLabel.removeFromLeft(highCutLabel.getWidth() * 0.25);

    auto lowControl = bounds.removeFromLeft(bounds.getWidth() * 0.33).reduced(4.f);
    auto midControl = bounds.removeFromLeft(bounds.getWidth() * 0.5).reduced(4.f);
    auto highControl = bounds.reduced(4.f);
    auto spectrumGraph = spectrumArea.reduced(22.f, 15.f);
    
    
    presetPanel.setBounds(menu);
    logo.setBounds(titleBar.expanded(25.f));
    imageAnalysisOutput.setBounds(description.reduced(4.f));
    imageUploadPanel.setBounds(thumbnail);
    
    highPassSymbolImg.setBounds(lowCutLabel.getX(), lowCutLabel.getY(), lowCutLabel.getWidth()/1.5, lowCutLabel.getHeight());
    lowCutBypass.setBounds(lowCutBypassSect);
    lowCutKnob.setBounds(lowCut.reduced(10.f, 0));
    
    lowMidCrossKnob.setBounds(lowMidCross);
    
    mixKnob.setBounds(mixControl);
    
    midHighCrossKnob.setBounds(midHighCross);
    
    lowPassSymbolImg.setBounds(highCutLabel.getX(), highCutLabel.getY(), highCutLabel.getWidth()/1.5, highCutLabel.getHeight());
    highCutBypass.setBounds(highCutBypassSect);
    highCutKnob.setBounds(highCut.reduced(10.f, 0.f));
    
    inputGainKnob.setBounds(gainControls.removeFromLeft(gainControls.getWidth() * 0.5));
    outputGainKnob.setBounds(gainControls);
    
    GainMeterInL.setBounds(leftLeftMeter);
    GainMeterInR.setBounds(rightLeftMeter);
    GainMeterOutL.setBounds(leftRightMeter);
    GainMeterOutR.setBounds(rightRightMeter);

    responseCurveComponent.setBounds(spectrumGraph);
    
    lowControls.setBounds(lowControl);
    midControls.setBounds(midControl);
    highControls.setBounds(highControl);
    
    onOffSwitch.setBounds(powerButton.removeFromLeft(powerButton.getWidth() * 0.5).reduced(5.f));
    onOffBulb.setBounds(powerButton.reduced(30.f));

}



std::vector<juce::Component*> DistortionProjAudioProcessorEditor::getComponents()
{
    return
    {
        &highCutKnob,
        &lowCutKnob,
        &inputGainKnob,
        &outputGainKnob,
        &mixKnob,
        &responseCurveComponent,
        &GainMeterOutR,
        &GainMeterOutL,
        &GainMeterInL,
        &GainMeterInR,
        &logo,
        &imageAnalysisOutput,
        &lowPassSymbolImg,
        &highPassSymbolImg,
        &onOffBulb,
        &lowMidCrossKnob,
        &midHighCrossKnob,
        &presetPanel,
        &imageUploadPanel,
        &lowControls,
        &midControls,
        &highControls
        
    };
}

std::vector<juce::Button*> DistortionProjAudioProcessorEditor::getButtons()
{
    return
    {
        &highCutBypass,
        &lowCutBypass,
        &onOffSwitch
    };
}
