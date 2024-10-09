/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

#include "../KrumModule.h"
#include "../UI/KrumKeyboard.h"
#include "../UI/KrumLookAndFeel.h"
#include "../UI/FileBrowser/KrumFileBrowser.h"
#include "../UI/KrumModuleContainer.h"
#include "../UI/InfoPanel.h"


//==============================================================================
/*
* 
* A class that represents the GUI. This is created and owned by the host app. 
* This holds all of the Krum GUI elements and draws the base component for the plugin.
* It's important to keep in mind that this can be created and deleted at any time!
* 
*/

namespace EditorDimensions
{
    const int windowW = 1200;
    const int windowH = 600;
    const int windowWNoBrowser = 900;
    const int maxWindowW = 1670;
    const int maxWindowH = windowH;
    const int maxWindowWNoBrowser = 1320;
    const int minWindowW = 800;
    const int minWindowH = windowH;
    const int minWindowWNoBrowser = 550;

    //---------------------------

    const int  topBar = 40;
    const int shrinkage = 5;
    const juce::Rectangle<int> madeByArea{ 0, 0, 150, 35 };
    const int bottomBarH = 18;

    const int moduleH = windowH * 0.68f;
    const int moduleW = 120;
    
    const int dropSampleAreaW = moduleW * 0.45f;

    const int collapseButtonH = 30;
    const int collapseButtonW = 23;

    const int infoButtonSize = 25;

    const int outputW = 80;
    
    const int presetsW = 130;
    const int presetsH = 25;
    
    const int settingsButtonW = 23;
    const int settingsButtonH = 23;

    const int keyboardH = 85;

    const int titleImageW = 420; //nice
    const int titleSubTextOffset = 90;
    const int fileBrowserW = 350;

    const float cornerSize = 5.0f;
    const float smallOutline = 0.75f;
    const float bigOutline = 1.5f;
    const float xlOutline = 3.5f;


    static int extraShrinkage(int extraMultplier = 2)
    {
        return shrinkage * extraMultplier;
    }
};




class KrumSamplerAudioProcessorEditor  :    public juce::AudioProcessorEditor,
                                            public juce::DragAndDropContainer
{
public:
    KrumSamplerAudioProcessorEditor (KrumSamplerAudioProcessor&, KrumSampler&,
                                    juce::AudioProcessorValueTreeState& vts,
                                    juce::ValueTree& valueTree, juce::ValueTree& fileBrowserTree);
    ~KrumSamplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void paintOutputVolumeLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void resized() override;


    static juce::String getMidiInfo(const juce::MidiMessage&);

    //void addNextModuleEditor();
    KrumModuleContainer& getModuleContainer();

    void printModules();

    bool isBrowserHidden();
    void hideFileBrowser();
    void showFileBrowser();

    float getOutputGainValue();
    
    void saveFileBrowserHiddenState();
    bool getSavedFileBrowserHiddenState();

    void infoButtonClicked();
    bool getSavedInfoButtonState();
    void saveInfoButtonState();
    
    int getSavedEditorWidth();
    void saveEditorWidth();

    int getSavedEditorHeight();
    void saveEditorHeight();


    void addKeyboardListener(juce::MidiKeyboardStateListener* listener);
    void removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove);

    juce::AudioFormatManager& getAudioFormatManager();
    juce::AudioThumbnailCache& getThumbnailCache();

    KrumSampler& getSampler();
    KrumKeyboard& getKeyboard();
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();
    juce::Viewport* getModuleViewport();

    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;

    KrumLookAndFeel* getKrumLaf();
    VolumeLookAndFeel* getVolumeLaf();
    PanLookAndFeel* getPanLaf();
    PitchSliderLookAndFeel* getPitchLaf();

    class DropSampleArea : public InfoPanelComponent,
        public juce::DragAndDropTarget,
        public juce::FileDragAndDropTarget,
        public juce::Timer
    {
    public:
        DropSampleArea(KrumModuleContainer* mc);
        ~DropSampleArea() override;

        void paint(juce::Graphics& g) override;

        void setDraggingMouseOver(bool isMouseOver);

    private:

        //void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        //void mouseEnter(const juce::MouseEvent& e) override;
        //void mouseExit(const juce::MouseEvent& e) override;

        bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
        void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped(const juce::StringArray& files, int x, int y) override;

        void timerCallback() override;

        KrumModuleContainer* moduleContainer = nullptr;
        bool draggingMouseOver = false;
        bool needsRepaint = false;
    };

    DropSampleArea* getDropSampleArea();

private:

    void updateOutputGainBubbleComp(juce::Component*);
    void setConstrainerLimits(bool checkBounds);
    void collapseButtonClicked();
    void saveEditorDimensions();
    bool showWebsite();


    friend class KrumModuleContainer;
    //friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;
    //friend class FavoritesTreeView;

    KrumLookAndFeel kLaf{};

    VolumeLookAndFeel vLaf{};
    PanLookAndFeel pLaf{};
    PitchSliderLookAndFeel pitchLaf{};
    FileBrowserLookAndFeel fbLaf{};

    juce::Image titleImage;

    const juce::Font defaultFont{ "Calibri", 11.0f, juce::Font::FontStyleFlags::plain };

    juce::Rectangle<int> modulesBG;

    juce::Viewport modulesViewport{ "ModulesViewport" };

    juce::AudioProcessorValueTreeState& parameters;
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    
    juce::ValueTree valueTree;

    InfoPanelSlider outputGainSlider{"Output Gain", "Overall volume control of the plug-in"};
    std::unique_ptr<SliderAttachment> outputGainAttachment;
 
    KrumSamplerAudioProcessor& audioProcessor;
    KrumSampler& sampler;
    KrumFileBrowser fileBrowser; 

    KrumModuleContainer moduleContainer{this, valueTree};
    KrumKeyboard keyboard{ audioProcessor.getMidiState(), juce::MidiKeyboardComponent::Orientation::horizontalKeyboard, moduleContainer, valueTree };

    juce::String madeByString{ "Made by Kris Crawford" };
    juce::URL websiteURL{ "https://www.krismakesmusic.com" };
    InfoPanelTextButton websiteButton{"Website", "Clicking this will open my website. Go check it out yo!"};
    InfoPanelDrawableButton infoButton { juce::DrawableButton::ButtonStyle::ImageStretched, "Info Button", "Toggles this Info Panel Box"};
    InfoPanelDrawableButton collapseBrowserButton { juce::DrawableButton::ButtonStyle::ImageFitted, "Hide Browser", "This will hide the browser and give you more screen real estate when you aren't using the browser anymore"};
    

    InfoPanelComboBox presetsComboBox{"Presets", "Coming in future update"};
    InfoPanelDrawableButton settingsButton{ juce::DrawableButton::ButtonStyle::ImageFitted, "Global Settings", "Coming in future update" };

    juce::ComponentBoundsConstrainer constrainer;

    DropSampleArea dropSampleArea{ &moduleContainer };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
