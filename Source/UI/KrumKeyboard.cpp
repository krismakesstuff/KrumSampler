/*
  ==============================================================================

    KrumKeyboard.cpp
    Created: 6 Mar 2021 12:09:55pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../UI/KrumKeyboard.h"
#include "../UI/KrumModuleContainer.h"
#include "../UI/KrumModule/KrumModuleEditor.h"  
#include "../UI/InfoPanel.h"
#include "../PluginProcessor.h"

//==============================================================================
KrumKeyboard::KrumKeyboard(juce::MidiKeyboardState& midiState, juce::MidiKeyboardComponent::Orientation ori , KrumModuleContainer& container, juce::ValueTree& valTree)
    : juce::MidiKeyboardComponent(midiState, ori), moduleContainer(container), valueTree(valTree)
{
    //setColour(juce::MidiKeyboardComponent::ColourIds::upDownButtonBackgroundColourId, juce::Colours::darkgrey.darker());
    
    setScrollButtonsVisible(true);

    valueTree.addListener(this);
}

KrumKeyboard::~KrumKeyboard()
{
}

void KrumKeyboard::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText("Keyboard", "This keyboard shows your midi inputs and assignments. Can be clicked on to send a midi message");
    juce::MidiKeyboardComponent::mouseEnter(e);
    //DBG("MouseEntered Keyboard: " + e.getPosition().toString());
}

void KrumKeyboard::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    clearModulesMouseOverKeys();
    juce::MidiKeyboardComponent::mouseExit(e);
}

void KrumKeyboard::mouseMove(const juce::MouseEvent& e)
{
    setModulesMouseOverKey(e, true);
    juce::MidiKeyboardComponent::mouseMove(e);
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
            auto modules = moduleContainer.getModulesFromMidiNote(midiNoteNumber);
            for (int i = 0; i < modules.size(); ++i)
            {
                auto mod = modules[i];
                mod->setModulePlaying(true);
            }
        }
    }
    
    return true;
}

void KrumKeyboard::mouseUpOnKey(int midiNoteNumber, const juce::MouseEvent& e)
{
    if (isMidiNoteAssigned(midiNoteNumber))
    {
        auto modules = moduleContainer.getModulesFromMidiNote(midiNoteNumber);
        for (int i = 0; i < modules.size(); ++i)
        {
            auto mod = modules[i];
            mod->setModulePlaying(false);
        }

        //mod->setModulePlaying(false);
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

    if (isKeyHighlighted(midiNoteNumber) && showHighlight) //the key is already highlighted, so we do nothing. (multiple modules have the same assignment)
    {
        return;
    }
    
    auto keyRect = getRectangleForKey(midiNoteNumber).toNearestInt();

    if (keyToHighlight > -1 && showHighlight) //if we are changing the highlighted key, we clear the old one
    {
        clearHighlightedKey();
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

}

bool KrumKeyboard::isKeyHighlighted(int midiNoteToTest)
{
    return midiNoteToTest == keyToHighlight;
}

void KrumKeyboard::clearHighlightedKey()
{
    if (keyToHighlight == -1)
    {
        return;
    }
    else
    {
        int oldKey = keyToHighlight;
        keyToHighlight = -1;
        repaint(getRectangleForKey(oldKey).toNearestInt());
    }
}

//Both the black and white note drawing functions are largely copy pasted from Juce, if the note is assigned then we change the color to the assigned color
void KrumKeyboard::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle< float > area,
                                bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    bool isHightlighted = midiNoteNumber == keyToHighlight;
    //bool isAssignedColor = isMidiNoteAssigned(midiNoteNumber);

    float lightness = isOver ? 0.4f : 0.5f;
    
    if (isDown)
    {
        lightness -= 0.3f;
    }

    juce::ColourGradient grade;
    grade.point1 = area.getTopLeft();
    grade.point2 = area.getBottomLeft();

    auto keyColors = getColorsForKey(midiNoteNumber);

    if (keyColors.size() > 1)
    {
        
        auto gradeRange = juce::NormalisableRange<double>(0, keyColors.size());
        gradeRange.setSkewForCentre(keyColors.size() * 0.2f);

        for (int i = 0; i < keyColors.size(); ++i)
        {
            grade.addColour(gradeRange.convertTo0to1(i), keyColors[i].withLightness(lightness));
        }
        
    }
    else if (keyColors.size() == 1)
    {
        auto keyColor = keyColors[0];
        grade.addColour(0.0, keyColor.withLightness(lightness));
        grade.addColour(1.0, keyColor.withLightness(lightness - 0.01f));
    }
    else 
    {
        grade.addColour(0.0, juce::Colours::white/*.withLightness(lightness)*/);
        grade.addColour(1.0, juce::Colours::white.withLightness(lightness + 0.01f));
    }
    
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
    
    float lightness = isOver ? 0.4f : 0.3f;
    float alpha = 0.9f;

    if (isDown)
    {
        lightness += 0.3f;
        alpha -= 0.1f;
    }

    juce::ColourGradient grade;
    grade.point1 = area.getTopLeft();
    grade.point2 = area.getBottomLeft();

    auto keyColors = getColorsForKey(midiNoteNumber);

    if (keyColors.size() > 1)
    {
        auto gradeRange = juce::NormalisableRange<double>(0, keyColors.size());

        for (int i = 0; i < keyColors.size(); ++i)
        {
            //grade.addColour(gradeRange.convertTo0to1(i), keyColors[i].withAlpha(alpha));
            grade.addColour(gradeRange.convertTo0to1(i), keyColors[i].withLightness(lightness + i * 0.1f));
        }

    }
    else if (keyColors.size() == 1)
    {
        auto keyColor = keyColors[0];
       /* grade.addColour(0.0, keyColor.withAlpha(alpha));
        grade.addColour(1.0, keyColor.withAlpha(alpha));*/
        grade.addColour(0.0, keyColor/*.withLightness(lightness)*/);
        grade.addColour(1.0, keyColor.withLightness(lightness));
    }
    else
    {
        grade.addColour(0.0, c.withLightness(lightness));
        grade.addColour(1.0, juce::Colours::black.brighter(0.1f));
    }

    //if (isDown) grade.multiplyOpacity(0.4f);
    //if (isOver) grade.multiplyOpacity(0.6f);


    //auto grade = juce::ColourGradient::vertical(c, juce::Colours::black, area);

    /*if (isDown)
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
    }*/
    
    g.setGradientFill(grade);
    g.fillRect(area);

    if (isHightlighted)
    {
        g.setColour(highlightKeyColor);
        g.drawRect(area, highlightThickness);
    }

}

bool KrumKeyboard::isMidiNoteAssigned(int midiNote)
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());

    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto modTree = modulesTree.getChild(i);
        if ((int)modTree.getProperty(TreeIDs::moduleState.getParamID()) == KrumModule::ModuleState::active
            && (int)modTree.getProperty(TreeIDs::moduleMidiNote.getParamID()) == midiNote)
        {
            return true;
        }
    }

    return false;
}

void KrumKeyboard::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if (treeWhoChanged.hasType(TreeIDs::MODULE.getParamID()) && (property == juce::Identifier(TreeIDs::moduleColor.getParamID()) || property == juce::Identifier(TreeIDs::moduleMidiNote.getParamID())))
    {
        repaint(getRectangleForKey(treeWhoChanged.getProperty(TreeIDs::moduleMidiNote.getParamID())).toNearestInt());
    }
}

int KrumKeyboard::getLowestKey()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());

    int lowest = -1;
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto modTree = modulesTree.getChild(i);
        if ((int)modTree.getProperty(TreeIDs::moduleState.getParamID()) == KrumModule::ModuleState::active)
        {
            auto key = (int)modTree.getProperty(TreeIDs::moduleMidiNote.getParamID());
            if (lowest == -1 || key < lowest)
            {
                lowest = key;
            }
        }
    }

    return lowest;
}

juce::Array<juce::Colour> KrumKeyboard::getColorsForKey(int midiNote)
{
    juce::Array<juce::Colour> retColors{};

    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());

    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto modTree = modulesTree.getChild(i);
        if ((int)modTree.getProperty(TreeIDs::moduleMidiNote.getParamID()) == midiNote)
        {
            retColors.add(juce::Colour::fromString(modTree.getProperty(TreeIDs::moduleColor.getParamID()).toString()));
        }
    }

    return retColors;
}

void KrumKeyboard::setModulesMouseOverKey(const juce::MouseEvent& e, bool mouseOver)
{
    int moduleNote = getNoteAtPosition(e.getPosition().toFloat());
    if (moduleNote > 0)
    {
        auto mods = moduleContainer.getModulesFromMidiNote(moduleNote);
        //DBG("Module Note Over: " + juce::String(moduleNote));

        clearModulesMouseOverKeys();

        for (int i = 0; i < mods.size(); ++i)
        {
            mods[i]->setMouseOverKey(mouseOver);
            //mods[i]->repaint();
        }
        
    }
}

void KrumKeyboard::clearModulesMouseOverKeys()
{
    for (int i = 0; i < moduleContainer.getNumActiveModules(); ++i)
    {
        moduleContainer.getActiveModuleEditor(i)->setMouseOverKey(false);
    }
}

//void KrumKeyboard::assignMidiNoteColor(int midiNote, juce::Colour moduleColor/*, bool deleteOld*/)
//{
//    if (isMidiNoteAssigned(midiNote))
//    {
//        updateMidiNoteColor(midiNote, moduleColor);
//    }
//    else
//    {
//        auto newKey = new KrumKey();
//        newKey->midiNote = midiNote;
//        newKey->addColor(moduleColor);
//        currentlyAssignedKeys.add(std::move(newKey));
//    }
//
//    repaint();
//}
//
//void KrumKeyboard::removeMidiNoteColorAssignment(int midiNote, bool shouldRepaint)
//{
//    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//    {
//        if (midiNote == currentlyAssignedKeys[i]->midiNote)
//        {
//            currentlyAssignedKeys.remove(i);
//            if (shouldRepaint)
//            {
//                repaint();
//            }
//        }
//    }
//}
//
//void KrumKeyboard::updateMidiNoteColor(int noteToUpdate, juce::Colour newColor)
//{
//    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//    {
//        auto key = currentlyAssignedKeys[i];
//        if (key->midiNote == noteToUpdate)
//        {
//            key->addColor(newColor);
//        }
//    }
//}
//
//bool KrumKeyboard::isMidiNoteAssigned(int midiNote)
//{
//    if (midiNote > 0)
//    {
//        for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//        {
//            if (midiNote == currentlyAssignedKeys[i]->midiNote)
//            {
//                return true;
//            }
//        }
//    }
//
//    return false;
//}
//
//void KrumKeyboard::updateKeysFromValueTree()
//{
//    
//
//    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);
//
//    for (int i = 0; i < modulesTree.getNumChildren(); i++)
//    {
//        auto modTree = modulesTree.getChild(i);
//        if ((int)modTree.getProperty(TreeIDs::moduleState) == KrumModule::ModuleState::active)
//        {
//            int midiNote = (int)modTree.getProperty(TreeIDs::moduleMidiNote);
//            auto color = juce::Colour::fromString(modTree.getProperty(TreeIDs::moduleColor).toString());
//            
//            assignMidiNoteColor(midiNote, color);
//
//
//            /*if (auto key = getKeyFromMidiNote(midiNote))
//            {
//                for (int i = 0; i < key->colors.size(); ++i)
//                {
//                    if (color != *key->colors[i])
//                    {
//                        key->addColor(color);
//                    }
//                }
//            }
//            else
//            {
//                assignMidiNoteColor(midiNote, color);
//            }
//            */
//        }
//    }
//
//    printCurrentlyAssignedMidiNotes();
//}

//void KrumKeyboard::printCurrentlyAssignedMidiNotes()
//{
//
//    if (currentlyAssignedKeys.size() == 0)
//    {
//        DBG("No Keys currently assigned");
//        return;
//    }
//
//    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//    {
//        auto key = currentlyAssignedKeys[i];
//        juce::String midiNote(key->midiNote);
//        DBG("Assigned Note: " + midiNote + ", Colors:");
//        for (int i = 0; i < key->colors.size(); ++i)
//        {
//            DBG(" " + key->colors[i]->toDisplayString(true));
//        }
//
//        DBG(".");
//    }
//
//}

//juce::OwnedArray<juce::Colour>& KrumKeyboard::getColorsFromMidiNote(int midiNote)
//{
//    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//    {
//        if (midiNote == currentlyAssignedKeys[i]->midiNote)
//        {
//            return currentlyAssignedKeys[i]->colors;
//        }
//    }
//}

//bool KrumKeyboard::hasAssignedKeys()
//{
//    return currentlyAssignedKeys.size() > 0;
//}


//int KrumKeyboard::getHighestKey()
//{
//    int highest = -1;
//    for (int i = 0; i < currentlyAssignedKeys.size(); i++)
//    {
//        auto key = currentlyAssignedKeys[i];
//        if (highest == -1 || key->midiNote > highest)
//        {
//            highest = key->midiNote;
//        }
//    }
//
//    return highest;
//}

//unchecked, returns nullptr if not found
//KrumKeyboard::KrumKey* KrumKeyboard::getKeyFromMidiNote(int midiNote)
//{
//    for (int i = 0; i < currentlyAssignedKeys.size(); ++i)
//    {
//        auto key = currentlyAssignedKeys[i];
//        if (key->midiNote == midiNote)
//        {
//            return key;
//        }
//    }
//
//    return nullptr;
//}

