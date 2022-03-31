/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//#include "Log.h"

//==============================================================================


float dBToGain(float gainValue)
{
    return juce::Decibels::decibelsToGain<float>(gainValue);
}

float gainToDB(float decibels)
{
    return juce::Decibels::gainToDecibels<float>(decibels);
}

//creates a blank ValueTree
juce::ValueTree createValueTree()
{
    juce::ValueTree appStateValueTree{ TreeIDs::APPSTATE };
    
    //---------------- Global Settings ----------------------------

    juce::ValueTree globalSettingsTree{ TreeIDs::GLOBALSETTINGS };

    globalSettingsTree.setProperty(TreeIDs::previewerAutoPlay, juce::var(0), nullptr);
    globalSettingsTree.setProperty(TreeIDs::fileBrowserHidden, juce::var(0), nullptr);
    globalSettingsTree.setProperty(TreeIDs::infoPanelToggle, juce::var(1), nullptr);

    appStateValueTree.addChild(globalSettingsTree, -1, nullptr);
    
    //----------------- Module Settings ---------------------------

    juce::ValueTree krumModulesTree{ TreeIDs::KRUMMODULES };
    
    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        juce::String index = juce::String(i);

        juce::ValueTree newModule { TreeIDs::MODULE, {{TreeIDs::moduleName,""}},{} }; 
        newModule.setProperty(TreeIDs::moduleState, juce::var(0), nullptr);
        newModule.setProperty(TreeIDs::moduleFile, juce::var(""), nullptr);
        newModule.setProperty(TreeIDs::moduleMidiNote, juce::var(-1), nullptr);
        newModule.setProperty(TreeIDs::moduleMidiChannel, juce::var(0), nullptr);
        newModule.setProperty(TreeIDs::moduleColor, juce::var(""), nullptr);
        newModule.setProperty(TreeIDs::moduleDisplayIndex, juce::var(-1), nullptr);
        newModule.setProperty(TreeIDs::moduleSamplerIndex, juce::var(i), nullptr);
        newModule.setProperty(TreeIDs::moduleStartSample, juce::var(0), nullptr);
        newModule.setProperty(TreeIDs::moduleEndSample, juce::var(0), nullptr);
        newModule.setProperty(TreeIDs::moduleNumSamplesLength, juce::var(0), nullptr);
        /*newModule.setProperty(TreeIDs::moduleFadeIn, juce::var(0), nullptr);
        newModule.setProperty(TreeIDs::moduleFadeOut, juce::var(0), nullptr);*/

        krumModulesTree.addChild(newModule, i, nullptr);
    }

    //--------------------------------------------

    appStateValueTree.addChild(krumModulesTree, -1, nullptr);
    
    //--------------------------------------------

    return appStateValueTree.createCopy();
}

juce::ValueTree createFileBrowserTree()
{
    juce::ValueTree retValTree{ TreeIDs::FILEBROWSERTREE, {}, };
    juce::ValueTree recTree = { TreeIDs::RECENT ,{} };
    juce::ValueTree favTree = { TreeIDs::FAVORITES , {} };
    juce::ValueTree locationsTree = { TreeIDs::LOCATIONS , {} };
    juce::ValueTree openTree = { TreeIDs::OPENSTATE, {}};

    retValTree.addChild(recTree, -1, nullptr);
    retValTree.addChild(favTree, -1, nullptr);
    retValTree.addChild(locationsTree, -1, nullptr);

    juce::File defaultLocation{ juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDesktopDirectory) };

    locationsTree.setProperty(TreeIDs::locationName, defaultLocation.getFileName(), nullptr);
    locationsTree.setProperty(TreeIDs::locationPath, defaultLocation.getFullPathName(), nullptr);

    retValTree.addChild(openTree, -1, nullptr);

    return retValTree.createCopy();

}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> paramsGroup;

    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        juce::String index = juce::String(i);

        juce::NormalisableRange<float> gainRange { dBToGain(-50.0f), dBToGain(2.0f), 0.0001f};
        //gainRange.setSkewForCentre(dBToGain(0.0f));
        gainRange.symmetricSkew = true;

        TreeIDs::gainRange = gainRange;

        juce::NormalisableRange<float> clipGainRange{ dBToGain(-30.0f), dBToGain(30.0f), 0.01f };
        clipGainRange.setSkewForCentre(dBToGain(0.0f));
        //clipGainRange.symmetricSkew = true;

        juce::NormalisableRange<float> pitchShiftRange{ -12, 12, 0.5f };
        pitchShiftRange.setSkewForCentre(0);
        pitchShiftRange.symmetricSkew = true;


        auto gainParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModuleGain + index, "Module Gain" + index,
                            gainRange, dBToGain(0.0f),
                            "Module" + index + " Gain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });
        
        auto clipGainParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModuleClipGain + index, "Module ClipGain" + index,
                            clipGainRange, dBToGain(0.0f),
                            "Module" + index + " ClipGain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto panParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModulePan + index, "Module Pan" + index,
                            juce::NormalisableRange<float>{0.01f, 1.0f, 0.001f}, 0.5f,
                            "Module" + index + " Pan",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return panRangeFrom0To1(value); },
                            [](juce::String text) {return panRangeTo0to1(text); });

        auto outputParam = std::make_unique<juce::AudioParameterChoice>(TreeIDs::paramModuleOutputChannel + index,
                            "Module " + index + " OuputChannel",
                            TreeIDs::outputStrings, 0);

        auto pitchParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModulePitchShift + index, "Module PitchShift" + index,
                            pitchShiftRange, 0,   
                            "Module " + index + " Pitch Shift",
                            juce::AudioProcessorParameter::genericParameter);

        auto reverseParam = std::make_unique<juce::AudioParameterBool>(TreeIDs::paramModuleReverse + index, "Module Reverse" + index,
                            false, "Module " + index + " Reverse");

        auto muteParam =    std::make_unique<juce::AudioParameterBool>(TreeIDs::paramModuleMute + index, "Module Mute" + index,
                            false, "Module " + index + " Mute");

        auto moduleGroup = std::make_unique<juce::AudioProcessorParameterGroup>("Module" + juce::String(i),
                            "Module" + juce::String(i),
                            "|",
                            std::move(gainParam),
                            std::move(clipGainParam),
                            std::move(panParam),
                            std::move(outputParam),
                            std::move(pitchParam),
                            std::move(reverseParam),
                            std::move(muteParam));

        paramsGroup.push_back(std::move(moduleGroup));
    }
    
    //================================== Global Gain(s) ==============================================================

    juce::NormalisableRange<float> outGainRange{ dBToGain(-60.0f), dBToGain(2.0f), 0.0001f };
    //outGainRange.setSkewForCentre(dBToGain(0.0f));
    outGainRange.symmetricSkew = true;

    auto outputGainParameter = std::make_unique<juce::AudioParameterFloat>(TreeIDs::outputGainParam.toString(), "Output Gain",
                            outGainRange, dBToGain(0.0f),
                            "OutputGain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

    auto previewerGainParameter = std::make_unique<juce::AudioParameterFloat>(TreeIDs::previewerGainParam.toString(), "Previewer Gain",
                            outGainRange, dBToGain(0.0f),
                            "PreviewerGain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });
  

    auto globalGroup = std::make_unique<juce::AudioProcessorParameterGroup>("Globals", "Global Parameters",
                            "|", std::move(outputGainParameter), std::move(previewerGainParameter));

    paramsGroup.push_back(std::move(globalGroup));

    return { paramsGroup.begin(), paramsGroup.end() };
}

//===================================================================================================================================


//===================================================================================================================================

KrumSamplerAudioProcessor::KrumSamplerAudioProcessor()
     : AudioProcessor( BusesProperties().withOutput("Output 1-2", juce::AudioChannelSet::stereo(), true)
                                        .withOutput("Output 3-4", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 5-6", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 7-8", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 9-10", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 11-12", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 13-14", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 15-16", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 17-18", juce::AudioChannelSet::stereo(), false)
                                        .withOutput("Output 19-20", juce::AudioChannelSet::stereo(), false)),
                                        parameters(*this, nullptr, TreeIDs::PARAMS, createParameterLayout())

{
    //juce::Logger::setCurrentLogger(Log::logger);
    //juce::Logger::writeToLog("--BUILD VERSION: " + juce::String(KRUM_BUILD_VERSION));
    valueTree = createValueTree();
    fileBrowserValueTree = createFileBrowserTree();
    registerFormats();
    
    //value is blank here
    initSampler();
    //previewer.assignSampler(&sampler);


    juce::Logger::writeToLog("----------------------------");
    juce::Logger::writeToLog("Sampler Processor Constructed");
    juce::Logger::writeToLog("Build Version: " + juce::String(KRUM_BUILD_VERSION));
    juce::Logger::writeToLog("Juce Build Version: " + juce::String(JUCE_BUILDNUMBER));
    juce::Logger::writeToLog("MaxNumModules: " + juce::String(MAX_NUM_MODULES));
    juce::Logger::writeToLog("MaxVoices: " + juce::String(MAX_VOICES));
    juce::Logger::writeToLog("MaxFileLengthInSeconds: " + juce::String(MAX_FILE_LENGTH_SECS));
    juce::Logger::writeToLog("----------------------------");

}

KrumSamplerAudioProcessor::~KrumSamplerAudioProcessor()
{

}

void KrumSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    outputGainParameter = parameters.getRawParameterValue(TreeIDs::outputGainParam);
    sampler.setCurrentPlaybackSampleRate(sampleRate);
    //juce::Logger::writeToLog("Processor prepared to play, sampleRate: " + juce::String(sampleRate) + ", samplesPerBlock: " +                      juce::String(samplesPerBlock));
}

void KrumSamplerAudioProcessor::releaseResources()
{
}

void KrumSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    midiState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    buffer.applyGain(*outputGainParameter);
    
    //this does not output midi, some hosts will freak out if you send them midi when you said you wouldn't
    midiMessages.clear();
}

void KrumSamplerAudioProcessor::addMidiKeyboardListener(juce::MidiKeyboardStateListener* newListener)
{
    midiState.addListener(newListener);
}

void KrumSamplerAudioProcessor::removeMidiKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove)
{
    midiState.removeListener(listenerToRemove);
}

//==============================================================================
void KrumSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
#if SAVE_RELOAD_STATE
    //we attach our tree to the main appStateTree
    valueTree.appendChild(parameters.state, nullptr);
    valueTree.appendChild(fileBrowserValueTree, nullptr);

    auto state = valueTree.createCopy();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);

    DBG("---SAVED STATE---");
    DBG(xml->toString());
#endif
}


void KrumSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
#if SAVE_RELOAD_STATE
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(valueTree.getType()))
        {
            //Audio parameters
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState->getChildByName(TreeIDs::PARAMS)));
            xmlState->removeChildElement(xmlState->getChildByName(TreeIDs::PARAMS), true);

            //File Browser
            auto xmlFileBrowserTree = xmlState->getChildByName(TreeIDs::FILEBROWSERTREE);
            if (xmlFileBrowserTree != nullptr)
            {
                fileBrowserValueTree.copyPropertiesAndChildrenFrom(juce::ValueTree::fromXml(*xmlFileBrowserTree), nullptr);
                xmlState->removeChildElement(xmlFileBrowserTree, true);

                DBG("File Browser Tree from Set State");
                DBG(fileBrowserValueTree.toXmlString());

                auto editor = static_cast<KrumSamplerAudioProcessorEditor*>(getActiveEditor());
                if (editor)
                {
                    auto fileBrowser = editor->getFileBrowser();
                    fileBrowser->rebuildBrowser(fileBrowserValueTree);
                    fileBrowser->buildDemoKit();
                    //fileBrowser.rebuildBrowser(fileBrowserValueTree);
                }
            }

            //Remaining App/Modules Settings
            valueTree.copyPropertiesAndChildrenFrom(juce::ValueTree::fromXml(*xmlState), nullptr);
            updateModulesFromValueTree();

            DBG("---SET STATE---");
            DBG(juce::ValueTree::fromXml(*xmlState).toXmlString());

        }
        else
        {
            DBG("XML has no tagname from ValueTree");
        }
    }
    else
    {
        DBG("XML state is null");
        DBG("Data Size = " + juce::String(sizeInBytes));
    }
#endif
}

juce::AudioFormatManager* KrumSamplerAudioProcessor::getFormatManager()
{
    return formatManager;
}

juce::ValueTree* KrumSamplerAudioProcessor::getValueTree()
{
    return &valueTree;
}

juce::MidiKeyboardState& KrumSamplerAudioProcessor::getMidiState()
{
    return midiState;
}

//updates the sampler based off the tree
void KrumSamplerAudioProcessor::updateModulesFromValueTree()
{
    initSampler();

    DBG("---- Updating Modules using this Tree vvvvv ----");
    DBG(valueTree.toXmlString());

    auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);
    
    for (int i = 0; i < modulesTree.getNumChildren(); i++)
    {
        auto moduleTree = modulesTree.getChild(i);
        int state = (int)moduleTree.getProperty(TreeIDs::moduleState);

        if (state > 0) 
        {            
            auto mod = sampler.getModule(moduleTree.getProperty(TreeIDs::moduleSamplerIndex));
            sampler.updateModuleSample(mod);
        }
        else
        {
            //DBG("ValueTree not Valid " + juce::String(i));
        }
    }
}

int KrumSamplerAudioProcessor::getNumModulesInSampler()
{
    return sampler.getNumModules();
}

juce::AudioThumbnailCache& KrumSamplerAudioProcessor::getThumbnailCache()
{
    return thumbnailCache.get();
}

//KrumFileBrowser& KrumSamplerAudioProcessor::getFileBrowser()
//{
//    return fileBrowser;
//}

//SimpleAudioPreviewer* KrumSamplerAudioProcessor::getAudioPreviewer()
//{
//    return &previewer;
//}

void KrumSamplerAudioProcessor::registerFormats()
{
    formatManager->registerBasicFormats();
}

void KrumSamplerAudioProcessor::initSampler()
{
    if (sampler.getNumModules() > 0)
    {
        sampler.clearModules();
    }

    sampler.initModules(&valueTree, &parameters);

}

//==============================================================================


#ifndef JucePlugin_PreferredChannelConfigurations
bool KrumSamplerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{

    for (const auto& bus : layout.outputBuses)
        if (bus != juce::AudioChannelSet::stereo())
            return false;

    return layout.inputBuses.isEmpty() && 1 <= layout.outputBuses.size();
}
#endif

bool KrumSamplerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* KrumSamplerAudioProcessor::createEditor()
{
    return new KrumSamplerAudioProcessorEditor(*this, sampler, parameters, valueTree, fileBrowserValueTree);
}

const juce::String KrumSamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KrumSamplerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool KrumSamplerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool KrumSamplerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double KrumSamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KrumSamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KrumSamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KrumSamplerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String KrumSamplerAudioProcessor::getProgramName(int index)
{
    return {};
}

void KrumSamplerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KrumSamplerAudioProcessor();
}
