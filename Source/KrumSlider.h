/*
  ==============================================================================

    KrumSlider.h
    Created: 10 Mar 2021 1:41:25pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class KrumSlider  : public juce::Slider
{
public:
    KrumSlider(juce::Slider::SliderStyle style);
    ~KrumSlider() override;

    void paint (juce::Graphics&) override;
    //void resized() override;

    //void updateBubbleComp();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSlider)
};
