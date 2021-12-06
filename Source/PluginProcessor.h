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

#define GUI_REFRESH_RATE_HZ const int 30
#define MAX_NUM_MODULES 20
#define MAX_VOICES 15
#define MAX_FILE_LENGTH_SECS 3
#define KRUM_BUILD_VERSION "1.1.0-Beta" //Module Re-work


namespace TreeIDs
{
#define DECLARE_ID(name) const juce::Identifier name(#name);

    DECLARE_ID(APPSTATE)

        DECLARE_ID(GLOBALSETTINGS) //GlobaalSettings tree

            DECLARE_ID(previewerGain)
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
         
        DECLARE_ID(PARAMS) //APVTS
            
            DECLARE_ID(paramModuleGain)
            DECLARE_ID(paramModulePan)
            DECLARE_ID(paramModuleOutputChannel)
            DECLARE_ID(paramModuleClipGain)
        
        DECLARE_ID(FILEBROWSERTREE) //File Browser Tree    

            DECLARE_ID(RECENT)
            DECLARE_ID(FAVORITES)

                DECLARE_ID(Folder)
                DECLARE_ID(File)

            DECLARE_ID(OPENSTATE)

#undef DECLARE_ID

    //Should I make these all preprocessor define instead?
    //Globals
    static const float defaultGain = 0.85f;
    static const float defaultPan = 0.5f;
    static const int defaultOutput = 1;
    static juce::String outputGainParam_ID{"outputGain"};

    //APVTS
   /* const static juce::String paramModuleGain_ID{"moduleGain"};
    const static juce::String paramModulePan_ID{"modulePan"};
    const static juce::String paramModuleOutputChannels_ID{"moduleOutputChannel"};
    const static juce::String paramModuleClipGain_ID{ "moduleClipGain" };*/


    //ValueTree Nodes
    //const static juce::String APPSTATE{ "AppState" };
//    const static juce::String GLOBALSETTINGS{ "GlobalSettings" };
 //   const static juce::String KRUMMODULES{ "KrumModules" };
 //   const static juce::String MODULE{ "Module" };
    
    //ValueTree - module's state parameters
    //const static juce::String moduleName_ID{ "name" };
    //const static juce::String moduleState_ID{"moduleState"};
    //const static juce::String moduleFile_ID{"moduleFilePath"};
    //const static juce::String moduleMidiNote_ID{"moduleMidiNote"};
    //const static juce::String moduleMidiChannel_ID{"moduleMidiChannel"};
    //const static juce::String moduleColor_ID{ "moduleColor" };
    //const static juce::String moduleDisplayIndex_ID{ "moduleDisplayIndex" };
    //const static juce::String moduleSamplerIndex_ID{ "moduleSamplerIndex" };

    //ValueTree - global settings
    /*const static juce::String previewerGain_ID {"PreviewerGain"};
    const static juce::String previewerAutoPlay_ID{ "PreviewerAutoPlay" };
    const static juce::String fileBrowserHidden_ID{ "BrowserHidden" };
    const static juce::String infoPanelToggle_ID{ "InfoPanelToggle" };*/

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
    void processMidiKeyStateBlock(juce::MidiBuffer& midiMessages, int startSample, int numSamples, bool injectDirectEvents);
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
    //void updateValueTreeState();
    
    //int findFreeModuleIndex();
    int getNumModulesInSampler();

    juce::AudioThumbnailCache& getThumbnailCache();
    KrumFileBrowser& getFileBrowser();

private:
    
    void registerFormats();

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
  
    KrumSampler sampler{ &valueTree, &parameters, formatManager.get(), *this };
    SimpleAudioPreviewer previewer{formatManager, valueTree};
    KrumFileBrowser fileBrowser{previewer, fileBrowserValueTree};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessor)
};
