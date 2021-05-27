/*
  ==============================================================================

    MidiState.h
    Created: 24 May 2021 7:14:37pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#pragma once
class MidiState : public juce::Thread
{
public:

    MidiState()
        :juce::Thread("MidiState")
    {
        startThread(8);
    }

    void run() override
    {
        if (threadShouldExit())
        {
            return;
        }
        
        if(newListenerQ.size() > 0 || removeListenerQ.size() > 0)
        {
            runQue();
        }

        if (!buffer.isEmpty())
        {
            processMidi();
        }

    }

    void copyState(juce::MidiBuffer& midiMessages, int sSample, int nSamples, bool injectEvents)
    {
        buffer = midiMessages;
        startSample = sSample;
        numSamples = nSamples;
        injectDirectEvents = injectEvents;

        if (!buffer.isEmpty())
        {
            notify();
        }
    }

    void processMidi()
    {
        processing = true;
        state.processNextMidiBuffer(buffer, startSample, numSamples, injectDirectEvents);
        processing = false;

        if (threadShouldExit())
        {
            return;
        }

        threadShouldExit();
        /*if (!isThreadRunning() && (newListenerQ.size() > 0 || removeListenerQ.size() > 0))
        {
            notify();
        }*/


        
    }


    void removeListener(juce::MidiKeyboardStateListener* listenerToRemove)
    {
        if (!processing)
        {
            state.removeListener(listenerToRemove);
        }
        else
        {
            removeListenerQ.add(listenerToRemove);
            notify();
        }
    }

    void addListener(juce::MidiKeyboardStateListener* newListener)
    {
        if (!processing)
        {
            state.addListener(newListener);
        }
        else
        {
            newListenerQ.add(newListener);
            notify();
        }
    }
    
    void runQue()
    {
        
        for (int i = 0; i < newListenerQ.size(); i++)
        {
            state.addListener(newListenerQ[i]);
        }

        for (int i = 0; i < removeListenerQ.size(); i++)
        {
            state.removeListener(removeListenerQ[i]);
        }

        newListenerQ.clear(false);
        removeListenerQ.clear(true);

        if(buffer.isEmpty())
        {
            signalThreadShouldExit();
        }

    }

    juce::MidiKeyboardState& getMidiKeyboardState()
    {
        return state;
    }



private:

    juce::CriticalSection critSection;

    juce::MidiKeyboardState state;
    bool processing = false;

    juce::MidiBuffer buffer{};
    int startSample = 0;
    int numSamples = 0;
    bool injectDirectEvents = true;


    juce::OwnedArray<juce::MidiKeyboardStateListener> newListenerQ;
    juce::OwnedArray<juce::MidiKeyboardStateListener> removeListenerQ;


};