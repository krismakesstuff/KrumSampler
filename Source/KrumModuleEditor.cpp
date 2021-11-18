/*
  ==============================================================================

    KrumModuleEditor.cpp
    Created: 30 Apr 2021 10:21:42am
    Author:  krisc

  ==============================================================================
*/


#include "KrumModuleEditor.h"
#include "KrumModuleProcessor.h"
#include "KrumModule.h"
#include "PluginEditor.h"
#include "KrumFileBrowser.h"


//This class will one day drag and drop the module to re-arrange the order of the modules displayed
//
//class DragHandle : public InfoPanelDrawableButton
//{
//public:
//    DragHandle(KrumModuleContainer& c, KrumModule& owner, const juce::String& buttonName, juce::DrawableButton::ButtonStyle buttonStyle)
//    : container(c), parentModule(owner), InfoPanelDrawableButton(buttonName, "Drag this handle to move the module around")
//    {
//       // auto edBounds = parentModule.getCurrentModuleEditor()->getBounds();
//        //constrainer.setMinimumOnscreenAmounts(0xffffff, 0xffffff, 0xffffff, 0xffffff);
//        //constrainer.setMinimumOnscreenAmounts(edBounds.getHeight(), edBounds.getWidth(), edBounds.getHeight(), edBounds.getWidth());
//    }
//
//    ~DragHandle() override
//    {}
//
//    void mouseDown(const juce::MouseEvent& e) override
//    {
//        //auto modEd = parentModule.getCurrentModuleEditor();
//        //dragger.startDraggingComponent(parentModule.getCurrentModuleEditor(), e);
//        //container.startModuleDrag(modEd, e.getEventRelativeTo(&container));
//        InfoPanelDrawableButton::mouseDown(e);
//    }
//
//    void mouseDrag(const juce::MouseEvent& e) override
//    {
//        auto modEd = parentModule.getCurrentModuleEditor();
//        modEd->toFront(false);
//        //juce::Point<int> offset = e.getEventRelativeTo(modEd).getPosition();
//        juce::Point<int>offset = getBoundsInParent().getCentre();
//        auto origMouseY = e.getMouseDownY();
//        auto clippedMouse = e.withNewPosition(juce::Point<int>(e.getPosition().getX(), origMouseY));
//
//        parentModule.setModuleDragging(true);
//        //container.startModuleDrag(modEd, e);
//        modEd->startDragging("ModuleDrag-", modEd, modEd->createComponentSnapshot(modEd->getLocalBounds()), false, &offset);//, &clippedMouse.source);
//
//
////        if(modEd->forcedMouseUp)
////        {
////            InfoPanelDrawableButton::mouseUp(e);
////            modEd->forcedMouseUp = false;
////            DBG("Forced Mouse Up");
////        }
////        else
////        {
////            parentModule.info.moduleDragging = true;
////            container.dragModule(modEd, e.getEventRelativeTo(&container));
////
////            DBG("Module Dragging: " + parentModule.getModuleName());
////        }
//
//    }
//
//    void mouseUp(const juce::MouseEvent& e) override
//    {
//        if(parentModule.isModuleDragging())
//        {
//            //auto modEd = parentModule.getCurrentModuleEditor();
//            parentModule.setModuleDragging(false);
//            //container.endModuleDrag(parentModule.getCurrentModuleEditor());
//            //modEd->dragOperationEnded({"ModuleDragEnded-", modEd, e.getPosition()});
//        }
//        InfoPanelDrawableButton::mouseUp(e);
//    }
//
//    //juce::ComponentDragger dragger;
//    //juce::ComponentBoundsConstrainer constrainer;
//
//    KrumModule& parentModule;
//    KrumModuleContainer& container;
//
//};


//===============================================================================================//
//===============================================================================================//


KrumModuleEditor::KrumModuleEditor(KrumModule& o, KrumSamplerAudioProcessorEditor& e)
    :   parent(o), editor(e),
        thumbnail(*this, THUMBNAIL_RES, parent.sampler.getFormatManager(), e.getThumbnailCache())//, settingsMenuCallback(handleSettingsMenuResult)
{
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);
    setVisible(true);
    setPaintingIsUnclipped(true);
    
    //this decides which GUI to draw. If this is active we draw a normal module, if not we draw a ModuleSettingsOverlay
    if (parent.isModuleActive())
    {
        buildModule();
        setAndDrawThumbnail();
    }
    else if(parent.info.moduleState == KrumModule::ModuleState::hasFile)
    {
        needsToBuildModuleEditor = true;
        settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
        showSettingsOverlay();
    }
}

KrumModuleEditor::~KrumModuleEditor()
{}

void KrumModuleEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);

    g.setColour(bgColor);
    juce::Colour c = parent.info.moduleColor.withAlpha(0.5f);

//    if (settingsOverlay != nullptr)
//    {
//        c = settingsOverlay->getSelectedColor().withAlpha(0.5f);
//    }
    if (parent.info.moduleState == KrumModule::ModuleState::empty)
    {
        g.setColour(isMouseOver() ? juce::Colours::grey : juce::Colours::darkgrey);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
        
        g.setColour(juce::Colours::darkgrey);
        g.drawFittedText("Drop Sample Here", area.reduced(20), juce::Justification::centred, 3);
    }
    else if (parent.info.moduleState == KrumModule::ModuleState::active)
    {
        
        juce::Colour bc = parent.info.modulePlaying ? c.brighter() : c;
        
        auto bgGrade = juce::ColourGradient::vertical(bc, (float)area.getY(), juce::Colours::black, area.getBottom());
        g.setGradientFill(bgGrade);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);
        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };

        g.setColour(c);

        g.setFont(11.0f);
        g.drawFittedText("L", leftLabelRect, juce::Justification::centred, 1);
        g.drawFittedText("R", rightLabelRect, juce::Justification::centred, 1);


        auto sliderBounds = volumeSlider.getBoundsInParent().toFloat();
        auto sliderLineBounds = sliderBounds.withTrimmedTop(22).withBottom(sliderBounds.getBottom() - 6).withWidth(sliderBounds.getWidth() + 10).withX(sliderBounds.getX() - 5);
        paintVolumeSliderLines(g, sliderLineBounds);
        
        auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
        auto panSliderAdBounds = panSliderBounds.withY(panSlider.getY() - 5);
        paintPanSliderLines(g, panSliderAdBounds);
        
        g.setColour(bc);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
    }
}

void KrumModuleEditor::paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 20;
    int spaceBetweenLines = bounds.getHeight() / numLines;

    g.setColour(parent.info.moduleColor.withAlpha(0.5f));
    
    juce::Line<float> firstLine{ {bounds.getX(), bounds.getY()}, {bounds.getCentreX() - 5, bounds.getY()} };
    juce::Point<int> firstPoint = firstLine.getStart().toInt();
    g.drawLine(firstLine);
    
    juce::Point<int> zeroLine;
    juce::Line<float> line;
    for (int i = 1; i < numLines; i ++)
    {
        float startX = bounds.getX();
        float endX = bounds.getCentreX() - 5;
        if (i % 2)
        {
            startX += 5;
            endX -= 5;
        }

        line.setStart({ startX, bounds.getY() + ( i * spaceBetweenLines)});
        line.setEnd({ endX,  bounds.getY() + (i * spaceBetweenLines)});
        g.drawLine(line);

        if (i == 7)
        {
            zeroLine = line.getStart().toInt();
        }
    }

    g.drawFittedText("+2", { firstPoint.getX() - 15, firstPoint.getY() - 8 , 15, 15 }, juce::Justification::centredLeft, 1);
    g.drawFittedText("0", { zeroLine.getX() - 15, zeroLine.getY() + 3, 15, 15 }, juce::Justification::centredLeft, 1);

}

void KrumModuleEditor::paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    
    g.setColour(parent.info.moduleColor.withAlpha(0.5f));
    juce::Line<float> midLine{ {bounds.getCentreX() - 2, bounds.getY()},{bounds.getCentreX() - 2 , bounds.getCentreY() } };
    g.drawLine(midLine, 2.0f);

}

void KrumModuleEditor::resized()
{
    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);

    int titleHeight = 32;

    int spacer = 5;
    int thumbnailH = 130;

    int midiLabelH = 40;
    
    int panSliderH = 30;
    int panSliderW = area.getWidth();

    int volumeSliderH = 310;
    int volumeSliderW = area.getWidth() / 2.5;

    int statusButtonH = 40;

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    //thumbnailBG = area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer);
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));
    midiLabel.setBounds(area.withTrimmedTop(thumbnail.getBottom()).withHeight(midiLabelH).reduced(spacer));

    panSlider.setBounds(area.withTop(thumbnail.getBottom() + (spacer * 10)).withBottom(thumbnail.getBottom() + panSliderH + (spacer * 7)).withWidth(panSliderW).withLeft(area.getCentreX() - (panSliderW/2)).withHeight(panSliderH/* - spacer*/)/*.reduced(spacer)*/);
    volumeSlider.setBounds(area.withTop(panSlider.getBottom()/* + spacer*/).withBottom(panSlider.getBottom() + volumeSliderH).withLeft(area.getCentreX() - (volumeSliderW / 2)).withWidth(volumeSliderW)/*.reduced(spacer)*/);

    playButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withWidth(area.getWidth() / 2).reduced(spacer));
    editButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withLeft(playButton.getRight() + spacer).withWidth(area.getWidth() / 2).reduced(spacer));

//    if (dragHandle != nullptr)
//    {
//        dragHandle->setBounds(area.withTop(editButton.getBottom()));
//    }

}

void KrumModuleEditor::mouseDown(const juce::MouseEvent& e)
{
    if (settingsOverlay != nullptr)
    {
        editor.moduleContainer.setModuleSelected(&parent);
    }
    else
    {
        editor.moduleContainer.deselectAllModules();
        //juce::Component::mouseDown(e);
    }
}

void KrumModuleEditor::forceMouseUp()
{
    parent.info.moduleDragging = false;
//    editor.moduleContainer.endModuleDrag(nullptr);
//    forcedMouseUp = true;
}

void KrumModuleEditor::buildModule()
{
    juce::String i = juce::String(parent.info.samplerIndex);

//    int dragHandleSize;
//    auto dragHandleData = BinaryData::getNamedResource("drag_handleblack18dp_svg", dragHandleSize);
//    auto dragHandelIm = juce::Drawable::createFromImageData(dragHandleData, dragHandleSize);

//    dragHandle.reset(new DragHandle{ editor.moduleContainer, parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
//    dragHandle->setImages(dragHandelIm.get());
//    addAndMakeVisible(dragHandle.get());
//    dragHandle->setTooltip("Future Kris will make this drag and drop to re-arrange modules");
//
    addAndMakeVisible(thumbnail);
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleClipGain_ID + i, thumbnail.clipGainSlider));

    addAndMakeVisible(titleBox);
    titleBox.setText(parent.info.name, juce::NotificationType::dontSendNotification);
    titleBox.setFont({ 17.0f });
    titleBox.setColour(juce::Label::ColourIds::textColourId, fontColor);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setEditable(false, true, false);
    titleBox.setTooltip("double-click to change name");
    
    titleBox.onTextChange = [this] { updateName(); };

    addAndMakeVisible(volumeSlider);
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setNumDecimalPlacesToDisplay(2);
    volumeSlider.setDoubleClickReturnValue(true, 1.0f);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTooltip(volumeSlider.getTextFromValue(volumeSlider.getValue()));
    volumeSlider.onValueChange = [this] { updateBubbleComp(&volumeSlider, volumeSlider.getCurrentPopupDisplay()); };
    
    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + i, volumeSlider));

    addAndMakeVisible(panSlider);
    panSlider.setScrollWheelEnabled(false);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setNumDecimalPlacesToDisplay(2); 
    panSlider.setDoubleClickReturnValue(true, 1.0f);
    panSlider.setPopupDisplayEnabled(true, false, this);
    panSlider.setTooltip(panSlider.getTextFromValue(panSlider.getValue()));

    panSlider.onValueChange = [this] { updateBubbleComp(&panSlider, panSlider.getCurrentPopupDisplay()); };

    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + i, panSlider));

    addAndMakeVisible(midiLabel);
    
    
    parent.updateAudioAtomics();

    int playButtonImSize;
    auto playButtonData = BinaryData::getNamedResource("play_arrowblack18dp_svg", playButtonImSize);
    auto playButtonImage = juce::Drawable::createFromImageData(playButtonData, playButtonImSize);
    
    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());

    playButton.onMouseDown = [this] { triggerNoteOnInParent(); };
    playButton.onMouseUp = [this] { triggerNoteOffInParent(); };
    
    
    int editButtonImSize;
    auto editButtonData = BinaryData::getNamedResource("settingsblack18dp_svg", editButtonImSize);
    auto editButtonImage = juce::Drawable::createFromImageData(editButtonData, editButtonImSize);
    
    addAndMakeVisible(editButton);
    editButton.setImages(editButtonImage.get());
    editButton.onClick = [this] { showSettingsMenu(); };
    
    //parent.setModuleActive(true);

    setChildCompColors();
    editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor);

    needsToBuildModuleEditor = false;

    resized();
    repaint();
}


//lots of colors to change
void KrumModuleEditor::setChildCompColors()
{
    auto moduleColor = parent.info.moduleColor;

//    dragHandle->setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
//    dragHandle->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
//    dragHandle->setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.99f));

    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());

    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));

    editButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
    editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));

    titleBox.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::textColourId, moduleColor.contrasting());

    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, moduleColor.darker(0.7f));
    
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker().withAlpha(0.7f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

}


void KrumModuleEditor::showSettingsMenu()
{
    juce::PopupMenu settingsMenu;
    juce::PopupMenu::Options options;
   
    //juce::Rectangle<int> showPoint{ editButton.getScreenBounds().getX(), editButton.getScreenBounds().getY(), 0, 0 };
    
    //settingsMenu.addItem(KrumModule::moduleReConfig_Id, "Re-Config");
    settingsMenu.addItem(KrumModule::ModuleSettingIDs::moduleMidiNote_Id, "Change Midi");
    settingsMenu.addItem(KrumModule::ModuleSettingIDs::moduleColor_Id, "Change Color");
    settingsMenu.addItem(KrumModule::moduleDelete_Id, "Delete Module");

    //settingsMenu.showMenuAsync(options.withTargetScreenArea(editButton.getScreenBounds()), juce::ModalCallbackFunction::create(handleSettingsMenuResult, this));
    //settingsMenu.showMenuAsync(options.withTargetScreenArea(editButton.getScreenBounds()), handleSettingsMenuResult);

    settingsMenu.showMenuAsync(options.withTargetScreenArea(editButton.getScreenBounds()), new ModalManager([this](int choice)
        {
            handleSettingsMenuResult(choice);
        }));
    
}


void KrumModuleEditor::setModuleSelected(bool isModuleSelected)
{
    if (settingsOverlay != nullptr)
    {
        if (isModuleSelected)
        {
            editor.addKeyboardListener(&parent);
        }
        else
        {
            editor.removeKeyboardListener(&parent);
        }

        settingsOverlay->setOverlaySelected(isModuleSelected);
    }
}

void KrumModuleEditor::removeSettingsOverlay(bool keepSettings)
{
    DBG("Module State: " + juce::String(parent.info.moduleState));
    editor.removeKeyboardListener(&parent);
    settingsOverlay.reset();
    cleanUpOverlay(keepSettings);
}


void KrumModuleEditor::showSettingsOverlay(bool selectOverlay)
{
    if (settingsOverlay == nullptr)
    {
        settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
    }

    setModuleButtonsClickState(false);
    addAndMakeVisible(settingsOverlay.get());

    if (selectOverlay)
    {
        //we have the container control selecting so we don't have multiple selections
        editor.getModuleContainer().setModuleSelected(&parent);
    }
    else
    {
        settingsOverlay->setOverlaySelected(false);
    }
}


void KrumModuleEditor::cleanUpOverlay(bool keepSettings)
{
    if (needsToBuildModuleEditor)
    {
        buildModule();
    }

   
    if (keepSettings)
    {
        if (oldMidiNote > 0) //removes the old color assignment on the keyboard
        {
            editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor, oldMidiNote);
        }
        else
        {
            editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor);
        }
        
        parent.sampler.updateModuleSample(&parent); //update midi assignment is Sound
        setChildCompColors();
        setAndDrawThumbnail();
        needsToBuildModuleEditor = true;
        parent.setModuleState(KrumModule::ModuleState::active);
    }

    setModuleButtonsClickState(true);
    resized();
    repaint();
    DBG("Module State: " + juce::String(parent.info.moduleState));
}

void KrumModuleEditor::setModuleButtonsClickState(bool isClickable)
{
    int numChildren = getNumChildComponents();

    for (int i = 0; i < numChildren; i++)
    {
        getChildComponent(i)->setInterceptsMouseClicks(isClickable, isClickable);
    }
}

int KrumModuleEditor::getModuleState()
{
    return parent.getModuleState();
}

int KrumModuleEditor::getModuleSamplerIndex()
{
    return parent.getModuleSamplerIndex();
}

void KrumModuleEditor::setModuleIndex(int newIndex)
{
    parent.setModuleSamplerIndex(newIndex);
}

int KrumModuleEditor::getModuleDisplayIndex()
{
    return parent.getModuleDisplayIndex();
}

void KrumModuleEditor::setModuleDisplayIndex(int newDisplayIndex)
{
    parent.setModuleDisplayIndex(newDisplayIndex);
}

void KrumModuleEditor::setModuleColor(juce::Colour newColor)
{
    parent.info.moduleColor = newColor;
}

juce::Colour KrumModuleEditor::getModuleColor()
{
    return parent.info.moduleColor;
}

int KrumModuleEditor::getModuleMidiNote()
{
    return parent.info.midiNote;
}

juce::String KrumModuleEditor::getModuleMidiNoteString(bool noteName)
{
    int noteNum = parent.info.midiNote;
    if (noteName)
    {
        return juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
    }
    else
    {
        return juce::String(noteNum);
    }
}

void KrumModuleEditor::setModuleMidiNote(int newMidiNote)
{
    parent.setMidiTriggerNote(newMidiNote);
}

int KrumModuleEditor::getModuleMidiChannel()
{
    return parent.info.midiChannel;
}

void KrumModuleEditor::setModuleMidiChannel(int newMidiChannel)
{
    parent.setMidiTriggerChannel(newMidiChannel);
}

void KrumModuleEditor::setModulePlaying(bool isPlaying)
{
    parent.setModulePlaying(isPlaying);
}

bool KrumModuleEditor::isModulePlaying()
{
    return parent.info.modulePlaying;
}

void KrumModuleEditor::updateName()
{
    juce::String name = titleBox.getText();
    parent.setModuleName(name);
}

juce::String KrumModuleEditor::getModuleName()
{
    return parent.info.name;
}

//called when the index of the module has changed so now we need to change the slider attachment assignments as well. Might need to approach this differently for cases of automation within the DAW
//void KrumModuleEditor::reassignSliderAttachments()
//{
//    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + parent.getIndexString(), volumeSlider));
//    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + parent.getIndexString(), panSlider));
//    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleClipGain_ID + parent.getIndexString(), thumbnail.clipGainSlider));
//}

//updates the postion of the "dB" readout of the sliders
void KrumModuleEditor::updateBubbleComp(juce::Slider* slider, juce::Component* comp)
{
    auto bubbleComp = static_cast<juce::BubbleComponent*>(comp);
    if (bubbleComp != nullptr)
    {
        juce::Point<int> pos; 
        juce::BubbleComponent::BubblePlacement bubblePlacement = juce::BubbleComponent::above;
        auto area = getLocalBounds();
        if (slider->getSliderStyle() == juce::Slider::LinearVertical) 
        {
            pos = { area.getCentreX() /*+ 6*/, getMouseXYRelative().getY() - 5 };
        }
        else if (slider->getSliderStyle() == juce::Slider::LinearHorizontal)
        {
            int mouseX = getMouseXYRelative().getX();
            int leftBound = area.getX() + 20;
            int rightBound = area.getRight() - 20;
            if (mouseX < leftBound)
            {
                mouseX = leftBound;
            }
            if (mouseX > rightBound) 
            {
                mouseX = rightBound;
            }
            pos = {  mouseX, slider->getBounds().getCentreY() - 10 };
        }

        bubbleComp->setAllowedPlacement(bubblePlacement);
        bubbleComp->setPosition(pos, 0);
        
    }
    slider->setTooltip(slider->getTextFromValue(slider->getValue()));

}

int KrumModuleEditor::getAudioFileLengthInMs()
{
    return thumbnail.getTotalLength() * 1000;
}


void KrumModuleEditor::setKeyboardColor()
{
    setChildCompColors();
    int midiNote = parent.getMidiTriggerNote();

    if (editor.keyboard.isMidiNoteAssigned(midiNote))
    {
        editor.keyboard.removeMidiNoteColorAssignment(midiNote);
    }

    editor.setKeyboardNoteColor(midiNote, parent.info.moduleColor);
}

//the editor should only want midi if it's being assigned
bool KrumModuleEditor::doesEditorWantMidi()
{
    return settingsOverlay != nullptr;
}

void KrumModuleEditor::handleMidi(int midiChannel, int midiNote)
{
    if (settingsOverlay != nullptr)
    {
        settingsOverlay->handleMidiInput(midiChannel, midiNote);
    }
}

void KrumModuleEditor::removeFromDisplay()
{
    editor.removeKeyboardListener(&parent);
    editor.keyboard.removeMidiNoteColorAssignment(parent.info.midiNote);
    editor.getModuleContainer().removeModuleEditor(this);
}

void KrumModuleEditor::triggerNoteOnInParent()
{
    parent.triggerNoteOn();
}

void KrumModuleEditor::triggerNoteOffInParent()
{
    parent.triggerNoteOff();
}

bool KrumModuleEditor::needsToBuildEditor()
{
    return needsToBuildModuleEditor;
}

bool KrumModuleEditor::needsToDrawThumbnail()
{
    return drawThumbnail;
}

void KrumModuleEditor::setAndDrawThumbnail()
{
    if (shouldCheckDroppedFile())
    {
        handleLastDroppedFile();
    }
    
    thumbnail.setSource (new juce::FileInputSource(parent.info.audioFile));
    auto newFileName = parent.info.audioFile.getFileName();
    parent.setModuleName(newFileName);
    titleBox.setText(parent.info.name, juce::dontSendNotification);
    drawThumbnail = false;
    repaint();
}

bool KrumModuleEditor::shouldCheckDroppedFile()
{
    return thumbnail.checkDroppedFile;
}

void KrumModuleEditor::handleLastDroppedFile()
{
    thumbnail.moveDroppedFileToParent();
}

void KrumModuleEditor::setOldMidiNote(int midiNote)
{
    oldMidiNote = midiNote;
}

bool KrumModuleEditor::isMouseOverThumbnail()
{
    return thumbnail.isMouseOver();
}

bool KrumModuleEditor::thumbnailHitTest(const juce::MouseEvent& mouseEvent)
{
    return thumbnail.contains(mouseEvent.getEventRelativeTo(&thumbnail).position.roundToInt());
}

void KrumModuleEditor::setClipGainSliderVisibility(bool sliderShouldBeVisible)
{
    thumbnail.clipGainSlider.setVisible(sliderShouldBeVisible);
}

bool KrumModuleEditor::canThumbnailAcceptFile()
{
    return thumbnail.canAcceptFile;
}

void KrumModuleEditor::setThumbnailCanAcceptFile(bool shouldAcceptFile)
{
    thumbnail.canAcceptFile = shouldAcceptFile;
    thumbnail.repaint();
}

//void KrumModuleEditor::handleSettingsMenuResult(int result, KrumModuleEditor* parentEditor)
void KrumModuleEditor::handleSettingsMenuResult(int result)
{
        auto localBounds = getLocalBounds();
       if (result == KrumModule::moduleReConfig_Id)
       {
           settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
           settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
           settingsOverlay->keepCurrentColor(true);
           showSettingsOverlay(true);
       }
       else if (result == KrumModule::ModuleSettingIDs::moduleMidiNote_Id)
       {
           settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
           settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
           settingsOverlay->keepCurrentColor(true);
           showSettingsOverlay(true);
       }
       else if (result == KrumModule::ModuleSettingIDs::moduleColor_Id)
       {
          settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
          settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
          settingsOverlay->showColorsOnly();
          showSettingsOverlay(true);
       }
       else if (result == KrumModule::moduleDelete_Id)
       {
           //removeFromDisplay();
           //parent.deleteEntireModule();
           parent.setModuleState(KrumModule::ModuleState::empty);
       }
    
}

//Drag and Drop Target
bool KrumModuleEditor::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    auto desc = dragDetails.description.toString();
    return desc.isNotEmpty() && (desc.contains("FileBrowserDrag-") || desc.contains("ModuleDrag-"));
}

//Drag and Drop Target
void KrumModuleEditor::itemDropped(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    auto desc = dragDetails.description.toString();
    
    if(desc.contains("FileBrowserDrag-"))
    {
        int numDroppedItems = editor.fileBrowser.getNumSelectedItems();
        if(numDroppedItems > 1)
        {
            int numFreeModules, firstFreeIndex;
            parent.sampler.getNumFreeModules(numFreeModules, firstFreeIndex);
            
            if(numDroppedItems <= numFreeModules) //checks to make sure we have enough modules
            {
                for (int i = 0; i < numDroppedItems; i++)
                {
                    auto krumItem = editor.fileBrowser.getSelectedItem(i);
                    if (krumItem != nullptr)
                    {
                        auto file = krumItem->getFile();
                        auto itemName = krumItem->getItemName();
                        if (!krumItem->mightContainSubItems())
                        {
                            if(parent.sampler.isFileAcceptable(file))
                            {
                                
                                auto mod = parent.sampler.getModule(firstFreeIndex++);
                                if(!mod->hasEditor())
                                {
                                    editor.moduleContainer.addModuleEditor(mod->createModuleEditor(editor));
                                }
                                
                                mod->getCurrentModuleEditor()->updateModuleFile(file);

                                DBG("-------");
                                DBG("Module: " + juce::String(mod->getModuleSamplerIndex()));
                                DBG("Item: " + file.getFullPathName());
                            }
                        }
                        else
                        {
                            DBG("Folders Not Supported");
                            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Folders Not Supported Yet", "I might support folder dropping in the future. For now, drop the folder into favorites to keep it locally.");
                        }
                    }
                    else
                    {
                        DBG("Krum Item NULL");
                    }
                }
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not enough modules available", "");
            }
        }
        else
        {
            auto krumItem = editor.fileBrowser.getSelectedItem(0);
            if (krumItem != nullptr)
            {
                auto file = krumItem->getFile();
                auto itemName = krumItem->getItemName();
                if (!krumItem->mightContainSubItems())
                {
                    if(parent.sampler.isFileAcceptable(file))
                    {
                        DBG("Item: " + file.getFullPathName());
                        updateModuleFile(file);
                        //editor.showLastModule();
                    }
                }
                else
                {
                    DBG("Folders Not Supported");
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Folders Not Supported Yet", "I might support folder dropping in the future. For now, drop the folder into favorites to keep it locally.");
                }
            }
            else
            {
                DBG("Krum Item NULL");
            }
        }
    }
//    else if (desc.contains("ModuleDrag-"))
//    {
//        auto modEdDropped = static_cast<KrumModuleEditor*>(dragDetails.sourceComponent.get());
//        DBG("Module " + juce::String(modEdDropped->getModuleSamplerIndex()) + " Dropped on Module " + juce::String(getModuleSamplerIndex()));
//        if(modEdDropped)
//        {
//            editor.moduleContainer.moveModule(modEdDropped->getModuleDisplayIndex(), parent.getModuleDisplayIndex());
//        }
//    }
    editor.addNextModuleEditor();
}

void KrumModuleEditor::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    //
}

//EXTERNAL File Drag and Drop Target
bool KrumModuleEditor::isInterestedInFileDrag(const juce::StringArray &files)
{
    //check if file format is supported?
    return true;
}

//EXTERNAL File Drag and Drop Target
void KrumModuleEditor::filesDropped(const juce::StringArray &files, int x, int y)
{
    
    if(files.size() > 1)
    {
        int numFreeModules, firstFreeIndex;
        parent.sampler.getNumFreeModules(numFreeModules, firstFreeIndex);
        
        if(files.size() <= numFreeModules)
        {
            for (auto file : files)
            {
                juce::File audioFile {file};
                if(parent.sampler.isFileAcceptable(audioFile))
                {
                    //auto mod = parent.sampler.getModule(firstFreeIndex);
                    auto mod = parent.sampler.getModule(firstFreeIndex++);
                    if(!mod->hasEditor())
                    {
                        editor.moduleContainer.addModuleEditor(mod->createModuleEditor(editor));
                    }
                    
                    mod->getCurrentModuleEditor()->updateModuleFile(audioFile);

                    DBG("-------");
                    DBG("Module: " + juce::String(mod->getModuleSamplerIndex()));
                    DBG("Item: " + audioFile.getFullPathName());
                }
            }
        }
    }
    else
    {
        juce::File audioFile{ files[0] };
        if (parent.sampler.isFileAcceptable(audioFile))
        {
            updateModuleFile(audioFile);
            DBG("File: " + audioFile.getFullPathName());
        }
        else
        {
            DBG("Audio Format Not Supported");
        }
        
    }
    //editor.moduleContainer.showFirstEmptyModule();
    editor.addNextModuleEditor();
}

void KrumModuleEditor::updateModuleFile(juce::File& file)
{
    parent.setSampleFile(file);
    parent.setModuleName(file.getFileName());
    parent.sampler.updateModuleSample(&parent);
    editor.fileBrowser.addFileToRecent(file, parent.info.name);
    
    repaint();
}

//============================================================================================================================

KrumModuleEditor::OneShotButton::OneShotButton()
    //:DrawableButton("PlayButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground)
: InfoPanelDrawableButton("One Shot", "Plays the currently assigned sample", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground)
{}

KrumModuleEditor::OneShotButton::~OneShotButton()
{}

void KrumModuleEditor::OneShotButton::mouseDown(const juce::MouseEvent& e)
{
    Button::mouseDown(e);
    if (onMouseDown)
    {
        onMouseDown();
    }

}

void KrumModuleEditor::OneShotButton::mouseUp(const juce::MouseEvent& e)
{
    Button::mouseUp(e);
    
    if (onMouseUp)
    {
        onMouseUp();
    }

}

//============================================================================================================================

KrumModuleEditor::MidiLabel::MidiLabel(KrumModuleEditor* editor)
    : moduleEditor(editor)
{
    setTooltip("Lane: " + juce::String(moduleEditor->getModuleSamplerIndex()));
}

KrumModuleEditor::MidiLabel::~MidiLabel()
{
    
}
 
void KrumModuleEditor::MidiLabel::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    g.setColour(juce::Colours::black);
    g.fillRect(area);
    
    g.setColour(juce::Colours::white.darker());
    g.setFont(14.0f);

    juce::Rectangle<int> midiNoteRect = area.withTrimmedBottom(area.getHeight() / 2).withX(5).withWidth(area.getWidth() - 10);
    juce::Rectangle<int> midiChanRect = area.withTrimmedTop(area.getHeight() / 2).withX(5).withWidth(area.getWidth() - 10);

    g.drawFittedText("Note:", midiNoteRect, juce::Justification::centredLeft, 1);
    g.drawFittedText(moduleEditor->getModuleMidiNoteString(true), midiNoteRect, juce::Justification::centredRight, 1);

    g.drawFittedText("Channel:", midiChanRect, juce::Justification::centredLeft, 1);
    g.drawFittedText(juce::String(moduleEditor->getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);
}

//juce::String KrumModuleEditor::MidiLabel::getTooltip()
//{
//    return "Lane: " + juce::String(moduleEditor->getModuleSamplerIndex());
//}

//============================================================================================================================
