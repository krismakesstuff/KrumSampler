/*
  ==============================================================================

    TimeHandle.h
    Created: 3 Jan 2022 1:09:59pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UI\InfoPanel.h"

/*
* This class is the two handles that live underneath the DragAndDropThumbnail. This will change the start and stop sample that the module's file will play from and to. 
* Changes are made and stored locally within this class and then updated to the valueTree where the KrumModule will handle the change as needed.
* 
* TODO:
*   - make a fade in and fade out
*/

class KrumModuleEditor;
class DragAndDropThumbnail;

class TimeHandle :  public InfoPanelComponent,
                    public juce::ValueTree::Listener
{
public:
    TimeHandle(KrumModuleEditor& editor);
    ~TimeHandle() override;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

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

    KrumModuleEditor& editor;
};