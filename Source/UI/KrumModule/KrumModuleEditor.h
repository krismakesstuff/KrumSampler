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
* The GUI side of a Module(sample). 
* 
* This class handles all GUI interaction and painting. 
* There are 3 module states, see KrumModule::ModuleState. The valueTreePropertyChanged callback responds to changes and check the state
* This class interfaces heavily with the KrumModuleContainer. Most actions go through the container first
* 
* The GUI can show a ModuleSettingsOverlay pop-up which allows the user to change the color of the module and delete the module.
* 
* The child components are custom subclasses of InfoPanel classes, see InfoPanel.h
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
    KrumModuleEditor(juce::ValueTree& moduleTree, KrumModuleContainer& mc, juce::AudioFormatManager& fm); 
    ~KrumModuleEditor() override;

    void paint (juce::Graphics&) override;
    
    void paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds);

    void resized() override;

    //mouse Listener
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    //valuetree callback
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
   
    void buildModule();
    void setChildCompColors();

    void setModuleSelected(bool isModuleSelected);
    bool isModuleSelected();
    void setModuleButtonsClickState(bool isClickable);
    
    void showSettingsOverlay();
    void hideSettingsOverlay();

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
    
    void setModuleFile(juce::File& newFile);

    bool shouldCheckDroppedFile();
    void handleLastDroppedFile();
    bool isMouseOverThumbnail();
    bool thumbnailHitTest(const juce::MouseEvent& mouseEvent);
    void setClipGainSliderVisibility(bool sliderShouldBeVisible);

    bool canThumbnailAcceptFile();
    void setThumbnailCanAcceptFile(bool shouldAcceptFile);

    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;

    
    bool shouldModuleAcceptFileDrop();
    
    bool getMouseOver();
    bool getMouseOverKey();
    void setMouseOverKey(bool isMouseOverKey);

    //tests if the given tree, is the same as this module's moduleTree;
    bool isModuleTree(juce::ValueTree& treeToTest);

    void setTitleBoxEditing(bool shouldBeEditing);

private:
    
    void reassignSliderAttachment(juce::Slider* sliderToAssign, bool beginGesture = true);
    void updateSliderFromMultiControl(juce::Slider* sourceSlider);
    void resetSliderAttachments();

    void reassignButtonAttachment(juce::Button* button, bool beginGesrture);
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
    bool moduleSelected = false;

    bool modulePlaying = false;
    float timerHz = 30.0f;
    float playingAnimationTimeMs = 75.0f;
    bool animatePlaying = false;
    float animationCurrentTime = 0.0f;

    juce::ValueTree moduleTree;
    KrumModuleContainer& moduleContainer;


    juce::Colour thumbBgColor{ juce::Colours::darkgrey.darker() };
    juce::Colour titleFontColor{ juce::Colours::black };
    class TitleBox : public InfoPanelLabel
    {
    public:
        TitleBox(KrumModuleEditor& e, juce::String title, juce::String message);
        ~TitleBox() override;

        //void focusLost(juce::Component::FocusChangeType cause) override;

    private:

        KrumModuleEditor& editor;

    };

    TitleBox titleBox {*this, "Title", "Double-click to edit the title of your module, by default it takes the name of your sample"};


    class MultiControlSlider : public InfoPanelSlider
    {
    public:
        MultiControlSlider(KrumModuleEditor& e, juce::String title, juce::String message);
        ~MultiControlSlider() override;
        
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

    private:
        
        KrumModuleEditor& editor;
    };
    
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

    MultiControlSlider volumeSlider {*this, "Module Gain", "Sliders can be double-clicked to zero out, or CMD + click"};
    MultiControlSlider panSlider {*this, "Module Pan", "Sliders can be double-clicked to zero out, or CMD + click"};
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
        CustomToggleButton(KrumModuleEditor& editor, juce::String title, juce::String message);
        ~CustomToggleButton() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
                         const bool shouldDrawButtonAsDown) override;

       void mouseDown(const juce::MouseEvent& event) override;
       void mouseUp(const juce::MouseEvent& event) override;

    private:
        KrumModuleEditor& editor;
    };

    void muteButtonClicked();

    CustomToggleButton reverseButton{ *this, "Reverse Button", "Plays the sample in reverse, active when highlighted" };
    CustomToggleButton muteButton{ *this, "Mute", "Mutes this sample from being played." };
    //TODO: implement solo feature
    CustomToggleButton soloButton{ *this, "Solo", "Soloes the sample." };

    class OneShotButton : public InfoPanelDrawableButton
    {
    public:
        OneShotButton(KrumModuleEditor&, juce::String title, juce::String message);
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

    OneShotButton playButton{ *this, "One Shot", "Plays the currently assigned sample" };
    
    class MenuButton : public InfoPanelDrawableButton
    {
    public:
        MenuButton(KrumModuleEditor& e, juce::String title, juce::String message);
        ~MenuButton() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
                            const bool shouldDrawButtonAsDown) override;

        void mouseUp(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& editor;
    };

    MenuButton menuButton { *this, "Settings", "Provides a list of actions to change the settings of the module" };
    
    friend class KrumLookAndFeel;

    class PitchSlider : public InfoPanelSlider
    {
    public:
        PitchSlider(KrumModuleEditor& editor, juce::String title, juce::String message);
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

    PitchSlider pitchSlider{ *this, "Module Pitch", "Click and Drag to shift the pitch by semi-tones, double-click to go back to zero" };

    class MidiLabel :   public InfoPanelComponent,
                        public juce::SettableTooltipClient
    {
    public:
        MidiLabel(KrumModuleEditor& parentEditor, juce::String title, juce::String message);
        ~MidiLabel() override;
        
        void paint(juce::Graphics& g) override;
  
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        
        juce::String noteNumber;
        juce::String channelNumber;

        bool isListeningForMidi();
        void setListeningForMidi(bool shouldListen);

        juce::Colour textColor = juce::Colours::white;

    private:
        bool listeningForMidi = false;
        KrumModuleEditor& moduleEditor;
        float fontSize = 13.0f;
        
        
    };

    MidiLabel midiLabel{*this,"Midi Label", "Displays the current Midi Note assignment, right click to learn a new key" };
    
    class DragHandle : public InfoPanelDrawableButton
    {
    public:
        DragHandle(KrumModuleEditor& parentEditor, juce::String title, juce::String message);
        ~DragHandle() override;

        void paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
            const bool shouldDrawButtonAsDown) override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

    private:
        KrumModuleEditor& moduleEditor;
    };

    DragHandle dragHandle{ *this,"Drag Handle", "Click to select/deselect module. Click and drag to move module." };
    
    std::unique_ptr<ModuleSettingsOverlay> settingsOverlay = nullptr;
    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumModuleEditor)
};

