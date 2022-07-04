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
* The KrumSound(juce::SamplerSound) is responsible for holding the audio data that is to be played back.
* The KrumVoice(juce::SamplerVoice) is responsible for rendering the audio from the KrumSound into the audio buffer. 
* The KrumSampler(juce::Synthesizer) handles the incoming midi and triggers the rendering of the KrumVoice.
* 
* There is also a dedicated voice for rendering the preview file. PreviewSound and PreviewVoice use slightly different methods of rendering then the it's Krum siblings
* see PreviewSound and PreviewVoice
* 
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
    
    std::atomic<int> getModuleStartSample() const;
    std::atomic<int> getModuleEndSample() const;

    std::atomic<float>* getModuleMute() const;
    std::atomic<float>* getModuleReverse() const;

    std::atomic<float>* getModulPitchShift() const;

    int getModuleOutputNumber() const;

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
    std::atomic<float> lgain = 0, rgain = 0, clipGain = 0;
    
    std::atomic<int> startSample, endSample;

    double sourceSamplePosition = 0;

    int outputChan = 0;
    juce::ADSR adsr;

    JUCE_LEAK_DETECTOR(KrumVoice)
};

//====================================================================================
class SimpleAudioPreviewer;
class PreviewVoice;

class PreviewSound : public juce::SamplerSound
{
public:
    PreviewSound(SimpleAudioPreviewer* previewer, const juce::String& name,
        juce::AudioFormatReader& source,
        const juce::BigInteger& midiNotes,
        int midiNoteForNormalPitch,
        double attackTimeSecs,
        double releaseTimeSecs,
        double maxSampleLengthSeconds);
    ~PreviewSound() override;

    std::atomic<float>* getPreviewerGain() const;

private:
    friend class PreviewVoice;

    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    int length = 0;

    juce::ADSR::Parameters params;

    SimpleAudioPreviewer* previewer = nullptr;

    JUCE_LEAK_DETECTOR(PreviewSound)
};

class PreviewVoice : public juce::SamplerVoice 
{
public: 
    PreviewVoice();
    ~PreviewVoice() override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    bool isVoiceActive() const override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:

    friend class SamplerSound;

    std::atomic<float> gain = 0;

    //std::atomic<bool> voiceActive = false;
    double sourceSamplePosition = 0;
    juce::ADSR adsr;

    JUCE_LEAK_DETECTOR(PreviewVoice)
};

//====================================================================================

class KrumSamplerAudioProcessor;

class KrumSampler : public juce::Synthesiser,
                    public juce::Timer
{
public:
    
    KrumSampler(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts, 
                juce::AudioFormatManager& fm, KrumSamplerAudioProcessor& o);
    ~KrumSampler() override;

    void initModules(juce::ValueTree* valTree, juce::AudioProcessorValueTreeState* apvts);
    void initVoices();

    void noteOn(const int midiChannel, const int midiNoteNumber, const float velocity) override;
    void noteOff(const int midiChannel, const int midiNoteNumber, const float veloctiy, bool allowTailOff) override;

    KrumModule* getModule(int index);

    //Only the processor should use this when rebuilding the sampler from the value tree
    void addModule(KrumModule* newModule);

    //if there is no sound that has this module as a parent, nothing will happen
    void removeModuleSample(KrumModule* moduleToDelete);

    //will remove the module's old sound(if it has one) and then add the sample set in the module
    void updateModuleSample(KrumModule* updatedModule);
    
    void clearModules();

    int getNumModules();
    void getNumFreeModules(int& totalFreeModules, int& firstFreeIndex);
    
    void addPreviewFile(juce::File& file);
    void playPreviewFile();

    void assignPreveiwer(SimpleAudioPreviewer* previewer);

    bool isFileAcceptable(const juce::File& file, juce::int64& numSamplesOfFile);

    juce::AudioFormatManager& getFormatManager();

private:
    
    void timerCallback()override;

    //makes a Krum Sound and adds it to the samplers sounds array, using the assigned file in the passed in module
    void addSample(KrumModule* moduleToAddSound);

    void removePreviewSound();

    //does the same thing as isFileAcceptable(), except returns the reader, will be nullptr if not acceptable
    std::unique_ptr<juce::AudioFormatReader> getFormatReader(juce::File& file);

    void printSounds();
    void printVoices();

    double attackTime = 0.01;
    double releaseTime = 0.01;

    juce::AudioFormatManager& formatManager;
    KrumSamplerAudioProcessor& owner;

    juce::OwnedArray<KrumModule> modules;

    SimpleAudioPreviewer* filePreviewer = nullptr;
    juce::File currentPreviewFile;

    JUCE_LEAK_DETECTOR(KrumSampler)
};
