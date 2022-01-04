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
    auto& tree = editor.moduleTree;
    tree.addListener(this);
    
    if (editor.getModuleState() > KrumModule::ModuleState::empty)
    {
        setHandles(tree.getProperty(TreeIDs::moduleStartSample), tree.getProperty(TreeIDs::moduleEndSample));
    }
}

TimeHandle::~TimeHandle()
{}

void TimeHandle::valueTreePropertyChanged(juce::ValueTree & treeWhoChanged, const juce::Identifier & property)
{
    if (treeWhoChanged == editor.moduleTree)
    {
        if (property == TreeIDs::moduleStartSample)
        {
            startSamplePosition = treeWhoChanged[property];
            repaint();
        }
        else if (property == TreeIDs::moduleEndSample)
        {
            endSamplePosition = treeWhoChanged[property];
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
    int handleH = area.getHeight()/* * 0.7f*/;
    int handleW = (int)(handleH * 0.65f);

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(area);

    g.setColour(juce::Colours::white.withAlpha(0.5f));

    juce::Rectangle<int> startRect{ getXFromSample(startSamplePosition), area.getY(), handleW, handleH };
    drawStartPosition(g, startRect);

    juce::Rectangle<int> endRect{ getXFromSample(endSamplePosition) - handleW, area.getY(), handleW, handleH };
    drawEndPosition(g, endRect);

}

void TimeHandle::drawStartPosition(juce::Graphics& g, juce::Rectangle<int>& area)
{
    juce::Path path;
    path.addTriangle(area.getTopLeft().toFloat(), area.getBottomLeft().toFloat(), { (float)area.getRight(), (float)area.getCentreY() });
    g.fillPath(path);
}

void TimeHandle::drawEndPosition(juce::Graphics& g, juce::Rectangle<int>& area)
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

void TimeHandle::setHandles(int startSample, int endSample)
{
    setStartPosition(startSample);
    setEndPosition(endSample);
}

void TimeHandle::resetHandles()
{
    setStartPosition(0);
    setEndPosition(0);
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
    auto& thumbnail = editor.thumbnail;
    juce::NormalisableRange<float> sampleRange{ 0, (float)thumbnail.getNumSamplesFinished() };
    juce::NormalisableRange<float> widthRange{ 0, (float)thumbnail.getWidth() };
    int limitedX = juce::jlimit<int>(0, widthRange.end, x);

    auto normalledX = widthRange.convertTo0to1(limitedX);
    int returnSample = (int)sampleRange.convertFrom0to1(normalledX);
    //DBG("Returned Sample: " + juce::String(returnSample));

    return returnSample;
}

int TimeHandle::getXFromSample(int sample)
{
    if (sample < 0)
    {
        return 0;
    }

    auto& thumbnail = editor.thumbnail;
    juce::NormalisableRange<float> sampleRange{ 0, (float)thumbnail.getNumSamplesFinished() };
    juce::NormalisableRange<float> widthRange{ 0, (float)thumbnail.getWidth() };

    auto normalledSample = sampleRange.convertTo0to1(sample);
    int returnX = juce::jlimit<int>(0, widthRange.end, widthRange.convertFrom0to1(normalledSample));
    //DBG("Returned X: " + juce::String(returnX));

    return returnX;
}

void TimeHandle::setPositionsFromMouse(const juce::MouseEvent& event)
{
    auto mouseX = event.getPosition().getX();
    
    int samplePos = getSampleFromXPos(mouseX);
    int startX = getXFromSample(startSamplePosition);
    int endX = getXFromSample(endSamplePosition);
    
    if (mouseX - startX < endX - mouseX) //checks which position the mouse is closest to
    {
            startSamplePosition = samplePos;
    }
    else 
    {
        endSamplePosition = samplePos;
    }
    
}