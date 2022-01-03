/*
  ==============================================================================

    TimeHandle.cpp
    Created: 3 Jan 2022 1:09:59pm
    Author:  krisc

  ==============================================================================
*/

#include "TimeHandle.h"
#include "KrumModuleEditor.h"
#include "PluginEditor.h"


//TimeHandle::TimeHandle(int startPos, int endPos, KrumModuleEditor& e)
//    : InfoPanelComponent("Time Handle", "Lets you adjust where the playback starts and ends when this sample is triggered"),
//    startSamplePosition(startPos),
//    endSamplePosition(endPos), editor(e)
//{}

TimeHandle::TimeHandle(KrumModuleEditor& e)
    : InfoPanelComponent("Time Handle", "Lets you adjust where the playback starts and ends when this sample is triggered"),
        editor(e)
{
    editor.moduleTree.addListener(this);
}

TimeHandle::~TimeHandle()
{}

void TimeHandle::valueTreePropertyChanged(juce::ValueTree & treeWhosePropertyHasChanged, const juce::Identifier & property)
{
    if (treeWhosePropertyHasChanged == editor.moduleTree)
    {
        if (property == TreeIDs::moduleStartSample || property == TreeIDs::moduleEndSample)
        {
            repaint();
        }
    }
}

void TimeHandle::resized()
{
}

void TimeHandle::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    int spacer = 5;
    int handleSize = area.getHeight() * 0.7f;

    g.setColour(juce::Colours::white.withAlpha(0.5f));

    juce::Rectangle<int> startRect{ getXFromSample(startSamplePosition), area.getY() + spacer, handleSize, handleSize };
    drawStartPositionBar(g, startRect);

    juce::Rectangle<int> endRect{ getXFromSample(endSamplePosition), area.getY() + spacer, handleSize, handleSize };
    drawEndPositionBar(g, endRect);

}

void TimeHandle::drawStartPositionBar(juce::Graphics& g, juce::Rectangle<int>& area)
{
    juce::Path path;
    path.addTriangle(area.getTopLeft().toFloat(), area.getBottomLeft().toFloat(), { (float)area.getRight(), (float)area.getCentreY() });


    g.fillPath(path);
}

void TimeHandle::drawEndPositionBar(juce::Graphics& g, juce::Rectangle<int>& area)
{
    juce::Path path;
    path.addTriangle({ (float)area.getX(), (float)area.getCentreY() }, area.getTopRight().toFloat(), area.getBottomRight().toFloat());
    g.fillPath(path);
}

void TimeHandle::mouseDown(const juce::MouseEvent& event)
{
    setPositionsFromMouse(event);
}

void TimeHandle::mouseDrag(const juce::MouseEvent& event)
{
    setPositionsFromMouse(event);
}

void TimeHandle::mouseUp(const juce::MouseEvent& event)
{
    //set Positions
    setPositionsFromMouse(event);

    //update value tree with position so sampler can adjust
    auto& moduleTree = editor.moduleTree;
    moduleTree.setProperty(TreeIDs::moduleStartSample, startSamplePosition, nullptr);
    moduleTree.setProperty(TreeIDs::moduleEndSample, endSamplePosition, nullptr);

    DBG("Start Position: " + juce::String(startSamplePosition));
    DBG("End Position: " + juce::String(endSamplePosition));
}

int TimeHandle::getStartPosition()
{
    return startSamplePosition;
}

int TimeHandle::getEndPosition()
{
    return endSamplePosition;
}

void TimeHandle::setStartPosition(int startPositionInSamples)
{
    startSamplePosition = startPositionInSamples;
}

void TimeHandle::setEndPosition(int endPositionInSamples)
{
    endSamplePosition = endPositionInSamples;
}

int TimeHandle::getSampleFromXPos(int x)
{
    juce::NormalisableRange<float> sampleRange{ 0, (float)editor.thumbnail.getNumSamplesFinished() };
    juce::NormalisableRange<float> widthRange{ 0, (float)editor.thumbnail.getWidth() };
    int limitedX = juce::jlimit<int>(0, widthRange.end, x);

    auto normalledX = widthRange.convertTo0to1(limitedX);
    int returnSample = (int)sampleRange.convertFrom0to1(normalledX);
    DBG("Returned Sample: " + juce::String(returnSample));

    return returnSample;
}

int TimeHandle::getXFromSample(int sample)
{
    juce::NormalisableRange<float> sampleRange{ 0, (float)editor.thumbnail.getNumSamplesFinished() };
    juce::NormalisableRange<float> widthRange{ 0, (float)editor.thumbnail.getWidth() };

    auto normalledSample = sampleRange.convertTo0to1(sample);
    int returnX = widthRange.convertFrom0to1(normalledSample);
    DBG("Returned X: " + juce::String(returnX));

    return returnX;
}

void TimeHandle::setPositionsFromMouse(const juce::MouseEvent& event)
{
    auto mouseX = event.getPosition().getX();
    int centerX = (endSamplePosition - startSamplePosition) / 2;

    if (mouseX > centerX)
    {
        //set endPosition
        endSamplePosition = getSampleFromXPos(mouseX);
    }
    else
    {
        //set startPosition
        startSamplePosition = getSampleFromXPos(mouseX);
    }
}