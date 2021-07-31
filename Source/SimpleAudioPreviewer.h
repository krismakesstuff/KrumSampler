/*
  ==============================================================================

    SimpleAudioPreviewer.h
    Created: 2 Apr 2021 3:29:00pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
* This Class actually plays the audio file that is selected in the file browser
*/
class SimpleAudioPreviewer  :   public juce::Component,
                                public juce::SettableTooltipClient
{
public:

    enum PreviewerState
    {
        stopped,
        playing,
        //paused
    };


    SimpleAudioPreviewer(juce::AudioFormatManager& formatManager, juce::ValueTree& valueTree);
    ~SimpleAudioPreviewer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool isAutoPlayActive();

    void playOrStop();

    void setGain();
    double getGain();

    juce::String toText(double value);
    float fromText(juce::String text);

    void updateBubbleComp(juce::Slider* slider, juce::Component* bubble);

    void loadFile(juce::File& fileToPreview);
    juce::AudioFormatManager& getFormatManager();

    void saveToggleState();
    bool getSavedToggleState();

    void savePreviewerGainState();
    float getSavedPreviewerGainState();

    void refreshSettings();

private:

    juce::CriticalSection lock;

    juce::ToggleButton autoPlayToggle;
    juce::Slider volumeSlider;

    juce::ValueTree& valueTree;

    int readAheadBufferSize = 32768;
    PreviewerState state{ PreviewerState::stopped };
    

    juce::File currentAudioFile;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;

    juce::AudioFormatManager& formatManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    juce::AudioDeviceManager audioDeviceManager;

    juce::TimeSliceThread previewThread{"AudioPreviewThread"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleAudioPreviewer)
};
