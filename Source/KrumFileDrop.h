/*
  ==============================================================================

    KrumFileDrop.h
    Created: 23 Mar 2021 2:38:22pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>



class KrumSampler;
class KrumModuleContainer;
class KrumSamplerAudioProcessorEditor;

class KrumTreeItem;
class KrumFileBrowser;
//==============================================================================
/*
* 
* A File drop component. This can accept files from outside of the app as well as from the in app file browser. 
* If valid files are dropped on it will automatically make a new module with that audio file. If there are multiple, it will make a new module for each file. 
* 
*/
class KrumFileDrop  :   public juce::Component,
                        public juce::DragAndDropTarget,
                        public juce::FileDragAndDropTarget,
                        public juce::SettableTooltipClient
{

public:
    KrumFileDrop(KrumSamplerAudioProcessorEditor& e, KrumModuleContainer& container, juce::AudioProcessorValueTreeState& a, KrumFileBrowser& browser);
    ~KrumFileDrop() override;

    void paint (juce::Graphics&) override;
    
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void createNewModule(juce::File audioFile, juce::String name = juce::String());

private:

    friend class KrumModuleContainer;

    KrumSamplerAudioProcessorEditor& editor;
    juce::AudioProcessorValueTreeState& parameters;
    KrumFileBrowser& fileBrowser;
    KrumModuleContainer& container;
    juce::StringArray droppedFiles;


    juce::Colour defBGColor{ juce::Colours::black };
    juce::Colour mouseOverColor{ juce::Colours::darkgrey };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumFileDrop)
};
