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
    const static int windowH = 600;
    const static int windowW = 1200;
    const static int windowWNoBrowser = 900;
    
    const static int  topBar = 60;
    const static int shrinkage = 5;
    const juce::Rectangle<int> madeByArea{ 0, 0, 150, 35 };

    const static int moduleH = windowH * 0.68f;
    const static int moduleW = 120;

    const static int addButtonH = 50;
    const static int addButtonW = 100;
    const static int collapseButtonH = 40;
    const static int collapseButtonW = 15;

    const static int outputW = 80;
    const static int keyboardH = 90;

    const static int titleImageW = 420; //nice
    
    const static int fileTreeH = 600;
    const static int fileTreeTitleH = 30;

    const static int fileTreeW = 350;

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

    void hideFileBrowser();
    void showFileBrowser();


    float getOutputGainValue();
    
    void saveFileBrowserHiddenState();
    bool getSavedFileBrowserHiddenState();

    void infoButtonClicked();
    bool getSavedInfoButtonState();
    void saveInfoButtonState();
    

    void addKeyboardListener(juce::MidiKeyboardStateListener* listener);
    void removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove);

    juce::AudioFormatManager* getAudioFormatManager();
    juce::AudioThumbnailCache& getThumbnailCache();

    KrumSampler& getSampler();
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();
    juce::Viewport* getModuleViewport();

    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;


private:

    void updateOutputGainBubbleComp(juce::Component*);


    bool needsToUpdateThumbs = false;

    friend class KrumModuleContainer;
    friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;

    KrumLookAndFeel kLaf{};

    //These should be one class called ModuleLookAndFeel
    VolumeLookAndFeel vLaf{};
    PanLookAndFeel pLaf{};

    juce::Image titleImage;

    const juce::Font defaultFont{ "Calibri", 11.0f, juce::Font::FontStyleFlags::plain };

    juce::Rectangle<int> modulesBG;
    juce::Colour modulesBGColor{ juce::Colours::darkgrey.darker(0.99f) };
    juce::Colour outlineColor{ juce::Colours::white };
    juce::Colour backOutlineColor{ juce::Colours::darkgrey };

    juce::Colour bgColor{ juce::Colours::black };
    juce::Colour outputThumbColor{ juce::Colours::cadetblue };
    juce::Colour outputTrackColor{ juce::Colours::darkgrey };
    juce::Colour mainFontColor{ juce::Colours::white };
    juce::Colour backFontColor{ juce::Colours::darkgrey };

    juce::Viewport modulesViewport{ "ModulesViewport" };

    juce::AudioProcessorValueTreeState& parameters;
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    
    juce::ValueTree valueTree;

    InfoPanelSlider outputGainSlider{"Output Gain", "Overall volume control of the plug-in"};
    std::unique_ptr<SliderAttachment> outputGainAttachment;
 
    KrumSamplerAudioProcessor& audioProcessor;
    KrumSampler& sampler;
    KrumFileBrowser& fileBrowser;

    KrumModuleContainer moduleContainer{this, valueTree};
    KrumKeyboard keyboard{ audioProcessor.getMidiState(), juce::MidiKeyboardComponent::Orientation::horizontalKeyboard, moduleContainer, valueTree };

    juce::String madeByString{ "Made by Kris Crawford" };
    juce::URL websiteURL{ "https://www.krismakesmusic.com" };
    InfoPanelTextButton websiteButton{"Website", "Clicking this will open my website. Go check it out yo!"};
    InfoPanelDrawableButton infoButton {"Info Button", "Toggles this Info Panel Box"};
    

    InfoPanelDrawableButton collapseBrowserButton {"Hide Browser", "This will hide the browser and give you more screen real estate when you aren't using the browser anymore","", juce::DrawableButton::ButtonStyle::ImageStretched};
    
    
    class GlobalOutputSlider : public InfoPanelDrawableButton
    {
    public:


        juce::Label label{ "Output" };

    //private:
    };
    
    //juce::DrawableButton collapseBrowserButton{ "Hide Broswer", juce::DrawableButton::ButtonStyle::ImageStretched};
   

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
