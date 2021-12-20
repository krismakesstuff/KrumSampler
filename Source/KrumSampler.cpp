/*
  ==============================================================================

    KrumSampler.cpp
    Created: 1 Mar 2021 11:15:22am
    Author:  krisc

  ==============================================================================
*/

#include "KrumSampler.h"
#include "PluginProcessor.h"
#include "SimpleAudioPreviewer.h"


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
    auto* krumSound = dynamic_cast<KrumSound*>(sound);
    if (krumSound != nullptr)
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

PreviewSound::PreviewSound(SimpleAudioPreviewer* prev,const juce::String& name,
                            juce::AudioFormatReader& source,
                            const juce::BigInteger& midiNotes,
                            int midiNoteForNormalPitch,
                            double attackTimeSecs,
                            double releaseTimeSecs,
                            double maxSampleLengthSeconds)
    : SamplerSound(name, source, 0, midiNoteForNormalPitch, attackTimeSecs, releaseTimeSecs, maxSampleLengthSeconds),
     previewer(prev), sourceSampleRate(source.sampleRate)
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

}

PreviewSound::~PreviewSound()
{
}

std::atomic<float>* PreviewSound::getPreviewerGain() const
{
    return previewer->getCurrentGain();
}

//====================================================================================//

PreviewVoice::PreviewVoice()
{
}

PreviewVoice::~PreviewVoice()
{
}

bool PreviewVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    auto previewSound = dynamic_cast<PreviewSound*>(sound);
    if (previewSound != nullptr)
    {
        return true;
    }
    
    return false;
}

bool PreviewVoice::isVoiceActive() const
{
    return adsr.isActive();
}

void PreviewVoice::startNote(int /*midiNoteNumber*/, float /*velocity*/, juce::SynthesiserSound* s, int /*pitchWheel*/)
{
    //voiceActive = true;
    if (auto* sound = dynamic_cast<const PreviewSound*> (s))
    {
        sourceSamplePosition = 0.0;

        gain.store(*sound->getPreviewerGain());
        //gain = newGain;

        adsr.setSampleRate(sound->sourceSampleRate);
        adsr.setParameters(sound->params);

        adsr.noteOn();
    }
}

void PreviewVoice::stopNote(float velocity, bool allowTailOff)
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

void PreviewVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (auto* playingSound = static_cast<PreviewSound*> (getCurrentlyPlayingSound().get()))
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

            //gain is set in PreviewVoice::startNote()
            l *= gain * envelopeValue;
            r *= gain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            if (++sourceSamplePosition > playingSound->length)
            {
                stopNote(0.0f, false);
                break;
            }
        }
    }
}


//====================================================================================//
KrumSampler::KrumSampler(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts, juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o, SimpleAudioPreviewer& fp)
    :formatManager(fm), owner(o), filePreviewer(fp)
{
    //initModules(valTree, apvts);
    //initVoices();
    startTimerHz(30);
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

    for (int i = 0; i < NUM_PREVIEW_VOICES; i++)
    {
        auto newVoice = voices.add(new PreviewVoice());
        newVoice->setCurrentPlaybackSampleRate(getSampleRate());
    }
    

    juce::Logger::writeToLog("Voices Initialized: " + juce::String(voices.size()));
    printVoices();
    
}

void KrumSampler::noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) 
{
    for (auto* sound : sounds)
    {
        if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
        {
            if (KrumSound* krumSound = dynamic_cast<KrumSound*>(sound))
            {
                auto voice = findFreeVoice(krumSound, midiChannel, midiNoteNumber, true);
                startVoice(voice, krumSound, midiChannel, midiNoteNumber, velocity);
            }
        }
        //else if(auto previewSound = static_cast<PreviewSound*>(sound))
        //{
        //    //make custom voice stealing? 
        //}
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

//juce::SynthesiserVoice* KrumSampler::findFreeVoice(juce::SynthesiserSound* soundToPlay, int midiChannel, int midiNoteNumber, const bool stealIfNoneAvailable) const
//{
//    //copied from Juce findFreeVoice implementation mostly
//    const juce::ScopedLock sl(lock);
//
//    /*for (auto* voice : voices)
//        if ((!voice->isVoiceActive()) && voice->canPlaySound(soundToPlay))
//            return voice;*/
//
//
//
//    if (auto sound = static_cast<PreviewSound*>(soundToPlay))
//    {
//
//    }
//
//
//    for (int i = 0; i < voices.size() - NUM_PREVIEW_VOICES; i++)
//    {
//        auto* voice = voices[i];
//        if ((!voice->isVoiceActive()) && voice->canPlaySound(soundToPlay))
//        {
//            return voice;
//        }
//    }
//
//    
//
//
//    if (stealIfNoneAvailable)
//        return findVoiceToSteal(soundToPlay, midiChannel, midiNoteNumber);
//
//    return nullptr;
//}

KrumModule* KrumSampler::getModule(int index)
{
    return modules[index];
}

void KrumSampler::addModule(KrumModule* newModule)
{
    modules.insert(newModule->getModuleSamplerIndex(), std::move(newModule));
}

void KrumSampler::removeModuleSample(KrumModule* moduleToDelete/*, bool updateTree*/)
{
    
    KrumSound* ksound = nullptr;
    for (int i = 0; i < sounds.size(); i++)
    {
        auto sound = sounds.getObjectPointer(i);
        ksound = static_cast<KrumSound*>(sound);
        if (ksound && ksound->isParent(moduleToDelete))
        {
            sounds.removeObject(sound);
            DBG("sound removed");
            printSounds();
            return;
        }
        
    }

    DBG("no sounds removed");

}
 
void KrumSampler::updateModuleSample(KrumModule* updatedModule)
{
    //removes the currently assigned sound of the module, if none exist this function will do nothing
    removeModuleSample(updatedModule);
    addSample(updatedModule);
}

void KrumSampler::addSample(KrumModule* moduleToAddSound)
{
    //if (isFileAcceptable(moduleToAddSound->getSampleFile()))
    if(auto reader = getFormatReader(moduleToAddSound->getSampleFile()))
    {
        //std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(moduleToAddSound->getSampleFile()));
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

//----------------------------------------------------------------

void KrumSampler::addPreviewFile(juce::File& file)
{
    if (std::unique_ptr<juce::AudioFormatReader> reader = getFormatReader(file))
    {

        currentPreviewFile = file;
        removePreviewSound();

        sounds.add(new PreviewSound(&filePreviewer, file.getFullPathName(), *reader, {}, 0, 0.001, 0.001, MAX_FILE_LENGTH_SECS));
        
    }
    else
    {
        DBG("Preview Reader NULL");
    }

}

void KrumSampler::playPreviewFile()
{
    PreviewSound* soundToPlay = nullptr;

    for (auto* sound : sounds)
    {
        if (PreviewSound* previewSound = dynamic_cast<PreviewSound*>(sound))
        {
            if (previewSound->getName().compare(currentPreviewFile.getFullPathName()) == 0)
            {
                soundToPlay = previewSound;
                break;
            }
        }
    }

    for (auto* voice : voices)
    {
        PreviewVoice* previewVoice = dynamic_cast<PreviewVoice*>(voice);
        if (previewVoice != nullptr)
        {
            if (/*!previewVoice->isVoiceActive() && */previewVoice->canPlaySound(soundToPlay))
            {
                startVoice(previewVoice, soundToPlay, 0, 0, 1);
                //previewVoice->startNote(0, 1, soundToPlay, 0);  // midiNote = 0, velocity = 1, pitchwheel = 0 
                break;
            }
        }
    }

}

void KrumSampler::removePreviewSound()
{
    //this is basically a voice stealing function

    //PreviewVoice* v1 = nullptr;
    //PreviewVoice* v2 = nullptr;

    //for (auto* voice : voices)
    //{
    //    auto* previewVoice = dynamic_cast<PreviewVoice*>(voice);
    //    if (previewVoice != nullptr/* && previewVoice->isVoiceActive()*/)
    //    {
    //        if (v1 == nullptr)
    //        {
    //            v1 = previewVoice;
    //        }
    //        else if (v2 == nullptr)
    //        {
    //            v2 = previewVoice;
    //        }
    //    }
    //}

    //juce::SynthesiserSound::Ptr sound = nullptr;

    //if (v1 != nullptr && v2 != nullptr)
    //{
    //    if (v1->wasStartedBefore(*v2))
    //    {
    //        v1->stopNote(0, false);
    //        sound = v1->getCurrentlyPlayingSound();
    //    }
    //    else
    //    {
    //        v2->stopNote(0, false);
    //        sound = v2->getCurrentlyPlayingSound();
    //    }
    //}
    

    for (auto* voice : voices)
    {
        if (auto* previewVoice = dynamic_cast<PreviewVoice*>(voice))
        {
            previewVoice->stopNote(0, false);
            sounds.removeObject(previewVoice->getCurrentlyPlayingSound());
            break;
        }
    }

 /*   for (auto* sound : sounds)
    {
        if (auto* preveiwSound = dynamic_cast<PreviewSound*>(sound))
        {
            sounds.removeObject(sound);
        }
    }*/


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

std::unique_ptr<juce::AudioFormatReader> KrumSampler::getFormatReader(juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "File type not supported!", "The current supported file types are: " + formatManager.getWildcardForAllFormats() + ".");
        return nullptr;
    }

    if (reader->lengthInSamples / reader->sampleRate >= MAX_FILE_LENGTH_SECS)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "File Too Long!", "The maximum file length is " + juce::String(MAX_FILE_LENGTH_SECS) + " seconds.");
        return nullptr;
    }

    return std::move(reader);
}

juce::AudioFormatManager& KrumSampler::getFormatManager()
{
    return formatManager;
}

void KrumSampler::timerCallback()
{
    if (filePreviewer.wantsToPlayFile())
    {
        playPreviewFile();
        filePreviewer.setWantsToPlayFile(false);
    }
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

