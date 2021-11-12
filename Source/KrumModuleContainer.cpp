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
    refreshModuleLayout(true);
    setInterceptsMouseClicks(true, true);
    setRepaintsOnMouseActivity(true);
    startTimerHz(30);
    //juce::Logger::writeToLog("Module Container created");
    
    editors.ensureStorageAllocated(MAX_NUM_MODULES);
  
    for(int i = 0; i < MAX_NUM_MODULES; i++)
    {
        auto newEditor = editors.add(new DummyKrumModuleEditor());
        newEditor->setBounds((i * EditorDimensions::moduleW) + EditorDimensions::extraShrinkage(), 5,EditorDimensions::moduleW, EditorDimensions::moduleH);
        addAndMakeVisible(newEditor);
    }
    
}

KrumModuleContainer::~KrumModuleContainer()
{
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto editor = moduleDisplayOrder[i];
        editor->setVisible(false);
    }
}

void KrumModuleContainer::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);

//    if (editor->sampler.getNumModules() == 0)
//    {
//        g.setColour(juce::Colours::darkgrey.darker());
//        g.drawFittedText("Drop a Sample up there", area.reduced(20), juce::Justification::centred, 1);
//    }
    
    //drawEditors(g);
}

//void KrumModuleContainer::drawEditors(juce::Graphics& g)
//{
//    g.setColour(juce::Colours::darkgrey);
//
//    for(int i = 0; i < editors.size(); i++)
//    {
//        juce::Rectangle<float> modBounds { ((float)i * EditorDimensions::moduleW) + 10, 5, EditorDimensions::moduleW, EditorDimensions::moduleH};
//
//        g.drawRoundedRectangle(modBounds.reduced(5), 3.0f, 1.0f);
//
//    }
//}

void KrumModuleContainer::paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition)
{
    juce::Rectangle<int> line{ mousePosition.withY(0), mousePosition.withY(getLocalBounds().getBottom()) };

    g.setColour(juce::Colours::white);
    g.fillRect(line);
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

void KrumModuleContainer::refreshModuleLayout(bool makeVisible)
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

    //int newWidth = viewportWidth;
    int newWidth = (MAX_NUM_MODULES) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());
    //5 is the number of modules that will fit in the container and not need to scroll. Maybe make this a variable for easy resizing of module size
//    if (numModules > 5)
//    {
//        newWidth += (numModules - 5) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());
//    }

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

//    auto zeroMod = moduleDisplayOrder[0];
//    if (zeroMod == nullptr)
//    {
//       // return;
//    }
//
//    zeroMod->setTopLeftPosition(area.getX() + EditorDimensions::extraShrinkage(), area.getY() + EditorDimensions::extraShrinkage());
//
//    for (int i = 1; i < numModules; i++)
//    {
//        auto modEd = moduleDisplayOrder[i];
//        auto prevModEd = moduleDisplayOrder[i - 1];
//
//        if (makeVisible)
//        {
//            addAndMakeVisible(modEd);
//        }
//
//        modEd->setTopLeftPosition(prevModEd->getRight() + EditorDimensions::extraShrinkage(), area.getY() + EditorDimensions::extraShrinkage());
//    }
}

void KrumModuleContainer::addMidiListener(juce::MidiKeyboardStateListener* newListener)
{
    editor->addKeyboardListener(newListener);
}

void KrumModuleContainer::removeMidiListener(juce::MidiKeyboardStateListener* listenerToDelete)
{
    editor->removeKeyboardListener(listenerToDelete);
}

int KrumModuleContainer::findFreeModuleIndex()
{
    return editor->audioProcessor.findFreeModuleIndex();
}

void KrumModuleContainer::addModuleEditor(KrumModuleEditor* newModuleEditor, bool refreshLayout)
{
    if (newModuleEditor != nullptr)
    {
        addAndMakeVisible(newModuleEditor);
        addModuleToDisplayOrder(newModuleEditor);
        if (refreshLayout)
        {
            refreshModuleLayout(false);
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
    removeModuleFromDisplayOrder(moduleToRemove);
    if (refreshLayout)
    {
        refreshModuleLayout(false);
    }
    
}


void KrumModuleContainer::moveModule(KrumModule* moduleToMove, int newDisplayIndex)
{
    //TODO
    //this would be the drag and drop functionality, to rearrange the order of the modules
}

void KrumModuleContainer::setModuleSelected(KrumModule* moduleToMakeActive)
{
    //sets all modules to NOT selected, then sets the specfied module
    for (int i = 0; i < editor->sampler.getNumModules(); i++)
    {
        auto mod = editor->sampler.getModule(i);
        mod->setModuleSelected(false);
    }

    moduleToMakeActive->setModuleSelected(true);

    repaint();
}

void KrumModuleContainer::setModuleUnselected(KrumModule* moduleToDeselect)
{
    moduleToDeselect->setModuleSelected(false);
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
    moduleDisplayOrder.insert(moduleToAdd->getModuleDisplayIndex(), moduleToAdd);
    //juce::Log::postMessage(__func__, "Module Editor added to Display order: " + moduleToAdd->getModuleName());
    juce::Logger::writeToLog("Module Editor added to Display order: " + moduleToAdd->getModuleName());
}

//Most likely you want to call removeModuleEditor() first, it will call this function
void KrumModuleContainer::removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove)
{
    moduleDisplayOrder.remove(moduleToRemove->getModuleDisplayIndex());
    //juce::Log::postMessage(__func__, "Module Editor removed from Display order: " + moduleToRemove->getModuleName());
    juce::Logger::writeToLog("Module Editor removed from Display order: " + moduleToRemove->getModuleName());
}

//For Module Dragging, To Be Implemented
// 
//bool KrumModuleContainer::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
//{
//    if (dragSourceDetails.description.toString().compare("ModuleDragAndDrop") == 0)
//    {
//        moduleDragging = true;
//        juce::MessageManagerLock lock;
//        repaint();
//        return true;
//    }
//    return false;
//}
//
//void KrumModuleContainer::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
//{
//    moduleDragging = false;
//
//    auto krumModule = static_cast<KrumModuleEditor*>(dragSourceDetails.sourceComponent.get());
//    if (krumModule)
//    {
//        int displayIndex = findDisplayIndexFromPoint(dragSourceDetails.localPosition);
//        //moveModule(krumModule, displayIndex);
//    }
//    else
//    {
//        DBG("Module "+ krumModule->getName() + " is NULL");
//    }
//}

KrumSamplerAudioProcessorEditor* KrumModuleContainer::getEditor()
{
    return editor;
}

juce::Array<KrumModuleEditor*>& KrumModuleContainer::getModuleDisplayOrder()
{
    return moduleDisplayOrder;
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
    for (int i = 0; i < moduleDisplayOrder.size(); i++)
    {
        auto modEd = moduleDisplayOrder[i];
        if (modEd->needsToDrawThumbnail())
        {
            modEd->setAndDrawThumbnail();
        }
        else
        {
            repaint();
        }
    }
}

