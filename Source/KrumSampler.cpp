/*
  ==============================================================================

    KrumSampler.cpp
    Created: 1 Mar 2021 11:15:22am
    Author:  krisc

  ==============================================================================
*/

#include "KrumSampler.h"
#include "PluginProcessor.h"


KrumSound::KrumSound    (KrumModule* pModule, 
                        const juce::String& soundName,
                        juce::AudioFormatReader& source,
                        const juce::BigInteger& notes,
                        int midiNoteForNormalPitch,
                        double attackTimeSecs,
                        double releaseTimeSecs,
                        double maxSampleLengthSeconds)
    : parentModule(pModule), name(soundName), sourceSampleRate(source.sampleRate), midiNotes(notes),midiRootNote(midiNoteForNormalPitch),
        SamplerSound(soundName, source, notes, midiNoteForNormalPitch, attackTimeSecs, releaseTimeSecs, maxSampleLengthSeconds)
{
    if (sourceSampleRate > 0 && source.lengthInSamples > 0)
    {
        length = juce::jmin((int)source.lengthInSamples,
            (int)(maxSampleLengthSeconds * sourceSampleRate));

        data.reset(new juce::AudioBuffer<float>(juce::jmin(2, (int)source.numChannels), length + 4));

        source.read(data.get(), 0, length + 4, 0, true, true);

        params.attack = static_cast<float> (attackTimeSecs);
        params.release = static_cast<float> (releaseTimeSecs);
    }

    DBG("I'm Alive: " + name);

}

KrumSound::~KrumSound()
{
    DBG("I'm DEAD: " + name);
}

std::atomic<float>* KrumSound::getModuleGain() const
{
    return parentModule->getModuleGain();
}

std::atomic<float>* KrumSound::getModulePan() const
{
    return parentModule->getModulePan();
}

std::atomic<float>* KrumSound::getModuleClipGain() const
{
    return parentModule->getModuleClipGain();
}

//void KrumSound::setModulePlaying(bool isPlaying)
//{
//    parentModule->setModulePlaying(isPlaying);
//}

bool KrumSound::isParent(KrumModule* moduleToTest)
{
    return parentModule == moduleToTest;
}

//==================================================================================================//

KrumVoice::KrumVoice()
{
}

KrumVoice::~KrumVoice()
{}

bool KrumVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    if (sound)
    {
        return true;
    }
    return false;
}

bool KrumVoice::isVoiceActive() const
{
    return adsr.isActive();
}

void KrumVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int pitchWheel)
{
    if (auto* sound = static_cast<const KrumSound*> (s))
    {
        //Pitchshifting will be a thing at some point.
        /*pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            * sound->sourceSampleRate / getSampleRate();*/

        sourceSamplePosition = 0.0;

        //only storing this, it will be applied in the render block
        clipGain = sound->getModuleClipGain()->load();

        float moduleGain = *sound->getModuleGain();
        float modulePan = *sound->getModulePan();

        //module gain
        lgain = velocity * (moduleGain);
        rgain = velocity * (moduleGain);

        //panned right
        if (modulePan > 0.5f)
        {
            lgain = lgain * (1 - modulePan);
        }

        //panned left
        if(modulePan < 0.5f)
        {
            rgain = rgain * (modulePan);
        }

        adsr.setSampleRate(sound->sourceSampleRate);
        adsr.setParameters(sound->params);

        adsr.noteOn();
    }
}

void KrumVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void KrumVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (auto* playingSound = static_cast<KrumSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->getAudioData();
        const float* const inL = data.getReadPointer(0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            auto pos = (int)sourceSamplePosition;
            auto alpha = (float)(sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

            l = clipGain * l;
            r = clipGain * r;

            auto envelopeValue = adsr.getNextSample();

            //lgain and rgain are set by our volume and panning sliders in KrumVoice::startNote()
            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            //sourceSamplePosition += pitchRatio;

            if (++sourceSamplePosition > playingSound->length)
            {
                stopNote(0.0f, false);
                break;
            }
        }
    }
}

//====================================================================================//

KrumSampler::KrumSampler(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts, juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o)
    :formatManager(fm), owner(o)
{
    //initModules(valTree, apvts);
    initVoices();
}

KrumSampler::~KrumSampler()
{
    clearModules();
}

void KrumSampler::initModules(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts)
{
    auto krumModulesTree = valTree->getChildWithName(TreeIDs::KRUMMODULES);

    for (int i = 0; i < MAX_NUM_MODULES; i++)
    {
        auto moduleTree = krumModulesTree.getChild(i);
        moduleTree.setProperty(TreeIDs::moduleSamplerIndex, i, nullptr);
        modules.add(new KrumModule(*this, moduleTree, apvts));
    }
    
    DBG("Modules Initialized: " + juce::String(modules.size()));

    initVoices();
}

void KrumSampler::initVoices()
{
    for (int i = 0; i < MAX_VOICES; i++)
    {
        auto newVoice = voices.add(new KrumVoice());
        newVoice->setCurrentPlaybackSampleRate(getSampleRate());
    }

    juce::Logger::writeToLog("Voices Initialized: " + juce::String(voices.size()));
    printVoices();
    
}

void KrumSampler::noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) 
{
    for (auto sound : sounds)
    {
        if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
        {
            auto krumSound = static_cast<KrumSound*>(sound);
            auto voice = findFreeVoice(sound, midiChannel, midiNoteNumber, true);

            startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
        }
    }
}

void KrumSampler::noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) 
{
           //the sampler only plays one shots, so no need to turn any voices off here.
    //for (auto sound : sounds)
    //{
    //    if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
    //    {
    //       /* auto krumSound = static_cast<KrumSound*>(sound);
    //        krumSound->setModulePlaying(false)*/
    //    }
    //}
}

KrumModule* KrumSampler::getModule(int index)
{
    return modules[index];
}

void KrumSampler::addModule(KrumModule* newModule)
{
    //modules.add(std::move(newModule));
    modules.insert(newModule->getModuleSamplerIndex(), std::move(newModule));
}


void KrumSampler::removeModuleSound(KrumModule* moduleToDelete/*, bool updateTree*/)
{

    DBG("Module Index To Delete: " + juce::String(moduleToDelete->getModuleSamplerIndex()));
    DBG("Module Sample: " + moduleToDelete->getSampleFile().getFullPathName());

    KrumSound* ksound = nullptr;
    for (int i = 0; i < sounds.size(); i++)
    {
        auto sound = sounds.getObjectPointer(i);
        //cast to juce::samplersound first? 
        ksound = static_cast<KrumSound*>(sound);
        if (ksound && ksound->isParent(moduleToDelete))
        {
            sounds.removeObject(sound);
            DBG("sound removed");
            printSounds();
            /*if (updateTree)
            {
                owner.updateValueTreeState();
            }*/
            return;
        }
        
    }

    DBG("no sounds removed");

}
 
void KrumSampler::updateModuleSample(KrumModule* updatedModule)
{
    //removes the currently assigned sound of the module, if none exist this function will do nothing
    //we pass in false to NOT update the valueTree as we are about to add a sample to it and we don't want the tree to set the module inactive
    removeModuleSound(updatedModule/*, false*/); 
    addSample(updatedModule);
}

void KrumSampler::addSample(KrumModule* moduleToAddSound)
{
    if (isFileAcceptable(moduleToAddSound->getSampleFile()))
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(moduleToAddSound->getSampleFile()));
        juce::BigInteger range;
        range.setBit(moduleToAddSound->getMidiTriggerNote());

        sounds.add(new KrumSound(moduleToAddSound, moduleToAddSound->getModuleName(), *reader, range, moduleToAddSound->getMidiTriggerNote(),
                            attackTime, releaseTime, MAX_FILE_LENGTH_SECS));


        //juce::Logger::writeToLog("Sample Added - Sounds: " + juce::String(sounds.size()));
        printSounds();
    }
    
}

void KrumSampler::clearModules()
{
    modules.clear();
    voices.clear();
    sounds.clear();

    juce::Logger::writeToLog("Modules Cleared - Sounds Size: " + juce::String(sounds.size()));
}

int KrumSampler::getNumModules()
{
    return modules.size();
}

void KrumSampler::getNumFreeModules(int& totalFreeModules, int& firstFreeIndex)
{
    totalFreeModules = 0;
    firstFreeIndex = -1; //-1 indicates the value hasn't been changed yet
    
    for(int i = 0; i < modules.size(); i++)
    {
        if(modules[i]->getModuleState() == KrumModule::ModuleState::empty)
        {
            totalFreeModules++;
            if(firstFreeIndex == -1)
            {
                firstFreeIndex = i;
            }
        }
    }
    
//    if(totalFreeModules == 0)
//    {
//        firstFreeIndex = -1;
//    }
    
    DBG("Free Modules: " + juce::String(totalFreeModules) + ", First Index " + juce::String(firstFreeIndex));
}

bool KrumSampler::isFileAcceptable(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "File type not supported!", "The current supported file types are: " + formatManager.getWildcardForAllFormats() + ".");
        return false;
    }

    if (reader->lengthInSamples / reader->sampleRate >= MAX_FILE_LENGTH_SECS)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"File Too Long!", "The maximum file length is " + juce::String(MAX_FILE_LENGTH_SECS) + " seconds.");
        return false;
    }

    return true;
}

juce::AudioFormatManager& KrumSampler::getFormatManager()
{
    return formatManager;
}

void KrumSampler::printSounds()
{
    DBG("Sounds Size = " + juce::String(sounds.size()));
    for (int i = 0; i < sounds.size(); i++)
    {
        auto sound = sounds[i];
        //auto krumSound = static_cast<KrumSound*>(&sounds[i]);
        DBG("Sound: " + juce::String(i) + (sound ? " is valid" : " is NULL"));
    }
}

void KrumSampler::printVoices()
{
    DBG("Voices Size = " + juce::String(voices.size()));
    for (int i = 0; i < voices.size(); i++)
    {
        auto voice = voices[i];
        //auto krumSound = static_cast<KrumSound*>(&sounds[i]);
        DBG("Voice: " + juce::String(i) + (voice ? " is valid" : " is NULL"));
    }
}


