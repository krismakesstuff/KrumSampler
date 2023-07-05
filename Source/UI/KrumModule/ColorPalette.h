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


private:
    ModuleSettingsOverlay& parent;

    juce::Colour selectedColor{ juce::Colours::white };
    juce::Colour* lastRandomColor = nullptr;
    juce::OwnedArray<juce::ShapeButton> colorButtons;

    JUCE_LEAK_DETECTOR(ColorPalette)
};
