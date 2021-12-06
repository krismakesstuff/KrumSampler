/*
  ==============================================================================

    ColorPalette.cpp
    Created: 25 Mar 2021 12:50:32pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ColorPalette.h"
#include "ModuleSettingsOverlay.h"

//FIX MEE
// Need to set the bounds in a resized method instead of in the ctor.... 
//==============================================================================
ColorPalette::ColorPalette(/*juce::Rectangle<int> bounds, */ModuleSettingsOverlay& pm/*, bool isColorOnly*/)
    : parent(pm)/*, colorOnly(isColorOnly)*/
{
    setInterceptsMouseClicks(false, true);
    setPaintingIsUnclipped(true);

    for (int i = 0; i < ColorPaletteColors::colorArray.size(); i++)
    {
        auto color = ColorPaletteColors::colorArray[i];
        auto button = new juce::ShapeButton(color.toDisplayString(true), color.darker(0.5f), color.withAlpha(0.8f), color.brighter());

        
        button->setClickingTogglesState(true);
        button->setOnColours(color.brighter(), color.brighter(0.5f), color.withAlpha(0.8f));
        button->shouldUseOnColours(true);
        button->setRadioGroupId(ColorRadio::colorRadioGroupId);
        
        button->onClick = [this, color, button] { colorClicked(color, button); };
        colorButtons.add(button);
        
        addAndMakeVisible(button);
    }
}

ColorPalette::~ColorPalette()
{
}

void ColorPalette::resized()
{
    auto area = getLocalBounds();
    //int buttonW = colorOnly ? 30 : 15;
    int buttonW = area.getWidth() * 0.18f;
    int buttonH = buttonW/* colorOnly ? 30 : 15*/;
    int space = area.getWidth() * 0.10f;
    int displayIndex = 0;                   //using as second row x placement

    for (int i = 0; i < colorButtons.size(); i++)
    {
        auto button = colorButtons[i];

        int bx = (i * buttonW) + space;
        int by = buttonH + space;

        if (bx + buttonW  > area.getWidth())
        {
            by *= 2;
            bx = (displayIndex++ * buttonW) + (space /** 2*/); //we increment the index because we are no longer using the for loop i index to get our x position
        }

        juce::Rectangle<int> buttonArea{ bx,  by, buttonW, buttonH };

        button->setBounds(buttonArea);

        juce::Path shape;
        shape.addEllipse(buttonArea.toFloat());
        button->setShape(shape, true, true, false);
    }
}

void ColorPalette::colorClicked(juce::Colour clickedColor, juce::ShapeButton* buttonClicked)
{
    selectedColor = clickedColor;
    parent.colorWasChanged(true);
    if (parent.hasMidi())
    {
        parent.showConfirmButton();
    }
}

juce::Colour ColorPalette::getSelectedColor()
{
    return selectedColor;
}

bool ColorPalette::isColorSelected()
{
    return selectedColor != juce::Colours::white;
}

juce::Colour* ColorPalette::getLastRandomColor()
{
    return lastRandomColor;
}
