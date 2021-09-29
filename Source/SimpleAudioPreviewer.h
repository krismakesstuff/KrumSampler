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
* This Class plays the audio file that is selected in the file browser
*/
class SimpleAudioPreviewer  :   public juce::Component,
                                public juce::SettableTooltipClient
{
public:

    SimpleAudioPreviewer(juce::AudioFormatManager* formatManager, juce::ValueTree& valueTree);
    ~SimpleAudioPreviewer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool isAutoPlayActive();

    void renderPreviewer(juce::AudioBuffer<float>& outputBuffer);

    void setGain();
    double getGain();

    juce::String toText(double value);
    float fromText(juce::String text);

    void updateBubbleComp(juce::Slider* slider, juce::Component* bubble);

    void loadFile(juce::File& fileToPreview);
    juce::AudioFormatManager* getFormatManager();

    void saveToggleState();
    bool getSavedToggleState();

    void savePreviewerGainState();
    float getSavedPreviewerGainState();

    void refreshSettings();

    bool wantsToPlayFile();
    void setWantsToPlayFile(bool wantsToPlay);

private:

    bool readyToPlayFile = false;

    int playBackSampleRate;

    juce::ToggleButton autoPlayToggle;
    juce::Slider volumeSlider;

    juce::ValueTree& valueTree;
    juce::File currentAudioFile;

    std::unique_ptr<juce::AudioFormatReader> currentFormatReader = nullptr;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource = nullptr;
    juce::AudioFormatManager* formatManager = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleAudioPreviewer)
};