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
    : juce::AudioThumbnail(sourceSamplesPerThumbnailSample, formatManagerToUse, cacheToUse), parentEditor(modEditor), /*timeHandle(0, getNumSamplesFinished(), *this),*/
        InfoPanelComponent("Waveform Thumbnail", "Displays the current sample. Also provides clip gain. With the mouse over the thumbnail, use the scroll wheel to set the gain, or use the slider that appears.")// You can also drop new samples on this and it will 'hot swap' to the new sample")
{
    setRepaintsOnMouseActivity(true);
    clipGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    clipGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    clipGainSlider.onValueChange = [this] { updateThumbnailClipGain(clipGainSlider.getValue()); };
    addChildComponent(clipGainSlider);

    //addAndMakeVisible(timeHandle);
}

DragAndDropThumbnail::~DragAndDropThumbnail()
{}

bool DragAndDropThumbnail::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
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
        moveDroppedFileToParent();
    }
    else
    {
        DBG("File Doesn't exist or invalid format::addDroppedFile()");
    }
}

void DragAndDropThumbnail::moveDroppedFileToParent()
{
    parentEditor.moduleTree.setProperty(TreeIDs::moduleFile, droppedFile.getFullPathName(), nullptr);

    //reset the clipgain value to 0?
    clipGainSliderAttachment.reset(new SliderAttachment(parentEditor.editor.parameters, TreeIDs::paramModuleClipGain + juce::String(parentEditor.getModuleSamplerIndex()), clipGainSlider));

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

    auto color = parentEditor.getModuleColor();
    g.setColour(juce::Colours::black);
    g.fillRect(area);

    if (getNumChannels() == 0)
    {
        paintIfNoFileLoaded(g, area, juce::Colours::black);
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
    int handleH = 15;

    clipGainSlider.setBounds(area.getRight() - sliderW - (spacer * 2), area.getY() + spacer, sliderW, area.getHeight() - (spacer * 2));
    //timeHandle.setBounds(area.getX(), )
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
    g.setColour(juce::Colours::black);
    g.fillRect(thumbnailBounds);

    g.setColour(moduleColor);

    drawChannels(g, thumbnailBounds, 0.0, getTotalLength(), verticalZoom);
}


void DragAndDropThumbnail::mouseEnter(const juce::MouseEvent& e)
{
    parentEditor.editor.moduleContainer.showModuleClipGainSlider(&parentEditor);
    InfoPanelComponent::mouseEnter(e);
}

void DragAndDropThumbnail::mouseExit(const juce::MouseEvent& e)
{
    if (isMouseOver(true))
    {
        return;
    }

    clipGainSlider.setVisible(false);
    canAcceptFile = false;
    InfoPanelComponent::mouseExit(e);
}

void DragAndDropThumbnail::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    clipGainSlider.mouseWheelMove(e, wheel);
}

//----------------------------------------------------------------------------------------------------------------







