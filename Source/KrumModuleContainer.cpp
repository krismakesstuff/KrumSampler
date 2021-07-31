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

//==============================================================================
KrumModuleContainer::KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner)
    : editor(owner), fadeArea(50, editor->modulesBG.getHeight())
{
    refreshModuleLayout(true);
    setInterceptsMouseClicks(true, true);
    setRepaintsOnMouseActivity(true);
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
    //g.fillAll(bgColor);
    auto area = getLocalBounds();
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), EditorDimensions::cornerSize);

    if (editor->sampler.getNumModules() == 0)
    {
        g.setColour(juce::Colours::darkgrey.darker());
        g.drawFittedText("Drop a Sample up there", area.reduced(20), juce::Justification::centred, 1);
    }

    if (moduleDragging)
    {
        paintLineUnderMouseDrag(g, getMouseXYRelative());
    }
}

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
    int numModules = moduleDisplayOrder.size();
    auto area = getLocalBounds();
    auto viewportBounds = editor->modulesViewport.getBounds();
    int viewportWidth = viewportBounds.getWidth();
    int viewportHeight = viewportBounds.getHeight();

    if (numModules == 0)
    {
        setSize(viewportWidth, viewportHeight);
        return;
    }

    int newWidth = viewportWidth;

    //5 is the number of modules that will fit in the container and not need to scroll. Maybe make this a variable for easy resizing of module size
    if (numModules > 5)
    {
        newWidth += (numModules - 5) * (EditorDimensions::moduleW + EditorDimensions::extraShrinkage());
    }

    //MUST set this size before we reposition the modules. Otherwise viewport won't scroll!
    setSize(newWidth, viewportHeight);

    auto zeroMod = moduleDisplayOrder[0];
    if (zeroMod == nullptr)
    {
       // return;
    }
    
    zeroMod->setTopLeftPosition(area.getX() + EditorDimensions::extraShrinkage(), area.getY() + EditorDimensions::extraShrinkage());

    for (int i = 1; i < numModules; i++)
    {
        auto modEd = moduleDisplayOrder[i];
        auto prevModEd = moduleDisplayOrder[i - 1];

        if (makeVisible)
        {
            addAndMakeVisible(modEd);
        }

        modEd->setTopLeftPosition(prevModEd->getRight() + EditorDimensions::extraShrinkage(), area.getY() + EditorDimensions::extraShrinkage());
    }

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
        DBG("New Editor is NULL");
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

    juce::MessageManagerLock lock;
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
}

void KrumModuleContainer::addModuleToDisplayOrder(KrumModuleEditor* moduleToAdd)
{
    moduleDisplayOrder.insert(moduleToAdd->getModuleDisplayIndex(), moduleToAdd);
}

//Most likely you want to call removeModuleEditor() first, it will call this function
void KrumModuleContainer::removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove)
{
    moduleDisplayOrder.remove(moduleToRemove->getModuleDisplayIndex());
}

bool KrumModuleContainer::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    if (dragSourceDetails.description.toString().compare("ModuleDragAndDrop") == 0)
    {
        moduleDragging = true;
        juce::MessageManagerLock lock;
        repaint();
        return true;
    }
    return false;
}

void KrumModuleContainer::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    moduleDragging = false;

    auto krumModule = static_cast<KrumModuleEditor*>(dragSourceDetails.sourceComponent.get());
    if (krumModule)
    {
        int displayIndex = findDisplayIndexFromPoint(dragSourceDetails.localPosition);
        //moveModule(krumModule, displayIndex);
    }
    else
    {
        DBG("Module "+ krumModule->getName() + " is NULL");
    }
}

int KrumModuleContainer::findDisplayIndexFromPoint(juce::Point<int> point)
{

    //come back
    //the idea was to locate the module under the mouse, but there might be a better way to do this

    return 0;
}

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


KrumModuleEditor* KrumModuleContainer::getEditorFromModule(KrumModule* krumModule)
{
    return krumModule->getCurrentModuleEditor();
}
