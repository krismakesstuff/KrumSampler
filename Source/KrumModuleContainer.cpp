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
KrumModuleContainer::KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner) 
    : editor(owner), fadeArea(50, editor->modulesBG.getHeight())
{
    
    addKeyListener(this);
    refreshModuleLayout();
    setInterceptsMouseClicks(true, true);
    setRepaintsOnMouseActivity(true);
    startTimerHz(30);
    //juce::Logger::writeToLog("Module Container created");
        
}

KrumModuleContainer::~KrumModuleContainer()
{
//    for (int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        auto editor = moduleDisplayOrder[i];
//        editor->setVisible(false);
//    }
}

void KrumModuleContainer::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);
    
    if (moduleDragInfo.dragging && moduleDragInfo.showOriginBounds)
    {
        g.setColour(juce::Colours::yellow);
        g.drawRoundedRectangle(moduleDragInfo.origBounds.reduced(EditorDimensions::shrinkage).toFloat(), EditorDimensions::cornerSize, 1.0f);
        //paintLineUnderMouseDrag(g,)
    }
    else if (moduleDragInfo.dragging && moduleDragInfo.drawLineBetween)
    {
        g.setColour(juce::Colours::yellow);
        
        auto lineX = moduleDisplayOrder[moduleDragInfo.leftDisplayIndex]->getRight();
        juce::Rectangle<int> lineRect {lineX, EditorDimensions::shrinkage, 5, EditorDimensions::moduleH};
        
        g.fillRoundedRectangle(lineRect.toFloat(), EditorDimensions::cornerSize/2);
    }
}

void KrumModuleContainer::paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition)
{
    juce::Rectangle<int> line{ mousePosition.withY(0), mousePosition.withY(getLocalBounds().getBottom()) };

    g.setColour(juce::Colours::white);
    g.fillRect(line);
}


void KrumModuleContainer::refreshModuleLayout()
{
    //int numModules = moduleDisplayOrder.size();
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

    int newWidth = (MAX_NUM_MODULES) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
        modEd->setTopLeftPosition((i * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage(), EditorDimensions::shrinkage);
    }

}

void KrumModuleContainer::mouseDown(const juce::MouseEvent& event)
{
    auto mousePos = event.getMouseDownPosition();
    
    for (int i = 0; i < editor->sampler.getNumModules(); i++)
    {
        auto mod = editor->sampler.getModule(i);
        juce::Rectangle<int> modBounds;
        if (mod->hasEditor())
        {
            auto modEditor = mod->getCurrentModuleEditor();
            modBounds = modEditor->getBoundsInParent();
        }
        //Controlling the mouseDown from here so we can "click-off" and it will unselect all modules.
        if (modBounds.contains(mousePos))
        {
            setModuleSelected(mod);
        }
        else
        {
            setModuleUnselected(mod);
        }
    }
    
}

bool KrumModuleContainer::keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent)
{
    if(key.isKeyCode(juce::KeyPress::escapeKey) && moduleDragInfo.dragging)
    {
        //endModuleDrag(nullptr);
        //mouseup in module editor

        //moduleDragInfo.draggedModule->mouseUp(dummyMouse);
        //moduleDragInfo.draggedModule->forceMouseUp();
        moduleDragInfo.escPressed = true;
        return true;
    }
    return false;
}

bool KrumModuleContainer::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    auto desc = dragDetails.description.toString();
    //bool isModuleDrag;
    if (desc.contains("ModuleDrag-") && (!moduleDragInfo.escPressed))
    {
        //get mouse position, draw appropriate lines and shapes
        KrumModuleEditor* moduleBeingDragged = static_cast<KrumModuleEditor*>(dragDetails.sourceComponent.get());
        moduleDragInfo.setInfo(true, dragDetails.sourceComponent->getBounds(), dragDetails.localPosition);
        
        isIntersectingWithModules(moduleBeingDragged);
        
        moduleDragInfo.showOriginBounds = true;
        
        if(modulesIntersecting.first != nullptr && modulesIntersecting.second != nullptr)
        {
            //need to draw line between modules
            moduleDragInfo.drawLineBetween = true;
            moduleDragInfo.showOriginBounds = false;
            moduleDragInfo.leftDisplayIndex = modulesIntersecting.first->getModuleDisplayIndex();
            moduleDragInfo.rightDisplayIndex = modulesIntersecting.second->getModuleDisplayIndex();
            
        }
        else if (modulesIntersecting.first != nullptr || modulesIntersecting.second != nullptr)
        {
            moduleDragInfo.drawLineBetween = false;
            moduleDragInfo.showOriginBounds = true;
        }
        return true;
    }

    return false;
}

void KrumModuleContainer::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails)
{
    //if module Drag
        //get position and move modules if necessary
    auto modEdDropped = static_cast<KrumModuleEditor*>(dragSourceDetails.sourceComponent.get());
    DBG("Module " + juce::String(modEdDropped->getModuleSamplerIndex()) + " Dropped");
    
    moduleDragInfo.reset();
    repaint();
}

void KrumModuleContainer::addMidiListener(juce::MidiKeyboardStateListener* newListener)
{
    editor->addKeyboardListener(newListener);
}

void KrumModuleContainer::removeMidiListener(juce::MidiKeyboardStateListener* listenerToDelete)
{
    editor->removeKeyboardListener(listenerToDelete);
}

//int KrumModuleContainer::findFreeModuleIndex()
//{
//    return editor->audioProcessor.findFreeModuleIndex();
//}

void KrumModuleContainer::addModuleEditor(KrumModuleEditor* newModuleEditor, bool refreshLayout)
{
    if (newModuleEditor != nullptr)
    {
        addAndMakeVisible(newModuleEditor);
        addModuleToDisplayOrder(newModuleEditor);
        repaint();
        
        if (refreshLayout)
        {
            refreshModuleLayout();
        }
    }
    else
    {
     //   DBG("New Editor is NULL");
        //juce::Log::postMessage(__func__, "New Editor is NULL");
    }
}

void KrumModuleContainer::removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout)
{
    removeModuleFromDisplayOrder(moduleToRemove); // we remove so the other modules can slide forward
    //moduleDisplayOrder.add(moduleToRemove); //we add this back to the end knowing it now has an "empty" state and won't be drawn
    if (refreshLayout)
    {
        refreshModuleLayout();
    }
    
}


void KrumModuleContainer::moveModule(int moduleIndexToMove, int newDisplayIndex)
{
    moduleDisplayOrder.swap(moduleIndexToMove, newDisplayIndex);
    updateModuleDisplayIndices(true);
}

void KrumModuleContainer::setModuleSelected(KrumModule* moduleToMakeActive)
{
    deselectAllModules();
    moduleToMakeActive->setModuleSelected(true);
    repaint();
}

void KrumModuleContainer::setModuleUnselected(KrumModule* moduleToDeselect)
{
    moduleToDeselect->setModuleSelected(false);
}

void KrumModuleContainer::deselectAllModules()
{
    for (int i = 0; i < editor->sampler.getNumModules(); i++)
    {
        auto mod = editor->sampler.getModule(i);
        mod->setModuleSelected(false);
    }
}

KrumModuleEditor* KrumModuleContainer::getModuleFromMidiNote(int midiNote)
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
        if (modEd->getModuleMidiNote() == midiNote)
        {
            return modEd;
        }
    }
    
//    juce::Log::postMessage(__func__, "No Module Found by midiNote: " + juce::String(midiNote));
    return nullptr;
}

void KrumModuleContainer::addModuleToDisplayOrder(KrumModuleEditor* moduleToAdd)
{
    //moduleDisplayOrder.insert(moduleToAdd->getModuleDisplayIndex(), moduleToAdd);
    int displayIndex = moduleToAdd->getModuleDisplayIndex();
    if(moduleToAdd->getModuleState() == KrumModule::ModuleState::active)
    {
        moduleDisplayOrder.insert(displayIndex, moduleToAdd);
    }
    else
    {
        moduleDisplayOrder.add(moduleToAdd);
        updateModuleDisplayIndices(true);
    }

    //juce::Log::postMessage(__func__, "Module Editor added to Display order: " + moduleToAdd->getModuleName());
    juce::Logger::writeToLog("Module Editor added to Display order: " + moduleToAdd->getModuleName());
}

//Most likely you want to call removeModuleEditor() first, it will call this function
void KrumModuleContainer::removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove)
{
    moduleDisplayOrder.remove(moduleToRemove->getModuleDisplayIndex());
    updateModuleDisplayIndices(true);
    //juce::Log::postMessage(__func__, "Module Editor removed from Display order: " + moduleToRemove->getModuleName());
    juce::Logger::writeToLog("Module Editor removed from Display order: " + moduleToRemove->getModuleName());
}

KrumSamplerAudioProcessorEditor* KrumModuleContainer::getEditor()
{
    return editor;
}

void KrumModuleContainer::matchModuleDisplayToMidiNotes(juce::Array<int> sortedMidiAssignments)
{
    juce::Array<KrumModuleEditor*> newDisplayOrder;
    
    for(int i = 0; i < sortedMidiAssignments.size(); i++)
    {
        auto modEd = getModuleFromMidiNote(sortedMidiAssignments[i]);
        newDisplayOrder.add(modEd);
    }
    
    
    moduleDisplayOrder = newDisplayOrder;
    refreshModuleLayout();
    updateModuleDisplayIndices(true);
    
    
}

void KrumModuleContainer::updateModuleDisplayIndices(bool shouldRepaint)
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        moduleDisplayOrder[i]->setModuleDisplayIndex(i);
    }
    
    if(shouldRepaint)
    {
        repaint();
    }
    
}

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

void KrumModuleContainer::isIntersectingWithModules(KrumModuleEditor* editorToTest)
{
    auto boundsToTest = editorToTest->getBounds();
    modulesIntersecting.reset();
    for(int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
        if(modEd == editorToTest)
        {
            continue;
        }
        
        if(modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getBounds().intersects(boundsToTest))
        {
            if(modulesIntersecting.first == nullptr)
            {
                modulesIntersecting.first = modEd;
            }
            else if(modulesIntersecting.second == nullptr)
            {
                modulesIntersecting.second = modEd;
                return;
            }
        }
    }
    
}

bool KrumModuleContainer::isMouseOverModule(const juce::Point<int> positionToTest, juce::Rectangle<int>& bounds)
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
        if (modEd->getModuleState() == KrumModule::ModuleState::active && modEd->getBounds().contains(positionToTest))
        {
            bounds = modEd->getBounds();
            DBG("Mouse Over: Module " + juce::String(modEd->getModuleDisplayIndex()) + " - Bounds: " + bounds.toString());
            return true;
        }
        else
        {
            //DBG("Not in Bounds: " +)
        }
    }
    return false;
}

juce::Array<KrumModuleEditor*>& KrumModuleContainer::getModuleDisplayOrder()
{
    return moduleDisplayOrder;
}

int KrumModuleContainer::getNumActiveModules()
{
    int count = 0;
    
    for(int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        if(moduleDisplayOrder[i]->getModuleState() == KrumModule::ModuleState::active)
        {
            ++count;
        }
    }
    return count;
    
}

int KrumModuleContainer::getNumModuleEditors()
{
    return moduleDisplayOrder.size();
}

void KrumModuleContainer::showModuleClipGainSlider(KrumModuleEditor* moduleEditor)
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        if (moduleDisplayOrder[i] != moduleEditor)
        {
            moduleDisplayOrder[i]->setClipGainSliderVisibility(false);
        }
    }

    moduleEditor->setClipGainSliderVisibility(true);
}

void KrumModuleContainer::showModuleCanAcceptFile(KrumModuleEditor* moduleEditor)
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
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

KrumModuleEditor* KrumModuleContainer::getEditorFromModule(KrumModule* krumModule)
{
    return krumModule->getCurrentModuleEditor();
}

void KrumModuleContainer::timerCallback()
{
//    for (int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        auto modEd = moduleDisplayOrder[i];
//        if (modEd->needsToDrawThumbnail())
//        {
//            modEd->setAndDrawThumbnail();
//        }
//        else
//        {
//            repaint();
//        }
//    }
    repaint();
}

