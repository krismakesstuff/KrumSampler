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
#include "KrumFileBrowser.h"

//==============================================================================
/**
*/

//static struct EditorDimensions

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

    //EditorDimensions() {}

    //EditorDimensions(juce::Rectangle<int> area)
    //{
    //    //set these with the proper ratios. (calculate from current static size of app)

    //}
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

    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNote, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNote, float velocity) override;
    
    static juce::String getMidiInfo(const juce::MidiMessage&);

    void createModule(juce::String& moduleName, int index, juce::File& file);
    void createModuleEditors();
    KrumModuleContainer& getModuleContainer();

    void reconstructModuleDisplay(juce::ValueTree& moduleDisplayTree);
    void printModules();

    void hideFileBrowser();
    void showFileBrowser();

    void saveFileBrowserHiddenState();
    bool getSavedFileBrowserHiddenState();

    void updateOutputGainBubbleComp(juce::Component*);

    void setKeyboardNoteColor(int midiNoteNumber, juce::Colour color);
    void addKeyboardListener(juce::MidiKeyboardStateListener* listener);
    void removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove);

    void postMessageToList(const juce::MidiMessage& message, const juce::String& source);
    void addMessageToList(const juce::MidiMessage& message, const juce::String& source);
    void setTextBox(juce::String);

    void cleanUpEmptyModuleTrees();

    juce::AudioFormatManager* getAudioFormatManager();
    KrumSampler& getSampler();
    juce::ValueTree* getValueTree();
    juce::AudioProcessorValueTreeState* getParameters();
    KrumFileBrowser* getFileBrowser();



    juce::SharedResourcePointer<juce::TooltipWindow> toolTipWindow;


private:

    //unsure if these are the right approach for access...
    friend class KrumModule;
    friend class KrumModuleContainer;
    friend class KrumModuleEditor;

    KrumLookAndFeel kLaf{};

    juce::CriticalSection lock;

    //int midiNoteNumber = 60;
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
    
    KrumModuleContainer moduleContainer{this};
    KrumKeyboard keyboard{ audioProcessor.getMidiState()/*.getMidiKeyboardState()*/, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard, moduleContainer };
    KrumFileBrowser fileBrowser;
    KrumFileDrop fileDrop{ *this, moduleContainer, parameters, fileBrowser };


    class IncomingMessageCallback : public juce::CallbackMessage
    {
    public:
        IncomingMessageCallback(KrumSamplerAudioProcessorEditor* o, const juce::MidiMessage& m, const juce::String& s)
            : owner(o), message(m), source(s)
        {}

        void messageCallback() override
        {
            if (owner != nullptr)
                owner->addMessageToList(message, source);
        }

        KrumSamplerAudioProcessorEditor* owner;
        juce::MidiMessage message;
        juce::String source;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IncomingMessageCallback)
    };
  
    static const juce::Font& getWackyFont()
    {
        static juce::Font wacky(juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::MONOGLYCERIDE_TTF,
                                                                                    BinaryData::MONOGLYCERIDE_TTFSize)));
        return wacky;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KrumSamplerAudioProcessorEditor)
};
