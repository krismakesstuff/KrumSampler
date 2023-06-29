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
    //setInterceptsMouseClicks(true, true);
    //setRepaintsOnMouseActivity(true);
    setWantsKeyboardFocus(true);

    pluginEditor->addKeyboardListener(this);
    pluginEditor->addKeyListener(this);
    valueTree.addListener(this);


    //moduleEditorConstrainer->setBoundsForComponent()

    refreshModuleLayout();
    startTimerHz(30);
        
}

KrumModuleContainer::~KrumModuleContainer()
{
    currentEditorDragging = nullptr;
    pluginEditor->removeKeyboardListener(this);
    pluginEditor->removeKeyListener(this);
    valueTree.removeListener(this);
    currentlySelectedModules.clear(false);
}

void KrumModuleContainer::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
   // g.setColour(bgColor.withAlpha(0.3f));
   // g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);

}

void KrumModuleContainer::paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition)
{
    juce::Rectangle<int> line{ mousePosition.withY(0), mousePosition.withY(getLocalBounds().getBottom()) };

    g.setColour(juce::Colours::white);
    g.fillRect(line);
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


    for (int i = 0; i < numModules; i++)
    {
        auto modEd = activeModuleEditors[i];
        modEd->setTopLeftPosition(((modEd->getModuleDisplayIndex() * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage()), EditorDimensions::shrinkage); //set position based off of stored index
        DBG("Module Editor " + juce::String(modEd->getModuleDisplayIndex()) +" set position to: " + juce::String(modEd->getBoundsInParent().toString()));
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
        if (currentEditorDragging && currentEditorDragging != getEditorFromDisplayIndex(treeWhoChanged.getProperty(TreeIDs::moduleDisplayIndex.getParamID())))
        {
            refreshModuleLayout();
        }
        else if(!isModuleDragging())
        {
            refreshModuleLayout();
        }
    }
    

    //we don't want selection changes, but we need to verify a module is selected before changing the other properties

    //when a tree changes a property that is part of a multi-select action, we apply the same changes to the other selected Trees and modules. This will only handle moduleTree properties, real-time audio properties will be handled sepearately
    /*if (currentlySelectedModules.size() > 1 && property == juce::Identifier(TreeIDs::moduleSelected.getParamID()) && (float)treeWhoChanged.getProperty(property) > 0.5f)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            if ((int)treeWhoChanged.getProperty(TreeIDs::moduleSamplerIndex.getParamID()) != currentlySelectedModules[i]->getModuleSamplerIndex())
            {
                currentlySelectedModules[i]->valueTreePropertyChanged(treeWhoChanged, property);
            }
        }
    }*/

}

void KrumModuleContainer::parameterChanged(const juce::String& parameterID, float newValue)
{
    //this function gets called for every parameter change on the APVTS, so we need to filter.
    //if we are actually interested in control other modules && are we changing a parameter that is from our moduleEditor
    if (isMultiControlActive() && doesParamIDsContain(parameterID))
    {
        //set the data to be applied to the other selected Modules
        nextParamChange.paramID = parameterID;
        nextParamChange.value = newValue;

        //this tells the timer callback to get the data from nextParaChange, which we set above. see timerCallback()
        applyNextParamChange = true;

        //DBG("PARAMID CONTAINS: ParameterID = " + parameterID + ". With value = " + juce::String(newValue));
    }
}

//
void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    //capture a mouseclick that isn't on any modules, that will clear all selected modules
    deselectAllModules();
    clearActiveModuleSettingsOverlays();
    


}
//
bool KrumModuleContainer::keyPressed(const juce::KeyPress& key, juce::Component* ogComp)
{


    return false;
}
//
bool KrumModuleContainer::keyStateChanged(bool isKeyDown, juce::Component* ogComp)
{
    //if (multiControlModifierKey.isCurrentlyDown())
    if (isKeyDown && juce::ModifierKeys::currentModifiers.isShiftDown())
    {
        //DBG("key DOWN from " + ogComp->getName());
        setMultiControlState(true);
    }
    else if (!isKeyDown)
    {
        //DBG("key UP from " + ogComp->getName());

        //DBG("keyStateChanged mcmk is up");
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

    //repaint();
}

//intended to be an internal function, use setModuleSelectedFromClick() if possible
void KrumModuleContainer::setModuleSelectedState(KrumModuleEditor* moduleToSelect, bool shouldSelect)
{
    moduleToSelect->setModuleSelected(shouldSelect);

    if (shouldSelect)
    {
        currentlySelectedModules.add(moduleToSelect);
        addModuleParamIDs(moduleToSelect);
    }
    else
    {
        currentlySelectedModules.removeObject(moduleToSelect, false);
        removeModuleParamIDs(moduleToSelect);
    }

}

void KrumModuleContainer::deselectAllModules()
{
    
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        setModuleSelectedState(activeModuleEditors[i], false);
    }
    
    //this is redundant, shoul be an assertion?
    currentlySelectedModules.clear(false);
    paramIDs.clear();
    DBG("ParamIDs cleared, size: " + paramIDs.size());
}

//selects the modules between your last selected module to the newly selected module
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

    //we are selecting to the right
    if (lastSelectedDisplayIndex < moduleToSelectDisplayIndex)
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

    //this assumes the shift key was down in order for this function to get called
    addParamListeners();

}

void KrumModuleContainer::setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect)
{
    //could add more functationality here
    setModuleSelectedState(moduleToSelect, true);
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

void KrumModuleContainer::applyValueTreeChangesToSelectedModules(juce::ValueTree& treeWhoChanged, const juce::Identifier& propertyWhoChanged)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (!mod->isModuleTree(treeWhoChanged)) //we don't want to send a change message to a module that triggered the message in the first place
        {
            mod->valueTreePropertyChanged(treeWhoChanged, propertyWhoChanged);
        }
    }
}

void KrumModuleContainer::applyParameterChangeToSelectedModules(const juce::String& parameterID, float newValue)
{

    auto apvts = pluginEditor->getParameters();
    juce::String moduleSamplerIndex{};

    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        int sampIndex = mod->getModuleSamplerIndex();
        if (sampIndex < 10)
        {
            moduleSamplerIndex = "0" + juce::String(sampIndex);
        }
        else
        {
            moduleSamplerIndex = juce::String(sampIndex);
        }
        
        //we don't want to change the same parameter that triggered this change
        if (parameterID.getLastCharacters(2) != moduleSamplerIndex)
        {
            //rewrite the parameterID string to reflect the module we actually want to change
            const juce::String newParamString = parameterID.dropLastCharacters(moduleSamplerIndex.length()) + moduleSamplerIndex;
            
            //set the new parameterID with the newValue
            //apvts->state.setProperty({ newParamString }, newValue, nullptr);
            //apvts->getParameter(newParamString)->beginChangeGesture();
            apvts->getParameter(newParamString)->setValueNotifyingHost(newValue);
            //apvts->getParameter(newParamString)->endChangeGesture();

            DBG("newParamString = " + newParamString);
            DBG("setProperty with: " + juce::String(newValue));
            
        }
    }

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

void KrumModuleContainer::showModulePitchSlider(KrumModuleEditor* moduleEditor)
{
    //clears all pitchsliders
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto modEd = activeModuleEditors.getUnchecked(i);
        if (modEd != nullptr && modEd != moduleEditor)
        {
            modEd->setPitchSliderVisibility(false);
        }
    }

    moduleEditor->setPitchSliderVisibility(true);
}

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

    //for multi-control selection 
    if (applyNextParamChange)
    {
        
        applyParameterChangeToSelectedModules(nextParamChange.paramID, nextParamChange.value);
        applyNextParamChange = false;
    }

    if (applyMidi)
    {
        loadMidiToModulesListening(nextMidiMessage.midiChannel, nextMidiMessage.midiNote);
        applyMidi = false;
    }

    if (moduleDragging)
    {

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
            addParamListeners();
        }
        else
        {
            removeParamListeners();
        }

        repaintAllSelectedModules();
    }

}

void KrumModuleContainer::addParamListeners()
{
    for (int i = 0; i < paramIDs.size(); i++)
    {
        pluginEditor->getParameters()->addParameterListener(paramIDs[i], this);
    }
    DBG("ParamListenersAdded");
}

void KrumModuleContainer::removeParamListeners()
{
    for (int i = 0; i < paramIDs.size(); i++)
    {
        pluginEditor->getParameters()->removeParameterListener(paramIDs[i], this);
    }
    DBG("ParamListenersRemoved");

}

void KrumModuleContainer::addModuleParamIDs(KrumModuleEditor* module)
{
    if (module)
    {
        //juce::String index = juce::String(module->getModuleSamplerIndex());
        juce::String index = module->getSamplerIndexString();

        paramIDs.add(TreeIDs::paramModuleGain.getParamID() + index);
        paramIDs.add(TreeIDs::paramModuleClipGain.getParamID() + index);
        paramIDs.add(TreeIDs::paramModulePan.getParamID() + index);
        paramIDs.add(TreeIDs::paramModuleOutputChannel.getParamID() + index);
        paramIDs.add(TreeIDs::paramModulePitchShift.getParamID() + index);
        paramIDs.add(TreeIDs::paramModuleReverse.getParamID() + index);
        paramIDs.add(TreeIDs::paramModuleMute.getParamID() + index);
        
        DBG("ParamIDs loaded from module at SamplerIndex: " + index);
    }


    /*for (int i = 0; i < paramIDs.size(); i++)
    {
        DBG("paramID[" + juce::String(i) + "] = " + paramIDs[i]);
    }*/

}

void KrumModuleContainer::removeModuleParamIDs(KrumModuleEditor* module)
{
    if (module)
    {
        juce::String index = module->getSamplerIndexString();
        //juce::String index = juce::String(module->getModuleSamplerIndex());

        paramIDs.removeString(TreeIDs::paramModuleGain.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModuleClipGain.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModulePan.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModuleOutputChannel.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModulePitchShift.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModuleReverse.getParamID() + index);
        paramIDs.removeString(TreeIDs::paramModuleMute.getParamID() + index);

        DBG("ParamIDs removed for module at SamplerIndex: " + index);

    }


    /*for (int i = 0; i < paramIDs.size(); i++)
    {
        DBG("paramID[" + juce::String(i) + "] = " + paramIDs[i]);
    }*/

}

bool KrumModuleContainer::doesParamIDsContain(const juce::String& paramIDToTest)
{
    for (int i = 0; i < paramIDs.size(); i++)
    {
        if (paramIDs[i] == paramIDToTest)
        {
            return true;
        }
    }

    return false;

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

bool KrumModuleContainer::handleNewFile(juce::ValueTree fileTree)
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
            auto newMod = insertNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()), 0); //could maybe make this index an on-screen position?
            newMod->setModuleFile(file); //this should turn the state to "hasfile" if necessary
            newMod->setNumSamplesOfFile(numSamples);

            newMod->setModuleName(name);
            addFileToRecentsFolder(file, name); 

            return true;
        }
    }
    else
    {
        DBG("Folders Not Supported");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
    }

    return false;
}

bool KrumModuleContainer::handleNewExternalFile(juce::String filePathName)
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

            return true;
        }
    }
    else
    {
        DBG("External File Not Supported");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
    }

    return false;

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
        currentEditorDragging = editorToDrag;
    }

    
}

void KrumModuleContainer::dragEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event)
{
    moduleDragging = true;
    //this needs to handle moving the modules around underneath this one based off of the mouse position
    

    //conastrain the events y value.
    auto newEvent = event.withNewPosition(juce::Point<int>(event.position.getX(), event.getMouseDownPosition().getY()));
    dragger.dragComponent(editorToDrag, newEvent, moduleEditorConstrainer);
    
    //if mousePosition is over a component, which side of the comp is it on? You can change this range to fine tune dragging feel
    auto mod = isMouseDragOverActiveEditor(editorToDrag, event.getEventRelativeTo(this));
    if (mod != nullptr)
    {
        if (editorToDrag->isModuleSelected() && isMultiControlActive())
        {
            //swap indexs while moving all the selected modules
            //we might need to base the mouse hittest off of the combined area of the selected modules, instead of the mouse event
            
        }
        else
        {

            auto e = event.getEventRelativeTo(mod);
            int displayIndex = mod->getModuleDisplayIndex();

            if (mod->getMouseXYRelative().getX() < (mod->getWidth() / 2))
            {
                swapModuleEditorsDisplayIndex(displayIndex -1, displayIndex);
            }
            else
            {
                swapModuleEditorsDisplayIndex(displayIndex, displayIndex + 1);
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
        
        
        //DBG("NULL-----");
        //this means the mouse is in between components, maybe draw something?
    }
}

KrumModuleEditor* KrumModuleContainer::isMouseDragOverActiveEditor(KrumModuleEditor* eventOrigin, const juce::MouseEvent& event)
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        auto mod = activeModuleEditors[i];
        if (mod != eventOrigin && mod->contains(mod->getMouseXYRelative()))
        {
            return mod;
        }
    }

    return nullptr;
}


void KrumModuleContainer::swapModuleEditorsDisplayIndex(int firstIndex, int secondIndex)
{

    auto firstMod = getEditorFromDisplayIndex(firstIndex);
    auto secondMod = getEditorFromDisplayIndex(secondIndex);


    firstMod->setModuleDisplayIndex(secondIndex);
    secondMod->setModuleDisplayIndex(firstIndex);


}

bool KrumModuleContainer::isModuleDragging()
{
    return moduleDragging;
}

void KrumModuleContainer::draggingMouseUp(const juce::MouseEvent& event)
{
    if (moduleDragging)
    {
        moduleDragging = false;
        refreshModuleLayout();
    }

    if (currentEditorDragging)
    {
        currentEditorDragging = nullptr;
    }
}

//===================================================================================================================================
//===================================================================================================================================



