/*
  ==============================================================================

    KrumSampler.cpp
    Created: 1 Mar 2021 11:15:22am
    Author:  krisc

  ==============================================================================
*/

#include "KrumSampler.h"
#include "PluginProcessor.h"
#include "UI\FileBrowser\SimpleAudioPreviewer.h"


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

std::atomic<int> KrumSound::getModuleStartSample() const
{
    return parentModule->getModuleStartSample();
}

std::atomic<int> KrumSound::getModuleEndSample() const
{
    return parentModule->getModuleEndSample();
}

std::atomic<float>* KrumSound::getModuleMute() const
{
    return parentModule->getModuleMute();
}

std::atomic<float>* KrumSound::getModuleReverse() const
{
    return parentModule->getModuleReverse();
}

std::atomic<float>* KrumSound::getModulPitchShift() const
{
    return parentModule->getModulePitchShift();
}

int KrumSound::getModuleOutputNumber() const
{
    return parentModule->getModuleOutputChannelNumber();
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
        if (*sound->getModuleMute() < 0.5f)
        {
            pitchRatio = std::pow(2.0, ((*sound->getModulPitchShift() / 12.0)
                                    * sound->sourceSampleRate / getSampleRate()));

            outputChan = sound->getModuleOutputNumber() - 1; //index offset

            startSample = sound->getModuleStartSample().load();
            endSample = sound->getModuleEndSample().load();

            if (*sound->getModuleReverse() < 0.5f)
            {
                sourceSamplePosition = startSample;
            }
            else
            {
                sourceSamplePosition = endSample;
            }

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
        else
        {
            stopNote(velocity, true);
        }
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

void KrumVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int firstSample, int numSamples)
{
    if (auto* playingSound = static_cast<KrumSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->getAudioData();
        const float* const inL = data.getReadPointer(0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

        int numChannels = outputBuffer.getNumChannels();

        float* outL = nullptr;
        float* outR = nullptr;
        
        if (numChannels >= outputChan + 1) // + 1 accounts for stereo pair
        {
            outL = outputBuffer.getWritePointer(outputChan, firstSample);
            outR = outputBuffer.getWritePointer(outputChan + 1, firstSample);
        }

        if(outL == nullptr || outR == nullptr) // just use main output bus if we can't get the busses we want
        {
            outL = outputBuffer.getWritePointer(0, firstSample);
            outR = numChannels > 1 ? outputBuffer.getWritePointer(1, firstSample) : nullptr;
        }

        if (*playingSound->getModuleReverse() > 0.5f)
        {
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

                sourceSamplePosition -= pitchRatio;
                //--sourceSamplePosition;
                if (sourceSamplePosition > playingSound->length || sourceSamplePosition < startSample)
                {
                    stopNote(0.0f, false);
                    break;
                }
            }
        }
        else
        {
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

                sourceSamplePosition += pitchRatio;
                //++sourceSamplePosition;
                if (sourceSamplePosition > playingSound->length || sourceSamplePosition > endSample)
                {
                    stopNote(0.0f, false);
                    break;
                }
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
KrumSampler::KrumSampler(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts, juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o)
    :formatManager(fm), owner(o)/*, filePreviewer(fp)*/
{
    //startTimerHz(30);
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
    }
}

void KrumSampler::noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) 
{
    //the sampler only plays one shots FOR NOW, so no need to turn any voices off here.
    //The voice will take care of any "turning off" when the data is done rendering.

}

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
    juce::File sampleFile = moduleToAddSound->getSampleFile();
    if(auto reader = getFormatReader(sampleFile))
    {
        moduleToAddSound->setNumSamplesInFile(reader->lengthInSamples);
        juce::BigInteger range;
        range.setBit(moduleToAddSound->getMidiTriggerNote());

        sounds.add(new KrumSound(moduleToAddSound, moduleToAddSound->getModuleName(), *reader, range, moduleToAddSound->getMidiTriggerNote(),
                            attackTime, releaseTime, MAX_FILE_LENGTH_SECS));

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
    firstFreeIndex = -1; //-1 indicates the value has not been changed yet
    
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

    DBG("Free Modules: " + juce::String(totalFreeModules) + ", First Index " + juce::String(firstFreeIndex));
}

//----------------------------------------------------------------

void KrumSampler::addPreviewFile(juce::File& file)
{
    if (std::unique_ptr<juce::AudioFormatReader> reader = getFormatReader(file))
    {
        currentPreviewFile = file;
        removePreviewSound();

        sounds.add(new PreviewSound(filePreviewer, file.getFullPathName(), *reader, {}, 0, 0.001, 0.001, MAX_FILE_LENGTH_SECS));
        
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
            if (previewVoice->canPlaySound(soundToPlay))
            {
                startVoice(previewVoice, soundToPlay, 0, 0, 1); // midiNote = 0, velocity = 1, pitchwheel = 0 
                break;
            }
        }
    }

}

void KrumSampler::assignPreveiwer(SimpleAudioPreviewer* previewerToUse)
{
    filePreviewer = previewerToUse;
    startTimerHz(30);
}

void KrumSampler::removePreviewSound()
{
    for (auto* voice : voices)
    {
        if (auto* previewVoice = dynamic_cast<PreviewVoice*>(voice))
        {
            previewVoice->stopNote(0, false);
            sounds.removeObject(previewVoice->getCurrentlyPlayingSound());
            break;
        }
    }
}

bool KrumSampler::isFileAcceptable(const juce::File& file, juce::int64& numSamplesOfFile)
{
    numSamplesOfFile = 0;
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

    numSamplesOfFile = reader->lengthInSamples;
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
    if (owner.getActiveEditor() != nullptr && filePreviewer->wantsToPlayFile())
    {
        playPreviewFile();
        filePreviewer->setWantsToPlayFile(false);
    }
}

void KrumSampler::printSounds()
{
    DBG("Sounds Size = " + juce::String(sounds.size()));
    for (int i = 0; i < sounds.size(); i++)
    {
        auto sound = sounds[i];
        DBG("Sound: " + juce::String(i) + (sound ? " is valid" : " is NULL"));
    }
}

void KrumSampler::printVoices()
{
    DBG("Voices Size = " + juce::String(voices.size()));
    for (int i = 0; i < voices.size(); i++)
    {
        auto voice = voices[i];
        DBG("Voice: " + juce::String(i) + (voice ? " is valid" : " is NULL"));
    }
}

