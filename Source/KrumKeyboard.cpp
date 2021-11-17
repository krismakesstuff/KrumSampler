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
#include "InfoPanel.h"

//==============================================================================
KrumKeyboard::KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori , KrumModuleContainer& container)
    : juce::MidiKeyboardComponent(midiState, ori), moduleContainer(container)
{
    setScrollButtonsVisible(true);
    updateKeysFromContainer();
    setColour(juce::MidiKeyboardComponent::ColourIds::shadowColourId, juce::Colours::black);
    setColour(juce::MidiKeyboardComponent::ColourIds::upDownButtonBackgroundColourId, juce::Colours::darkgrey.darker());
}

KrumKeyboard::~KrumKeyboard()
{
}

void KrumKeyboard::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText("Keyboard", "This keyboard shows your midi inputs and assignments. Can be clicked on to send a midi message");
    juce::MidiKeyboardComponent::mouseEnter(e);
}

void KrumKeyboard::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::MidiKeyboardComponent::mouseExit(e);
}

bool KrumKeyboard::mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        auto mods = e.mods;
        if(mods.isAltDown() && mods.isCommandDown() && mods.isCtrlDown())
        {
            moduleContainer.matchModuleDisplayToMidiNotes(getMidiAssignmentsInOrder());
        }
        else
        {
            auto mod = moduleContainer.getModuleFromMidiNote(midiNoteNumber);
            mod->setModulePlaying(true);
            mod->triggerNoteOnInParent();
        }
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

    //g.setColour(c);
    auto grade = juce::ColourGradient::vertical(c, juce::Colours::black, area);
    g.setGradientFill(grade);
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

    //g.setColour(c);
    auto grade = juce::ColourGradient::vertical(c, juce::Colours::black, area);
    g.setGradientFill(grade);
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

//void KrumKeyboard::drawUpDownButton(juce::Graphics &g, int w, int h, bool isMouseOver, bool isButtonPressed, bool movesOctavesUp)
//{
//    g.fillAll (findColour (upDownButtonBackgroundColourId));
//
//    float angle = 0;
//
//    switch (orientation)
//    {
//        case horizontalKeyboard:            angle = movesOctavesUp ? 0.0f  : 0.5f;  break;
//        case verticalKeyboardFacingLeft:    angle = movesOctavesUp ? 0.25f : 0.75f; break;
//        case verticalKeyboardFacingRight:   angle = movesOctavesUp ? 0.75f : 0.25f; break;
//        default:                            jassertfalse; break;
//    }
//
//    Path path;
//    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
//    path.applyTransform (AffineTransform::rotation (MathConstants<float>::twoPi * angle, 0.5f, 0.5f));
//
//    g.setColour (findColour (upDownButtonArrowColourId)
//                  .withAlpha (buttonDown ? 1.0f : (mouseOver ? 0.6f : 0.4f)));
//
//    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f, (float) w - 2.0f, (float) h - 2.0f, true));
//}


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

juce::Array<int> KrumKeyboard::getMidiAssignmentsInOrder()
{
    juce::Array<int> retArray;
    
    int lastMidiNote = 0; //64
    
    for (auto it = currentlyAssignedMidiNotes.begin(); it != currentlyAssignedMidiNotes.end(); it++)
    {
        int currentMidiNote = it->first;
        if(currentMidiNote > lastMidiNote)
        {
            retArray.add(currentMidiNote);
        }
        else if(currentMidiNote < lastMidiNote)
        {
            retArray.insert(retArray.getLast() - 1, currentMidiNote);
        }
        
        lastMidiNote = currentMidiNote;
    }
    
    return retArray;
    
}
