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

    //int getMidiRootNote();
    //double getSourceSampleRate();
    //juce::ADSR::Parameters getADSRParams();
    //juce::AudioBuffer<float>& getAudioData();

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
    KrumVoice(/*KrumModule* attachedModule*/);
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

    void noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) override;
    void noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) override;
    
    juce::SynthesiserVoice* findFreeVoice(juce::SynthesiserSound* soundToPlay,
                                            int midiChannel,
                                            int midiNoteNumber,
                                            bool stealIfNoneAvailable) const override;

    //juce::SynthesiserVoice* quickAddVoice(KrumVoice* newVoice);
                                            
    //void renderVoices(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) override;

    KrumModule* getModule(int index);
    void addModule(KrumModule* newModule, bool addVoice = false);
    void removeModule(KrumModule* moduleToDelete);
    void updateModule(KrumModule* updatedModule);
    
    void clearModules();

    int getNumModules();

    //void setModulePlaying(juce::String moduleName, bool isPlaying);

    juce::AudioFormatManager& getFormatManager();

private:

    juce::CriticalSection lock;

    bool treeNeedsCleaning = false;

    double attackTime = 0.01;
    double releaseTime = 0.01;
    double maxFileLengthInSeconds = 5;

    juce::OwnedArray<KrumModule> modules;
    juce::AudioFormatManager& formatManager;
    KrumSamplerAudioProcessor& owner;

    JUCE_LEAK_DETECTOR(KrumSampler)

};