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
#include "KrumFileDrop.h"
#include "KrumModuleContainer.h"
#include "InfoPanel.h"


//==============================================================================
/*
* 
* A JUCE generated class that represents the GUI. This is created and owned by the host app. 
* This holds all of the Krum GUI elements.
* 
*/

namespace EditorDimensions
{
    const static int windowH = 650;
    const static int windowW = 1200;
    const static int windowWNoBrowser = 900;
    
    const static int  topBar = 65;
    const static int shrinkage = 5;
    const juce::Rectangle<int> madeByArea{ 0, 0, 150, 35 };

    const static int moduleH = 450;
    const static int moduleW = 120;

    const static int addButtonH = 50;
    const static int addButtonW = 100;
    const static int collapseButtonH = 40;
    const static int collapseButtonW = 15;

    const static int outputW = 80;
    const static int keyboardH = 90;

    //const static int titleImageH;
    const static int titleImageW = 420; //nice
    
    //const static int infoH = 650;
    const static int fileTreeH = 600;
    const static int fileTreeTitleH = 30;

    //const static int emptyAreaMinW = 350;
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
                                            public juce::MidiKeyboardStateListener,
                                            public juce::DragAndDropContainer
{
public:
    KrumSamplerAudioProcessorEditor (KrumSamplerAudioProcessor&, KrumSampler&,/* KrumModuleContainer* contain,*/ 
                                    juce::AudioProcessorValueTreeState& vts,
                                    juce::ValueTree& valueTree, juce::ValueTree& fileBrowserTree);
    ~KrumSamplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void paintOutputVolumeLines(juce::Graphics& g, juce::Rectangle<float> bounds);
    void resized() override;

    void visibilityChanged() override;

    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNote, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNote, float velocity) override;
    
    static juce::String getMidiInfo(const juce::MidiMessage&);

    //bool createModule(juce::String& moduleName, int index, juce::File& file);
    //void createModuleEditors();
    void addNextModuleEditor();
    KrumModuleContainer& getModuleContainer();

    void reconstructModuleDisplay(juce::ValueTree& moduleDisplayTree);
    void printModules();

    void hideFileBrowser();
    void showFileBrowser();

    
    void saveFileBrowserHiddenState();
    bool getSavedFileBrowserHiddenState();

    void infoButtonClicked();
    bool getSavedInfoButtonState();
    void saveInfoButtonState();
    
    
    void updateOutputGainBubbleComp(juce::Component*);

    //void setKeyboardNoteColor(int midiNoteNumber, juce::Colour color, int oldNote = 0);
    void removeKeyboardNoteColor(int midiNoteNumber);

    void addKeyboardListener(juce::MidiKeyboardStateListener* listener);
    void removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove);

    //void cleanUpEmptyModuleTrees();

    juce::AudioFormatManager* getAudioFormatManager();
    juce::AudioThumbnailCache& getThumbnailCache();

    KrumSampler& getSampler();
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();
    juce::Viewport* getModuleViewport();

    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;

    //void updateThumbnails();
    
    

private:


    bool needsToUpdateThumbs = false;

    friend class KrumModuleContainer;
    friend class KrumModuleEditor;
    friend class DragAndDropThumbnail;

    KrumLookAndFeel kLaf{};

    juce::Image titleImage;

    //juce::DrawableButton collapseBrowserButton {"Collapse", juce::DrawableButton::ButtonStyle::ImageStretched};
    InfoPanelDrawableButton collapseBrowserButton {"Hide Browser", "This will hide the browser and give you more screen real estate when you aren't using the browser anymore"};
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
    
    

    //Not Using this font anymore, keeping this here incase I want to add a custom font later
    
//    static const juce::Font& getWackyFont()
//    {
//        static juce::Font wacky(juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::MONOGLYCERIDE_TTF,
//                                                                    BinaryData::MONOGLYCERIDE_TTFSize)));
//        return wacky;
//    }

    
    

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
