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
    updateKeysFromContainer();
}

KrumKeyboard::~KrumKeyboard()
{
}

bool KrumKeyboard::mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        auto mod = moduleContainer.getModuleFromMidiNote(midiNoteNumber);
        mod->setModulePlaying(true);
        mod->triggerNoteOnInParent();
    }
    
    return true;
}

void KrumKeyboard::mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        auto mod = moduleContainer.getModuleFromMidiNote(midiNoteNumber);
        mod->setModulePlaying(false);
    }
}

//Both the black and white note drawing functions are largely copy pasted from Juce, if the note is assigned then we change the color to the assigned color
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
//oldNote is default 0;
void KrumKeyboard::assignMidiNoteColor(int midiNote, juce::Colour moduleColor, int oldNote)
{
    int testNote = oldNote > 0 ? oldNote : midiNote;
    if (isMidiNoteAssigned(testNote))
    {
        removeMidiNoteColorAssignment(testNote);
    }

    currentlyAssignedMidiNotes.emplace(std::make_pair(midiNote, moduleColor));
    repaint();
}

void KrumKeyboard::removeMidiNoteColorAssignment(int midiNote, bool shouldRepaint)
{
    auto search = currentlyAssignedMidiNotes.find(midiNote);
    if (search != currentlyAssignedMidiNotes.end())
    {
        currentlyAssignedMidiNotes.erase(midiNote);
        if (shouldRepaint)
        {
            repaint();
        }
    }
}

bool KrumKeyboard::isMidiNoteAssigned(int midiNote)
{
    return currentlyAssignedMidiNotes.find(midiNote) != currentlyAssignedMidiNotes.end();
}

void KrumKeyboard::updateKeysFromContainer()
{
    auto displayOrder = moduleContainer.getModuleDisplayOrder();
    for (int i = 0; i < displayOrder.size(); i++)
    {
        auto modEd = displayOrder[i];
        if (modEd->getModuleMidiNote() > 0)
        {
            assignMidiNoteColor(modEd->getModuleMidiNote(), modEd->getModuleColor());
        }
    }

    printCurrentlyAssignedMidiNotes();
    repaint();
}

void KrumKeyboard::printCurrentlyAssignedMidiNotes()
{
    for (auto it = currentlyAssignedMidiNotes.begin(); it != currentlyAssignedMidiNotes.end(); it++)
    {
        juce::String midiNote(it->first);
        juce::String color(it->second.toDisplayString(true));

        DBG("Assigned Note: " + midiNote + ", Color: " + color);
    }
}