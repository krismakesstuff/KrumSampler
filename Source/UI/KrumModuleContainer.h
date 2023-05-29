/*
  ==============================================================================

    KrumModuleContainer.h
    Created: 23 Mar 2021 2:56:05pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


//==============================================================================
/*
* 
* A class to hold and manage KrumModuleEditors. It holds the module-editors and not the actual modules. It defines the viewport in which they are seen. 
* This also manages interactions with the mouse selecting module-editors, as well as showing the clip-gain slider
* 
* 
*/

class KrumModuleEditor;
class DummyKrumModuleEditor;
class KrumModule;
class KrumSamplerAudioProcessorEditor;



class KrumModuleContainer : public juce::Component,
                            public juce::Timer,
                            public juce::MidiKeyboardStateListener,
                            public juce::ValueTree::Listener,
                            public juce::AudioProcessorValueTreeState::Listener,
                            public juce::Slider::Listener,
                            public juce::ComboBox::Listener,
                            public juce::Button::Listener, 
                            public juce::KeyListener
{
public:
    KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree);
    ~KrumModuleContainer() override;

    void paint(juce::Graphics& g) override;

    void paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition);
    
    void refreshModuleLayout();
    
    //valueTreeListener
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    //APVTS Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    //Slider Listener
    void sliderValueChanged(juce::Slider* slider) override;
    //ComboBoxListener
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    //Button::Listener
    void buttonClicked(juce::Button* buttonClicked) override;
    void buttonStateChanged(juce::Button* buttonChanged) override;
    //Mouse Listener
    void mouseDown(const juce::MouseEvent& event) override;

    //Key Listener
    bool keyPressed(const juce::KeyPress& key, juce::Component* ogComp) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* ogComp) override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    KrumModuleEditor* addNewModuleEditor(KrumModuleEditor* newModuleEditor);
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);

    void setModuleSelected(KrumModuleEditor* moduleToMakeActive);
    void setModuleUnselected(KrumModuleEditor* moduleToMakeDeselect);
    void deselectAllModules();
    void setModulesSelectedToLastSelection(KrumModuleEditor* moduleToSelect);
    void setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect);
    bool isModuleSelected(KrumModuleEditor* moduleToCheck);
    bool multipleModulesSelected();

    void applyChangesToSelectedModules(juce::ValueTree& treeWhoChanged, const juce::Identifier& propertyWhoChanged);
    void clickOneShotOnSelectedModules(const juce::MouseEvent& mouseDownEvent, KrumModuleEditor* eventOrigin, bool mouseDown);


    juce::Array<KrumModuleEditor*> getModulesFromMidiNote(int midiNote);

    KrumSamplerAudioProcessorEditor* getEditor();
    juce::OwnedArray<KrumModuleEditor>& getModuleEditors();
    
    int getNumActiveModules();
    int getNumModuleEditors();
    int getNumEmptyModules();
    KrumModuleEditor* getModuleEditor(int index);

    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);
    void showModulePitchSlider(KrumModuleEditor* moduleEditor);

    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);

    void showFirstEmptyModule();
    void createModuleEditors();

    bool isMultiControlActive();


    KrumModuleEditor* getEditorFromDisplayIndex(int displayIndex);
private:

    KrumModuleEditor* addModuleEditor(KrumModuleEditor* moduleToAdd, bool refreshLayout = true);


    bool findCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type);

    void updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted);
    int getNumVisibleModules();

    juce::ValueTree valueTree;

    void timerCallback() override;
    void setMultiControlState(bool shouldControl);

    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    juce::OwnedArray<KrumModuleEditor> moduleEditors{};
    juce::OwnedArray<KrumModuleEditor> currentlySelectedModules{};

    KrumSamplerAudioProcessorEditor* editor = nullptr;
    
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    bool multiSelectControlState = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
