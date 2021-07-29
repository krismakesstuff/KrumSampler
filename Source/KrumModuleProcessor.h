/*
  ==============================================================================

    KrumModuleProcessor.h
    Created: 30 Apr 2021 10:21:59am
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


/*
* 
* The audio engine of KrumModule. This class interfaces with the sampler class for trigger notes and is Owned by the KrumModule parent.
* 
*/


class KrumSampler;
class KrumModule;

class KrumModuleProcessor : public juce::Timer
{
public:
    KrumModuleProcessor(KrumModule& p, KrumSampler& s);
    
    void timerCallback() override;
    
    void triggerNoteOn();
    void triggerNoteOff();
    
private:

    friend class KrumModuleEditor;
    friend class KrumModule;

    KrumModule& parent;
    KrumSampler& sampler;

    std::atomic<float>* moduleGain = nullptr;
    std::atomic<float>* modulePan = nullptr;

    //float moduleGain;
    //float modulePan;
    

    float buttonClickVelocity = 0.5f;


};