/*
  ==============================================================================

    KrumModuleEditor.cpp
    Created: 30 Apr 2021 10:21:42am
    Author:  krisc

  ==============================================================================
*/


#include "KrumModuleEditor.h"
#include "KrumModule.h"
#include "PluginEditor.h"
#include "KrumFileBrowser.h"
#include "InfoPanel.h"


//===============================================================================================//

KrumModuleEditor::KrumModuleEditor(juce::ValueTree& modTree, KrumSamplerAudioProcessorEditor& e, juce::AudioFormatManager& fm/*, int state*/)
    : moduleTree(modTree), editor(e), timeHandle(*this),
    thumbnail(*this, THUMBNAIL_RES, fm, e.getThumbnailCache())
{
    setPaintingIsUnclipped(true);

    //We make a settings overlay in the ctor and keep it for the entire life of the module editor so we don't have to heap allocate when changing settings
    settingsOverlay.reset(new ModuleSettingsOverlay(*this));
    addChildComponent(settingsOverlay.get());

    moduleTree.addListener(this);

    startTimerHz(30);
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);

    int state = getModuleState();

    valueTreePropertyChanged(moduleTree, TreeIDs::moduleState);

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

    juce::Colour c = getModuleColor().darker(0.4f).withAlpha(0.8f);

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
        juce::Colour bc = c;
        if (isModuleMuted())
        {
            bc = c.withAlpha(0.2f);
        }

        if (modulePlaying)
        {
            bc = bc.brighter();
        }

        //auto bgGrade = juce::ColourGradient::vertical(bc, (float)area.getY(), juce::Colours::black, area.getBottom());
        auto bgGrade = juce::ColourGradient::vertical(bc, (float)area.getY(), c.darker(0.55f), area.getBottom());
        g.setGradientFill(bgGrade);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);
        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };

        g.setColour(c);

        //g.setFont(11.0f);
        //g.drawFittedText("L", leftLabelRect, juce::Justification::centred, 1);
        //g.drawFittedText("R", rightLabelRect, juce::Justification::centred, 1);


        auto sliderBounds = volumeSlider.getBoundsInParent().toFloat();
        auto sliderLineBounds = sliderBounds.withTrimmedTop(10);
        //paintVolumeSliderLines(g, sliderBounds);
        
        auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
        //paintPanSliderLines(g, panSliderBounds);
        
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
            //this draws on top of the zeroLine, which we've already drawn
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
    juce::Line<float> midLine{ {bounds.getCentreX() - 2, bounds.getY() - 2},{bounds.getCentreX() - 2 , bounds.getCentreY() } };
    g.drawLine(midLine, 1.5f);

}

void KrumModuleEditor::resized()
{
    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);

    int titleHeight = area.getHeight() * 0.075f;

    int spacer = 5;
    int thumbnailH = area.getHeight() * 0.25f;
    int timeHandleH = 13;

    //int midiLabelH = area.getHeight() * 0.093f;
    int midiLabelH = area.getHeight() * 0.08f;

    int panSliderH = area.getHeight() * 0.05f;
    int panSliderW = area.getWidth() * 0.9f;

    int volumeSliderH = area.getHeight() * 0.5f;
    int volumeSliderW = area.getWidth() * 0.4;

    int buttonH = area.getHeight() * 0.085f;
    int buttonW = area.getWidth() * 0.3f;

    int outputComboH = area.getHeight() * 0.073f;

    settingsOverlay->setBounds(area);

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));
    timeHandle.setBounds(thumbnail.getX(), thumbnail.getBottom(), thumbnail.getWidth(), timeHandleH);


    panSlider.setBounds(area.getX() + spacer, timeHandle.getBottom() + (spacer), panSliderW, panSliderH);
    volumeSlider.setBounds(area.getX() + spacer/*area.getCentreX() - (volumeSliderW / 2)*/, panSlider.getBottom() + (spacer/* * 3*/), volumeSliderW, volumeSliderH);
    
    playButton.setBounds(area.withTop(panSlider.getBottom()).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));
    editButton.setBounds(area.withTop(playButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW +spacer)).withWidth(buttonW));
    muteButton.setBounds(area.withTop(editButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));
    reverseButton.setBounds(area.withTop(muteButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));
    pitchButton.setBounds(area.withTop(reverseButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));

    midiLabel.setBounds(area.withTrimmedTop(volumeSlider.getBottom()).withHeight(midiLabelH));
    outputCombo.setBounds(area.withTop(midiLabel.getBottom() - spacer).withHeight(outputComboH).reduced(spacer * 1.75, spacer));


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

    juce::Component::mouseEnter(event);
}

void KrumModuleEditor::mouseExit(const juce::MouseEvent& event)
{
    KrumModule::ModuleState moduleState = static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState));

    if (moduleState == KrumModule::ModuleState::empty || settingsOverlay->isVisible())
    {
        InfoPanel::shared_instance().clearPanelText();
    }

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
    }

    juce::Component::mouseDown(e);
}

//===============================================================================================================
void KrumModuleEditor::valueTreePropertyChanged(juce::ValueTree& treeWhoChanged, const juce::Identifier& property)
{
    if(treeWhoChanged == moduleTree)
    {
        if (property == TreeIDs::moduleState)
        {
            int state = treeWhoChanged[property];
            if (state == KrumModule::ModuleState::hasFile)
            {
                showNewSettingsOverlay();
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
//===============================================================================================================

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
    addAndMakeVisible(titleBox);
    titleBox.setText(moduleTree.getProperty(TreeIDs::moduleName).toString(), juce::NotificationType::dontSendNotification);
    titleBox.setFont({ 17.0f });
    titleBox.setColour(juce::Label::ColourIds::textColourId, titleFontColor);
    titleBox.setColour(juce::Label::ColourIds::textWhenEditingColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::highlightColourId, juce::Colours::lightgrey);
    titleBox.setColour(juce::CaretComponent::ColourIds::caretColourId, juce::Colours::black);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setEditable(false, true, false);
    titleBox.setTooltip("double-click to change name");
    
    titleBox.onTextChange = [this]
    {
        juce::String newName = titleBox.getText(true); //compiler reasons
        setModuleName(newName);
    };
   
    addAndMakeVisible(thumbnail);
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModuleClipGain + i, thumbnail.clipGainSlider));

    addAndMakeVisible(timeHandle);
    
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
    //auto playButtonData = BinaryData::getNamedResource("play_arrowblack18dp_svg", playButtonImSize);
    auto playButtonData = BinaryData::getNamedResource("nounplaywhite_svg", playButtonImSize);
    auto playButtonImage = juce::Drawable::createFromImageData(playButtonData, playButtonImSize);

    //DBG("Play Icon: " + juce::String(playButtonImage ? "valid" : "null"));

    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());
    playButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    playButton.onMouseDown = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseDown(e); };
    playButton.onMouseUp = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseUp(e); };
    
    
    addAndMakeVisible(pitchButton);
    auto& pitchSlider = pitchButton.getSlider();
    pitchSlider.setScrollWheelEnabled(false);
    pitchSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    pitchSlider.setDoubleClickReturnValue(true, 0.0);
    pitchSlider.setTooltip(pitchSlider.getTextFromValue(pitchSlider.getValue()));

    //pitchSlider.onDragStart = [this] { pitchSlider.setShowSlider(true); };
    //pitchSlider.onDragEnd = [this] { pitchSlider.setShowSlider(false); };

    pitchSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModulePitchShift + i, pitchSlider));

    addAndMakeVisible(reverseButton);
    reverseButton.setButtonText("REV");
    reverseButton.setToggleState(getModuleReverseState(), juce::dontSendNotification);
    reverseButton.setClickingTogglesState(true);
    reverseButtonAttachment.reset(new ButtonAttachment(editor.parameters, TreeIDs::paramModuleReverse + i, reverseButton));

    addAndMakeVisible(muteButton);
    muteButton.setButtonText("MUTE");
    muteButton.setToggleState(isModuleMuted(), juce::dontSendNotification);
    muteButton.setClickingTogglesState(true);
    muteButtonAttachment.reset(new ButtonAttachment(editor.parameters, TreeIDs::paramModuleMute + i, muteButton));

    //int editButtonImSize;
    //auto editButtonData = BinaryData::getNamedResource("settingsblack18dp_svg", editButtonImSize);
    //auto editButtonData = BinaryData::getNamedResource("nounmenuwhite_png", editButtonImSize);
    //auto editButtonImage = juce::Drawable::createFromImageData(editButtonData, editButtonImSize);
    auto editButtonImage = juce::Drawable::createFromImageData(BinaryData::nounmenuwhite_svg, BinaryData::nounmenuwhite_svgSize);
    
    addAndMakeVisible(editButton);
    editButton.setImages(editButtonImage.get());
    editButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    editButton.onClick = [this] { showSettingsOverlay(true, true); };

    addAndMakeVisible(outputCombo);
    outputCombo.addItemList(TreeIDs::outputStrings, 1);
    outputComboAttachment.reset(new ComboBoxAttachment(editor.parameters, TreeIDs::paramModuleOutputChannel + i, outputCombo));

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

    //playButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
    //playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));
     
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    //editButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
    //editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));
    
    editButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkred);

    reverseButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    reverseButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkblue);

    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, moduleColor.darker(0.6f).withAlpha(0.25f));
    titleBox.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, moduleColor.darker(0.7f));
    titleBox.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, moduleColor.darker());
    titleBox.setColour(juce::TextEditor::ColourIds::highlightedTextColourId, moduleColor.contrasting());

    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker().withAlpha(0.7f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

    //outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, moduleColor.darker(0.55f).withAlpha(0.5f));
    outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::black);
    outputCombo.setColour(juce::ComboBox::ColourIds::textColourId, moduleColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::arrowColourId, moduleColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.8f));

    //outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));
    outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, juce::Colours::black);
    outputCombo.setColour(juce::PopupMenu::ColourIds::textColourId, moduleColor.darker());
    //outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, moduleColor.darker());
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, juce::Colours::black);
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, moduleColor);

    midiLabel.textColor = moduleColor;
    
    //getLookAndFeel().setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));

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
    showModule();
}


void KrumModuleEditor::showNewSettingsOverlay()
{
    showSettingsOverlay(false, true);
}

void KrumModuleEditor::showSettingsOverlay(bool keepCurrentColorOnExit, bool selectOverlay)
{
    hideModule();
    setModuleButtonsClickState(false);

    settingsOverlay->setVisible(true);
    settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
    settingsOverlay->keepCurrentColor(keepCurrentColorOnExit);
    juce::String name = getModuleName(); //compiler reasons
    settingsOverlay->setTitle(name); 
    
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
    for (int i = 0; i < getNumChildComponents(); i++)
    {
        auto child = getChildComponent(i);
        if (child != settingsOverlay.get())
        {
            child->setVisible(false);
        }
    }
}

void KrumModuleEditor::showModule()
{
    for (int i = 0; i < getNumChildComponents(); i++)
    {
        auto child = getChildComponent(i);
        if (child != settingsOverlay.get())
        {
            child->setVisible(true);
        }
    }
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
    settingsOverlay->setTitle(newName);
}

juce::String KrumModuleEditor::getModuleName()
{
    return moduleTree.getProperty(TreeIDs::moduleName).toString();
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

bool KrumModuleEditor::isModuleMuted()
{
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModuleMute + juce::String(getModuleSamplerIndex()));
    return *val > 0.5;
}

bool KrumModuleEditor::getModuleReverseState()
{
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModuleReverse + juce::String(getModuleSamplerIndex()));
    return *val > 0.5;
}

void KrumModuleEditor::setNumSamplesOfFile(int numSamplesInFile)
{
    moduleTree.setProperty(TreeIDs::moduleNumSamplesLength, numSamplesInFile, nullptr);
}

int KrumModuleEditor::getNumSamplesInFile()
{
    return moduleTree.getProperty(TreeIDs::moduleNumSamplesLength);
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

void KrumModuleEditor::setTimeHandles()
{
    timeHandle.setHandles(0, getNumSamplesInFile());
}

//the editor should only want midi if it's being assigned
bool KrumModuleEditor::doesEditorWantMidi()
{
    //need to check for MidiListen toggle state here?
    return settingsOverlay->isVisible() && settingsOverlay->isOverlaySelected();
}

void KrumModuleEditor::handleMidi(int midiChannel, int midiNote)
{
    if (settingsOverlay != nullptr && settingsOverlay->isVisible())
    {
        settingsOverlay->handleMidiInput(midiChannel, midiNote);
    }
}

void KrumModuleEditor::triggerMouseDownOnNote(const juce::MouseEvent& e)
{
    int note = getModuleMidiNote();
    editor.keyboard.mouseDownOnKey(note, e);
    editor.sampler.noteOn(getModuleMidiChannel(), note, buttonClickVelocity);
}

void KrumModuleEditor::triggerMouseUpOnNote(const juce::MouseEvent& e)
{
    int note = getModuleMidiNote();
    editor.keyboard.mouseUpOnKey(note, e);
    editor.sampler.noteOff(getModuleMidiChannel(), note, buttonClickVelocity, true);
}

bool KrumModuleEditor::needsToDrawThumbnail()
{
    return drawThumbnail;
}

void KrumModuleEditor::setAndDrawThumbnail()
{
    juce::File file{ moduleTree.getProperty(TreeIDs::moduleFile) };
    thumbnail.setSource (new juce::FileInputSource(file));
    
    //if this is new file, or being reloaded from the tree
    if (timeHandle.getEndPosition() == 0)
    {
        setTimeHandles();
    }

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

void KrumModuleEditor::setPitchSliderVisibility(bool sliderShouldBeVisible)
{
    pitchButton.getSlider().setVisible(sliderShouldBeVisible);
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
        if (numDroppedItems <= editor.moduleContainer.getNumEmptyModules()) //checks to make sure we have enough modules
        {
            for (int i = 0; i < numDroppedItems; i++)
            {
                auto krumItem = editor.fileBrowser.getSelectedItem(i);
                if (krumItem != nullptr)
                {
                    auto file = krumItem->getFile();
                    auto itemName = krumItem->getItemName();
                    juce::int64 numSamples = 0;
                    if (!krumItem->mightContainSubItems() && sampler.isFileAcceptable(file, numSamples))
                    {
                        if (i == 0)
                        {
                            //we set this module with the first dropped file and then create the rest, if we need to
                            handleNewFile(file, numDroppedItems == 1, numSamples);

                            addNextModule = true;
                            continue;
                        }

                        //we create the new modules from the value tree
                        auto modulesTree = moduleTree.getParent();
                        if (modulesTree.hasType(TreeIDs::KRUMMODULES))
                        {
                            for (int j = 0; j < modulesTree.getNumChildren(); j++)
                            {
                                auto itTree = modulesTree.getChild(j);
                                if ((int)itTree.getProperty(TreeIDs::moduleState) == KrumModule::ModuleState::empty)
                                {
                                    auto newModEd = editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));
                                    newModEd->handleNewFile(file, false, numSamples);
                                        
                                    addNextModule = true;
                                        
                                    DBG("-------");
                                    DBG("Module Sampler Index: " + itTree.getProperty(TreeIDs::moduleSamplerIndex).toString());
                                    DBG("Item: " + file.getFullPathName());                            
                                        
                                    break; //ends the loop of module trees
                                }
                            }
                        }
                    }
                    else
                    {
                        DBG("Folders Not Supported");
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
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
    return shouldModuleAcceptFileDrop();
}

//EXTERNAL File Drag and Drop Target
void KrumModuleEditor::filesDropped(const juce::StringArray &files, int x, int y)
{
    bool addNextModule = false; //set flag true if files are accepted by module, otherwise leave false
    auto& sampler = editor.sampler;
    int numFilesDropped = files.size();

    if(numFilesDropped <= editor.moduleContainer.getNumEmptyModules())
    {
        for (int i = 0; i < files.size(); i++)
        {
            juce::File audioFile {files[i]};
            juce::int64 numSamples = 0;
            if(!audioFile.isDirectory() && sampler.isFileAcceptable(audioFile, numSamples))
            {
                if (i == 0)
                {
                    handleNewFile(audioFile, numFilesDropped == 1, numSamples);
                    addNextModule = true;
                    continue;
                }

                auto modulesTree = moduleTree.getParent();
                if (modulesTree.hasType(TreeIDs::KRUMMODULES))
                {
                    for (int j = 0; j < modulesTree.getNumChildren(); j++)
                    {
                        auto itTree = modulesTree.getChild(j);
                        if ((int)itTree.getProperty(TreeIDs::moduleState) == 0) //we grab the first empty module
                        {
                            auto newModEd = editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));
                            newModEd->handleNewFile(audioFile, false, numSamples);
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
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");

            }
        }
    }

    if (addNextModule)
    {
        editor.addNextModuleEditor();
    }
}

void KrumModuleEditor::handleNewFile(juce::File& file, bool overlayShouldListen, int numSamples)
{
    DBG("Item: " + file.getFullPathName());
    juce::String name = file.getFileName(); //compiler reasons
    setModuleName(name);
    setModuleFile(file);
    setNumSamplesOfFile(numSamples);
    setModuleState(KrumModule::ModuleState::hasFile);
    settingsOverlay->setMidiListen(overlayShouldListen);
    addFileToRecentsFolder(file, file.getFileName());
}

void KrumModuleEditor::setModuleFile(juce::File& file)
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

bool KrumModuleEditor::getMouseOver()
{
    return mouseOver;
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
    timeHandle.resetHandles();
    

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

    if (getLocalBounds().contains(getMouseXYRelative()) && getModuleState() > 0) //if mouse is over module and module is active
    {
        mouseOver = true;
        keyboard.setHighlightKey(currentMidiNote, true);
    }
    else
    {
        //module container will use this to clear the highlighted keys
        mouseOver = false;
    }
}

void KrumModuleEditor::printValueAndPositionOfSlider()
{
    
    auto val = volumeSlider.getValue();
    auto pos = volumeSlider.getPositionOfValue(val);
    DBG("Slider Pos: " + juce::String(pos) + " value: " + juce::String(val));
}

void KrumModuleEditor::handleOneShotButtonMouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isShiftDown())
    {
        editor.keyboard.scrollToKey(getModuleMidiNote());
    }

    triggerMouseDownOnNote(e);
}

void KrumModuleEditor::handleOneShotButtonMouseUp(const juce::MouseEvent& e)
{
    triggerMouseUpOnNote(e);
}

//============================================================================================================================

KrumModuleEditor::OneShotButton::OneShotButton(KrumModuleEditor& e)
: InfoPanelDrawableButton("One Shot", "Plays the currently assigned sample", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground), editor(e)
{}

KrumModuleEditor::OneShotButton::~OneShotButton()
{}

void KrumModuleEditor::OneShotButton::paintButton(juce::Graphics & g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{

    auto area = getLocalBounds();

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    auto path = getCurrentImage()->getOutlineAsPath();
    
    auto color = editor.getModuleColor();
    color = shouldDrawButtonAsHighlighted ? color.withAlpha(0.5f) : (shouldDrawButtonAsDown ? color.withAlpha(0.1f) : color.withAlpha(0.8f));

    area.reduce(7, 7);
    path.scaleToFit(area.getX(), area.getY(), area.getWidth(), area.getHeight(), true);
    path.closeSubPath();

    g.setColour(color);
    g.strokePath(path, juce::PathStrokeType(1.0f));

    //g.fillPath(path);
}

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


KrumModuleEditor::MenuButton::MenuButton(KrumModuleEditor& e)
    : InfoPanelDrawableButton("Settings", "Provides a list of actions to change the settings of the module", "", juce::DrawableButton::ButtonStyle::ImageFitted), editor(e)
{}

KrumModuleEditor::MenuButton::~MenuButton()
{}

void KrumModuleEditor::MenuButton::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto area = getLocalBounds();

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    auto path = getCurrentImage()->getOutlineAsPath();

    auto color = isMouseOver() ? editor.getModuleColor().withAlpha(0.7f) : editor.getModuleColor();
    //color = shouldDrawButtonAsHighlighted ? color.withAlpha(0.5f) : (shouldDrawButtonAsDown ? color.withAlpha(0.1f) : color.withAlpha(0.8f));

    g.setColour(color);
    area.reduce(7, 7);
    path.scaleToFit(area.getX(), area.getY(), area.getWidth(), area.getHeight(), true);

    g.strokePath(path, juce::PathStrokeType(1.0f));
}


//============================================================================================================================

KrumModuleEditor::MidiLabel::MidiLabel(KrumModuleEditor* editor)
    : moduleEditor(editor)
{
    setTooltip("Lane: " + juce::String(moduleEditor->getModuleSamplerIndex()));
}

KrumModuleEditor::MidiLabel::~MidiLabel()
{}
 
void KrumModuleEditor::MidiLabel::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    float cornerSize = 3.0f;
    int spacer = 5;


    //g.setColour(juce::Colours::black);
    //g.fillRect(area);
    
    juce::Rectangle<int> midiNoteRect = area.reduced(7);
    juce::Rectangle<int> midiChanRect = area.withX(midiNoteRect.getRight() + spacer).withWidth(area.getWidth() * 0.5f);


    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(midiNoteRect.toFloat(), cornerSize);


    //g.setColour(juce::Colours::black);
    //g.fillRoundedRectangle(midiChanRect.toFloat(), cornerSize);
    
    g.setColour(textColor);
    g.setFont(fontSize);

    g.drawFittedText("Note", midiNoteRect, juce::Justification::centredLeft, 1);
    g.drawFittedText(moduleEditor->getModuleMidiNoteString(true), midiNoteRect, juce::Justification::centredRight, 1);

    //g.drawFittedText("Chan", midiChanRect, juce::Justification::centredLeft, 1);
    //g.drawFittedText(juce::String(moduleEditor->getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);
}

//============================================================================================================================

KrumModuleEditor::PitchButton::PitchButton(KrumModuleEditor& e)
    :InfoPanelButton("Pitch", "Change the pitch at which the sample is played"), editor(e)
{
    //Need to make this bounds of the button as big as the slider, have a the hitTest return for the button bounds

    addChildComponent(&slider);
    slider.setVisible(false);
    //slider.onDragEnd = [this] {slider.setVisible(false); };
}

KrumModuleEditor::PitchButton::~PitchButton()
{
}

void KrumModuleEditor::PitchButton::resized()
{
    auto area = getLocalBounds();

    int expand = 10;

    slider.setBounds(area.withWidth(area.getWidth() + expand).withX(area.getX() + expand).withTrimmedTop(expand));
}

void KrumModuleEditor::PitchButton::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted,
    const bool shouldDrawButtonAsDown)
{
    auto area = getLocalBounds();

    int titleH = 10;
    auto color = isMouseOverOrDragging() ? editor.getModuleColor().withAlpha(0.7f) : editor.getModuleColor();

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    g.setColour(color);
    g.drawFittedText("Pitch", area.withY(3).withBottom(titleH).reduced(3), juce::Justification::centred, 1);

    auto value = slider.getValue();
    auto valueText = slider.getTextFromValue(value);

    /*if (value > 0)
    {
        valueText = "+" + valueText;
    }*/

    g.drawFittedText(valueText, area.withTrimmedTop(titleH + 5), juce::Justification::centred, 1);


}

void KrumModuleEditor::PitchButton::mouseEnter(const juce::MouseEvent& e)
{
    editor.editor.moduleContainer.showModulePitchSlider(&editor);
    InfoPanelButton::mouseEnter(e);
}

void KrumModuleEditor::PitchButton::mouseExit(const juce::MouseEvent& e)
{

    if (isMouseOver(true))
    {
        return;
    }

    slider.setVisible(false);
    InfoPanelButton::mouseExit(e);

}

//void KrumModuleEditor::PitchButton::mouseDown(const juce::MouseEvent& e)
//{
//    //setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
//}
//
//void KrumModuleEditor::PitchButton::mouseDrag(const juce::MouseEvent& e)
//{
//    //setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
//
//    ////set Value off of drag distance
//    //auto mouseDownPos = e.getMouseDownPosition();
//    //auto currentMousePos = e.getPosition();
//
//    //auto yDistance = (mouseDownPos.y - currentMousePos.y)/* * 0.0125f*/;
//    //auto rawValue = getValue() + yDistance;
//    //auto range = getRange();
//
//    ////auto newValue = valueToProportionOfLength(juce::jlimit(range.getStart(), range.getEnd(), rawValue));
//
//    //float newValue = /*getValue() + */(yDistance * 0.00125f); //drag sensitivity
//    //DBG("yDistance: " + juce::String(yDistance) + " || newValue: " + juce::String(newValue));
//
//    //editor.editor.parameters.getParameter(TreeIDs::paramModulePitchShift + juce::String(editor.getModuleSamplerIndex()))->setValueNotifyingHost(newValue);
//    
//}
//
//void KrumModuleEditor::PitchButton::mouseUp(const juce::MouseEvent& e)
//{
//    setMouseCursor(juce::MouseCursor::NormalCursor);
//}

void KrumModuleEditor::PitchButton::setShowSlider(bool shouldShow)
{
    showSlider = shouldShow;
}

bool KrumModuleEditor::PitchButton::isSliderShowing()
{
    return showSlider;
}

juce::Slider& KrumModuleEditor::PitchButton::getSlider()
{
    return slider;
}

float KrumModuleEditor::PitchButton::getSliderPos(double value)
{
    return 0;
    //double pos;

    //juce::NormalisableRange<double> normRange = getRange();

    //if (normRange.end <= normRange.start)
    //    pos = 0.5;
    //else if (value < normRange.start)
    //    pos = 0.0;
    //else if (value > normRange.end)
    //    pos = 1.0;
    //else
    //    pos = valueToProportionOfLength(value);

    //*if (isVertical() || style == IncDecButtons)
    //    pos = 1.0 - pos;*/

    ////jassert(pos >= 0 && pos <= 1.0);
    ////return (float)(sliderRegionStart + pos * sliderRegionSize);
    //auto layout = getLookAndFeel().getSliderLayout(*this);
    //return (float)(layout.sliderBounds.getX() + pos * layout.sliderBounds.getWidth());
}

//=======================================================================================

KrumModuleEditor::CustomToggleButton::CustomToggleButton(juce::String title, juce::String message, KrumModuleEditor& e)
    :InfoPanelTextButton(title, message), editor(e)
{
}

KrumModuleEditor::CustomToggleButton::~CustomToggleButton()
{
}

void KrumModuleEditor::CustomToggleButton::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto area = getLocalBounds();
    int titleH = 10;

    auto bgColor = getToggleState() ? findColour(juce::TextButton::ColourIds::buttonOnColourId) : juce::Colours::black;
    auto textColor = editor.getModuleColor();

    if (isMouseOver())
    {
        bgColor = bgColor.withAlpha(0.7f);
        textColor = textColor.withAlpha(0.7f);
    }

    g.setColour(bgColor);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    g.setColour(textColor);
    g.drawFittedText(getButtonText(), area.withY(3).withBottom(titleH).reduced(3), juce::Justification::centred, 1);

}
