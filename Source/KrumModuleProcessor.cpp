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

KrumModuleProcessor::KrumModuleProcessor(KrumModule& p, KrumSampler& s)
    : parent(p), sampler(s)
{
}

void KrumModuleProcessor::triggerNoteOn()
{
    sampler.noteOn(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity);
}

void KrumModuleProcessor::triggerNoteOff()
{
    sampler.noteOff(parent.getMidiTriggerChannel(), parent.getMidiTriggerNote(), buttonClickVelocity, false);
}