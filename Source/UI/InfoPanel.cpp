/*
  ==============================================================================

    InfoPanel.cpp
    Created: 10 Nov 2021 8:42:25am
    Author:  Kris Crawford

  ==============================================================================
*/

#include "InfoPanel.h"

//==============================================================================
InfoPanel::InfoPanel()
{
}

InfoPanel::~InfoPanel()
{
}

void InfoPanel::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    //float cornerSize = 3.0f;
    //auto panelBG = area/*.reduced(5)*/.toFloat();
    
    if (showText)
    {
        
        auto messageFont = g.getCurrentFont();
        
        //title
        juce::Path titleBG;
        g.setFont(fontSize * 1.05f);
        int titleWidth = g.getCurrentFont().getStringWidthFloat(title);
        
        g.setColour(fontColor);
        g.drawFittedText(title, area.withRight(titleWidth + 5), juce::Justification::centred, 1, 1.0f);
        
        //message
        g.setFont(messageFont.withHeight(fontSize)); 
        g.setColour(fontColor.darker(0.2f));
        g.drawText(message, area.withLeft(titleWidth + 5).withRight(getRight() - 10), juce::Justification::centredLeft, true);
        
        //keycommand
        //g.setFont(messageFont.withStyle(juce::Font::FontStyleFlags::italic).withHeight(numLines * 0.85f));
        //g.drawFittedText(keycommand, panelBG.withTrimmedTop(messageArea.getBottom()).toNearestInt(), juce::Justification::centredLeft, 1);
    }
    
}

void InfoPanel::setInfoPanelText(juce::String compTitle, juce::String newMessage, juce::String newKeycommand)
{
    title = compTitle;
    message = newMessage;
    keycommand = newKeycommand;
    
    showText = true;
    repaint();
}

void InfoPanel::clearPanelText()
{
    title = juce::String{};
    message = juce::String{};
    keycommand = juce::String{};
    
    showText = false;
    repaint();
}

//=============================================================================

InfoPanelComponent::InfoPanelComponent(juce::String title, juce::String newMessage, juce::String newKeycommand)
 : compTitle(title), message(newMessage), keycommand(newKeycommand)
{}

InfoPanelComponent::~InfoPanelComponent() {}

void InfoPanelComponent::mouseEnter(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::Component::mouseEnter(event);
}

void InfoPanelComponent::mouseExit(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::Component::mouseExit(event);
}

void InfoPanelComponent::setNewPanelMessage(juce::String newTitle, juce::String newMessage, juce::String newKeycommand)
{
    compTitle = newTitle;
    message = newMessage;
    keycommand = newKeycommand;

}

//=============================================================================

InfoPanelButton::InfoPanelButton(juce::String title, juce::String newMessage, juce::String newKeycommand)
    : compTitle(title), message(newMessage), keycommand(newKeycommand), juce::Button(juce::String())
{}

InfoPanelButton::~InfoPanelButton()
{}

void InfoPanelButton::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::Button::mouseEnter(e);
}

void InfoPanelButton::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::Button::mouseExit(e);
}

//=============================================================================

InfoPanelDrawableButton::InfoPanelDrawableButton(juce::DrawableButton::ButtonStyle buttonStyle, juce::String title, juce::String newMessage, juce::String newKeycommand)
    : compTitle(title), message(newMessage), keycommand(newKeycommand), juce::DrawableButton(title, buttonStyle)
{}

InfoPanelDrawableButton::~InfoPanelDrawableButton() 
{}

void InfoPanelDrawableButton::mouseEnter(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::DrawableButton::mouseEnter(event);
}

void InfoPanelDrawableButton::mouseExit(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::DrawableButton::mouseExit(event);
}

//=============================================================================

InfoPanelToggleButton::InfoPanelToggleButton(juce::String title, juce::String newMessage, juce::String newKeycommand)
    : compTitle(title), message(newMessage), keycommand(newKeycommand)
{}

InfoPanelToggleButton::~InfoPanelToggleButton() {}

void InfoPanelToggleButton::mouseEnter(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::ToggleButton::mouseEnter(event);
}

void InfoPanelToggleButton::mouseExit(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::ToggleButton::mouseExit(event);
}

//=============================================================================

InfoPanelSlider::InfoPanelSlider(juce::String title, juce::String newMessage, juce::String newKeycommand)
    : compTitle(title), message(newMessage), keycommand(newKeycommand), juce::Slider(title)
{}

InfoPanelSlider::~InfoPanelSlider() {}

void InfoPanelSlider::mouseEnter(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::Slider::mouseEnter(event);
}

void InfoPanelSlider::mouseExit(const juce::MouseEvent& event)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::Slider::mouseExit(event);
}

//=============================================================================

InfoPanelLabel::InfoPanelLabel(juce::String title, juce::String newMessage, juce::String newKeycommand)
: compTitle(title), message(newMessage), keycommand(newKeycommand)
{}

InfoPanelLabel::~InfoPanelLabel() {}

void InfoPanelLabel::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::Label::mouseEnter(e);
}

void InfoPanelLabel::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::Label::mouseExit(e);
}
                              
//=============================================================================

InfoPanelTextButton::InfoPanelTextButton(juce::String title, juce::String m, juce::String kc)
    :compTitle(title), message(m), keycommand(kc), juce::TextButton(title)
{}

InfoPanelTextButton::~InfoPanelTextButton()
{}

void InfoPanelTextButton::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::TextButton::mouseEnter(e);
}

void InfoPanelTextButton::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::TextButton::mouseExit(e);
}

//=============================================================================

InfoPanelComboBox::InfoPanelComboBox(juce::String title, juce::String m, juce::String kc)
    :compTitle(title), message(m), keycommand(kc), juce::ComboBox(title)
{}

InfoPanelComboBox::~InfoPanelComboBox()
{}

void InfoPanelComboBox::mouseEnter(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
    juce::ComboBox::mouseEnter(e);
}

void InfoPanelComboBox::mouseExit(const juce::MouseEvent& e)
{
    InfoPanel::shared_instance().clearPanelText();
    juce::ComboBox::mouseExit(e);
}

//=============================================================================

