/*
  ==============================================================================

    KrumKeyboard.h
    Created: 6 Mar 2021 12:09:55pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class KrumModuleContainer;

//==============================================================================
/*
* 
* This is the keyboard you see at the bottom of the app,
* it draws a normal keyboard but also holds color assignments of choosen notes. 
* This handles drawing the correct notes, but also can be clicked, and triggers the sample file, if any exists for that note. 
* 
*/

class KrumKeyboard  :   public juce::MidiKeyboardComponent
{
public:
    KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori, KrumModuleContainer& container );
    ~KrumKeyboard() override;


    bool mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;
    void mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

    void assignMidiNoteColor(int midiNote, juce::Colour moduleColor, int oldNote = 0);
    void removeMidiNoteColorAssignment(int midiNote, bool repaint = true);
    bool isMidiNoteAssigned(int midiNote);

    

    void setKeyDown(int midiNote, bool isKeyDown);

    void updateKeysFromContainer();

    void printCurrentlyAssignedMidiNotes();

private:

    std::map<int, juce::Colour> currentlyAssignedMidiNotes{};

    KrumModuleContainer& moduleContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumKeyboard)
};
