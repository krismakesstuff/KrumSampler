/*
  ==============================================================================

    KrumModuleContainer.h
    Created: 23 Mar 2021 2:56:05pm
    Author:  krisc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InfoPanel.h"


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
    //void sliderValueChanged(juce::Slider* slider) override;
    //ComboBoxListener
    //void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    //Button::Listener
    //void buttonClicked(juce::Button* buttonClicked) override;
    //void buttonStateChanged(juce::Button* buttonChanged) override;
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

    void applyValueTreeChangesToSelectedModules(juce::ValueTree& treeWhoChanged, const juce::Identifier& propertyWhoChanged);
    void applyParameterChangeToSelectedModules(const juce::String& parameterID, float newValue);
   
    void clearSelectedSettingsOverlays();
    void setSettingsOverlayOnSelectedModules(bool show, KrumModuleEditor* eventOrigin);
    void clickOneShotOnSelectedModules(const juce::MouseEvent& mouseDownEvent, KrumModuleEditor* eventOrigin, bool mouseDown);

    //void sendMouseEventToSelectedComponents(juce::Component* compToExlcude, const juce::MouseEvent& event);
    void setListeningForMidiOnSelectedModules(bool shouldListen, KrumModuleEditor* originComp);

    juce::Array<KrumModuleEditor*> getModulesFromMidiNote(int midiNote);

    KrumSamplerAudioProcessorEditor* getPluginEditor();
    juce::OwnedArray<KrumModuleEditor>& getActiveModuleEditors();
    
    int getNumActiveModules();
    //int getNumModuleEditors();
    int getNumEmptyModules();
    KrumModuleEditor* getActiveModuleEditor(int index);

    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);
    void showModulePitchSlider(KrumModuleEditor* moduleEditor);

    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);

    //void showFirstEmptyModule();
    void createModuleEditors();

    void setMultiControlModifierKey(juce::ModifierKeys::Flags newModKeyFlag);
    juce::ModifierKeys getMultiControlModifierKey();

    bool isMultiControlActive();
    void repaintAllSelectedModules();

    KrumModuleEditor* getEditorFromDisplayIndex(int displayIndex);
    juce::AudioProcessorValueTreeState* getAPVTS();

    bool handleNewFile(juce::ValueTree fileTree);

private:


    class DropSampleArea : public InfoPanelComponent,
        public juce::DragAndDropTarget,
        public juce::FileDragAndDropTarget
    {
    public:
        DropSampleArea(KrumModuleContainer* mc);
        ~DropSampleArea() override;

        void paint(juce::Graphics& g) override;

    private:

        //void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
        void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;

        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void filesDropped(const juce::StringArray& files, int x, int y) override;

        KrumModuleContainer* moduleContainer = nullptr;

    };

    DropSampleArea dropSampleArea{ this };

    void clearActiveModuleSettingsOverlays();

    void addParamListeners();
    void removeParamListeners();
    
    void setMultiControlState(bool shouldControl);

    KrumModuleEditor* addModuleEditor(KrumModuleEditor* moduleToAdd, bool refreshLayout = true);

    bool findChildCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type);

    void updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted);
    int getNumVisibleModules();

    juce::ValueTree& getFirstEmptyModuleTree();

    juce::ValueTree valueTree;

    void addFileToRecentsFolder(juce::File& file, juce::String name);

    void timerCallback() override;
    //void setMultiControlState(bool shouldControl);

    void addModuleParamIDs(KrumModuleEditor* module);
    void removeModuleParamIDs(KrumModuleEditor* module);

    bool doesParamIDsContain(const juce::String& paramIDToTest);

    //this isn't working as expected, find a way to programmatically set the mod key
    juce::ModifierKeys multiControlModifierKey{juce::ModifierKeys::shiftModifier};
    juce::StringArray paramIDs{};

    struct ParamIDValue
    {
        juce::String paramID{};
        float value{};
    };

    ParamIDValue nextParamChange{};
    ParamIDValue lastParamChange{}; //use this for handling relative slider moves?

    bool applyNextParamChange = false;
    //ParamIDValue ParamChange{};

    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    juce::OwnedArray<KrumModuleEditor> activeModuleEditors{};
    juce::OwnedArray<KrumModuleEditor> currentlySelectedModules{};

    KrumSamplerAudioProcessorEditor* pluginEditor = nullptr;
    
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    bool multiSelectControlState = false;

    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
