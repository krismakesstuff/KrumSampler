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
* A class to hold and manage KrumModuleEditors. It defines the viewport in which they live. 
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
                            public juce::ValueTree::Listener//,
                            //public juce::KeyListener
{
public:
    KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree);
    ~KrumModuleContainer() override;

    void paint(juce::Graphics& g) override;
    
    //positions modules according to thier moduleDisplayIndex
    void refreshModuleLayout();
    
    //valueTree Listener
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    //mouse Listener
    void mouseDown(const juce::MouseEvent& event) override;

    //Key Listener
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

    //MidiKeyboardState Listener
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    
    //makes new modules from a passed file. Used by drop sample area
    KrumModuleEditor* handleNewFile(juce::ValueTree fileTree);
    KrumModuleEditor* handleNewExternalFile(juce::String fullPathName);

    //Module creation and deletion, used by handleNewFile()
    KrumModuleEditor* addNewModuleEditor(KrumModuleEditor* newModuleEditor);
    KrumModuleEditor* insertNewModuleEditor(KrumModuleEditor* newModuleEditor, int indexToInsert);
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);

    //Module selection methods
    void setModuleSelectedFromClick(KrumModuleEditor* moduleToMakeActive, bool setSelected ,const juce::MouseEvent& event);
    void setModulesSelectedToLastSelection(KrumModuleEditor* moduleToSelect);
    void setModuleSelectedWithCommand(KrumModuleEditor* moduleToSelect);
    bool isModuleSelected(KrumModuleEditor* moduleToCheck);
    void deselectAllModules();
    bool multipleModulesSelected();

    //action methods on selected modules
    void clearSelectedSettingsOverlays();
    void setSettingsOverlayOnSelectedModules(bool show, KrumModuleEditor* eventOrigin);
    void clickOneShotOnSelectedModules(const juce::MouseEvent& mouseDownEvent, KrumModuleEditor* eventOrigin, bool mouseDown);
    void setSelectedModulesState(KrumModuleEditor* eventOrigin, int newState);
    void setSelectedModulesColor(KrumModuleEditor* eventOrigin, juce::Colour newColor);
    void setListeningForMidiOnSelectedModules(bool shouldListen, KrumModuleEditor* originComp);
    void repaintAllSelectedModules();

    //multicontrol get & set
    void setMultiControlModifierKey(juce::ModifierKeys::Flags newModKeyFlag);
    juce::ModifierKeys getMultiControlModifierKey();
    bool isMultiControlActive();
    
    //various module getters
    juce::Array<KrumModuleEditor*> getModulesFromMidiNote(int midiNote);
    juce::OwnedArray<KrumModuleEditor>& getActiveModuleEditors();
    int getNumActiveModules();
    int getNumEmptyModules();
    KrumModuleEditor* getActiveModuleEditor(int index);
    KrumModuleEditor* getEditorFromDisplayIndex(int displayIndex);

    //drag handle methods
    void startDraggingEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event);
    void dragEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event);
    void draggingMouseUp(const juce::MouseEvent& event);
    bool isModuleDragging();

    //Multicontrol interface methods
    void reassignSelectedSliderAttachments(KrumModuleEditor* sourceEditor, juce::Slider* slider);
    void updateSlidersIfBeingMultiControlled(KrumModuleEditor* editor, juce::Slider* slider);
    void reassignSelectedButtonAttachments(KrumModuleEditor* sourceEditor, juce::Button* button);
    void reassignSelectedComboAttachments(KrumModuleEditor* sourceEditor, juce::ComboBox* comboBox);
    void updateComboIfBeingMultiControlled(KrumModuleEditor* editor, juce::ComboBox* comboBox);

    //contextual methods usually used by mouseEvents
    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);
    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    
    
    KrumSamplerAudioProcessorEditor* getPluginEditor();
    juce::AudioProcessorValueTreeState* getAPVTS();

private:

    //functions for handling multicontrol drag and drop. WIP, TODO: make work
    juce::Rectangle<int> getModulesDraggedArea();
    KrumModuleEditor* isMouseDragAreaOverActiveEditor(KrumModuleEditor* eventOrigin ,juce::Rectangle<int>& draggedArea);
    void swapModuleEditorsDisplayIndex(int firstIndex, int secondIndex, bool forward);
    
    //internal multicontrol functions
    void setMultiControlState(bool shouldControl);
    void setModuleSelectedState(KrumModuleEditor* moduleToSelect, bool shouldSelect);
    
    //meant to be used in loading the state only
    void createModuleEditors();
    void loadSelectedModules();
    
    //used as internal container function. Use addNewModuleEditor() and insertNewModuleEditor() instead
    KrumModuleEditor* addModuleEditor(KrumModuleEditor* moduleToAdd, int indexToInsert = -1, bool refreshLayout = true);

    //internal
    bool findChildCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type);
    void updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted);
    int getNumVisibleModules();
    juce::ValueTree getFirstEmptyModuleTree();
    void clearActiveModuleSettingsOverlays();
    void loadMidiToModulesListening(int channel, int note);

    //file browser interfacing
    void addFileToRecentsFolder(juce::File& file, juce::String name);
    void addFileToFavoritesFolder(juce::File& file, juce::String name);

    void timerCallback() override;

    //this isn't working as expected, find a way to programmatically set the mod key
    juce::ModifierKeys multiControlModifierKey{juce::ModifierKeys::shiftModifier};

    struct MidiMessage
    {
        int midiNote = 0;
        int midiChannel = 0;
    };
    
    MidiMessage nextMidiMessage{};
    bool applyMidi = false;
    
    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    juce::ValueTree valueTree;
    juce::OwnedArray<KrumModuleEditor> activeModuleEditors{};
    juce::OwnedArray<KrumModuleEditor> currentlySelectedModules{};
    KrumSamplerAudioProcessorEditor* pluginEditor = nullptr;

    bool multiSelectControlState = false;
    juce::ComponentDragger dragger;
    // need to consider multi-selection
    juce::ComponentBoundsConstrainer* moduleEditorConstrainer = nullptr;
    bool moduleBeingDragged = false;
    KrumModuleEditor* editorBeingDragged = nullptr;

    bool modulesSwapping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
