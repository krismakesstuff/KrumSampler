/*
  ==============================================================================

    ModuleSettingsOverlay.cpp
    Created: 6 May 2021 11:58:52am
    Author:  krisc

  ==============================================================================
*/

#include "ModuleSettingsOverlay.h"
#include "KrumModuleEditor.h"



ModuleSettingsOverlay::ModuleSettingsOverlay(juce::Rectangle<int> area, KrumModuleEditor& parent, bool colorOnly)
    : parentEditor(parent), colorPalette(area.withTop(225), *this, colorOnly), isColorOnly(colorOnly)
{
    setSize(area.getWidth(), area.getHeight());
    setRepaintsOnMouseActivity(true);

    addAndMakeVisible(titleBox);
    titleBox.setText(parentEditor.getModuleName(), juce::dontSendNotification);
    titleBox.setFont(20.0f);
    titleBox.setBounds(getLocalBounds().withBottom(50));
    titleBox.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setEditable(false, true, false);

    addAndMakeVisible(midiNoteNumberLabel);
    midiNoteNumberLabel.setFont({ 50.0f });
    midiNoteNumberLabel.setEditable(false, false, false);
    midiNoteNumberLabel.setBounds(getLocalBounds().withTop(100).withX(10).withHeight(midiNoteNumberLabel.getFont().getHeight()).withWidth(area.reduced(10).getWidth()));
    midiNoteNumberLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiNoteTitleLabel);
    midiNoteTitleLabel.setFont({ 13.0f });
    midiNoteTitleLabel.setEditable(false, false, false);
    midiNoteTitleLabel.setBounds(10, midiNoteNumberLabel.getBottom() - 20, midiNoteNumberLabel.getWidth(), 50);
    midiNoteTitleLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiNoteTitleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiChannelNumberLabel);
    midiChannelNumberLabel.setFont({ 20.0f });
    midiChannelNumberLabel.setEditable(false, false, false);
    midiChannelNumberLabel.setBounds(midiNoteTitleLabel.getX(), midiNoteTitleLabel.getBottom(), midiNoteTitleLabel.getWidth(), midiChannelNumberLabel.getFont().getHeight());
    midiChannelNumberLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(midiChannelTitleLabel);
    midiChannelTitleLabel.setFont({ 13.0f });
    midiChannelTitleLabel.setEditable(false, false, false);
    midiChannelTitleLabel.setBounds(midiChannelNumberLabel.getX(), midiChannelNumberLabel.getBottom(), midiChannelNumberLabel.getWidth(), midiChannelTitleLabel.getFont().getHeight());
    midiChannelTitleLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    midiChannelTitleLabel.setJustificationType(juce::Justification::centred);

    addChildComponent(&colorPalette);
    
    setMidiLabels();
}

ModuleSettingsOverlay::~ModuleSettingsOverlay()
{}

void ModuleSettingsOverlay::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(5);
    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(area.toFloat(), cornerSize);
    
    if (updateMidiLabels)
    {
        setMidiLabels();
        updateMidiLabels = false;

        if (!confirmButton.isVisible())
        {
            showConfirmButton();
        }
    }

    moduleSelectedColor = colorPalette.getSelectedColor();

    g.setColour(moduleSelectedColor);
    
    //g.drawFittedText(parentModule.info.name, area.withTrimmedBottom(area.getHeight() * 0.9f), juce::Justification::centred, 1);
    
    if (!moduleOverlaySelected)
    {
        g.setFont(18.0f);

        g.drawFittedText("Click to Select", area.withTop(area.getBottom() - 75), juce::Justification::centred, 1);
        g.setColour(juce::Colours::darkgrey.darker());
    }
    else if (isColorOnly)
    {
        g.setFont(18.0f);
        g.drawFittedText("Select A New Color", area.withTop(area.getBottom() - 75), juce::Justification::centred, 1);
    }
    else
    {
        g.setFont(18.0f);
        g.drawFittedText("Select Midi Note", area.withTop(area.getBottom() - 75), juce::Justification::centred, 1);
    }

    g.drawRoundedRectangle(area.toFloat(), cornerSize, outlineSize);
}

void ModuleSettingsOverlay::handleMidiInput(int midiChannelNumber, int midiNoteNumber)
{
    if (moduleOverlaySelected && !isColorOnly)
    {
        setMidi(midiNoteNumber, midiChannelNumber);
    }
}



void ModuleSettingsOverlay::setOverlaySelected(bool isSelected)
{
    if (isSelected && isVisible())
    {
        showButtons();
    }
    else
    {
        hideButtons();
    }
    moduleOverlaySelected = isSelected;
}

bool ModuleSettingsOverlay::isOverlaySelected()
{
    return moduleOverlaySelected;
}

void ModuleSettingsOverlay::confirmMidi()
{
    juce::Colour color;

    //this logic works out the context of leaving the moduleSettingsOverlay
    if (colorChanged)
    {
        color = colorPalette.getSelectedColor();
        parentEditor.setModuleColor(color);
    }
    else if (!keepColorOnExit) //when we make a new module but don't select a new color, we grab a random one, 
                               //but if we have set the color previously, just not this time, then we don't want to set the module with a random color 
    {
        color = colorPalette.getRandomColor();
        parentEditor.setModuleColor(color);
    }
    
    bool removeOld = parentEditor.getModuleMidiNote() > 0;
    juce::String text = titleBox.getText(true);

    parentEditor.setModuleMidiNote(midiNoteNum);
    parentEditor.setModuleMidiChannel(midiChanNum);
    parentEditor.setModuleName(text);
    parentEditor.removeSettingsOverlay(true);
}

void ModuleSettingsOverlay::showConfirmButton() 
{
    addAndMakeVisible(confirmButton);
    auto area = getLocalBounds();
    int buttonWidth = area.getWidth() / 1.25;
    int buttonHeight = 35;

    confirmButton.setBounds(area.getCentreX() - buttonWidth / 2, area.getCentreY() + buttonHeight * 3.75, buttonWidth, buttonHeight);
    confirmButton.setButtonText("Confirm");
    confirmButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    confirmButton.onClick = [this] { confirmMidi(); };
    showingConfirmButton = true;
    repaint();
}

void ModuleSettingsOverlay::showButtons()
{
    setInterceptsMouseClicks(false, true);
    addAndMakeVisible(colorPalette);
    addAndMakeVisible(cancelButton);

    auto area = getLocalBounds();
    int cancelButtonWidth = area.getWidth() / 2;
    int cancelButtonHeight = 30;

    cancelButton.setButtonText("Cancel");
    cancelButton.setBounds(area.getCentreX() - cancelButtonWidth / 2, area.getBottom() - (cancelButtonHeight * 2) - 50, cancelButtonWidth, cancelButtonHeight);
    cancelButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    cancelButton.onClick = [this] { cancelSettings(); };

    if (showingConfirmButton && !confirmButton.isVisible())
    {
        confirmButton.setVisible(true);
    }
}

void ModuleSettingsOverlay::cancelSettings()
{
    if (parentEditor.getModuleMidiNote() > 0)
    {
        parentEditor.removeSettingsOverlay(false);
        DBG("Removed Settings");
    }
    else
    {
        parentEditor.setModuleState(0); // 0 is empty module state
        DBG("Deleted");
    }

}

void ModuleSettingsOverlay::hideButtons()
{
    setInterceptsMouseClicks(false, false);
    removeChildComponent(&colorPalette);
    removeChildComponent(&deleteButton);
    removeChildComponent(&cancelButton);
    if (confirmButton.isVisible())
    {
        showingConfirmButton = true;
        confirmButton.setVisible(false);
    }

}

juce::Colour ModuleSettingsOverlay::getSelectedColor()
{
    return moduleSelectedColor;
}

bool ModuleSettingsOverlay::isModuleOverlaySelected()
{
    return moduleOverlaySelected;
}

void ModuleSettingsOverlay::setMidi(int midiNote, int midiChannel)
{
    midiNoteNum = midiNote;
    midiChanNum = midiChannel;
    updateMidiLabels = true;
}

void ModuleSettingsOverlay::setMidiLabels()
{
    juce::String midiNoteString; 
    juce::String midiChanString; 

    if (midiNoteNum == 0)
    {
        midiNoteString = "None Selected";
        midiChanString = "None Selected";
        midiNoteNumberLabel.setFont(20.0f);
    }
    else
    {
        midiNoteString = juce::String(juce::MidiMessage::getMidiNoteName(midiNoteNum, true, true, 3));
        midiChanString = juce::String(midiChanNum);
        midiNoteNumberLabel.setFont(50.0f);
    }

    midiNoteNumberLabel.setText(midiNoteString, juce::dontSendNotification);
    midiChannelNumberLabel.setText(midiChanString, juce::dontSendNotification);
    
    if(isColorOnly)
    {
        midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::darkgrey);
        midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::darkgrey);
    }
}

bool ModuleSettingsOverlay::hasMidi()
{
    return midiNoteNum > 0;
}

void ModuleSettingsOverlay::keepCurrentColor(bool keepColor)
{
    keepColorOnExit = keepColor;
}

void ModuleSettingsOverlay::colorWasChanged(bool colorWasChanged)
{
    colorChanged = colorWasChanged;
    midiNoteNumberLabel.setColour(juce::Label::ColourIds::textColourId, colorPalette.getSelectedColor());
    midiChannelNumberLabel.setColour(juce::Label::ColourIds::textColourId, colorPalette.getSelectedColor());
}

void ModuleSettingsOverlay::setToOnlyShowColors(bool onlyShowColors)
{
    isColorOnly = onlyShowColors;
    colorPalette.setVisible(isColorOnly);
}
