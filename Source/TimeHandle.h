/*
  ==============================================================================

    TimeHandle.h
    Created: 3 Jan 2022 1:09:59pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "InfoPanel.h"

class KrumModuleEditor;
class DragAndDropThumbnail;

class TimeHandle :  public InfoPanelComponent,
                    public juce::ValueTree::Listener
{
public:
    //TimeHandle(int startPosition, int endPosition, KrumModuleEditor& editor);
    TimeHandle(KrumModuleEditor& editor);
    ~TimeHandle() override;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void drawStartPosition(juce::Graphics& g, juce::Rectangle<int>& area);
    void drawEndPosition(juce::Graphics& g, juce::Rectangle<int>& area);

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    int getStartPosition();
    int getEndPosition();

    void setHandles(int startSample, int endSample);
    void resetHandles();


private:

    friend class DragAndDropThumbnail;
    void setStartPosition(int startPositionInSamples);
    void setEndPosition(int endPositionInSamples);

    int getSampleFromXPos(int x);
    int getXFromSample(int sample);
    void setPositionsFromMouse(const juce::MouseEvent& event);

    int startSamplePosition = 0;
    int endSamplePosition = 0;

    KrumModuleEditor& editor;
};