/*
  ==============================================================================

    KrumModule.cpp
    Created: 1 Mar 2021 11:15:42am
    Author:  krisc

  ==============================================================================
*/

#include "KrumModule.h"
#include "PluginEditor.h"


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


//creates a module from the ValueTree passed in
KrumModule::KrumModule(int newIndex, KrumSampler& km, juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts)
    : valueTree(valTree), parameters(apvts), sampler(km)
{
    info.samplerIndex = newIndex;
}

KrumModule::~KrumModule()
{
}

void KrumModule::updateModuleFromTree()
{
    getValuesFromTree();
    updateAudioAtomics();
}

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


void KrumModule::setModuleSelected(bool isModuleSelected)
{
    if (moduleEditor != nullptr)
    {
        moduleEditor->setModuleSelected(isModuleSelected);
    }
}

void KrumModule::removeSettingsOverlay(bool keepSettings)
{
    if (moduleEditor != nullptr) 
    {
        moduleEditor->removeSettingsOverlay(keepSettings);
    }
}

void KrumModule::setModuleState(ModuleState newState)
{
    if(newState == ModuleState::hasFile && info.moduleState == ModuleState::hasFile)
    {
        info.moduleState = ModuleState::active;
    }
    else if (info.moduleState == ModuleState::active && newState != ModuleState::empty)
    {
        return;
    }
    else
    {
        info.moduleState = newState;
    }
    
    updateEditorFromState();
    updateValuesInTree();
    
}

KrumModule::ModuleState KrumModule::getModuleState()
{
    return info.moduleState;
}

juce::File& KrumModule::getSampleFile()
{
    return info.audioFile;
}

void KrumModule::setSampleFile(juce::File& newFile)
{
    if (newFile.existsAsFile())
    {
        info.audioFile = newFile;
        setModuleState(ModuleState::hasFile);
        updateValuesInTree();
    }
}


int KrumModule::getMidiTriggerNote()
{
    return info.midiNote;
}

void KrumModule::setMidiTriggerNote(int midiNoteNumber, bool removeOld)
{
    if (removeOld)
    {
        moduleEditor->setOldMidiNote(info.midiNote);
    }

    info.midiNote = midiNoteNumber;
    //setModuleState(ModuleState::hasMidi);
    updateValuesInTree();
}

int KrumModule::getMidiTriggerChannel()
{
    return info.midiChannel;
}

void KrumModule::setMidiTriggerChannel(int newMidiChannel)
{
    
    info.midiChannel = newMidiChannel;
    updateValuesInTree();
}

juce::String& KrumModule::getModuleName()
{
    return info.name;
}

void KrumModule::setModuleName(juce::String newName)
{
    info.name = newName;
    
    updateValuesInTree();
}

bool KrumModule::isModulePlaying()
{
    return info.modulePlaying;
}

void KrumModule::setModulePlaying(const bool isPlaying)
{
    info.modulePlaying = isPlaying;
//    if (!isPlaying && moduleEditor &&  moduleEditor->shouldCheckDroppedFile())
//    {
//        moduleEditor->handleLastDroppedFile();
//    }
}

bool KrumModule::isModuleActive()
{
    return info.moduleState == ModuleState::active;
}

bool KrumModule::isModuleEmpty()
{
    return info.moduleState == ModuleState::empty;
}

bool KrumModule::isModuleActiveOrHasFile()
{
    return isModuleActive() || info.moduleState == ModuleState::hasFile;
}

int KrumModule::getModuleSamplerIndex()
{
    return info.samplerIndex;
}

//automatically sets display index for now.
void KrumModule::setModuleSamplerIndex(int newIndex)
{
    info.samplerIndex = newIndex;
    //setModuleDisplayIndex(newIndex);
    updateValuesInTree();
}

int KrumModule::getModuleDisplayIndex() 
{
    return info.displayIndex;
}

void KrumModule::setModuleDisplayIndex(int newIndex)
{
    info.displayIndex = newIndex; 
    updateValuesInTree();
}

juce::Colour KrumModule::getModuleColor() 
{
    return info.moduleColor;
}
void KrumModule::setModuleColor(juce::Colour newModuleColor, bool refreshChildren)
{
    info.moduleColor = newModuleColor;
    updateValuesInTree();

    if (moduleEditor != nullptr && refreshChildren)
    {
        moduleEditor->setKeyboardColor();
    }
}

bool KrumModule::isModuleDragging()
{
    return info.moduleDragging;
}

void KrumModule::setModuleDragging(bool isDragging)
{
    info.moduleDragging = isDragging;
}

void KrumModule::triggerNoteOn()
{
    //not sure if this is the best way to do this
    //moduleProcessor->triggerNoteOn();
    sampler.noteOn(info.midiChannel, info.midiNote, buttonClickVelocity);
}

void KrumModule::triggerNoteOff() 
{
    //setModulePlaying(false);
    //moduleProcessor->triggerNoteOff();
    sampler.noteOff(info.midiChannel, info.midiNote, 0, true);
}

void KrumModule::setModuleGain(float newGain)
{
    auto param = parameters->getParameter(TreeIDs::paramModuleGain_ID + getIndexString());
    auto newGainParam = param->getNormalisableRange().convertTo0to1(newGain);
    param->setValueNotifyingHost(newGainParam);
}

std::atomic<float>* KrumModule::getModuleGain()
{
    return moduleGain;
}

std::atomic<float>* KrumModule::getModuleClipGain()
{
    return moduleClipGain;
}

void KrumModule::setModulePan(float newPan)
{
    auto param = parameters->getParameter(TreeIDs::paramModulePan_ID + getIndexString());
    param->setValueNotifyingHost(newPan); 
}

//float KrumModule::getModulePan()
std::atomic<float>* KrumModule::getModulePan()
{
    return modulePan;
}

bool KrumModule::doesModuleNeedToUpdateTree()
{
    return needsToUpdateTree;
}

//loads the ModuleInfo with the values passed in from the ValueTree 
void KrumModule::getValuesFromTree()
{
    if (valueTree != nullptr)
    {
        auto modulesTree = valueTree->getChildWithName("KrumModules");
        auto moduleTree = modulesTree.getChildWithName("Module" + getIndexString()); //the index from getIndexString() is set in ctor
        
        info.name = moduleTree.getProperty(TreeIDs::paramModuleName_ID);
        info.moduleState = static_cast<ModuleState>((int)moduleTree.getProperty(TreeIDs::paramModuleState_ID));
        info.audioFile = juce::File(moduleTree.getProperty(TreeIDs::paramModuleFile_ID).toString());
        info.midiNote = (int)moduleTree.getProperty(TreeIDs::paramModuleMidiNote_ID);
        info.midiChannel = (int)moduleTree.getProperty(TreeIDs::paramModuleMidiChannel_ID);
        info.moduleColor = juce::Colour::fromString(moduleTree.getProperty(TreeIDs::paramModuleColor_ID).toString());
        info.displayIndex = (int)moduleTree.getProperty(TreeIDs::paramModuleDisplayIndex_ID);

        /*juce::var nameValue = moduleTree.getProperty("name");
        info.name = nameValue.toString();*/

        /*juce::ValueTree stateTree;
        juce::var id;
        juce::var val;

        for (int j = 0; j < moduleTree.getNumChildren(); j++)              
        {
            stateTree = moduleTree.getChild(j);
            id = stateTree.getProperty("id");
            val = stateTree.getProperty("value");
            DBG(id.toString() + " " + val.toString());
            if (id.toString() == TreeIDs::paramModuleState_ID)
            {
                info.moduleState = static_cast<ModuleState>((int)val);
            }
            else if (id.toString() == TreeIDs::paramModuleFile_ID && !val.isVoid())
            {
                info.audioFile = juce::File(val.toString()); 
            }
            else if (id.toString() == TreeIDs::paramModuleMidiNote_ID && !val.isVoid())
            {
                info.midiNote = int(val);
            }
            else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID && !val.isVoid())
            {
                info.midiChannel = int(val);
            }
            else if (id.toString() == TreeIDs::paramModuleColor_ID && !val.isVoid())
            {
                info.moduleColor = juce::Colour::fromString(val.toString());
            }
            else if (id.toString() == TreeIDs::paramModuleDisplayIndex_ID && !val.isVoid())
            {
                info.displayIndex = int(val);
            }
        }*/
    }
}

//uses the ModuleInfo struct to update the ValueTree
void KrumModule::updateValuesInTree(bool printBefore)
{
    if (valueTree != nullptr)
    {
        if (printBefore)
        {
            DBG("Value Tree In Module " + info.name);
            auto state = valueTree->createCopy();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());
            DBG(xml->toString());
        }

        auto modulesTree = valueTree->getChildWithName("KrumModules");
        auto moduleTree = modulesTree.getChildWithName("Module" + getIndexString());

        moduleTree.setProperty(TreeIDs::paramModuleName_ID, juce::var(info.name), nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleState_ID, (int)info.moduleState, nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleFile_ID, juce::var(info.audioFile.getFullPathName()), nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleMidiNote_ID, juce::var(info.midiNote), nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleMidiChannel_ID, juce::var(info.midiChannel), nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleColor_ID, juce::var(info.moduleColor.toDisplayString(true)), nullptr);
        moduleTree.setProperty(TreeIDs::paramModuleDisplayIndex_ID, juce::var(info.displayIndex), nullptr);

        /*juce::ValueTree stateTree;
        juce::var id;*/
        
        /*for (int i = 0; i < moduleTree.getNumChildren(); i++)         
        {
            stateTree = moduleTree.getChild(i);
            id = stateTree.getProperty("id");

            if (id == TreeIDs::paramModuleState_ID)
            {
                stateTree.setProperty("value", (int)info.moduleState, nullptr);
            }
            else if (id == TreeIDs::paramModuleFile_ID)
            {
                stateTree.setProperty("value", juce::var(info.audioFile.getFullPathName()), nullptr);
            }
            else if (id == TreeIDs::paramModuleMidiNote_ID)
            {
                stateTree.setProperty("value", juce::var(info.midiNote), nullptr);
            }
            else if (id == TreeIDs::paramModuleMidiChannel_ID)
            {
                stateTree.setProperty("value", juce::var(info.midiChannel), nullptr);
            }
            else if (id == TreeIDs::paramModuleColor_ID)
            {
                stateTree.setProperty("value", juce::var(info.moduleColor.toDisplayString(true)), nullptr);
            }
            else if(id == TreeIDs::paramModuleDisplayIndex_ID)
            {
                stateTree.setProperty("value", juce::var(info.displayIndex), nullptr);
            }
        }*/

        /*DBG("Value Tree In Module " + name);
        auto state = valueTree->createCopy();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        DBG(xml->toString());*/
    }

    needsToUpdateTree = false;
}

void KrumModule::clearModuleValueTree()
{
    auto modulesTree = valueTree->getChildWithName("KrumModules");
    auto moduleTree = modulesTree.getChildWithName("Module" + getIndexString());

    //moduleTree.setProperty("name", juce::var(""), nullptr);

    //juce::ValueTree stateTree;
    ////juce::var id;

    //for (int i = 0; i < moduleTree.getNumChildren(); i++)
    //{
    //    stateTree = moduleTree.getChild(i);
    //    stateTree.setProperty("value", juce::var(""), nullptr);
    //}

    moduleTree.setProperty(TreeIDs::paramModuleName_ID, juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleState_ID, juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleFile_ID, juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleMidiNote_ID, juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleMidiChannel_ID, juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleColor_ID, juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::paramModuleDisplayIndex_ID, juce::var(-1), nullptr);


}

void KrumModule::updateAudioAtomics()
{
    moduleGain = parameters->getRawParameterValue(TreeIDs::paramModuleGain_ID + getIndexString());
    modulePan = parameters->getRawParameterValue(TreeIDs::paramModulePan_ID + getIndexString());
    moduleClipGain = parameters->getRawParameterValue(TreeIDs::paramModuleClipGain_ID + getIndexString());
    DBG("Raw Value: " + juce::String(*moduleGain));
}

void KrumModule::updateEditorFromState()
{
    if(hasEditor())
    {
        auto currentEditor = getCurrentModuleEditor();
        if (info.moduleState == ModuleState::active) //has a file and a midi assignment
        {
            if (currentEditor->needsToBuildEditor())
            {
                currentEditor->buildModule();
            }
            else
            {
                currentEditor->repaint();
            }
        }
        else if (info.moduleState == ModuleState::hasFile) //has a file but NO midi assignment
        {
            currentEditor->showSettingsOverlay();
            currentEditor->repaint();
        }
        else if (info.moduleState == ModuleState::empty) //has nothing
        {
            deleteModuleEditor();
            deleteEntireModule();
            sampler.removeModuleSound(this);
            
        }
    }
    
}

//this is needed when deleting modules and needing to reassign the slider listeners in the ValueTree
//void KrumModule::reassignSliders()
//{
//    if (moduleEditor != nullptr)
//    {
//        moduleEditor->reassignSliderAttachments();
//        updateAudioAtomics();
//    }
//}

KrumModuleEditor* KrumModule::createModuleEditor(KrumSamplerAudioProcessorEditor& editor)
{
    if (moduleEditor == nullptr)
    {
        moduleEditor.reset(new KrumModuleEditor(*this, editor));
    }

    setEditorVisibility(true);
    return moduleEditor.get();
}

bool KrumModule::hasEditor()
{
    return moduleEditor != nullptr;
}

void KrumModule::setEditorVisibility(bool isVisible)
{
    if (moduleEditor)
    {
        moduleEditor->setVisible(isVisible);
    }
}

KrumModuleEditor* KrumModule::getCurrentModuleEditor()
{
    return moduleEditor.get();
}

void KrumModule::deleteModuleEditor()
{
    moduleEditor->removeFromDisplay();
    moduleEditor = nullptr;
}

//returns an int for possible error codes, none exist at the moment
int KrumModule::deleteEntireModule()
{
    clearModuleValueTree();
    getValuesFromTree();
    return 0;
}

//void KrumModule::setParentEditor(KrumSamplerAudioProcessorEditor* parent)
//{
//    parentEditor = parent;
//}

juce::String KrumModule::getIndexString()
{
    return juce::String(info.samplerIndex);
}

 
