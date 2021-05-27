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
    //SamplerSound::~SamplerSound();
}

std::atomic<float>* KrumSound::getModuleGain() const
{
    return parentModule->getModuleGain();
}

std::atomic<float>* KrumSound::getModulePan() const
{
    return parentModule->getModulePan();
}

void KrumSound::setModulePlaying(bool isPlaying)
{
    parentModule->setModulePlaying(isPlaying);
}


//KrumModule* KrumSound::getParentModule()
//{
//    return parentModule;
//}


//==================================================================================================//

KrumVoice::KrumVoice(/*KrumModule* attachedModule*/)
   // : parentModule(attachedModule)
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

    //if (auto krumSound = dynamic_cast<const KrumSound*>(sound))
    //{
    //    //doesn't account for different midi channels
    //    return parentModule->getMidiTriggerNote() == krumSound->midiRootNote;
    //}
    //
    //return false;
}

bool KrumVoice::isVoiceActive() const
{
    return adsr.isActive();
}


void KrumVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int pitchWheel)
{
    
    if (auto* sound = static_cast<const KrumSound*> (s))
    {
        /*pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            * sound->sourceSampleRate / getSampleRate();*/

        sourceSamplePosition = 0.0;

        std::atomic<float>* moduleGain = sound->getModuleGain();
        std::atomic<float>* modulePan = sound->getModulePan();

        //module gain
        lgain = velocity * (*moduleGain);
        rgain = velocity * (*moduleGain);

        //panned right
        if (*modulePan > 0.5f)
        {
            lgain = lgain * (1 - *modulePan);
        }

        //panned left
        if(*modulePan < 0.5f)
        {
            rgain = rgain * (*modulePan);
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
    if (auto* playingSound = static_cast</*juce::Sampler*/KrumSound*> (getCurrentlyPlayingSound().get()))
    {
        //auto& data = *playingSound->data;
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

            auto envelopeValue = adsr.getNextSample();

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

            //if (sourceSamplePosition > playingSound->length)
            if (++sourceSamplePosition > /*data.getNumSamples()*/playingSound->length)
            {
                stopNote(0.0f, false);
                break;
            }
        }
    }
}




//====================================================================================//

KrumSampler::KrumSampler(juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o)
    :formatManager(fm), owner(o)
{
    formatManager.registerBasicFormats();
}
KrumSampler::~KrumSampler()
{
    clearModules();
}


//May need to revisit this approach of telling the module it is playing. This is a GUI action and probably should NOT be in the audio processing.
//It works for now but might be an issue in larger configs(?). 
void KrumSampler::noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) 
{
    for (auto sound : sounds)
    {
        if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
        {
            auto krumSound = static_cast<KrumSound*>(sound);
            krumSound->setModulePlaying(true);

            auto voice = findFreeVoice(sound, midiChannel, midiNoteNumber, true);
            startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
            
            /*auto krumVoice = static_cast<KrumVoice*>(voice);
            if (krumVoice->getParentModule()->hasEditor())
            {
                krumVoice->getParentModule()->setModulePlaying(true);
            }*/
           // auto voice = findFreeVoice(sound, midiChannel, midiNoteNumber, false);
            /*if (auto krumVoice = static_cast<KrumVoice*>(voice))
            {
                DBG("Voice Name: " + krumVoice->getParentModule()->getModuleName());
            }
            else
            {
                DBG("KrumVoice NULL");
            }*/

            //startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);

            //getVoice(0)
            /*auto samSound = static_cast<juce::SamplerSound*>(sound);
            setModulePlaying(samSound->getName(), true);*/

            //renderVoices();
        }
    }
}

void KrumSampler::noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) 
{
    for (auto sound : sounds)
    {
        if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
        {
            auto krumSound = static_cast<KrumSound*>(sound);
            krumSound->setModulePlaying(false);
            for (auto* voice : voices) 
            {
                if (    (voice->getCurrentlyPlayingNote() == midiNoteNumber) &&
                         voice->isPlayingChannel(midiChannel) &&
                        (voice->getCurrentlyPlayingSound() == sound)
                   )
                {
                    /*auto krumVoice = static_cast<KrumVoice*>(voice);
                    if (krumVoice->getParentModule()->hasEditor())
                    {
                        krumVoice->getParentModule()->setModulePlaying(false);
                    }*/

                    stopVoice(voice, 0.0f, true);
                    /*auto samSound = static_cast<juce::SamplerSound*>(sound);
                    setModulePlaying(samSound->getName(), false);*/
                }
            }
        }
    }
}

juce::SynthesiserVoice* KrumSampler::findFreeVoice  (juce::SynthesiserSound* soundToPlay, int midiChannel, int midiNoteNumber, bool stealIfNoneAvailable) const
{
    const juce::ScopedLock sl(lock);

    for (int i = 0; i < voices.size(); i++)
    {
        auto voice = voices[i]; 
        if (voice->canPlaySound(soundToPlay) && !voice->isVoiceActive())
        {
            
            return voice; 
            
            
            //return ++voice;

            //YIKES this const cast feels wrong....
            /*auto krumVoice = static_cast<KrumVoice*>(voice);
            auto newVoice = new KrumVoice(krumVoice->getParentModule());
            return const_cast<KrumSampler*>(this)->addVoice(newVoice);*/
            
        }
    }

    if (stealIfNoneAvailable)
    {

        /*for (auto* voice : voices)
        {
            if (voice->canPlaySound(soundToPlay))
            {
                
            }
        }*/

        //return new KrumVoice();
    }

    return nullptr;
    
}

//juce::SynthesiserVoice* KrumSampler::quickAddVoice(KrumVoice* newVoice)
//{
//    return addVoice(newVoice);;
//}


KrumModule* KrumSampler::getModule(int index)
{
    return modules[index];
}

void KrumSampler::addModule(KrumModule* newModule, bool addVoice)
{
    modules.insert(newModule->getModuleIndex(), std::move(newModule));

    //in some cases you just want to add the module, but leave it not configured
    if (addVoice)
    {
        updateModule(newModule);
    }

    /*DBG("Sampler---");
    DBG("Num Voices: " + juce::String(getNumVoices()));
    DBG("Num Sounds: " + juce::String(getNumSounds()));*/
}

void KrumSampler::removeModule(KrumModule* moduleToDelete)
{
    //Im not certain this index will stay the same between the different arrays....
    int index = 0;
    for (int i = 0; i < getNumModules(); i++)
    {
        if (modules[i] == moduleToDelete)
        {
            modules.remove(i, true);
            index = i;
        }
    }

    //modules.removeObject(moduleToDelete);
    
    removeVoice(index);
    removeSound(index);
    //removeSound(++index);
    
    

    //updating the module's knowledge of it's own index from the removal upwards
    for (int i = index; i < getNumModules(); i++)
    {
        auto mod = modules[i];
        mod->setModuleIndex(i);
        mod->reassignSliders();
    }

    owner.updateValueTreeState();
}
 
void KrumSampler::updateModule(KrumModule* updatedModule)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(updatedModule->getSampleFile()));
    juce::BigInteger range;
    range.setBit(updatedModule->getMidiTriggerNote());

    if (reader == nullptr)
    {
        //removeModule(updatedModule);
        DBG("Format Not Supported");
        return;
    }

    for (int i = 0; i < getNumModules(); i++)
    {
        if (updatedModule == modules[i])
        {
            //adding an extra voice for overlapping notes
            addVoice(new KrumVoice());
            //addVoice(new KrumVoice(updatedModule));
            addSound(new KrumSound(updatedModule, updatedModule->getModuleName(), *reader , range, updatedModule->getMidiTriggerNote(), 
                                    attackTime, releaseTime, maxFileLengthInSeconds));
        }
    }
}

void KrumSampler::clearModules()
{
    modules.clear();
    voices.clear();
    sounds.clear();

    DBG("Sounds Size: " + juce::String(sounds.size()));
    
}

int KrumSampler::getNumModules()
{
    return modules.size();
}

juce::AudioFormatManager& KrumSampler::getFormatManager()
{

    return formatManager;

}