/*
  ==============================================================================

    Log.h
    Created: 17 Oct 2021 10:44:52am
    Author:  Kris Crawford

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Log
{
    static juce::String logFolderName {"KrumSampler"};
    static juce::String logFileName{"Log"};
    static juce::String logFileExtension{".txt"};
    static juce::String welcomeMessage{"KrumSampler started, now logging"};

    //static juce::FileLogger* logger = juce::FileLogger::createDateStampedLogger (Log::logFolderName, Log::logFileName, Log::logFileExtension, Log::welcomeMessage);
        
}

//would like the log to capture the function name eventually, this is probably wrong
//namespace juce
//{
//    const juce::String formatLogMessage(juce::String& functionName, juce::String& message, bool includeTimeStamp = true)
//    {
//        if(includeTimeStamp)
//        {
//            juce::String time = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
//            return time + " - Function[" + functionName +"]: " + message;
//        }
//        else
//        {
//            return "Function[" + functionName +"]: " + message;
//        }
//    }
//}
//
