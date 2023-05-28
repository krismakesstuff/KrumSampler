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
* This also manages interactions with the mouse selecting module-editors, as well as showing the clip-gain slider
* 
* 
*/

class KrumModuleEditor;
class DummyKrumModuleEditor;
class KrumModule;
class KrumSamplerAudioProcessorEditor;



class KrumModuleContainer : public juce::Component,
                            public juce::Timer,
                            public juce::MidiKeyboardStateListener,
                            public juce::ValueTree::Listener
{
public:
    KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree);
    ~KrumModuleContainer() override;

    void paint(juce::Graphics& g) override;

    void paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition);
    
    void refreshModuleLayout();
    
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    
    void mouseDown(const juce::MouseEvent& event) override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    KrumModuleEditor* addNewModuleEditor(KrumModuleEditor* newModuleEditor);
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);

    void setModuleSelected(KrumModuleEditor* moduleToMakeActive);
    void setModuleUnselected(KrumModuleEditor* moduleToMakeDeselect);
    void deselectAllModules();
    void setModuleSelectedWithShift(KrumModuleEditor* moduleToSelect);
    void setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect);

    bool isModuleSelected(KrumModuleEditor* moduleToCheck);
    
    juce::Array<KrumModuleEditor*> getModulesFromMidiNote(int midiNote);

    KrumSamplerAudioProcessorEditor* getEditor();
    juce::OwnedArray<KrumModuleEditor>& getModuleEditors();
    
    int getNumActiveModules();
    int getNumModuleEditors();
    int getNumEmptyModules();
    KrumModuleEditor* getModuleEditor(int index);

    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);
    void showModulePitchSlider(KrumModuleEditor* moduleEditor);

    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);

    void showFirstEmptyModule();
    void createModuleEditors();


    KrumModuleEditor* getEditorFromDisplayIndex(int displayIndex);
private:

    KrumModuleEditor* addModuleEditor(KrumModuleEditor* moduleToAdd, bool refreshLayout = true);


    void updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted);
    int getNumVisibleModules();

    juce::ValueTree valueTree;

    void timerCallback() override;

    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    juce::OwnedArray<KrumModuleEditor> moduleEditors{};
    juce::OwnedArray<KrumModuleEditor> currentlySelectedModules{};

    KrumSamplerAudioProcessorEditor* editor = nullptr;
    
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
