/*
  ==============================================================================

    DragAndDropThumbnail.h
    Created: 28 Sep 2021 8:01:54pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
/*
* This draws the thumbnail of the audioFile that the KrumModuleEditor (parentEditor) gives to it. 
* It can also accept a file that is dragged onto it and will "hot swap" the audio file out with the dropped one, and boy is it hot! 
*       - This feature was one that I have been wanting to make since the start, it was one of the main interactions I wanted in my worflow. 
*       - It is one of the main reasons I wanted to build a drum sampler in the first place
*
*/

class KrumModuleEditor;

class DragAndDropThumbnail :    public juce::Component,
                                public juce::AudioThumbnail,
                                public juce::DragAndDropTarget,
                                public juce::FileDragAndDropTarget

{
public:
    DragAndDropThumbnail(KrumModuleEditor& parentEditor, int sourceSamplesPerThumbnailSample,
        juce::AudioFormatManager& formatManagerToUse,
        juce::AudioThumbnailCache& cacheToUse);

    ~DragAndDropThumbnail() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, juce::Colour bgColor);

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void addDroppedFile(juce::File& newFile);
    void moveDroppedFileToParent();

    void updateThumbnailClipGain(float newVerticalZoom);


    float verticalZoom = 1.0f;
    juce::File droppedFile;
    bool checkDroppedFile = false;
    bool canAcceptFile = false;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    juce::Slider clipGainSlider;
    std::unique_ptr<SliderAttachment> clipGainSliderAttachment;

    KrumModuleEditor& parentEditor;
};