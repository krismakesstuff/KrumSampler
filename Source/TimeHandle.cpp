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


TimeHandle::TimeHandle(KrumModuleEditor& e)
    : InfoPanelComponent("Time Handle", "Lets you adjust the playback start and end when this sample is triggered"),
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
        /*if (property == TreeIDs::moduleFile)
        {
            setHandles(0, editor.thumbnail.getNumSamplesFinished());
        }
        else*/ /*if (property == TreeIDs::moduleStartSample || property == TreeIDs::moduleEndSample)
        {
            repaint();
        }*/
    }
}

void TimeHandle::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    int spacer = 5;
    int handleH = area.getHeight()/* * 0.7f*/;
    int handleW = (int)(handleH * 0.65f);

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(area);

    g.setColour(juce::Colours::white.withAlpha(0.3f));

    juce::Rectangle<int> startRect{ getXFromSample(getStartPosition()), area.getY(), handleW, handleH };
    drawStartPosition(g, startRect);

    juce::Rectangle<int> endRect{ getXFromSample(getEndPosition()) - handleW, area.getY(), handleW, handleH };
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
    setPositionsFromMouse(event);
}

int TimeHandle::getStartPosition()
{
    return editor.moduleTree.getProperty(TreeIDs::moduleStartSample);
}

int TimeHandle::getEndPosition()
{
    return editor.moduleTree.getProperty(TreeIDs::moduleEndSample);
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
    editor.moduleTree.setProperty(TreeIDs::moduleStartSample, startPositionInSamples, nullptr);
}

void TimeHandle::setEndPosition(int endPositionInSamples)
{
    editor.moduleTree.setProperty(TreeIDs::moduleEndSample, endPositionInSamples, nullptr);
}

int TimeHandle::getSampleFromXPos(int x)
{
    auto& thumbnail = editor.thumbnail;
    juce::NormalisableRange<float> sampleRange{ 0, (float)editor.getNumSamplesInFile() };
    juce::NormalisableRange<float> widthRange{ 0, (float)thumbnail.getWidth() };

    int limitedX = juce::jlimit<int>(0, widthRange.end, x);

    auto normalledX = widthRange.convertTo0to1(limitedX);
    int returnSample = (int)sampleRange.convertFrom0to1(normalledX);

    return returnSample;
}

int TimeHandle::getXFromSample(int sample)
{
    if (sample < 0)
    {
        return 0;
    }

    auto& thumbnail = editor.thumbnail;
    juce::NormalisableRange<float> sampleRange{ 0, (float)editor.getNumSamplesInFile() };
    juce::NormalisableRange<float> widthRange{ 0, (float)thumbnail.getWidth() };

    auto normalledSample = sampleRange.convertTo0to1(sample);
    int returnX = juce::jlimit<int>(0, widthRange.end, widthRange.convertFrom0to1(normalledSample));

    return returnX;
}

void TimeHandle::setPositionsFromMouse(const juce::MouseEvent& event)
{
    auto mouseX = event.getPosition().getX();
    
    int samplePos = getSampleFromXPos(mouseX);
    int startX = getXFromSample(getStartPosition());
    int endX = getXFromSample(getEndPosition());
    
    if (mouseX - startX < endX - mouseX) //checks which position the mouse is closest to
    {
        setStartPosition(samplePos);
    }
    else 
    {
        setEndPosition(samplePos);
    }
    
}
