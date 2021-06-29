/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KrumSampler.h"
#include "MidiState.h"

//==============================================================================
/**
*/

namespace TreeIDs 
{
    //Globals
    static const int maxNumModules = {30};
    static const float defaultGain = 0.85f;
    static const float defaultPan = 0.5f;
    static const int defaultOutput = 1;
    static juce::String outputGainParam_ID{"outputGain"};

    //APVTS
    static juce::String paramModuleGain_ID{"moduleGain"};
    static juce::String paramModulePan_ID{"modulePan"};
    static juce::String paramModuleOutputChannels_ID{"moduleOutputChannel"};

    //ValueTree
    static juce::String paramModuleFile_ID{"moduleFilePath"};
    static juce::String paramModuleMidiNote_ID{"moduleMidiNote"};
    static juce::String paramModuleMidiChannel_ID{"moduleMidiChannel"};
    static juce::String paramModuleActive_ID{ "moduleActive" };
    static juce::String paramModuleColor_ID{ "moduleColor" };
    static juce::String paramModuleDisplayIndex_ID{ "moduleDisplayIndex" };
}

static float panRangeTo0to1(juce::String text)
{
    juce::NormalisableRange<float>range{ -100.0f, 100.0f, 0.01f };
    //if (text[0] == (wchar_t)"<")
    if(text.startsWithChar('<'))
    {
        text.removeCharacters("< ");
    }

    if (text.endsWithChar('>'))
    {
        text.dropLastCharacters(2);
    }

    return range.convertTo0to1(text.getFloatValue());
}
static juce::String panRangeFrom0To1(float value)
{
    juce::NormalisableRange<float>range{ -100.0f, 100.0f, 0.01f };
    int converted = (int)range.convertFrom0to1(value);
    juce::String returnString{};
    if (converted > 0)
    {
        returnString = juce::String(converted) + " >";
    }
    else if (converted < 0)
    {
        returnString = "< " + juce::String(converted * (-1));
    }
    else
    {
        returnString = "< 0 >";
    }
    return returnString;
}


//class KrumSamplerAudioProcessorEditor;

class KrumSamplerAudioProcessor  :  public juce::AudioProcessor/*,
                                    public juce::Thread*/
{
public:

    //==============================================================================
    KrumSamplerAudioProcessor();
    ~KrumSamplerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processMidiKeyStateBlock(juce::MidiBuffer& midiMessages, int startSample, int numSamples, bool injectDirectEvents);
    void addMidiKeyboardListener(juce::MidiKeyboardStateListener*);
    void removeMidiKeyboardListener(juce::MidiKeyboardStateListener*);


    bool getPostMessage();
    void setPostMessage(bool messageState);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioFormatManager* getFormatManager();
    juce::ValueTree* getValueTree();
    juce::MidiKeyboardState& getMidiState();

    void makeModulesFromValueTree();
    void updateValueTreeState();
    int findFreeModuleIndex();


    int getNumModulesInSampler()
    {
        return sampler.getNumModules();
    }
    //juce::AudioProcessorValueTreeState getParameterTree();
    /*void addModuleGroupToTree(KrumModule& mod, int index);
    void removeModuleGroupFromTree(juce::String name, int index);*/
    
    

private:

    juce::CriticalSection critSection;

    juce::ValueTree valueTree{"AppState"};
    juce::AudioProcessorValueTreeState parameters; 
    juce::ValueTree fileBrowserValueTree{ "FileBrowserTree" }; 

    std::atomic<float>* outputGainParameter = nullptr;
    //MidiState midiState;
    juce::MidiKeyboardState midiState;

    juce::AudioFormatManager formatManager;
    KrumSampler sampler{ formatManager, *this };
    
    //KrumModuleContainer* moduleContainer = nullptr;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessor)
};
