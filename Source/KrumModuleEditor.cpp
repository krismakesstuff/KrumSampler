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



//For now this class doesn't actuall do anything. It will one day drag and drop the module to re-arrange the order of the modules displayed
//class DragHandle : public juce::DrawableButton
//{
//public:
//    DragHandle(KrumModule& owner, const juce::String& buttonName, juce::DrawableButton::ButtonStyle buttonStyle)
//        : parentModule(owner), juce::DrawableButton(buttonName, buttonStyle)
//    {}
//
//    ~DragHandle() override
//    {}
//
//    void mouseDrag(const juce::MouseEvent& e) override
//    {
//        parentModule.info.moduleDragging = true;
//        //DBG("Module Dragging: " + parentModule->getModuleName());
//        parentModule.startDragging("ModuleDragAndDrop", parentModule.getCurrentModuleEditor(), juce::Image(), true);
//    }
//
//    void mouseUp(const juce::MouseEvent& e) override
//    {
//        parentModule.info.moduleDragging = false;
//    }
//
//
//    KrumModule& parentModule;
//
//};


//===============================================================================================//
//===============================================================================================//


KrumModuleEditor::KrumModuleEditor(KrumModule& o, KrumModuleProcessor& p, KrumSamplerAudioProcessorEditor& e)
    :   parent(o), moduleProcessor(p), editor(e),
        thumbnail(*this, THUMBNAIL_RES, moduleProcessor.sampler.getFormatManager(), e.getThumbnailCache())
{
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);
    setVisible(true);
    setPaintingIsUnclipped(true);

    //this decides if this is a brand new module with no information or an exisiting one. If it's new, we create a settings overlay by default. Otherwise we just build out as normal
    if (parent.info.midiNote == 0 || parent.info.midiChannel == 0)
    {
        needsToBuildModuleEditor = true;
        settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
        showSettingsOverlay();
    }
    else
    {
        buildModule();
        needsToDrawThumbnail = true;
    }

}

KrumModuleEditor::~KrumModuleEditor()
{
}

void KrumModuleEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(bgColor);
    juce::Colour c = parent.info.moduleColor.withAlpha(0.5f);

    if (settingsOverlay != nullptr)
    {
        c = settingsOverlay->getSelectedColor().withAlpha(0.5f);
    }
    else
    {
        g.setColour(parent.info.modulePlaying ? c.brighter() : c);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);

        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> midiNoteRect{ 10, thumbnail.getBottom() + 5, area.getWidth() - 20, 20};
        juce::Rectangle<int> midiChanRect{ 10, midiNoteRect.getBottom() - 5, area.getWidth() - 20, 20 };

        juce::Rectangle<int> labelsBGRect = midiNoteRect.withBottom(midiChanRect.getBottom()).withX(thumbnail.getX()).withWidth(thumbnail.getWidth());

        g.setColour(c.darker(0.5f));
        g.fillRect(labelsBGRect);
        
        g.setColour(c);
        //g.drawRect(labelsBGRect);
        //g.setColour(fontColor);

        g.setFont(11.0f);
        g.drawFittedText("L", leftLabelRect, juce::Justification::centred, 1);
        g.drawFittedText("R", rightLabelRect, juce::Justification::centred, 1);

        /*juce::Rectangle<int> gainLabelRect{ area.getCentreX() - 20, volumeSlider.getBottom() - 25, 40, 40 };
        g.drawFittedText("Gain", gainLabelRect, juce::Justification::centred, 1);*/
        g.setColour(fontColor);
        g.setFont(14.0f);

        g.drawFittedText("Note:", midiNoteRect, juce::Justification::centredLeft, 1);
        g.drawFittedText(getModuleMidiNoteString(true), midiNoteRect, juce::Justification::centredRight, 1);

        g.drawFittedText("Channel:", midiChanRect, juce::Justification::centredLeft, 1);
        g.drawFittedText(juce::String(getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);

    }

    auto sliderBounds = volumeSlider.getBoundsInParent().toFloat();
    auto sliderLineBounds = sliderBounds.withTrimmedTop(22).withBottom(sliderBounds.getBottom() - 6).withWidth(sliderBounds.getWidth() + 10).withX(sliderBounds.getX() - 5);
    paintVolumeSliderLines(g, sliderLineBounds);

    auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
    auto panSliderAdBounds = panSliderBounds.withY(panSlider.getY() - 5);
    paintPanSliderLines(g, panSliderAdBounds);
    
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
    auto area = getLocalBounds();

    int titleHeight = 32;

    int spacer = 5;
    int thumbnailH = 120;

    int panSliderH = 25;
    int panSliderW = area.getWidth();

    int volumeSliderH = 260;
    int volumeSliderW = area.getWidth() / 2.5;

    int statusButtonH = 40;

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    //thumbnailBG = area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer);
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));


    panSlider.setBounds(area.withTop(thumbnail.getBottom() + (spacer * 10)).withBottom(thumbnail.getBottom() + panSliderH + (spacer * 7)).withWidth(panSliderW).withLeft(area.getCentreX() - (panSliderW/2)).withHeight(panSliderH/* - spacer*/)/*.reduced(spacer)*/);
    volumeSlider.setBounds(area.withTop(panSlider.getBottom()/* + spacer*/).withBottom(panSlider.getBottom() + volumeSliderH).withLeft(area.getCentreX() - (volumeSliderW / 2)).withWidth(volumeSliderW)/*.reduced(spacer)*/);

    playButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withWidth(area.getWidth() / 2).reduced(spacer));
    editButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withLeft(playButton.getRight() + spacer).withWidth(area.getWidth() / 2).reduced(spacer));

    /*if (dragHandle != nullptr)
    {
        dragHandle->setBounds(area.withTop(editButton.getBottom()));
    }*/

}

void KrumModuleEditor::mouseDown(const juce::MouseEvent& e)
{
    if (settingsOverlay != nullptr)
    {
        editor.moduleContainer.setModuleSelected(&parent);
    }
    else
    {
        juce::Component::mouseDown(e);
    }
}

void KrumModuleEditor::buildModule()
{
    juce::String i = juce::String(parent.info.index);

    /*int dragHandleSize;
    auto dragHandleData = BinaryData::getNamedResource("drag_handleblack18dp_svg", dragHandleSize);
    auto dragHandelIm = juce::Drawable::createFromImageData(dragHandleData, dragHandleSize);

    dragHandle.reset(new DragHandle{ parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
    dragHandle->setImages(dragHandelIm.get());
    addAndMakeVisible(dragHandle.get());
    dragHandle->setTooltip("Future Kris will make this drag and drop to re-arrange modules");*/

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
    
    parent.setModuleActive(true);

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

    /*dragHandle->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::transparentBlack);
    dragHandle->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
    dragHandle->setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);*/

    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());

    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.7f));
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));

    editButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.7f));
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

    settingsMenu.showMenuAsync(options.withTargetScreenArea(editButton.getScreenBounds()), juce::ModalCallbackFunction::create(handleSettingsMenuResult, this));

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
    editor.removeKeyboardListener(&parent);
    settingsOverlay.reset();
    cleanUpOverlay(keepSettings);
}


void KrumModuleEditor::showSettingsOverlay(bool selectOverlay)
{
    if (settingsOverlay != nullptr)
    {
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
    else
    {
        //removeMouseListener(this);
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
        if (oldMidiNote > 0)
        {
            editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor, oldMidiNote);
        }
        else
        {
            editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor);
        }
        moduleProcessor.sampler.updateModuleSample(&parent);
        setAndDrawThumbnail();
        setChildCompColors();
    }

    setModuleButtonsClickState(true);
}

void KrumModuleEditor::setModuleButtonsClickState(bool isClickable)
{
    int numChildren = getNumChildComponents();

    for (int i = 0; i < numChildren; i++)
    {
        getChildComponent(i)->setInterceptsMouseClicks(isClickable, isClickable);
    }
}

int KrumModuleEditor::getModuleIndex()
{
    return parent.getModuleIndex();
}

void KrumModuleEditor::setModuleIndex(int newIndex)
{
    parent.setModuleIndex(newIndex);
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

//called when the index of the module has changed so now we need to change the slider attachment assignments as well. Might need to approach this differently for cases of automation within the DAW
void KrumModuleEditor::reassignSliderAttachments()
{
    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + parent.getIndexString(), volumeSlider));
    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + parent.getIndexString(), panSlider));
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleClipGain_ID + parent.getIndexString(), thumbnail.clipGainSlider));
}

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

void KrumModuleEditor::setAndDrawThumbnail()
{
    thumbnail.setSource(new juce::FileInputSource(parent.info.audioFile)); 
    repaint();
    needsToDrawThumbnail = false;
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

void KrumModuleEditor::handleSettingsMenuResult(int result, KrumModuleEditor* parentEditor)
{
    auto& parent = parentEditor->parent;
    auto localBounds = parentEditor->getLocalBounds();
    if (result == KrumModule::moduleReConfig_Id)
    {

        parentEditor->settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
        parentEditor->settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
        parentEditor->settingsOverlay->keepCurrentColor(true);
        parentEditor->showSettingsOverlay(true);
    }
    else if (result == KrumModule::ModuleSettingIDs::moduleMidiNote_Id)
    {
        parentEditor->settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
        parentEditor->settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
        parentEditor->settingsOverlay->keepCurrentColor(true);
        parentEditor->showSettingsOverlay(true);
    }
    else if (result == KrumModule::ModuleSettingIDs::moduleColor_Id)
    {
        parentEditor->settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
        parentEditor->settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
        parentEditor->settingsOverlay->showColorsOnly();
        parentEditor->showSettingsOverlay(true);
    }
    else if (result == KrumModule::moduleDelete_Id)
    {
        //removeFromDisplay();
        parentEditor->parent.deleteEntireModule();
    }

}

//============================================================================================================================

KrumModuleEditor::OneShotButton::OneShotButton()
    :DrawableButton("PlayButton", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground)
{
}

KrumModuleEditor::OneShotButton::~OneShotButton()
{
}

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
