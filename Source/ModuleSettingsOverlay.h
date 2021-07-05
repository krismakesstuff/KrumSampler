/*
  ==============================================================================

    ModuleSettingsOverlay.h
    Created: 6 May 2021 11:58:52am
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ColorPalette.h"

class KrumModule;

/*
* This class represents the overlay that lives on top of the modules. Midi and color can be changed from this overlay and is shown by default when a module is created. 
* You can also access it from the settings menu on the module. 
*/

class ModuleSettingsOverlay : public juce::Component
{
public:

    ModuleSettingsOverlay(juce::Rectangle<int> area, KrumModule& parent);
    ~ModuleSettingsOverlay() override;
    
    void paint(juce::Graphics& g) override;

    //void mouseDown(const juce::MouseEvent& e) override;

    void handleMidiInput(int midiChannelNumber, int midiNoteNumber);
    void showConfirmButton();
    void confirmMidi();

    void setOverlaySelected(bool isSelected);
    bool isOverlaySelected();

    void showButtons();

    void cancelSettings();
    void hideButtons();

    juce::Colour getSelectedColor();
    bool isModuleOverlaySelected();

    void setMidi(int midiNote, int midiChannel);
    void setMidiLabels();
    bool hasMidi();

    void keepCurrentColor(bool keepColor);
    void colorWasChanged(bool colorWasChanged);
private:

    juce::TextButton confirmButton;
    juce::TextButton deleteButton;
    juce::TextButton cancelButton;
    juce::Label titleBox;
    //juce::TextEditor midiTitleBox;

    juce::Label midiNoteNumberLabel;
    juce::Label midiNoteTitleLabel{ "Midi Note", "Midi Note" };
    juce::Label midiChannelNumberLabel;
    juce::Label midiChannelTitleLabel{ "Midi Channel", "Midi Channel" };

    ColorPalette colorPalette;
    KrumModule& parentModule;

    int midiNoteNum = 0;
    int midiChanNum = 0;

    bool moduleOverlaySelected = false;
    bool keepColorOnExit = false;
    bool colorChanged = false;
    float cornerSize = 5.0f;
    float outlineSize = 1.0f;

    juce::Colour moduleSelectedColor{ juce::Colours::white };

    JUCE_LEAK_DETECTOR(ModuleSettingsOverlay)
};