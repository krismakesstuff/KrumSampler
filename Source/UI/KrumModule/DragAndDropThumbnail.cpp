/*
  ==============================================================================

    DragAndDropThumbnail.cpp
    Created: 28 Sep 2021 8:01:54pm
    Author:  krisc

  ==============================================================================
*/

#include "DragAndDropThumbnail.h"
#include "KrumModuleEditor.h"
#include "../PluginEditor.h"
#include "../Source/KrumModule.h"
#include "TimeHandle.h"



DragAndDropThumbnail::DragAndDropThumbnail(KrumModuleEditor& modEditor, int sourceSamplesPerThumbnailSample, juce::AudioFormatManager& formatManagerToUse, juce::AudioThumbnailCache& cacheToUse)
    : juce::AudioThumbnail(sourceSamplesPerThumbnailSample, formatManagerToUse, cacheToUse), parentEditor(modEditor), /*timeHandle(0, getNumSamplesFinished(), *this),*/
        InfoPanelComponent("Waveform Thumbnail", "Displays the current sample. Also provides clip gain. With the mouse over the thumbnail, use the scroll wheel to set the gain, or use the slider that appears.")// You can also drop new samples on this and it will 'hot swap' to the new sample")
{
    setRepaintsOnMouseActivity(true);
    clipGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    clipGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    clipGainSlider.onValueChange = [this] { updateThumbnailClipGain(clipGainSlider.getValue()); };
    addChildComponent(clipGainSlider);

    parentEditor.moduleTree.addListener(this);
}

DragAndDropThumbnail::~DragAndDropThumbnail()
{}

void DragAndDropThumbnail::valueTreePropertyChanged(juce::ValueTree & treeWhoChanged, const juce::Identifier & property)
{
    if (treeWhoChanged == parentEditor.moduleTree && (property == juce::Identifier(TreeIDs::moduleStartSample.getParamID()) || property == juce::Identifier(TreeIDs::moduleEndSample.getParamID())))
    {
        repaint();
    }
}

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
    if (newFile.existsAsFile() && (parentEditor.moduleContainer.getPluginEditor()->getAudioFormatManager().findFormatForFileExtension(newFile.getFileExtension()) != nullptr))
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
    parentEditor.moduleTree.setProperty(TreeIDs::moduleFile.getParamID(), droppedFile.getFullPathName(), nullptr);
    clipGainSlider.setValue(juce::Decibels::decibelsToGain(0.0));
    parentEditor.timeHandle.resetHandles();

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

    //auto color = parentEditor.getModuleColor();

    g.setColour(thumbnailBGColor);
    g.fillRect(area);

    if (getNumChannels() == 0)
    {
        paintIfNoFileLoaded(g, area, juce::Colours::white);
    }
    else
    {
        paintIfFileLoaded(g, area, channelColor);
    }

    //auto barColor = juce::Colours::white.withAlpha(0.3f);
    auto barColor = parentEditor.getModuleColor().darker().withAlpha(0.4f);
    auto fillColor = thumbnailBGColor.darker(0.9f).withAlpha(0.5f);
    //auto fillColor = barColor.darker(0.5f).withAlpha(0.3f);
    int barWidth = 1;

    paintStartBar(g, area, barColor, fillColor, barWidth);
    paintEndBar(g, area, barColor, fillColor,  barWidth);

    if (canAcceptFile)
    {
        g.setColour(Colors::getCanDropFileColor());
        g.drawRect(area, 2);
    }

}


void DragAndDropThumbnail::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor)
{
    g.setColour(bgColor);
    g.fillRect(thumbnailBounds);
    g.setColour(bgColor.contrasting());
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void DragAndDropThumbnail::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour channelColor)
{
    /*g.setColour(juce::Colours::black);
    g.fillRect(thumbnailBounds);*/

    g.setColour(channelColor);

    drawChannels(g, thumbnailBounds, 0.0, getTotalLength(), verticalZoom);
}

void DragAndDropThumbnail::paintStartBar(juce::Graphics& g, juce::Rectangle<int>& area, juce::Colour barColor, juce::Colour fillColor, int barWidth)
{
    auto& timeHandle = parentEditor.timeHandle;
    int startX = timeHandle.getXFromSample(timeHandle.getStartPosition());

    juce::Rectangle<int> barRect{ startX, area.getY(), barWidth, area.getHeight() };
    juce::Rectangle<int> startToBarRect { area.getX(), area.getY(), startX, area.getHeight() };

    g.setColour(fillColor);
    g.fillRect(startToBarRect);

    g.setColour(barColor);
    g.fillRect(barRect);
}

void DragAndDropThumbnail::paintEndBar(juce::Graphics& g, juce::Rectangle<int>& area, juce::Colour barColor, juce::Colour fillColor, int barWidth)
{
    auto& timeHandle = parentEditor.timeHandle;
    int endX = timeHandle.getXFromSample(timeHandle.getEndPosition());

    juce::Rectangle<int> barRect{ endX - barWidth, area.getY(), barWidth, area.getHeight() };
    juce::Rectangle<int> endToBarRect = barRect.withWidth(area.getWidth() - (endX - barWidth));

    g.setColour(fillColor);
    g.fillRect(endToBarRect);

    //g.setColour(barColor);
    g.setColour(barColor);
    g.fillRect(barRect);
}

void DragAndDropThumbnail::setChannelColor(juce::Colour newColor)
{
    channelColor = newColor;
}

void DragAndDropThumbnail::setThumbnailBGColor(juce::Colour newColor)
{
    thumbnailBGColor = newColor;
}

void DragAndDropThumbnail::resized()
{
    auto area = getLocalBounds();

    int sliderW = 15;
    int spacer = 5;
    //int handleH = 15;

    clipGainSlider.setBounds(area.getRight() - sliderW - (spacer * 2), area.getY() + spacer, sliderW, area.getHeight() - (spacer * 2));
}

void DragAndDropThumbnail::mouseDown(const juce::MouseEvent& e)
{
    //if (e.mods.isPopupMenu())
    //{
    //    //add two
    //}

    InfoPanelComponent::mouseDown(e);
}

void DragAndDropThumbnail::mouseEnter(const juce::MouseEvent& e)
{
    parentEditor.moduleContainer.showModuleClipGainSlider(&parentEditor);
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







