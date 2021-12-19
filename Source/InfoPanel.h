/*
  ==============================================================================

    InfoPanel.h
    Created: 10 Nov 2021 8:42:25am
    Author:  Kris Crawford

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/


class InfoPanel  : public juce::Component
{
public:

    static InfoPanel& shared_instance()
    {
        static InfoPanel infoPanel;
        return infoPanel;
    }

    void setInfoPanelText(juce::String compTitle, juce::String message, juce::String keycommand = juce::String{});
    void clearPanelText();
    
private:
    
    InfoPanel();
    ~InfoPanel() override;
    
    void paint (juce::Graphics&) override;
    
    bool showText = false;
    
    juce::String message;
    juce::String title;
    juce::String keycommand;
    
    juce::Colour fontColor {juce::Colours::grey.brighter(0.2f)};
    juce::Colour bgColor {juce::Colours::black};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoPanel)
};

//==============================================================================

class InfoPanelComponent : public juce::Component
{
public:
    InfoPanelComponent(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelComponent() override;
    
    //a work-around for components to set a new message, for various situations
    void setNewPanelMessage(juce::String newTitle = juce::String(), juce::String newMessage = juce::String(), juce::String newKeyCommand = juce::String());

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:

    juce::String compTitle, message, keycommand;
};
//==============================================================================
class InfoPanelTextButton : public juce::TextButton
{
public:
    InfoPanelTextButton(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelTextButton() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:
    juce::String compTitle, message, keycommand;
};


//==============================================================================

class InfoPanelDrawableButton : public juce::DrawableButton
{
public:
    InfoPanelDrawableButton(juce::String title, juce::String message, juce::String keycommand = juce::String(), juce::DrawableButton::ButtonStyle buttonStyle = juce::DrawableButton::ButtonStyle::ImageStretched);
    ~InfoPanelDrawableButton() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:
    
    juce::String compTitle, message, keycommand;
};
//==============================================================================

class InfoPanelToggleButton : public juce::ToggleButton
{
public:
    InfoPanelToggleButton(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelToggleButton() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:
    
    juce::String compTitle, message, keycommand;
    
};
//==============================================================================

class InfoPanelSlider : public juce::Slider
{
public:
    InfoPanelSlider(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelSlider() override;
    
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:

    juce::String compTitle, message, keycommand;
};
//==============================================================================

class InfoPanelLabel : public juce::Label
{
public:
    InfoPanelLabel(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelLabel() override;
    
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
private:

    juce::String compTitle, message, keycommand;
};
