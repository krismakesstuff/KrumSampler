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
    startTimerHz(30);
    editor->addKeyboardListener(this);
    valueTree.addListener(this);
    refreshModuleLayout();
        
}

KrumModuleContainer::~KrumModuleContainer()
{
    editor->removeKeyboardListener(this);
    valueTree.removeListener(this);
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
        if (state == KrumModule::ModuleState::empty && index > -1)
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

}

void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    auto mousePos = event.getMouseDownPosition();
    
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        auto modBounds = modEd->getBoundsInParent();

        if (modBounds.contains(mousePos))
        {
            setModuleSelected(modEd);
        }
        else
        {
            setModuleUnselected(modEd);
        }
    }
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

void KrumModuleContainer::setModuleSelected(KrumModuleEditor* moduleToMakeActive)
{
    deselectAllModules();
    moduleToMakeActive->setModuleSelected(true);
    //repaint();
}

void KrumModuleContainer::setModuleUnselected(KrumModuleEditor* moduleToDeselect)
{
    moduleToDeselect->setModuleSelected(false);
}

void KrumModuleContainer::deselectAllModules()
{
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        moduleEditors[i]->setModuleSelected(false);
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

juce::OwnedArray<KrumModuleEditor>& KrumModuleContainer::getModuleDisplayOrder()
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
