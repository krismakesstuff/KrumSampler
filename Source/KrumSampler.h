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
* - Need a solution to reliably delete modules and not have it affect other modules sliderattachments
*    - Reassigning slider attachments makes automation tricky/impossible. 
*/

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
    std::atomic<float>* getModuleClipGain()const;

    //void setModulePlaying(bool playing);
    bool isParent(KrumModule* moduleToTest);

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
    std::atomic<float> lgain = 0, rgain = 0, clipGain = 0;
    
    juce::ADSR adsr;

    JUCE_LEAK_DETECTOR(KrumVoice)
};

class KrumSamplerAudioProcessor;

class KrumSampler : public juce::Synthesiser
{
public:
    KrumSampler(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts, juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o);
    ~KrumSampler() override;

    void initModules(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);
    
    void initVoices();

    void noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) override;
    void noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) override;

    KrumModule* getModule(int index);
    void addModule(KrumModule* newModule);
    
    //if there is no sound that has this module as a parent, nothing will happen
    void removeModuleSound(KrumModule* moduleToDelete/*, bool updateTree = true*/);
    //will remove the modules current sound(if it has one) and then add the sample set in the module
    void updateModuleSample(KrumModule* updatedModule);
    //makes a Krum Sound and adds it to the samplers sounds array, using the assigned file in the passed in module
    void addSample(KrumModule* moduleToAddSound);
    
    void clearModules();
    int getNumModules();

    void getNumFreeModules(int& totalFreeModules, int& firstFreeIndex);
    
    bool isFileAcceptable(const juce::File& file);
    juce::AudioFormatManager& getFormatManager();

private:

    void printSounds();
    void printVoices();

    double attackTime = 0.01;
    double releaseTime = 0.01;

    juce::AudioFormatManager& formatManager;
    KrumSamplerAudioProcessor& owner;

    juce::OwnedArray<KrumModule> modules;

    JUCE_LEAK_DETECTOR(KrumSampler)
};
