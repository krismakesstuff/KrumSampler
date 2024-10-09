/*
  ==============================================================================

    ModuleSettingsOverlay.h
    Created: 6 May 2021 11:58:52am
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ColorPalette.h"
#include "../InfoPanel.h"

class KrumModuleEditor;

/*
* This class represents the overlay that lives on top of the modules when settings are being changed. 
* This is only used to change the color of the modules now. The name ModuleSettingsOverlay is from it's origin of being a settings overlay.
* The KrumModuleEditor::MidiLabel class now handles midi settings.
* TODO: rename this class to reflect it's current use.
* 
*/

class ModuleSettingsOverlay :   public juce::Component/*,
                                public juce::Timer*/
{
public:

    ModuleSettingsOverlay(KrumModuleEditor& parent);
    ~ModuleSettingsOverlay() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void colorButtonClicked(juce::Colour newColor);

    void cancelButtonClicked();
    void deleteButtonClicked();


private:

    
    void visibilityChanged() override;

    InfoPanelDrawableButton deleteButton{juce::DrawableButton::ButtonStyle::ImageFitted, "Delete", "This will delete your module!"};
    InfoPanelDrawableButton cancelButton{juce::DrawableButton::ButtonStyle::ImageFitted, "Cancel", "This will exit the settings overlay without keeping any of the changes" };
    

    ColorPalette colorPalette;
    KrumModuleEditor& parentEditor;
    

    float cornerSize = 5.0f;
    float outlineSize = 1.0f;

    bool keepColorOnExit = false;
    bool colorChanged = false;

    JUCE_LEAK_DETECTOR(ModuleSettingsOverlay)
};
