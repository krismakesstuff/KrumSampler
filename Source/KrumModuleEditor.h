/*
  ==============================================================================

    KrumModuleEditor.h
    Created: 30 Apr 2021 10:21:42am
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ColorPalette.h"
#include "KrumSlider.h"
//==============================================================================


class KrumModule;
class KrumModuleProcessor;
class KrumSamplerAudioProcessorEditor;
class DragHandle;
class ModuleSettingsOverlay;

class KrumModuleEditor  : public juce::Component
{
public:
    KrumModuleEditor(KrumModule& o, KrumModuleProcessor& p, KrumSamplerAudioProcessorEditor& e);
    ~KrumModuleEditor() override;

    void paint (juce::Graphics&) override;

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);

    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    

    void buildModule();

    void setChildCompColors();

    void showSettingsMenu();

    void setModuleSelected(bool isModuleSelected);
    void removeSettingsOverlay(bool keepSettings);

    void showSettingsOverlay(bool selectOverlay = false);
    void cleanUpOverlay(bool keepSettings);
    void setModuleButtonsClickState(bool isClickable);

    int getModuleIndex();
    void setModuleIndex(int newIndex);

    int getModuleDisplayIndex();
    void setModuleDisplayIndex(int newDisplayIndex);

    void setModuleColor(juce::Colour newColor);
    juce::Colour getModuleColor();

    int getModuleMidiNote();
    juce::String getModuleMidiNoteString(bool noteName);
    void setModuleMidiNote(int newMidiNote);

    int getModuleMidiChannel();
    void setModuleMidiChannel(int newMidiChannel);

    void setModulePlaying(bool isPlaying);
    bool isModulePlaying();

    void updateName();
    
    void reassignSliderAttachments();
    void updateBubbleComp(juce::Slider* slider, juce::Component* comp);

    int getAudioFileLengthInMs();

    void setKeyboardColor();

    bool doesEditorWantMidi();

    void handleMidi(int midiChannel, int midiNote);
    
    void removeFromDisplay();
private:

    //friend class KrumModuleProcessor;

    KrumModule& parent;
    KrumModuleProcessor& moduleProcessor;
    KrumSamplerAudioProcessorEditor& editor;

    juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour thumbBgColor{ juce::Colours::darkgrey.darker() };
    //juce::Colour fontColor{ juce::Colours::lightgrey };
    juce::Colour fontColor{ juce::Colours::white.darker() };
    
    juce::Label titleBox;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    juce::Rectangle<int> thumbnailBG;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    juce::Slider volumeSlider, panSlider;
    //KrumSlider volumeSlider{juce::Slider::SliderStyle::LinearVertical};
    //KrumSlider panSlider{ juce::Slider::SliderStyle::LinearHorizontal };

    std::unique_ptr<SliderAttachment> volumeSliderAttachment;
    std::unique_ptr<SliderAttachment> panSliderAttachment;

    float buttonClickVelocity = 15.0f;

    //COME BACK
    //Make these image buttons!!
    juce::DrawableButton playButton{ "Play Button", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground };
    juce::DrawableButton editButton{ "Edit Button", juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize };;
    //put in settings menu
    //juce::DrawableButton deleteButton{ "Delete Button", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground };

    friend class ColorPalette;
    
    //std::unique_ptr<DragHandle> dragHandle = nullptr;
    //std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    juce::OptionalScopedPointer<DragHandle> dragHandle;
    juce::OptionalScopedPointer<ModuleSettingsOverlay> settingsOverlay;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};
