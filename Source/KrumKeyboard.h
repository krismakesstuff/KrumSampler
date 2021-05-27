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
*/
class KrumKeyboard  :   public juce::MidiKeyboardComponent
                        //public juce::Timer,
                        //public juce::SettableTooltipClient
{
public:
    KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation or, KrumModuleContainer& container );
    ~KrumKeyboard() override;

    //void paint (juce::Graphics&) override;
    //void resized() override;

    //juce::String getTooltip() override;

    //void timerCallback() override;

    //void mouseEnter(const juce::MouseEvent& e) override;

    bool mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;
    void mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e) override;

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

    void assignMidiNoteColor(int midiNote, juce::Colour moduleColor);
    void removeMidiNoteColorAssignment(int midiNote);
    
    bool isMidiNoteAssigned(int midiNote);

    void setKeyDown(int midiNote, bool isKeyDown);


private:

    std::map<int, juce::Colour> currentlyAssignedMidiNotes{};

    KrumModuleContainer& moduleContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumKeyboard)
};
