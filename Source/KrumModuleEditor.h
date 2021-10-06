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
#include "ModuleSettingsOverlay.h"
#include "DragAndDropThumbnail.h"

//==============================================================================

/*
* 
* The GUI side of a Module. 
* 
* To construct one, you must have a parent (KrumModule) and it's processor, as well as a reference to the PluginEditor (KrumSmpalerAudioProcessorEditor)
* 
* This class handles all GUI interaction and painting. 
* The GUI can enter a ModuleSettingsOverlay state which allows the user to change the midi assignment as well as change the color of the module (redesigned settings menu to come).
* 
* TODO:
* - Redisgn the ModuleSettingsOverlay GUI
*   - Better Color Pallette
*   - Maybe even have seperate ModuleSettingsOverlays for each settings change
*   - Should be simple and quick to use
* 
* 
*/

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

    void triggerNoteOnInParent();
    void triggerNoteOffInParent();

    bool needsToDrawThumbnail();
    void setAndDrawThumbnail();
    
    bool shouldCheckDroppedFile();
    void handleLastDroppedFile();
    void setOldMidiNote(int midiNote);

    bool isMouseOverThumbnail();
    bool thumbnailHitTest(const juce::MouseEvent& mouseEvent);
    void setClipGainSliderVisibility(bool sliderShouldBeVisible);

    bool canThumbnailAcceptFile();
    void setThumbnailCanAcceptFile(bool shouldAcceptFile);

private:

    static void handleSettingsMenuResult(int result, KrumModuleEditor* parentEditor);

    friend class DragAndDropThumbnail;

    bool drawThumbnail = false;
    bool needsToBuildModuleEditor = false;
    KrumModule& parent;
    KrumModuleProcessor& moduleProcessor;
    KrumSamplerAudioProcessorEditor& editor;

    int oldMidiNote = 0;

    juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour thumbBgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour fontColor{ juce::Colours::white.darker() };
    
    juce::Label titleBox;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    juce::Slider volumeSlider, panSlider;

    std::unique_ptr<SliderAttachment> volumeSliderAttachment;
    std::unique_ptr<SliderAttachment> panSliderAttachment;

   

    DragAndDropThumbnail thumbnail;


    class OneShotButton : public juce::DrawableButton
    {
    public:
        OneShotButton();
        ~OneShotButton() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        std::function<void()> onMouseUp;
        std::function<void()> onMouseDown;
    };


    OneShotButton playButton;
    juce::DrawableButton editButton{ "Edit Button", juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize };;

    friend class ColorPalette;
    
    std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    
    //For Later..
    //std::unique_ptr<DragHandle> dragHandle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};
