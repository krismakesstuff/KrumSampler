/*
  ==============================================================================

    KrumModuleEditor.cpp
    Created: 30 Apr 2021 10:21:42am
    Author:  krisc

  ==============================================================================
*/


#include "KrumModuleEditor.h"
#include "../Source/KrumModule.h"
#include "../PluginEditor.h"
#include "../FileBrowser/KrumFileBrowser.h"
#include "../InfoPanel.h"


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
    
    valueTreePropertyChanged(moduleTree, TreeIDs::moduleState.getParamID());
        
}

KrumModuleEditor::~KrumModuleEditor()
{
    pitchSliderAttachment.reset();
}

void KrumModuleEditor::paint (juce::Graphics& g)
{
    if (settingsOverlay->isVisible())
    {
        return;
    }

    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);
    juce::Colour c = getModuleColor().darker(0.4f);

    int moduleState = getModuleState();

    if (moduleState == KrumModule::ModuleState::empty)
    {
        g.setColour(isMouseOver() ? juce::Colours::grey : juce::Colours::darkgrey);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
        
        g.setColour(juce::Colours::darkgrey);
        g.drawFittedText("Drop Sample(s) Here", area.reduced(20), juce::Justification::centred, 3);
    }
    else if (moduleState == KrumModule::ModuleState::active) //if the moduleState is hasfile we will be showing the settingsOverlay
    {
        juce::Colour bc = c.withSaturation(0.7f).darker(0.4f);

        if (isModuleMuted())
        {
            //bc = c.withAlpha(0.2f);
            bc = juce::Colours::black;
            c = juce::Colours::black;
        }
        else if (modulePlaying)
        {
            bc = bc.brighter();
        }

        if (mouseOver || mouseOverKey)
        {
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 2.0f);
        }

        //auto bgGrade = juce::ColourGradient::vertical(bc, (float)area.getY(), juce::Colours::black, area.getBottom());
        auto bgGrade = juce::ColourGradient::vertical(bc/*.darker(0.35f)*/, (float)area.getY(), c, area.getBottom());

        auto gain = getModuleGain();
        //auto adjustedGain = juce::jlimit<double>(0.0, 1.0, gain);
        auto gainProp = 1 - normalizeGainValue(gain);
        bgGrade.addColour(juce::jlimit<double>(0.00001,0.9999, gainProp), bc);
        

        g.setGradientFill(bgGrade);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);
        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };

       // g.setColour(Colors::modulesBGColor);

        /*g.setFont(11.0f);
        g.drawFittedText("L", leftLabelRect, juce::Justification::centred, 1);
        g.drawFittedText("R", rightLabelRect, juce::Justification::centred, 1);*/


        //auto sliderBounds = volumeSlider.getBoundsInParent().toFloat();
        //auto sliderLineBounds = sliderBounds.withTrimmedTop(10);
        //paintVolumeSliderLines(g, sliderBounds);
        
        //auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
        //paintPanSliderLines(g, panSliderBounds);
        
        g.setColour(bc);
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
        
    }
}

void KrumModuleEditor::resized()
{
    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);
    int height = area.getHeight();
    int width = area.getWidth();


    int titleHeight = height * 0.10f;

    int spacer = 5;
    int thumbnailH = height * 0.25f;
    int timeHandleH = 13;

    //int midiLabelH = area.getHeight() * 0.093f;
    int midiLabelH = height * 0.07f;

    int panSliderH = height * 0.040f;
    int panSliderW = width * 0.95f;

    int volumeSliderH = height * 0.55f;
    int volumeSliderW = width * 0.57f;

    int buttonH = height * 0.085f;
    int buttonW = width * 0.3f;

    int smallButtonH = height * 0.07f;
    int smallButtonW = width * 0.25f;

    int outputComboH = height * 0.073f;

    settingsOverlay->setBounds(area);

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));
    timeHandle.setBounds(thumbnail.getX(), thumbnail.getBottom(), thumbnail.getWidth(), timeHandleH);

    panSlider.setBounds(area.getX() + 2, timeHandle.getBottom() + (spacer), panSliderW, panSliderH);
    volumeSlider.setBounds(area.getX() + spacer/*area.getCentreX() - (volumeSliderW / 2)*/, panSlider.getBottom() + (spacer/* * 3*/), volumeSliderW, volumeSliderH);
    
    pitchSlider.setBounds(area.withTop(panSlider.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    reverseButton.setBounds(area.withTop(pitchSlider.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    muteButton.setBounds(area.withTop(reverseButton.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    
    playButton.setBounds(area.withTop(muteButton.getBottom() + (spacer * 11)).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));
    editButton.setBounds(area.withTop(playButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW +spacer)).withWidth(buttonW));

    midiLabel.setBounds(area.withTrimmedTop(volumeSlider.getBottom() - spacer).withHeight(midiLabelH).reduced(spacer));
    outputCombo.setBounds(area.withTop(midiLabel.getBottom()).withHeight(outputComboH).reduced(spacer));


//    if (dragHandle != nullptr)
//    {
//        dragHandle->setBounds(area.withTop(editButton.getBottom()));
//    }

}

void KrumModuleEditor::mouseEnter(const juce::MouseEvent& event)
{
    KrumModule::ModuleState moduleState = static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()));
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
    KrumModule::ModuleState moduleState = static_cast<KrumModule::ModuleState>((int)moduleTree.getProperty(TreeIDs::moduleState.getParamID()));

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
        if (property == juce::Identifier(TreeIDs::moduleState.getParamID()))
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
        else if (property == juce::Identifier(TreeIDs::moduleColor.getParamID()))
        {
            setChildCompColors();
        }
    }
}
//===============================================================================================================

void KrumModuleEditor::buildModule()
{
    juce::String i = moduleTree.getProperty(TreeIDs::moduleSamplerIndex.getParamID()).toString();

    //    int dragHandleSize;
    //    auto dragHandleData = BinaryData::getNamedResource("drag_handleblack18dp_svg", dragHandleSize);
    //    auto dragHandelIm = juce::Drawable::createFromImageData(dragHandleData, dragHandleSize);

    //    dragHandle.reset(new DragHandle{ editor.moduleContainer, parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
    //    dragHandle->setImages(dragHandelIm.get());
    //    addAndMakeVisible(dragHandle.get());
    //    dragHandle->setTooltip("Future Kris will make this drag and drop to re-arrange modules");
    //
   

    addAndMakeVisible(titleBox);
    titleBox.setText(moduleTree.getProperty(TreeIDs::moduleName.getParamID()).toString(), juce::NotificationType::dontSendNotification);
    titleBox.setFont(editor.kLaf.getMontBlackTypeface());
    titleBox.setFont({ 16.0f });
    titleBox.setColour(juce::Label::ColourIds::textColourId, titleFontColor);
    titleBox.setColour(juce::Label::ColourIds::textWhenEditingColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::highlightColourId, juce::Colours::lightgrey);
    titleBox.setColour(juce::CaretComponent::ColourIds::caretColourId, juce::Colours::black);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setMinimumHorizontalScale(1.0f);
    titleBox.setEditable(false, true, false);
    titleBox.setTooltip("double-click to change name");
    
    titleBox.onTextChange = [this]
    {
        juce::String newName = titleBox.getText(true); //compiler reasons
        setModuleName(newName);
    };
   
    addAndMakeVisible(thumbnail);
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModuleClipGain.getParamID() + i, thumbnail.clipGainSlider));
    

    addAndMakeVisible(timeHandle);
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setLookAndFeel(&editor.vLaf);
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setNumDecimalPlacesToDisplay(2);
    volumeSlider.setDoubleClickReturnValue(true, 1.0f);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTooltip(volumeSlider.getTextFromValue(volumeSlider.getValue()));
    volumeSlider.onValueChange = [this] { updateBubbleComp(&volumeSlider, volumeSlider.getCurrentPopupDisplay()); };
    volumeSlider.onDragEnd = [this] {   printValueAndPositionOfSlider(); };

    volumeSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModuleGain.getParamID() + i, volumeSlider));

    addAndMakeVisible(panSlider);
    panSlider.setLookAndFeel(&editor.pLaf);
    panSlider.setScrollWheelEnabled(false);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setNumDecimalPlacesToDisplay(2);
    panSlider.setDoubleClickReturnValue(true, 1.0f);
    panSlider.setPopupDisplayEnabled(true, false, this);
    panSlider.setTooltip(panSlider.getTextFromValue(panSlider.getValue()));

    panSlider.onValueChange = [this] { updateBubbleComp(&panSlider, panSlider.getCurrentPopupDisplay()); };

    panSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModulePan.getParamID() + i, panSlider));

    addAndMakeVisible(midiLabel);

    auto playButtonImage = juce::Drawable::createFromImageData(BinaryData::nounplaywhite_svg, BinaryData::nounplaywhite_svgSize);

    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());
    playButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    playButton.onMouseDown = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseDown(e); };
    playButton.onMouseUp = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseUp(e); };
    

    addAndMakeVisible(pitchSlider);
    pitchSlider.setScrollWheelEnabled(false);
    pitchSlider.setTooltip(pitchSlider.getTextFromValue(pitchSlider.getValue()));

    pitchSlider.onValueChange = [this] { pitchSlider.setTooltip(pitchSlider.getTextFromValue(pitchSlider.getValue())); };
    pitchSliderAttachment.reset(new SliderAttachment(editor.parameters, TreeIDs::paramModulePitchShift.getParamID() + i, pitchSlider));

    addAndMakeVisible(reverseButton);
    reverseButton.setButtonText("REV");
    reverseButton.setToggleState(getModuleReverseState(), juce::dontSendNotification);
    reverseButton.setClickingTogglesState(true);
    reverseButtonAttachment.reset(new ButtonAttachment(editor.parameters, TreeIDs::paramModuleReverse.getParamID() + i, reverseButton));

    addAndMakeVisible(muteButton);
    muteButton.setButtonText("MUTE");
    muteButton.setToggleState(isModuleMuted(), juce::dontSendNotification);
    muteButton.setClickingTogglesState(true);
    muteButtonAttachment.reset(new ButtonAttachment(editor.parameters, TreeIDs::paramModuleMute.getParamID() + i, muteButton));

    auto editButtonImage = juce::Drawable::createFromImageData(BinaryData::nounmenuwhite_svg, BinaryData::nounmenuwhite_svgSize);
    
    addAndMakeVisible(editButton);
    editButton.setImages(editButtonImage.get());
    editButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    editButton.onClick = [this] { showSettingsOverlay(true, true); };

    addAndMakeVisible(outputCombo);
    outputCombo.addItemList(TreeIDs::outputStrings, 1);
    outputComboAttachment.reset(new ComboBoxAttachment(editor.parameters, TreeIDs::paramModuleOutputChannel.getParamID() + i, outputCombo));

    setAndDrawThumbnail();
    setChildCompColors();

    needsToBuildModuleEditor = false;

    resized();
    repaint();
}


//lots of colors to change
void KrumModuleEditor::setChildCompColors()
{
    auto moduleColor = getModuleColor()/*.withAlpha(0.5f)*/;

    auto accentColor = moduleColor.brighter(0.05f);
    auto bgColor = Colors::moduleBGColor;

//    dragHandle->setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.99f));
//    dragHandle->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
//    dragHandle->setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.99f));

    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, bgColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, accentColor);
    panSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, bgColor);
    panSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));

    //pitchSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    //pitchSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());
    //pitchSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));
    pitchSlider.setColour(juce::Slider::ColourIds::backgroundColourId, accentColor);
    pitchSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, bgColor);

    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, bgColor);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, accentColor);
    volumeSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, accentColor);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, accentColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOffId, bgColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOnId, bgColor.brighter(0.2f));
     
    //playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    //playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    editButton.setColour(juce::TextButton::ColourIds::buttonColourId, accentColor);
    editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, accentColor);
    editButton.setColour(juce::TextButton::ColourIds::textColourOffId, bgColor);
    editButton.setColour(juce::TextButton::ColourIds::textColourOnId, bgColor.brighter(0.2f));
    
    //editButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    //editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, accentColor);
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOnId, accentColor);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOffId, bgColor);

    //muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.darker());

    /*reverseButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    reverseButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOffId, moduleColor.darker());
    */
    reverseButton.setColour(juce::TextButton::ColourIds::buttonColourId, accentColor);
    reverseButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, bgColor.brighter(0.9f));
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOnId, accentColor);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOffId, bgColor);

    titleBox.setColour(juce::Label::ColourIds::textColourId, bgColor);//*.darker(0.6f)*/.withAlpha(0.1f));
    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, accentColor);
    titleBox.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, moduleColor.darker(0.7f));
    titleBox.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, moduleColor.darker());
    titleBox.setColour(juce::TextEditor::ColourIds::highlightedTextColourId, moduleColor.contrasting());


    thumbnail.setChannelColor(bgColor);
    thumbnail.setThumbnailBGColor(accentColor);
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker(0.99f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

    timeHandle.setTrackBackgroundColor(accentColor);
    timeHandle.setHandleColor(bgColor);

    //outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, moduleColor.darker(0.55f).withAlpha(0.5f));
    outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, accentColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::textColourId, bgColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::arrowColourId, bgColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.8f));

    //outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));
    outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, juce::Colours::black);
    outputCombo.setColour(juce::PopupMenu::ColourIds::textColourId, moduleColor.darker());
    //outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, moduleColor.darker());
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, juce::Colours::black);
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, moduleColor);

    midiLabel.textColor = bgColor;
    midiLabel.setColour(juce::Label::ColourIds::backgroundColourId, accentColor);
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
    return moduleTree.getProperty(TreeIDs::moduleState.getParamID());
}

int KrumModuleEditor::getModuleSamplerIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleSamplerIndex.getParamID());
}

void KrumModuleEditor::setModuleState(int newState)
{
    moduleTree.setProperty(TreeIDs::moduleState.getParamID(), newState, nullptr);
}

int KrumModuleEditor::getModuleDisplayIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleDisplayIndex.getParamID());
}

void KrumModuleEditor::setModuleDisplayIndex(int newDisplayIndex)
{
    moduleTree.setProperty(TreeIDs::moduleDisplayIndex.getParamID(), newDisplayIndex, nullptr);
}

void KrumModuleEditor::setModuleName(juce::String& newName)
{
    moduleTree.setProperty(TreeIDs::moduleName.getParamID(), newName, nullptr);
    titleBox.setText(newName, juce::dontSendNotification);
    settingsOverlay->setTitle(newName);
}

juce::String KrumModuleEditor::getModuleName()
{
    return moduleTree.getProperty(TreeIDs::moduleName.getParamID()).toString();
}

juce::Colour KrumModuleEditor::getModuleColor()
{
    return  juce::Colour::fromString(moduleTree.getProperty(TreeIDs::moduleColor.getParamID()).toString());
}

void KrumModuleEditor::setModuleColor(juce::Colour newColor)
{
    moduleTree.setProperty(TreeIDs::moduleColor.getParamID(), newColor.toDisplayString(true), nullptr);
}

int KrumModuleEditor::getModuleMidiNote()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiNote.getParamID());
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
    moduleTree.setProperty(TreeIDs::moduleMidiNote.getParamID(), newMidiNote, nullptr);
}

int KrumModuleEditor::getModuleMidiChannel()
{
    return moduleTree.getProperty(TreeIDs::moduleMidiChannel.getParamID());
}

void KrumModuleEditor::setModuleMidiChannel(int newMidiChannel)
{
    moduleTree.setProperty(TreeIDs::moduleMidiChannel.getParamID(), newMidiChannel, nullptr);
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
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModuleMute.getParamID() + juce::String(getModuleSamplerIndex()));
    return *val > 0.5;
}

bool KrumModuleEditor::getModuleReverseState()
{
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModuleReverse.getParamID() + juce::String(getModuleSamplerIndex()));
    return *val > 0.5;
}

int KrumModuleEditor::getModulePitchShift()
{
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModulePitchShift.getParamID() + juce::String(getModuleSamplerIndex()));
    return (int)*val;
}

double KrumModuleEditor::getModuleGain()
{
    auto val = editor.parameters.getRawParameterValue(TreeIDs::paramModuleGain.getParamID() + juce::String(getModuleSamplerIndex()));
    return (double)*val;
}

void KrumModuleEditor::setNumSamplesOfFile(int numSamplesInFile)
{
    moduleTree.setProperty(TreeIDs::moduleNumSamplesLength.getParamID(), numSamplesInFile, nullptr);
}

int KrumModuleEditor::getNumSamplesInFile()
{
    return moduleTree.getProperty(TreeIDs::moduleNumSamplesLength.getParamID());
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
            pos = { slider->getBounds().getCentreX() - 5 /*+ 6*/, getMouseXYRelative().getY() - 5 };
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
        bubbleComp->setColour(juce::BubbleComponent::outlineColourId, getModuleColor().darker(0.7f));
        
    }
    slider->setTooltip(slider->getTextFromValue(slider->getValue()));

}

double KrumModuleEditor::normalizeGainValue(double gain)
{
    auto param = editor.parameters.getParameter(TreeIDs::paramModuleGain.getParamID() + juce::String(getModuleSamplerIndex()));
    double norm = param->convertTo0to1(gain);
    return norm;
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
    juce::File file{ moduleTree.getProperty(TreeIDs::moduleFile.getParamID()) };
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
    pitchSlider.setVisible(sliderShouldBeVisible);
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
        return desc.isNotEmpty() && (desc.contains(DragStrings::favoritesDragString) || desc.contains(DragStrings::recentsDragString));
    }
    return false;
}

//Drag and Drop Target
void KrumModuleEditor::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragDetails)
{
    auto desc = dragDetails.description.toString();
    bool addNextModule = false;                         //set flag true if files are accepted by module, otherwise leave false
    auto& sampler = editor.sampler;
    juce::Array<juce::ValueTree> selectedTrees;

    //grab the correct valueTree from the file browser
    if (desc.contains(DragStrings::favoritesDragString))
    {
        selectedTrees = editor.fileBrowser.getSelectedFileTrees(KrumFileBrowser::BrowserSections::favorites);
    }
    else if (desc.contains(DragStrings::recentsDragString))
    {
        selectedTrees = editor.fileBrowser.getSelectedFileTrees(KrumFileBrowser::BrowserSections::recent);
    }

    //checks to make sure we have enough modules
    if (selectedTrees.size() <= editor.moduleContainer.getNumEmptyModules()) 
    {
        for (int i = 0; i < selectedTrees.size(); ++i)
        {
            auto fileTree = selectedTrees[i];
            
            if (i == 0)
            {
                if (handleNewFile(fileTree))
                {
                    addNextModule = true;
                }
                else
                {
                    DBG("File Not Added");
                }
            }
            else
            {
                auto modulesTree = moduleTree.getParent();
                if (modulesTree.hasType(TreeIDs::KRUMMODULES.getParamID()))
                {
                    for (int j = 0; j < modulesTree.getNumChildren(); j++)
                    {
                        auto itTree = modulesTree.getChild(j);
                        if ((int)itTree.getProperty(TreeIDs::moduleState.getParamID()) == 0) //we grab the first empty module
                        {
                            auto newModEd = editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));
                            newModEd->handleNewFile(fileTree);
                            addNextModule = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not enough modules available", "");
    }


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
    //return shouldModuleAcceptFileDrop();
    return false;
}

//EXTERNAL File Drag and Drop Target
void KrumModuleEditor::filesDropped(const juce::StringArray &files, int x, int y)
{
    //bool addNextModule = false; //set flag true if files are accepted by module, otherwise leave false
    //auto& sampler = editor.sampler;
    //int numFilesDropped = files.size();

    //if(numFilesDropped <= editor.moduleContainer.getNumEmptyModules())
    //{
    //    for (int i = 0; i < files.size(); i++)
    //    {
    //        juce::File audioFile {files[i]};
    //        juce::String fileName = audioFile.getFileName();
    //        juce::int64 numSamples = 0;
    //        if(!audioFile.isDirectory() && sampler.isFileAcceptable(audioFile, numSamples))
    //        {
    //            if (i == 0)
    //            {
    //                handleNewFile(fileName, audioFile, numSamples);
    //                addNextModule = true;
    //                continue;
    //            }

    //            auto modulesTree = moduleTree.getParent();
    //            if (modulesTree.hasType(TreeIDs::KRUMMODULES))
    //            {
    //                for (int j = 0; j < modulesTree.getNumChildren(); j++)
    //                {
    //                    auto itTree = modulesTree.getChild(j);
    //                    if ((int)itTree.getProperty(TreeIDs::moduleState) == 0) //we grab the first empty module
    //                    {
    //                        auto newModEd = editor.moduleContainer.addNewModuleEditor(new KrumModuleEditor(itTree, editor, sampler.getFormatManager()));
    //                        newModEd->handleNewFile(fileName, audioFile, numSamples);
    //                        addNextModule = true;

    //                        DBG("-------");
    //                        DBG("Module Sampler Index: " + itTree.getProperty(TreeIDs::moduleSamplerIndex).toString());
    //                        DBG("Item: " + audioFile.getFullPathName());

    //                        break;
    //                    }
    //                }
    //            }
    //        }
    //        else
    //        {
    //            DBG("External File Not Acceptable");
    //            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");

    //        }
    //    }
    //}

    /*if (addNextModule)
    {
        editor.addNextModuleEditor();
    }*/
}

bool KrumModuleEditor::handleNewFile(juce::ValueTree fileTree, bool overlayShouldListen)
{
    auto file = juce::File{ fileTree.getProperty(TreeIDs::filePath.getParamID()).toString() };
    auto name = fileTree.getProperty(TreeIDs::fileName.getParamID()).toString();
    juce::int64 numSamples = 0;

    if (editor.sampler.isFileAcceptable(file, numSamples))
    {
        DBG("Item: " + file.getFullPathName());
        //juce::String name = file.getFileName(); //compiler reasons
        setModuleName(name);
        setModuleFile(file);
        setNumSamplesOfFile(numSamples);
        setModuleState(KrumModule::ModuleState::hasFile);
        settingsOverlay->setMidiListen(overlayShouldListen);
        addFileToRecentsFolder(file, name);
        return true;
    }
    else
    {
        DBG("Folders Not Supported");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Not Supported", "Either you dropped a folder on here or the file you dropped isn't a supported format");
    }

    return false;
}

void KrumModuleEditor::setModuleFile(juce::File& file)
{
    juce::String filePath = file.getFullPathName();
    moduleTree.setProperty(TreeIDs::moduleFile.getParamID(), filePath, nullptr);
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

bool KrumModuleEditor::getMouseOverKey()
{
    return mouseOverKey;
}

void KrumModuleEditor::setMouseOverKey(bool isMouseOverKey)
{
    mouseOverKey = isMouseOverKey;
}

void KrumModuleEditor::addFileToRecentsFolder(juce::File& file, juce::String name)
{
    editor.fileBrowser.addFileToRecent(file, name);
}

void KrumModuleEditor::zeroModuleTree()
{
    moduleTree.setProperty(TreeIDs::moduleName.getParamID(), juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleFile.getParamID(), juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiNote.getParamID(), juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiChannel.getParamID(), juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleColor.getParamID(), juce::var(""), nullptr);
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

    if (mouseOverKey)
    {
        repaint();
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

    auto color = findColour(shouldDrawButtonAsHighlighted ? juce::TextButton::ColourIds::buttonOnColourId : juce::TextButton::ColourIds::buttonColourId);
    
    auto lineColor = findColour(shouldDrawButtonAsHighlighted ? juce::TextButton::ColourIds::textColourOnId : juce::TextButton::ColourIds::textColourOffId);

    g.setColour(color);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    auto path = getCurrentImage()->getOutlineAsPath();
    
    //auto color = editor.getModuleColor();
    color = shouldDrawButtonAsHighlighted ? color.withAlpha(0.5f) : (shouldDrawButtonAsDown ? color.withAlpha(0.1f) : color.withAlpha(0.8f));

    area.reduce(7, 7);
    path.scaleToFit(area.getX(), area.getY(), area.getWidth(), area.getHeight(), true);
    path.closeSubPath();

    //g.setColour(juce::Colours::black);
    g.setColour(lineColor);
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


    auto color = findColour(shouldDrawButtonAsHighlighted ? juce::TextButton::ColourIds::buttonOnColourId : juce::TextButton::ColourIds::buttonColourId);
    auto lineColor = findColour(shouldDrawButtonAsHighlighted ? juce::TextButton::ColourIds::textColourOnId : juce::TextButton::ColourIds::textColourOffId);

    g.setColour(color);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    auto path = getCurrentImage()->getOutlineAsPath();
    area.reduce(7, 7);
    path.scaleToFit(area.getX(), area.getY(), area.getWidth(), area.getHeight(), true);

    g.setColour(lineColor);
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
    
    juce::Rectangle<int> midiNoteRect = area;//.reduced(7).withX(spacer);
    juce::Rectangle<int> midiChanRect = area.withX(midiNoteRect.getRight() + spacer).withWidth(area.getWidth() * 0.5f);


    g.setColour(findColour(juce::Label::ColourIds::backgroundColourId));
    g.fillRoundedRectangle(midiNoteRect.toFloat(), cornerSize);


    //g.setColour(juce::Colours::black);
    //g.fillRoundedRectangle(midiChanRect.toFloat(), cornerSize);
    
    g.setColour(textColor);
    g.setFont(fontSize);

    g.drawFittedText("Note", midiNoteRect.withX(5), juce::Justification::centredLeft, 1);
    g.drawFittedText(moduleEditor->getModuleMidiNoteString(true), midiNoteRect.withWidth(area.getWidth() - 10), juce::Justification::centredRight, 1);

    //g.drawFittedText("Chan", midiChanRect, juce::Justification::centredLeft, 1);
    //g.drawFittedText(juce::String(moduleEditor->getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);
}

//============================================================================================================================

KrumModuleEditor::PitchSlider::PitchSlider(KrumModuleEditor& e)
    : editor(e), InfoPanelSlider("Pitch", "Click and Drag to shift the pitch by semi-tones, double-click to go back to zero")
{
    setSliderStyle(juce::Slider::LinearBarVertical);

    setVelocityBasedMode(true);
    setVelocityModeParameters();
    setDoubleClickReturnValue(true, 0);


}
KrumModuleEditor::PitchSlider::~PitchSlider()
{
}

void KrumModuleEditor::PitchSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto len = juce::jmin(h, w) * 0.15f;
    auto thick = len / 1.8f;
    int titleH = 12;

    //auto bgColor = findColour(juce::Slider::ColourIds::backgroundColourId);
    auto moduleColor = findColour(juce::Slider::ColourIds::backgroundColourId);
    auto bgColor = Colors::moduleBGColor;
    auto textColor = findColour(juce::Slider::ColourIds::textBoxTextColourId);

    if (isMouseOverOrDragging())
    {
        moduleColor = moduleColor.withAlpha(0.7f);
        textColor = textColor.withAlpha(0.7f);
    }

    g.setColour(moduleColor);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(textColor);

    g.drawFittedText(getTextFromValue(getValue()), bounds.withTop(titleH).reduced(2).toNearestInt(), juce::Justification::centred, 1);

    g.setFont({editor.buttonTextSize + 0.7f});
    g.drawFittedText("Pitch", bounds.withBottom(titleH).toNearestInt(), juce::Justification::centred, 1);
}

void KrumModuleEditor::PitchSlider::mouseDown(const juce::MouseEvent& e)
{

   setMouseCursor(juce::MouseCursor::NoCursor);
   InfoPanelSlider::mouseDown(e);
}


void KrumModuleEditor::PitchSlider::mouseUp(const juce::MouseEvent& e)
{

    juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(e.source.getLastMouseDownPosition());

    setMouseCursor(juce::MouseCursor::NormalCursor);
    InfoPanelSlider::mouseUp(e);
}

//void KrumModuleEditor::PitchSlider::mouseExit(const juce::MouseEvent& e)
//{
//    editor.setPitchSliderVisibility(false);
//}

//=======================================================================================

KrumModuleEditor::CustomToggleButton::CustomToggleButton(juce::String title, juce::String message, KrumModuleEditor& e)
    : InfoPanelTextButton(title, message), editor(e)
{
}

KrumModuleEditor::CustomToggleButton::~CustomToggleButton()
{
}

void KrumModuleEditor::CustomToggleButton::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto area = getLocalBounds();
    int titleH = 10;
    bool buttonOn = getToggleState();

    auto bgColor = findColour(buttonOn ? juce::TextButton::ColourIds::buttonOnColourId : juce::TextButton::ColourIds::buttonColourId);
    auto textColor = findColour(buttonOn ? juce::TextButton::ColourIds::textColourOnId : juce::TextButton::ColourIds::textColourOffId);

    if (isMouseOver())
    {
        bgColor = bgColor.withAlpha(0.7f);
        textColor = textColor.withAlpha(0.7f);
    }

    g.setColour(bgColor);
    g.fillRoundedRectangle(area.toFloat(), 5.0f);

    g.setColour(textColor);
    g.setFont({editor.buttonTextSize});
    g.drawFittedText(getButtonText(), area/*.withY(3).withBottom(titleH)*/.reduced(3), juce::Justification::centred, 1);

}
