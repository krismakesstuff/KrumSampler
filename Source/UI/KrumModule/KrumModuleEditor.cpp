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
//#include "../FileBrowser/KrumFileBrowser.h"
#include "../InfoPanel.h"
#include "../Source/UI/KrumModuleContainer.h"


//===============================================================================================//

KrumModuleEditor::KrumModuleEditor(juce::ValueTree& modTree, KrumModuleContainer& mc, juce::AudioFormatManager& fm)
    : moduleTree(modTree), moduleContainer(mc), timeHandle(*this),
    thumbnail(*this, THUMBNAIL_RES, fm, mc.getPluginEditor()->getThumbnailCache())
{
    setPaintingIsUnclipped(true);
    setRepaintsOnMouseActivity(true);

    moduleTree.addListener(this);

    startTimerHz(30);
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);

    //int state = getModuleState();
    
    valueTreePropertyChanged(moduleTree, TreeIDs::moduleState.getParamID());
    
    if (isModuleMuted())
    {
        setChildCompMuteColors();
    }
}

KrumModuleEditor::~KrumModuleEditor()
{
    pitchSliderAttachment.reset();
}

void KrumModuleEditor::paint (juce::Graphics& g)
{

    auto area = getLocalBounds().reduced(EditorDimensions::shrinkage);
    juce::Colour c = getModuleColor();

    int moduleState = getModuleState();

    if (moduleState == KrumModule::ModuleState::active || moduleState == KrumModule::ModuleState::hasFile) 
    {
        //juce::Colour bc = c.darker(0.8f)/*.withSaturation(0.5f)*/;
        juce::Colour bc = juce::Colours::black.brighter(0.075f);
        //juce::Colour bc = c.darker(0.9f);

        if (isModuleMuted())
        {
            //bc = c.withAlpha(0.2f);
            bc = juce::Colours::black;
            c = juce::Colours::black;
        }
        else if (modulePlaying)
        {
            c = c.overlaidWith(Colors::getModulePlayingHightlightColor());
        }

        if (mouseOver || mouseOverKey) 
        {
            g.setColour(Colors::getModuleHoverOutlineColor());
            g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, EditorDimensions::bigOutline);
        }

        if (isModuleSelected())
        {
            if (moduleContainer.isMultiControlActive())
            {
                g.setColour(Colors::getModuleMultiControlAcitveColor());
            }
            else
            {
                g.setColour(Colors::getModuleSelectedOutlineColor());
            }

            g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, EditorDimensions::xlOutline);
        }

        //auto bgGrade = juce::ColourGradient::vertical(c.darker(0.3f), (float)area.getY(), bc, area.getBottom());
        auto bgGrade = juce::ColourGradient::vertical(bc, (float)area.getY(), c.darker(0.8f), area.getBottom());

        auto gain = getModuleGain();
        auto gainProp = 1 - normalizeGainValue(gain);
        //bgGrade.addColour(juce::jlimit<double>(0.00001,0.9999, gainProp),c.darker(0.2f));
        bgGrade.addColour(juce::jlimit<double>(0.00001,0.9999, gainProp), bc.overlaidWith(c.darker(0.8f).withAlpha(0.3f)));
        

        g.setGradientFill(bgGrade);
        //g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);
        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };

        //g.setColour(bc);
        //g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, 1.0f);
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

    int volumeSliderH = height * 0.48f;
    int volumeSliderW = width * 0.57f;

    int buttonH = height * 0.085f; 
    int buttonW = width * 0.3f;

    int smallButtonH = height * 0.07f;
    int smallButtonW = width * 0.25f;

    int outputComboH = height * 0.073f;
    int dragHandleH = height * 0.089f;

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    thumbnail.setBounds(area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer));
    timeHandle.setBounds(thumbnail.getX(), thumbnail.getBottom(), thumbnail.getWidth(), timeHandleH);

    panSlider.setBounds(area.getX() + 2, timeHandle.getBottom() + (spacer), panSliderW, panSliderH);
    volumeSlider.setBounds(area.getX() + spacer/*area.getCentreX() - (volumeSliderW / 2)*/, panSlider.getBottom() + (spacer/* * 3*/), volumeSliderW, volumeSliderH);
    
    pitchSlider.setBounds(area.withTop(panSlider.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    reverseButton.setBounds(area.withTop(pitchSlider.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    muteButton.setBounds(area.withTop(reverseButton.getBottom() + spacer).withHeight(smallButtonH).withLeft(area.getRight() - (smallButtonW + spacer)).withWidth(smallButtonW));
    
    playButton.setBounds(area.withTop(muteButton.getBottom() + (spacer * 6)).withHeight(buttonH).withLeft(area.getRight() - (buttonW + spacer)).withWidth(buttonW));
    menuButton.setBounds(area.withTop(playButton.getBottom() + spacer).withHeight(buttonH).withLeft(area.getRight() - (buttonW +spacer)).withWidth(buttonW));

    midiLabel.setBounds(area.withTrimmedTop(volumeSlider.getBottom() - spacer).withHeight(midiLabelH).reduced(spacer));
    outputCombo.setBounds(area.withTop(midiLabel.getBottom()).withHeight(outputComboH).reduced(spacer));

    dragHandle.setBounds(area.withTop(outputCombo.getBottom()).withHeight(dragHandleH).reduced(spacer));

    if (settingsOverlay && settingsOverlay->isVisible())
    {
        settingsOverlay->setBounds(area.withBottom(menuButton.getBottom() + spacer).withTop(playButton.getY()).withHeight(menuButton.getHeight() + playButton.getHeight() + spacer).withRight(menuButton.getX()));
    }

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
        InfoPanel::shared_instance().setInfoPanelText("Settings Overlay", "To assign a new midi note, enable midi listen, and play/click your note. You can also change the color or delete the module");
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
    if (settingsOverlay && settingsOverlay->isVisible())
    {
        hideSettingsOverlay();
    }


    juce::Component::mouseDown(e);
}

void KrumModuleEditor::mouseUp(const juce::MouseEvent& e)
{
    moduleContainer.setModuleSelectedFromClick(this, !isModuleSelected(), e);

    juce::Component::mouseUp(e);
    
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
                buildModule();
                setModuleColor(Colors::getModuleDefaultColor());
                setModuleListeningForMidi(true);
                showSettingsOverlay(); 

            }
            else if (state == KrumModule::ModuleState::active)
            {
                if (getNumChildComponents() == 0)
                {
                    buildModule();
                }
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
        else if (property == juce::Identifier(TreeIDs::moduleSelected.getParamID()))
        {
            repaint();
        }
    
        //TODO: review,
        //this passes the data for the time handles, I don't think anything else gets here... not sure
        if (moduleContainer.isMultiControlActive() && isModuleSelected())
        {
            //moduleContainer.applyValueTreeChangesToSelectedModules(treeWhoChanged, property);
        }

    }
    else if (treeWhoChanged != moduleTree && treeWhoChanged.isValid() && isModuleSelected()) 
    {
        //this branch is when another module changes it's properties but we triggered this property callback manually using applyChangesToSelectedModules()
        //drag handles seem to kind of work through this



        moduleTree.setProperty(property, treeWhoChanged.getProperty(property), nullptr);
        repaint();
    }


}
//===============================================================================================================

void KrumModuleEditor::buildModule()
{


    addAndMakeVisible(titleBox);
    titleBox.setText(moduleTree.getProperty(TreeIDs::moduleName.getParamID()).toString(), juce::NotificationType::dontSendNotification);
    titleBox.setFont(moduleContainer.getPluginEditor()->getKrumLaf()->getMontBoldTypeface());
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
   
    juce::String i{ getSamplerIndexString() };

    auto& parameters = *moduleContainer.getAPVTS();

    addAndMakeVisible(thumbnail);
    thumbnail.clipGainSliderAttachment.reset(new SliderAttachment(parameters, TreeIDs::paramModuleClipGain.getParamID() + i, thumbnail.clipGainSlider));
    

    addAndMakeVisible(timeHandle);
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setLookAndFeel(moduleContainer.getPluginEditor()->getVolumeLaf());
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setNumDecimalPlacesToDisplay(2);
    volumeSlider.setDoubleClickReturnValue(true, 1.0f);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTooltip(volumeSlider.getTextFromValue(volumeSlider.getValue()));
    volumeSlider.onValueChange = [this] 
    { 
        updateBubbleComp(&volumeSlider, volumeSlider.getCurrentPopupDisplay());
        moduleContainer.updateSlidersIfBeingMultiControlled(this, &volumeSlider);
    };
    
    volumeSlider.onDragEnd = [this] {   printValueAndPositionOfSlider(); };

    volumeSliderAttachment.reset(new SliderAttachment(parameters, TreeIDs::paramModuleGain.getParamID() + i, volumeSlider));

    addAndMakeVisible(panSlider);
    panSlider.setLookAndFeel(moduleContainer.getPluginEditor()->getPanLaf());
    panSlider.setScrollWheelEnabled(false);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setNumDecimalPlacesToDisplay(2);
    panSlider.setDoubleClickReturnValue(true, 1.0f);
    panSlider.setPopupDisplayEnabled(true, false, this);
    panSlider.setTooltip(panSlider.getTextFromValue(panSlider.getValue()));

    panSlider.onValueChange = [this] 
    { 
        updateBubbleComp(&panSlider, panSlider.getCurrentPopupDisplay()); 
        moduleContainer.updateSlidersIfBeingMultiControlled(this, &panSlider);
    };

    panSliderAttachment.reset(new SliderAttachment(parameters, TreeIDs::paramModulePan.getParamID() + i, panSlider));

    addAndMakeVisible(midiLabel);

    auto playButtonImage = juce::Drawable::createFromImageData(BinaryData::nounplaywhite_svg, BinaryData::nounplaywhite_svgSize);

    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());
    playButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    playButton.onMouseDown = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseDown(e); };
    playButton.onMouseUp = [this](const juce::MouseEvent& e) { handleOneShotButtonMouseUp(e); };
    

    addAndMakeVisible(pitchSlider);
    pitchSlider.setLookAndFeel(moduleContainer.getPluginEditor()->getPitchLaf());
    pitchSlider.setScrollWheelEnabled(false);
    pitchSlider.setTooltip(pitchSlider.getTextFromValue(pitchSlider.getValue()));
    pitchSlider.setPopupDisplayEnabled(true, false, this, 1000);

    pitchSlider.onValueChange = [this] 
    { 
        updateBubbleComp(&pitchSlider, pitchSlider.getCurrentPopupDisplay()); 
        moduleContainer.updateSlidersIfBeingMultiControlled(this, &pitchSlider);
    };
    
    pitchSliderAttachment.reset(new SliderAttachment(parameters, TreeIDs::paramModulePitchShift.getParamID() + i, pitchSlider));

    addAndMakeVisible(reverseButton);
    reverseButton.setButtonText("REV");
    reverseButton.setToggleState(getModuleReverseState(), juce::dontSendNotification);
    reverseButton.setClickingTogglesState(true);
    reverseButtonAttachment.reset(new ButtonAttachment(parameters, TreeIDs::paramModuleReverse.getParamID() + i, reverseButton));

    addAndMakeVisible(muteButton);
    muteButton.setButtonText("MUTE");
    muteButton.setToggleState(isModuleMuted(), juce::dontSendNotification);
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, Colors::getModuleMuteActiveColor());
    muteButton.onClick = [this] { muteButtonClicked(); };
    muteButtonAttachment.reset(new ButtonAttachment(parameters, TreeIDs::paramModuleMute.getParamID() + i, muteButton));
    

    auto editButtonImage = juce::Drawable::createFromImageData(BinaryData::nounmenuwhite_svg, BinaryData::nounmenuwhite_svgSize);
    

    addAndMakeVisible(menuButton);
    menuButton.setImages(editButtonImage.get());
    
    menuButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    menuButton.onClick = [this] { toggleMenuButton(); };


    addAndMakeVisible(outputCombo);
    outputCombo.addItemList(TreeIDs::outputStrings, 1);
    outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, Colors::getModuleOutputMenuBG());
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, Colors::getModuleOutputMenuBG());
    outputCombo.onChange = [this] { moduleContainer.updateComboIfBeingMultiControlled(this, &outputCombo); };

    outputComboAttachment.reset(new ComboBoxAttachment(parameters, TreeIDs::paramModuleOutputChannel.getParamID() + i, outputCombo));


    addAndMakeVisible(dragHandle);
    auto dragHandleImage = juce::Drawable::createFromImageData(BinaryData::drag_handleblack18dp_svg, BinaryData::drag_handleblack18dp_svgSize);

    dragHandle.setImages(dragHandleImage.get());
    dragHandle.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);


    settingsOverlay.reset(new ModuleSettingsOverlay(*this));
    addChildComponent(settingsOverlay.get());
    

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

    auto bgColor = Colors::getModuleBGColor().overlaidWith(moduleColor.withAlpha(0.4f));
    auto titleTextColor = moduleColor.withSaturation(0.65f).darker(0.1f);
    auto textColor = moduleColor.withSaturation(0.6f).brighter(0.2f);

    if (moduleColor == Colors::getModuleDefaultColor())
    {
        titleTextColor = moduleColor;
        textColor = moduleColor.brighter();
    }

    dragHandle.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, moduleColor.darker(0.5f));
    dragHandle.setColour(juce::DrawableButton::ColourIds::backgroundColourId, moduleColor.darker(0.79f));
    dragHandle.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.99f));

    //panSlider.setColour(juce::Slider::ColourIds::thumbColourId, textColor);
    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, titleTextColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, titleTextColor);
    panSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, titleTextColor);
    panSlider.setColour(juce::TooltipWindow::textColourId, titleTextColor);

    pitchSlider.setColour(juce::Slider::ColourIds::backgroundColourId, bgColor);
    pitchSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, textColor);
    pitchSlider.setColour(juce::Slider::ColourIds::trackColourId, titleTextColor.darker(0.3f));

    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, titleTextColor);
    //volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, bgColor.darker(0.5f));
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, titleTextColor);
    volumeSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, bgColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOnId, textColor.brighter(0.2f));
     
    menuButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    menuButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, bgColor);
    menuButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);
    menuButton.setColour(juce::TextButton::ColourIds::textColourOnId, textColor.brighter(0.2f));

    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOnId, bgColor);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);

    reverseButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    reverseButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, titleTextColor);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOnId, textColor);

    titleBox.setColour(juce::Label::ColourIds::textColourId, titleTextColor.brighter(0.2f));//*.darker(0.6f)*/.withAlpha(0.1f));
    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, bgColor.darker(0.3f));
    titleBox.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, moduleColor.darker(0.7f));
    titleBox.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, moduleColor.darker());
    titleBox.setColour(juce::TextEditor::ColourIds::highlightedTextColourId, moduleColor.contrasting());


    thumbnail.setChannelColor(titleTextColor.brighter());
    //thumbnail.setChannelColor(juce::Colours::black);
    thumbnail.setThumbnailBGColor(bgColor.withAlpha(0.4f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker(0.99f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

    timeHandle.setTrackBackgroundColor(moduleColor.darker(0.9f));
    //timeHandle.setTrackBackgroundColor(ju);
    timeHandle.setHandleColor(titleTextColor.darker(0.2f));

    //outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, moduleColor.darker(0.55f).withAlpha(0.5f));
    outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, bgColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::textColourId, textColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::arrowColourId, textColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.8f));

    //outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));

    outputCombo.setColour(juce::PopupMenu::ColourIds::textColourId, titleTextColor);
    //outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, moduleColor.darker());
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, titleTextColor);

    midiLabel.textColor = textColor;
    midiLabel.setColour(juce::Label::ColourIds::backgroundColourId, bgColor);

}

//sets visibility to false
void KrumModuleEditor::hideSettingsOverlay()
{
    //setModuleButtonsClickState(true);
    if (settingsOverlay)
    {
        settingsOverlay->setVisible(false);
        resized();
        repaint();
    }   
    //showModule();
}

//void KrumModuleEditor::showNewSettingsOverlay()
//{
//    showSettingsOverlay(false, true);
//}

//sets visibility to true
void KrumModuleEditor::showSettingsOverlay()
{
    //hideModule();
    //setModuleButtonsClickState(false);
    if (settingsOverlay)
    {
        settingsOverlay->setVisible(true);
        resized();
        repaint();
    }
    //settingsOverlay->setMidi(getModuleMidiNote(), getModuleMidiChannel());
    //settingsOverlay->keepCurrentColor(keepCurrentColorOnExit);
    //juce::String name = getModuleName(); //compiler reasons
    //settingsOverlay->setTitle(name); 
    
    //if (selectOverlay)
    //{
    //    //need to clear the other modules that were previously selected
    //    //unless you do want to, i.e. isShiftDown()
    //    pluginEditor.getModuleContainer().deselectAllModules();
    //    pluginEditor.getModuleContainer().setModuleSelected(this);
    //    pluginEditor.keyboard.scrollToKey(getModuleMidiNote());
    //    repaint();
    //}
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

//sets either the editor or the settings overlay to selected, they are seperate values
void KrumModuleEditor::setModuleSelected(bool isModuleSelected)
{
    if (!isModuleSelected && moduleSelected && settingsOverlay->isVisible()) 
    {
        //we are deselecting a currently selected module while the settings overaly is still open. This closes/hides it.
        hideSettingsOverlay();
    }

    moduleTree.setProperty(TreeIDs::moduleSelected.getParamID(), isModuleSelected ? juce::var(1) : juce::var(0), nullptr);
    moduleSelected = isModuleSelected;
    repaint();

}

bool KrumModuleEditor::isModuleSelected()
{
    return (float)moduleTree.getProperty(TreeIDs::moduleSelected.getParamID()) > 0.5;
}

int KrumModuleEditor::getModuleState()
{
    return moduleTree.getProperty(TreeIDs::moduleState.getParamID());
}

int KrumModuleEditor::getModuleSamplerIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleSamplerIndex.getParamID());
}

juce::String KrumModuleEditor::getSamplerIndexString()
{
    int i = getModuleSamplerIndex();
    if (i < 10)
    {
        return "0" + juce::String{ i };
    }
    
    return juce::String{ i };

}

void KrumModuleEditor::setModuleState(int newState)
{
    moduleTree.setProperty(TreeIDs::moduleState.getParamID(), newState, nullptr);
}

int KrumModuleEditor::getModuleDisplayIndex()
{
    return moduleTree.getProperty(TreeIDs::moduleDisplayIndex.getParamID());
}

void KrumModuleEditor::setModuleDisplayIndex(int newDisplayIndex, bool sendChange)
{
    if (sendChange)
    {
        moduleTree.setProperty(TreeIDs::moduleDisplayIndex.getParamID(), newDisplayIndex, nullptr);

    }
    else
    {
        moduleTree.setPropertyExcludingListener(this, TreeIDs::moduleDisplayIndex.getParamID(), newDisplayIndex, nullptr);

    }
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
    auto val = moduleContainer.getAPVTS()->getRawParameterValue(TreeIDs::paramModuleMute.getParamID() + juce::String(getSamplerIndexString()));
    return *val > 0.5;
}

bool KrumModuleEditor::getModuleReverseState()
{
    auto val = moduleContainer.getAPVTS()->getRawParameterValue(TreeIDs::paramModuleReverse.getParamID() + juce::String(getSamplerIndexString()));
    return *val > 0.5;
}

int KrumModuleEditor::getModulePitchShift()
{
    auto val = moduleContainer.getAPVTS()->getRawParameterValue(TreeIDs::paramModulePitchShift.getParamID() + juce::String(getSamplerIndexString()));
    return (int)*val;
}

double KrumModuleEditor::getModuleGain()
{
    auto val = moduleContainer.getAPVTS()->getRawParameterValue(TreeIDs::paramModuleGain.getParamID() + juce::String(getSamplerIndexString()));
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
        auto sliderName = slider->getName();
        if (sliderName == volumeSlider.getName())
        {
            pos = { slider->getBounds().getCentreX() - 5 /*+ 6*/, getMouseXYRelative().getY() - 5 };
        }
        else if (sliderName == panSlider.getName())
        {
            int mouseX = getMouseXYRelative().getX();
            int leftBound = area.getX() + 20;
            int rightBound = area.getRight() - 20;
            
            pos = {juce::jlimit<int>(leftBound, rightBound, mouseX), slider->getBounds().getCentreY() - 10 };
        }
        else if (sliderName == pitchSlider.getName())
        {
            pos = {slider->getBounds().getCentreX(), slider->getBounds().getY()};
        }
        

        bubbleComp->setAllowedPlacement(bubblePlacement);
        bubbleComp->setPosition(pos, 0);
        bubbleComp->setColour(juce::BubbleComponent::outlineColourId, getModuleColor().darker(0.7f));
        
    }
    slider->setTooltip(slider->getTextFromValue(slider->getValue()));

}

double KrumModuleEditor::normalizeGainValue(double gain)
{
    auto param = moduleContainer.getAPVTS()->getParameter(TreeIDs::paramModuleGain.getParamID() + juce::String(getSamplerIndexString()));
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
    //return settingsOverlay->isVisible() && settingsOverlay->isOverlaySelected();
    return midiLabel.isListeningForMidi();
}

//this has many ways it can call a repaint, make sure not to call this method from a real-time source
void KrumModuleEditor::handleMidi(int midiChannel, int midiNote)
{
    if (midiLabel.isListeningForMidi())
    {
        setModuleMidiChannel(midiChannel);
        setModuleMidiNote(midiNote);
        midiLabel.setListeningForMidi(false);

        if (getModuleState() == KrumModule::ModuleState::hasFile)
        {
            setModuleState(KrumModule::ModuleState::active);
            if (getModuleColor() != Colors::getModuleDefaultColor())
            {
                toggleMenuButton();
            }
        }
    }

    /*if (settingsOverlay != nullptr && settingsOverlay->isVisible())
    {
        settingsOverlay->handleMidiInput(midiChannel, midiNote);
    }*/
}

void KrumModuleEditor::toggleMenuButton()
{
    if (settingsOverlay)
    {
        bool visible = settingsOverlay->isVisible();
        if (visible)
        {
            hideSettingsOverlay();
        }
        else
        {
            showSettingsOverlay();
        }
        
        if (isModuleSelected() && moduleContainer.isMultiControlActive())
        {
            moduleContainer.setSettingsOverlayOnSelectedModules(visible,this);
        }
    }
}

void KrumModuleEditor::triggerMouseDownOnNote(const juce::MouseEvent& e)
{
    int note = getModuleMidiNote();
    moduleContainer.getPluginEditor()->getKeyboard().mouseDownOnKey(note, e);
    moduleContainer.getPluginEditor()->getSampler().noteOn(getModuleMidiChannel(), note, buttonClickVelocity);
}

void KrumModuleEditor::triggerMouseUpOnNote(const juce::MouseEvent& e)
{
    int note = getModuleMidiNote();
     moduleContainer.getPluginEditor()->getKeyboard().mouseUpOnKey(note, e);
     moduleContainer.getPluginEditor()->getSampler().noteOff(getModuleMidiChannel(), note, buttonClickVelocity, true);
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

//void KrumModuleEditor::setPitchSliderVisibility(bool sliderShouldBeVisible)
//{
//    pitchSlider.setVisible(sliderShouldBeVisible);
//}

bool KrumModuleEditor::canThumbnailAcceptFile()
{
    return thumbnail.canAcceptFile;
}

void KrumModuleEditor::setThumbnailCanAcceptFile(bool shouldAcceptFile)
{
    thumbnail.canAcceptFile = shouldAcceptFile;
    thumbnail.repaint();
}



void KrumModuleEditor::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails &dragDetails)
{
    //
}



void KrumModuleEditor::setModuleFile(juce::File& file)
{
    juce::String filePath = file.getFullPathName();
    if (getModuleState() == KrumModule::ModuleState::empty)
    {
        setModuleState(KrumModule::ModuleState::hasFile);
    }

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

bool KrumModuleEditor::isModuleTree(juce::ValueTree& treeToTest)
{
    if (treeToTest.isValid())
    {
        return moduleTree == treeToTest && (int)treeToTest.getProperty(TreeIDs::moduleSamplerIndex.getParamID()) == getModuleSamplerIndex();
    }

    return false;
}

void KrumModuleEditor::muteButtonClicked()
{
    if (isModuleMuted())
    {
        setChildCompMuteColors();
    }
    else
    {
        setChildCompColors();
    }

}

void KrumModuleEditor::setChildCompMuteColors()
{
    auto moduleColor = getModuleColor()/*.withAlpha(0.5f)*/;

    auto bgColor = Colors::getModuleBGColor();
    auto textColor = moduleColor.darker(0.1f);

    dragHandle.setColour(juce::DrawableButton::ColourIds::backgroundColourId, moduleColor.darker(0.99f));
    dragHandle.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, moduleColor.darker(0.79f));
    dragHandle.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.99f));

    //panSlider.setColour(juce::Slider::ColourIds::thumbColourId, textColor);
    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, bgColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, textColor);
    panSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, bgColor);
    panSlider.setColour(juce::TooltipWindow::textColourId, textColor);

    //pitchSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    //pitchSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());
    //pitchSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));
    pitchSlider.setColour(juce::Slider::ColourIds::backgroundColourId, bgColor);
    pitchSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, textColor);

    //volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, textColor);
    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, bgColor);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, textColor);
    volumeSlider.setColour(juce::TooltipWindow::textColourId, moduleColor.brighter(0.8f));
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, bgColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);
    playButton.setColour(juce::TextButton::ColourIds::textColourOnId, textColor.brighter(0.2f));

    //playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    //playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    menuButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    menuButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, bgColor);
    menuButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);
    menuButton.setColour(juce::TextButton::ColourIds::textColourOnId, textColor.brighter(0.2f));

    //editButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    //editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);

    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOnId, bgColor);
    muteButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);

    reverseButton.setColour(juce::TextButton::ColourIds::buttonColourId, bgColor);
    reverseButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, textColor.brighter(0.9f));
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOnId, bgColor);
    reverseButton.setColour(juce::TextButton::ColourIds::textColourOffId, textColor);

    titleBox.setColour(juce::Label::ColourIds::textColourId, textColor.brighter(0.2f));//*.darker(0.6f)*/.withAlpha(0.1f));
    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, bgColor);
    titleBox.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, moduleColor.darker(0.7f));
    titleBox.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, moduleColor.darker());
    titleBox.setColour(juce::TextEditor::ColourIds::highlightedTextColourId, moduleColor.contrasting());


    thumbnail.setChannelColor(textColor.darker(0.95f));
    thumbnail.setThumbnailBGColor(juce::Colours::black);
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker(0.99f));
    thumbnail.clipGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, moduleColor.brighter().withAlpha(0.5f));

    timeHandle.setTrackBackgroundColor(moduleColor.darker(0.9f));
    timeHandle.setHandleColor(juce::Colours::black);

    //outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, moduleColor.darker(0.55f).withAlpha(0.5f));
    outputCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, bgColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::textColourId, textColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::arrowColourId, textColor);
    outputCombo.setColour(juce::ComboBox::ColourIds::outlineColourId, moduleColor.darker(0.8f));

    //outputCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));

    outputCombo.setColour(juce::PopupMenu::ColourIds::textColourId, moduleColor.darker());
    //outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId, moduleColor.darker());
    outputCombo.setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, moduleColor);

    midiLabel.textColor = textColor;
    midiLabel.setColour(juce::Label::ColourIds::backgroundColourId, bgColor);

    //dragHandle.setColour();

    //getLookAndFeel().setColour(juce::PopupMenu::ColourIds::backgroundColourId, moduleColor.darker(0.55f));

}

void KrumModuleEditor::setModuleListeningForMidi(bool shouldListen)
{
    midiLabel.setListeningForMidi(shouldListen);
}

void KrumModuleEditor::zeroModuleTree()
{
    moduleTree.setProperty(TreeIDs::moduleName.getParamID(), juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleFile.getParamID(), juce::var(""), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiNote.getParamID(), juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleMidiChannel.getParamID(), juce::var(0), nullptr);
    moduleTree.setProperty(TreeIDs::moduleColor.getParamID(), juce::var(""), nullptr);
    timeHandle.resetHandles();
    

    DBG("Module " + getSamplerIndexString() + " zeroed");
}

void KrumModuleEditor::timerCallback()
{
    //Thumbnail
    if (drawThumbnail)
    {
        setAndDrawThumbnail();
    }

    //keyboard highlighting if mouse is over this component, we also draw the outline based off the mouseOver value. see KrumModuleEditor::paint()
    int currentMidiNote = getModuleMidiNote();
    if (getLocalBounds().contains(getMouseXYRelative()) && getModuleState() > 0) //if mouse is over module and module is active
    {
        mouseOver = true;
        moduleContainer.getPluginEditor()->getKeyboard().setHighlightKey(currentMidiNote, true);
    }
    else
    {
        //module container will use this to clear the highlighted keys
        mouseOver = false;
    }

    
    //sendToSelectedModules = moduleContainer.multipleModulesSelected() && isModuleSelected();
    

    if (mouseOverKey || modulePlaying)
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
    /*if (e.mods.isShiftDown())
    {
        editor.keyboard.scrollToKey(getModuleMidiNote());
    }*/

    triggerMouseDownOnNote(e);
}

void KrumModuleEditor::handleOneShotButtonMouseUp(const juce::MouseEvent& e)
{
    triggerMouseUpOnNote(e);
}


void KrumModuleEditor::reassignSliderAttachment(juce::Slider* sliderToAssign, bool beginGesture)
{
    auto sliderName = sliderToAssign->getName();
    auto& apvts = *moduleContainer.getAPVTS();
    if (sliderName == volumeSlider.getName())
    {
        juce::String newParamString = TreeIDs::paramModuleGain.getParamID() + getSamplerIndexString();
        volumeSliderAttachment.reset(new SliderAttachment(apvts, newParamString, *sliderToAssign));
        if(beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
    else if (sliderName == panSlider.getName())
    {
        juce::String newParamString = TreeIDs::paramModulePan.getParamID() + getSamplerIndexString();
        panSliderAttachment.reset(new SliderAttachment(apvts, newParamString, *sliderToAssign));
        if (beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
    else if (sliderName == pitchSlider.getName())
    {
        juce::String newParamString = TreeIDs::paramModulePitchShift.getParamID() + getSamplerIndexString();
        pitchSliderAttachment.reset(new SliderAttachment(apvts, newParamString, *sliderToAssign));
        if (beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
}

void KrumModuleEditor::updateSliderFromMultiControl(juce::Slider* sourceSlider)
{
    auto sliderName = sourceSlider->getName();

    if (sliderName == volumeSlider.getName())
    {
        volumeSlider.setValue(sourceSlider->getValue(), juce::dontSendNotification);
    }
    else if (sliderName == panSlider.getName())
    {
        panSlider.setValue(sourceSlider->getValue(), juce::dontSendNotification);
    }
    else if (sliderName == pitchSlider.getName())
    {
        pitchSlider.setValue(sourceSlider->getValue(), juce::dontSendNotification);
    }
}

void KrumModuleEditor::resetSliderAttachments()
{
    auto& apvts = *moduleContainer.getAPVTS();

    volumeSliderAttachment.reset(new SliderAttachment(apvts, TreeIDs::paramModuleGain.getParamID() + getSamplerIndexString(), volumeSlider));
    panSliderAttachment.reset(new SliderAttachment(apvts, TreeIDs::paramModulePan.getParamID() + getSamplerIndexString(), panSlider));
    pitchSliderAttachment.reset(new SliderAttachment(apvts, TreeIDs::paramModulePitchShift.getParamID() + getSamplerIndexString(), pitchSlider));

}


void KrumModuleEditor::reassignButtonAttachment(juce::Button* buttonToAssign, bool beginGesture)
{
    auto buttonName = buttonToAssign->getName();
    auto& apvts = *moduleContainer.getAPVTS();
    if (buttonName == reverseButton.getName())
    {
        juce::String newParamString = TreeIDs::paramModuleReverse.getParamID() + getSamplerIndexString();
        reverseButtonAttachment.reset(new ButtonAttachment(apvts, newParamString, *buttonToAssign));
        if (beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
    else if (buttonName == muteButton.getName())
    {
        juce::String newParamString = TreeIDs::paramModuleMute.getParamID() + getSamplerIndexString();
        muteButtonAttachment.reset(new ButtonAttachment(apvts, newParamString, *buttonToAssign));
        if (beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
    
}

void KrumModuleEditor::resetButtonAttachments()
{
    auto& apvts = *moduleContainer.getAPVTS();

    reverseButtonAttachment.reset(new ButtonAttachment(apvts, TreeIDs::paramModuleReverse.getParamID() + getSamplerIndexString(), reverseButton));
    muteButtonAttachment.reset(new ButtonAttachment(apvts, TreeIDs::paramModuleMute.getParamID() + getSamplerIndexString(), muteButton));
}

void KrumModuleEditor::reassignComboAttachment(juce::ComboBox* comboToAssign, bool beginGesture)
{
    auto comboName = comboToAssign->getName();
    auto& apvts = *moduleContainer.getAPVTS();
    if (comboName == outputCombo.getName())
    {
        juce::String newParamString = TreeIDs::paramModuleOutputChannel.getParamID() + getSamplerIndexString();
        outputComboAttachment.reset(new ComboBoxAttachment(apvts, newParamString, *comboToAssign));
        if (beginGesture)
            apvts.getParameter(newParamString)->beginChangeGesture();
    }
    
}

void KrumModuleEditor::updateComboFromMultiControl(juce::ComboBox* sourceCombo)
{
    auto comboName = sourceCombo->getName();

    if (comboName == outputCombo.getName())
    {
        outputCombo.setSelectedId(sourceCombo->getSelectedId(), juce::dontSendNotification);
    }
}

void KrumModuleEditor::resetComboAttachments()
{
    auto& apvts = *moduleContainer.getAPVTS();

    outputComboAttachment.reset(new ComboBoxAttachment(apvts, TreeIDs::paramModuleOutputChannel.getParamID() + getSamplerIndexString(), outputCombo));

}

//======================= OneShotButton =====================================================================================================

KrumModuleEditor::OneShotButton::OneShotButton(KrumModuleEditor& e, juce::String title, juce::String message)
: InfoPanelDrawableButton(juce::DrawableButton::ButtonStyle::ImageOnButtonBackground, title, message), editor(e)
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

    if (editor.isModuleSelected() && editor.moduleContainer.isMultiControlActive())
    {
        editor.moduleContainer.clickOneShotOnSelectedModules(e, &editor, true);
    }

    if (onMouseDown)
    {
        onMouseDown(e);
    }

}

void KrumModuleEditor::OneShotButton::mouseUp(const juce::MouseEvent& e)
{
    Button::mouseUp(e);
    
    if (editor.isModuleSelected() && editor.moduleContainer.isMultiControlActive())
    {
        editor.moduleContainer.clickOneShotOnSelectedModules(e, &editor, false);
    }

    if (onMouseUp)
    {
        onMouseUp(e);
    }

}

//================== MenuButton ==========================================================================================================

KrumModuleEditor::MenuButton::MenuButton(KrumModuleEditor& e, juce::String title, juce::String message)
    : InfoPanelDrawableButton(juce::DrawableButton::ButtonStyle::ImageOnButtonBackground,title, message ), editor(e)
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

void KrumModuleEditor::MenuButton::mouseUp(const juce::MouseEvent& e)
{
    if (editor.isModuleSelected() && editor.moduleContainer.isMultiControlActive())
    {
        editor.moduleContainer.setSettingsOverlayOnSelectedModules(false, &editor);
    }

    InfoPanelDrawableButton::mouseUp(e);
}



//================== MidiLabel ==========================================================================================================

KrumModuleEditor::MidiLabel::MidiLabel(KrumModuleEditor& editor, juce::String title, juce::String message)
    : moduleEditor(editor), InfoPanelComponent(title, message)
{
    setTooltip("Lane: " + juce::String(moduleEditor.getSamplerIndexString()));
    setRepaintsOnMouseActivity(true);
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

    
    g.setColour(textColor);
    g.setFont(fontSize);

    g.drawFittedText("Note", midiNoteRect.withX(5), juce::Justification::centredLeft, 1);
    
    if (isListeningForMidi())
    {
        g.drawFittedText("Listening...", midiNoteRect.withWidth(area.getWidth() - 10), juce::Justification::centredRight, 1);

        g.setColour(juce::Colours::red);
        g.drawRoundedRectangle(area.toFloat(), cornerSize, EditorDimensions::bigOutline);
    }
    else
    {
        g.drawFittedText(moduleEditor.getModuleMidiNoteString(true), midiNoteRect.withWidth(area.getWidth() - 10), juce::Justification::centredRight, 1);
    }

    //g.drawFittedText("Chan", midiChanRect, juce::Justification::centredLeft, 1);
    //g.drawFittedText(juce::String(moduleEditor->getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);
}

void KrumModuleEditor::MidiLabel::mouseDown(const juce::MouseEvent& e)
{

    if (e.mods.isPopupMenu())
    {
        setListeningForMidi(true);

        if (moduleEditor.isModuleSelected() && moduleEditor.moduleContainer.isMultiControlActive())
        {
            moduleEditor.moduleContainer.setListeningForMidiOnSelectedModules(true, &moduleEditor);
        }

    }
    else
    {
        setListeningForMidi(false);
        if (moduleEditor.isModuleSelected() && moduleEditor.moduleContainer.isMultiControlActive())
        {
            moduleEditor.moduleContainer.setListeningForMidiOnSelectedModules(false, &moduleEditor);
        }
    }
}

void KrumModuleEditor::MidiLabel::mouseUp(const juce::MouseEvent& e)
{

}


bool KrumModuleEditor::MidiLabel::isListeningForMidi()
{
    return listeningForMidi;
}

void KrumModuleEditor::MidiLabel::setListeningForMidi(bool shouldListen)
{
    if (listeningForMidi != shouldListen)
    {
        listeningForMidi = shouldListen;
        repaint();

        if (shouldListen)
        {
            DBG("Set Module Listening for Midi");
        }
        else
        {
            DBG("Set Module NOT Listening for Midi");
        }
    }
}

//================= PitchSlider ===========================================================================================================

KrumModuleEditor::PitchSlider::PitchSlider(KrumModuleEditor& e, juce::String title, juce::String message)
    : editor(e), InfoPanelSlider(title, message)
{
    setSliderStyle(juce::Slider::LinearBarVertical);
    setVelocityModeParameters();
    setDoubleClickReturnValue(true, 0);

}

KrumModuleEditor::PitchSlider::~PitchSlider()
{}


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
    auto bgColor = Colors::getModuleBGColor();
    auto textColor = findColour(juce::Slider::ColourIds::textBoxTextColourId);

    if (isMouseOverOrDragging())
    {
        moduleColor = moduleColor.withAlpha(0.7f);
        textColor = textColor.withAlpha(0.7f);
    }


    g.setColour(moduleColor);
    g.fillRoundedRectangle(bounds, 5.0f);

    InfoPanelSlider::paint(g);

    /*juce::DropShadow dropShadow{ Colors::getDropShadowColor(), 5,{-5, 5} };
    dropShadow.drawForRectangle(g, bounds.toNearestInt());*/


    g.setColour(textColor);
    g.drawFittedText(getTextFromValue(getValue()), bounds.withTop(titleH).reduced(2).toNearestInt(), juce::Justification::centred, 1);

    g.setFont({editor.buttonTextSize + 0.7f});
    g.drawFittedText("Pitch", bounds.withBottom(titleH).toNearestInt(), juce::Justification::centred, 1);
}

void KrumModuleEditor::PitchSlider::mouseEnter(const juce::MouseEvent& e)
{
    
    InfoPanelSlider::mouseEnter(e);
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);

}

void KrumModuleEditor::PitchSlider::mouseExit(const juce::MouseEvent& e)
{
    InfoPanelSlider::mouseExit(e);
    setMouseCursor(juce::MouseCursor::NormalCursor);

}

void KrumModuleEditor::PitchSlider::mouseDown(const juce::MouseEvent& e)
{

   setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
   /*bubbleComp.setVisible(true);
   auto comp = static_cast<juce::Component*>(this);
   bubbleComp.showAt(comp, juce::AttributedString(getTextFromValue(getValue())), 500, false);*/

   InfoPanelSlider::mouseDown(e);
}

void KrumModuleEditor::PitchSlider::mouseDrag(const juce::MouseEvent& e)
{
    /*setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    auto comp = static_cast<juce::Component*>(this);
    bubbleComp.showAt(comp, juce::AttributedString(getTextFromValue(getValue())), 500, false);*/
    InfoPanelSlider::mouseDrag(e);
}

void KrumModuleEditor::PitchSlider::mouseUp(const juce::MouseEvent& e)
{

    //juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(e.source.getLastMouseDownPosition());

    setMouseCursor(juce::MouseCursor::NormalCursor);

    /*bubbleComp.setVisible(false);*/

    InfoPanelSlider::mouseUp(e);
}

//void KrumModuleEditor::PitchSlider::mouseExit(const juce::MouseEvent& e)
//{
//    editor.setPitchSliderVisibility(false);
//}

//=============== MultiControlComboBox =====================================================================================================

KrumModuleEditor::MultiControlComboBox::MultiControlComboBox(KrumModuleEditor& e, juce::String title, juce::String message)
    : editor(e), InfoPanelComboBox(title, message)
{
}

KrumModuleEditor::MultiControlComboBox::~MultiControlComboBox()
{}

void KrumModuleEditor::MultiControlComboBox::mouseDown(const juce::MouseEvent& e)
{
    InfoPanelComboBox::mouseDown(e);

    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedComboAttachments(&editor, this);
    }
}

//void KrumModuleEditor::MultiControlSlider::mouseDrag(const juce::MouseEvent& e)
//{
//    InfoPanelSlider::mouseDrag(e);
//
//}

void KrumModuleEditor::MultiControlComboBox::mouseUp(const juce::MouseEvent& e)
{
    InfoPanelComboBox::mouseUp(e);

    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedComboAttachments(nullptr, this);

    }

}

//=============== MultiControlSlider ==============================================================================================================

KrumModuleEditor::MultiControlSlider::MultiControlSlider(KrumModuleEditor& e, juce::String title, juce::String message)
    : editor(e), InfoPanelSlider(title, message)
{
}

KrumModuleEditor::MultiControlSlider::~MultiControlSlider()
{}

void KrumModuleEditor::MultiControlSlider::mouseDown(const juce::MouseEvent& e)
{
    InfoPanelSlider::mouseDown(e);

    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedSliderAttachments(&editor, this);
    }
}

//void KrumModuleEditor::MultiControlSlider::mouseDrag(const juce::MouseEvent& e)
//{
//    InfoPanelSlider::mouseDrag(e);
//
//}

void KrumModuleEditor::MultiControlSlider::mouseUp(const juce::MouseEvent& e)
{
    InfoPanelSlider::mouseUp(e);
    
    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedSliderAttachments(nullptr, this);

    }

}

//================= CustomToggleButton ======================================================================

KrumModuleEditor::CustomToggleButton::CustomToggleButton(KrumModuleEditor& e, juce::String title, juce::String message)
    : InfoPanelTextButton(title, message), editor(e)
{}

KrumModuleEditor::CustomToggleButton::~CustomToggleButton()
{}

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

void KrumModuleEditor::CustomToggleButton::mouseDown(const juce::MouseEvent& e)
{
    InfoPanelTextButton::mouseDown(e);
    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedButtonAttachments(&editor, this);
    }
}

void KrumModuleEditor::CustomToggleButton::mouseUp(const juce::MouseEvent& e)
{
    InfoPanelTextButton::mouseUp(e);
    if (editor.moduleContainer.isMultiControlActive() && editor.isModuleSelected())
    {
        editor.moduleContainer.reassignSelectedButtonAttachments(nullptr, this);
    }

}

//void KrumModuleEditor::CustomToggleButton::mouseEnter(const juce::MouseEvent& e)
//{
//    //InfoPanelTextButton::mouseEnter(e);
//} 
//
//void KrumModuleEditor::CustomToggleButton::mouseExit(const juce::MouseEvent& e)
//{
//    //InfoPanelTextButton::mouseExit(e);
//}


//=============== DragHandle ==============================================================================================================

KrumModuleEditor::DragHandle::DragHandle(KrumModuleEditor& parentEditor, juce::String title, juce::String message)
    :moduleEditor(parentEditor), InfoPanelDrawableButton( juce::DrawableButton::ImageFitted, title, message)
{}

KrumModuleEditor::DragHandle::~DragHandle()
{}

void KrumModuleEditor::DragHandle::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto area = getLocalBounds();

    if (isMouseOver())
    {
        g.setColour(findColour(juce::DrawableButton::ColourIds::backgroundOnColourId));
    }
    else
    {
        g.setColour(findColour(juce::DrawableButton::ColourIds::backgroundColourId));
    }

    g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);
}


void KrumModuleEditor::DragHandle::mouseDown(const juce::MouseEvent& e)
{
    

    moduleEditor.setAlwaysOnTop(true);

    //moduleEditor.toFront(true);
  
    if (moduleEditor.isModuleSelected() && moduleEditor.moduleContainer.isMultiControlActive())
    {
       // moduleEditor.moduleContainer.startDraggingSelectedEditors(&moduleEditor, e);
    }
    else
    {
        moduleEditor.moduleContainer.startDraggingEditor(&moduleEditor, e);
    }
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    InfoPanelDrawableButton::mouseDown(e);
}

void KrumModuleEditor::DragHandle::mouseDrag(const juce::MouseEvent& e)
{
    //moduleEditor.toFront(false);

    moduleEditor.moduleContainer.dragEditor(&moduleEditor, e);

    

    InfoPanelDrawableButton::mouseDrag(e);
}

void KrumModuleEditor::DragHandle::mouseUp(const juce::MouseEvent& e)
{
    moduleEditor.setAlwaysOnTop(false);
    setMouseCursor(juce::MouseCursor::NormalCursor);
    if (moduleEditor.moduleContainer.isModuleDragging())
    {
        moduleEditor.moduleContainer.draggingMouseUp(e);
    }

    moduleEditor.moduleContainer.setModuleSelectedFromClick(&moduleEditor, !moduleEditor.isModuleSelected(), e);
    InfoPanelDrawableButton::mouseUp(e);
}


