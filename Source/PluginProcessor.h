/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KrumSampler.h"
#include "KrumFileBrowser.h"
#include "SimpleAudioPreviewer.h"



//==============================================================================
/*
* 
* A JUCE generated class that represents the audio engine of the app. This will handle all audio and midi calls to and from the DAW, as well as state changes on startup and exit. 
* 
* In PluginProcessoer.cpp, there function defined to create the AudioProcessorValueTreeState. This is defined by JUCE, and is reffered to as "APVTS". 
* 
* There is really only one large ValueTree that holds the state and settings of the app. That Tree then has other trees for specific sections of the app. 
* When loading and saving the ValueTree, it peels the children trees on and off in their respective contexts. see getStateInformation() and setStateInformation() for implementation. 
* 
* 
*/

#define MAX_NUM_MODULES 30
#define MAX_VOICES 8
#define MAX_FILE_LENGTH_SECS 5

namespace TreeIDs 
{
    //Should I make these all macros instead? 
    //Globals
    //static const int maxNumModules = {30};
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

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    void updateEditor();
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

    juce::AudioThumbnailCache& getThumbnailCache();

    KrumFileBrowser& getFileBrowser();


private:

    void registerFormats();

    juce::ValueTree valueTree{"AppState"};
    juce::AudioProcessorValueTreeState parameters; 
    juce::ValueTree fileBrowserValueTree{ "FileBrowserTree" }; 

    std::atomic<float>* outputGainParameter = nullptr;
    juce::MidiKeyboardState midiState;

    
    class FormatManager : public juce::AudioFormatManager
    {
    public:
        FormatManager()
        {
            registerBasicFormats();
        }
        ~FormatManager() {}
    };

    juce::SharedResourcePointer <juce::AudioFormatManager> formatManager;
    KrumSampler sampler{ formatManager.get(), *this };
    SimpleAudioPreviewer previewer{formatManager.get(), valueTree};
    KrumFileBrowser fileBrowser{previewer, fileBrowserValueTree/*, formatManager*/};

    class ThumbnailCache : public juce::AudioThumbnailCache
    {
    public:
        ThumbnailCache()
            :juce::AudioThumbnailCache(MAX_NUM_MODULES)
        {}
        ~ThumbnailCache() override {}
    };

    juce::SharedResourcePointer<ThumbnailCache> thumbnailCache;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessor)
};
