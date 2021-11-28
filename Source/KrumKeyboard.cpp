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
#include "PluginProcessor.h"

//==============================================================================
KrumKeyboard::KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori , KrumModuleContainer& container, juce::ValueTree& valTree)
    : juce::MidiKeyboardComponent(midiState, ori), moduleContainer(container), valueTree(valTree)
{
    setScrollButtonsVisible(true);
    //updateKeysFromContainer();
    updateKeysFromValueTree();
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
           // moduleContainer.matchModuleDisplayToMidiNotes(getMidiAssignmentsInOrder());
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
        //c = c.interpolatedWith(currentlyAssignedMidiNotes.find(midiNoteNumber)->second, 0.8f);
        c = c.interpolatedWith(findColorFromMidiNote(midiNoteNumber), 0.8f);
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
        //c = c.interpolatedWith(currentlyAssignedMidiNotes.find(midiNoteNumber)->second, 0.8f);
        c = c.interpolatedWith(findColorFromMidiNote(midiNoteNumber), 0.8f);
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

//oldNote is default 0;
void KrumKeyboard::assignMidiNoteColor(int midiNote, juce::Colour moduleColor, int oldNote)
{
    //we test the midi note to make sure the same module doesn't have multiple assignments
    int testNote = oldNote > 0 ? oldNote : midiNote;
    if (isMidiNoteAssigned(testNote))
    {
        removeMidiNoteColorAssignment(testNote);
    }

    KrumKey newKey;
    newKey.midiNote = midiNote;
    newKey.color = moduleColor;
    currentlyAssignedKeys.add(newKey);
    //currentlyAssignedMidiNotes.emplace(std::make_pair(midiNote, moduleColor));
    
   // const juce::MessageManagerLock mm;
    repaint();
}

void KrumKeyboard::removeMidiNoteColorAssignment(int midiNote, bool shouldRepaint)
{

    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
    {
        if (midiNote == currentlyAssignedKeys[i].midiNote)
        {
            currentlyAssignedKeys.remove(i);
            if (shouldRepaint)
            {
                repaint();
            }
        }
    }

    //auto search = currentlyAssignedMidiNotes.find(midiNote);
    //if (search != currentlyAssignedMidiNotes.end())
    //{
    //    currentlyAssignedMidiNotes.erase(midiNote);
    //    if (shouldRepaint)
    //    {
    //        repaint();
    //    }
    //}
}

void KrumKeyboard::updateMidiNoteColor(int noteToUpdate, juce::Colour newColor)
{
    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
    {
        auto key = currentlyAssignedKeys[i];
        if (key.midiNote == noteToUpdate)
        {
            key.color = newColor;
        }
    }
}

bool KrumKeyboard::isMidiNoteAssigned(int midiNote)
{
    if (midiNote > 0)
    {
        for (int i = 0; i < currentlyAssignedKeys.size(); i++)
        {
            if (midiNote == currentlyAssignedKeys[i].midiNote)
            {
                return true;
            }
        }


        //return currentlyAssignedMidiNotes.find(midiNote) != currentlyAssignedMidiNotes.end();
    }
    return false;
}

//void KrumKeyboard::updateKeysFromContainer()
//{
//    //not getting called on init from plugin editor?
//    auto displayOrder = moduleContainer.getModuleDisplayOrder();
//    for (int i = 0; i < displayOrder.size(); i++)
//    {
//        auto modEd = displayOrder[i];
//        if (modEd->getModuleMidiNote() > 0)
//        {
//            assignMidiNoteColor(modEd->getModuleMidiNote(), modEd->getModuleColor());
//        }
//    }
//
//    printCurrentlyAssignedMidiNotes();
//    repaint();
//}

void KrumKeyboard::updateKeysFromValueTree()
{
    auto krumModuleTree = valueTree.getChildWithName("KrumModules");

    for (int i = 0; i < krumModuleTree.getNumChildren(); i++)
    {
        auto modTree = krumModuleTree.getChildWithName("Module" + juce::String(i));
        if (modTree.isValid())
        {
            for (int j = 1; j < modTree.getNumChildren(); j++) //we start at 1, the 0 index is the state which we grab manually
            {
                auto stateTree = modTree.getChild(0);
                auto stateId = stateTree.getProperty("id");
                auto stateVal = stateTree.getProperty("value");

                KrumModule::ModuleState state;

                if (stateId.toString() == TreeIDs::paramModuleState_ID)
                {
                    state = static_cast<KrumModule::ModuleState>((int)stateVal);
                    if (state == KrumModule::ModuleState::active)
                    {
                        KrumKey newKey;

                        juce::var id;
                        juce::var val;
                        auto modChild = modTree.getChild(i);
                        id = modChild.getProperty("id");

                        if (id.toString() == TreeIDs::paramModuleMidiNote_ID)
                        {
                            newKey.midiNote = (int)modChild.getProperty("val");
                        }
                        
                        if (id.toString() == TreeIDs::paramModuleColor_ID)
                        {
                            newKey.color = juce::Colour::fromString(modChild.getProperty("val").toString());
                        }

                        if (!newKey.color.isOpaque() && newKey.midiNote > 0)
                        {
                            currentlyAssignedKeys.add(newKey);
                            break;
                        }

                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    printCurrentlyAssignedMidiNotes();
    repaint();
}

void KrumKeyboard::printCurrentlyAssignedMidiNotes()
{

    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
    {
        juce::String midiNote(currentlyAssignedKeys[i].midiNote);
        juce::String color(currentlyAssignedKeys[i].color.toDisplayString(true));
        DBG("Assigned Note: " + midiNote + ", Color: " + color);
    }

    /*for (auto it = currentlyAssignedMidiNotes.begin(); it != currentlyAssignedMidiNotes.end(); it++)
    {
        juce::String midiNote(it->first);
        juce::String color(it->second.toDisplayString(true));

        DBG("Assigned Note: " + midiNote + ", Color: " + color);
    }*/
}

juce::Colour KrumKeyboard::findColorFromMidiNote(int midiNote)
{
    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
    {
        if (midiNote == currentlyAssignedKeys[i].midiNote)
        {
            return currentlyAssignedKeys[i].color;
        }
    }
}

//juce::Array<int> KrumKeyboard::getMidiAssignmentsInOrder()
//{
//    juce::Array<int> retArray;
//    
//    int lastMidiNote = 0; //64
//    
//    for (auto it = currentlyAssignedMidiNotes.begin(); it != currentlyAssignedMidiNotes.end(); it++)
//    {
//        int currentMidiNote = it->first;
//        if(currentMidiNote > lastMidiNote)
//        {
//            retArray.add(currentMidiNote);
//        }
//        else if(currentMidiNote < lastMidiNote)
//        {
//            retArray.insert(retArray.getLast() - 1, currentMidiNote);
//        }
//        
//        lastMidiNote = currentMidiNote;
//    }
//    
//    return retArray;
//    
//}
