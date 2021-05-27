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
*/

class KrumModuleEditor;
class KrumModule;
class KrumSamplerAudioProcessorEditor;

class KrumModuleContainer : public juce::Component,
                            public juce::DragAndDropTarget
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

    //Most likely you want to call removeModuleEditor() first, it will call this function
    void removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove);
    KrumModuleEditor* getEditorFromModule(KrumModule* krumModule);

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    int findDisplayIndexFromPoint(juce::Point<int> point);
    KrumSamplerAudioProcessorEditor* getEditor();
    juce::Array<KrumModuleEditor*>& getModuleDisplayOrder();

private:
    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
    //juce::Array<int> moduleDisplayOrder{};
    juce::Array<KrumModuleEditor*> moduleDisplayOrder{};

    bool moduleDragging = false;

    KrumSamplerAudioProcessorEditor* editor;
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    juce::Rectangle<int> fadeArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};