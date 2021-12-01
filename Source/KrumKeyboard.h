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
* This handles drawing the correct notes, but also can be clicked, and triggers the sample file, if any exist for that note. 
* 
*/

class KrumKeyboard  :   public juce::MidiKeyboardComponent,
                        public juce::ValueTree::Listener
{
public:
    KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori, KrumModuleContainer& container, juce::ValueTree& valTree);
    ~KrumKeyboard() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    bool mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;
    void mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

    //void drawUpDownButton(juce::Graphics& g, int w, int h, bool isMouseOver, bool isButtonPressed, bool movesOctavesUp) override;
    
    void assignMidiNoteColor(int midiNote, juce::Colour moduleColor, int oldNote = 0);
    void removeMidiNoteColorAssignment(int midiNote, bool repaint = true);
    void updateMidiNoteColor(int noteToUpdate, juce::Colour newColor);
    bool isMidiNoteAssigned(int midiNote);

    //void updateKeysFromContainer();
    void updateKeysFromValueTree();
    
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    void printCurrentlyAssignedMidiNotes();
    juce::Colour findColorFromMidiNote(int midiNote);
    //smallest to biggest
    //juce::Array<int> getMidiAssignmentsInOrder();

private:

    //std::map<int, juce::Colour> currentlyAssignedMidiNotes{};


    //struct KrumKeyboardLayout
  /*  {
        
    };*/
    struct KrumKey
    {
        KrumKey() = default;
        KrumKey(int midi, juce::Colour& c)
            : midiNote(midi), color(c) {}

        bool hasColor() { return color != juce::Colour(); }
        bool hasMidiNote() { return midiNote > 0; }

        int midiNote = 0;
        juce::Colour color = juce::Colour{};
    };


    juce::Array<KrumKey> currentlyAssignedKeys;

    KrumModuleContainer& moduleContainer;

    juce::ValueTree valueTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumKeyboard)
};
