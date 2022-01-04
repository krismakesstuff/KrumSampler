/*
  ==============================================================================

    KrumModule.cpp
    Created: 1 Mar 2021 11:15:42am
    Author:  krisc

  ==============================================================================
*/

#include "KrumModule.h"
#include "PluginEditor.h"

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
    if (treeWhoChanged.hasType(TreeIDs::MODULE) &&
        ((int)treeWhoChanged.getProperty(TreeIDs::moduleSamplerIndex) == getModuleSamplerIndex())) //check to make sure the module that changed is the same as this one
    {
        if (property == TreeIDs::moduleFile && treeWhoChanged[property].toString().isNotEmpty())
        {
            //update module file in sampler
            updateSamplerSound();
        }
        else if (property == TreeIDs::moduleMidiNote && (int)treeWhoChanged[property] > 0)
        {
            // update module sound in sampler
            updateSamplerSound();
        }
        else if (property == TreeIDs::moduleMidiChannel && (int)treeWhoChanged[property] > 0)
        {
            //update module sound in sampler
            updateSamplerSound();
        }
        else if (property == TreeIDs::moduleState && (int)treeWhoChanged[property] == KrumModule::ModuleState::empty)
        {
            removeSamplerSound();
        }
    }
}

KrumModule::ModuleState KrumModule::getModuleState()
{
    return static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState));
}

juce::File KrumModule::getSampleFile()
{
    return juce::File(moduleTree.getProperty(TreeIDs::moduleFile).toString());
}

int KrumModule::getMidiTriggerNote()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiNote);
}

int KrumModule::getMidiTriggerChannel()
{
    return  moduleTree.getProperty(TreeIDs::moduleMidiChannel);
}

juce::String KrumModule::getModuleName()
{
    return moduleTree.getProperty(TreeIDs::moduleName).toString();
}

bool KrumModule::isModuleActive()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState) == ModuleState::active;
}

bool KrumModule::isModuleEmpty()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState) == ModuleState::empty;
}

bool KrumModule::isModuleActiveOrHasFile()
{
    return (int)moduleTree.getProperty(TreeIDs::moduleState) > 0;
}

int KrumModule::getModuleSamplerIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleSamplerIndex);
}

int KrumModule::getModuleDisplayIndex() 
{
    return moduleTree.getProperty(TreeIDs::moduleDisplayIndex);
}

juce::Colour KrumModule::getModuleColor() 
{
    return juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor).toString());
}


std::atomic<float>* KrumModule::getModuleGain()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleGain + getIndexString());
}

std::atomic<float>* KrumModule::getModuleClipGain()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleClipGain + getIndexString());
}

std::atomic<float>* KrumModule::getModulePan()
{
    return parameters->getRawParameterValue(TreeIDs::paramModulePan + getIndexString());
}

std::atomic<float>* KrumModule::getModuleOutputChannel()
{
    return parameters->getRawParameterValue(TreeIDs::paramModuleOutputChannel + getIndexString());
}

std::atomic<int> KrumModule::getModuleStartSample()
{
    return moduleTree.getProperty(TreeIDs::moduleStartSample);
}

std::atomic<int> KrumModule::getModuleEndSample()
{
    return moduleTree.getProperty(TreeIDs::moduleEndSample);
}

int KrumModule::getModuleOutputChannelNumber()
{
    auto outputString = TreeIDs::outputStrings.getReference((int)*getModuleOutputChannel());

    return outputString.dropLastCharacters(2).getIntValue();
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
