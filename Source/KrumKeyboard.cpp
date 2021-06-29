/*
  ==============================================================================

    KrumKeyboard.cpp
    Created: 6 Mar 2021 12:09:55pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KrumKeyboard.h"
#include "KrumModuleContainer.h"
#include "KrumModuleEditor.h"

//==============================================================================
KrumKeyboard::KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori , KrumModuleContainer& container)
    : juce::MidiKeyboardComponent(midiState, ori), moduleContainer(container)
{
    setScrollButtonsVisible(true);
    //midiState.addListener(this);
  //  setTooltip("Hover over a note to see it's assigned sample");

    auto displayOrder = moduleContainer.getModuleDisplayOrder();

    for (int i = 0; i < displayOrder.size(); i++)
    {
        auto modEd = displayOrder[i];
        if (modEd->getModuleMidiNote() > 0)
        {
            assignMidiNoteColor(modEd->getModuleIndex(), modEd->getModuleColor());
        }
    }

}

KrumKeyboard::~KrumKeyboard()
{
}


//juce::String KrumKeyboard::getTooltip()
//{
//    auto comp = getComponentAt(getMouseXYRelative());
//
//    return juce::String();
//}

//void KrumKeyboard::timerCallback()
//{
//}
//
//void KrumKeyboard::mouseEnter(const juce::MouseEvent& e)
//{
//    startTimer
//
//}

bool KrumKeyboard::mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    //juce::MessageManagerLock lock;
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        auto mod = moduleContainer.getModuleFromMidiNote(midiNoteNumber);
        mod->setModulePlaying(true);
        //needs to trigger the sample
        //mod->
    }

    return true;
}

void KrumKeyboard::mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        moduleContainer.getModuleFromMidiNote(midiNoteNumber)->setModulePlaying(false);
    }
}

void KrumKeyboard::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
    bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    auto c = juce::Colours::transparentWhite;
    if (isDown)  c = findColour(keyDownOverlayColourId);
    if (isOver)  c = c.overlaidWith(findColour(mouseOverKeyOverlayColourId));

    if (isMidiNoteAssigned(midiNoteNumber))
    {
        c = c.interpolatedWith(currentlyAssignedMidiNotes.find(midiNoteNumber)->second, 0.8f);
        if (isDown)  c = c.darker(0.2f);
        if (isOver)  c = c.darker();
    }

    g.setColour(c); 
    g.fillRect(area);

    auto text = getWhiteNoteText(midiNoteNumber);

    if (text.isNotEmpty())
    {
        auto fontHeight = juce::jmin(12.0f, getKeyWidth() * 0.9f);

        g.setColour(textColour);
        g.setFont(juce::Font(fontHeight).withHorizontalScale(0.8f));

        switch (getOrientation())
        {
        case horizontalKeyboard:            g.drawText(text, area.withTrimmedLeft(1.0f).withTrimmedBottom(2.0f), juce::Justification::centredBottom, false); break;
        case verticalKeyboardFacingLeft:    g.drawText(text, area.reduced(2.0f), juce::Justification::centredLeft, false); break;
        case verticalKeyboardFacingRight:   g.drawText(text, area.reduced(2.0f), juce::Justification::centredRight, false); break;
        default: break;
        }
    }

    if (!lineColour.isTransparent())
    {
        g.setColour(lineColour);

        switch (getOrientation())
        {
        case horizontalKeyboard:            g.fillRect(area.withWidth(1.0f)); break;
        case verticalKeyboardFacingLeft:    g.fillRect(area.withHeight(1.0f)); break;
        case verticalKeyboardFacingRight:   g.fillRect(area.removeFromBottom(1.0f)); break;
        default: break;
        }

        if (midiNoteNumber == getRangeEnd())
        {
            switch (getOrientation())
            {
            case horizontalKeyboard:            g.fillRect(area.expanded(1.0f, 0).removeFromRight(1.0f)); break;
            case verticalKeyboardFacingLeft:    g.fillRect(area.expanded(0, 1.0f).removeFromBottom(1.0f)); break;
            case verticalKeyboardFacingRight:   g.fillRect(area.expanded(0, 1.0f).removeFromTop(1.0f)); break;
            default: break;
            }
        }
    }
}

void KrumKeyboard::drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
    bool isDown, bool isOver, juce::Colour noteFillColour)
{
    auto c = noteFillColour;
    if (isDown)  c = c.overlaidWith(findColour(keyDownOverlayColourId));
    if (isOver)  c = c.overlaidWith(findColour(mouseOverKeyOverlayColourId));

    if (isMidiNoteAssigned(midiNoteNumber))
    {
        c = c.interpolatedWith(currentlyAssignedMidiNotes.find(midiNoteNumber)->second, 0.8f);
        if (isDown)  c = c.darker(0.2f);
        if (isOver)  c = c.darker();
    }


    g.setColour(c);
    g.fillRect(area);

    if (isDown)
    {
        g.setColour(noteFillColour);
        g.drawRect(area);
    }
    else
    {
        g.setColour(c.brighter());
        auto sideIndent = 1.0f / 8.0f;
        auto topIndent = 7.0f / 8.0f;
        auto w = area.getWidth();
        auto h = area.getHeight();

        switch (getOrientation())
        {
        case horizontalKeyboard:            g.fillRect(area.reduced(w * sideIndent, 0).removeFromTop(h * topIndent)); break;
        case verticalKeyboardFacingLeft:    g.fillRect(area.reduced(0, h * sideIndent).removeFromRight(w * topIndent)); break;
        case verticalKeyboardFacingRight:   g.fillRect(area.reduced(0, h * sideIndent).removeFromLeft(w * topIndent)); break;
        default: break;
        }
    }
}

void KrumKeyboard::assignMidiNoteColor(int midiNote, juce::Colour moduleColor)
{
    if (isMidiNoteAssigned(midiNote))
    {
        removeMidiNoteColorAssignment(midiNote);
    }
    currentlyAssignedMidiNotes.emplace(std::make_pair(midiNote, moduleColor));
    juce::MessageManagerLock lock;
    repaint();
}

void KrumKeyboard::removeMidiNoteColorAssignment(int midiNote)
{
    auto search = currentlyAssignedMidiNotes.find(midiNote);
    if (search != currentlyAssignedMidiNotes.end())
    {
        currentlyAssignedMidiNotes.erase(midiNote);
        juce::MessageManagerLock lock;
        repaint();
    }
}

bool KrumKeyboard::isMidiNoteAssigned(int midiNote)
{
    return currentlyAssignedMidiNotes.find(midiNote) != currentlyAssignedMidiNotes.end();
}

void KrumKeyboard::setKeyDown(int midiNote, bool isKeyDown)
{
    juce::MessageManagerLock lock;
    repaint();
    

    //static const juce::uint8 whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
    //static const juce::uint8 blackNotes[] = { 1, 3, 6, 8, 10 };
    //int rangeStart = 0, rangeEnd = 127;
    //
    //for (int octave = 0; octave < 128; octave += 12)
    //{
    //    for (int white = 0; white < 7; ++white)
    //    {
    //        //auto noteNum = octave + whiteNotes[white];
    //        auto noteNum = octave + whiteNotes[white];

    //        //if (noteNum >= rangeStart && noteNum <= rangeEnd)
    //        if (noteNum == midiNote)
    //        {
    //            repaint();
    //            //drawWhiteNote(noteNum, g, getRectangleForKey(noteNum), isKeyDown, false, juce::Colour{}, juce::Colour{});
    //        }

    //    }
    //}



    //for (int octave = 0; octave < 128; octave += 12)
    //{
    //    for (int black = 0; black < 5; ++black)
    //    {
    //        auto noteNum = octave + blackNotes[black];

    //        //if (noteNum >= rangeStart && noteNum <= rangeEnd)
    //        if (noteNum == midiNote)
    //        {
    //            
    //            drawBlackNote(noteNum, g, getRectangleForKey(noteNum), isKeyDown, false, juce::Colour{});
    //        }

    //    }
    //}

    //

}
