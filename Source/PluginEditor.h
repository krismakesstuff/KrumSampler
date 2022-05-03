/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "KrumModule.h"
#include "KrumKeyboard.h"
#include "KrumLookAndFeel.h"
#include "KrumFileBrowser.h"
#include "KrumModuleContainer.h"
#include "InfoPanel.h"


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
    const static int windowW = 1200;
    const static int windowH = 600;
    const static int windowWNoBrowser = 900;

    const static int maxWindowW = 1670;
    const static int maxWindowH = windowH;
    const static int maxWindowWNoBrowser = 1320;
    
    const static int minWindowW = 800;
    const static int minWindowH = windowH;
    const static int minWindowWNoBrowser = 550;

    //---------------------------

    const static int  topBar = 40;
    const static int shrinkage = 5;
    const juce::Rectangle<int> madeByArea{ 0, 0, 150, 35 };
    const static int bottomBarH = 20;

    const static int moduleH = windowH * 0.68f;
    const static int moduleW = 120;

    //const static int addButtonH = 50;
    //const static int addButtonW = 100;
    const static int collapseButtonH = 30;
    const static int collapseButtonW = 13;

    const static int infoButtonSize = 25;

    const static int outputW = 80;
    
    const static int presetsW = 130;
    const static int presetsH = 25;
    
    const static int settingsButtonW = 23;
    const static int settingsButtonH = 23;

    const static int keyboardH = 85;

    const static int titleImageW = 420; //nice

    const static int fileBrowserW = 350;

    const static float cornerSize = 5.0f;
    const static float smallOutline = 1.0f;
    const static float bigOutline = 2.0f;


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

    void addNextModuleEditor();
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
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();
    juce::Viewport* getModuleViewport();

    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;


private:

    void updateOutputGainBubbleComp(juce::Component*);

    void setConstrainerLimits(bool checkBounds);

    void setWindowSizeAndPosition();
    void collapseButtonClicked();

    void saveEditorDimensions();

    bool showWebsite();

    bool needsToUpdateThumbs = false;

    friend class KrumModuleContainer;
    friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;

    KrumLookAndFeel kLaf{};

    //These should be one class called ModuleLookAndFeel?
    VolumeLookAndFeel vLaf{};
    PanLookAndFeel pLaf{};
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
    InfoPanelDrawableButton infoButton {"Info Button", "Toggles this Info Panel Box"};
    

    InfoPanelDrawableButton collapseBrowserButton {"Hide Browser", "This will hide the browser and give you more screen real estate when you aren't using the browser anymore","", juce::DrawableButton::ButtonStyle::ImageStretched};
    

    InfoPanelComboBox presetsComboBox{"Presets", "COMING SOON!!"};
    InfoPanelDrawableButton settingsButton{ "Global Settings", "COMING SOON!!" };
    
    //class Constrainer : public juce::ComponentBoundsConstrainer
    //{
    //public:
    //    Constrainer();
    //    ~Constrainer() override;

    //    void applyBoundsToComponent(juce::Component& comp, juce::Rectangle<int> bounds) override;

    //    //void checkBounds(Rectangle< int >& bounds, const Rectangle< int >& previousBounds, const Rectangle< int >& limits, bool isStretchingTop, bool isStretchingLeft, bool isStretchingBottom, bool isStretchingRight)

    //};


    juce::ComponentBoundsConstrainer constrainer;
    class Positioner : public juce::Component::Positioner
    {
    public:
        Positioner(KrumSamplerAudioProcessorEditor& comp);
        ~Positioner() override;

        void applyNewBounds(const juce::Rectangle<int>& bounds) override;
    private:
        KrumSamplerAudioProcessorEditor& editor;
    };
     
    Positioner positioner{ *this };

    class GlobalOutputSlider : public InfoPanelDrawableButton
    {
    public:


        juce::Label label{ "Output" };

    //private:
    };
    
    //juce::DrawableButton collapseBrowserButton{ "Hide Broswer", juce::DrawableButton::ButtonStyle::ImageStretched};
   

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
