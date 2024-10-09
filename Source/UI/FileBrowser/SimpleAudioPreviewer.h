/*
  ==============================================================================

    SimpleAudioPreviewer.h
    Created: 2 Apr 2021 3:29:00pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../InfoPanel.h"

//==============================================================================
/*
* This Class plays the audio file that is selected in the file browser. 
* You MUST assign this the sampler using assignSampler() before it will work.
* There is a dedicated voice in the sampler for rendering the file. 
* To give a file to the sampler, simply use loadFile(juce::File), then call setWantsToPlayFile(true);
* The sampler will check if this wants to play in it's timerCallback().
* 
*/

class KrumSampler;

class SimpleAudioPreviewer  :   public juce::Component,
                                public juce::SettableTooltipClient
{
public:

    SimpleAudioPreviewer(juce::AudioFormatManager* formatManager, juce::ValueTree& valueTree, juce::AudioProcessorValueTreeState& apvts);
    ~SimpleAudioPreviewer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isAutoPlayActive();

    void setCurrentGain();
    std::atomic<float>* getCurrentGain();

    juce::String toText(double value);
    float fromText(juce::String text);

    void loadFile(juce::File fileToPreview);
    juce::AudioFormatManager* getFormatManager();

    void saveToggleState();
    bool getSavedToggleState();

    void savePreviewerGainState();
    float getSavedPreviewerGainState();

    void refreshSettings();

    juce::File& getCurrentFile();
    bool wantsToPlayFile();
    void setWantsToPlayFile(bool wantsToPlay);

    void assignSampler(KrumSampler* samplerToAssign);

private:

    void updateBubbleComp(juce::Slider* slider, juce::Component* bubble);

    std::atomic<bool> readyToPlayFile = false;
    std::atomic<bool> rendering = false;
    std::atomic<float> currentGain = 0.0f;
    bool newFileWaiting = false;

    int playBackSampleRate;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    InfoPanelToggleButton autoPlayToggle {"Auto-Play", "If unchecked, files will play when they are double-clicked. If this is checked, the will play as they are selected"};
    InfoPanelSlider volumeSlider {"Preview Volume", "This is the volume that files are previewed at", "Snap To Default by double-clicking or CMD + click the slider"};
    std::unique_ptr<SliderAttachment> volumeSliderAttachment;

    juce::ValueTree& valueTree;
    juce::AudioProcessorValueTreeState& apvts;
    juce::File currentAudioFile{};

    std::unique_ptr<juce::AudioFormatReader> currentFormatReader = nullptr;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource = nullptr;
    juce::AudioFormatManager* formatManager = nullptr;
    KrumSampler* sampler = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleAudioPreviewer)
};
