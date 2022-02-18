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
* This is the keyboard you see at the bottom of the plug-in,
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

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    
    void scrollToKey(int midiNoteNumber);

    void setHighlightKey(int midiNoteNumber, bool showHighlight);
    bool isKeyHighlighted(int midiNoteToTest);
    void clearHighlightedKey();

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

    //void assignMidiNoteColor(int midiNote, juce::Colour moduleColor/*, bool deleteOld*/);
    //void removeMidiNoteColorAssignment(int midiNote, bool repaint = true);
    //void updateMidiNoteColor(int noteToUpdate, juce::Colour newColor);
    bool isMidiNoteAssigned(int midiNote);

    //void updateKeysFromValueTree();
    

    //void printCurrentlyAssignedMidiNotes();
    //juce::OwnedArray<juce::Colour>& getColorsFromMidiNote(int midiNote);
    

    //bool hasAssignedKeys();
    int getLowestKey();
    //int getHighestKey();

private:

    //struct KrumKey
    //{
    //    KrumKey() = default;
    //    ~KrumKey() = default;

    //    //bool hasAnyColor() { return colors.size() > 0; }
    //    //bool hasMidiNote() { return midiNote > 0; }

    //    void addColor(juce::Colour& newColor) { colors.add(std::move(&newColor)); }
    //    void clearColors() { colors.clear(); }
    //

    //    int midiNote = 0;
    //    juce::OwnedArray<juce::Colour> colors{};

    //};

    //unchecked, returns nullptr if not found
   // KrumKey* getKeyFromMidiNote(int midiNote);

    juce::Array<juce::Colour> getColorsForKey(int midiNote);

    //void updateKey(int midiNote, juce::Colour color)

    int autoscrollOffset = 35;

    int keyToHighlight = -1;
    juce::Colour highlightKeyColor{ juce::Colours::yellow.darker() };
    float highlightThickness = 2.0f;

    //juce::OwnedArray<KrumKey, juce::CriticalSection> currentlyAssignedKeys;

    KrumModuleContainer& moduleContainer;

    juce::ValueTree valueTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumKeyboard)
};