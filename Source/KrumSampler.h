/*
  ==============================================================================

    KrumSampler.h
    Created: 20 Feb 2021 5:33:40pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "KrumModule.h"

/*
* 
* The Sampler is comprised of three classes: juce::SamplerSound, juce::SamplerVoice and juce::Synthesizer.
* 
* The KrumSound(juce::SamplerSound) is responsible for holding the audio file that is to be played back.
* The KrumVoice(juce::SamplerVoice) is responsible for rendering the audio from the SamplerSound into the audio buffer. 
* The KrumSampler(juce::Synthesizer) handles the incoming midi and triggers the rendering of the KrumVoice.
* 
* TODO:
* - Come up with a voice stealing scheme. We want files to continue to play even if the same note is triggered again. So the sound overlap and don't create sudden endings and potentially a click
* 
*/

#define MAX_VOICES 8
#define MAX_FILE_LENGTH_SECS 5

class KrumSound : public juce::SamplerSound
{
public:
    KrumSound   (KrumModule* parentModule, const juce::String& name,
                juce::AudioFormatReader& source,
                const juce::BigInteger& midiNotes,
                int midiNoteForNormalPitch,
                double attackTimeSecs,
                double releaseTimeSecs,
                double maxSampleLengthSeconds);
    ~KrumSound() override;
    
    std::atomic<float>* getModuleGain()const;
    std::atomic<float>* getModulePan()const;

    void setModulePlaying(bool playing);

private:
    friend class KrumVoice;


    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int length = 0, midiRootNote = 0, midiChannel = 0;
    

    juce::ADSR::Parameters params;
    KrumModule* parentModule = nullptr;

    JUCE_LEAK_DETECTOR(KrumSound)
};




class KrumVoice : public juce::SamplerVoice
{
public:
    KrumVoice();
    ~KrumVoice() override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    bool isVoiceActive() const override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:

    friend class juce::SamplerSound;

    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    std::atomic<float> lgain = 0, rgain = 0;

    juce::ADSR adsr;


    JUCE_LEAK_DETECTOR(KrumVoice)
};


class KrumSamplerAudioProcessor;

class KrumSampler : public juce::Synthesiser
{
public:
    KrumSampler(juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o);
    ~KrumSampler() override;

    void initVoices();

    void noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) override;
    void noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) override;
    
    /*juce::SynthesiserVoice* findFreeVoice(juce::SynthesiserSound* soundToPlay,
                                            int midiChannel,
                                            int midiNoteNumber,
                                            bool stealIfNoneAvailable) const override;*/

    //juce::SynthesiserVoice* findVoiceToSteal(juce::SynthesiserSound* sound, int midiChannel, int midiNoteNumber)const override;


    //void handleMidiEvent(const juce::MidiMessage& midiMessage) override;

    KrumModule* getModule(int index);
    void addModule(KrumModule* newModule, bool addVoice = false);
    void removeModule(KrumModule* moduleToDelete);
    void updateModule(KrumModule* updatedModule);
    
    void clearModules();
    int getNumModules();

    juce::AudioFormatReader* isFileAcceptable(const juce::File& file);

    juce::AudioFormatManager& getFormatManager();

private:

    juce::CriticalSection lock;

    bool treeNeedsCleaning = false;

    double attackTime = 0.01;
    double releaseTime = 0.01;
    double maxFileLengthInSeconds = 5;

    juce::AudioFormatManager& formatManager;
    KrumSamplerAudioProcessor& owner;

    juce::OwnedArray<KrumModule> modules;

    JUCE_LEAK_DETECTOR(KrumSampler)

};