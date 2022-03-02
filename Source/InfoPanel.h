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
* This InfoPanel has a static instance which is used by all the components that want to display text.
* 
* There are some classes that override their components mouse movements so I don't have to worry about that when handling other functionality in those classes
* There's probably a template way of handling this.. (?)
*/

//static instance
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
    
    juce::Colour fontColor {juce::Colours::grey.brighter(0.4f)};
    //juce::Colour bgColor {juce::Colours::darkgrey.darker(0.99f)};
    juce::Colour bgColor {juce::Colours::black.withAlpha(0.2f)};
    
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

class InfoPanelButton : public juce::Button
{
public:
    InfoPanelButton(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelButton() override;

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

class InfoPanelComboBox : public juce::ComboBox
{
public:
    InfoPanelComboBox(juce::String title, juce::String message, juce::String keycommand = juce::String());
    ~InfoPanelComboBox() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:

    juce::String compTitle, message, keycommand;
};

//For classes that inher
//template<class T>
//class InfoPaneller : public T
//{
//public:
//    InfoPaneller(const char* Title, const char* Message, const char* Keycommand = "") 
//        : compTitle(Title), message(Message), keycommand(Keycommand)
//    {}
//    ~InfoPaneller()
//    {}
//
//    //lets you contextual change this components Info Panel text
//    void setNewPanelMessage(juce::String newTitle, juce::String newMessage, juce::String newKeycommand)
//    {
//        compTitle = newTitle;
//        message = newMessage;
//        keycommand = newKeycommand;
//    }
//
//    void mouseEnter(const juce::MouseEvent& e) override
//    {
//        InfoPanel::shared_instance().setInfoPanelText(compTitle, message, keycommand);
//        T::mouseEnter(e);
//    }
//    void mouseExit(const juce::MouseEvent& e) override
//    {
//        InfoPanel::shared_instance().clearPanelText();
//        T::mouseExit(e);
//    }
//
//private:
//    juce::String compTitle, message, keycommand;
//
//};
