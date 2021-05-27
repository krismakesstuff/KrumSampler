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


//==============================================================================
ColorPalette::ColorPalette(juce::Rectangle<int> bounds, ModuleSettingsOverlay& pm)
    : parent(pm)
{
    setInterceptsMouseClicks(false, true);
    setPaintingIsUnclipped(true);
    setBounds(bounds);

    int numColors = ColorPaletteColors::colorArray.size();

    int displayIndex = 1;                   //using as second row x placement

    for (int i = 0; i < numColors; i++)
    {
        auto color = ColorPaletteColors::colorArray[i];
        auto button = new juce::ShapeButton(color.toDisplayString(true), color.darker(0.5f), color.withAlpha(0.8f), color.brighter());
        
        auto area = getLocalBounds();
        int buttonW = 15;
        int buttonH = 15;
        int space = 5;
        int bx = (buttonW + space) * (i + 1) - space;
        int by = buttonH + space;

        if (bx + buttonW > area.getWidth())
        {
            by *= 2;
            bx = (buttonW + space) * (displayIndex++) - space;
        }

        juce::Rectangle<int> buttonArea{ bx,  by, buttonW, buttonH };

        juce::Path shape;
        shape.addEllipse(buttonArea.toFloat());
        
        addAndMakeVisible(button);
        button->setBounds(buttonArea);
        button->setShape(shape, true, true, false);
        button->setClickingTogglesState(true);
        
        button->setOnColours(color.brighter(), color.brighter(0.5f), color.withAlpha(0.8f));
        button->shouldUseOnColours(true);
        button->setRadioGroupId(ColorRadio::colorRadioGroupId);
        
        button->onClick = [this, color, button] { colorClicked(color, button); };
        //button->onStateChange = [this, color] { selectedColor = color; DBG("Color: " + selectedColor.toDisplayString(true)); };
        colorButtons.add(button);
        
    }
}

ColorPalette::~ColorPalette()
{
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
