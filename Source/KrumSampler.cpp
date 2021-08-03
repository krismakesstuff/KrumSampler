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

//float KrumSound::getModuleGain() const
std::atomic<float>* KrumSound::getModuleGain() const
{
    return parentModule->getModuleGain();
}

//float KrumSound::getModulePan() const
std::atomic<float>* KrumSound::getModulePan() const
{
    return parentModule->getModulePan();
}

void KrumSound::setModulePlaying(bool isPlaying)
{
    parentModule->setModulePlaying(isPlaying);
}

//void KrumSound::setMidi(int newMidiNote, int newMidiChannel)
//{
//    midiRootNote = newMidiNote;
//    midiChannel = newMidiChannel;
//    midiNotes.setBit(midiRootNote);
//
//
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

        //std::atomic<float>* moduleGain = sound->getModuleGain();
        //std::atomic<float>* modulePan = sound->getModulePan();
        
       // const juce::ScopedLock sl(voiceLock);
        
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
    if (auto* playingSound = static_cast</*juce::Sampler*/KrumSound*> (getCurrentlyPlayingSound().get()))
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

KrumSampler::KrumSampler(juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o)
    :formatManager(fm), owner(o)
{
    formatManager.registerBasicFormats();

    

}
KrumSampler::~KrumSampler()
{
    clearModules();
}

void KrumSampler::initVoices()
{
    for (int i = 0; i < MAX_VOICES; i++)
    {
        auto newVoice = addVoice(new KrumVoice());
        newVoice->setCurrentPlaybackSampleRate(getSampleRate());
    }

    DBG("Voices: " + juce::String(voices.size()));
}


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

            //one shot so we don't need this loop.
            /*for (auto* voice : voices) 
            {
                if (    (voice->getCurrentlyPlayingNote() == midiNoteNumber) &&
                         voice->isPlayingChannel(midiChannel) &&
                        (voice->getCurrentlyPlayingSound() == sound)
                   )
                {
                    stopVoice(voice, 0.0f, true);
                }
            }*/
        }
    }
}

//juce::SynthesiserVoice* KrumSampler::findFreeVoice  (juce::SynthesiserSound* soundToPlay, int midiChannel, int midiNoteNumber, bool stealIfNoneAvailable) const
//{
//    const juce::ScopedLock sl(lock);
//
//    for (int i = 0; i < voices.size(); i++)
//    {
//        auto voice = voices[i]; 
//        if (voice->canPlaySound(soundToPlay) && !voice->isVoiceActive())
//        {
//           
//            return voice; 
//            
//        }
//    }
//
//    if (stealIfNoneAvailable)
//    {
//        findVoiceToSteal()
//    }
//
//    return nullptr;
//    
//}

//juce::SynthesiserVoice* KrumSampler::findVoiceToSteal(juce::SynthesiserSound* sound, int midiChannel, int midiNoteNumber) const
//{
//    
//
//
//
//    return nullptr;
//}

//
//void KrumSampler::handleMidiEvent(const juce::MidiMessage& m)
//{
//    const int channel = m.getChannel();
//
//    if (m.isNoteOn())
//    {
//        noteOn(channel, m.getNoteNumber(), m.getFloatVelocity());
//    }
//    else if (m.isNoteOff())
//    {
//        noteOff(channel, m.getNoteNumber(), m.getFloatVelocity(), true);
//    }
//    else if (m.isAllNotesOff() || m.isAllSoundOff())
//    {
//        allNotesOff(channel, true);
//    }
//    else if (m.isPitchWheel())
//    {
//        const int wheelPos = m.getPitchWheelValue();
//        lastPitchWheelValues[channel - 1] = wheelPos;
//        handlePitchWheel(channel, wheelPos);
//    }
//    else if (m.isAftertouch())
//    {
//        handleAftertouch(channel, m.getNoteNumber(), m.getAfterTouchValue());
//    }
//    else if (m.isChannelPressure())
//    {
//        handleChannelPressure(channel, m.getChannelPressureValue());
//    }
//    else if (m.isController())
//    {
//        handleController(channel, m.getControllerNumber(), m.getControllerValue());
//    }
//    else if (m.isProgramChange())
//    {
//        handleProgramChange(channel, m.getProgramChangeNumber());
//    }
//}

KrumModule* KrumSampler::getModule(int index)
{
    return modules[index];
}

void KrumSampler::addModule(KrumModule* newModule, bool hasSample)
{
    if (voices.size() == 0)
    {
        initVoices();
    }

    modules.insert(newModule->getModuleIndex(), std::move(newModule));

    //in some cases you just want to add the module, but leave it with no sample
    if (hasSample)
    {
        addSample(newModule);
    }

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

    //removeVoice(index);
    removeSound(index);
    
    //updating the module's knowledge of it's own index from the removal upwards
    for (int i = index; i < getNumModules(); i++)
    {
        auto mod = modules[i];
        mod->setModuleIndex(i);
        mod->reassignSliders();
    }

    owner.updateValueTreeState();
}
 
void KrumSampler::updateModuleSample(KrumModule* updatedModule)
{
    int indexToRemove = -1;
    for (int i = 0; i < sounds.size(); i++)
    {
        if (auto krumSound = static_cast<KrumSound*>(sounds[i].get()))
        {
            if (krumSound->isParent(updatedModule))
            {
                indexToRemove = i;
                //krumSound->setMidi(updatedModule->getMidiTriggerNote(), updatedModule->getMidiTriggerChannel());
            }
        }
    }

    if (indexToRemove >= 0)
    {
        sounds.remove(indexToRemove);
    }
    
    addSample(updatedModule);

}

void KrumSampler::addSample(KrumModule* moduleToAddSound)
{
    if (isFileAcceptable(moduleToAddSound->getSampleFile()))
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(moduleToAddSound->getSampleFile()));
        juce::BigInteger range;
        range.setBit(moduleToAddSound->getMidiTriggerNote());

        for (int i = 0; i < getNumModules(); i++)
        {
            if (moduleToAddSound == modules[i])
            {
                addSound(new KrumSound(moduleToAddSound, moduleToAddSound->getModuleName(), *reader, range, moduleToAddSound->getMidiTriggerNote(),
                        attackTime, releaseTime, maxFileLengthInSeconds));
            }
        }

    }

    DBG("Sounds: " + juce::String(sounds.size()));
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

bool KrumSampler::isFileAcceptable(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        auto alert = juce::AlertWindow::showNativeDialogBox("File type not supported!", "The current supported file types are: " + formatManager.getWildcardForAllFormats() + ".", true);
        return false;
    }
    if (reader->lengthInSamples / reader->sampleRate >= MAX_FILE_LENGTH_SECS)
    {
        auto alert = juce::AlertWindow::showNativeDialogBox("File Too Long!", "The maximum file length is " + juce::String(MAX_FILE_LENGTH_SECS) + " seconds.", true);
        return false;
    }

    return true;
}

juce::AudioFormatManager& KrumSampler::getFormatManager()
{
    return formatManager;
}