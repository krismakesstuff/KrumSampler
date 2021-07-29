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

KrumModuleProcessor::KrumModuleProcessor(KrumModule& p, KrumSampler& s/*, juce::File& sampleFile*/)
    : parent(p), sampler(s)/*, audioFile(sampleFile)*/
{}

//automatically triggers a note off after the length of the sample has passed.
void KrumModuleProcessor::timerCallback()
{
    triggerNoteOff();
}

void KrumModuleProcessor::triggerNoteOn()
{
    //int timerLength = parent.moduleEditor->getAudioFileLengthInMs();
    
    sampler.noteOn(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity);
    //startTimer(timerLength);
}

void KrumModuleProcessor::triggerNoteOff()
{
    //stopTimer();
    //sampler.noteOff(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity, false);
}