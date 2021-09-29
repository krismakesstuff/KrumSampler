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

class KrumModuleProcessor 
{
public:

    KrumModuleProcessor(KrumModule& p, KrumSampler& s);
    
    void triggerNoteOn();
    void triggerNoteOff();
    
private:

    friend class KrumModuleEditor;
    friend class KrumModule;
    friend class DragAndDropThumbnail;
    KrumModule& parent;
    KrumSampler& sampler;

    std::atomic<float>* moduleGain = nullptr;
    std::atomic<float>* modulePan = nullptr;
    std::atomic<float>* moduleClipGain = nullptr;

    float buttonClickVelocity = 0.5f;
};