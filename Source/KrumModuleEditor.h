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
#include "TimeHandle.h"

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
*   - Should be simple and quick to use
*   - Icons for buttons?
* 
* 
*/

#define THUMBNAIL_RES 256

class KrumModule;
class KrumModuleProcessor;
class KrumSamplerAudioProcessorEditor;
class DragHandle;
class ModuleSettingsOverlay;
class KrumFileBrowser;

class KrumModuleEditor  :   public juce::Component,
                            public juce::DragAndDropTarget,
                            public juce::FileDragAndDropTarget,
                            public juce::DragAndDropContainer,
                            public juce::Timer,
                            public juce::ValueTree::Listener
{
public:
    KrumModuleEditor(juce::ValueTree& moduleTree, KrumSamplerAudioProcessorEditor& e, juce::AudioFormatManager& fm/*, int state = 0*/); 
    ~KrumModuleEditor() override;

    void paint (juce::Graphics&) override;
    
    void paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);

    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& e) override;
    
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    
    void buildModule();
    void setChildCompColors();

    //void showSettingsMenu();
    void setModuleSelected(bool isModuleSelected);
    void setModuleButtonsClickState(bool isClickable);
    
    void showNewSettingsOverlay();
    void showSettingsOverlay(bool keepCurrentColorOnExit ,bool selectOverlay = false);
    void removeSettingsOverlay(bool keepSettings);


    void hideModule();
    void showModule();
    
    int getModuleState();
     
    int getModuleSamplerIndex();

    void setModuleState(int newState);

    int getModuleDisplayIndex();
    void setModuleDisplayIndex(int newDisplayIndex);

    void setModuleName(juce::String& newName);
    juce::String getModuleName();

    void setModuleColor(juce::Colour newColor);
    juce::Colour getModuleColor();

    int getModuleMidiNote();
    juce::String getModuleMidiNoteString(bool noteName);
    void setModuleMidiNote(int newMidiNote);

    int getModuleMidiChannel();
    void setModuleMidiChannel(int newMidiChannel);

    void setModulePlaying(bool isPlaying);
    bool isModulePlaying();

    int getAudioFileLengthInMs();

    void setTimeHandles();

    bool doesEditorWantMidi();
    void handleMidi(int midiChannel, int midiNote);

    void triggerMouseDownOnNote(const juce::MouseEvent& e);
    void triggerMouseUpOnNote(const juce::MouseEvent& e);
    
    bool needsToDrawThumbnail();
    void setAndDrawThumbnail();
    
    void handleNewFile(juce::File& file, bool overlayShouldListen);
    void setModuleFile(juce::File& newFile);
    void addFileToRecentsFolder(juce::File& file, juce::String name);

    bool shouldCheckDroppedFile();
    void handleLastDroppedFile();
    bool isMouseOverThumbnail();
    bool thumbnailHitTest(const juce::MouseEvent& mouseEvent);
    void setClipGainSliderVisibility(bool sliderShouldBeVisible);

    bool canThumbnailAcceptFile();
    void setThumbnailCanAcceptFile(bool shouldAcceptFile);

    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
    
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
    
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    bool shouldModuleAcceptFileDrop();
    

private:

    void updateBubbleComp(juce::Slider* slider, juce::Component* comp);


    friend class DragAndDropThumbnail;
    friend class TimeHandle;

    void zeroModuleTree();
    void timerCallback() override;

    bool isMouseOverAnyChildren();

    void printValueAndPositionOfSlider();

    void handleOneShotButtonMouseDown(const juce::MouseEvent& e);
    void handleOneShotButtonMouseUp(const juce::MouseEvent& e);

    
    bool drawThumbnail = false;
    bool needsToBuildModuleEditor = false;

    juce::ValueTree moduleTree;
    KrumSamplerAudioProcessorEditor& editor;
    
    int oldMidiNote = 0;
    bool modulePlaying = false;

    //juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour thumbBgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour titleFontColor{ juce::Colours::black };

    InfoPanelLabel titleBox {"Title", "Double-click to edit the title of your module, by default it takes the name of your sample"};
    InfoPanelSlider volumeSlider {"Module Gain", "Sliders can be double-clicked to zero out, or CMD + click"};
    InfoPanelSlider panSlider {"Module Pan", "Sliders can be double-clicked to zero out, or CMD + click"};
    
    InfoPanelComboBox outputCombo{ "Output Channel", "Select which output bus you would like this module to go to. Default is the Main Bus (1-2)" };

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> volumeSliderAttachment;
    std::unique_ptr<SliderAttachment> panSliderAttachment;

    std::unique_ptr<ComboBoxAttachment> outputComboAttachment;
    
    DragAndDropThumbnail thumbnail;
    TimeHandle timeHandle;

    float buttonClickVelocity = 0.5f;

    class OneShotButton : public InfoPanelDrawableButton
    {
    public:
        OneShotButton();
        ~OneShotButton() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        std::function<void(const juce::MouseEvent& e)> onMouseUp;
        std::function<void(const juce::MouseEvent& e)> onMouseDown;
    };

    class ModalManager : public juce::ModalComponentManager::Callback
    {
    public:
        ModalManager(std::function<void(int)> menuResult)
            : handleSettingsResult(menuResult)
        {}
        
        void modalStateFinished(int returnValue) override
        {
            handleSettingsResult(returnValue);
        }
        
        std::function<void(int)> handleSettingsResult;
    };
    
    class MidiLabel :   public juce::Component,
                        public juce::SettableTooltipClient
    {
    public:
        MidiLabel(KrumModuleEditor* parentEditor);
        ~MidiLabel() override;
        
        void paint(juce::Graphics& g) override;
        //juce::String getTooltip() override;
        void setStrings(juce::String note, juce::String channel);
        
        
        juce::String noteNumber;
        juce::String channelNumber;
        
        KrumModuleEditor* moduleEditor = nullptr;
    };
    
    MidiLabel midiLabel{this};
    
    OneShotButton playButton;
    InfoPanelDrawableButton editButton {"Settings", "Provides a list of actions to change the settings of the module", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackgroundOriginalSize };
    friend class ColorPalette;
    
    std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    
    //For Later..
//    std::unique_ptr<DragHandle> dragHandle;
//    friend class DragHandle;
//    bool forcedMouseUp = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};

