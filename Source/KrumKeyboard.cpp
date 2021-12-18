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
    //setColour(juce::MidiKeyboardComponent::ColourIds::shadowColourId, juce::Colours::black);
    setColour(juce::MidiKeyboardComponent::ColourIds::upDownButtonBackgroundColourId, juce::Colours::darkgrey.darker());
    
    setScrollButtonsVisible(true);
    updateKeysFromValueTree();
    valueTree.addListener(this);
}

KrumKeyboard::~KrumKeyboard()
{
}

void KrumKeyboard::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText("Keyboard", "This keyboard shows your midi inputs and assignments. Can be clicked on to send a midi message");
    juce::MidiKeyboardComponent::mouseEnter(e);
    DBG("MouseEntered Keyboard: " + e.getPosition().toString());
}

void KrumKeyboard::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::MidiKeyboardComponent::mouseExit(e);
}

//void KrumKeyboard::mouseMove(const juce::MouseEvent& e)
//{
//    juce::MidiKeyboardComponent::mouseMove(e);
//    getKey
//    DBG("MouseMoved")
//}

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

void KrumKeyboard::scrollToKey(int midiNoteNumber)
{
    bool isKeyVisible = midiNoteNumber > getLowestVisibleKey() && midiNoteNumber < (getNoteAtPosition(getBounds().getTopRight().toFloat()));
    if (midiNoteNumber > 0 && !isKeyVisible)
    {
        setLowestVisibleKey(midiNoteNumber - autoscrollOffset); //the offset puts the desired key in the middle(ish) of the view
    }
}

void KrumKeyboard::setHighlightKey(int midiNoteNumber, bool showHighlight)
{
    auto keyRect = getRectangleForKey(midiNoteNumber).toNearestInt();

    if (keyToHighlight > -1) //if we are changing the highlighted key, we clear the old one
    {
        int oldKey = keyToHighlight;
        keyToHighlight = -1;
        repaint(getRectangleForKey(oldKey).toNearestInt());
    }
    
    if (showHighlight)
    {
        keyToHighlight = midiNoteNumber;
    }
    else
    {
        keyToHighlight = -1;
    }
    
    repaint(keyRect);

    /*auto keyX = getKeyStartPosition(midiNoteNumber);

    if (keyX > 0)
    {
        juce::Point<float> newKeyPos{ keyX + 5.0f, (float)event.getPosition().getY()};
        auto e = event.withNewPosition(newKeyPos);

        juce::MidiKeyboardComponent::mouseEnter(e);
        DBG("Show Key : " + juce::String(midiNoteNumber) + " pressed at " + newKeyPos.toString());
    }*/
}

bool KrumKeyboard::isKeyHighlighted(int midiNoteToTest)
{
    return midiNoteToTest == keyToHighlight;
}

//void KrumKeyboard::resetKeyMouseOver(/*int midiNoteNumber, const juce::MouseEvent& event*/)
//{
//    keyToShowMouseOver = -1; 
//    repaint(getRectangleForKey(midiNoteNumber).toNearestInt());
//
//    /*auto keyX = getKeyStartPosition(midiNoteNumber);
//    juce::Point<float> newKeyPos{ keyX + 5.0f, 5.0f };
//    auto e = event.withNewPosition(newKeyPos);
//    juce::MidiKeyboardComponent::mouseExit(e);
//    DBG("Show Key : " + juce::String(midiNoteNumber) + " Unpressed at " + newKeyPos.toString());*/
//}

//Both the black and white note drawing functions are largely copy pasted from Juce, if the note is assigned then we change the color to the assigned color
void KrumKeyboard::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                                bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    bool isHightlighted = midiNoteNumber == keyToHighlight;
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

    if (isHightlighted)
    {
        g.setColour(highlightKeyColor);
        g.drawRect(area, highlightThickness);
    }

}

void KrumKeyboard::drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                                bool isDown, bool isOver, juce::Colour noteFillColour)
{
    //bool isOver = midiNoteNumber == keyToShowMouseOver ? true : mouseOver;
    bool isHightlighted = midiNoteNumber == keyToHighlight;
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

    if (isHightlighted)
    {
        g.setColour(highlightKeyColor);
        g.drawRect(area, highlightThickness);
    }


}

void KrumKeyboard::assignMidiNoteColor(int midiNote, juce::Colour moduleColor/*, bool deleteOld*/)
{
    //we test the midi note to make sure the same key doesn't have multiple assignments
    if (isMidiNoteAssigned(midiNote))
    {
        //this is where you could implement some sort of gradient when assigning  the same key multiple colors. But for now, we only assign one
        removeMidiNoteColorAssignment(midiNote, false);
    }

    KrumKey newKey;
    newKey.midiNote = midiNote;
    newKey.color = moduleColor;
    currentlyAssignedKeys.add(newKey);

    /*if (deleteOld)
    {
        
    }*/


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
    currentlyAssignedKeys.clear();
    currentlyAssignedKeys.ensureStorageAllocated(MAX_NUM_MODULES);

    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);

    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto modTree = modulesTree.getChild(i);
        if ((int)modTree.getProperty(TreeIDs::moduleState) == KrumModule::ModuleState::active)
        {
            assignMidiNoteColor((int)modTree.getProperty(TreeIDs::moduleMidiNote), juce::Colour::fromString(modTree.getProperty(TreeIDs::moduleColor).toString()));
        }
    }

    printCurrentlyAssignedMidiNotes();
    //repaint();
}

void KrumKeyboard::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if (treeWhoChanged.hasType(TreeIDs::MODULE) && (property == TreeIDs::moduleColor || property == TreeIDs::moduleMidiNote))
    {
        //int midi = treeWhoChanged[TreeIDs::moduleMidiNote];
        //scrollToKey(midi);
        
        updateKeysFromValueTree();

    }
}

void KrumKeyboard::printCurrentlyAssignedMidiNotes()
{

    if (currentlyAssignedKeys.size() == 0)
    {
        DBG("No Keys currently assigned");
        return;
    }

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
