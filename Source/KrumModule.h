/*
  ==============================================================================

    KrumModule.h
    Created: 20 Feb 2021 1:53:52pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "KrumModuleProcessor.h"
#include "KrumModuleEditor.h"


/*
* 
* KrumModule represents the module channel strip. It is split into two classes to handle audio and GUI separately. Much like PluginProcessor and PluginEditor.
* 
* KrumModuleProcessor(audio) and KrumModuleEditor(GUI) are defined in their own .cpp and .h files.
* 
* The KrumModuleEditor can be deleted and created at any time and thus the KrumModule and KrumModuleProcessor must be able to exist without it. 
* The KrumModuleProcessor interfaces with the KrumSampler engine directly.
* Both of these classes have access to this(KrumModule) class via reference, often reffered to as "parent". 
* This "parent" object holds the ModuleInfo, which is all of the info needed to operate, save, and restore modules.
* 
* In the future, I will make the module drag and droppable. That is why there are two indexes tracked. "Index" and "displayIndex", displayIndex is unused for now but is still tracked and saved.
* 
* TODO:
* - Make the module draggable to move display order on screen. Use DragHandle
* 
*/




class KrumModule  : public juce::ValueTree::Listener
{
public:

    enum ModuleSettingIDs
    {
        moduleFilePath_Id = 1,
        moduleMidiNote_Id,
        moduleMidiChannel_Id,
        moduleColor_Id,
        moduleDisplayIndex_Id,
        moduleReConfig_Id,
        moduleDelete_Id,
    };
    
    enum ModuleState
    {
        empty,      //0
        hasFile,    //1
        active,     //2
        //numerical values are used in the value tree saving
    };

    
    KrumModule(KrumSampler& km, juce::ValueTree& valTree, juce::AudioProcessorValueTreeState* apvts);

    ~KrumModule();

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    ModuleState getModuleState();
    juce::File& getSampleFile();
    int getMidiTriggerNote();
    int getMidiTriggerChannel();
    juce::String& getModuleName();
    int getModuleSamplerIndex();
    int getModuleDisplayIndex();
    juce::Colour getModuleColor();

    bool isModuleActive();
    bool isModuleEmpty();
    bool isModuleActiveOrHasFile();    

    std::atomic<float>* getModuleGain();
    std::atomic<float>* getModuleClipGain();
    std::atomic<float>* getModulePan();


    //bool doesModuleNeedToUpdateTree();
    /*void triggerNoteOn();
    void triggerNoteOff();*/
    /*bool isModulePlaying();
    void setModulePlaying(const bool isPlaying);*/
    
    //KrumModule(KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);
    
    //KrumModule(juce::String& moduleName, int index, juce::File file, KrumSampler& km,
    //    juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);

    //void updateModuleFromTree();
    
    //void handleNoteOn(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;
    //void handleNoteOff(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;

    //void setModuleActive(bool isActive);
   // void setModuleGain(float newGain);
    //void setModulePan(float newPan);

    //void getValuesFromTree();
    //void updateValuesInTree(bool printBefore = false);
    //void clearModuleValueTree();

    //void updateAudioAtomics();
    
    //void updateEditorFromState();
    
  /*  void setModuleColor(juce::Colour newModuleColor, bool refreshChildren = true);
    void setModuleSelected(bool isModuleSelected);
    void removeSettingsOverlay(bool keepSettings = true);

    void setModuleState(ModuleState newState);
    void setSampleFile(juce::File& newSample);

    void setMidiTriggerNote(int midiNoteNumber, bool removOld = false);
    void setMidiTriggerChannel(int newMidiChannel);
    void setModuleName(juce::String newName);
    void setModuleSamplerIndex(int newIndex);
    void setModuleDisplayIndex(int newIndex);

    bool isModuleDragging();
    void setModuleDragging(bool isDragging);*/
    //void reassignSliders();

    //KrumModuleEditor* createModuleEditor(KrumSamplerAudioProcessorEditor& editor);
    //KrumModuleEditor* getCurrentModuleEditor();
    //bool hasEditor();
    //void setEditorVisibility(bool isVisible);
    //void deleteModuleEditor();

    //int deleteEntireModule();

    /*
        ModuleInfo struct holds the info that needs to be saved
            - provides easy access for the processor and editor.
            - BUT, prefer to access these via setters and getters!!
            - Although, you can access them directly, doing that doesn't ensure they will be saved, among doing other tasks that might need to be done
    */

    //struct ModuleInfo
    //{
    //    juce::Colour moduleColor{ juce::Colours::blue };
    //    juce::File audioFile;

    //    //These rely on being intialized like this to see if we need to display the module differently, i.e. ModuleSettingsOverlay
    //    int midiNote = 0;
    //    int midiChannel = 0;
    //    juce::String name;
    //    int samplerIndex;
    //    int displayIndex;
    //    ModuleState moduleState = ModuleState::empty;
    //    std::atomic<bool> modulePlaying = false;
    //    
    //    
    //    //Is not used yet, but is here as a flag to be used with the draggingHandle being used. Will be impelemented soon!
    //    bool moduleDragging = false;

    //};

    //ModuleInfo info;

private:

    void updateSamplerSound();
    void removeSamplerSound();
    juce::String getIndexString();

    bool needsToUpdateTree = false;

    friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;

    //the sampler owns this module so this editor should be owned by the plugin editor, not this module
    //std::shared_ptr<KrumModuleEditor> moduleEditor = nullptr;

    juce::AudioProcessorValueTreeState* parameters = nullptr;
    juce::ValueTree moduleTree;
    
    KrumSampler& sampler;

    
    
    JUCE_LEAK_DETECTOR(KrumModule)

};
