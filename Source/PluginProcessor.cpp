/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================


float dBToGain(float gainValue)
{
    return juce::Decibels::decibelsToGain<float>(gainValue);
}

float gainToDB(float decibels)
{
    return juce::Decibels::gainToDecibels<float>(decibels);
}

juce::ValueTree createValueTree()
{
    //add additional global settings here    
    juce::ValueTree retValueTree
    { "AppState" ,{},
        {
            {"GlobalSettings", {},
                {
                    {"PreviewerGain",       {{"value", "0.75"}}},
                    {"PreviewerAutoPlay",   {{"value", "0"}}},
                    {"BrowserHidden",       {{"value", "0"}}}
                }
            }
        }
    };

    juce::ValueTree krumModulesTree{ "KrumModules" };
    
    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        juce::String index = juce::String(i);
        juce::ValueTree newModule =
        { "Module" + index, {{"name",""}},
                {
                    {"State", {{"id", TreeIDs::paramModuleActive_ID},       {"value", ""}}},
                    {"State", {{"id", TreeIDs::paramModuleFile_ID},         {"value", ""}}},
                    {"State", {{"id", TreeIDs::paramModuleMidiNote_ID},     {"value", ""}}},
                    {"State", {{"id", TreeIDs::paramModuleMidiChannel_ID},  {"value", ""}}},
                    {"State", {{"id", TreeIDs::paramModuleColor_ID},        {"value", ""}}},
                    {"State", {{"id", TreeIDs::paramModuleDisplayIndex_ID}, {"value", ""}}}
                }
        };
        krumModulesTree.addChild(newModule, i, nullptr);
    }

    retValueTree.addChild(krumModulesTree, -1, nullptr);

    return retValueTree.createCopy();
}

juce::ValueTree createFileBrowserTree()
{
    juce::ValueTree retValTree{ "FileBrowserTree", {}, };
    juce::ValueTree recTree = { "Recent" ,{} };
    juce::ValueTree favTree = { "Favorites", {} };
    juce::ValueTree openTree = { "OpennessState", {} };

    retValTree.addChild(recTree, FileBrowserSectionIds::recentFolders_Ids, nullptr);
    retValTree.addChild(favTree, FileBrowserSectionIds::favoritesFolders_Ids, nullptr);
    retValTree.addChild(openTree, FileBrowserSectionIds::openness_Ids, nullptr);

    return retValTree.createCopy();

}


juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> paramsGroup;
    
    //Not currently using aux outputs but would like to add these soon
    //const juce::StringArray outputStrings {"1 & 2", "3 & 4", "5 & 6"};

    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        juce::String index = juce::String(i);

        juce::NormalisableRange<float> gainRange { dBToGain(-50.0f), dBToGain(2.0f), 0.0001f};
        gainRange.setSkewForCentre(dBToGain(0.0f));
        gainRange.symmetricSkew = true;

        juce::NormalisableRange<float> clipGainRange{ dBToGain(-30.0f), dBToGain(20.0f), 0.01f };
        clipGainRange.setSkewForCentre(dBToGain(0.0f));
        //clipGainRange.symmetricSkew = true;

        auto gainParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModuleGain_ID + index, "Module Gain" + index,
                            gainRange, dBToGain(0.0f),
                            "Module" + index + " Gain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });
        
        auto clipGainParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModuleClipGain_ID + index, "Module ClipGain" + index,
                            clipGainRange, dBToGain(0.0f),
                            "Module" + index + " ClipGain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto panParam = std::make_unique<juce::AudioParameterFloat>(TreeIDs::paramModulePan_ID + index, "Module Pan" + index,
                            juce::NormalisableRange<float>{0.01f, 1.0f, 0.001f}, 0.5f,
                            "Module" + index + " Pan",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return panRangeFrom0To1(value); },
                            [](juce::String text) {return panRangeTo0to1(text); });

        /*auto outputParam = std::make_unique<juce::AudioParameterChoice>(TreeIDs::paramModuleOutputChannels_ID + index,
                            "Module Ouput" + index,
                            outputStrings, 1);*/

        auto moduleGroup = std::make_unique<juce::AudioProcessorParameterGroup>("Module" + juce::String(i),
                            "Module" + juce::String(i),
                            "|",
                            std::move(gainParam),
                            std::move(clipGainParam),
                            std::move(panParam)
                          /*, std::move(outputParam)*/);

        paramsGroup.push_back(std::move(moduleGroup));
    }
    
    juce::NormalisableRange<float> outGainRange{ dBToGain(-60.0f), dBToGain(2.0f), 0.0001f };
    outGainRange.setSkewForCentre(dBToGain(0.0f));
    outGainRange.symmetricSkew = true;
    auto outputGainParameter = std::make_unique<juce::AudioParameterFloat>(TreeIDs::outputGainParam_ID, "Output Gain",
                            outGainRange, dBToGain(0.0f),
                            "OutputGain",
                            juce::AudioProcessorParameter::genericParameter,
                            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
                            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

    auto globalGroup = std::make_unique<juce::AudioProcessorParameterGroup>("Globals", "Global Parameters",
                            "|", std::move(outputGainParameter));


    paramsGroup.push_back(std::move(globalGroup));
    return { paramsGroup.begin(), paramsGroup.end() };
}

//===================================================================================================================================
//===================================================================================================================================

KrumSamplerAudioProcessor::KrumSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(  BusesProperties()
                        #if ! JucePlugin_IsMidiEffect
                        #if ! JucePlugin_IsSynth
                                    .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        #endif
                                    .withOutput("Output", juce::AudioChannelSet::stereo(), true))
                        #endif
                                    ,parameters(*this, nullptr, "PARAMS", createParameterLayout())
                        #endif
{
    valueTree = createValueTree();
    fileBrowserValueTree = createFileBrowserTree();
    registerFormats();

#if JucePlugin_Build_Standalone
    fileBrowser.buildDemoKit();
#endif
}

KrumSamplerAudioProcessor::~KrumSamplerAudioProcessor()
{
}


void KrumSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    outputGainParameter = parameters.getRawParameterValue(TreeIDs::outputGainParam_ID);
    sampler.setCurrentPlaybackSampleRate(sampleRate);
}

void KrumSamplerAudioProcessor::releaseResources()
{
}

void KrumSamplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    if (previewer.wantsToPlayFile())
    {
        previewer.renderPreviewer(buffer);
    }
    
    buffer.applyGain(*outputGainParameter);
    midiState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    
    //this does not output midi, some hosts will freak out if you send them midi when you said you wouldn't
    midiMessages.clear();
}

void KrumSamplerAudioProcessor::processMidiKeyStateBlock(juce::MidiBuffer& midiMessages, int startSample, int numSamples, bool injectDirectEvents)
{
    //midiState.processMidi(midiMessages, startSample, numSamples, injectDirectEvents);
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
#ifndef JucePlugin_PreferredChannelConfigurations
bool KrumSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

bool KrumSamplerAudioProcessor::hasEditor() const
{
    return true; 
}

//void KrumSamplerAudioProcessor::updateEditor()
//{
//    if (hasEditor())
//    {
//        auto editor = static_cast<KrumSamplerAudioProcessorEditor*>(getActiveEditor());
//        if (editor)
//        {
//            editor->updateEditor();
//            editorWantsToUpdate = false;
//        }
//    }
//}

juce::AudioProcessorEditor* KrumSamplerAudioProcessor::createEditor()
{
    return new KrumSamplerAudioProcessorEditor(*this, sampler, parameters, valueTree, fileBrowserValueTree);
}

//=======================================================================================//
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


//==============================================================================
void KrumSamplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

    valueTree.appendChild(parameters.state, nullptr);
    valueTree.appendChild(fileBrowserValueTree, nullptr);
    
    auto state = valueTree.createCopy();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);

    DBG("---SAVED STATE---");
    DBG(xml->toString());

}

void KrumSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(valueTree.getType()))
        {
            //Audio parameters
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState->getChildByName("PARAMS")));
            xmlState->removeChildElement(xmlState->getChildByName("PARAMS"), true);
            
            //File Browser 
            auto xmlFileBrowserTree = xmlState->getChildByName("FileBrowserTree");
            if (xmlFileBrowserTree != nullptr)
            {
                fileBrowserValueTree.copyPropertiesAndChildrenFrom(juce::ValueTree::fromXml(*xmlFileBrowserTree), nullptr);
                xmlState->removeChildElement(xmlFileBrowserTree, true);
                fileBrowser.rebuildBrowser(fileBrowserValueTree);
            }
            
            //Remaining App/Modules Settings
            valueTree.copyPropertiesAndChildrenFrom(juce::ValueTree::fromXml(*xmlState), nullptr); 
            makeModulesFromValueTree();
            fileBrowser.getAudioPreviewer()->refreshSettings();
            
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

void KrumSamplerAudioProcessor::makeModulesFromValueTree()
{
    sampler.clearModules();
    auto modulesTree = valueTree.getChildWithName("KrumModules");
    
    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        auto moduleTree = modulesTree.getChildWithName("Module" + juce::String(i));
        if (moduleTree.isValid())
        {
            juce::var nameValue = moduleTree.getProperty("name");
            
            juce::ValueTree stateTree;
            bool moduleActive = false;
            int midiChannel = 0;
            juce::var id;
            juce::var val;

            for (int j = 0; j < 4; j++)                 
            {
                stateTree = moduleTree.getChild(j);
                id = stateTree.getProperty("id");
                val = stateTree.getProperty("value");
                //DBG(id.toString() + " " + val.toString());
                if (id.toString() == TreeIDs::paramModuleActive_ID && int(val) > 0)
                {
                    moduleActive = true;
                }
                else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID)
                {
                    midiChannel = int(val);
                }
            }

            if (moduleActive)
            {
                auto newModule = new KrumModule(i, sampler, &valueTree, &parameters);
                sampler.addModule(newModule, newModule->getMidiTriggerNote() != 0);
                midiState.addListener(newModule);
            }
        }
        else
        {
            DBG("ValueTree not Valid" + juce::String(i));
        }

    }

    /*DBG("Module Display:");
    for (int i = 0; i < editor->getModuleDisplayOrder().size(); i++)
    {
        DBG("Position " + juce::String(i) + " Module " + juce::String(editor->getModuleDisplayOrder().getUnchecked(i)));
    }
    DBG("Display Size: " + juce::String(editor->getModuleDisplayOrder().size()));*/

}

void KrumSamplerAudioProcessor::updateValueTreeState()
{
    auto modulesTree = valueTree.getChildWithName("KrumModules");

    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        auto moduleTree = modulesTree.getChildWithName("Module" + juce::String(i));
        auto mod = sampler.getModule(i);
        if (mod != nullptr)
        {
            for (int j = 0; j < moduleTree.getNumChildren(); j++)
            {
                auto stateTree = moduleTree.getChild(j);
                auto id = stateTree.getProperty("id");

                if (id.toString() == TreeIDs::paramModuleActive_ID)
                {
                    stateTree.setProperty("value", mod->isModuleActive() ? juce::var(1) : juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleFile_ID)
                {
                    stateTree.setProperty("value", juce::var(mod->getSampleFile().getFullPathName()), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiNote_ID)
                {
                    stateTree.setProperty("value", juce::var(mod->getMidiTriggerNote()), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID)
                {
                    stateTree.setProperty("value", juce::var(mod->getMidiTriggerChannel()), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleColor_ID)
                {
                    stateTree.setProperty("value", juce::var(mod->getModuleColor().toDisplayString(true)), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleDisplayIndex_ID)
                {
                    stateTree.setProperty("value", juce::var(mod->getModuleDisplayIndex()), nullptr);
                }
            }
        }
        else //zero the state for this module
        {
            
            auto gainParam = parameters.getParameter(TreeIDs::paramModuleGain_ID + juce::String(i));
            auto clipGainParam = parameters.getParameter(TreeIDs::paramModuleClipGain_ID + juce::String(i));
            auto panParam = parameters.getParameter(TreeIDs::paramModulePan_ID + juce::String(i));

            auto zeroGain = gainParam->getNormalisableRange().convertTo0to1(dBToGain(0.0f));
            gainParam->setValueNotifyingHost(zeroGain);
            clipGainParam->setValueNotifyingHost(zeroGain);
            panParam->setValueNotifyingHost(0.5f);

            moduleTree.setProperty("name", juce::var(""), nullptr);

            for (int j = 0; j < moduleTree.getNumChildren(); j++)
            {
                auto stateTree = moduleTree.getChild(j);
                auto id = stateTree.getProperty("id");

                if (id.toString() == TreeIDs::paramModuleActive_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleFile_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiNote_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleColor_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleDisplayIndex_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
            }
        }
    }

    DBG("---Updated Tree State---");
    auto state = valueTree.createCopy();
    std::unique_ptr<juce::XmlElement> xml = state.createXml();
    DBG(xml->toString());

}


int KrumSamplerAudioProcessor::findFreeModuleIndex()
{
    auto modulesTree = valueTree.getChildWithName("KrumModules");

    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        auto moduleTree = modulesTree.getChildWithName("Module" + juce::String(i));
        
        juce::ValueTree stateTree;
        juce::var id;
        juce::var val;
        
        stateTree = moduleTree.getChild(0);             //index of ModuleActive parameter in ValueTree.
        id = stateTree.getProperty("id");
        val = stateTree.getProperty("value");
        if (id.toString() == TreeIDs::paramModuleActive_ID && int(val) == 0)
        {
            return i;
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

KrumFileBrowser& KrumSamplerAudioProcessor::getFileBrowser()
{
    return fileBrowser;
}

void KrumSamplerAudioProcessor::registerFormats()
{
    formatManager->registerBasicFormats();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KrumSamplerAudioProcessor();
}
