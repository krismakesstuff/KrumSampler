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
#include "InfoPanel.h"

class KrumModuleEditor;

/*
* This class represents the overlay that lives on top of the modules when settings are being changed. 
* Midi and color can be changed from this overlay and it is shown by default when a module is created.  
* 
* TODO:
* - Redeisgn GUI (feels crowded)
* 
*/

class ModuleSettingsOverlay :   public juce::Component,
                                public juce::Timer
{
public:

    ModuleSettingsOverlay(KrumModuleEditor& parent);
    ~ModuleSettingsOverlay() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void handleMidiInput(int midiChannelNumber, int midiNoteNumber);

    void setOverlaySelected(bool isSelected);
    bool isOverlaySelected();

    void showConfirmButton();
    void confirmButtonClicked();

    void showButtons();
    void cancelSettings();
    void hideButtons();

    juce::Colour getSelectedColor();
    bool isModuleOverlaySelected();

    void setMidiListen(bool shouldListen);
    void setMidi(int midiNote, int midiChannel);
    void setMidiLabels();
    bool hasMidi();

    void setTitle(juce::String& newTitle);

    void keepCurrentColor(bool keepColor);
    void colorWasChanged(bool colorWasChanged);

private:

    void timerCallback() override;
    void midiListenButtonClicked();

    //does not set confirm button visibility
    void setButtonVisibilities(bool shouldBeVisible);

    void setMidiLabelColors();
    void visibilityChanged() override;

    InfoPanelTextButton midiListenButton{"Midi Listen", "Activate this to play or click a new midi assignment. NOTE: will turn red when listening"};
    InfoPanelTextButton confirmButton{"Confirm Button", "This will confirm all changes and remove the settings overlay"};
    InfoPanelTextButton deleteButton{"Delete", "This will delete your module!"};
    InfoPanelTextButton cancelButton{ "Cancel", "This will exit the settings overlay without keeping any of the changes" };
    
    InfoPanelLabel titleBox{"Module Name", "Double-click to edit."};

    InfoPanelLabel midiNoteNumberLabel{"Midi Note", "The currently assigned Midi Note. To change, enable the Midi Listen button"};
    InfoPanelLabel midiNoteTitleLabel{ "Midi Note", "The currently assigned Midi Note. To change, enable the Midi Listen button" };
    InfoPanelLabel midiChannelNumberLabel{ "Midi Channel", "The currently assigned Midi Channel. To change, enable the Midi Listen button" };
    InfoPanelLabel midiChannelTitleLabel{ "Midi Channel", "The currently assigned Midi Channel. To change, enable the Midi Listen button" };


    //InfoPanelLabel

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
    //bool isColorOnly = false;
    bool keepColorOnExit = false;
    bool colorChanged = false;

    JUCE_LEAK_DETECTOR(ModuleSettingsOverlay)
};
