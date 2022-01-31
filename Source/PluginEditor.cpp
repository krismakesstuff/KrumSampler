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
    //TODO: change this to the resource folder in "CommonFiles", that is put there by the download installer
    int titleImageSize;
    auto titleImageData = BinaryData::getNamedResource("KrumSamplerTitle_png", titleImageSize);
    titleImage = juce::ImageFileFormat::loadFrom(titleImageData, titleImageSize);

    auto& laf = getLookAndFeel();
    laf.setDefaultLookAndFeel(&kLaf);
    laf.setDefaultSansSerifTypefaceName("Calibri");

    InfoPanel::shared_instance().getLookAndFeel().setDefaultSansSerifTypefaceName("Calibri");
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
    outputGainSlider.onValueChange = [this] { updateOutputGainBubbleComp(outputGainSlider.getCurrentPopupDisplay()); };
    
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
    
    int leftChevSize, rightChevSize;

    auto leftChevData = BinaryData::getNamedResource("chevron_left_black_24dp_svg", leftChevSize);
    auto rightChevData = BinaryData::getNamedResource("chevron_right_black_24dp_svg", rightChevSize);

    auto collapseLeftIm = juce::Drawable::createFromImageData(leftChevData, leftChevSize);
    auto collapseRightIm = juce::Drawable::createFromImageData(rightChevData, rightChevSize);

    collapseBrowserButton.setImages(collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(),
                                    collapseLeftIm.get());

    collapseBrowserButton.setClickingTogglesState(true);
    collapseBrowserButton.onClick = [this] { collapseBrowserButton.getToggleState() ? hideFileBrowser() : showFileBrowser(); };
    collapseBrowserButton.setToggleState(getSavedFileBrowserHiddenState(), juce::sendNotification);
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::darkgrey.darker());
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::darkgrey);
    addAndMakeVisible(collapseBrowserButton);

    int infoBlackSize, infoBlackFilledSize;
    
    auto infoBlackData = BinaryData::getNamedResource("info_white_24dp_svg", infoBlackSize);
    auto infoBlackFilledData = BinaryData::getNamedResource("info_white_filled_24dp_svg", infoBlackFilledSize);
    
    auto infoOffIm = juce::Drawable::createFromImageData(infoBlackData, infoBlackSize);
    auto infoOnIm = juce::Drawable::createFromImageData(infoBlackFilledData, infoBlackFilledSize);
    
    infoButton.setImages(infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOnIm.get());
    infoButton.setClickingTogglesState(true);
    infoButton.onClick = [this] { infoButtonClicked();  };
    addAndMakeVisible(infoButton);

    
    moduleContainer.createModuleEditors();
    moduleContainer.showFirstEmptyModule();
    keyboard.updateKeysFromValueTree();
    
    if (keyboard.hasAssignedKeys())
    {
        keyboard.scrollToKey(keyboard.getLowestKey());
    }
    
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

    auto bgGrade = juce::ColourGradient::vertical(bgColor, bgColor.brighter(0.1f), area);
    g.setGradientFill(bgGrade);
    g.fillRect(area);
    
    auto titleRect = area.withHeight(EditorDimensions::topBar).withWidth(EditorDimensions::titleImageW).withCentre({area.getCentreX(), EditorDimensions::topBar / 2}).toFloat();

    g.drawImage(titleImage, titleRect.toFloat(), juce::RectanglePlacement::xMid );

    auto moduleBGGrade = juce::ColourGradient::vertical(modulesBGColor.darker(0.2f), modulesBGColor, modulesBG);
    g.setGradientFill(moduleBGGrade);
    g.fillRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize);

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(modulesViewport.getBounds().toFloat(), EditorDimensions::cornerSize);
    
    g.setColour(juce::Colours::grey.brighter(0.2f));
    g.drawFittedText(madeByString, EditorDimensions::madeByArea.withX(area.getRight() - (EditorDimensions::madeByArea.getWidth() + 10)).withY(area.getY()), juce::Justification::centred, 1);
    g.drawRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);

    g.setColour(backOutlineColor);
    g.drawRoundedRectangle(outputGainSlider.getBounds().withBottom(modulesBG.getBottom()).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    
    if (!collapseBrowserButton.getToggleState())
    {
        g.drawRoundedRectangle(fileBrowser.getBounds().withY(fileBrowser.getBounds().getY()).withBottom(fileBrowser.getBounds().getBottom()).expanded(EditorDimensions::extraShrinkage(1), EditorDimensions::extraShrinkage(1)).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    }
    
    g.setColour(mainFontColor);
    g.setFont(18.0f);
    g.drawFittedText("Output", area.withTop(outputGainSlider.getBottom()).withLeft(modulesBG.getRight()).withBottom(modulesBG.getBottom()),
                    juce::Justification::centred,1);

    juce::Rectangle<int> linesBounds{ outputGainSlider.getBoundsInParent().withWidth(outputGainSlider.getWidth() - 10).withTrimmedLeft(10).withTrimmedTop(21).withTrimmedBottom(5) };

    paintOutputVolumeLines(g, linesBounds.toFloat());
    
    //Version
    g.setColour(juce::Colours::red.darker());
    juce::String versionString = "Build Version: " + juce::String(KRUM_BUILD_VERSION);
    int versionW = g.getCurrentFont().getStringWidth(versionString);
    g.drawFittedText(versionString, { area.getRight() - 350, 10, versionW + 10, 35 }, juce::Justification::centred, 1);
}

void KrumSamplerAudioProcessorEditor::paintOutputVolumeLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 40;
    int spaceBetweenLines = bounds.getHeight() / numLines;

    g.setColour(outputTrackColor);
    juce::Line<float> firstLine{ {bounds.getX(), bounds.getY() - 16}, {bounds.getCentreX(), bounds.getY() - 16} };

    g.drawLine(firstLine);

    float zerodBY = outputGainSlider.getPositionOfValue(1.0f) - 22;
    juce::Line<float> zeroLine{ {bounds.getX() - 0, bounds.getY() + zerodBY}, {bounds.getCentreX() - 5 ,  bounds.getY() + zerodBY} };
    g.drawLine(zeroLine, 2.0f);

    juce::Line<float> line;

    for (int i = 1; i < numLines; i++)
    {
        float startX = bounds.getX();
        float endX = bounds.getCentreX();
        if (i % 2)
        {
            startX += 6;
            endX -= 6;
        }

        line.setStart({ startX, firstLine.getStartY() + (i * spaceBetweenLines) });
        line.setEnd({ endX,  firstLine.getStartY() + (i * spaceBetweenLines) });
        g.drawLine(line);

    }

    g.drawFittedText("0", { (int)zeroLine.getStartX() - 8, (int)zeroLine.getStartY() - 7, 15, 15 }, juce::Justification::centredLeft, 1);
}

void KrumSamplerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int infoButtonSize = 21;
    
    websiteButton.setBounds(EditorDimensions::madeByArea.withX(area.getRight() - (EditorDimensions::madeByArea.getWidth() + 10)).withY(area.getY() + (EditorDimensions::madeByArea.getHeight() / 2) + 5).withHeight(EditorDimensions::madeByArea.getHeight() * 0.8f).withWidth(EditorDimensions::madeByArea.getWidth() + 10));
    infoButton.setBounds(websiteButton.getBounds().getCentreX() - (infoButtonSize /2), websiteButton.getBottom(), infoButtonSize, infoButtonSize);
    
    
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

    outputGainSlider.setBounds(area.withTop(modulesBG.getY()).withBottom(area.getBottom() - EditorDimensions::extraShrinkage(10)).withLeft(modulesBG.getRight() + EditorDimensions::extraShrinkage(3)).withRight(area.getRight() - EditorDimensions::extraShrinkage(3)));
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


