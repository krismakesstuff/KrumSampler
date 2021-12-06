/*
  ==============================================================================

    InfoPanel.cpp
    Created: 10 Nov 2021 8:42:25am
    Author:  Kris Crawford

  ==============================================================================
*/

#include <JuceHeader.h>
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
    float cornerSize = 3.0f;
    auto panelBG = area/*.reduced(5)*/.toFloat();
    
    //bg fill
    g.setColour(bgColor);
    g.fillRoundedRectangle(panelBG, cornerSize);
    
//    //outline
//    g.setColour(fontColor.withAlpha(0.5f));
//    g.drawRoundedRectangle(panelBG, cornerSize, 1.0f);
    
    if (showText)
    {
        //g.setColour(juce::Colours::darkgrey.darker());
        //g.fillRoundedRectangle(area.toFloat(), 3.0f);

        auto messageFont = g.getCurrentFont();
        
        //title
        float titleTextH = panelBG.getHeight() / 4;
        int numLines = panelBG.getHeight() / 5;
        juce::Path titleBG;
        titleBG.addRoundedRectangle(panelBG.getX() + 5, panelBG.getY(), panelBG.getWidth(), titleTextH, cornerSize, cornerSize, true, true, false, false);

//        g.setColour(bgColor.brighter(0.2f));
//        g.fillPath(titleBG);

        g.setColour(fontColor);
        g.setFont(messageFont.withStyle(juce::Font::FontStyleFlags::bold).withHeight(titleTextH));
        g.drawFittedText(title, titleBG.getBounds().toNearestInt(), juce::Justification::centredLeft, 1);
        
        //message
        juce::Rectangle<int> messageArea = panelBG.withTrimmedTop(titleBG.getBounds().getHeight()).withTrimmedLeft(5).toNearestInt();
        g.setFont(messageFont.withStyle(juce::Font::FontStyleFlags::plain).withHeight(17.0f)); 
        g.drawFittedText(message, messageArea, juce::Justification::centredLeft, numLines);
        
        //keycommand
        g.setFont(messageFont.withStyle(juce::Font::FontStyleFlags::italic).withHeight(numLines * 0.85f));
        g.drawFittedText(keycommand, panelBG.withTrimmedTop(messageArea.getBottom()).toNearestInt(), juce::Justification::centredLeft, 1);
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
InfoPanelDrawableButton::InfoPanelDrawableButton(juce::String title, juce::String newMessage, juce::String newKeycommand,  juce::DrawableButton::ButtonStyle buttonStyle)
    : compTitle(title), message(newMessage), keycommand(newKeycommand), juce::DrawableButton(title, buttonStyle)
{}

InfoPanelDrawableButton::~InfoPanelDrawableButton() {}

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
    : compTitle(title), message(newMessage), keycommand(newKeycommand)
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
                                                         
InfoPanelTextButton::InfoPanelTextButton(juce::String title, juce::String m, juce::String kc)
    :compTitle(title), message(m), keycommand(kc)
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
