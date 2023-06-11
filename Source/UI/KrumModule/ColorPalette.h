/*
  ==============================================================================

    ColorPalette.h
    Created: 25 Mar 2021 12:50:32pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
* 
* A class the represents the color buttons that show up with a ModuleSettingsOverlay. This is where the color scheme is set.
* 
*/


class ModuleSettingsOverlay;

class ColorPalette  : public juce::Component
{
public:

    enum ColorRadio
    {
        colorRadioGroupId = 42
    };


    ColorPalette(/*juce::Rectangle<int> bounds, */ModuleSettingsOverlay& parentOverlay/*, bool isColorOnly*/);
    ~ColorPalette() override;

    void resized() override;

    void colorClicked(juce::Colour clickedColor, juce::ShapeButton* button);

    juce::Colour getSelectedColor();
    bool isColorSelected();

    juce::Colour* getLastRandomColor();


    //need to fix this function
    //static juce::Colour* getRandomColor(juce::Colour* lastRandom)
    //{
    //    juce::Random random;
    //    int randIt = random.nextInt(juce::Range<int>(0, ColorPaletteColors::colorArray.size() - 1));
    //    auto randColor = ColorPaletteColors::colorArray[randIt];

    //    if (randColor != *lastRandom)
    //    {
    //        *lastRandom = randColor;
    //    }
    //    else //avoids duplicates in succession
    //    {
    //        *lastRandom = randIt == ColorPaletteColors::colorArray.size() - 1 ?     ColorPaletteColors::colorArray[0] :                                                                                                        ColorPaletteColors::colorArray[++randIt];
    //    }

    //    *lastRandom = randColor;
    //    
    //    return lastRandom;
    //}

    //static juce::Colour getRandomColor()
    //{
    //    juce::Random random;
    //    int randIt = random.nextInt(juce::Range<int>(0, ColorPaletteColors::colorArray.size() - 1));
    //    return ColorPaletteColors::colorArray[randIt];
    //}

private:
    ModuleSettingsOverlay& parent;

    juce::Colour selectedColor{ juce::Colours::white };
    juce::Colour* lastRandomColor = nullptr;
    juce::OwnedArray<juce::ShapeButton> colorButtons;

    JUCE_LEAK_DETECTOR(ColorPalette)
};
