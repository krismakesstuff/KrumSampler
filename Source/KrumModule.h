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

//class KrumSampler;

class KrumModule :  /*public juce::Component,*/
                    /*public juce::Timer,*/
                    public juce::MidiKeyboardStateListener,
                    public juce::DragAndDropContainer
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

    enum ModuleStateIDs
    {
        active,
        inactive,
        blank,  //needs everything
        needSample,
        needMidi,
        needColor,
    };

    //KrumModule(int index, KrumSamplerAudioProcessorEditor& editor, KrumSampler& sampler);
   /* KrumModule( juce::String& moduleName,
                int index, KrumSamplerAudioProcessorEditor& parent,
                juce::File file, int midiNoteNumber,
                int midiChannelNumber, juce::Colour color,
                juce::AudioFormatManager& fm, KrumSampler& km,
                juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);*/

    KrumModule(juce::String& moduleName, int index, juce::File file, KrumSampler& km,
        juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);

    KrumModule(int index, KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);

    ~KrumModule() override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity) override;

    
    void setModuleSelected(bool isModuleSelected);
    void removeSettingsOverlay(bool keepSettings = true);

    juce::File& getSampleFile();
    void setSampleFile(juce::File& newSample);

    int getMidiTriggerNote();
    void setMidiTriggerNote(int midiNoteNumber);

    int getMidiTriggerChannel();
    void setMidiTriggerChannel(int newMidiChannel);

    juce::String& getModuleName();
    void setModuleName(juce::String& newName);

    bool isModulePlaying();
    void setModulePlaying(const bool isPlaying);

    bool isModuleActive();
    void setModuleActive(bool isActive);

    int getModuleIndex();
    void setModuleIndex(int newIndex);

    int getModuleDisplayIndex();
    void setModuleDisplayIndex(int newIndex);

    juce::Colour getModuleColor();
    void setModuleColor(juce::Colour newModuleColor, bool refreshChildren = true);

    void triggerNoteOn();
    void triggerNoteOff();

    void setModuleGain(float newGain);
    std::atomic<float>* getModuleGain();

    void setModulePan(float newPan);
    std::atomic<float>* getModulePan();

    void getValuesFromTree();
    void updateValuesInTree(bool printBefore = false);
    void clearModuleValueTree();

    void reassignSliders();

    KrumModuleEditor* createModuleEditor(KrumSamplerAudioProcessorEditor& editor);
    KrumModuleEditor* getCurrentModuleEditor();
    bool hasEditor();

    void setEditorVisibility(bool isVisible);

    void deleteModuleEditor();

    int deleteEntireModule();


    struct ModuleInfo
    {
        juce::Colour moduleColor{ juce::Colours::blue };
        juce::File audioFile;

        int midiNote = 0;
        int midiChannel = 0;
        juce::String name;
        int index;
        int displayIndex;

        std::atomic<bool> modulePlaying = false;
        bool moduleWantsToPlay = false;
        bool moduleActive = false;
        bool moduleDragging = false;

    };

    ModuleInfo info;

private:

    //KrumSampler& sampler;
    //friend class KrumSamplerAudioProcessorEditor;
    friend class KrumModuleEditor;
    friend class KrumModuleProcessor;

    std::shared_ptr<KrumModuleProcessor> moduleProcessor;
    std::shared_ptr<KrumModuleEditor> moduleEditor = nullptr;

    juce::AudioProcessorValueTreeState* parameters = nullptr;
    juce::ValueTree* valueTree = nullptr;
    //KrumModuleContainer* parent = nullptr;

    JUCE_LEAK_DETECTOR(KrumModule)

};