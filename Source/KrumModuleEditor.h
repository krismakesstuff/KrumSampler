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
#include "InfoPanel.h"

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
    juce::String getModuleName();
    
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

    void handleSettingsMenuResult(int result);
    
private:

    //static void handleSettingsMenuResult(int result, KrumModuleEditor* parentEditor);

    //std::function<void(int)> settingsMenuCallback;
    
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
    
    //juce::Label titleBox;
    InfoPanelLabel titleBox {"Title", "Double-click to edit the title of your module, by default it takes the name of your sample"};
    
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    //juce::Slider volumeSlider, panSlider;

    InfoPanelSlider volumeSlider {"Module Gain", "Sliders can be double-clicked to zero out, or CMD + click"};
    InfoPanelSlider panSlider {"Module Pan", "Sliders can be double-clicked to zero out, or CMD + click"};
    
    std::unique_ptr<SliderAttachment> volumeSliderAttachment;
    std::unique_ptr<SliderAttachment> panSliderAttachment;
    
    DragAndDropThumbnail thumbnail;

    class OneShotButton : /*public juce::DrawableButton*/ public InfoPanelDrawableButton
    {
    public:
        OneShotButton();
        ~OneShotButton() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        std::function<void()> onMouseUp;
        std::function<void()> onMouseDown;
    };

    class ModalManager : public juce::ModalComponentManager::Callback
    {
    public:
        ModalManager(std::function<void(int)> menuResult)//, KrumModuleEditor* parentEditor)
            : handleSettingsResult(menuResult)//, parent(parentEditor)
        {}
        
        void modalStateFinished(int returnValue) override
        {
            handleSettingsResult(returnValue);
        }
        
        std::function<void(int)> handleSettingsResult;
        //KrumModuleEditor* parent = nullptr;
    };
    
    
    OneShotButton playButton;
    //juce::DrawableButton editButton{ "Edit Button", juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize };
    InfoPanelDrawableButton editButton {"Settings", "Provides a list of actions to change the settings of the module", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize };
    friend class ColorPalette;
    
    std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    
    //For Later..
    //std::unique_ptr<DragHandle> dragHandle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};


class DummyKrumModuleEditor :   public juce::Component,
                                public juce::DragAndDropTarget
{
public:
    DummyKrumModuleEditor() { setRepaintsOnMouseActivity(true);}
    ~DummyKrumModuleEditor() override {}
    
    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        
        g.setColour(isMouseOver() ? juce::Colours::grey : juce::Colours::darkgrey);
        g.drawRoundedRectangle(area.reduced(5).toFloat(), 3.0f, 1.0f);
        
        g.setColour(juce::Colours::grey);
        g.drawFittedText("Drop A Sample Here", area.reduced(5), juce::Justification::centred, 3);
    }

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails) override
    {
        return true;
    }
    
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails) override
    {
        DBG(dragDetails.description.toString());
    }
    
//    void resized() override
//    {
//    }

};

class KrumModuleEditorBase :    public juce::Component,
                                public juce::DragAndDropTarget
{
    
};
