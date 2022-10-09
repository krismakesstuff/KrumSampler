/*
  ==============================================================================

    KrumModule.cpp
    Created: 1 Mar 2021 11:15:42am
    Author:  krisc

  ==============================================================================
*/

#include "KrumModule.h"
#include "UI/PluginEditor.h"

KrumModule::KrumModule(KrumSampler& km, juce::ValueTree& valTree, juce::AudioProcessorValueTreeState* apvts)
    : moduleTree(valTree), parameters(apvts), sampler(km)
{
    moduleTree.addListener(this);
}

KrumModule::~KrumModule()
{
}

void KrumModule::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if (treeWhoChanged.hasType(juce::Identifier(TreeIDs::MODULE.getParamID())) &&
        ((int)treeWhoChanged.getProperty(TreeIDs::moduleSamplerIndex.getParamID()) == getModuleSamplerIndex())) //check to make sure the module that changed is the same as this one
    {
        if (property == juce::Identifier(TreeIDs::moduleFile.getParamID()) && treeWhoChanged[property].toString().isNotEmpty())
        {
            //update module file in sampler
            updateSamplerSound();
        }
        else if (property == juce::Identifier(TreeIDs::moduleMidiNote.getParamID()) && (int)treeWhoChanged[property] > 0)
        {
            // update module sound in sampler
            updateSamplerSound();
        }
        else if (property == juce::Identifier(TreeIDs::moduleMidiChannel.getParamID()) && (int)treeWhoChanged[property] > 0)
        {
            //update module sound in sampler
            updateSamplerSound();
        }
        else if (property == juce::Identifier(TreeIDs::moduleState.getParamID()) && (int)treeWhoChanged[property] == KrumModule::ModuleState::empty)
        {
            removeSamplerSound();
        }
    }
}

KrumModule::ModuleState KrumModule::getModuleState()
{
    return static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()));
}

juce::File KrumModule::getSampleFile()
{
    return juce::File(moduleTree.getProperty(TreeIDs::moduleFile.getParamID()).toString());
}

int KrumModule::getMidiTriggerNote()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiNote.getParamID());
}

int KrumModule::getMidiTriggerChannel()
{
    return  moduleTree.getProperty(TreeIDs::moduleMidiChannel.getParamID());
}

juce::String KrumModule::getModuleName()
{
    return moduleTree.getProperty(TreeIDs::moduleName.getParamID()).toString();
}

bool KrumModule::isModuleActive()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()) == ModuleState::active;
}
    
bool KrumModule::isModuleEmpty()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()) == ModuleState::empty;
}

bool KrumModule::isModuleActiveOrHasFile()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()) > 0;
}

int KrumModule::getModuleSamplerIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleSamplerIndex.getParamID());
}

int KrumModule::getModuleDisplayIndex() 
{
    return moduleTree.getProperty(TreeIDs::moduleDisplayIndex.getParamID());
}

juce::Colour KrumModule::getModuleColor() 
{
    return juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor.getParamID()).toString());
}


std::atomic<float>* KrumModule::getModuleGain()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleGain.getParamID() + getIndexString());
}

std::atomic<float>* KrumModule::getModuleClipGain()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleClipGain.getParamID() + getIndexString());
}

std::atomic<float>* KrumModule::getModulePan()
{
    return parameters->getRawParameterValue(TreeIDs::paramModulePan.getParamID() + getIndexString());
}

std::atomic<float>* KrumModule::getModuleOutputChannel()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleOutputChannel.getParamID() + getIndexString());
}

std::atomic<int> KrumModule::getModuleStartSample()
{
    int startSample = (int)moduleTree.getProperty(TreeIDs::moduleStartSample.getParamID());
    return startSample;
}
    
std::atomic<int> KrumModule::getModuleEndSample()
{
    int endSample = (int)moduleTree.getProperty(TreeIDs::moduleEndSample.getParamID());
    return endSample;
    
}

std::atomic<float>* KrumModule::getModuleMute()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleMute.getParamID() + getIndexString());
}

std::atomic<float>* KrumModule::getModuleReverse()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleReverse.getParamID() + getIndexString());
}

std::atomic<float>* KrumModule::getModulePitchShift()
{
    return parameters->getRawParameterValue(TreeIDs::paramModulePitchShift.getParamID() + getIndexString());
}

int KrumModule::getModuleOutputChannelNumber()
{
    auto outputString = TreeIDs::outputStrings.getReference((int)*getModuleOutputChannel());

    return outputString.dropLastCharacters(2).getIntValue();
}

void KrumModule::setNumSamplesInFile(int numSamples)
{
    moduleTree.setProperty(TreeIDs::moduleNumSamplesLength.getParamID(), numSamples, nullptr);
}


void KrumModule::updateSamplerSound()
{
    sampler.updateModuleSample(this);
}

void KrumModule::removeSamplerSound()
{
    sampler.removeModuleSample(this);
}

juce::String KrumModule::getIndexString()
{
    return juce::String(getModuleSamplerIndex());
}
