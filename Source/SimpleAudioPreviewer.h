/*
  ==============================================================================

    SimpleAudioPreviewer.h
    Created: 2 Apr 2021 3:29:00pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InfoPanel.h"

//==============================================================================
/*
* This Class plays the audio file that is selected in the file browser
*/
class SimpleAudioPreviewer  :   public juce::Component,
                                public juce::SettableTooltipClient,
                                public juce::Timer
                                //public InfoPanelComponent
{
public:

    SimpleAudioPreviewer(juce::AudioFormatManager* formatManager, juce::ValueTree& valueTree);
    ~SimpleAudioPreviewer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

//    void mouseEnter(const juce::MouseEvent& e) override;
//    void mouseExit(const juce::MouseEvent& e) override;
//
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

    juce::File& getCurrentFile();
    bool wantsToPlayFile();
    void setWantsToPlayFile(bool wantsToPlay);
    void reloadCurrentFile();


private:

    void timerCallback() override;
    void updateFormatReader();


    bool readyToPlayFile = false;
    std::atomic<bool> rendering = false;
    bool newFileWaiting = false;

    int playBackSampleRate;

    //juce::ToggleButton autoPlayToggle;
    //juce::Slider volumeSlider;

    InfoPanelToggleButton autoPlayToggle {"Auto-Play", "If unchecked, files will play when they are double-clicked. If this is checked, the will play as they are selected"};
    InfoPanelSlider volumeSlider {"Preview Volume", "This is the volume that files are previewed at", "Snap To Default by double-clicking or CMD + click the slider"};
    
    juce::ValueTree& valueTree;
    juce::File currentAudioFile;

    std::unique_ptr<juce::AudioFormatReader> currentFormatReader = nullptr;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource = nullptr;
    juce::AudioFormatManager* formatManager = nullptr;

    
    //juce::String compTitle {"Audio Previewer"};
    //juce::String message {"Double-Click files to preview them, if auto-play is active they will play on a single click. Volume is sepearte from the rest of the sampler"};
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleAudioPreviewer)
};
