/*
  ==============================================================================

    KrumModuleProcessor.cpp
    Created: 30 Apr 2021 10:21:59am
    Author:  krisc

  ==============================================================================
*/

#include "KrumModuleProcessor.h"
#include "KrumModule.h"
#include "KrumSampler.h"

//#include "KrumModuleEditor.h"


KrumModuleProcessor::KrumModuleProcessor(KrumModule& p, KrumSampler& s/*, juce::File& sampleFile*/)
    : parent(p), sampler(s)/*, audioFile(sampleFile)*/
{
    
}


void KrumModuleProcessor::timerCallback()
{
    triggerNoteOff();
}

void KrumModuleProcessor::triggerNoteOn()
{
    int timerLength = parent.moduleEditor->getAudioFileLengthInMs();
    
    sampler.noteOn(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity);
    startTimer(timerLength);
}

void KrumModuleProcessor::triggerNoteOff()
{
    sampler.noteOff(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity, false);
}