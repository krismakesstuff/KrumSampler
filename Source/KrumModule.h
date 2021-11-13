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

#define THUMBNAIL_RES 256


class KrumModule :  public juce::MidiKeyboardStateListener
                    //public juce::DragAndDropContainer
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
        hasMidi,    //2
        active,     //3
        muted,      //4
        soloed,     //5
        //numerical values are used for value tree saving
    };

    
    
    //KrumModule(KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);
    
    KrumModule(juce::String& moduleName, int index, juce::File file, KrumSampler& km,
        juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);

    KrumModule(int index, KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);

    ~KrumModule() override;

    void updateModuleFromTree();
    
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;

    
    void setModuleSelected(bool isModuleSelected);
    void removeSettingsOverlay(bool keepSettings = true);

    void setModuleState(ModuleState newState);
    ModuleState getModuleState();
    
    juce::File& getSampleFile();
    void setSampleFile(juce::File& newSample);

    int getMidiTriggerNote();
    void setMidiTriggerNote(int midiNoteNumber, bool removOld = false);

    int getMidiTriggerChannel();
    void setMidiTriggerChannel(int newMidiChannel);

    juce::String& getModuleName();
    void setModuleName(juce::String newName);

    bool isModulePlaying();
    void setModulePlaying(const bool isPlaying);

    bool isModuleActive();
    //void setModuleActive(bool isActive);

    int getModuleSamplerIndex();
    void setModuleSamplerIndex(int newIndex);

    int getModuleDisplayIndex();
    void setModuleDisplayIndex(int newIndex);

    juce::Colour getModuleColor();
    void setModuleColor(juce::Colour newModuleColor, bool refreshChildren = true);

    void triggerNoteOn();
    void triggerNoteOff();

    void setModuleGain(float newGain);
    std::atomic<float>* getModuleGain();
    std::atomic<float>* getModuleClipGain();

    void setModulePan(float newPan);
    std::atomic<float>* getModulePan();

    bool doesModuleNeedToUpdateTree();

    void getValuesFromTree();
    void updateValuesInTree(bool printBefore = false);
    void clearModuleValueTree();

    void updateAudioAtomics();

    void reassignSliders();

    KrumModuleEditor* createModuleEditor(KrumSamplerAudioProcessorEditor& editor);
    KrumModuleEditor* getCurrentModuleEditor();
    bool hasEditor();
    void setEditorVisibility(bool isVisible);
    void deleteModuleEditor();

    int deleteEntireModule();

    /*
        ModuleInfo struct holds the info that needs to be saved
            - provides easy access for the processor and editor.
            - BUT, prefer to access these via setters and getters!!
            - Although, you can access them directly, doing that doesn't ensure they will be saved, among doing other tasks that might need to be done
    */
    struct ModuleInfo
    {
        juce::Colour moduleColor{ juce::Colours::blue };
        juce::File audioFile;

        //These rely on being intialized like this to see if we need to display the module differently, i.e. ModuleSettingsOverlay
        int midiNote = 0;
        int midiChannel = 0;
        juce::String name;
        int samplerIndex;
        int displayIndex;
        //bool moduleActive = false;
        ModuleState moduleState = ModuleState::empty;
        
        std::atomic<bool> modulePlaying = false;
        //Is not used yet, but is here as a flag to be used with the draggingHandle being used. Will be impelemented soon!
        // 
        bool moduleDragging = false;

    };

    ModuleInfo info;

private:

    bool needsToUpdateTree = false;

    friend class KrumModuleEditor;
    friend class KrumModuleProcessor;
    friend class DragAndDropThumbnail;

    //std::shared_ptr<KrumModuleProcessor> moduleProcessor;
    std::shared_ptr<KrumModuleEditor> moduleEditor = nullptr;

    juce::AudioProcessorValueTreeState* parameters = nullptr;
    juce::ValueTree* valueTree = nullptr;

    juce::String getIndexString();

    //From Processor
    KrumSampler& sampler;

    std::atomic<float>* moduleGain = nullptr;
    std::atomic<float>* modulePan = nullptr;
    std::atomic<float>* moduleClipGain = nullptr;

    float buttonClickVelocity = 0.5f;
    
    JUCE_LEAK_DETECTOR(KrumModule)

};
