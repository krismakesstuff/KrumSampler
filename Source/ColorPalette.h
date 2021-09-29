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
* 
* TODO: 
* - Fix getRandomColor() and make sure the color is not the same as the color before it. 
* - Redesign with ModuleSettingsOverlay, bigger color buttons? 
*/

namespace ColorPaletteColors
{
    static juce::Colour redSalsa =      juce::Colour::fromRGB(249, 65, 68);
    static juce::Colour orangeRed =     juce::Colour::fromRGB(243, 114, 44);
    static juce::Colour yellowOrange =  juce::Colour::fromRGB(248, 150, 30);
    static juce::Colour mangoTango =    juce::Colour::fromRGB(249, 132, 74);
    static juce::Colour maize =         juce::Colour::fromRGB(249, 199, 79);
    static juce::Colour pistachio =     juce::Colour::fromRGB(144, 190, 109);
    static juce::Colour zomp =          juce::Colour::fromRGB(67, 170, 139);
    static juce::Colour cadetBlue =     juce::Colour::fromRGB(77, 144, 142);
    static juce::Colour queenBlue =     juce::Colour::fromRGB(87, 117, 144);
    static juce::Colour cgBlue =        juce::Colour::fromRGB(39, 125, 161);

    static juce::Array<juce::Colour> colorArray{ redSalsa, orangeRed, yellowOrange, mangoTango, maize,
                                                pistachio, zomp, cadetBlue, queenBlue, cgBlue };

}


class ModuleSettingsOverlay;

class ColorPalette  : public juce::Component
{
public:

    enum ColorRadio
    {
        colorRadioGroupId = 42
    };


    ColorPalette(juce::Rectangle<int> bounds, ModuleSettingsOverlay& parentOverlay, bool isColorOnly);
    ~ColorPalette() override;

    void colorClicked(juce::Colour clickedColor, juce::ShapeButton* button);

    juce::Colour getSelectedColor();
    bool isColorSelected();


    juce::Colour* getLastRandomColor();


    //need to fix this function
    static juce::Colour* getRandomColor(juce::Colour* lastRandom)
    {
        juce::Random random;
        int randIt = random.nextInt(juce::Range<int>(0, ColorPaletteColors::colorArray.size() - 1));
        auto randColor = ColorPaletteColors::colorArray[randIt];

//        if (randColor != *lastRandom)
//        {
//            *lastRandom = randColor;
//        }
//        else //avoids duplicates in succession
//        {
//            *lastRandom = randIt == ColorPaletteColors::colorArray.size() - 1 ?     ColorPaletteColors::colorArray[0] :                                                                                                        ColorPaletteColors::colorArray[++randIt];
//        }

        *lastRandom = randColor;
        
        return lastRandom;
    }

    static juce::Colour getRandomColor()
    {
        juce::Random random;
        int randIt = random.nextInt(juce::Range<int>(0, ColorPaletteColors::colorArray.size() - 1));
        return ColorPaletteColors::colorArray[randIt];
    }

private:
    ModuleSettingsOverlay& parent;

    juce::Colour selectedColor{ juce::Colours::white };
    juce::Colour* lastRandomColor = nullptr;
    juce::OwnedArray<juce::ShapeButton> colorButtons;

    JUCE_LEAK_DETECTOR(ColorPalette)
};
