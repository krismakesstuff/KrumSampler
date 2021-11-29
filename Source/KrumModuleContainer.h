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
* This also manages interactions with the mouse being over module-editors, specifically the when dragging from the browser.
* 
* 
*/

class KrumModuleEditor;
class DummyKrumModuleEditor;
class KrumModule;
class KrumSamplerAudioProcessorEditor;



class KrumModuleContainer : public juce::Component,
                            public juce::DragAndDropTarget,
                            public juce::Timer,
                            public juce::KeyListener,
                            public juce::MidiKeyboardStateListener
                            //START HERE
                            //public juce::ValueTree::Listener ??
{
public:
    KrumModuleContainer(KrumSamplerAudioProcessorEditor* owner, juce::ValueTree& valTree);
    ~KrumModuleContainer() override;

    void paint(juce::Graphics& g) override;

    void paintLineUnderMouseDrag(juce::Graphics& g, juce::Point<int> mousePosition);
    
    void refreshModuleLayout();
    
   
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)override;
    
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    void addModuleEditor(KrumModuleEditor* newModule, bool refreshLayout = true);
    void removeModuleEditor(KrumModuleEditor* moduleToRemove, bool refreshLayout = true);
    void moveModule(int moduleIndexToMove, int newDisplayIndex);

    void setModuleSelected(KrumModuleEditor* moduleToMakeActive);
    void setModuleUnselected(KrumModuleEditor* moduleToMakeDeselect);
    void deselectAllModules();
    
    KrumModuleEditor* getModuleFromMidiNote(int midiNote);

    void addModuleToDisplayOrder(KrumModuleEditor* moduleToAdd);

    void removeModuleFromDisplayOrder(KrumModuleEditor* moduleToRemove);
    KrumModuleEditor* getEditorFromModule(KrumModule* krumModule);

    void matchModuleDisplayToMidiNotes(juce::Array<int> sortedMidiAssignments);
    
    void updateModuleDisplayIndices(bool repaint);
    
    bool isMouseOverModule(const juce::Point<int> positionToTest, juce::Rectangle<int>& boundsOfModuleUnderMouse);
    void isIntersectingWithModules(KrumModuleEditor* editorToTest);
    
    //    void startModuleDrag(KrumModuleEditor* moduleToDrag, const juce::MouseEvent& e);
    //    void dragModule(KrumModuleEditor* moduleToDrag, const juce::MouseEvent& e);
    //    void endModuleDrag(KrumModuleEditor* moduleToDrag);
    //    bool isModuleBeingDragged();
    //
    //bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    //void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    KrumSamplerAudioProcessorEditor* getEditor();
    juce::OwnedArray<std::shared_ptr<KrumModuleEditor>>& getModuleDisplayOrder();
    
    int getNumActiveModules();
    int getNumModuleEditors();

    void showModuleClipGainSlider(KrumModuleEditor* moduleEditor);

    void showModuleCanAcceptFile(KrumModuleEditor* moduleEditor);
    void hideModuleCanAcceptFile(KrumModuleEditor* moduleEditor);

    //void showFirstEmptyModule();
    
private:

    juce::ValueTree valueTree;

    void timerCallback() override;

    friend class KrumSamplerAudioProcessorEditor;
    friend class KrumSampler;
 
    //could probably use a linked list here? Would make storing position easier when rearranging.
    juce::OwnedArray<KrumModuleEditor> moduleDisplayOrder{};

    //juce::CriticalSection crit;
    //juce::ReferenceCountedArray<KrumModuleEditor*, juce::CriticalSection()> moduleDispalyOrder;

    struct ModuleDragInfo
    {
        bool dragging = false;
        juce::Rectangle<int> origBounds;
        bool showOriginBounds = false;
        juce::Point<int> mousePos;
        bool escPressed = false;
        
        bool drawLineBetween = false;
        int leftDisplayIndex = 0;
        int rightDisplayIndex = 0;
        
        KrumModuleEditor* draggedModule = nullptr;
        
        void setInfo(bool isDragging, juce::Rectangle<int> draggingBounds, juce::Point<int> mousePosition)
        {
            dragging = isDragging;
            origBounds = draggingBounds;
            mousePos = mousePosition;
        }
        
        void reset()
        {
            dragging = false;
            showOriginBounds = false;
            origBounds = {};
            mousePos = {};
            escPressed = false;
            
            drawLineBetween = false;
            leftDisplayIndex = 0;
            rightDisplayIndex = 0;
            
            draggedModule = nullptr;
            DBG("Drag Info reset");
        }
        
    };
    
    ModuleDragInfo moduleDragInfo;
    
    struct ModulesIntersecting
    {
        KrumModuleEditor* first = nullptr;
        KrumModuleEditor* second = nullptr;
        
        void reset() { first = nullptr; second = nullptr;}
    };

    ModulesIntersecting modulesIntersecting;
    

    KrumSamplerAudioProcessorEditor* editor = nullptr;
    
    juce::Colour bgColor{ juce::Colours::black };

    bool modulesOutside = false;
    //juce::Rectangle<int> fadeArea;
    
    int getNumVisibleModules();
    
    //juce::ComponentDragger dragger;
    
    //set constrainer and make esc key exit drag
    //juce::ComponentBoundsConstrainer boundsConstrainer;
    
    //juce::OwnedArray<DummyKrumModuleEditor> editors;

//    class MidiSorter
//    {
//    public:
//        MidiSorter() {}
//
//        int compareElements(int firstMidiNote, int secondMidiNote)
//        {
//
//        }
//
//    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumModuleContainer)

};
