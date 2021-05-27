/*
  ==============================================================================

    KrumSlider.cpp
    Created: 10 Mar 2021 1:41:25pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KrumSlider.h"

//==============================================================================
KrumSlider::KrumSlider(juce::Slider::SliderStyle style)
{
    //setLookAndFeel(&getLookAndFeel().getDefaultLookAndFeel());

    setScrollWheelEnabled(false);
    setSliderStyle(style);
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    setNumDecimalPlacesToDisplay(2);
    setPopupDisplayEnabled(true, false, this);
    setTooltip(getTextFromValue(getValue()));


}

KrumSlider::~KrumSlider()
{

}

void KrumSlider::paint (juce::Graphics& g)
{
    juce::Slider::paint(g);
}

//void KrumSlider::resized()
//{
//   
//}
//
//void KrumSlider::updateBubbleComp()
//{
//    auto bubbleComp = static_cast<juce::BubbleComponent*>(getCurrentPopupDisplay());
//    if (bubbleComp != nullptr)
//    {
//        juce::Point<int> pos{ getLocalBounds().getCentreX() + 6, getMouseXYRelative().getY() + 3 };
//
//        bubbleComp->setAllowedPlacement(juce::BubbleComponent::right);
//        bubbleComp->setPosition(pos, 0);
//
//    }
//    setTooltip(getTextFromValue(getValue()));
//
//}
