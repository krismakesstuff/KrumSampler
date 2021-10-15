/*
  ==============================================================================

    DragAndDropThumbnail.cpp
    Created: 28 Sep 2021 8:01:54pm
    Author:  krisc

  ==============================================================================
*/

#include "DragAndDropThumbnail.h"
#include "KrumModuleEditor.h"
#include "PluginEditor.h"
#include "KrumModule.h"


DragAndDropThumbnail::DragAndDropThumbnail(KrumModuleEditor& modEditor, int sourceSamplesPerThumbnailSample, juce::AudioFormatManager& formatManagerToUse, juce::AudioThumbnailCache& cacheToUse)
    :juce::AudioThumbnail(sourceSamplesPerThumbnailSample, formatManagerToUse, cacheToUse), parentEditor(modEditor)
{
    setRepaintsOnMouseActivity(true);
    clipGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    clipGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    clipGainSlider.onValueChange = [this] { updateThumbnailClipGain(clipGainSlider.getValue()); };
    addChildComponent(clipGainSlider);
}

DragAndDropThumbnail::~DragAndDropThumbnail()
{
}

bool DragAndDropThumbnail::isInterestedInFileDrag(const juce::StringArray& files)
{
    //parentEditor.editor.moduleContainer.showModuleCanAcceptFile(&parentEditor);
    return true;
}

void DragAndDropThumbnail::filesDropped(const juce::StringArray& files, int x, int y)
{
    if (!(files.size() > 1))
    {
        for (auto file : files)
        {
            juce::File newFile{ file };
            addDroppedFile(newFile);
        }
    }
}

bool DragAndDropThumbnail::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    //parentEditor.editor.moduleContainer.showModuleCanAcceptFile(&parentEditor);
    return true;
}

void DragAndDropThumbnail::itemDropped(const SourceDetails& dragSourceDetails)
{
    auto description = dragSourceDetails.description.toString();
    DBG("Drag description: " + description);

    auto filePath = description.substring(description.indexOf("-") + 1);
    DBG("File Path: " + filePath);
    juce::File newFile{ filePath };
    addDroppedFile(newFile);
}

//This will check for validaty before adding
void DragAndDropThumbnail::addDroppedFile(juce::File& newFile)
{
    canAcceptFile = false;
    if (newFile.existsAsFile() && (parentEditor.editor.getAudioFormatManager()->findFormatForFileExtension(newFile.getFileExtension()) != nullptr))
    {
        droppedFile = newFile;
        if (parentEditor.parent.info.modulePlaying)
        {
            checkDroppedFile = true;
            parentEditor.drawThumbnail = true;
        }
        else
        {
            moveDroppedFileToParent();
        }
    }
    else
    {
        DBG("File Doesn't exist or invalid format::addDroppedFile()");
    }
}

void DragAndDropThumbnail::moveDroppedFileToParent()
{
    auto& parent = parentEditor.parent;

    parent.setSampleFile(droppedFile);
    parent.moduleProcessor->sampler.updateModuleSample(&parent);
    clipGainSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleClipGain_ID + juce::String(parent.getModuleIndex()), clipGainSlider));

    parentEditor.drawThumbnail = true;
    checkDroppedFile = false;
}

void DragAndDropThumbnail::updateThumbnailClipGain(float newVerticalZoom)
{
    verticalZoom = newVerticalZoom;
}

void DragAndDropThumbnail::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    auto color = parentEditor.parent.info.moduleColor;
    g.setColour(color);
    g.fillRect(area);

    if (getNumChannels() == 0)
    {
        paintIfNoFileLoaded(g, area, color);
    }
    else
    {
        paintIfFileLoaded(g, area, color);
    }

    if (canAcceptFile)
    {
        g.setColour(juce::Colours::red);
        g.drawRect(area, 2);
    }

}

void DragAndDropThumbnail::resized()
{
    auto area = getLocalBounds();

    int sliderW = 15;
    int spacer = 5;

    clipGainSlider.setBounds(area.getRight() - sliderW - (spacer * 2), area.getY() + spacer, sliderW, area.getHeight() - (spacer * 2));
}

void DragAndDropThumbnail::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor)
{
    g.setColour(bgColor);
    g.fillRect(thumbnailBounds);
    g.setColour(bgColor.contrasting());
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void DragAndDropThumbnail::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour moduleColor)
{
    g.setColour(moduleColor.darker(0.8f));
    g.fillRect(thumbnailBounds);

    g.setColour(moduleColor);

    drawChannels(g, thumbnailBounds, 0.0, getTotalLength(), verticalZoom);
}


void DragAndDropThumbnail::mouseEnter(const juce::MouseEvent& e)
{
    parentEditor.editor.moduleContainer.showModuleClipGainSlider(&parentEditor);

    //clipGainSlider.setVisible(true);
    repaint();
}

void DragAndDropThumbnail::mouseExit(const juce::MouseEvent& e)
{
    if (isMouseOver(true))
    {
        return;
    }

    clipGainSlider.setVisible(false);
    canAcceptFile = false;
    repaint();
}

void DragAndDropThumbnail::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    clipGainSlider.mouseWheelMove(e, wheel);
}
