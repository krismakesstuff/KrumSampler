/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "KrumFileBrowser.h"


//==============================================================================
KrumSamplerAudioProcessorEditor::KrumSamplerAudioProcessorEditor (KrumSamplerAudioProcessor& p, KrumSampler& s, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& valTree, juce::ValueTree& fileBrowserTree)
    : AudioProcessorEditor (&p), audioProcessor (p), sampler(s), parameters(apvts), fileBrowser(audioProcessor.getFileBrowser()), valueTree(valTree)
{
    
    titleImage = juce::ImageFileFormat::loadFrom(BinaryData::KrumSamplerTitle_png, BinaryData::KrumSamplerTitle_pngSize);

    auto& laf = getLookAndFeel();
    laf.setDefaultLookAndFeel(&kLaf);

    toolTipWindow->setMillisecondsBeforeTipAppears(2000);

    addAndMakeVisible(websiteButton);
    websiteButton.setButtonText(websiteURL.getDomain());
    websiteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    websiteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::grey.brighter(0.2f));
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::grey.brighter(0.2f));
    websiteButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    websiteButton.onClick = [this] { websiteURL.launchInDefaultBrowser(); };

    addAndMakeVisible(outputGainSlider);
    
    outputGainSlider.setScrollWheelEnabled(false);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 10, 10);
    outputGainSlider.setColour(juce::Slider::ColourIds::thumbColourId, outputThumbColor);
    outputGainSlider.setColour(juce::Slider::ColourIds::trackColourId, outputTrackColor);
    outputGainSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, outlineColor);
    
    outputGainSlider.setNumDecimalPlacesToDisplay(2);
    outputGainSlider.setPopupDisplayEnabled(true, false, this);
    outputGainSlider.setTooltip(outputGainSlider.getTextFromValue(outputGainSlider.getValue()));
    outputGainSlider.onValueChange = [this] { updateOutputGainBubbleComp(outputGainSlider.getCurrentPopupDisplay()); repaint(); };
    
    outputGainAttachment.reset(new SliderAttachment(parameters, TreeIDs::outputGainParam.toString(), outputGainSlider));
    
    addAndMakeVisible(keyboard);
    
    addAndMakeVisible(modulesViewport);
    
    modulesViewport.setViewedComponent(&moduleContainer);
    modulesViewport.setSingleStepSizes(10, 10);
    modulesViewport.setInterceptsMouseClicks(true, true);
    modulesViewport.setScrollBarsShown(false, true, false, false);
    
    fileBrowser.assignModuleContainer(&moduleContainer);
    addAndMakeVisible(InfoPanel::shared_instance());
    addAndMakeVisible(fileBrowser);
    

    auto collapseLeftIm = juce::Drawable::createFromImageData(BinaryData::chevron_left_black_24dp_svg, BinaryData::chevron_left_black_24dp_svgSize);
    auto collapseRightIm = juce::Drawable::createFromImageData(BinaryData::chevron_right_black_24dp_svg, BinaryData::chevron_right_black_24dp_svgSize);

    collapseBrowserButton.setImages(collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(),
                                    collapseLeftIm.get());

    collapseBrowserButton.setClickingTogglesState(true);
    collapseBrowserButton.setToggleState(getSavedFileBrowserHiddenState(), juce::sendNotification);
    collapseBrowserButton.onClick = [this] { collapseBrowserButton.getToggleState() ? hideFileBrowser() : showFileBrowser(); };
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::darkgrey.darker());
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::darkgrey);
    collapseBrowserButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(collapseBrowserButton);

    auto infoOffIm = juce::Drawable::createFromImageData(BinaryData::info_white_24dp_svg, BinaryData::info_white_24dp_svgSize);
    auto infoOnIm = juce::Drawable::createFromImageData(BinaryData::info_white_filled_24dp_svg, BinaryData::info_white_filled_24dp_svgSize);
    
    infoButton.setImages(infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOnIm.get());
    infoButton.setClickingTogglesState(true);
    infoButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    infoButton.onClick = [this] { infoButtonClicked();  };

    addAndMakeVisible(infoButton);

    moduleContainer.createModuleEditors();
    moduleContainer.showFirstEmptyModule();
    
    keyboard.scrollToKey(keyboard.getLowestKey());
    keyboard.repaint();

    fileBrowser.buildDemoKit();
    
    setPaintingIsUnclipped(true);
    
    if (getSavedInfoButtonState())
    {
        infoButton.setToggleState(true, juce::sendNotification); //a button's toggle state is false by default and wont call lambda if the state passed in is the same as the current one
    }
    else
    {
        infoButtonClicked();
    }

    if (collapseBrowserButton.getToggleState())
    {
        //Browser is Hidden
        setSize(EditorDimensions::windowWNoBrowser, EditorDimensions::windowH);
    }
    else
    {
        //Browser is Visible
        setSize (EditorDimensions::windowW, EditorDimensions::windowH);
    }
}

KrumSamplerAudioProcessorEditor::~KrumSamplerAudioProcessorEditor()
{
}

//==============================================================================

void KrumSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    float gain = outputGainSlider.valueToProportionOfLength(getOutputGainValue());
    auto outputGainProp = 1 - parameters.getParameter(TreeIDs::outputGainParam)->convertTo0to1(gain);

    auto bgGrade = juce::ColourGradient::vertical(bgColor, bgColor.brighter(0.1f), area);
    bgGrade.addColour(juce::jlimit<double>(0.00001, 0.9999, outputGainProp), bgColor.brighter(0.077f));

    g.setGradientFill(bgGrade);
    g.fillRect(area);
    
    auto titleRect = area.withHeight(EditorDimensions::topBar).withWidth(EditorDimensions::titleImageW).withCentre({area.getCentreX(), EditorDimensions::topBar / 2}).toFloat();

    g.drawImage(titleImage, titleRect.toFloat(), juce::RectanglePlacement::xMid );

    auto moduleBGGrade = juce::ColourGradient::vertical(modulesBGColor.darker(0.2f), modulesBGColor, modulesBG);

    //g.setGradientFill(moduleBGGrade);
    //g.fillRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize);

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(modulesViewport.getBounds().toFloat(), EditorDimensions::cornerSize);
    
    g.setColour(juce::Colours::grey.brighter(0.2f));
    g.drawFittedText(madeByString, EditorDimensions::madeByArea.withX(area.getRight() - (EditorDimensions::madeByArea.getWidth() + 10)).withY(area.getY()), juce::Justification::centredRight, 1);
    //g.drawRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);

    //g.setColour(backOutlineColor);
    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.drawRoundedRectangle(outputGainSlider.getBounds().withTop(modulesBG.getY() ).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    
    if (!collapseBrowserButton.getToggleState())
    {
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(fileBrowser.getBounds().withY(fileBrowser.getBounds().getY()).withBottom(fileBrowser.getBounds().getBottom()).expanded(EditorDimensions::extraShrinkage(1), EditorDimensions::extraShrinkage(1)).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    }
    
    g.setColour(mainFontColor);
    g.setFont(22.0f);
    g.drawFittedText("Output", area.withTop(modulesBG.getY()).withLeft(modulesBG.getRight()).withRight(outputGainSlider.getRight()).withBottom(outputGainSlider.getY()),
                    juce::Justification::centred,1);

    //juce::Rectangle<int> linesBounds{ outputGainSlider.getBoundsInParent().withWidth(outputGainSlider.getWidth() - 10).withTrimmedLeft(10).withTrimmedTop(21).withTrimmedBottom(5) };

    
    //Version
    g.setColour(juce::Colours::red.darker());
    g.setFont({ 17 });
    juce::String versionString = "Build Version: " + juce::String(KRUM_BUILD_VERSION);
    int versionW = g.getCurrentFont().getStringWidth(versionString);
    g.drawFittedText(versionString, { area.getRight() - 160, websiteButton.getBottom() - 12, versionW + 10, 35 }, juce::Justification::centred, 1);
}

void KrumSamplerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int infoButtonSize = 21;
    
    websiteButton.setBounds(EditorDimensions::madeByArea.withX(area.getRight() - (EditorDimensions::madeByArea.getWidth() + 10)).withY(area.getY() + (EditorDimensions::madeByArea.getHeight() / 2) + 5).withHeight(EditorDimensions::madeByArea.getHeight() * 0.75f).withWidth(EditorDimensions::madeByArea.getWidth() + 10));
    infoButton.setBounds(area.getRight() - (websiteButton.getWidth() + (infoButtonSize * 2)), EditorDimensions::madeByArea.getY() + 20, infoButtonSize, infoButtonSize);
    
    if (!collapseBrowserButton.getToggleState())
    {
        //File Browser is Visible
        InfoPanel::shared_instance().setBounds(area.getX() + EditorDimensions::shrinkage, area.getY() + EditorDimensions::shrinkage, EditorDimensions::fileTreeW, EditorDimensions::topBar);
        modulesBG = area.withTop(EditorDimensions::topBar).withLeft(area.getX()  + EditorDimensions::fileTreeW).withRight(area.getRight() - EditorDimensions::outputW).reduced(EditorDimensions::extraShrinkage());
        fileBrowser.setBounds(area.withTop(EditorDimensions::topBar).withRight(modulesBG.getX()).reduced(EditorDimensions::extraShrinkage(3)));
    }
    else
    {
        //File Browser is Hidden
        modulesBG = area.withTop(EditorDimensions::topBar).withRight(area.getRight() - EditorDimensions::outputW).reduced(EditorDimensions::extraShrinkage());
    }

    modulesViewport.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH).withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
    moduleContainer.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH).withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
    moduleContainer.refreshModuleLayout();

    outputGainSlider.setBounds(area.withTop(EditorDimensions::extraShrinkage(20)).withBottom(modulesBG.getBottom()).withLeft(modulesBG.getRight()).withRight(area.getRight() - EditorDimensions::extraShrinkage(2)));
    keyboard.setBounds(modulesBG.withTop(modulesBG.getBottom() - EditorDimensions::keyboardH).withRight(modulesBG.getRight()).reduced(EditorDimensions::extraShrinkage()));

    collapseBrowserButton.setBounds(area.withTop(area.getHeight() / 2).withRight(area.getX() + EditorDimensions::collapseButtonW).withHeight(EditorDimensions::collapseButtonH));

}

void KrumSamplerAudioProcessorEditor::addNextModuleEditor()
{
    moduleContainer.showFirstEmptyModule();
}

KrumModuleContainer& KrumSamplerAudioProcessorEditor::getModuleContainer()
{
    return moduleContainer;
}

void KrumSamplerAudioProcessorEditor::printModules()
{
    DBG("-----Modules In Sampler-----");
    for (int i = 0; i < sampler.getNumModules(); i++)
    {
        auto mod = sampler.getModule(i);
        DBG("Module Index: " + juce::String(mod->getModuleSamplerIndex()));
        DBG("Module Display Index: " + juce::String(mod->getModuleDisplayIndex()));
        DBG("Module Name: " + juce::String(mod->getModuleName()));
        DBG("Midi Note: " + juce::String(mod->getMidiTriggerNote()));
    }
}

void KrumSamplerAudioProcessorEditor::hideFileBrowser()
{
    //fileDrop.setVisible(false);
    fileBrowser.setVisible(false);
    InfoPanel::shared_instance().setVisible(false);
    
    setSize(EditorDimensions::windowWNoBrowser, EditorDimensions::windowH);

    saveFileBrowserHiddenState();
    repaint();
}

void KrumSamplerAudioProcessorEditor::showFileBrowser()
{
    fileBrowser.setVisible(true);
    
    InfoPanel::shared_instance().setVisible(getSavedInfoButtonState());
    setSize(EditorDimensions::windowW, EditorDimensions::windowH);

    saveFileBrowserHiddenState();
    repaint();
}

float KrumSamplerAudioProcessorEditor::getOutputGainValue()
{
    return *parameters.getRawParameterValue(TreeIDs::outputGainParam);
}

void KrumSamplerAudioProcessorEditor::saveFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
    globalTree.setProperty(TreeIDs::fileBrowserHidden, collapseBrowserButton.getToggleState() ? juce::var(1) : juce::var(0), nullptr);
}

bool KrumSamplerAudioProcessorEditor::getSavedFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
    return (int)globalTree.getProperty(TreeIDs::fileBrowserHidden) > 0;
}

void KrumSamplerAudioProcessorEditor::infoButtonClicked()
{
    if (fileBrowser.isVisible())
    {
        InfoPanel::shared_instance().setVisible(infoButton.getToggleState());
    }
    else
    {
        InfoPanel::shared_instance().setVisible(false);
    }

    saveInfoButtonState();
}


bool KrumSamplerAudioProcessorEditor::getSavedInfoButtonState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
    return (int)globalTree.getProperty(TreeIDs::infoPanelToggle) > 0;
}

void KrumSamplerAudioProcessorEditor::saveInfoButtonState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
    
    globalTree.setProperty(TreeIDs::infoPanelToggle, infoButton.getToggleState() ? juce::var(1) : juce::var(0), nullptr);
}

void KrumSamplerAudioProcessorEditor::updateOutputGainBubbleComp(juce::Component* comp)
{
    auto bubbleComp = static_cast<juce::BubbleComponent*>(comp);
    if (bubbleComp != nullptr)
    {
        juce::Point<int> pos{ outputGainSlider.getBoundsInParent().getCentreX() , getMouseXYRelative().getY() - 10 };

        bubbleComp->setAllowedPlacement(juce::BubbleComponent::above);
        bubbleComp->setPosition(pos, 0);

    }
    outputGainSlider.setTooltip(outputGainSlider.getTextFromValue(outputGainSlider.getValue()));
}

void KrumSamplerAudioProcessorEditor::addKeyboardListener(juce::MidiKeyboardStateListener* listener)
{
    audioProcessor.addMidiKeyboardListener(listener);
}

void KrumSamplerAudioProcessorEditor::removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove)
{
    audioProcessor.removeMidiKeyboardListener(listenerToRemove);
}

juce::String KrumSamplerAudioProcessorEditor::getMidiInfo(const juce::MidiMessage& midiMessage)
{
    return midiMessage.getDescription() + " Note Number: " + juce::String(midiMessage.getNoteNumber());
}

KrumSampler& KrumSamplerAudioProcessorEditor::getSampler()
{
    return sampler;
}

juce::AudioFormatManager* KrumSamplerAudioProcessorEditor::getAudioFormatManager()
{
    return audioProcessor.getFormatManager();
}

juce::AudioThumbnailCache& KrumSamplerAudioProcessorEditor::getThumbnailCache()
{
    return audioProcessor.getThumbnailCache();
}

juce::ValueTree* KrumSamplerAudioProcessorEditor::getValueTree()
{
    return audioProcessor.getValueTree();
}

juce::AudioProcessorValueTreeState* KrumSamplerAudioProcessorEditor::getParameters()
{
    return &parameters;
}

KrumFileBrowser* KrumSamplerAudioProcessorEditor::getFileBrowser()
{
    return &fileBrowser;
}

juce::Viewport* KrumSamplerAudioProcessorEditor::getModuleViewport()
{
    return &modulesViewport; 
}

//=========================================================================================


