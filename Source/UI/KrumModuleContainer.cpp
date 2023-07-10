/*
  ==============================================================================

    KrumModuleContainer.cpp
    Created: 23 Mar 2021 2:56:05pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../UI/KrumModuleContainer.h"
#include "../UI/PluginEditor.h"
//#include "Log.h"

//==============================================================================

KrumModuleContainer::KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree)
    : pluginEditor(owner), valueTree(valTree), juce::Component("ModuleContainer")
{

    setWantsKeyboardFocus(true);

    pluginEditor->addKeyboardListener(this);
    pluginEditor->addKeyListener(this);
    valueTree.addListener(this);

    refreshModuleLayout();
    startTimerHz(30);
        
}

KrumModuleContainer::~KrumModuleContainer()
{
    editorBeingDragged = nullptr;
    pluginEditor->removeKeyboardListener(this);
    pluginEditor->removeKeyListener(this);
    valueTree.removeListener(this);
    currentlySelectedModules.clear(false);
}

void KrumModuleContainer::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
   g.setColour(Colors::getModuleBGColor());
   g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);

}

void KrumModuleContainer::refreshModuleLayout()
{
    int numModules = activeModuleEditors.size();
    auto area = getLocalBounds();
    auto viewportBounds = pluginEditor->modulesViewport.getBounds();
    int viewportWidth = viewportBounds.getWidth();
    int viewportHeight = viewportBounds.getHeight();
    int newWidth = numModules == 0 ? viewportWidth : (numModules + 1) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

    //position modules based of displayIndex
    for (int i = 0; i < numModules; i++)
    {
        auto modEd = activeModuleEditors[i];
        modEd->setTopLeftPosition(((modEd->getModuleDisplayIndex() * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage()), EditorDimensions::shrinkage); //set position based off of stored index
    }

}

void KrumModuleContainer::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if (property == juce::Identifier(TreeIDs::moduleState.getParamID()))
    {
        int state = treeWhoChanged.getProperty(TreeIDs::moduleState.getParamID());
        int index = treeWhoChanged.getProperty(TreeIDs::moduleDisplayIndex.getParamID());
        if (state == KrumModule::ModuleState::empty && index > -1) //if we set this module to an empty state and the editor is still showing, we remove it
        {
            removeModuleEditor(getEditorFromDisplayIndex(index));
        }
        else if (state > 0 && index > -1 && activeModuleEditors.size() > 0)
        {
            //moduleEditors[index]->repaint();
        }
    }
    else if (property == juce::Identifier(TreeIDs::moduleDisplayIndex.getParamID()))
    {
        //all other cases we don't want to refresh layout
        if (editorBeingDragged && editorBeingDragged != getEditorFromDisplayIndex(treeWhoChanged.getProperty(TreeIDs::moduleDisplayIndex.getParamID())))
        {
            refreshModuleLayout();
        }
        else if(!isModuleDragging())
        {
            refreshModuleLayout();
        }
    }

}

void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    //capture a mouseclick that isn't on any modules, that will clear all selected modules
    deselectAllModules();
    clearActiveModuleSettingsOverlays();
    
}

bool KrumModuleContainer::keyPressed(const juce::KeyPress& key, juce::Component* ogComp)
{
    return false;
}

bool KrumModuleContainer::keyStateChanged(bool isKeyDown, juce::Component* ogComp)
{
    if (isKeyDown && juce::ModifierKeys::currentModifiers.isShiftDown())
    {
        setMultiControlState(true);
    }
    else if (!isKeyDown)
    {
        setMultiControlState(false);
    }
    
    return false;
}

//this gets called by the realtime thread, need to copy data and return. No painting.
void KrumModuleContainer::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    bool alreadySentMidi = false;
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getModuleMidiNote() == midiNoteNumber)
        {
            //sets the flag to paint the module
            modEd->setModulePlaying(true);
        }
        else if (modEd->doesEditorWantMidi())
        {
            nextMidiMessage.midiChannel = midiChannel;
            nextMidiMessage.midiNote = midiNoteNumber;
            applyMidi = true;
            return; //since we have the midi information and we know atleast one module wants the info, we can return here and the timercallback will distribute the midi to the modules that want it
        }
    }
}

void KrumModuleContainer::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getModuleMidiNote() == midiNoteNumber)
        {
            modEd->setModulePlaying(false);
        }
    }
}

KrumModuleEditor* KrumModuleContainer::handleNewFile(juce::ValueTree fileTree)
{
    auto file = juce::File{ fileTree.getProperty(TreeIDs::filePath.getParamID()).toString() };
    auto name = fileTree.getProperty(TreeIDs::fileName.getParamID()).toString();
    juce::int64 numSamples = 0;

    if (pluginEditor->sampler.isFileAcceptable(file, numSamples))
    {
        auto modTree = getFirstEmptyModuleTree();
        if (modTree.isValid())
        {
            //auto newMod = addNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()));
            auto newMod = insertNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()), 0);
            newMod->setModuleFile(file); //this turns the state to "hasfile" if necessary
            newMod->setNumSamplesOfFile(numSamples);
            newMod->timeHandle.setHandles(0, numSamples);
            newMod->setModuleName(name);

            addFileToRecentsFolder(file, name);

            return newMod;
        }
    }
    else
    {
        DBG("Folders Not Supported");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
    }

    return nullptr;
}

KrumModuleEditor* KrumModuleContainer::handleNewExternalFile(juce::String filePathName)
{
    auto file = juce::File{ filePathName };
    juce::int64 numSamples = 0;

    if (pluginEditor->sampler.isFileAcceptable(file, numSamples))
    {
        auto modTree = getFirstEmptyModuleTree();

        if (modTree.isValid())
        {
            //auto newMod = addNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()));
            auto newMod = insertNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()), 0); //could maybe make this index an on-screen position?
            newMod->setModuleFile(file); //this should turn the state to "hasfile" if necessary
            newMod->setNumSamplesOfFile(numSamples);

            newMod->setModuleName(filePathName);

            //this could be a global setting
            //if(auto add external drops to favorites
            addFileToFavoritesFolder(file, filePathName);
            addFileToRecentsFolder(file, filePathName);

            return newMod;
        }
    }
    else
    {
        DBG("External File Not Supported");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
    }

    return nullptr;

}



KrumModuleEditor* KrumModuleContainer::addNewModuleEditor(KrumModuleEditor* newModuleEditor)
{
     newModuleEditor->setModuleDisplayIndex(activeModuleEditors.size());
     addModuleEditor(newModuleEditor);
     return newModuleEditor;
}

KrumModuleEditor* KrumModuleContainer::insertNewModuleEditor(KrumModuleEditor* newModuleEditor, int indexToInsert)
{
    newModuleEditor->setModuleDisplayIndex(indexToInsert);
    
    //we are changing all of the editor's displayIndexes to reflect the new insertion. We push everything back
    for (int i = indexToInsert; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        int currentDisplayIndex = mod->getModuleDisplayIndex();
        mod->setModuleDisplayIndex(currentDisplayIndex + 1);
        
    }

    return addModuleEditor(newModuleEditor, indexToInsert);
  
}

void KrumModuleContainer::removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout)
{
    if (moduleToRemove)
    {
        updateModuleDisplayIndicesAfterDelete(moduleToRemove->getModuleDisplayIndex());
        activeModuleEditors.removeObject(moduleToRemove);

        if (refreshLayout)
        {
            refreshModuleLayout();
        }
    }
    else
    {
        juce::Logger::writeToLog("Tried To Remove NULL module editor");
    }
    
}

void KrumModuleContainer::setModuleSelectedFromClick(KrumModuleEditor* moduleToSelect, bool setSelected, const juce::MouseEvent& e)
{
    //I think I can get rid of the mouse event?
    bool shiftDown = e.mods.isShiftDown();
    bool cmdDown = e.mods.isCommandDown();
    bool multControl = isMultiControlActive();

    if (setSelected)
    {
        if (shiftDown)
        {
            setModulesSelectedToLastSelection(moduleToSelect);
        }
        else if (cmdDown)
        {
            setModuleSelectedState(moduleToSelect, true);
        }
        else
        {
            deselectAllModules();
            setModuleSelectedState(moduleToSelect, true);
        }

    }
    else
    {

        if (shiftDown)
        {
            deselectAllModules();
        }

        setModuleSelectedState(moduleToSelect, false);

    }

}

//intended to be an internal function, use setModuleSelectedFromClick() if possible
void KrumModuleContainer::setModuleSelectedState(KrumModuleEditor* moduleToSelect, bool shouldSelect)
{
    moduleToSelect->setModuleSelected(shouldSelect);

    if (shouldSelect)
    {
        currentlySelectedModules.add(moduleToSelect);
    }
    else
    {
        currentlySelectedModules.removeObject(moduleToSelect, false);
    }
}

void KrumModuleContainer::loadSelectedModules()
{
    if (currentlySelectedModules.size() == 0)
    {
        for (int i = 0; i < activeModuleEditors.size(); i++)
        {
            auto mod = activeModuleEditors[i];
            if (mod->isModuleSelected())
            {
                currentlySelectedModules.add(mod);
            }
        }
    }
}


void KrumModuleContainer::deselectAllModules()
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        setModuleSelectedState(activeModuleEditors[i], false);
    }
    
    //this is redundant, should be an assertion?
    currentlySelectedModules.clear(false);
}

//selects the modules between your last selected module to the newly selected module, useful for a shift click selection
void KrumModuleContainer::setModulesSelectedToLastSelection(KrumModuleEditor* moduleToSelect)
{
    if (currentlySelectedModules.size() < 1)
    {
        setModuleSelectedState(moduleToSelect, true);
        //if there is no "Last Selection", there is nothing to select up to
        return;
    }

    //get the last selected module's display index
    int lastSelectedDisplayIndex = currentlySelectedModules.getLast()->getModuleDisplayIndex();
   
    //get the current moduleToSelect's dispaly index
    int moduleToSelectDisplayIndex = moduleToSelect->getModuleDisplayIndex();


    if (lastSelectedDisplayIndex < moduleToSelectDisplayIndex) //we are selecting to the right
    {
        for (int i = 0; i < activeModuleEditors.size(); i++)
        {
            int displayIndex = activeModuleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want (if the index is between the selectedModule Indexes
            if (displayIndex == moduleToSelectDisplayIndex ||  
                (displayIndex > lastSelectedDisplayIndex && displayIndex < moduleToSelectDisplayIndex)) 
            {
                setModuleSelectedState(activeModuleEditors[i], true);
            }
        }
    }
    else if (lastSelectedDisplayIndex > moduleToSelectDisplayIndex) //we are selecting to the left
    {
        for (int i = 0; i < activeModuleEditors.size(); i++)
        {
            int displayIndex = activeModuleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want
            if (displayIndex == moduleToSelectDisplayIndex || 
                (displayIndex < lastSelectedDisplayIndex && displayIndex > moduleToSelectDisplayIndex))
            {
                setModuleSelectedState(activeModuleEditors[i], true);
            }
        }
    }
    else
    {
        setModuleSelectedState(moduleToSelect, true);
    }

}

void KrumModuleContainer::setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect)
{
    //could add more functationality here
    setModuleSelectedState(moduleToSelect, true);
}

void KrumModuleContainer::setSelectedModulesState(KrumModuleEditor* eventOrigin, int newState)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != eventOrigin)
        {
            mod->setModuleState(newState);
        }
    }
}

void KrumModuleContainer::setSelectedModulesColor(KrumModuleEditor* eventOrigin, juce::Colour newColor)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != eventOrigin)
        {
            mod->setModuleColor(newColor);
        }
    }
}

bool KrumModuleContainer::isModuleSelected(KrumModuleEditor* moduleToCheck)
{

    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        if (moduleToCheck == currentlySelectedModules[i])
        {
            return true;
        }
    }

    return false;
}

bool KrumModuleContainer::multipleModulesSelected()
{
    return currentlySelectedModules.size() > 1;
}

void KrumModuleContainer::setSettingsOverlayOnSelectedModules(bool show, KrumModuleEditor* eventOrigin)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != eventOrigin)
        {
            if (show)
            {
                mod->hideSettingsOverlay();
            }
            else
            {
                mod->showSettingsOverlay();
            }
        }
    }
}

void KrumModuleContainer::clickOneShotOnSelectedModules(const juce::MouseEvent& event, KrumModuleEditor* eventOrigin, bool mouseDown)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != eventOrigin)
        {
            if (mouseDown)
            {
                mod->handleOneShotButtonMouseDown(event.getEventRelativeTo(mod));
            }
            else
            {
                mod->handleOneShotButtonMouseUp(event.getEventRelativeTo(mod));
            }
        }
    }
}




void KrumModuleContainer::setListeningForMidiOnSelectedModules(bool shouldListen, KrumModuleEditor* originComp)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != originComp)
        {
            mod->setModuleListeningForMidi(shouldListen);
        }
    }

}

juce::Array<KrumModuleEditor*> KrumModuleContainer::getModulesFromMidiNote(int midiNote)
{
    juce::Array<KrumModuleEditor*>retArray{};

    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->getModuleMidiNote() == midiNote)
        {
            retArray.add(modEd);
            //return modEd;
        }
    }
    
    //return nullptr;
    return retArray;
}


KrumSamplerAudioProcessorEditor* KrumModuleContainer::getPluginEditor()
{
    return pluginEditor;
}

juce::OwnedArray<KrumModuleEditor>& KrumModuleContainer::getActiveModuleEditors()
{
    return activeModuleEditors;
}


int KrumModuleContainer::getNumActiveModules()
{
    int count = 0;
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for(int i = 0; i < activeModuleEditors.size(); i++)
    {
        if((int)modulesTree.getChild(i).getProperty(TreeIDs::moduleState.getParamID()) == KrumModule::ModuleState::active)
        {
            ++count;
        }
    }
    return count;
    
}

int KrumModuleContainer::getNumEmptyModules()
{
    int count = 0;
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        if ((int)modulesTree.getChild(i).getProperty(TreeIDs::moduleState.getParamID()) == KrumModule::ModuleState::empty)
        {
            count++;
        }
    }
    return count;
}

KrumModuleEditor* KrumModuleContainer::getActiveModuleEditor(int index)
{
    return activeModuleEditors.getUnchecked(index);
}

KrumModuleEditor* KrumModuleContainer::getEditorFromDisplayIndex(int displayIndex)
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->getModuleDisplayIndex() == displayIndex)
        {
            return modEd;
        }
    }

    return nullptr;
}

void KrumModuleContainer::showModuleClipGainSlider(KrumModuleEditor* moduleEditor)
{
    //this loop clears any shown clipGain sliders, this is why the container handles this and not the module itself
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors.getUnchecked(i);
        if (modEd != nullptr && modEd != moduleEditor)
        {
            modEd->setClipGainSliderVisibility(false);
        }
    }

    moduleEditor->setClipGainSliderVisibility(true);
}

//void KrumModuleContainer::showModulePitchSlider(KrumModuleEditor* moduleEditor)
//{
//    //clears all pitchsliders
//    for (int i = 0; i < activeModuleEditors.size(); i++)
//    {
//        auto modEd = activeModuleEditors.getUnchecked(i);
//        if (modEd != nullptr && modEd != moduleEditor)
//        {
//            modEd->setPitchSliderVisibility(false);
//        }
//    }
//
//    moduleEditor->setPitchSliderVisibility(true);
//}

void KrumModuleContainer::showModuleCanAcceptFile(KrumModuleEditor* moduleEditor)
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->canThumbnailAcceptFile() && modEd != moduleEditor)
        {
            modEd->setThumbnailCanAcceptFile(false);
        }
    }

    moduleEditor->setThumbnailCanAcceptFile(true);
}

void KrumModuleContainer::hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor)
{
    moduleEditor->setThumbnailCanAcceptFile(false);
}

void KrumModuleContainer::timerCallback()
{

    bool mouseOver = false;
    for (int i = 0; i < activeModuleEditors.size(); ++i)
    {
        auto modEd = activeModuleEditors[i];
        if (modEd->getMouseOver())
        {
            mouseOver = true;
            break;
        }
    }

    //this avoids keys on the keyboard staying highlighted after the mouse has left any module
    if (pluginEditor && (!mouseOver))
    {
        pluginEditor->keyboard.clearHighlightedKey();
    }


    if (applyMidi)
    {
        loadMidiToModulesListening(nextMidiMessage.midiChannel, nextMidiMessage.midiNote);
        applyMidi = false;
    }

    if (moduleBeingDragged)
    {

    }

    //TODO: review
    //not sure I like this
    if (currentlySelectedModules.size() > 0 && pluginEditor->fileBrowser.isMouseButtonDown(true))
    {
        deselectAllModules();
    }

    repaint();
}

void KrumModuleContainer::loadMidiToModulesListening(int channel, int note)
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        if (mod->doesEditorWantMidi())
        {
            mod->handleMidi(channel, note);
        }
    }
}

void KrumModuleContainer::addFileToRecentsFolder(juce::File& file, juce::String name)
{
    pluginEditor->fileBrowser.addFileToRecent(file, name);
}

void KrumModuleContainer::addFileToFavoritesFolder(juce::File& file, juce::String name)
{
    pluginEditor->fileBrowser.addFileToFavorites(file);
}

KrumModuleEditor* KrumModuleContainer::addModuleEditor(KrumModuleEditor* newModuleEditor, int indexToInsert, bool refreshLayout)
{
    if (newModuleEditor != nullptr)
    {
        addAndMakeVisible(newModuleEditor);

        if (indexToInsert > -1)
        {
            activeModuleEditors.insert(indexToInsert, newModuleEditor);
        }
        else
        {
            activeModuleEditors.add(newModuleEditor);
        }

        if (refreshLayout) //defaults true
        {
            refreshModuleLayout();
        }

        repaint();

        return newModuleEditor;
    }

    DBG("New Editor is NULL");
    return nullptr;
}


void KrumModuleContainer::createModuleEditors()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());

    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        int state = (int)moduleTree.getProperty(TreeIDs::moduleState.getParamID());
        if (moduleTree.isValid() && ( state > 0)) //reference KrumModule::ModuleState, 0 is empty module
        {
            auto modEd = addModuleEditor(new KrumModuleEditor(moduleTree, *this, getPluginEditor()->getAudioFormatManager()));
        }
    }

    loadSelectedModules();

}

bool KrumModuleContainer::isMultiControlActive()
{
    return multiSelectControlState && multipleModulesSelected();
}

void KrumModuleContainer::repaintAllSelectedModules()
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        currentlySelectedModules[i]->repaint();
    }
}

bool KrumModuleContainer::findChildCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        if (compToTest->getParentComponent() == currentlySelectedModules[i])
        {
            origin = i;
            type = compToTest->getName();
            return true;
        }
    }

    return false;
}

void KrumModuleContainer::updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted)
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        int currentDisplayIndex = (int)moduleTree.getProperty(TreeIDs::moduleDisplayIndex.getParamID());

        if (currentDisplayIndex > displayIndexDeleted)
        {
            moduleTree.setProperty(TreeIDs::moduleDisplayIndex.getParamID(), currentDisplayIndex - 1, nullptr);
        }
        else if (currentDisplayIndex == displayIndexDeleted)
        {
            moduleTree.setProperty(TreeIDs::moduleDisplayIndex.getParamID(), -1, nullptr);
        }
    }
}

int KrumModuleContainer::getNumVisibleModules()
{
    int numVisible = 0;
    for(int i = 0; i < activeModuleEditors.size(); i++)
    {
        if(activeModuleEditors[i]->isVisible())
        {
            numVisible++;
        }
    }
    return numVisible;
    
}

juce::AudioProcessorValueTreeState* KrumModuleContainer::getAPVTS()
{
    return pluginEditor->getParameters();
}

void KrumModuleContainer::setMultiControlState(bool shouldControl)
{
    if (shouldControl != multiSelectControlState)
    {
        multiSelectControlState = shouldControl;

        if (shouldControl)
        {
            //addParamListeners();
        }
        else
        {
            //removeParamListeners();

        }

        repaintAllSelectedModules();
    }

}

//if sourceEditor is null, it will reset the selected Modules Attachments to it's own parameters
void KrumModuleContainer::reassignSelectedSliderAttachments(KrumModuleEditor* sourceEditor, juce::Slider* slider)
{
    auto& apvts = *getAPVTS();

    if (sourceEditor == nullptr)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->resetSliderAttachments();
            }
        }
    }
    else
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->reassignSliderAttachment(slider, true);
            }
        }
    }

}


void KrumModuleContainer::updateSlidersIfBeingMultiControlled(KrumModuleEditor* editor, juce::Slider* slider)
{
    if (isMultiControlActive() && editor->isModuleSelected())
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (editor != mod)
            {
                mod->updateSliderFromMultiControl(slider);
            }
        }
    }
}

void KrumModuleContainer::reassignSelectedButtonAttachments(KrumModuleEditor* sourceEditor, juce::Button* button)
{
    auto& apvts = *getAPVTS();

    if (sourceEditor == nullptr)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->resetButtonAttachments();
            }
        }
    }
    else
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->reassignButtonAttachment(button, false);
            }
        }
    }
}

void KrumModuleContainer::reassignSelectedComboAttachments(KrumModuleEditor* sourceEditor, juce::ComboBox* comboBox)
{
    auto& apvts = *getAPVTS();

    if (sourceEditor == nullptr)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->resetComboAttachments();
            }
        }
    }
    else
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->reassignComboAttachment(comboBox, false);
            }
        }
    }
}

void KrumModuleContainer::updateComboIfBeingMultiControlled(KrumModuleEditor* sourceEditor, juce::ComboBox* comboBox)
{
    if (isMultiControlActive() && sourceEditor->isModuleSelected())
    {

        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            auto mod = currentlySelectedModules[i];
            if (mod != sourceEditor)
            {
                mod->updateComboFromMultiControl(comboBox);
            }
        }
    }
}

juce::ModifierKeys KrumModuleContainer::getMultiControlModifierKey()
{
    return multiControlModifierKey;
}

void KrumModuleContainer::setMultiControlModifierKey(juce::ModifierKeys::Flags newModKeyFlag)
{
    if (newModKeyFlag == 0)
    {
        return;
    }
    else
    {
        multiControlModifierKey = newModKeyFlag;
    }
}



juce::ValueTree& KrumModuleContainer::getFirstEmptyModuleTree()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        if ((int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()) == KrumModule::ModuleState::empty)
        {
            return moduleTree;
        }
    }

    return juce::ValueTree{};

}

void KrumModuleContainer::clearSelectedSettingsOverlays()
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        currentlySelectedModules[i]->hideSettingsOverlay();
    }
}

void KrumModuleContainer::clearActiveModuleSettingsOverlays()
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        activeModuleEditors[i]->hideSettingsOverlay();
    }
}

void KrumModuleContainer::startDraggingEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event)
{
    if (editorToDrag->isModuleSelected() && isMultiControlActive())
    {
        //startDraggingSelectedComponents(editorToDrag, event);
    }
    else
    {
        dragger.startDraggingComponent(editorToDrag, event);
        editorBeingDragged = editorToDrag;
    }
}

void KrumModuleContainer::dragEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event)
{
    moduleBeingDragged = true;
    //this needs to handle moving the modules underneath this one based off of the mouse position
    
    //conastrain the events' y value.
    auto newEvent = event.withNewPosition(juce::Point<int>(event.position.getX(), event.getMouseDownPosition().getY()));
    dragger.dragComponent(editorToDrag, newEvent, moduleEditorConstrainer);
    
   
    auto viewport = pluginEditor->getModuleViewport();
    auto mousePos = newEvent.getEventRelativeTo(viewport).getPosition();
    int borderThickness = 50;
    int scrollAmount = 20;
    int repeatMillis = 30;

    if (viewport->autoScroll(mousePos.getX(), mousePos.getY(), borderThickness, scrollAmount))
    {
        beginDragAutoRepeat(repeatMillis);
        //DBG("mouseX = " + juce::String(mousePos.getX()));
    }


    //if mousePosition is over a component, which side of the comp is it on? You can change this range to fine tune dragging feel

    auto draggedArea = getModulesDraggedArea();
    DBG("Dragged Area: " + draggedArea.toString());

    //TODO: write this function to be accurate to it's name. Right now it just returns the module that the mouse is over
    //auto mod = isMouseDragAreaOverActiveEditor(editorToDrag, draggedArea);
    KrumModuleEditor* modOver = nullptr;

    //Temp, should be in it's own fucntion I think?
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        if (mod != editorToDrag && mod->contains(mod->getMouseXYRelative().withY(newEvent.y)))
        {
            modOver = mod;
        }
    }


    if (modOver != nullptr)
    {
        DBG("Non-Null --");
        if (editorToDrag->isModuleSelected() && isMultiControlActive())
        {
            //swap indexs while moving all the selected modules
            //we might need to base the mouse hittest off of the combined area of the selected modules, instead of the mouse event
            
        }
        else
        {
            int displayIndex = modOver->getModuleDisplayIndex();
            int endPadding = modOver->getWidth() * 0.3f;
            auto mouseXY = modOver->getMouseXYRelative();

            if (mouseXY.getX() < endPadding)
            {
                swapModuleEditorsDisplayIndex(displayIndex - 1, displayIndex, false);
            }
            else if(mouseXY.getX() > modOver->getWidth() - endPadding)
            {
                swapModuleEditorsDisplayIndex(displayIndex, displayIndex + 1, true);
            }
        }
    }
    else if(!contains(getMouseXYRelative()))
    {
        //need to scroll the moduleViewport
        // account for when the mouse is either negative or positive coordinates and scroll accordingly
        // mouse should stay held down and it will not stop scrolling until the end
        // 
        
        //pluginEditor->getModuleViewport()->setViewPosition(pluginEditor->getMouseXYRelative());
        


        
        DBG("NULL-----");
        //this means the mouse is in between components, maybe draw something?
    }
}

//swaps modules display indexes. Forward decides which index gets changed first
void KrumModuleContainer::swapModuleEditorsDisplayIndex(int firstIndex, int secondIndex, bool forward)
{

    auto firstMod = getEditorFromDisplayIndex(firstIndex);
    auto secondMod = getEditorFromDisplayIndex(secondIndex);

    if (firstMod == nullptr || secondMod == nullptr)
        return;

    //not sure if this logic is working. 
    //I'm trying to make sure they get swapped correctly depending on which direction you are dragging. 
    //It sometimes changes an the wrong module. 
    if (forward)
    {
        secondMod->setModuleDisplayIndex(firstIndex, false);
        firstMod->setModuleDisplayIndex(secondIndex, false);

    }
    else
    {
        firstMod->setModuleDisplayIndex(secondIndex, false);
        secondMod->setModuleDisplayIndex(firstIndex, false);
    }

}

KrumModuleEditor* KrumModuleContainer::isMouseDragAreaOverActiveEditor(KrumModuleEditor* eventOrigin, juce::Rectangle<int>& draggedArea)
{
    //TODO: 
    //figure out the interaction you want. It currently is not doing what the function says it does
    //lets try to find out if the dragged area is over 2 modules, should we return index instead? 

    
    /*for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        if (!mod->isModuleSelected() && draggedArea.contains(mod->getBoundsInParent()))
        {
            return mod;
        }
    }*/


    //OLD
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        if (mod != eventOrigin && mod->contains(mod->getMouseXYRelative())/*!mod->isModuleSelected() && draggedArea.contains(mod->getBounds()*/)
        {
            return mod;
        }
    }

    return nullptr;
}

//TODO: get this function working.
juce::Rectangle<int> KrumModuleContainer::getModulesDraggedArea()
{
    juce::Rectangle<int> retRect{};

    if (moduleBeingDragged && editorBeingDragged != nullptr)
    {
        //if (isMultiControlActive())
        //{
            //retRect.setWidth(currentlySelectedModules.size() * EditorDimensions::moduleW);

            for (int i = 0; i < currentlySelectedModules.size(); i++)
            {
                auto mod = currentlySelectedModules[i];
                auto bounds = mod->getBoundsInParent();
                if (retRect.isEmpty())
                {
                    retRect.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
                }
                else
                {
                    retRect.setX(bounds.getX());
                    retRect.setWidth(retRect.getWidth() + bounds.getWidth());
                    
                    /*if (bounds.getX() < retRect.getX())
                    {
                        retRect.setX(bounds.getX());
                    }

                    if (bounds.getRight() > retRect.getRight())
                    {
                        retRect.setRight(bounds.getRight());
                    }*/

                }
            }
        /*}
        else
        {
            retRect = editorBeingDragged->getBoundsInParent();
        }*/
    }

    return retRect;
}

bool KrumModuleContainer::isModuleDragging()
{
    return moduleBeingDragged;
}

void KrumModuleContainer::draggingMouseUp(const juce::MouseEvent& event)
{
    if (moduleBeingDragged)
    {
        moduleBeingDragged = false;
        refreshModuleLayout();
    }

    if (editorBeingDragged)
    {
        editorBeingDragged = nullptr;
    }
}

//
//void KrumModuleContainer::focusLost(juce::Component::FocusChangeType cause)
//{
//    if(cause == juce::Component::focusChangedByMouseClick)
//        deselectAllModules();
//}

//===================================================================================================================================
//===================================================================================================================================