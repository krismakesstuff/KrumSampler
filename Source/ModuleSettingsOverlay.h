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

class KrumModuleEditor;

/*
* This class represents the overlay that lives on top of the modules when settings are being changed. 
* Midi and color can be changed from this overlay and it is shown by default when a module is created. 
* You can also access it from the settings menu on the module. 
* 
* TODO:
* - Redeisgn GUI
* - Maybe even reconsider the KrumModule's settings menu, and it's interactions with this class
* 
*/

class ModuleSettingsOverlay : public juce::Component
{
public:

    ModuleSettingsOverlay(juce::Rectangle<int> area, KrumModuleEditor& parent, bool isColorOnly = false);
    ~ModuleSettingsOverlay() override;
    
    void paint(juce::Graphics& g) override;

    void handleMidiInput(int midiChannelNumber, int midiNoteNumber);

    void setOverlaySelected(bool isSelected);
    bool isOverlaySelected();

    void showConfirmButton();
    void confirmMidi();

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
    void setToOnlyShowColors(bool onlyShowColors);

private:

    juce::TextButton confirmButton;
    juce::TextButton deleteButton;
    juce::TextButton cancelButton;
    juce::Label titleBox;

    juce::Label midiNoteNumberLabel;
    juce::Label midiNoteTitleLabel{ "Midi Note", "Midi Note" };
    juce::Label midiChannelNumberLabel;
    juce::Label midiChannelTitleLabel{ "Midi Channel", "Midi Channel" };

    ColorPalette colorPalette;
    KrumModuleEditor& parentEditor;
    
    //Needs to be initialized white
    juce::Colour moduleSelectedColor{ juce::Colours::white };

    int midiNoteNum = 0;
    int midiChanNum = 0;
    

    float cornerSize = 5.0f;
    float outlineSize = 1.0f;

    bool moduleOverlaySelected = false;
    bool updateMidiLabels = false;
    bool showingConfirmButton = false;

    //Flags to set in different cases while using the overlay.. Needs a redesign
    bool isColorOnly = false;
    bool keepColorOnExit = false;
    bool colorChanged = false;

    JUCE_LEAK_DETECTOR(ModuleSettingsOverlay)
};
