/*
  ==============================================================================

    KrumFileDrop.cpp
    Created: 23 Mar 2021 2:38:22pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KrumFileDrop.h"
#include "KrumModuleContainer.h"
#include "KrumSampler.h"
#include "PluginEditor.h"
//#include "KrumFileBrowser.cpp"

//==============================================================================
KrumFileDrop::KrumFileDrop(KrumSamplerAudioProcessorEditor& e, KrumModuleContainer& c, juce::AudioProcessorValueTreeState& a, KrumFileBrowser& browser)
    : editor(e), container(c), parameters(a), fileBrowser(browser)
{
    setTooltip("Drag and drop samples from anywhere!");   
}

KrumFileDrop::~KrumFileDrop()
{
    
}

void KrumFileDrop::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    juce::Colour bgColor = isMouseOver() ? mouseOverColor : defBGColor;


    g.setColour(mouseOverColor);
    g.drawRoundedRectangle(area.toFloat(), /*container.getEditor().getEditorDimensions().*/EditorDimensions::cornerSize,/* container->editor->dimensions.*/EditorDimensions::bigOutline);


    g.setColour(bgColor);
    g.fillRoundedRectangle(area.toFloat().reduced(EditorDimensions::bigOutline), EditorDimensions::cornerSize);

    g.setColour(bgColor.contrasting(0.5f));
    g.drawFittedText("Drop Samples Here", area.reduced(10), juce::Justification::centred, 3, 0.50f);
}

//Drag and Drop Target
bool KrumFileDrop::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    //DBG("Checking Interest");
    auto desc = dragSourceDetails.description.toString();
    return desc.contains("FileBrowserDrag");
}

//Drag and Drop Target
void KrumFileDrop::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    //doesn't accept folders!!
    
    for (int i = 0; i < fileBrowser.getNumSelectedItems(); i++)
    {
        auto krumItem = fileBrowser.getSelectedItem(i);
        if (krumItem != nullptr)
        {
            auto file = krumItem->getFile();
            auto itemName = krumItem->getItemName(); 
            if (!krumItem->mightContainSubItems())
            {
                if (createNewModule(file, itemName))
                {
                    fileBrowser.addFileToRecent(file, itemName);
                    DBG("Item" + file.getFullPathName());
                }
            }
            else
            {
                DBG("Folders Not Supported");
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Folders Not Supported Yet", "I might support folder dropping in the future. For now, drop the folder into favorites to keep it locally.");
            }
        }
        else
        {
            DBG("Krum Item NULL");
        }

    }
    DBG("Dropped: " + dragSourceDetails.sourceComponent.get()->getName());

}

//EXTERNAL File Drag and Drop Target
bool KrumFileDrop::isInterestedInFileDrag(const juce::StringArray& files)
{
    return true;
}

//EXTERNAL File Drag and Drop Target
void KrumFileDrop::filesDropped(const juce::StringArray& files, int x, int y)
{
    
    droppedFiles = files;
    for (auto file : files)
    {
        juce::File audioFile{ file };
        if (fileBrowser.doesPreviewerSupport(audioFile.getFileExtension()))
        {
            if (createNewModule(audioFile, audioFile.getFileName()))
            {
               fileBrowser.addFileToRecent(audioFile, audioFile.getFileName());
            }
        }
        else
        {
            DBG("Audio Format Not Supported");
        }

    }
}


bool KrumFileDrop::createNewModule(juce::File audioFile, juce::String name)
{
    int index = container.findFreeModuleIndex();
    return editor.createModule(name, index, audioFile);
}
