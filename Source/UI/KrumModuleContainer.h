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

    void mouseDown(const juce::MouseEvent& event) override;

    //Key Listener
    bool keyPressed(const juce::KeyPress& key, juce::Component* ogComp) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* ogComp) override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    KrumModuleEditor* addNewModuleEditor(KrumModuleEditor* newModuleEditor);
    KrumModuleEditor* insertNewModuleEditor(KrumModuleEditor* newModuleEditor, int indexToInsert);
    
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);

    void setModuleSelectedFromClick(KrumModuleEditor* moduleToMakeActive, bool setSelected ,const juce::MouseEvent& event);
    //void setModuleUnselected(KrumModuleEditor* moduleToMakeDeselect, const juce::MouseEvent& event);
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

    void setSelectedModulesState(KrumModuleEditor* eventOrigin, int newState);
    void setSelectedModulesColor(KrumModuleEditor* eventOrigin, juce::Colour newColor);

    //void sendMouseEventToSelectedComponents(juce::Component* compToExlcude, const juce::MouseEvent& event);
    void setListeningForMidiOnSelectedModules(bool shouldListen, KrumModuleEditor* originComp);

    juce::Array<KrumModuleEditor*> getModulesFromMidiNote(int midiNote);

    KrumSamplerAudioProcessorEditor* getPluginEditor();
    juce::OwnedArray<KrumModuleEditor>& getActiveModuleEditors();
    
    int getNumActiveModules();
    int getNumEmptyModules();
    KrumModuleEditor* getActiveModuleEditor(int index);

    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);

    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);

    void createModuleEditors();

    void setMultiControlModifierKey(juce::ModifierKeys::Flags newModKeyFlag);
    juce::ModifierKeys getMultiControlModifierKey();

    bool isMultiControlActive();
    void repaintAllSelectedModules();

    KrumModuleEditor* getEditorFromDisplayIndex(int displayIndex);
    juce::AudioProcessorValueTreeState* getAPVTS();

    KrumModuleEditor* handleNewFile(juce::ValueTree fileTree);
    KrumModuleEditor* handleNewExternalFile(juce::String fullPathName);

    void startDraggingEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event);
    void dragEditor(KrumModuleEditor* editorToDrag, const juce::MouseEvent& event);
    

    void draggingMouseUp(const juce::MouseEvent& event);
    bool isModuleDragging();

    void reassignSelectedSliderAttachments(KrumModuleEditor* sourceEditor, juce::Slider* slider);
    void updateSlidersIfBeingMultiControlled(KrumModuleEditor* editor, juce::Slider* slider);
    
    void reassignSelectedButtonAttachments(KrumModuleEditor* sourceEditor, juce::Button* button);

    void reassignSelectedComboAttachments(KrumModuleEditor* sourceEditor, juce::ComboBox* comboBox);
    void updateComboIfBeingMultiControlled(KrumModuleEditor* editor, juce::ComboBox* comboBox);


private:

    KrumModuleEditor* isMouseDragAreaOverActiveEditor(KrumModuleEditor* eventOrigin ,juce::Rectangle<int>& draggedArea);
    
    void swapModuleEditorsDisplayIndex(int firstIndex, int secondIndex, bool forward);

    juce::Rectangle<int> getModulesDraggedArea();

    void clearActiveModuleSettingsOverlays();


    //void addParamListeners();
    //void removeParamListeners();
    
    void setMultiControlState(bool shouldControl);
    void setModuleSelectedState(KrumModuleEditor* moduleToSelect, bool shouldSelect);
    
    //meant to be used in loading the state only
    void loadSelectedModules();
    
    KrumModuleEditor* addModuleEditor(KrumModuleEditor* moduleToAdd, int indexToInsert = -1, bool refreshLayout = true);

    bool findChildCompInSelectedModules(juce::Component* compToTest, int& origin, juce::String& type);

    void updateModuleDisplayIndicesAfterDelete(int displayIndexDeleted);
    int getNumVisibleModules();

    juce::ValueTree& getFirstEmptyModuleTree();

    juce::ValueTree valueTree;

    void addFileToRecentsFolder(juce::File& file, juce::String name);
    void addFileToFavoritesFolder(juce::File& file, juce::String name);

    void timerCallback() override;

    //void focusLost(juce::Component::FocusChangeType cause) override;

    //this isn't working as expected, find a way to programmatically set the mod key
    juce::ModifierKeys multiControlModifierKey{juce::ModifierKeys::shiftModifier};
    
    bool resetParameterAttachments = false;
    

    void loadMidiToModulesListening(int channel, int note);

    struct MidiMessage
    {
        int midiNote = 0;
        int midiChannel = 0;
    };
    
    MidiMessage nextMidiMessage{};
    bool applyMidi = false;

    
    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    juce::OwnedArray<KrumModuleEditor> activeModuleEditors{};
    juce::OwnedArray<KrumModuleEditor> currentlySelectedModules{};

    KrumSamplerAudioProcessorEditor* pluginEditor = nullptr;
    
    //juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    bool multiSelectControlState = false;


    juce::ComponentDragger dragger;

    // need to consider multi-selection
    juce::ComponentBoundsConstrainer* moduleEditorConstrainer = nullptr;
    bool moduleBeingDragged = false;
    KrumModuleEditor* editorBeingDragged = nullptr;

    bool modulesSwapping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
