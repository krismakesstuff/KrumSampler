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

void KrumSound::setModulePlaying(bool isPlaying)
{
    parentModule->setModulePlaying(isPlaying);
}

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
        //Pitchshifting will be a thing at some point.
        /*pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            * sound->sourceSampleRate / getSampleRate();*/

        sourceSamplePosition = 0.0;

        //only storing this, will be applied in the render block
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

KrumSampler::KrumSampler(juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o)
    :formatManager(fm), owner(o)
{
}

KrumSampler::~KrumSampler()
{
    clearModules();
}

void KrumSampler::initVoices()
{
    for (int i = 0; i < MAX_VOICES; i++)
    {
        auto newVoice = voices.add(new KrumVoice());
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
            //the sampler only plays one shots, so no need to turn any voices off here.
        }
    }
}

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
    int index = moduleToDelete->getModuleIndex();
    modules.remove(index, true);
    sounds.remove(index);

    //updating the module's knowledge of it's own index from the removal upwards
    for (int i = index; i < getNumModules(); i++)
    {
        //grab the old values
        auto mod = modules[i];
        auto modGain = mod->getModuleGain()->load();
        auto modPan = mod->getModulePan()->load();
        
        //reassign the module with it's new index, which the slider attachments use, then give it back it's old values.
        mod->setModuleIndex(i);
        mod->reassignSliders();

        //this is my hacky way to easily delete modules and just reassign them on the next slider attachment. Will be making this better to have a realiable automation workflow.
        mod->setModuleGain(modGain);
        mod->setModulePan(modPan);
        mod->updateAudioAtomics();
    }

    owner.updateValueTreeState();
}
 
void KrumSampler::updateModuleSample(KrumModule* updatedModule)
{
    //if this module already has a sound, it removes the original, then adds then new one. 
    auto sound = sounds[updatedModule->getModuleIndex()];
    if (sound)
    {
        sounds.removeObject(sound);
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
                sounds.insert(moduleToAddSound->getModuleIndex(), new KrumSound(moduleToAddSound, moduleToAddSound->getModuleName(), *reader, range, moduleToAddSound->getMidiTriggerNote(),
                    attackTime, releaseTime, MAX_FILE_LENGTH_SECS));
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