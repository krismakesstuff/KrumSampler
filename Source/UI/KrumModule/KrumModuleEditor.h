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
#include "../InfoPanel.h"
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
* 
* 
*/

#define THUMBNAIL_RES 256

class KrumModule;
class KrumModuleProcessor;
class KrumSamplerAudioProcessorEditor;
class KrumModuleContainer;
class DragHandle;
class ModuleSettingsOverlay;
class KrumFileBrowser;



class KrumModuleEditor  :   public juce::Component,                            public juce::DragAndDropContainer,
                            public juce::Timer,
                            public juce::ValueTree::Listener
{
public:
    //KrumModuleEditor(juce::ValueTree& moduleTree, KrumSamplerAudioProcessorEditor& e, juce::AudioFormatManager& fm); 
    KrumModuleEditor(juce::ValueTree& moduleTree, KrumModuleContainer& mc, juce::AudioFormatManager& fm); 
    ~KrumModuleEditor() override;

    void paint (juce::Graphics&) override;
    
    void paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);

    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
   
    
    void buildModule();
    void setChildCompColors();

    //void showSettingsMenu();
    void setModuleSelected(bool isModuleSelected);
    bool isModuleSelected();
    void setModuleButtonsClickState(bool isClickable);
    
    //void showNewSettingsOverlay();
    void showSettingsOverlay();
    void hideSettingsOverlay();


    void hideModule();
    void showModule();
    
    int getModuleState();
     
    int getModuleSamplerIndex();
    juce::String getSamplerIndexString();

    void setModuleState(int newState);

    int getModuleDisplayIndex();
    void setModuleDisplayIndex(int newDisplayIndex, bool sendChange = true);

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

    bool isModuleMuted();
    bool getModuleReverseState();

    int getModulePitchShift();

    double getModuleGain();

    void setNumSamplesOfFile(int numSampleInFile);
    int getNumSamplesInFile();

    int getAudioFileLengthInMs();

    void setTimeHandles();

    bool doesEditorWantMidi();
    void handleMidi(int midiChannel, int midiNote);

    void triggerMouseDownOnNote(const juce::MouseEvent& e);
    void triggerMouseUpOnNote(const juce::MouseEvent& e);
    
    bool needsToDrawThumbnail();
    void setAndDrawThumbnail();
    

    //make handleNewFileDrop?
   // bool handleNewFile(juce::ValueTree fileTree, bool overlayShouldListen = true);
    
    void setModuleFile(juce::File& newFile);
   // void addFileToRecentsFolder(juce::File& file, juce::String name);

    bool shouldCheckDroppedFile();
    void handleLastDroppedFile();
    bool isMouseOverThumbnail();
    bool thumbnailHitTest(const juce::MouseEvent& mouseEvent);
    void setClipGainSliderVisibility(bool sliderShouldBeVisible);
    //void setPitchSliderVisibility(bool sliderShouldBeVisible);

    bool canThumbnailAcceptFile();
    void setThumbnailCanAcceptFile(bool shouldAcceptFile);

    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;

    
    bool shouldModuleAcceptFileDrop();
    
    bool getMouseOver();
    bool getMouseOverKey();
    void setMouseOverKey(bool isMouseOverKey);

    //tests if the given tree, is the same as this module's moduleTree;
    bool isModuleTree(juce::ValueTree& treeToTest);

private:
    
    void reassignSliderAttachment(juce::Slider* sliderToAssign, bool beginGesture = true);
    void updateSliderFromMultiControl(juce::Slider* sourceSlider);
    void resetSliderAttachments();

    void reassignButtonAttachment(juce::Button* button, bool beginGesrture);
    //void updateButtonFromMultiControl(juce::Button* sourceButton);
    void resetButtonAttachments();

    void reassignComboAttachment(juce::ComboBox* combo, bool beginGesrture);
    void updateComboFromMultiControl(juce::ComboBox* sourceCombo);
    void resetComboAttachments();

    void toggleMenuButton();

    void updateBubbleComp(juce::Slider* slider, juce::Component* comp);
    double normalizeGainValue(double gain);

    friend class DragAndDropThumbnail;
    friend class TimeHandle;
    friend class KrumModuleContainer;
    friend class ModuleSettingsOverlay;

    void setModuleListeningForMidi(bool shouldListen);

    void zeroModuleTree();
    void timerCallback() override;

    void printValueAndPositionOfSlider();

    void handleOneShotButtonMouseDown(const juce::MouseEvent& e);
    void handleOneShotButtonMouseUp(const juce::MouseEvent& e);

    void setChildCompMuteColors();

    bool drawThumbnail = false;
    bool needsToBuildModuleEditor = false;
    bool mouseOver = false;
    bool mouseOverKey = false;

    bool modulePlaying = false;
    bool moduleSelected = false;
   
    // sendToSelectedModules = false;

    juce::ValueTree moduleTree;
    KrumModuleContainer& moduleContainer;
    //KrumSamplerAudioProcessorEditor& pluginEditor;
    //Handler handler;


    juce::Colour thumbBgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour titleFontColor{ juce::Colours::black };

    InfoPanelLabel titleBox {"Title", "Double-click to edit the title of your module, by default it takes the name of your sample"};
    
    class MultiControlSlider : public InfoPanelSlider
    {
    public:
        MultiControlSlider(KrumModuleEditor& e, juce::String title, juce::String message);
        ~MultiControlSlider() override;
        
        //TODO: for multi control, change the slider attachments, and return when done
        //will probably have to write some functions in the moduleContainer as well
        void mouseDown(const juce::MouseEvent& e) override;
        //void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;



    private:
        
        KrumModuleEditor& editor;
    };
    
    //InfoPanelSlider volumeSlider {"Module Gain", "Sliders can be double-clicked to zero out, or CMD + click"};
    //InfoPanelSlider panSlider {"Module Pan", "Sliders can be double-clicked to zero out, or CMD + click"};
    MultiControlSlider volumeSlider {*this, "Module Gain", "Sliders can be double-clicked to zero out, or CMD + click"};
    MultiControlSlider panSlider {*this, "Module Pan", "Sliders can be double-clicked to zero out, or CMD + click"};
    
    class MultiControlComboBox : public InfoPanelComboBox
    {
    public:
        MultiControlComboBox(KrumModuleEditor& e, juce::String title, juce::String message);
        ~MultiControlComboBox();

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
    private:
        KrumModuleEditor& editor;
    };

    MultiControlComboBox outputCombo{*this, "Output Channel", "Select which output bus you would like this module to go to. Default is Main Bus (1-2)" };
   

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

    std::unique_ptr<SliderAttachment> volumeSliderAttachment;
    std::unique_ptr<SliderAttachment> panSliderAttachment;
    std::unique_ptr<ComboBoxAttachment> outputComboAttachment;

    std::unique_ptr<SliderAttachment> pitchSliderAttachment;
    std::unique_ptr<ButtonAttachment> reverseButtonAttachment;
    std::unique_ptr<ButtonAttachment> muteButtonAttachment;
    

    DragAndDropThumbnail thumbnail;
    TimeHandle timeHandle;

    float buttonClickVelocity = 0.5f;
    float buttonTextSize = 13.0f;

    class CustomToggleButton : public InfoPanelTextButton
    {
    public:
        CustomToggleButton(juce::String title, juce::String message, KrumModuleEditor& editor);
        ~CustomToggleButton() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
                         const bool shouldDrawButtonAsDown) override;

       void mouseDown(const juce::MouseEvent& event) override;
       void mouseUp(const juce::MouseEvent& event) override;
        //void mouseEnter(const juce::MouseEvent& e) override;
        //void mouseExit(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& editor;
    };

    void muteButtonClicked();

    CustomToggleButton reverseButton{ "Reverse Button", "Plays the sample in reverse, active when highlighted", *this };
    CustomToggleButton muteButton{ "Mute", "Mutes this sample from being played.", *this };

    //KComp<juce::TextButton> muteButton {"Mute", "Mutes this sample from being played.", "M"};

    class OneShotButton : public InfoPanelDrawableButton
    {
    public:
        OneShotButton(KrumModuleEditor&);
        ~OneShotButton() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
                            const bool shouldDrawButtonAsDown) override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        std::function<void(const juce::MouseEvent& e)> onMouseUp;
        std::function<void(const juce::MouseEvent& e)> onMouseDown;
    private:
        KrumModuleEditor& editor;
    };

    class MenuButton : public InfoPanelDrawableButton
    {
    public:
        MenuButton(KrumModuleEditor& e);
        ~MenuButton() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
                            const bool shouldDrawButtonAsDown) override;

        void mouseUp(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& editor;
    };

    friend class KrumLookAndFeel;

    class PitchSlider : public InfoPanelSlider
    {
    public:
        PitchSlider(KrumModuleEditor& editor);
        ~PitchSlider();

        void paint(juce::Graphics& g) override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void mouseEnter(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& editor;
        juce::BubbleMessageComponent bubbleComp{};
    };

    PitchSlider pitchSlider{ *this };
    //PitchButton pitchButton{  *this };
    
    class MidiLabel :   public InfoPanelComponent,
                        public juce::SettableTooltipClient
    {
    public:
        MidiLabel(KrumModuleEditor& parentEditor);
        ~MidiLabel() override;
        
        void paint(juce::Graphics& g) override;
        //juce::String getTooltip() override;
        //void setStrings(juce::String note, juce::String channel);
        //void mouseEnter(const juce::MouseEvent& e) override;
        //void mouseExit(const juce::MouseEvent& e) override;

        
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        
        juce::String noteNumber;
        juce::String channelNumber;

        //static void handleResult(int result, MidiLabel* label);

        bool isListeningForMidi();
        void setListeningForMidi(bool shouldListen);

        juce::Colour textColor = juce::Colours::white;

    private:
        bool listeningForMidi = false;
        KrumModuleEditor& moduleEditor;
        float fontSize = 13.0f;
        
        
    };

    //TODO: change these classes to take the Infopanel info in the ctor like the sliders
    MidiLabel midiLabel{*this};
    
    OneShotButton playButton{ *this };
    MenuButton menuButton { *this};
    
    class DragHandle : public InfoPanelDrawableButton
    {
    public:
        DragHandle(KrumModuleEditor& parentEditor);
        ~DragHandle() override;

        /*void setImage(juce::Drawable* newImage);

        void paint(juce::Graphics& g) override;
        void resized() override;*/
        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
            const bool shouldDrawButtonAsDown) override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& moduleEditor;
    };

    DragHandle dragHandle{ *this };
    
    std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};

