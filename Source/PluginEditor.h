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

//==============================================================================
/*
* 
* A JUCE generated class that represents the GUI. This is created and owned by the host app. 
* This holds all of the Krum GUI elements.
* 
*/

namespace EditorDimensions
{
    static int  topBar = 55;

    static int shrinkage = 5;

    static int moduleH = 500;
    static int moduleW = 125;

    static int addButtonH = 50;
    static int addButtonW = 100;
    static int collapseButtonH = 40;
    static int collapseButtonW = 15;

    static int outputW = 80;
     
    static int keyboardH = 90;

    static int infoH = 500;
    static int fileTreeH = infoH;
    static int fileTreeTitleH = 30;

    static int emptyAreaMinW = 300;
    static int fileTreeW = emptyAreaMinW;

    static float cornerSize = 5.0f;
    static float smallOutline = 1.0f;
    static float bigOutline = 2.0f;

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

    bool createModule(juce::String& moduleName, int index, juce::File& file);
    void createModuleEditors();
    KrumModuleContainer& getModuleContainer();

    void reconstructModuleDisplay(juce::ValueTree& moduleDisplayTree);
    void printModules();

    void hideFileBrowser();
    void showFileBrowser();

    void saveFileBrowserHiddenState();
    bool getSavedFileBrowserHiddenState();

    void updateOutputGainBubbleComp(juce::Component*);

    void setKeyboardNoteColor(int midiNoteNumber, juce::Colour color, int oldNote = 0);
    void removeKeyboardNoteColor(int midiNoteNumber);

    void addKeyboardListener(juce::MidiKeyboardStateListener* listener);
    void removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove);

    void cleanUpEmptyModuleTrees();

    juce::AudioFormatManager* getAudioFormatManager();
    juce::AudioThumbnailCache& getThumbnailCache();

    KrumSampler& getSampler();
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();

    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;

    void updateThumbnails();

private:


    bool needsToUpdateThumbs = false;

    //unsure if these are the right approach for access...
    friend class KrumModule;
    friend class KrumModuleContainer;
    friend class KrumModuleEditor;

    KrumLookAndFeel kLaf{};

    //juce::CriticalSection lock;
    juce::Image titleImage;

    juce::DrawableButton collapseBrowserButton {"Collapse", juce::DrawableButton::ButtonStyle::ImageStretched};
    const juce::Font defaultFont{ "Calibri", 11.0f, juce::Font::FontStyleFlags::plain };

    juce::Rectangle<int> modulesBG;
    juce::Colour modulesBGColor{ juce::Colours::darkgrey.darker(0.999f) };
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
    
    juce::Slider outputGainSlider;
    std::unique_ptr<SliderAttachment> outputGainAttachment;
    
 
    KrumSamplerAudioProcessor& audioProcessor;
    KrumSampler& sampler;
    KrumFileBrowser& fileBrowser;

    KrumModuleContainer moduleContainer{this};
    KrumKeyboard keyboard{ audioProcessor.getMidiState(), juce::MidiKeyboardComponent::Orientation::horizontalKeyboard, moduleContainer };
    KrumFileDrop fileDrop{ *this, moduleContainer, parameters, fileBrowser };

    juce::String madeByString{ "Made by Kris Crawford" };
    juce::URL websiteURL{ "https://www.krismakesmusic.com" };
    juce::TextButton websiteButton;
    juce::Rectangle<int> madeByArea{ 0, 0, 150, 50 };



    //Not Using this font anymore, keeping this here incase I want to add a custom font later
    
//    static const juce::Font& getWackyFont()
//    {
//        static juce::Font wacky(juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::MONOGLYCERIDE_TTF,
//                                                                    BinaryData::MONOGLYCERIDE_TTFSize)));
//        return wacky;
//    }


    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
