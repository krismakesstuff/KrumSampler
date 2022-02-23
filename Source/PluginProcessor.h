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
* In PluginProcessoer.cpp, there are functions defined to create the AudioProcessorValueTreeState. This is defined by JUCE, and is reffered to as "APVTS". This ValueTree connects to your
* sliders to parameters. 
* 
* There is really only one large ValueTree that holds the state and settings of the app. That Tree then has other sub-trees for specific sections of the app. 
* When loading and saving the ValueTree, the children trees append or peel off in their respective contexts. see getStateInformation() and setStateInformation() for implementation. 
* 
* 
*/

#define GUI_REFRESH_RATE_HZ const int 30
#define MAX_NUM_MODULES 20
#define MAX_VOICES 14
#define NUM_PREVIEW_VOICES 1
#define MAX_FILE_LENGTH_SECS 3
#define NUM_AUX_OUTS 20                     //mono channels
#define SAVE_RELOAD_STATE 1                 //quick way to enable and disable getStateInfo() and setStateInfo()
#define KRUM_BUILD_VERSION "1.4.0-Beta"     //


//The general Tree structure
namespace TreeIDs
{
#define DECLARE_ID(name) const juce::Identifier name(#name);

    DECLARE_ID(APPSTATE)

        DECLARE_ID(GLOBALSETTINGS) //GlobalSettings tree

            DECLARE_ID(previewerAutoPlay)
            DECLARE_ID(fileBrowserHidden)
            DECLARE_ID(infoPanelToggle)

        DECLARE_ID(KRUMMODULES) //Module Tree

            DECLARE_ID(MODULE)

                DECLARE_ID(moduleName)
                DECLARE_ID(moduleState)
                DECLARE_ID(moduleFile)
                DECLARE_ID(moduleMidiNote)
                DECLARE_ID(moduleMidiChannel)
                DECLARE_ID(moduleColor)
                DECLARE_ID(moduleDisplayIndex)
                DECLARE_ID(moduleSamplerIndex)
                DECLARE_ID(moduleStartSample)
                DECLARE_ID(moduleEndSample)
                DECLARE_ID(moduleNumSamplesLength)
           /*     DECLARE_ID(moduleFadeIn)
                DECLARE_ID(moduleFadeOut) */

        DECLARE_ID(PARAMS) //APVTS, these are parameters exposed to the DAW for Automation
            
            DECLARE_ID(paramModuleGain)
            DECLARE_ID(paramModulePan)
            DECLARE_ID(paramModuleOutputChannel)
            DECLARE_ID(paramModuleClipGain)
            DECLARE_ID(paramModulePitchShift)
            DECLARE_ID(paramModuleReverse)
            DECLARE_ID(paramModuleMute)

            DECLARE_ID(outputGainParam)
            DECLARE_ID(previewerGainParam)

        DECLARE_ID(FILEBROWSERTREE) //File Browser Tree    

            DECLARE_ID(RECENT)
            DECLARE_ID(FAVORITES)

                DECLARE_ID(Folder)
                DECLARE_ID(File)

            DECLARE_ID(OPENSTATE)

#undef DECLARE_ID

    //Should I make these all preprocessor define instead?
    //These should be in their own namespace atleast
    static const float defaultGain = 0.85f;
    static const float defaultPan = 0.5f;
    static const int defaultOutput = 1;
    static const juce::StringArray outputStrings{ "1-2", "3-4", "5-6", "7-8", "9-10", "11-12", "13-14", "15-16", "17-18", "19-20" };


}

static float panRangeTo0to1(juce::String text)
{
    juce::NormalisableRange<float>range{ -100.0f, 100.0f, 0.01f };
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


class KrumSamplerAudioProcessor  :  public juce::AudioProcessor
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
    //void processMidiKeyStateBlock(juce::MidiBuffer& midiMessages, int startSample, int numSamples, bool injectDirectEvents);
    void addMidiKeyboardListener(juce::MidiKeyboardStateListener*);
    void removeMidiKeyboardListener(juce::MidiKeyboardStateListener*);

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

    void updateModulesFromValueTree();

    int getNumModulesInSampler();

    juce::AudioThumbnailCache& getThumbnailCache();
    KrumFileBrowser& getFileBrowser();

private:
    
    void registerFormats();
    void initSampler();

    juce::ValueTree valueTree{"AppState"};
    juce::AudioProcessorValueTreeState parameters; 
    juce::ValueTree fileBrowserValueTree{ "FileBrowserTree" }; 

    std::atomic<float>* outputGainParameter = nullptr;
    juce::MidiKeyboardState midiState;


    class ThumbnailCache : public juce::AudioThumbnailCache
    {
    public:
        ThumbnailCache()
            :juce::AudioThumbnailCache(MAX_NUM_MODULES)
        {}
        ~ThumbnailCache() override {}
    };

    juce::SharedResourcePointer<ThumbnailCache> thumbnailCache;
    
    juce::SharedResourcePointer <juce::AudioFormatManager> formatManager;
  
    SimpleAudioPreviewer previewer{formatManager, valueTree, parameters};
    KrumSampler sampler{ &valueTree, &parameters, formatManager.get(), *this, previewer };
    KrumFileBrowser fileBrowser{previewer, fileBrowserValueTree};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessor)
};
