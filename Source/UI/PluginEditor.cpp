/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "UI\PluginEditor.h"
#include "UI\FileBrowser\KrumFileBrowser.h"


//==============================================================================
KrumSamplerAudioProcessorEditor::KrumSamplerAudioProcessorEditor (KrumSamplerAudioProcessor& p, KrumSampler& s, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& valTree, juce::ValueTree& fileBrowserTree)
    : AudioProcessorEditor (&p), audioProcessor (p), sampler(s), parameters(apvts), valueTree(valTree), fileBrowser(fileBrowserTree, getAudioFormatManager(), valTree, apvts, s)
{
    
    titleImage = juce::ImageFileFormat::loadFrom(BinaryData::KrumSamplerTitle_png, BinaryData::KrumSamplerTitle_pngSize);

    auto& laf = getLookAndFeel();
    laf.setDefaultLookAndFeel(&kLaf);

    toolTipWindow->setMillisecondsBeforeTipAppears(2000);

    addAndMakeVisible(websiteButton);
    websiteButton.setButtonText(madeByString);
    
    websiteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::transparentBlack);
    websiteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOffId, Colors::fontColor.darker(0.1f));
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOnId, Colors::highlightFontColor);
    websiteButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    websiteButton.onClick = [this] { websiteURL.launchInDefaultBrowser(); };

    addAndMakeVisible(outputGainSlider);
    
    outputGainSlider.setScrollWheelEnabled(false);
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 10, 10);
    outputGainSlider.setColour(juce::Slider::ColourIds::thumbColourId, Colors::outputThumbColor);
    outputGainSlider.setColour(juce::Slider::ColourIds::trackColourId, Colors::outputTrackColor);
    outputGainSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, Colors::outlineColor);
    
    outputGainSlider.setNumDecimalPlacesToDisplay(2);
    outputGainSlider.setPopupDisplayEnabled(true, false, this);
    outputGainSlider.setTooltip(outputGainSlider.getTextFromValue(outputGainSlider.getValue()));

    outputGainSlider.onValueChange = [this] { updateOutputGainBubbleComp(outputGainSlider.getCurrentPopupDisplay()); repaint(); };
    outputGainSlider.onDragEnd = [this] { outputGainSlider.setTooltip(outputGainSlider.getTextFromValue(outputGainSlider.getValue())); };
    outputGainAttachment.reset(new SliderAttachment(parameters, TreeIDs::outputGainParam.toString(), outputGainSlider));
    
    addAndMakeVisible(keyboard);
    
    presetsComboBox.setTextWhenNothingSelected("Coming Soon!!");
    presetsComboBox.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    presetsComboBox.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::black);
    presetsComboBox.setColour(juce::ComboBox::ColourIds::arrowColourId, Colors::fontColor);
    presetsComboBox.setColour(juce::ComboBox::ColourIds::textColourId, Colors::fontColor);
    addAndMakeVisible(presetsComboBox);


    auto settingsIm = juce::Drawable::createFromImageData(BinaryData::settingsgrey18dp_svg, BinaryData::settingsgrey18dp_svgSize);

    settingsButton.setImages(settingsIm.get());
    settingsButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    settingsButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    addAndMakeVisible(settingsButton);
    addAndMakeVisible(modulesViewport);
    
    modulesViewport.setViewedComponent(&moduleContainer);
    modulesViewport.setSingleStepSizes(10, 10);
    modulesViewport.setInterceptsMouseClicks(true, true);
    modulesViewport.setScrollBarsShown(false, true, false, false);
    
    addAndMakeVisible(InfoPanel::shared_instance());
    
    fileBrowser.setLookAndFeel(&fbLaf);
    fileBrowser.assignModuleContainer(&moduleContainer);
    fileBrowser.buildDemoKit();
    addAndMakeVisible(fileBrowser);

    auto collapseLeftIm = juce::Drawable::createFromImageData(BinaryData::chevron_left_black_24dp_svg, BinaryData::chevron_left_black_24dp_svgSize);
    auto collapseRightIm = juce::Drawable::createFromImageData(BinaryData::chevron_right_black_24dp_svg, BinaryData::chevron_right_black_24dp_svgSize);

    collapseBrowserButton.setImages(collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(),
                                    collapseLeftIm.get());

    collapseBrowserButton.setClickingTogglesState(true);
    collapseBrowserButton.setToggleState(getSavedFileBrowserHiddenState(), juce::sendNotification);
    collapseBrowserButton.onClick = [this] { collapseButtonClicked(); };
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, Colors::fontColor.darker(0.4f));
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, Colors::fontColor.darker(0.4f));
    collapseBrowserButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(collapseBrowserButton);

    auto infoOffIm = juce::Drawable::createFromImageData(BinaryData::info_grey_bigger_24dp_svg, BinaryData::info_grey_bigger_24dp_svgSize);
    auto infoOnIm = juce::Drawable::createFromImageData(BinaryData::info_grey_filled_bigger_24dp_svg, BinaryData::info_grey_filled_bigger_24dp_svgSize);
    
    infoButton.setImages(infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOffIm.get(), infoOnIm.get());
    infoButton.setClickingTogglesState(true);
    infoButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    infoButton.onClick = [this] { infoButtonClicked();  };

    addAndMakeVisible(infoButton);

    moduleContainer.createModuleEditors();
    moduleContainer.showFirstEmptyModule();
    
    keyboard.scrollToKey(keyboard.getLowestKey());
    keyboard.repaint();

    
    setPaintingIsUnclipped(true);
    
    if (getSavedInfoButtonState())
    {
        infoButton.setToggleState(true, juce::sendNotification); //a button's toggle state is false by default and wont call lambda if the state passed in is the same as the current one
    }
    else
    {
        infoButtonClicked();
    }

    setResizable(true, true);
    setConstrainer(&constrainer);
    setConstrainerLimits(false);
    
    //setSize(getSavedEditorWidth(), getSavedEditorHeight());

    setSize(EditorDimensions::windowW, EditorDimensions::windowH);
}

KrumSamplerAudioProcessorEditor::~KrumSamplerAudioProcessorEditor()
{
}

//==============================================================================

void KrumSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(Colors::bgColor);
    g.fillRect(area);
    
    //Title Image
    auto titleRect = area.withTop(EditorDimensions::shrinkage).withLeft(EditorDimensions::extraShrinkage()).withHeight(EditorDimensions::topBar).withWidth(EditorDimensions::titleImageW).toFloat();
    g.drawImage(titleImage, titleRect.toFloat(), juce::RectanglePlacement::xLeft );

    auto moduleBGGrade = juce::ColourGradient::vertical(Colors::modulesBGColor.darker(0.1f), Colors::modulesBGColor, modulesBG);

    //Module Container Outline
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    
    //Module Container Fill
    g.setColour(juce::Colours::black.withAlpha(0.03f));
    g.fillRoundedRectangle(modulesViewport.getBounds().toFloat(), EditorDimensions::cornerSize);
    

    //Output Slider Outline
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(outputGainSlider.getBounds().withTop(modulesBG.getY() ).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    
    //FileBrowser Outline
    if (!collapseBrowserButton.getToggleState())
    {
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(fileBrowser.getBounds().withY(fileBrowser.getBounds().getY()).withBottom(fileBrowser.getBounds().getBottom()).expanded(EditorDimensions::extraShrinkage(1), EditorDimensions::extraShrinkage(1)).toFloat(), EditorDimensions::cornerSize, EditorDimensions::smallOutline);
    }
    
    //Presets Label
    g.setColour(Colors::fontColor);
    g.setFont(16.0f);
    g.drawFittedText("PRESETS", {presetsComboBox.getX() - 60, presetsComboBox.getY(), presetsComboBox.getWidth() /2, presetsComboBox.getHeight()}, juce::Justification::centredLeft, 1, 1.0f);


    //Output Label
    g.setColour(Colors::fontColor);
    g.setFont(15.0f);
    g.drawFittedText("OUTPUT", area.withTop(modulesBG.getY()).withLeft(modulesBG.getRight() + EditorDimensions::extraShrinkage()).withRight(outputGainSlider.getRight()).withBottom(outputGainSlider.getY()),
                    juce::Justification::centred,1);

    //Version text
    if (showWebsite())
    {
        g.setColour(Colors::fontColor.darker(0.1f));
        g.setFont(12.0f);
        juce::String versionString = "Build Version: " + juce::String(KRUM_BUILD_VERSION);
        int versionW = g.getCurrentFont().getStringWidth(versionString);
        g.drawFittedText(versionString, { titleImage.getBounds().getRight() - 102, websiteButton.getBottom() - 17, versionW + 10, 35 }, juce::Justification::centredLeft, 1);
    }
    g.setFont(15.0f);
}

void KrumSamplerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    if (showWebsite())
    {
        websiteButton.setBounds(EditorDimensions::madeByArea.withX(titleImage.getBounds().getRight() - 125).withY(area.getY() + 7).reduced(12,7));
    }
    else
    {
        websiteButton.setBounds({});
    }

    presetsComboBox.setBounds(area.withLeft(area.getRight() - (EditorDimensions::presetsW + EditorDimensions::settingsButtonW + 30)).withTop(area.getY() + 10).withWidth(EditorDimensions::presetsW).withHeight(EditorDimensions::presetsH));
    settingsButton.setBounds(area.withLeft(presetsComboBox.getRight() + 15).withTop(presetsComboBox.getY() + 2).withWidth(EditorDimensions::settingsButtonW).withHeight(EditorDimensions::settingsButtonH));

    if (!collapseBrowserButton.getToggleState())
    {
        //File Browser is Visible
        modulesBG = area.withTop(EditorDimensions::topBar).withLeft(area.getX()  + EditorDimensions::fileBrowserW).withBottom(getHeight() - EditorDimensions::bottomBarH).withRight(area.getRight() - EditorDimensions::outputW).reduced(EditorDimensions::extraShrinkage());
        fileBrowser.setBounds(area.withTop(EditorDimensions::topBar).withRight(modulesBG.getX()).withBottom(getHeight() - EditorDimensions::bottomBarH).reduced(EditorDimensions::extraShrinkage(3)));
    }
    else
    {
        //File Browser is Hidden
        modulesBG = area.withTop(EditorDimensions::topBar).withRight(area.getRight() - EditorDimensions::outputW).withBottom(getHeight() - EditorDimensions::bottomBarH).reduced(EditorDimensions::extraShrinkage());
    }

    modulesViewport.setBounds(modulesBG.withBottom(area.getBottom() - (EditorDimensions::keyboardH + EditorDimensions::bottomBarH)).withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
    moduleContainer.setBounds(modulesBG.withBottom(area.getBottom() - (EditorDimensions::keyboardH + EditorDimensions::bottomBarH)).withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
    moduleContainer.refreshModuleLayout();

    infoButton.setBounds(area.withLeft(EditorDimensions::shrinkage).withTop(area.getBottom() - (EditorDimensions::bottomBarH + EditorDimensions::shrinkage + 2)).withHeight(EditorDimensions::infoButtonSize).withWidth(EditorDimensions::infoButtonSize).reduced(EditorDimensions::shrinkage));
    InfoPanel::shared_instance().setBounds(area.withX(infoButton.getRight()).withTop(area.getBottom() - (EditorDimensions::bottomBarH + EditorDimensions::shrinkage)));

    outputGainSlider.setBounds(area.withTop(EditorDimensions::extraShrinkage(20)).withBottom(modulesBG.getBottom()).withLeft(modulesBG.getRight() + EditorDimensions::extraShrinkage()).withRight(area.getRight() - EditorDimensions::extraShrinkage(2)));
    keyboard.setBounds(modulesBG.withTop(modulesBG.getBottom() - EditorDimensions::keyboardH).withRight(modulesBG.getRight()).reduced(EditorDimensions::extraShrinkage()));

    collapseBrowserButton.setBounds(area.withTop(area.getHeight() / 2).withRight(area.getX() + EditorDimensions::collapseButtonW).withHeight(EditorDimensions::collapseButtonH));

    saveEditorDimensions();
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

bool KrumSamplerAudioProcessorEditor::isBrowserHidden()
{
    return collapseBrowserButton.getToggleState();
}

void KrumSamplerAudioProcessorEditor::hideFileBrowser()
{
    fileBrowser.setVisible(false);
}

void KrumSamplerAudioProcessorEditor::showFileBrowser()
{
    fileBrowser.setVisible(true);
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
    InfoPanel::shared_instance().setVisible(infoButton.getToggleState());
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

int KrumSamplerAudioProcessorEditor::getSavedEditorWidth()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);;
    return (int)globalTree.getProperty(TreeIDs::editorWidth);
}

void KrumSamplerAudioProcessorEditor::saveEditorWidth()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);;
    globalTree.setProperty(TreeIDs::editorWidth, getWidth(), nullptr);
}

int KrumSamplerAudioProcessorEditor::getSavedEditorHeight()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);;
    return (int)globalTree.getProperty(TreeIDs::editorHeight);
}

void KrumSamplerAudioProcessorEditor::saveEditorHeight()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);;
    globalTree.setProperty(TreeIDs::editorHeight, getHeight(), nullptr);
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
}

void KrumSamplerAudioProcessorEditor::setConstrainerLimits(bool updateComp)
{

    if (isBrowserHidden())
    {
        constrainer.setSizeLimits(EditorDimensions::minWindowWNoBrowser, EditorDimensions::minWindowH, EditorDimensions::maxWindowWNoBrowser, EditorDimensions::maxWindowH);
    }
    else
    {
        constrainer.setSizeLimits(EditorDimensions::minWindowW, EditorDimensions::minWindowH, EditorDimensions::maxWindowW, EditorDimensions::maxWindowH);
    }
        
    if (updateComp)
    {
        constrainer.checkComponentBounds(this);
    }
}

void KrumSamplerAudioProcessorEditor::collapseButtonClicked()
{
    setConstrainerLimits(true);
    
    if (isBrowserHidden())
    {
        hideFileBrowser();
    }
    else
    {
        showFileBrowser();
    }

    saveFileBrowserHiddenState();
    resized();
    repaint();
    saveEditorDimensions();
}

void KrumSamplerAudioProcessorEditor::saveEditorDimensions()
{
    saveEditorHeight();
    saveEditorWidth();
}

bool KrumSamplerAudioProcessorEditor::showWebsite()
{
    return getLocalBounds().getWidth() > 640;
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

juce::AudioFormatManager& KrumSamplerAudioProcessorEditor::getAudioFormatManager()
{
    return *audioProcessor.getFormatManager();
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

