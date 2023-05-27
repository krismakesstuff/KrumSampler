/*
  ==============================================================================

    KrumModule.h
    Created: 20 Feb 2021 1:53:52pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "UI/KrumModule/KrumModuleEditor.h" 


/*
* 
* KrumModule represents the module channel strip. The GUI conterpart of this is the KrumModuleEditor. Although, they reference the same ValueTree the do not talk directly to each other.
* They are seperately owned, KrumModule is owned by KrumSampler and KrumModuleEditor is owned by KrumModuleContainer.
* 
* The two classes are independently responsible for responding to tree changes. 
* 
* 
* TODO:
* 
* 
*/


class KrumSampler;

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
        //numerical values are used in the value tree saving
        empty,      //0
        hasFile,    //1
        active,     //2
    };

    
    KrumModule(KrumSampler& km, juce::ValueTree& valTree, juce::AudioProcessorValueTreeState* apvts);

    ~KrumModule();

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    ModuleState getModuleState();
    juce::File getSampleFile();
    int getMidiTriggerNote();
    int getMidiTriggerChannel();
    juce::String getModuleName();
    int getModuleSamplerIndex();
    int getModuleDisplayIndex();
    juce::Colour getModuleColor();

    bool isModuleActive();
    bool isModuleEmpty();
    bool isModuleActiveOrHasFile();    
    //bool isModuleMuted();
    //bool isModuleReversed();

    std::atomic<float>* getModuleGain();
    std::atomic<float>* getModuleClipGain();
    std::atomic<float>* getModulePan();
    std::atomic<float>* getModuleOutputChannel();
    std::atomic<int> getModuleStartSample();
    std::atomic<int> getModuleEndSample();
    std::atomic<float>* getModuleMute();
    std::atomic<float>* getModuleReverse();
    std::atomic<float>* getModulePitchShift();

    int getModuleOutputChannelNumber();

    void setNumSamplesInFile(int numSamples);

private:

    void updateSamplerSound();
    void removeSamplerSound();
    juce::String getIndexString();

    bool needsToUpdateTree = false;

    friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;

    juce::AudioProcessorValueTreeState* parameters = nullptr;
    juce::ValueTree moduleTree;
    
    KrumSampler& sampler;

    JUCE_LEAK_DETECTOR(KrumModule)

};
