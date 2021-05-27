/*
  ==============================================================================

    KrumModuleProcessor.h
    Created: 30 Apr 2021 10:21:59am
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


//class KrumModuleEditor;
class KrumSampler;
class KrumModule;

class KrumModuleProcessor : public juce::Timer
{
public:
    KrumModuleProcessor(KrumModule& p, KrumSampler& s/*, juce::File& audioFile*/);
    //KrumModuleProcessor(KrumModule& p, KrumSampler& s);
    
    void timerCallback() override;
    

    
    void triggerNoteOn();
    void triggerNoteOff();
    

private:

    friend class KrumModuleEditor;
    friend class KrumModule;

    KrumModule& parent;
    KrumSampler& sampler;

    //juce::File audioFile;
    std::atomic<float>* moduleGain = nullptr;
    std::atomic<float>* modulePan = nullptr;

    float buttonClickVelocity = 15.0f;


};