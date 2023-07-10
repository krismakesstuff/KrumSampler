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
#include "../KrumLookAndFeel.h"



ModuleSettingsOverlay::ModuleSettingsOverlay(KrumModuleEditor& parent)
    : parentEditor(parent), colorPalette(*this)
{

    //addChildComponent(cancelButton);
    auto cancelImage = juce::Drawable::createFromImageData(BinaryData::cancel_opsz24_svg, BinaryData::cancel_opsz24_svgSize);
    cancelImage->replaceColour(juce::Colours::black, juce::Colours::white.withAlpha(0.5f));
    addAndMakeVisible(cancelButton);
    cancelButton.setImages(cancelImage.get());
    cancelButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    cancelButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::darkgrey);

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


void ModuleSettingsOverlay::colorButtonClicked(juce::Colour newColor)
{
    /*if (parentEditor.getModuleColor() == Colors::getModuleDefaultColor() && newColor != Colors::getModuleDefaultColor())
    {
        parentEditor.hideSettingsOverlay();
    }*/
    
    if (parentEditor.isModuleSelected() && parentEditor.moduleContainer.isMultiControlActive())
    {
        parentEditor.moduleContainer.setSelectedModulesColor(&parentEditor, newColor);
    }

    parentEditor.setModuleColor(newColor);
    
    

}

void ModuleSettingsOverlay::cancelButtonClicked()
{
    parentEditor.hideSettingsOverlay();

    if (parentEditor.isModuleSelected() && parentEditor.moduleContainer.isMultiControlActive())
    {
        parentEditor.moduleContainer.setSettingsOverlayOnSelectedModules(false, &parentEditor);
    }
}

void ModuleSettingsOverlay::deleteButtonClicked()
{

    if (parentEditor.isModuleSelected() && parentEditor.moduleContainer.isMultiControlActive())
    {
        parentEditor.moduleContainer.setSelectedModulesState(&parentEditor, (int)KrumModule::ModuleState::empty);
    }

    parentEditor.setModuleState(KrumModule::ModuleState::empty);

}


void ModuleSettingsOverlay::visibilityChanged()
{
    //this makes clicking the buttons actually work
    if (isVisible())
    {
        setInterceptsMouseClicks(false, true);
    }
    else
    {
        setInterceptsMouseClicks(false, false);
    }
}
