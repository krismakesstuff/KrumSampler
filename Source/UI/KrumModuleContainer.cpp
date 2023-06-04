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

    addAndMakeVisible(dropSampleArea);

    refreshModuleLayout();
    startTimerHz(30);
        
}

KrumModuleContainer::~KrumModuleContainer()
{
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

    dropSampleArea.setBounds(EditorDimensions::extraShrinkage(), EditorDimensions::extraShrinkage(), EditorDimensions::dropSampleAreaW, EditorDimensions::moduleH - EditorDimensions::extraShrinkage());


    for (int i = 0; i < numModules; i++)
    {
        auto modEd = activeModuleEditors[i];
        modEd->setTopLeftPosition(((modEd->getModuleDisplayIndex() * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage()) + dropSampleArea.getWidth() + EditorDimensions::shrinkage, EditorDimensions::shrinkage); //set position based off of stored index
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
        refreshModuleLayout();
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
    //see if we are actually interested in control other modules
    if (multipleModulesSelected() && isMultiControlActive())
    {
        //make sure we changin a parameter that is from our moduleEditor
        if (doesParamIDsContain(parameterID))
        {
            //set the data to be applied to the other selected Modules
            nextParamChange.paramID = parameterID;
            nextParamChange.value = newValue;

            //this tells the timer callback to get the data from nextParaChange, which we set above. see timerCallback()
            applyNextParamChange = true;

            //DBG("PARAMID CONTAINS: ParameterID = " + parameterID + ". With value = " + juce::String(newValue));
        }
    }
}
//
//void KrumModuleContainer::sliderValueChanged(juce::Slider* slider)
//{
//    int sliderOrigin = -1;
//    juce::String sliderType{};
//
//    findChildCompInSelectedModules(slider, sliderOrigin, sliderType);
//
//    DBG("SliderType = " + sliderType);
//
//    if (sliderOrigin >= 0)
//    {
//        for (int i = 0; i < currentlySelectedModules.size(); i++)
//        {
//            if (i != sliderOrigin)
//            {
//                if (sliderType == "Module Gain")
//                {
//                    currentlySelectedModules[i]->volumeSlider.setValue(slider->getValue());
//                }
//                else if (sliderType == "Pitch")
//                {
//                    currentlySelectedModules[i]->pitchSlider.setValue(slider->getValue());
//                }
//                else if (sliderType == "Module Pan")
//                {
//                    currentlySelectedModules[i]->panSlider.setValue(slider->getValue());
//                }
//            }
//        }
//    }
//
//}
//
//void KrumModuleContainer::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
//{
//    int comboOrigin = -1;
//    juce::String comboType{};
//
//    findChildCompInSelectedModules(comboBoxThatHasChanged, comboOrigin, comboType);
//
//    DBG("comboType = " + comboType);
//
//    if (comboOrigin >= 0)
//    {
//        for (int i = 0; i < currentlySelectedModules.size(); i++)
//        {
//            if (i != comboOrigin)
//            {
//                if (comboType == "Output Channel")
//                {
//                    currentlySelectedModules[i]->outputCombo.setSelectedId(comboBoxThatHasChanged->getSelectedId());
//                }
//            }
//        }
//    }
//}
//
//void KrumModuleContainer::buttonClicked(juce::Button* buttonClicked)
//{
//    int buttonOrigin = -1;
//    juce::String buttonType{};
//
//    findChildCompInSelectedModules(buttonClicked, buttonOrigin, buttonType);
//
//    DBG("buttonType =" + buttonType);
//
//    if (buttonOrigin >= 0)
//    {
//        for (int i = 0; i < currentlySelectedModules.size(); i++)
//        {
//            if (i != buttonOrigin)
//            {
//
//                if (buttonType == "Reverse Button")
//                {
//                    currentlySelectedModules[i]->reverseButton.setToggleState(buttonClicked->getToggleState(),juce::sendNotificationSync);
//
//                }
//                else if (buttonType == "Mute")
//                {
//                    currentlySelectedModules[i]->muteButton.setToggleState(buttonClicked->getToggleState(), juce::sendNotificationSync);
//
//                }
//                else if (buttonType == "Settings")
//                {
//                    //show module's settings overlay. This will change
//                    //currentlySelectedModules[i]->editButton.triggerClick();
//                }
//                
//            }
//        }
//    }
//
//}
//
//void KrumModuleContainer::buttonStateChanged(juce::Button* buttonChanged)
//{
//
//    if (buttonChanged->getState() == juce::Button::buttonDown)
//    {
//        int buttonOrigin = -1;
//        juce::String buttonType{};
//
//        findChildCompInSelectedModules(buttonChanged, buttonOrigin, buttonType);
//
//       /* if (buttonOrigin >= 0)
//        {
//            for (int i = 0; i < currentlySelectedModules.size(); i++)
//            {
//                if (i != buttonOrigin)
//                {
//                    if (buttonType == "One Shot")
//                    {
//                        auto note = currentlySelectedModules[i]->getModuleMidiNote();
//                        auto chan = currentlySelectedModules[i]->getModuleMidiChannel();
//                        auto vel = currentlySelectedModules[i]->buttonClickVelocity;
//
//                        editor->sampler.noteOn(chan, note, vel);
//                    }
//                }
//            }
//        }*/
//
//    }
//}
//
void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    //TODO:
    //capture a mouseclick that isn't on any modules, that will clear all selected modules
    deselectAllModules();
    clearActiveModuleSettingsOverlays();
    
    //auto mousePos = event.getMouseDownPosition();
    

    //for (int i = 0; i < moduleEditors.size(); i++)
    //{
    //    auto modEd = moduleEditors[i];
    //    auto modBounds = modEd->getBoundsInParent();

    //    if (modBounds.contains(mousePos))
    //    {
    //        setModuleSelected(modEd);
    //    }
    //    else
    //    {
    //        setModuleUnselected(modEd);
    //    }
    //}

}
//
bool KrumModuleContainer::keyPressed(const juce::KeyPress& key, juce::Component* ogComp)
{
    //DBG("KeyPressed");
    //if (key.getModifiers().isShiftDown())
    //{
    //    DBG("keyStateChanged mcmk is down");
    //    //setMultiControlState(true);
    //}
    //else /*if (!isKeyDown)*/
    //{
    //    DBG("keyStateChanged mcmk is up");
    //    //setMultiControlState(false);
    //} 

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

void KrumModuleContainer::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    bool alreadySentMidi = false;
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        //take this out for multi-selection
        /*if (alreadySentMidi)
            return;*/

        auto modEd = activeModuleEditors[i];
        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getModuleMidiNote() == midiNoteNumber)
        {
            //sets the flag to paint the module
            modEd->setModulePlaying(true);
        }
        else if (modEd->doesEditorWantMidi())
        {
            modEd->handleMidi(midiChannel, midiNoteNumber);
            if (isMultiControlActive())
            {

            }
            alreadySentMidi = true; //restricting this to only pass the midi message to one module, but removing this could pass the midi to multiple selected modules
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

KrumModuleEditor* KrumModuleContainer::addModuleEditor(KrumModuleEditor* newModuleEditor, bool refreshLayout)
{
    if (newModuleEditor != nullptr)
    {
        addAndMakeVisible(newModuleEditor);
        activeModuleEditors.add(newModuleEditor);
        
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

void KrumModuleContainer::setModuleSelected(KrumModuleEditor* moduleToSelect)
{
    moduleToSelect->setModuleSelected(true);
    currentlySelectedModules.add(moduleToSelect);
    addModuleParamIDs(moduleToSelect);

    
    //repaint();
}

void KrumModuleContainer::setModuleUnselected(KrumModuleEditor* moduleToDeselect)
{
    moduleToDeselect->setModuleSelected(false);
    currentlySelectedModules.removeObject(moduleToDeselect, false);
    removeModuleParamIDs(moduleToDeselect);
    
}

void KrumModuleContainer::deselectAllModules()
{
    
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        setModuleUnselected(activeModuleEditors[i]);
    }
    
    //this should be redundant
    currentlySelectedModules.clear(false);
    paramIDs.clear();
    DBG("ParamIDs cleared, size: " + paramIDs.size());
}

//selects the modules between your last selected module to the newly selected module
void KrumModuleContainer::setModulesSelectedToLastSelection(KrumModuleEditor* moduleToSelect)
{
    if (currentlySelectedModules.size() < 1)
    {
        setModuleSelected(moduleToSelect);
        //if there is no "Last Selection", there is nothing to select up to
        return;
    }

    //get the last selected module's display index
    int lastSelectedDisplayIndex = currentlySelectedModules.getLast()->getModuleDisplayIndex();
   
    //get the current moduleTOSelect's dispaly index
    int moduleToSelectDisplayIndex = moduleToSelect->getModuleDisplayIndex();

    if (lastSelectedDisplayIndex < moduleToSelectDisplayIndex)
    {
        for (int i = 0; i < activeModuleEditors.size(); i++)
        {
            int displayIndex = activeModuleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want
            if (displayIndex == moduleToSelectDisplayIndex ||  
                (displayIndex > lastSelectedDisplayIndex && displayIndex < moduleToSelectDisplayIndex)) 
            {
                setModuleSelected(activeModuleEditors[i]);
            }
        }
    }
    else if (lastSelectedDisplayIndex > moduleToSelectDisplayIndex)
    {
        for (int i = 0; i < activeModuleEditors.size(); i++)
        {
            int displayIndex = activeModuleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want
            if (displayIndex == moduleToSelectDisplayIndex || 
                (displayIndex < lastSelectedDisplayIndex && displayIndex > moduleToSelectDisplayIndex))
            {
                setModuleSelected(activeModuleEditors[i]);
            }
        }
    }
    else
    {
        setModuleSelected(moduleToSelect);
    }

    //this assumes the shift key was down in order for this function to get called
    addParamListeners();

}

void KrumModuleContainer::setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect)
{
    //could add more functationality here
    setModuleSelected(moduleToSelect);
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
        if (!currentlySelectedModules[i]->isModuleTree(treeWhoChanged)) //we don't want to send a change message to a module that sent the message in the first place
        {
            currentlySelectedModules[i]->valueTreePropertyChanged(treeWhoChanged, propertyWhoChanged);
        }
    }
}

//this should probably be triggered by a timercallback. It should NOT be triggered by the APVTS listener call back
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
            apvts->getParameter(newParamString)->setValueNotifyingHost(newValue);

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
                mod->removeSettingsOverlay();
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

//void KrumModuleContainer::sendMouseEventToSelectedComponents(juce::Component* compToExlcude, const juce::MouseEvent& event)
//{
//    for (int i = 0; i < currentlySelectedModules.size(); i++)
//    {
//        auto mod = currentlySelectedModules[i];
//        if (mod != compToExlcude)
//        {
//            mod->mouseDown(event.getEventRelativeTo(mod));
//        }
//    }
//}

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

    if (applyNextParamChange)
    {
        applyParameterChangeToSelectedModules(nextParamChange.paramID, nextParamChange.value);
        applyNextParamChange = false;
    }

    repaint();
}

void KrumModuleContainer::addFileToRecentsFolder(juce::File& file, juce::String name)
{
    pluginEditor->fileBrowser.addFileToRecent(file, name);
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
            auto newMod = addNewModuleEditor(new KrumModuleEditor(modTree, *this, pluginEditor->getAudioFormatManager()));
            newMod->setModuleFile(file); //this should turn the state to "hasfile" if necessary
            newMod->setNumSamplesOfFile(numSamples);
            //newMod->setModuleState(KrumModule::ModuleState::hasFile);

            newMod->setModuleName(name);
            addFileToRecentsFolder(file, name); //should this be handle directly by the container?

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
        currentlySelectedModules[i]->removeSettingsOverlay();
    }
}

void KrumModuleContainer::clearActiveModuleSettingsOverlays()
{
    for (int i = 0; i < activeModuleEditors.size(); i++)
    {
        activeModuleEditors[i]->removeSettingsOverlay();
    }
}

//===================================================================================================================================
//===================================================================================================================================


KrumModuleContainer::DropSampleArea::DropSampleArea(KrumModuleContainer* mc)
    : moduleContainer(mc), InfoPanelComponent("Drop Sample Area", "Drop samples here to make new modules")
{
}

KrumModuleContainer::DropSampleArea::~DropSampleArea()
{}

void KrumModuleContainer::DropSampleArea::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    juce::Colour bgColor = Colors::modulesBGColor;
    juce::Colour borderColor = Colors::moduleHoverOutlineColor;

    if (isMouseOver())
    {
        g.setColour(bgColor.brighter());
    }
    else
    {
        g.setColour(bgColor);
    }

    g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);

    g.setColour(borderColor);
    g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, EditorDimensions::bigOutline);



}

void KrumModuleContainer::DropSampleArea::mouseUp(const juce::MouseEvent& event)
{
    moduleContainer->deselectAllModules();
}


//Files from Favorites or Recents 
bool KrumModuleContainer::DropSampleArea::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails)
{
    auto desc = dragDetails.description.toString();
    return desc.isNotEmpty() && (desc.contains(DragStrings::favoritesDragString) || desc.contains(DragStrings::recentsDragString));
    
    return false;
}

//Files from Favorites or Recents, Drag and Drop Target
void KrumModuleContainer::DropSampleArea::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails)
{
    auto desc = dragDetails.description.toString();
    bool addNextModule = false;                         //set flag true if files are accepted by module, otherwise leave false
    auto& sampler = moduleContainer->pluginEditor->sampler;
    juce::Array<juce::ValueTree> selectedTrees;

    //grab the correct valueTree from the file browser
    if (desc.contains(DragStrings::favoritesDragString))
    {
        selectedTrees = moduleContainer->pluginEditor->fileBrowser.getSelectedFileTrees(KrumFileBrowser::BrowserSections::favorites);
    }
    else if (desc.contains(DragStrings::recentsDragString))
    {
        selectedTrees = moduleContainer->pluginEditor->fileBrowser.getSelectedFileTrees(KrumFileBrowser::BrowserSections::recent);
    }

    //checks to make sure we have enough modules
    if (selectedTrees.size() <= moduleContainer->getNumEmptyModules())
    {
        for (int i = 0; i < selectedTrees.size(); ++i)
        {
            auto fileTree = selectedTrees[i];

            if (!moduleContainer->handleNewFile(fileTree))
            {
                DBG("handling new file failed");
            }

            //if (i == 0)
            //{
            //}
            //else
            //{
            //    auto modulesTree = moduleTree.getParent();
            //    if (modulesTree.hasType(TreeIDs::KRUMMODULES.getParamID()))
            //    {
            //        for (int j = 0; j < modulesTree.getNumChildren(); j++)
            //        {
            //            auto itTree = modulesTree.getChild(j);
            //            if ((int)itTree.getProperty(TreeIDs::moduleState.getParamID()) == 0) //we grab the first empty module
            //            {
            //                auto newModEd = pluginEditor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, pluginEditor, sampler.getFormatManager()));
            //                newModEd->handleNewFile(fileTree);
            //                addNextModule = true;
            //                break;
            //            }
            //        }
            //    }
            //}
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not enough modules available", "");
    }


    //if (addNextModule)
    //{
    //    pluginEditor.addNextModuleEditor();
    //}
}

//EXTERNAL File Drag and Drop Target
bool KrumModuleContainer::DropSampleArea::isInterestedInFileDrag(const juce::StringArray& files)
{
    //return shouldModuleAcceptFileDrop();
    return false;
}

//EXTERNAL File Drag and Drop Target
void KrumModuleContainer::DropSampleArea::filesDropped(const juce::StringArray& files, int x, int y)
{
    //bool addNextModule = false; //set flag true if files are accepted by module, otherwise leave false
    //auto& sampler = editor.sampler;
    //int numFilesDropped = files.size();

    //if(numFilesDropped <= editor.moduleContainer.getNumEmptyModules())
    //{
    //    for (int i = 0; i < files.size(); i++)
    //    {
    //        juce::File audioFile {files[i]};
    //        juce::String fileName = audioFile.getFileName();
    //        juce::int64 numSamples = 0;
    //        if(!audioFile.isDirectory() && sampler.isFileAcceptable(audioFile, numSamples))
    //        {
    //            if (i == 0)
    //            {
    //                handleNewFile(fileName, audioFile, numSamples);
    //                addNextModule = true;
    //                continue;
    //            }

    //            auto modulesTree = moduleTree.getParent();
    //            if (modulesTree.hasType(TreeIDs::KRUMMODULES))
    //            {
    //                for (int j = 0; j < modulesTree.getNumChildren(); j++)
    //                {
    //                    auto itTree = modulesTree.getChild(j);
    //                    if ((int)itTree.getProperty(TreeIDs::moduleState) == 0) //we grab the first empty module
    //                    {
    //                        auto newModEd = editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));
    //                        newModEd->handleNewFile(fileName, audioFile, numSamples);
    //                        addNextModule = true;

    //                        DBG("-------");
    //                        DBG("Module Sampler Index: " + itTree.getProperty(TreeIDs::moduleSamplerIndex).toString());
    //                        DBG("Item: " + audioFile.getFullPathName());

    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //        else
    //        {
    //            DBG("External File Not Acceptable");
    //            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");

    //        }
    //    }
    //}

    /*if (addNextModule)
    {
        editor.addNextModuleEditor();
    }*/
}
