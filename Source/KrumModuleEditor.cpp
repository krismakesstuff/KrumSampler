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
#include "InfoPanel.h"


//===============================================================================================//

KrumModuleEditor::KrumModuleEditor(juce::ValueTree& modTree, KrumSamplerAudioProcessorEditor& e, juce::AudioFormatManager& fm/*, int state*/)
    : moduleTree(modTree), editor(e),
    thumbnail(*this, THUMBNAIL_RES, fm, e.getThumbnailCache())
{
    setPaintingIsUnclipped(true);

    //We make a settings overlay in the ctor and keep it for the entire life of the module editor so we don't have to heap allocate when changing settings
    settingsOverlay.reset(new ModuleSettingsOverlay(*this));
    addChildComponent(settingsOverlay.get());

    moduleTree.addListener(this);
    //setModuleState(state);

    startTimerHz(30);
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);

    int state = getModuleState();

    if (state == KrumModule::ModuleState::hasFile)
    {
        needsToBuildModuleEditor = true;
        showSettingsOverlay();
    }
    else if (state == KrumModule::ModuleState::active)
    {
        buildModule();
    }

}

KrumModuleEditor::~KrumModuleEditor()
{}

void KrumModuleEditor::paint (juce::Graphics& g)
{
    if (settingsOverlay->isVisible())
    {
        return;
    }

    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);

    juce::Colour c = getModuleColor().withAlpha(0.8f);

    int moduleState = getModuleState();

    if (moduleState == KrumModule::ModuleState::empty)
    {
        g.setColour(isMouseOver() ? juce::Colours::grey : juce::Colours::darkgrey);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
        
        g.setColour(juce::Colours::darkgrey);
        g.drawFittedText("Drop Sample Here", area.reduced(20), juce::Justification::centred, 3);
    }
    else if (moduleState == KrumModule::ModuleState::active) //if the moduleState is hasfile we will be showing the settingsOverlay
    {
        juce::Colour bc = modulePlaying ? c.brighter() : c;
        
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
        //auto sliderLineBounds = sliderBounds.withTrimmedTop(22).withBottom(sliderBounds.getBottom() - 6).withWidth(sliderBounds.getWidth() + 10).withX(sliderBounds.getX() - 5);
        auto sliderLineBounds = sliderBounds.withTrimmedTop(10);
        paintVolumeSliderLines(g, sliderBounds);
        
        auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
        auto panSliderAdBounds = panSliderBounds.withY(panSlider.getY() - 5);
        paintPanSliderLines(g, panSliderAdBounds);
        
        g.setColour(bc);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
    }
}

void KrumModuleEditor::paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 17;
    int spaceBetweenLines = (bounds.getHeight() - 10) / numLines;
    juce::Colour c = juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor).toString()).withAlpha(0.5f);

    float zerodBY = volumeSlider.getPositionOfValue(1.0f) - 1;
    juce::Line<float> zeroLine{ {bounds.getX() - 2, bounds.getY() + zerodBY }, {bounds.getCentreX() - 5 ,  bounds.getY()+ zerodBY } };
    g.setColour(c.darker(0.5f));
    g.drawLine(zeroLine);
    
    
    g.setColour(c);

    juce::Line<float> firstLine{ {bounds.getX(), bounds.getY() + 9}, {bounds.getCentreX() - 5, bounds.getY() + 9} };
    juce::Point<int> firstPoint = firstLine.getStart().toInt();
    g.drawLine(firstLine);




    juce::Line<float> line;

    for (int i = 1; i < numLines; i++)
    {
        if (i == 7) 
        {
            //this draws on top of the zeroDb Line, which we've already drawn
            continue;
        }

        float startX = bounds.getX() + 2;
        float endX = bounds.getCentreX() - 5;
        if (i % 2)
        {
            startX += 5;
            endX -= 4;
        }

        line.setStart({ startX, firstLine.getStartY() + ( i * spaceBetweenLines)});
        line.setEnd({ endX, firstLine.getStartY() + (i * spaceBetweenLines)});
        g.drawLine(line);

    }

    g.drawFittedText("+2", { firstPoint.getX() - 15, firstPoint.getY() - 8 , 17, 17 }, juce::Justification::centredLeft, 1);
    g.drawFittedText("0", { (int)zeroLine.getStartX() - 15, (int)zeroLine.getStartY() - 8, 17, 17 }, juce::Justification::centredLeft, 1);

}

void KrumModuleEditor::paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::Colour c = juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor).toString()).withAlpha(0.5f);
    g.setColour(c);
    juce::Line<float> midLine{ {bounds.getCentreX() - 2, bounds.getY()},{bounds.getCentreX() - 2 , bounds.getCentreY() } };
    g.drawLine(midLine, 1.5f);

}

void KrumModuleEditor::resized()
{
    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);

    int titleHeight = area.getHeight() * 0.08f;

    int spacer = 5;
    int thumbnailH = area.getHeight() * 0.25f;

    int midiLabelH = area.getHeight() * 0.093f;

    int panSliderH = area.getHeight() * 0.05f;
    int panSliderW = area.getWidth() * 0.9f;

    int volumeSliderH = area.getHeight() * 0.48f;
    int volumeSliderW = area.getWidth() * 0.4;

    int buttonH = area.getHeight() * 0.085f;

    settingsOverlay->setBounds(area);

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));
    midiLabel.setBounds(area.withTrimmedTop(thumbnail.getBottom()).withHeight(midiLabelH).reduced(spacer));

    //panSlider.setBounds(area.withTop(thumbnail.getBottom() + (spacer * 10)).withBottom(thumbnail.getBottom() + panSliderH + (spacer * 7)).withWidth(panSliderW).withLeft(area.getCentreX() - (panSliderW/2)).withHeight(panSliderH/* - spacer*/)/*.reduced(spacer)*/);
    panSlider.setBounds(area.getX() + spacer, midiLabel.getBottom() + spacer, panSliderW, panSliderH);
    volumeSlider.setBounds(area.getCentreX() - (volumeSliderW / 2), panSlider.getBottom() + (spacer * 3), volumeSliderW, volumeSliderH);
    
    playButton.setBounds(area.withTop(volumeSlider.getBottom() /*- (spacer * 2)*/).withHeight(buttonH).withWidth(area.getWidth() / 2).reduced(spacer));
    editButton.setBounds(area.withTop(volumeSlider.getBottom() /*- (spacer * 2)*/).withHeight(buttonH).withLeft(playButton.getRight() + spacer).withWidth(area.getWidth() / 2).reduced(spacer));

//    if (dragHandle != nullptr)
//    {
//        dragHandle->setBounds(area.withTop(editButton.getBottom()));
//    }

}

void KrumModuleEditor::mouseEnter(const juce::MouseEvent& event)
{
    KrumModule::ModuleState moduleState = static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState));
    if(moduleState == KrumModule::ModuleState::empty)
    {
        InfoPanel::shared_instance().setInfoPanelText("Empty Module", "Drop a file(s) to fill this module with a sample and then assign it a midi note and color.");
    }
    else if (settingsOverlay->isVisible())
    {
        InfoPanel::shared_instance().setInfoPanelText("Settings Overlay", "To assign a new midi note, enable midi listen, and play/click your note. You can also change the color or delte the module");
    }

 //   editor.keyboard.setHighlightKey(getModuleMidiNote(), true);

    juce::Component::mouseEnter(event);
}

void KrumModuleEditor::mouseExit(const juce::MouseEvent& event)
{
    KrumModule::ModuleState moduleState = static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState));

    if (moduleState == KrumModule::ModuleState::empty || settingsOverlay->isVisible())
    {
        InfoPanel::shared_instance().clearPanelText();
    }

    //if (!getBounds().contains(event.getEventRelativeTo(this).getPosition())) //making sure this mouseExit is being called for leaving the whole module
    //{
    //    editor.keyboard.setHighlightKey(getModuleMidiNote(), false);
    //}

    juce::Component::mouseExit(event);
}

void KrumModuleEditor::mouseDown(const juce::MouseEvent& e)
{
    if (settingsOverlay->isVisible())
    {
        editor.moduleContainer.setModuleSelected(this);
    }
    else
    {
        editor.moduleContainer.deselectAllModules();
        //juce::Component::mouseDown(e);
    }
    juce::Component::mouseDown(e);
}

void KrumModuleEditor::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if(treeWhoChanged == moduleTree)
    {
        if (property == TreeIDs::moduleState)
        {
            int state = treeWhoChanged[property];
            if (state == KrumModule::ModuleState::hasFile)
            {
                needsToBuildModuleEditor = true;
                showSettingsOverlay(true);
                settingsOverlay->keepCurrentColor(false);
            }
            else if (state == KrumModule::ModuleState::active)
            {
                buildModule();
            }
            else if (state == KrumModule::ModuleState::empty)
            {
                zeroModuleTree();
            }
        }
        else if (property == TreeIDs::moduleColor)
        {
            setChildCompColors();
        }
    }
}

void KrumModuleEditor::buildModule()
{
    juce::String i = moduleTree.getProperty(TreeIDs::moduleSamplerIndex).toString();

    //    int dragHandleSize;
    //    auto dragHandleData = BinaryData::getNamedResource("drag_handleblack18dp_svg", dragHandleSize);
    //    auto dragHandelIm = juce::Drawable::createFromImageData(dragHandleData, dragHandleSize);

    //    dragHandle.reset(new DragHandle{ editor.moduleContainer, parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
    //    dragHandle->setImages(dragHandelIm.get());
    //    addAndMakeVisible(dragHandle.get());
    //    dragHandle->setTooltip("Future Kris will make this drag and drop to re-arrange modules");
    //
    addAndMakeVisible(thumbnail);
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModuleClipGain + i, thumbnail.clipGainSlider));

    addAndMakeVisible(titleBox);
    titleBox.setText(moduleTree.getProperty(TreeIDs::moduleName).toString(), juce::NotificationType::dontSendNotification);
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
    volumeSlider.onDragEnd = [this] {   printValueAndPositionOfSlider(); };

    volumeSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModuleGain + i, volumeSlider));

    addAndMakeVisible(panSlider);
    panSlider.setScrollWheelEnabled(false);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setNumDecimalPlacesToDisplay(2);
    panSlider.setDoubleClickReturnValue(true, 1.0f);
    panSlider.setPopupDisplayEnabled(true, false, this);
    panSlider.setTooltip(panSlider.getTextFromValue(panSlider.getValue()));

    panSlider.onValueChange = [this] { updateBubbleComp(&panSlider, panSlider.getCurrentPopupDisplay()); };

    panSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModulePan + i, panSlider));

    addAndMakeVisible(midiLabel);


    int playButtonImSize;
    auto playButtonData = BinaryData::getNamedResource("play_arrowblack18dp_svg", playButtonImSize);
    auto playButtonImage = juce::Drawable::createFromImageData(playButtonData, playButtonImSize);

    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());
    playButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    playButton.onMouseDown = [this]( const juce::MouseEvent& e) { triggerNoteOnInParent(e); };
    playButton.onMouseUp = [this](const juce::MouseEvent& e) { triggerNoteOffInParent(e); };
    
    
    int editButtonImSize;
    auto editButtonData = BinaryData::getNamedResource("settingsblack18dp_svg", editButtonImSize);
    auto editButtonImage = juce::Drawable::createFromImageData(editButtonData, editButtonImSize);
    
    addAndMakeVisible(editButton);
    editButton.setImages(editButtonImage.get());
    editButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    editButton.onClick = [this] { showSettingsOverlay(true); };
    //editButton.onClick = [this] { showSettingsMenu(); };

    setAndDrawThumbnail();
    setChildCompColors();

    needsToBuildModuleEditor = false;

    resized();
    repaint();
}


//lots of colors to change
void KrumModuleEditor::setChildCompColors()
{
    auto moduleColor = getModuleColor().withAlpha(0.7f);

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

    //titleBox.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
    //titleBox.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::textColourId, moduleColor.contrasting());

    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, moduleColor.darker(0.9f));
    
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker().withAlpha(0.7f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

}


void KrumModuleEditor::showSettingsMenu()
{
    juce::PopupMenu settingsMenu;
    juce::PopupMenu::Options options;

    //settingsMenu.addItem(KrumModule::ModuleSettingIDs::moduleMidiNote_Id, "Change Midi");
    //settingsMenu.addItem(KrumModule::ModuleSettingIDs::moduleColor_Id, "Change Color");
    settingsMenu.addItem(KrumModule::ModuleSettingIDs::moduleReConfig_Id, "Settings");
    settingsMenu.addItem(KrumModule::moduleDelete_Id, "Delete Module");

    settingsMenu.showMenuAsync(options.withTargetScreenArea(editButton.getScreenBounds()), new ModalManager([this](int choice)
        {
            handleSettingsMenuResult(choice);
        }));
    
}


void KrumModuleEditor::setModuleSelected(bool isModuleSelected)
{
    if (settingsOverlay != nullptr)
    {
        settingsOverlay->setOverlaySelected(isModuleSelected);
    }
}

void KrumModuleEditor::removeSettingsOverlay(bool keepSettings)
{
    setModuleButtonsClickState(true);
    settingsOverlay->setVisible(false);

    if (needsToBuildModuleEditor)
    {
        setModuleState(KrumModule::ModuleState::active);
    }
    else
    {
        showModule();
    }
}


void KrumModuleEditor::showSettingsOverlay(bool selectOverlay)
{
    hideModule();
    setModuleButtonsClickState(false);

    settingsOverlay->setVisible(true);
    settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
    settingsOverlay->keepCurrentColor(true);
    
    if (selectOverlay)
    {
        //we have the container control selecting so it can clear the other selected modules
        editor.getModuleContainer().setModuleSelected(this);
        editor.keyboard.scrollToKey(getModuleMidiNote());
        repaint();
    }
}

void KrumModuleEditor::setModuleButtonsClickState(bool isClickable)
{
    int numChildren = getNumChildComponents();

    for (int i = 0; i < numChildren; i++)
    {
        getChildComponent(i)->setInterceptsMouseClicks(isClickable, isClickable);
    }
}

void KrumModuleEditor::hideModule()
{
    //set child components to not visible
    titleBox.setVisible(false);
    midiLabel.setVisible(false);
    panSlider.setVisible(false);
    volumeSlider.setVisible(false);
    playButton.setVisible(false);
    editButton.setVisible(false);
    thumbnail.setVisible(false);
}

void KrumModuleEditor::showModule()
{
    //set child components to visible
    titleBox.setVisible(true);
    midiLabel.setVisible(true);
    panSlider.setVisible(true);
    volumeSlider.setVisible(true);
    playButton.setVisible(true);
    editButton.setVisible(true);
    thumbnail.setVisible(true);
}

int KrumModuleEditor::getModuleState()
{
    return moduleTree.getProperty(TreeIDs::moduleState);
}

int KrumModuleEditor::getModuleSamplerIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleSamplerIndex);
}


void KrumModuleEditor::setModuleState(int newState)
{
    moduleTree.setProperty(TreeIDs::moduleState, newState, nullptr);
}

int KrumModuleEditor::getModuleDisplayIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleDisplayIndex);
}

void KrumModuleEditor::setModuleDisplayIndex(int newDisplayIndex)
{
    moduleTree.setProperty(TreeIDs::moduleDisplayIndex, newDisplayIndex, nullptr);
}

void KrumModuleEditor::setModuleName(juce::String& newName)
{
    moduleTree.setProperty(TreeIDs::moduleName, newName, nullptr);
    titleBox.setText(newName, juce::dontSendNotification);
}

juce::Colour KrumModuleEditor::getModuleColor()
{
    return  juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor).toString());
}

void KrumModuleEditor::setModuleColor(juce::Colour newColor)
{
    moduleTree.setProperty(TreeIDs::moduleColor, newColor.toDisplayString(true), nullptr);
}

int KrumModuleEditor::getModuleMidiNote()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiNote);
}

juce::String KrumModuleEditor::getModuleMidiNoteString(bool noteName)
{
    int noteNum = getModuleMidiNote();
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
    moduleTree.setProperty(TreeIDs::moduleMidiNote, newMidiNote, nullptr);
}

int KrumModuleEditor::getModuleMidiChannel()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiChannel);
}

void KrumModuleEditor::setModuleMidiChannel(int newMidiChannel)
{
    moduleTree.setProperty(TreeIDs::moduleMidiChannel, newMidiChannel, nullptr);
}

void KrumModuleEditor::setModulePlaying(bool isPlaying)
{
    modulePlaying = isPlaying;
}

bool KrumModuleEditor::isModulePlaying()
{
    return modulePlaying;
}

void KrumModuleEditor::updateName()
{
    moduleTree.setProperty(TreeIDs::moduleName, titleBox.getText(), nullptr);
}

juce::String KrumModuleEditor::getModuleName()
{
    return moduleTree.getProperty(TreeIDs::moduleName).toString();
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

//the editor should only want midi if it's being assigned
bool KrumModuleEditor::doesEditorWantMidi()
{
    //return settingsOverlay != nullptr;
    return settingsOverlay->isVisible() && settingsOverlay->isOverlaySelected();
}

void KrumModuleEditor::handleMidi(int midiChannel, int midiNote)
{
    if (settingsOverlay != nullptr && settingsOverlay->isVisible())
    {
        settingsOverlay->handleMidiInput(midiChannel, midiNote);
    }
}

void KrumModuleEditor::triggerNoteOnInParent(const juce::MouseEvent& e)
{
    //setModulePlaying(true);
    editor.keyboard.mouseDownOnKey(getModuleMidiNote(), e);
    //editor.sampler.noteOn(getModuleMidiChannel(), getModuleMidiNote(), buttonClickVelocity);
}

void KrumModuleEditor::triggerNoteOffInParent(const juce::MouseEvent& e)
{
    //setModulePlaying(false);
    editor.keyboard.mouseUpOnKey(getModuleMidiNote(), e);
    //editor.sampler.noteOff(getModuleMidiChannel(), getModuleMidiNote(), buttonClickVelocity, true);
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
    //using for drag and drop thumbnail
    //if (shouldCheckDroppedFile())
    //{
    //    handleLastDroppedFile();
    //}

    juce::File file{ moduleTree.getProperty(TreeIDs::moduleFile) };
    thumbnail.setSource (new juce::FileInputSource(file));
    auto newFileName = file.getFileName();

    setModuleName(newFileName);
    
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
           settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
           settingsOverlay->keepCurrentColor(true);
           showSettingsOverlay(true);
       }
       else if (result == KrumModule::moduleDelete_Id)
       {
           moduleTree.setProperty(TreeIDs::moduleState, KrumModule::ModuleState::empty, nullptr);
       }
       
       
       //else if (result == KrumModule::ModuleSettingIDs::moduleMidiNote_Id)
       //{
       //    //settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
       //    settingsOverlay->setToOnlyShowColors(false);
       //    settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
       //    settingsOverlay->keepCurrentColor(true);
       //    showSettingsOverlay(true);
       //}
       //else if (result == KrumModule::ModuleSettingIDs::moduleColor_Id)
       //{
       //   //settingsOverlay.reset(new ModuleSettingsOverlay(localBounds, parent));
       //    settingsOverlay->setToOnlyShowColors(true);
       //    settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
       //    showSettingsOverlay(true);
       //}
    
}

//Drag and Drop Target
bool KrumModuleEditor::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    if(shouldModuleAcceptFileDrop())
    {
        auto desc = dragDetails.description.toString();
        return desc.isNotEmpty() && (desc.contains("FileBrowserDrag-") || desc.contains("ModuleDrag-"));
    }
    return false;
}

//Drag and Drop Target
void KrumModuleEditor::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails)
{
    auto desc = dragDetails.description.toString();
    bool addNextModule = false; //set flag true if files are accepted by module, otherwise leave false
    auto& sampler = editor.sampler;

    if (desc.contains("FileBrowserDrag-"))
    {
        int numDroppedItems = editor.fileBrowser.getNumSelectedItems();
        if (numDroppedItems > 1)
        {
            if (numDroppedItems <= editor.moduleContainer.getNumEmptyModules()) //checks to make sure we have enough modules
            {
                for (int i = 0; i < numDroppedItems; i++)
                {
                    auto krumItem = editor.fileBrowser.getSelectedItem(i);
                    if (krumItem != nullptr)
                    {
                        auto file = krumItem->getFile();
                        auto itemName = krumItem->getItemName();
                        if (!krumItem->mightContainSubItems() && sampler.isFileAcceptable(file))
                        {
                            if (i == 0)
                            {
                                //we set this module with the first dropped file and then create the rest
                                updateModuleFile(editor.fileBrowser.getSelectedItem(0)->getFile());
                                setModuleState(KrumModule::ModuleState::hasFile);
                                settingsOverlay->setMidiListen(true);
                                addFileToRecentsFolder(file, itemName);
                                addNextModule = true;
                                continue;
                            }
                            
                            auto modulesTree = moduleTree.getParent();
                            if (modulesTree.hasType(TreeIDs::KRUMMODULES))
                            {

                                for (int j = 0; j < modulesTree.getNumChildren(); j++)
                                {
                                    auto itTree = modulesTree.getChild(j);
                                    if ((int)itTree.getProperty(TreeIDs::moduleState) == KrumModule::ModuleState::empty)
                                    {
                                        itTree.setProperty(TreeIDs::moduleFile, file.getFullPathName(), nullptr);
                                        itTree.setProperty(TreeIDs::moduleName, itemName, nullptr);
                                        itTree.setProperty(TreeIDs::moduleState, KrumModule::ModuleState::hasFile, nullptr);

                                        editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()/*, KrumModule::ModuleState::hasFile*/));
                                        addFileToRecentsFolder(file, itemName);
                                        addNextModule = true;
                                
                                        DBG("-------");
                                        DBG("Module Sampler Index: " + itTree.getProperty(TreeIDs::moduleSamplerIndex).toString());
                                        DBG("Item: " + file.getFullPathName());                            
                                        
                                        break;
                                    }
                                }
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
        else // only 1 file was dropped
        {
            auto krumItem = editor.fileBrowser.getSelectedItem(0);
            if (krumItem != nullptr)
            {
                auto file = krumItem->getFile();
                auto itemName = krumItem->getItemName();
                if (!krumItem->mightContainSubItems())
                {
                    if (editor.sampler.isFileAcceptable(file))
                    {
                        DBG("Item: " + file.getFullPathName());
                        updateModuleFile(file);
                        setModuleState(KrumModule::ModuleState::hasFile);
                        settingsOverlay->setMidiListen(true);
                        addFileToRecentsFolder(file, itemName);
                        addNextModule = true;
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
    if (addNextModule)
    {
        editor.addNextModuleEditor();
    }
}

void KrumModuleEditor::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    //
}

//EXTERNAL File Drag and Drop Target
bool KrumModuleEditor::isInterestedInFileDrag(const juce::StringArray &files)
{
    //TODO: check if file format is supported?
    return shouldModuleAcceptFileDrop();
}

//EXTERNAL File Drag and Drop Target
void KrumModuleEditor::filesDropped(const juce::StringArray &files, int x, int y)
{
    bool addNextModule = false; //set flag true if files are accepted by module, otherwise leave false

    auto& sampler = editor.sampler;

    if(files.size() > 1)
    {
        int numFreeModules, firstFreeIndex;
        sampler.getNumFreeModules(numFreeModules, firstFreeIndex);
        
        if(files.size() <= numFreeModules)
        {
            for (auto file : files)
            {
                juce::File audioFile {file};
                if(sampler.isFileAcceptable(audioFile))
                {
                    auto modulesTree = moduleTree.getParent();
                    if (modulesTree.hasType(TreeIDs::KRUMMODULES))
                    {
                        for (int j = 0; j < modulesTree.getNumChildren(); j++)
                        {
                            auto itTree = modulesTree.getChild(j);
                            if ((int)itTree.getProperty(TreeIDs::moduleState) == 0) //we grab the first empty module
                            {
                                itTree.setProperty(TreeIDs::moduleFile, audioFile.getFullPathName(), nullptr);
                                itTree.setProperty(TreeIDs::moduleState, KrumModule::ModuleState::hasFile, nullptr);
                                
                                editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));

                                addFileToRecentsFolder(audioFile, audioFile.getFileName());
                                
                                addNextModule = true;

                                DBG("-------");
                                DBG("Module Sampler Index: " + itTree.getProperty(TreeIDs::moduleSamplerIndex).toString());
                                DBG("Item: " + audioFile.getFullPathName());

                                break;
                            }
                        }
                    }
                }
                else
                {
                    DBG("External File Not Acceptable");
                }
            }
        }
    }
    else // only dropped one file
    {
        juce::File audioFile{ files[0] };
        if (sampler.isFileAcceptable(audioFile))
        {
            updateModuleFile(audioFile);
            setModuleState(KrumModule::ModuleState::hasFile);
            settingsOverlay->setMidiListen(true);
            addFileToRecentsFolder(audioFile, audioFile.getFileName());
            addNextModule = true;
            DBG("File: " + audioFile.getFullPathName());
        }
        else
        {
            DBG("Audio Format Not Supported");
        }
        
    }
    //editor.moduleContainer.showFirstEmptyModule();
    if (addNextModule)
    {
        editor.addNextModuleEditor();
    }
}

void KrumModuleEditor::updateModuleFile(juce::File& file)
{
    juce::String filePath = file.getFullPathName();
    moduleTree.setProperty(TreeIDs::moduleFile, filePath, nullptr);
    drawThumbnail = true;
}

bool KrumModuleEditor::shouldModuleAcceptFileDrop()
{
    int state = getModuleState();
    return state == 0;
}

void KrumModuleEditor::addFileToRecentsFolder(juce::File& file, juce::String name)
{
    editor.fileBrowser.addFileToRecent(file, name);
}

void KrumModuleEditor::zeroModuleTree()
{
    moduleTree.setProperty(TreeIDs::moduleName, juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleFile, juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiNote, juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiChannel, juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleColor, juce::var(""), nullptr);

    DBG("Module " + juce::String(getModuleSamplerIndex()) + " zeroed");
}

void KrumModuleEditor::timerCallback()
{
    if (drawThumbnail)
    {
        setAndDrawThumbnail();
    }

    auto& keyboard = editor.keyboard;
    int currentMidiNote = getModuleMidiNote();

    if (getLocalBounds().contains(getMouseXYRelative()))
    {
        keyboard.setHighlightKey(currentMidiNote, true);
    }
    else if (keyboard.isKeyHighlighted(currentMidiNote))
    {
        keyboard.setHighlightKey(currentMidiNote, false);
    }
}

bool KrumModuleEditor::isMouseOverAnyChildren()
{
    return (titleBox.isMouseOver(true)      ||
            midiLabel.isMouseOver(true)     ||
            panSlider.isMouseOver(true)     ||
            volumeSlider.isMouseOver(true)  ||
            playButton.isMouseOver(true)    ||
            editButton.isMouseOver(true)    ||
            thumbnail.isMouseOver(true));
    
}

void KrumModuleEditor::printValueAndPositionOfSlider()
{
    
    auto val = volumeSlider.getValue();
    auto pos = volumeSlider.getPositionOfValue(val);
    DBG("Slider Pos: " + juce::String(pos) + " value: " + juce::String(val));
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
        onMouseDown(e);
    }

}

void KrumModuleEditor::OneShotButton::mouseUp(const juce::MouseEvent& e)
{
    Button::mouseUp(e);
    
    if (onMouseUp)
    {
        onMouseUp(e);
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


//void KrumModuleEditor::setModuleIndex(int newIndex)
//{
//    parent.setModuleSamplerIndex(newIndex);
//}

//void KrumModuleEditor::setOldMidiNote(int midiNote)
//{
//    oldMidiNote = midiNote;
//}

//called when the index of the module has changed so now we need to change the slider attachment assignments as well. Might need to approach this differently for cases of automation within the DAW
//void KrumModuleEditor::reassignSliderAttachments()
//{
//    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + parent.getIndexString(), volumeSlider));
//    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + parent.getIndexString(), panSlider));
//    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleClipGain_ID + parent.getIndexString(), thumbnail.clipGainSlider));
//}

//void KrumModuleEditor::handleOverlayData(bool keepSettings)
//{
//    if (needsToBuildModuleEditor)
//    {
//        buildModule();
//    }
//
//    //int midiNote = moduleTree.getProperty(TreeIDs::moduleMidiNote);
//    //auto moduleColor = juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor).toString());
//
//    //if (keepSettings)
//    //{
//    //    if (oldMidiNote > 0) //removes the old color assignment on the keyboard
//    //    {
//    //        editor.setKeyboardNoteColor(midiNote, moduleColor, oldMidiNote);
//    //    }
//    //    else
//    //    {
//    //        editor.setKeyboardNoteColor(midiNote, moduleColor);
//    //    }
//    //    
//    //    //should the sampler by a valueTree Listener?? or the module?? 
//    //    editor.sampler.updateModuleSample(&parent); //update midi assignment in Sound
//    //    setChildCompColors();
//    //    setAndDrawThumbnail();
//    //    needsToBuildModuleEditor = true;
//    //    parent.setModuleState(KrumModule::ModuleState::active);
//    //}
//
//    //resized();
//    //repaint();
//    //DBG("Module State: " + juce::String(parent.info.moduleState));
//}


//void KrumModuleEditor::forceMouseUp()
//{
    //parent.info.moduleDragging = false;
//    editor.moduleContainer.endModuleDrag(nullptr);
//    forcedMouseUp = true;
//}

//void KrumModuleEditor::setKeyboardColor()
//{
//    setChildCompColors();
//    //int midiNote = parent.getMidiTriggerNote();
//
//   /* if (editor.keyboard.isMidiNoteAssigned(midiNote))
//    {
//        editor.keyboard.removeMidiNoteColorAssignment(midiNote);
//    }*/
//
//    editor.setKeyboardNoteColor(getModuleMidiNote(), getModuleColor());
//}


//void KrumModuleEditor::removeFromDisplay()
//{
//    //editor.removeKeyboardListener(&parent);
//    editor.keyboard.removeMidiNoteColorAssignment(getModuleMidiNote());
//    editor.getModuleContainer().removeModuleEditor(this);
//}



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