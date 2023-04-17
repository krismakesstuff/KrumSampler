/*
  ==============================================================================

    DragAndDropThumbnail.h
    Created: 28 Sep 2021 8:01:54pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../InfoPanel.h"

/*
* This draws the thumbnail of the audioFile that the KrumModuleEditor (parentEditor) gives to it. 
* It can also accept a file that is dragged onto it and will "hot swap" the audio file out with the dropped one, and boy is it hot! 
* 
*
* On a personal note - this feature was one that I have been wanting to make since the start, it was one of the main interactions I wanted in my worflow. 
*
*/


class KrumModuleEditor;

class DragAndDropThumbnail :    public InfoPanelComponent,
                                public juce::AudioThumbnail,
                                public juce::DragAndDropTarget,
                                public juce::ValueTree::Listener/*,
                                public juce::FileDragAndDropTarget*/

{
public:
    DragAndDropThumbnail(KrumModuleEditor& parentEditor, int sourceSamplesPerThumbnailSample,
        juce::AudioFormatManager& formatManagerToUse,
        juce::AudioThumbnailCache& cacheToUse);

    ~DragAndDropThumbnail() override;


    void valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property) override;


    //For External D&D, still not sure if I should add this or not.
    //bool isInterestedInFileDrag(const juce::StringArray& files) override;
    //void filesDropped(const juce::StringArray& files, int x, int y) override;

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor);

    void paintStartBar(juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds, juce::Colour barColor, int barWidth);
    void paintEndBar(juce::Graphics& g, juce::Rectangle<int>& thumbnailBounds, juce::Colour barColor, int barWidth);

    void setChannelColor(juce::Colour newColor);
    void setThumbnailBGColor(juce::Colour newColor);

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void addDroppedFile(juce::File& newFile);
    void moveDroppedFileToParent();

    void updateThumbnailClipGain(float newVerticalZoom);

    float verticalZoom = 2.0f;
    juce::File droppedFile;
    bool checkDroppedFile = false;
    bool canAcceptFile = false;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    juce::Slider clipGainSlider;
    std::unique_ptr<SliderAttachment> clipGainSliderAttachment;

    KrumModuleEditor& parentEditor;
    juce::Colour thumbnailBGColor{ juce::Colours::white };
    juce::Colour channelColor{ juce::Colours::blue };
    
};


