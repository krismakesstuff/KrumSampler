/*
  ==============================================================================

    KrumModuleContainer.h
    Created: 23 Mar 2021 2:56:05pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
* 
* A class to hold and manage KrumModuleEditors. It holds the module-editors and not the actual modules. It defines the viewport in which they are seen. 
* 
*/

class KrumModuleEditor;
class KrumModule;
class KrumSamplerAudioProcessorEditor;

class KrumModuleContainer : public juce::Component,
                            public juce::DragAndDropTarget,
                            public juce::Timer
{
public:
    KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner);
    ~KrumModuleContainer() override;

    void paint(juce::Graphics& g) override;

    void paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition);
    
    void refreshModuleLayout(bool makeVisible);
    
    void mouseDown(const juce::MouseEvent& event) override;

    void addMidiListener(juce::MidiKeyboardStateListener* newListener);
    void removeMidiListener(juce::MidiKeyboardStateListener* listenerToDelete);

    int findFreeModuleIndex();
    void addModuleEditor(KrumModuleEditor* newModule, bool refreshLayout = true);
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);
    void moveModule(KrumModule* moduleToMove, int newDisplayIndex);

    void setModuleSelected(KrumModule* moduleToMakeActive);
    void setModuleUnselected(KrumModule* moduleToMakeDeselect);
    
    KrumModuleEditor* getModuleFromMidiNote(int midiNote);

    void addModuleToDisplayOrder(KrumModuleEditor* moduleToAdd);

    
    void removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove);
    KrumModuleEditor* getEditorFromModule(KrumModule* krumModule);

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    int findDisplayIndexFromPoint(juce::Point<int> point);
    KrumSamplerAudioProcessorEditor* getEditor();
    juce::Array<KrumModuleEditor*>& getModuleDisplayOrder();
    int getNumModuleEditors();

private:

    void timerCallback() override;

    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    //could probably use a linked list here..
    juce::Array<KrumModuleEditor*> moduleDisplayOrder{};

    bool moduleDragging = false;

    KrumSamplerAudioProcessorEditor* editor;
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    juce::Rectangle<int> fadeArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};