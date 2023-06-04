/*
  ==============================================================================

    ModuleSettingsOverlay.cpp
    Created: 6 May 2021 11:58:52am
    Author:  krisc

  ==============================================================================
*/

#include "ModuleSettingsOverlay.h"
#include "KrumModuleEditor.h"
#include "../KrumModuleContainer.h"
#include "../Source/KrumModule.h"



ModuleSettingsOverlay::ModuleSettingsOverlay(KrumModuleEditor& parent)
    : parentEditor(parent), colorPalette(*this)
{

    /*addAndMakeVisible(titleBox);
    titleBox.setText(parentEditor.getModuleName(), juce::dontSendNotification);
    titleBox.setFont(18.0f);
    titleBox.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    titleBox.setColour(juce::Label::ColourIds::textWhenEditingColourId, juce::Colours::black);
    titleBox.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, juce::Colours::grey);
    titleBox.setColour(juce::TextEditor::ColourIds::highlightColourId, juce::Colours::lightgrey);
    titleBox.setColour(juce::TextEditor::ColourIds::highlightedTextColourId, juce::Colours::black);
    titleBox.setColour(juce::CaretComponent::ColourIds::caretColourId, juce::Colours::black);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setEditable(false, true, false);

    addAndMakeVisible(midiNoteNumberLabel);
    midiNoteNumberLabel.setFont({ 40.0f });
    midiNoteNumberLabel.setEditable(false, false, false);
    midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiNoteNumberLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiNoteTitleLabel);
    midiNoteTitleLabel.setText("Midi Note", juce::dontSendNotification);
    midiNoteTitleLabel.setFont({ 13.0f });
    midiNoteTitleLabel.setEditable(false, false, false);
    midiNoteTitleLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiNoteTitleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiChannelNumberLabel);
    midiChannelNumberLabel.setFont({ 20.0f });
    midiChannelNumberLabel.setEditable(false, false, false);
    midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiChannelNumberLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiChannelTitleLabel);
    midiChannelTitleLabel.setText("Midi Channel", juce::dontSendNotification);
    midiChannelTitleLabel.setFont({ 13.0f });
    midiChannelTitleLabel.setEditable(false, false, false);
    midiChannelTitleLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiChannelTitleLabel.setJustificationType(juce::Justification::centred);

    addChildComponent(midiListenButton);
    midiListenButton.setButtonText("Midi Listen");
    midiListenButton.setClickingTogglesState(true);
    midiListenButton.setToggleState(false, juce::dontSendNotification);
    midiListenButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    midiListenButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    midiListenButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    midiListenButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkred);
    midiListenButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::darkgrey);
    midiListenButton.onClick = [this] { midiListenButtonClicked(); };*/

    //addChildComponent(cancelButton);
    auto cancelImage = juce::Drawable::createFromImageData(BinaryData::cancel_opsz24_svg, BinaryData::cancel_opsz24_svgSize);
    cancelImage->replaceColour(juce::Colours::black, juce::Colours::white.withAlpha(0.5f));
    addAndMakeVisible(cancelButton);
    cancelButton.setImages(cancelImage.get());
    cancelButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    cancelButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::darkgrey);

    /*cancelButton.setButtonText("Cancel");
    cancelButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    cancelButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::darkgrey);*/
    cancelButton.onClick = [this] { cancelButtonClicked(); };

    auto deleteImage = juce::Drawable::createFromImageData(BinaryData::trash_wgt200_svg, BinaryData::trash_wgt200_svgSize);
    deleteImage->replaceColour(juce::Colours::black, juce::Colours::white.withAlpha(0.5f));
    addAndMakeVisible(deleteButton);
    deleteButton.setImages(deleteImage.get());
    deleteButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    deleteButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::red.darker(0.2f));
    deleteButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::red.darker());
    /*deleteButton.setButtonText("Delete");
    deleteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred.darker());
    deleteButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::darkgrey);*/
    deleteButton.onClick = [this] { deleteButtonClicked(); };
   

    /*addChildComponent(confirmButton);
    confirmButton.setButtonText("Confirm");
    confirmButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    confirmButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::darkgrey);
    confirmButton.onClick = [this] { confirmButtonClicked(); };*/
    

    //addChildComponent(&colorPalette);
    addAndMakeVisible(colorPalette);

    
    //setMidiLabels();

    setRepaintsOnMouseActivity(true);
    //startTimerHz(30);
}

ModuleSettingsOverlay::~ModuleSettingsOverlay()
{}

void ModuleSettingsOverlay::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.fillRoundedRectangle(area.toFloat(), cornerSize);

    /*juce::Rectangle<int> botTextArea = area.withTop(area.getBottom() - 75).reduced(3);

    moduleSelectedColor = colorPalette.getSelectedColor();

    g.setColour(moduleSelectedColor);

    if (!moduleOverlaySelected)
    {
        g.setFont(18.0f);

        g.drawFittedText("Click to Select", botTextArea, juce::Justification::centred, 1);
        g.setColour(juce::Colours::darkgrey.darker());
    }

    g.drawRoundedRectangle(area.toFloat(), cornerSize, outlineSize);*/
}

void ModuleSettingsOverlay::resized()
{
    auto area = getLocalBounds();

    int spacer = 5;
    int titleBoxH = area.getHeight() * 0.11f;
    int midiListenButtonH = area.getHeight() * 0.056f;
    int paletteH = area.getHeight() * 0.28; 
    int noteNumberH = area.getHeight() * 0.145f;
    int noteTitleH = area.getHeight() * 0.078f;

    int buttonWidth = area.getWidth() * 0.325f;
    int buttonHeight = area.getHeight() * 0.275f;

    /*int confirmButtonWidth = area.getWidth() / 1.25;
    int confirmButtonHeight = area.getHeight() * 0.078f; 

    int cancelButtonWidth = (area.getWidth() - (spacer * 2)) / 2;
    int cancelButtonHeight = area.getHeight() * 0.3f;*/

    /*titleBox.setBounds(area.withBottom(titleBoxH));

    midiNoteNumberLabel.setBounds(area.getX(), titleBox.getBottom() + (spacer * 5), area.getWidth(), noteNumberH);
    midiNoteTitleLabel.setBounds(area.getX(), midiNoteNumberLabel.getBottom() + spacer, midiNoteNumberLabel.getWidth(), noteTitleH);

    midiChannelNumberLabel.setBounds(midiNoteTitleLabel.getX(), midiNoteTitleLabel.getBottom(), midiNoteTitleLabel.getWidth(), midiChannelNumberLabel.getFont().getHeight());
    midiChannelTitleLabel.setBounds(midiChannelNumberLabel.getX(), midiChannelNumberLabel.getBottom() + spacer, midiChannelNumberLabel.getWidth(), midiChannelTitleLabel.getFont().getHeight());
    
    midiListenButton.setBounds(area.getX() + spacer, midiChannelTitleLabel.getBottom() + (spacer * 3), area.getWidth() - (spacer * 2), midiListenButtonH);*/

    colorPalette.setBounds(area.getX(), area.getY() - spacer, area.getWidth(), area.getHeight() - buttonHeight);

//    confirmButton.setBounds(area.getCentreX() - confirmButtonWidth / 2, colorPalette.getBottom() + (spacer * 2), confirmButtonWidth, confirmButtonHeight);
    deleteButton.setBounds(area.getX() + spacer, colorPalette.getBottom()/* + (spacer * 2)*/, buttonWidth - spacer, buttonHeight);
    cancelButton.setBounds(deleteButton.getRight() + spacer, deleteButton.getY(), buttonWidth - spacer, buttonHeight);

}

void ModuleSettingsOverlay::mouseEnter(const juce::MouseEvent& e)
{
    //Probably could get rid of this but I think I might use them.. 
    //InfoPanel::shared_instance().setInfoPanelText("Settings Overlay", "To assign a new midi note, enable midi listen, and play/click your note. You can also change the color or delte the module");
    juce::Component::mouseEnter(e);
}

void ModuleSettingsOverlay::mouseExit(const juce::MouseEvent& e)
{
    //Probably could get rid of this but I think I might use them.. 
    //InfoPanel::shared_instance().clearPanelText();
    juce::Component::mouseExit(e);
}

//void ModuleSettingsOverlay::handleMidiInput(int midiChannelNumber, int midiNoteNumber)
//{
//    if (moduleOverlaySelected && midiListenButton.getToggleState())
//    {
//        setMidi(midiNoteNumber, midiChannelNumber);
//    }
//}
//
//void ModuleSettingsOverlay::setOverlaySelected(bool isSelected)
//{
//    if (isSelected && isVisible())
//    {
//        showButtons();
//    }
//    else
//    {
//        hideButtons();
//    }
//    moduleOverlaySelected = isSelected;
//}
//
//bool ModuleSettingsOverlay::isOverlaySelected()
//{
//    return moduleOverlaySelected;
//}
//
//void ModuleSettingsOverlay::confirmButtonClicked()
//{
//    juce::Colour color;
//
//    //this logic works out the context of leaving the moduleSettingsOverlay.
//    //essentially we only want to change moduleColor if a new one has been selected or if the module was just made, then we get a random color
//    
//    juce::String name = titleBox.getText(true); //compiler reasons
//    parentEditor.setModuleName(name);
//    
//    if (parentEditor.getModuleState() != KrumModule::ModuleState::active)
//    {
//        parentEditor.setModuleState(KrumModule::ModuleState::active); //important to set the state before setting the color and midiNote
//    }
//    
//    if (colorChanged)
//    {
//        color = colorPalette.getSelectedColor();
//        parentEditor.setModuleColor(color);
//    }
//    else if (!keepColorOnExit) //when we make a new module but don't select a new color, we grab a random one,
//    {
//        color = colorPalette.getRandomColor();
//        parentEditor.setModuleColor(color);
//    }
//
//    parentEditor.setModuleMidiNote(midiNoteNum);
//    parentEditor.setModuleMidiChannel(midiChanNum);
//    parentEditor.removeSettingsOverlay(true);
//}
//
//void ModuleSettingsOverlay::showConfirmButton() 
//{
//    confirmButton.setVisible(true);
//    showingConfirmButton = true;
//}

//void ModuleSettingsOverlay::showButtons()
//{
//    setInterceptsMouseClicks(false, true);
//    setButtonVisibilities(true);
//
//    if (showingConfirmButton && !confirmButton.isVisible())
//    {
//        confirmButton.setVisible(true);
//    }
//}
//
//void ModuleSettingsOverlay::hideButtons()
//{
//    setInterceptsMouseClicks(false, false);
//    setButtonVisibilities(false);
//
//    if (confirmButton.isVisible())
//    {
//        showingConfirmButton = true; //if the confirm button was visible before we hide, we set a flag to remeber to show it when the module gets re-selected
//        confirmButton.setVisible(false);
//    }
//}

void ModuleSettingsOverlay::colorButtonClicked(juce::Colour newColor)
{
    parentEditor.setModuleColor(newColor);
}

void ModuleSettingsOverlay::cancelButtonClicked()
{
    parentEditor.removeSettingsOverlay();

    if (parentEditor.isModuleSelected() && parentEditor.moduleContainer.isMultiControlActive())
    {
        parentEditor.moduleContainer.setSettingsOverlayOnSelectedModules(false, &parentEditor);
    }
}

void ModuleSettingsOverlay::deleteButtonClicked()
{
    parentEditor.setModuleState(KrumModule::ModuleState::empty);
}

//
//juce::Colour ModuleSettingsOverlay::getSelectedColor()
//{
//    return moduleSelectedColor;
//}
//
//bool ModuleSettingsOverlay::isModuleOverlaySelected()
//{
//    return moduleOverlaySelected;
//}

//void ModuleSettingsOverlay::setMidiListen(bool shouldListen)
//{
//    midiListenButton.setToggleState(shouldListen, juce::sendNotification);
//    midiListenButtonClicked();
//}
//
//void ModuleSettingsOverlay::setMidi(int midiNote, int midiChannel)
//{
//    midiNoteNum = midiNote;
//    midiChanNum = midiChannel;
//    updateMidiLabels = true;
//}
//
//void ModuleSettingsOverlay::setMidiLabels()
//{
//    juce::String midiNoteString; 
//    juce::String midiChanString; 
//
//    if (midiNoteNum == 0)
//    {
//        midiNoteString = "None";
//        midiChanString = "None";
//        midiNoteNumberLabel.setFont(20.0f);
//    }
//    else
//    {
//        midiNoteString = juce::String(juce::MidiMessage::getMidiNoteName(midiNoteNum, true, true, 3));
//        midiChanString = juce::String(midiChanNum);
//        midiNoteNumberLabel.setFont(50.0f);
//
//        if (midiNoteNum != parentEditor.getModuleMidiNote() || midiChanNum != parentEditor.getModuleMidiChannel())
//        {
//            showConfirmButton();
//        }
//    }
//
//    midiNoteNumberLabel.setText(midiNoteString, juce::dontSendNotification);
//    midiChannelNumberLabel.setText(midiChanString, juce::dontSendNotification);
//  
//    setMidiLabelColors();
//    updateMidiLabels = false;
//}
//
//bool ModuleSettingsOverlay::hasMidi()
//{
//    return midiNoteNum > 0;
//}
//
//void ModuleSettingsOverlay::setTitle(juce::String& newTitle)
//{
//    titleBox.setText(newTitle, juce::dontSendNotification);
//}

//void ModuleSettingsOverlay::keepCurrentColor(bool keepColor)
//{
//    keepColorOnExit = keepColor;
//}
//
//void ModuleSettingsOverlay::colorWasChanged(bool colorWasChanged)
//{
//    colorChanged = colorWasChanged;
//    midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, colorPalette.getSelectedColor());
//    midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, colorPalette.getSelectedColor());
//}

//void ModuleSettingsOverlay::timerCallback()
//{
//    if (updateMidiLabels)
//    {
//        setMidiLabels();
//    }
//
//    repaint();
//}

//void ModuleSettingsOverlay::midiListenButtonClicked()
//{
//    setMidiLabelColors();
//}
//
//void ModuleSettingsOverlay::setButtonVisibilities(bool shouldBeVisible)
//{
//    colorPalette.setVisible(shouldBeVisible);
//    cancelButton.setVisible(shouldBeVisible);
//    deleteButton.setVisible(shouldBeVisible);
//    midiListenButton.setVisible(shouldBeVisible);
//}

//void ModuleSettingsOverlay::setMidiLabelColors()
//{
//    if (midiListenButton.getToggleState())
//    {
//        auto color = juce::Colours::white;
//            
//        midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, color);
//        midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, color);
//    }
//    else
//    {
//        midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::darkgrey);
//        midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::darkgrey);
//    }
//}
//
void ModuleSettingsOverlay::visibilityChanged()
{
    if (isVisible())
    {
        setInterceptsMouseClicks(false, true);
    }
    else
    {
        setInterceptsMouseClicks(false, false);
    }
}
