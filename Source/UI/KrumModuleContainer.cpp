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
    : editor(owner), valueTree(valTree)
{
    setInterceptsMouseClicks(true, true);
    setRepaintsOnMouseActivity(true);
    editor->addKeyboardListener(this);
    editor->addKeyListener(this);
    valueTree.addListener(this);
    refreshModuleLayout();
    startTimerHz(30);
        
}

KrumModuleContainer::~KrumModuleContainer()
{
    editor->removeKeyboardListener(this);
    editor->removeKeyListener(this);
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
    int numModules = moduleEditors.size();
    auto area = getLocalBounds();
    auto viewportBounds = editor->modulesViewport.getBounds();
    int viewportWidth = viewportBounds.getWidth();
    int viewportHeight = viewportBounds.getHeight();

    if (numModules == 0)
    {
        setSize(viewportWidth, viewportHeight);
        return;
    }

    int newWidth = (numModules * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage()));

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

    for (int i = 0; i < numModules; i++)
    {
        auto modEd = moduleEditors[i];
        modEd->setTopLeftPosition((modEd->getModuleDisplayIndex() * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage(), EditorDimensions::shrinkage); //set position based off of stored index
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
        else if (state > 0 && index > -1 && moduleEditors.size() > 0)
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

void KrumModuleContainer::parameterChanged(const juce::String& paramterID, float newValue)
{
    //if (currentlySelectedModules.size() > 1)
    //{
    //    for (int i = 0; i < currentlySelectedModules.size(); i++)
    //    {
    //        //if ((int)treeWhoChanged.getProperty(TreeIDs::moduleSamplerIndex.getParamID()) != currentlySelectedModules[i]->getModuleSamplerIndex())
    //        {
    //            
    //            editor->parameters.
    //        }

    //    }
    //}
}

void KrumModuleContainer::sliderValueChanged(juce::Slider* slider)
{
    int sliderOrigin = -1;
    juce::String sliderType{};

    findCompInSelectedModules(slider, sliderOrigin, sliderType);

    DBG("SliderType = " + sliderType);

    if (sliderOrigin >= 0)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            if (i != sliderOrigin)
            {
                if (sliderType == "Module Gain")
                {
                    currentlySelectedModules[i]->volumeSlider.setValue(slider->getValue());
                }
                else if (sliderType == "Pitch")
                {
                    currentlySelectedModules[i]->pitchSlider.setValue(slider->getValue());
                }
                else if (sliderType == "Module Pan")
                {
                    currentlySelectedModules[i]->panSlider.setValue(slider->getValue());
                }
            }
        }
    }

}

void KrumModuleContainer::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    int comboOrigin = -1;
    juce::String comboType{};

    findCompInSelectedModules(comboBoxThatHasChanged, comboOrigin, comboType);

    DBG("comboType = " + comboType);

    if (comboOrigin >= 0)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            if (i != comboOrigin)
            {
                if (comboType == "Output Channel")
                {
                    currentlySelectedModules[i]->outputCombo.setSelectedId(comboBoxThatHasChanged->getSelectedId());
                }
            }
        }
    }
}

void KrumModuleContainer::buttonClicked(juce::Button* buttonClicked)
{
    int buttonOrigin = -1;
    juce::String buttonType{};

    findCompInSelectedModules(buttonClicked, buttonOrigin, buttonType);

    DBG("buttonType =" + buttonType);

    if (buttonOrigin >= 0)
    {
        for (int i = 0; i < currentlySelectedModules.size(); i++)
        {
            if (i != buttonOrigin)
            {

                if (buttonType == "Reverse Button")
                {
                    currentlySelectedModules[i]->reverseButton.setToggleState(buttonClicked->getToggleState(),juce::sendNotificationSync);

                }
                else if (buttonType == "Mute")
                {
                    currentlySelectedModules[i]->muteButton.setToggleState(buttonClicked->getToggleState(), juce::sendNotificationSync);

                }
                else if (buttonType == "Settings")
                {
                    //show module's settings overlay. This will change
                    //currentlySelectedModules[i]->editButton.triggerClick();
                }
                
            }
        }
    }

}

void KrumModuleContainer::buttonStateChanged(juce::Button* buttonChanged)
{

    if (buttonChanged->getState() == juce::Button::buttonDown)
    {
        int buttonOrigin = -1;
        juce::String buttonType{};

        findCompInSelectedModules(buttonChanged, buttonOrigin, buttonType);

       /* if (buttonOrigin >= 0)
        {
            for (int i = 0; i < currentlySelectedModules.size(); i++)
            {
                if (i != buttonOrigin)
                {
                    if (buttonType == "One Shot")
                    {
                        auto note = currentlySelectedModules[i]->getModuleMidiNote();
                        auto chan = currentlySelectedModules[i]->getModuleMidiChannel();
                        auto vel = currentlySelectedModules[i]->buttonClickVelocity;

                        editor->sampler.noteOn(chan, note, vel);
                    }
                }
            }
        }*/

    }
}

void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    //TODO:
    //capture a mouseclick that isn't on any modules, that will clear all selected modules
    
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

bool KrumModuleContainer::keyPressed(const juce::KeyPress& key, juce::Component* ogComp)
{

    return false;
}

bool KrumModuleContainer::keyStateChanged(bool isKeyDown, juce::Component* ogComp)
{

    auto modKeys = juce::ModifierKeys::currentModifiers;
    if (modKeys.isShiftDown())
    {
        setMultiControlState(true);
    }
    else if (!isKeyDown && !modKeys.isShiftDown())
    {
        setMultiControlState(false);
    }
    


    return false;
}

void KrumModuleContainer::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    bool alreadySentMidi = false;
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getModuleMidiNote() == midiNoteNumber)
        {
            modEd->setModulePlaying(true);
        }
        else if (modEd->doesEditorWantMidi())
        {
            modEd->handleMidi(midiChannel, midiNoteNumber);
            alreadySentMidi = true; //restricting this to only pass the midi message to one module, but removing this could pass the midi to multiple selected modules
        }
    }
}

void KrumModuleContainer::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
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
        moduleEditors.add(newModuleEditor);
        
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
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        if (modEd->getModuleDisplayIndex() == displayIndex)
        {
            return modEd;
        }
    }

    return nullptr;
}

KrumModuleEditor* KrumModuleContainer::addNewModuleEditor(KrumModuleEditor* newModuleEditor)
{
     newModuleEditor->setModuleDisplayIndex(moduleEditors.size());
     addModuleEditor(newModuleEditor);
     return newModuleEditor;
}

void KrumModuleContainer::removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout)
{
    if (moduleToRemove)
    {
        updateModuleDisplayIndicesAfterDelete(moduleToRemove->getModuleDisplayIndex());
        moduleEditors.removeObject(moduleToRemove);

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
    moduleToSelect->addParamListener(this);
    //repaint();
}

void KrumModuleContainer::setModuleUnselected(KrumModuleEditor* moduleToDeselect)
{
    moduleToDeselect->setModuleSelected(false);
    currentlySelectedModules.removeObject(moduleToDeselect, false);
    moduleToDeselect->removeParamListener(this);
}

void KrumModuleContainer::deselectAllModules()
{
    currentlySelectedModules.clear(false);
    
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto moduleEditor = moduleEditors[i];
        moduleEditor->removeParamListener(this);
        moduleEditor->setModuleSelected(false);
        moduleEditor->repaint();
    }

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
        for (int i = 0; i < moduleEditors.size(); i++)
        {
            int displayIndex = moduleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want
            if (displayIndex == moduleToSelectDisplayIndex ||  
                (displayIndex > lastSelectedDisplayIndex && displayIndex < moduleToSelectDisplayIndex)) 
            {
                setModuleSelected(moduleEditors[i]);
            }
        }
    }
    else if (lastSelectedDisplayIndex > moduleToSelectDisplayIndex)
    {
        for (int i = 0; i < moduleEditors.size(); i++)
        {
            int displayIndex = moduleEditors[i]->getModuleDisplayIndex();
            //we make sure the display index is in the range we want
            if (displayIndex == moduleToSelectDisplayIndex || 
                (displayIndex < lastSelectedDisplayIndex && displayIndex > moduleToSelectDisplayIndex))
            {
                setModuleSelected(moduleEditors[i]);
            }
        }
    }
    else
    {
        setModuleSelected(moduleToSelect);
    }

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

void KrumModuleContainer::applyChangesToSelectedModules(juce::ValueTree& treeWhoChanged, const juce::Identifier& propertyWhoChanged)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        if (!currentlySelectedModules[i]->isModuleTree(treeWhoChanged)) //we don't want to send a change message to a module that sent the message in the first place
        {
            currentlySelectedModules[i]->valueTreePropertyChanged(treeWhoChanged, propertyWhoChanged);
        }
    }
}

void KrumModuleContainer::clickOneShotOnSelectedModules(const juce::MouseEvent& mouseDownEvent, KrumModuleEditor* eventOrigin, bool mouseDown)
{
    for (int i = 0; i < currentlySelectedModules.size(); i++)
    {
        auto mod = currentlySelectedModules[i];
        if (mod != eventOrigin)
        {
            if (mouseDown)
            {
                mod->handleOneShotButtonMouseDown(mouseDownEvent.getEventRelativeTo(mod));
            }
            else
            {
                mod->handleOneShotButtonMouseUp(mouseDownEvent.getEventRelativeTo(mod));
            }
        }
    }


}

juce::Array<KrumModuleEditor*> KrumModuleContainer::getModulesFromMidiNote(int midiNote)
{
    juce::Array<KrumModuleEditor*>retArray{};

    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        if (modEd->getModuleMidiNote() == midiNote)
        {
            retArray.add(modEd);
            //return modEd;
        }
    }
    
    //return nullptr;
    return retArray;
}


KrumSamplerAudioProcessorEditor* KrumModuleContainer::getEditor()
{
    return editor;
}

juce::OwnedArray<KrumModuleEditor>& KrumModuleContainer::getModuleEditors()
{
    return moduleEditors;
}


int KrumModuleContainer::getNumActiveModules()
{
    int count = 0;
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for(int i = 0; i < moduleEditors.size(); i++)
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

KrumModuleEditor* KrumModuleContainer::getModuleEditor(int index)
{
    
    return moduleEditors.getUnchecked(index);
}

int KrumModuleContainer::getNumModuleEditors()
{
    return moduleEditors.size();
}

void KrumModuleContainer::showModuleClipGainSlider(KrumModuleEditor* moduleEditor)
{
    //this loop clears any shown clipGain sliders
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors.getUnchecked(i);
        if (modEd != nullptr && modEd != moduleEditor)
        {
            modEd->setClipGainSliderVisibility(false);
        }
    }

    moduleEditor->setClipGainSliderVisibility(true);
}

void KrumModuleContainer::showModulePitchSlider(KrumModuleEditor* moduleEditor)
{
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors.getUnchecked(i);
        if (modEd != nullptr && modEd != moduleEditor)
        {
            modEd->setPitchSliderVisibility(false);
        }
    }

    moduleEditor->setPitchSliderVisibility(true);
}

void KrumModuleContainer::showModuleCanAcceptFile(KrumModuleEditor* moduleEditor)
{
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
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

    for (int i = 0; i < moduleEditors.size(); ++i)
    {
        auto modEd = moduleEditors[i];
        if (modEd->getMouseOver())
        {
            mouseOver = true;
            break;
        }
    }


    if (editor && (!mouseOver))
    {
        editor->keyboard.clearHighlightedKey();
    }

    repaint();
}

void KrumModuleContainer::setMultiControlState(bool shouldControl)
{
    multiSelectControlState = shouldControl;
}

void KrumModuleContainer::showFirstEmptyModule()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES.getParamID());
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        if ((int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()) == 0)
        {
            addNewModuleEditor(new KrumModuleEditor(moduleTree, *editor, editor->sampler.getFormatManager()));
            return;
        }
    }
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
            auto modEd = addModuleEditor(new KrumModuleEditor(moduleTree, *editor, editor->sampler.getFormatManager()));
        }
    }
}

bool KrumModuleContainer::isMultiControlActive()
{
    return multiSelectControlState && multipleModulesSelected();
}

bool KrumModuleContainer::findCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type)
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
    for(int i = 0; i < moduleEditors.size(); i++)
    {
        if(moduleEditors[i]->isVisible())
        {
            numVisible++;
        }
    }
    return numVisible;
    
}
