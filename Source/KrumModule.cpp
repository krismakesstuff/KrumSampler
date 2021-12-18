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

juce::File& KrumModule::getSampleFile()
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

juce::String& KrumModule::getModuleName()
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
    //return moduleGain;
    return parameters->getRawParameterValue(TreeIDs::paramModuleGain + getIndexString());
}

std::atomic<float>* KrumModule::getModuleClipGain()
{
    //return moduleClipGain;
    return parameters->getRawParameterValue(TreeIDs::paramModuleClipGain + getIndexString());
}


//float KrumModule::getModulePan()
std::atomic<float>* KrumModule::getModulePan()
{
    //return modulePan;
    return parameters->getRawParameterValue(TreeIDs::paramModulePan + getIndexString());
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
    //auto modulesTree = valueTree->getChildWithName(TreeIDs::KRUMMODULES);

}

//void KrumModule::triggerNoteOn()
//{
//    //not sure if this is the best way to do this
//    //moduleProcessor->triggerNoteOn();
//    sampler.noteOn(getMidiTriggerChannel(), getMidiTriggerNote(), buttonClickVelocity);
//}
//
//void KrumModule::triggerNoteOff() 
//{
//    //setModulePlaying(false);
//    //moduleProcessor->triggerNoteOff();
//    sampler.noteOff(getMidiTriggerChannel(), getMidiTriggerNote(), 0, true);
//}


//bool KrumModule::doesModuleNeedToUpdateTree()
//{
//    return needsToUpdateTree;
//}

//bool KrumModule::isModulePlaying()
//{
//    return info.modulePlaying;
//}
//
//void KrumModule::setModulePlaying(const bool isPlaying)
//{
//    info.modulePlaying = isPlaying;
//}

//void KrumModule::updateModuleFromTree()
//{
//    //getValuesFromTree();
//    //updateAudioAtomics();
//}

//void KrumModule::setMidiTriggerChannel(int newMidiChannel)
//{
//    
//    info.midiChannel = newMidiChannel;
//    updateValuesInTree();
//}

//void KrumModule::setModuleName(juce::String newName)
//{
//    info.name = newName;
//    
//    updateValuesInTree();
//}

//void KrumModule::setMidiTriggerNote(int midiNoteNumber, bool removeOld)
//{
//    if (removeOld)
//    {
//        moduleEditor->setOldMidiNote(info.midiNote);
//    }
//
//    info.midiNote = midiNoteNumber;
//    //setModuleState(ModuleState::hasMidi);
//    updateValuesInTree();
//}

//void KrumModule::setSampleFile(juce::File& newFile)
//{
//    if (newFile.existsAsFile())
//    {
//        info.audioFile = newFile;
//        setModuleState(ModuleState::hasFile);
//        updateValuesInTree();
//    }
//}

////Blank Module Ctor
//KrumModule::KrumModule(KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts)
//    : sampler(km), valueTree(valTree), parameters(apvts)
//{
//    info.moduleState = ModuleState::empty;
//}

//Creates a module with NO MIDI assigned, up to the module to get this from the user.
//This ctor is used when creating a new module from the GUI, okay to update Value Tree
//KrumModule::KrumModule(juce::String& moduleName, int index, juce::File file, KrumSampler& km,
//                        juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts)
//    : valueTree(valTree), parameters(apvts), sampler(km)
//{
//    //moduleProcessor.reset(new KrumModuleProcessor(*this, km));
//
//    info.samplerIndex = index;
//    info.audioFile = file;
//    info.name = moduleName;
//    info.moduleState = hasFile;
//    //info.moduleState = ModuleState::empty;
//
//    //for now..
//    //info.displayIndex = info.ndex;
//
//    updateValuesInTree();
//}

//void KrumModule::setModuleSelected(bool isModuleSelected)
//{
//    if (moduleEditor != nullptr)
//    {
//        moduleEditor->setModuleSelected(isModuleSelected);
//    }
//}
//
//void KrumModule::removeSettingsOverlay(bool keepSettings)
//{
//    if (moduleEditor != nullptr) 
//    {
//        moduleEditor->removeSettingsOverlay(keepSettings);
//    }
//}
//
//void KrumModule::setModuleState(ModuleState newState)
//{
//    if(newState == ModuleState::hasFile && info.moduleState == ModuleState::hasFile)
//    {
//        info.moduleState = ModuleState::active;
//    }
//    else if (info.moduleState == ModuleState::active && newState != ModuleState::empty)
//    {
//        return;
//    }
//    else
//    {
//        info.moduleState = newState;
//    }
//    
//    updateEditorFromState();
//    updateValuesInTree();
//    
//}

//use this only to capture midi assignments, does not trigger any sound
//void KrumModule::handleNoteOn(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity)
//{
//    if (moduleEditor == nullptr)
//    {
//        source->removeListener(this);
//    }
//    else if (moduleEditor->doesEditorWantMidi())
//    {
//        moduleEditor->handleMidi(midiChannelNumber, midiNoteNumber);
//    }
//   
//}
//void KrumModule::handleNoteOff(juce::MidiKeyboardState* source, int midiChannelNumber, int midiNoteNumber, float velocity)
//{
//    //nothing to do with note off messages in this context. It is a virtual function from inheriting from juce::MidiKeyboardStateListener, making it manditory. 
//}


//automatically sets display index for now.
//void KrumModule::setModuleSamplerIndex(int newIndex)
//{
//    info.samplerIndex = newIndex;
//    //setModuleDisplayIndex(newIndex);
//    updateValuesInTree();
//}

//void KrumModule::setModuleDisplayIndex(int newIndex)
//{
//    info.displayIndex = newIndex; 
//    updateValuesInTree();
//}

//void KrumModule::setModuleGain(float newGain)
//{
//    auto param = parameters->getParameter(TreeIDs::paramModuleGain + getIndexString());
//    auto newGainParam = param->getNormalisableRange().convertTo0to1(newGain);
//    param->setValueNotifyingHost(newGainParam);
//}

//void KrumModule::setModuleColor(juce::Colour newModuleColor, bool refreshChildren)
//{
//    info.moduleColor = newModuleColor;
//    updateValuesInTree();
//
//    if (moduleEditor != nullptr && refreshChildren)
//    {
//        moduleEditor->setKeyboardColor();
//    }
//}

//bool KrumModule::isModuleDragging()
//{
//    return info.moduleDragging;
//}
//
//void KrumModule::setModuleDragging(bool isDragging)
//{
//    info.moduleDragging = isDragging;
//}

//void KrumModule::setModulePan(float newPan)
//{
//    auto param = parameters->getParameter(TreeIDs::paramModulePan + getIndexString());
//    param->setValueNotifyingHost(newPan); 
//}

//void KrumModule::updateAudioAtomics()
//{
//    moduleGain = parameters->getRawParameterValue(TreeIDs::paramModuleGain + getIndexString());
//    modulePan = parameters->getRawParameterValue(TreeIDs::paramModulePan + getIndexString());
//    moduleClipGain = parameters->getRawParameterValue(TreeIDs::paramModuleClipGain + getIndexString());
//    DBG("Raw Value: " + juce::String(*moduleGain));
//}

//loads the ModuleInfo with the values passed in from the ValueTree 
//void KrumModule::getValuesFromTree()
//{
//
//        //auto modulesTree = valueTree->getChildWithName(TreeIDs::KRUMMODULES);
//        //auto moduleTree = modulesTree.getChildWithName(TreeIDs::MODULE + getIndexString()); //the index from getIndexString() is set in ctor
//        //
//        //info.name = moduleTree.getProperty(TreeIDs::moduleName_ID);
//        //info.moduleState = static_cast<ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState_ID));
//        //info.audioFile = juce::File(moduleTree.getProperty(TreeIDs::moduleFile_ID).toString());
//        //info.midiNote = (int)moduleTree.getProperty(TreeIDs::moduleMidiNote_ID);
//        //info.midiChannel = (int)moduleTree.getProperty(TreeIDs::moduleMidiChannel_ID);
//        //info.moduleColor = juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor_ID).toString());
//        //info.displayIndex = (int)moduleTree.getProperty(TreeIDs::moduleDisplayIndex_ID);
//
//
//    
//}

//uses the ModuleInfo struct to update the ValueTree
//void KrumModule::updateValuesInTree(bool printBefore)
//{
//    if (valueTree != nullptr)
//    {
//        if (printBefore)
//        {
//            DBG("Value Tree In Module " + info.name);
//            auto state = valueTree->createCopy();
//            std::unique_ptr<juce::XmlElement> xml(state.createXml());
//            DBG(xml->toString());
//        }
//
//        auto modulesTree = valueTree->getChildWithName(TreeIDs::KRUMMODULES);
//        auto moduleTree = modulesTree.getChildWithName(TreeIDs::MODULE + getIndexString());
//
//        moduleTree.setProperty(TreeIDs::moduleName_ID, juce::var(info.name), nullptr);
//        moduleTree.setProperty(TreeIDs::moduleState_ID, (int)info.moduleState, nullptr);
//        moduleTree.setProperty(TreeIDs::moduleFile_ID, juce::var(info.audioFile.getFullPathName()), nullptr);
//        moduleTree.setProperty(TreeIDs::moduleMidiNote_ID, juce::var(info.midiNote), nullptr);
//        moduleTree.setProperty(TreeIDs::moduleMidiChannel_ID, juce::var(info.midiChannel), nullptr);
//        moduleTree.setProperty(TreeIDs::moduleColor_ID, juce::var(info.moduleColor.toDisplayString(true)), nullptr);
//        moduleTree.setProperty(TreeIDs::moduleDisplayIndex_ID, juce::var(info.displayIndex), nullptr);
//
//        /*juce::ValueTree stateTree;
//        juce::var id;*/
//        
//        /*for (int i = 0; i < moduleTree.getNumChildren(); i++)         
//        {
//            stateTree = moduleTree.getChild(i);
//            id = stateTree.getProperty("id");
//
//            if (id == TreeIDs::paramModuleState_ID)
//            {
//                stateTree.setProperty("value", (int)info.moduleState, nullptr);
//            }
//            else if (id == TreeIDs::paramModuleFile_ID)
//            {
//                stateTree.setProperty("value", juce::var(info.audioFile.getFullPathName()), nullptr);
//            }
//            else if (id == TreeIDs::paramModuleMidiNote_ID)
//            {
//                stateTree.setProperty("value", juce::var(info.midiNote), nullptr);
//            }
//            else if (id == TreeIDs::paramModuleMidiChannel_ID)
//            {
//                stateTree.setProperty("value", juce::var(info.midiChannel), nullptr);
//            }
//            else if (id == TreeIDs::paramModuleColor_ID)
//            {
//                stateTree.setProperty("value", juce::var(info.moduleColor.toDisplayString(true)), nullptr);
//            }
//            else if(id == TreeIDs::paramModuleDisplayIndex_ID)
//            {
//                stateTree.setProperty("value", juce::var(info.displayIndex), nullptr);
//            }
//        }*/
//
//        /*DBG("Value Tree In Module " + name);
//        auto state = valueTree->createCopy();
//        std::unique_ptr<juce::XmlElement> xml(state.createXml());
//        DBG(xml->toString());*/
//    }
//
//    needsToUpdateTree = false;
//}
//
//void KrumModule::clearModuleValueTree()
//{
//    //auto modulesTree = valueTree->getChildWithName(TreeIDs::KRUMMODULES);
//    //auto moduleTree = modulesTree.getChildWithName(TreeIDs::MODULE + getIndexString());
//
//    //moduleTree.setProperty("name", juce::var(""), nullptr);
//
//    //juce::ValueTree stateTree;
//    ////juce::var id;
//
//    //for (int i = 0; i < moduleTree.getNumChildren(); i++)
//    //{
//    //    stateTree = moduleTree.getChild(i);
//    //    stateTree.setProperty("value", juce::var(""), nullptr);
//    //}
//
//    moduleTree.setProperty(TreeIDs::moduleName, juce::var(""), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleState, juce::var(0), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleFile, juce::var(""), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleMidiNote, juce::var(0), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleMidiChannel, juce::var(0), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleColor, juce::var(""), nullptr);
//    moduleTree.setProperty(TreeIDs::moduleDisplayIndex, juce::var(-1), nullptr);
//
//
//}


//void KrumModule::updateEditorFromState()
//{
//    if(hasEditor())
//    {
//        auto currentEditor = getCurrentModuleEditor();
//        if (getModuleState() == ModuleState::active) //has a file and a midi assignment
//        {
//            if (currentEditor->needsToBuildEditor())
//            {
//                currentEditor->buildModule();
//            }
//            else
//            {
//                currentEditor->repaint();
//            }
//        }
//        else if (getModuleState() == ModuleState::hasFile) //has a file but NO midi assignment
//        {
//            currentEditor->showSettingsOverlay();
//            currentEditor->repaint();
//        }
//        else if (getModuleState() == ModuleState::empty) //has nothing
//        {
//            deleteModuleEditor();
//            deleteEntireModule();
//            sampler.removeModuleSound(this);
//            
//        }
//    }
//    
//}

//this is needed when deleting modules and needing to reassign the slider listeners in the ValueTree
//void KrumModule::reassignSliders()
//{
//    if (moduleEditor != nullptr)
//    {
//        moduleEditor->reassignSliderAttachments();
//        updateAudioAtomics();
//    }
//}

//int KrumModule::deleteEntireModule()
//{
//    clearModuleValueTree();
//    getValuesFromTree();
//    return 0;
//}

//KrumModuleEditor* KrumModule::createModuleEditor(KrumSamplerAudioProcessorEditor& editor)
//{
//    if (moduleEditor == nullptr)
//    {
//        moduleEditor.reset(new KrumModuleEditor(*this, editor));
//    }
//
//    setEditorVisibility(true);
//    return moduleEditor.get();
//}
//
//bool KrumModule::hasEditor()
//{
//    return moduleEditor != nullptr;
//}
//
//void KrumModule::setEditorVisibility(bool isVisible)
//{
//    if (moduleEditor)
//    {
//        moduleEditor->setVisible(isVisible);
//    }
//}
//
//KrumModuleEditor* KrumModule::getCurrentModuleEditor()
//{
//    return moduleEditor.get();
//}
//
//void KrumModule::deleteModuleEditor()
//{
//    moduleEditor->removeFromDisplay();
//    moduleEditor = nullptr;
//}
//
////returns an int for possible error codes, none exist at the moment
//
////void KrumModule::setParentEditor(KrumSamplerAudioProcessorEditor* parent)
////{
////    parentEditor = parent;
////}
//

//
// 
