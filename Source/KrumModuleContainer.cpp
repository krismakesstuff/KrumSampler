/*
  ==============================================================================

    KrumModuleContainer.cpp
    Created: 23 Mar 2021 2:56:05pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KrumModuleContainer.h"
#include "PluginEditor.h"
//#include "Log.h"

//==============================================================================
KrumModuleContainer::KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree)
    : editor(owner), valueTree(valTree)//, fadeArea(50, editor->modulesBG.getHeight())
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
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);
    
    //if (moduleDragInfo.dragging && moduleDragInfo.showOriginBounds)
    //{
    //    g.setColour(juce::Colours::yellow);
    //    g.drawRoundedRectangle(moduleDragInfo.origBounds.reduced(EditorDimensions::shrinkage).toFloat(), EditorDimensions::cornerSize, 1.0f);
    //    //paintLineUnderMouseDrag(g,)
    //}
    //else if (moduleDragInfo.dragging && moduleDragInfo.drawLineBetween)
    //{
    //    g.setColour(juce::Colours::yellow);
    //    
    //    auto lineX = moduleDisplayOrder[moduleDragInfo.leftDisplayIndex]->getRight();
    //    juce::Rectangle<int> lineRect {lineX, EditorDimensions::shrinkage, 5, EditorDimensions::moduleH};
    //    
    //    g.fillRoundedRectangle(lineRect.toFloat(), EditorDimensions::cornerSize/2);
    //}
}

void KrumModuleContainer::paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition)
{
    juce::Rectangle<int> line{ mousePosition.withY(0), mousePosition.withY(getLocalBounds().getBottom()) };

    g.setColour(juce::Colours::white);
    g.fillRect(line);
}


void KrumModuleContainer::refreshModuleLayout()
{
    int numModules = MAX_NUM_MODULES;
    auto area = getLocalBounds();
    auto viewportBounds = editor->modulesViewport.getBounds();
    int viewportWidth = viewportBounds.getWidth();
    int viewportHeight = viewportBounds.getHeight();

    if (numModules == 0)
    {
        setSize(viewportWidth, viewportHeight);
        return;
    }

    int newWidth = (getNumVisibleModules()) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        modEd->setTopLeftPosition((modEd->getModuleDisplayIndex() * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage(), EditorDimensions::shrinkage);
    }

}

void KrumModuleContainer::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if (property == TreeIDs::moduleState)
    {
        int state = treeWhoChanged.getProperty(TreeIDs::moduleState);
        int index = treeWhoChanged.getProperty(TreeIDs::moduleDisplayIndex);
        if (state == KrumModule::ModuleState::empty && index > -1)
        {
            removeModuleEditor(getEditorFromDisplayIndex(index));
        }
        else if (state > 0 && index > -1)
        {
            moduleEditors[index]->repaint();
        }
    }
    else if (property == TreeIDs::moduleDisplayIndex)
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



bool KrumModuleContainer::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    //auto desc = dragDetails.description.toString();
    ////bool isModuleDrag;
    //if (desc.contains("ModuleDrag-") && (!moduleDragInfo.escPressed))
    //{
    //    //get mouse position, draw appropriate lines and shapes
    //    KrumModuleEditor* moduleBeingDragged = static_cast<KrumModuleEditor*>(dragDetails.sourceComponent.get());
    //    moduleDragInfo.setInfo(true, dragDetails.sourceComponent->getBounds(), dragDetails.localPosition);
    //    
    //    isIntersectingWithModules(moduleBeingDragged);
    //    
    //    moduleDragInfo.showOriginBounds = true;
    //    
    //    if(modulesIntersecting.first != nullptr && modulesIntersecting.second != nullptr)
    //    {
    //        //need to draw line between modules
    //        moduleDragInfo.drawLineBetween = true;
    //        moduleDragInfo.showOriginBounds = false;
    //        moduleDragInfo.leftDisplayIndex = modulesIntersecting.first->getModuleDisplayIndex();
    //        moduleDragInfo.rightDisplayIndex = modulesIntersecting.second->getModuleDisplayIndex();
    //        
    //    }
    //    else if (modulesIntersecting.first != nullptr || modulesIntersecting.second != nullptr)
    //    {
    //        moduleDragInfo.drawLineBetween = false;
    //        moduleDragInfo.showOriginBounds = true;
    //    }
    //    return true;
    //}

    return false;
}

void KrumModuleContainer::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    ////if module Drag
    //    //get position and move modules if necessary
    //auto modEdDropped = static_cast<KrumModuleEditor*>(dragSourceDetails.sourceComponent.get());
    //DBG("Module " + juce::String(modEdDropped->getModuleSamplerIndex()) + " Dropped");
    //
    //moduleDragInfo.reset();
    //repaint();
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

void KrumModuleContainer::addModuleEditor(KrumModuleEditor* newModuleEditor, bool refreshLayout) 
{
    if (newModuleEditor != nullptr)
    {
        addAndMakeVisible(newModuleEditor);
        moduleEditors.add(newModuleEditor);
        
        if (refreshLayout) //default true
        {
            refreshModuleLayout();
        }

        repaint();
    }
    else
    {
       DBG("New Editor is NULL");
        //juce::Log::postMessage(__func__, "New Editor is NULL");
    }
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

void KrumModuleContainer::addNewModuleEditor(KrumModuleEditor* newModuleEditor)
{
     newModuleEditor->setModuleDisplayIndex(moduleEditors.size());
     addModuleEditor(newModuleEditor);
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

    /*if (editor)
    {
        for (int i = 0; i < editor->sampler.getNumModules(); i++)
        {
            auto mod = editor->sampler.getModule(i);
            mod->setModuleSelected(false);
        }
    }
    else
    {
        DBG("DeselectingAllModules() -- editor is NULL");
    }*/
}

KrumModuleEditor* KrumModuleContainer::getModuleFromMidiNote(int midiNote)
{
    for (int i = 0; i < moduleEditors.size(); i++)
    {
        auto modEd = moduleEditors[i];
        if (modEd->getModuleMidiNote() == midiNote)
        {
            return modEd;
        }
    }
    
    //juce::Log::postMessage(__func__, "No Module Found by midiNote: " + juce::String(midiNote));
    return nullptr;
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
    
    for(int i = 0; i < moduleEditors.size(); i++)
    {
        if(moduleEditors[i]->getModuleState() == KrumModule::ModuleState::active)
        {
            ++count;
        }
    }
    return count;
    
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
    repaint();
}

void KrumModuleContainer::showFirstEmptyModule()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        if ((int)moduleTree.getProperty(TreeIDs::moduleState) == 0)
        {
            addNewModuleEditor(new KrumModuleEditor(moduleTree, *editor, editor->sampler.getFormatManager()));
            return;
        }
    }
}

void KrumModuleContainer::createModuleEditors()
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);

    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        if (moduleTree.isValid() && ((int)moduleTree.getProperty(TreeIDs::moduleState) > 0)) //reference KrumModule::ModuleState, 0 is empty module
        {
            addModuleEditor(new KrumModuleEditor(moduleTree, *editor, editor->sampler.getFormatManager()));
        }
    }
}

void KrumModuleContainer::updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted)
{
    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        int currentDisplayIndex = (int)moduleTree.getProperty(TreeIDs::moduleDisplayIndex);

        if (currentDisplayIndex > displayIndexDeleted)
        {
            moduleTree.setProperty(TreeIDs::moduleDisplayIndex, currentDisplayIndex - 1, nullptr);
        }
        else if (currentDisplayIndex == displayIndexDeleted)
        {
            moduleTree.setProperty(TreeIDs::moduleDisplayIndex, -1, nullptr);
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
  
//void KrumModuleContainer::moveModule(int moduleIndexToMove, int newDisplayIndex)
//{
//    moduleDisplayOrder.swap(moduleIndexToMove, newDisplayIndex);
//    updateModuleDisplayIndices(true);
//}

//====================================================================================================
// 
// 
//Most likely you want to call removeModuleEditor() first, it will call this function
//void KrumModuleContainer::removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove)
//{
//   
//}
// 
// 
// 
// //void KrumModuleContainer::addModuleToDisplayOrder(KrumModuleEditor* moduleToAdd)
//{
//    /*int displayIndex = moduleToAdd->getModuleDisplayIndex();
// 
//    if(moduleToAdd->getModuleState() == KrumModule::ModuleState::active)
//    {
//        moduleDisplayOrder.insert(displayIndex, std::make_unique<KrumModuleEditor>(std::make_shared<KrumModuleEditor>(moduleToAdd).get()));
//    }
//    else
//    {
//        moduleDisplayOrder.add(std::make_shared<KrumModuleEditor>(moduleToAdd));
//        updateModuleDisplayIndices(true);
//    }*/
//
//    if (moduleToAdd)
//    {
//        moduleEditors.add(std::move(moduleToAdd));
//        addAndMakeVisible(moduleToAdd);
//    }
//
//
//    //juce::Log::postMessage(__func__, "Module Editor added to Display order: " + moduleToAdd->getModuleName());
//    juce::Logger::writeToLog("Module Editor added to Display order: " + moduleToAdd->getModuleName());
//}
// 
// 
// 
// //KrumModuleEditor* KrumModuleContainer::getEditorFromModule(KrumModule* krumModule)
//{
//    return krumModule->getCurrentModuleEditor();
//}

// 
// 
// //void KrumModuleContainer::matchModuleDisplayToMidiNotes(juce::Array<int> sortedMidiAssignments)
//{
//    //juce::Array<KrumModuleEditor*> newDisplayOrder;
//    //
//    //
//    //for(int i = 0; i < sortedMidiAssignments.size(); i++)
//    //{
//    //    auto modEd = getModuleFromMidiNote(sortedMidiAssignments[i]);
//    //    newDisplayOrder.add(modEd);
//    //    //moduleDisplayOrder.move(modEd->getModuleDisplayIndex(), i);
//    //    //DBG("Module: " + juce::String(modEd->getModuleDisplayIndex()) + " Moved To " + juce::String(i));
//    //}
//    //
//    //
//    //int remaining = MAX_NUM_MODULES - newDisplayOrder.size();
//    //for(int i = remaining - 1; i < moduleDisplayOrder.size(); i++)
//    //{
//    //    newDisplayOrder.add(moduleDisplayOrder[i]);
//    //}
//    //
//    //moduleDisplayOrder = newDisplayOrder;
//    //showFirstEmptyModule();
//    refreshModuleLayout();
//    //updateModuleDisplayIndices(true);
//    //repaint();
//    DBG("Match Notes, Bounds: " + getLocalBounds().toString());
//    
//}


// 
// 
// 
// 
//bool KrumModuleContainer::keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent)
//{
//    if(key.isKeyCode(juce::KeyPress::escapeKey) && moduleDragInfo.dragging)
//    {
//        //endModuleDrag(nullptr);
//        //mouseup in module editor
//
//        //moduleDragInfo.draggedModule->mouseUp(dummyMouse);
//        //moduleDragInfo.draggedModule->forceMouseUp();
//        moduleDragInfo.escPressed = true;
//        return true;
//    }
//    return false;
//}

//void KrumModuleContainer::startModuleDrag(KrumModuleEditor* moduleToDrag, const juce::MouseEvent& e)
//{
//    //dragger.startDraggingComponent(moduleToDrag, e);
//    moduleDragInfo.setInfo(true, moduleToDrag->getBoundsInParent(), e.getPosition());
//    setInterceptsMouseClicks(true, false);
//}
//
//void KrumModuleContainer::dragModule(KrumModuleEditor* moduleToDrag, const juce::MouseEvent& e)
//{
//    isIntersectingWithModules(moduleToDrag);
//
//    moduleDragInfo.showOriginBounds = true;
//
//    if(modulesIntersecting.first != nullptr && modulesIntersecting.second != nullptr)
//    {
//        //need to draw line between modules
//        moduleDragInfo.drawLineBetween = true;
//        moduleDragInfo.showOriginBounds = false;
//        moduleDragInfo.leftDisplayIndex = modulesIntersecting.first->getModuleDisplayIndex();
//        moduleDragInfo.rightDisplayIndex = modulesIntersecting.second->getModuleDisplayIndex();
//
//    }
//    else if (modulesIntersecting.first != nullptr || modulesIntersecting.second != nullptr)
//    {
//
//        moduleDragInfo.drawLineBetween = false;
//        moduleDragInfo.showOriginBounds = true;
//    }
//
//       //see if intersect right(?) half
//            //account for right half being the immediate left of moduleToDrag
//        //account for the left side of the 0 module
//
//    moduleDragInfo.draggedModule = moduleToDrag;
//    moduleDragInfo.mousePos = e.getPosition();
//    moduleToDrag->toFront(false);
//    //dragger.dragComponent(moduleToDrag, e, nullptr);
//    //juce::Rectangle<int> boundsIfMouseOver;
//    DBG("Position To Test: " + e.getPosition().toString());
//
//}
//
//void KrumModuleContainer::endModuleDrag(KrumModuleEditor* moduleToDrag)
//{
//    if(moduleToDrag == nullptr || moduleToDrag->getBounds().intersects(moduleDragInfo.origBounds))
//    {
//        refreshModuleLayout();
//    }
//
//    moduleDragInfo.reset();
//    updateDisplayIndices(true);
//
//    setInterceptsMouseClicks(true, true);
//}
//
//bool KrumModuleContainer::isModuleBeingDragged()
//{
//    return moduleDragInfo.dragging;
//}

//void KrumModuleContainer::isIntersectingWithModules(KrumModuleEditor* editorToTest)
//{
//    auto boundsToTest = editorToTest->getBounds();
//    modulesIntersecting.reset();
//    for(int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        auto modEd = moduleDisplayOrder[i]->get();
//        if(modEd == editorToTest)
//        {
//            continue;
//        }
//        
//        if(modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getBounds().intersects(boundsToTest))
//        {
//            if(modulesIntersecting.first == nullptr)
//            {
//                modulesIntersecting.first = modEd;
//            }
//            else if(modulesIntersecting.second == nullptr)
//            {
//                modulesIntersecting.second = modEd;
//                return;
//            }
//        }
//    }
//    
//}
//
//bool KrumModuleContainer::isMouseOverModule(const juce::Point<int> positionToTest, juce::Rectangle<int>& bounds)
//{
//    for (int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        auto modEd = moduleDisplayOrder[i]->get();
//        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getBounds().contains(positionToTest))
//        {
//            bounds = modEd->getBounds();
//            DBG("Mouse Over: Module " + juce::String(modEd->getModuleDisplayIndex()) + " - Bounds: " + bounds.toString());
//            return true;
//        }
//        else
//        {
//            //DBG("Not in Bounds: " +)
//        }
//    }
//    return false;
//}
