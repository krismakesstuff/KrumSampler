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
    //change this to the resource folder in "CommonFiles", that is put there by the download installer
    int titleImageSize;
    auto titleImageData = BinaryData::getNamedResource("KrumSamplerTitle_png", titleImageSize);
    titleImage = juce::ImageFileFormat::loadFrom(titleImageData, titleImageSize);

    auto& laf = getLookAndFeel();
    laf.setDefaultLookAndFeel(&kLaf);
    laf.setDefaultSansSerifTypefaceName("Calibri");
    //static_cast<KrumLookAndFeel*>(&laf)->outputGainSlider = &outputGainSlider;
    InfoPanel::shared_instance().getLookAndFeel().setDefaultSansSerifTypefaceName("Calibri");
    toolTipWindow->setMillisecondsBeforeTipAppears(2000);

    addAndMakeVisible(websiteButton);
    websiteButton.setButtonText(websiteURL.getDomain());
    websiteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    websiteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    websiteButton.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
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
    
    outputGainAttachment.reset(new SliderAttachment(parameters, TreeIDs::outputGainParam_ID, outputGainSlider));

//    addAndMakeVisible(fileDrop);
//    fileDrop.setRepaintsOnMouseActivity(true);


    
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
    setPaintingIsUnclipped(true);
    
    if (getSavedInfoButtonState())
    {
        infoButton.setToggleState(true, juce::sendNotification); //a button's toggle state is false by default and wont call lambda if the state passed in is the same as the current one
    }
    else
    {
        //infoButton.onClick();
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
    audioProcessor.removeMidiKeyboardListener(this);
}

//==============================================================================

void KrumSamplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(bgColor);
    //g.setGradientFill(ColorPaletteColors::makeGradientFromAllColors(false, {(float)area.getCentreX(), (float)area.getY()}, {(float)area.getCentreX(), (float)area.getBottom()}));
    g.fillRect(area);
    
    auto titleRect = area.withHeight(EditorDimensions::topBar).withWidth(EditorDimensions::titleImageW).withCentre({area.getCentreX(), EditorDimensions::topBar / 2}).toFloat();
    //juce::Rectangle<int> titleRect { 5 , 50, EditorDimensions::topBar, EditorDimensions::titleImageW};
//    g.setColour(juce::Colours::red);
//    g.drawRect(titleRect);
    g.drawImage(titleImage, titleRect.toFloat(), juce::RectanglePlacement::xMid );

    g.setColour(modulesBGColor);
    g.fillRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize);

    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(modulesViewport.getBounds().toFloat(), EditorDimensions::cornerSize);
    
    g.setColour(outlineColor);
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
}

void KrumSamplerAudioProcessorEditor::paintOutputVolumeLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 40;
    int spaceBetweenLines = bounds.getHeight() / numLines;

    g.setColour(outputTrackColor/*.withAlpha(0.5f)*/);
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

void KrumSamplerAudioProcessorEditor::visibilityChanged()
{
    if (needsToUpdateThumbs)
    {
        //updateThumbnails();
    }

    juce::Component::visibilityChanged();
}

void KrumSamplerAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState* keyState, int midiChannel, int midiNote, float velocity)
{
    //auto m = juce::MidiMessage::noteOn(midiChannel, midiNote, velocity);
    //postMessageToList(m, juce::String());
}

void KrumSamplerAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState* keyState, int midiChannel, int midiNote, float velocity)
{
    //auto m = juce::MidiMessage::noteOff(midiChannel, midiNote, velocity);
    //postMessageToList(m, juce::String());
}

//called when we create a new module from the editor
//bool KrumSamplerAudioProcessorEditor::createModule(juce::String& moduleName, int index, juce::File& file)
//{
//    if (sampler.getNumModules() >= MAX_NUM_MODULES)
//    {
//        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Too many samples!",
//            "Right now this only supports " + juce::String(MAX_NUM_MODULES) + " samples.");
//        return false;
//    }
//
//    if (sampler.isFileAcceptable(file))
//    {
//        auto mod = new KrumModule(moduleName, index, file, sampler, audioProcessor.getValueTree(), &parameters);
//        auto modEd = mod->createModuleEditor(*this);
//
//        if (mod != nullptr && modEd != nullptr)
//        {
//            addKeyboardListener(mod);
//            sampler.addModule(mod, false);
//            moduleContainer.addModuleEditor(modEd);
//            modulesViewport.setViewPositionProportionately(1, 0);
//            modEd->setWantsKeyboardFocus(true);
//            return true;
//        }
//        else
//        {
//            DBG("Module Creation failed");
//            return false;
//        }
//    }
//    else
//    {
//        DBG("File is unacceptable");
//        return false;
//    }
//}


void KrumSamplerAudioProcessorEditor::addNextModuleEditor()
{
    moduleContainer.showFirstEmptyModule();
    //It through valueTree instead!
  /*  for (int i = 0; i < sampler.getNumModules(); i++)
    {
        auto mod = sampler.getModule(i);
        if(mod->isModuleEmpty() && (!mod->hasEditor()))
        {
            moduleContainer.addModuleEditor(mod->createModuleEditor(*this));
            return;
        }
    }*/


    //auto modulesTree = valueTree.getChildWithName(TreeIDs::KRUMMODULES);
    //for (int i = 0; i < modulesTree.getNumChildren(); i++)
    //{
    //    auto moduleTree = modulesTree.getChild(i);
    //    if ((int)moduleTree.getProperty(TreeIDs::moduleState) == 0)
    //    {
    //        moduleContainer.addModuleEditor(new KrumModuleEditor(moduleTree, *this, sampler.getFormatManager()));
    //        return; //we only want to show one more empty module
    //    }
    //}

}

KrumModuleContainer& KrumSamplerAudioProcessorEditor::getModuleContainer()
{
    return moduleContainer;
}

void KrumSamplerAudioProcessorEditor::reconstructModuleDisplay(juce::ValueTree& moduleDisplayTree)
{
    if (moduleDisplayTree.isValid() && !moduleDisplayTree.getProperty("value").isVoid())
    {
        auto modArray = moduleDisplayTree.getProperty("value");
        
        //if (modArray != nullptr && modArray.isArray())
        //{
        //    /*for (int i = 0; i < modArray->size(); i++)
        //    {
        //        auto modIndex = (int)modArray->getUnchecked(i); 
        //        moduleDisplayOrder.add(modIndex);
        //    }*/
        //}
        //else if (modArray != nullptr)
        //{

        //}

    }
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
    //fileDrop.setVisible(true);
    fileBrowser.setVisible(true);
    
    InfoPanel::shared_instance().setVisible(getSavedInfoButtonState());
    setSize(EditorDimensions::windowW, EditorDimensions::windowH);

    saveFileBrowserHiddenState();
    repaint();
}

void KrumSamplerAudioProcessorEditor::saveFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
 //   auto hiddenTree = globalTree.getChildWithName(TreeIDs::fileBrowserHidden);
    globalTree.setProperty(TreeIDs::fileBrowserHidden, collapseBrowserButton.getToggleState() ? juce::var(1) : juce::var(0), nullptr);
}

bool KrumSamplerAudioProcessorEditor::getSavedFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName(TreeIDs::GLOBALSETTINGS);
    //auto hiddenTree = globalTree.getChildWithName(TreeIDs::fileBrowserHidden);

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

//void KrumSamplerAudioProcessorEditor::setKeyboardNoteColor(int midiNoteNumber, juce::Colour color, int oldNote)
//{
//    keyboard.assignMidiNoteColor(midiNoteNumber, color, oldNote);
//}

void KrumSamplerAudioProcessorEditor::removeKeyboardNoteColor(int midiNoteNumber)
{
    keyboard.removeMidiNoteColorAssignment(midiNoteNumber);
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

//void KrumSamplerAudioProcessorEditor::cleanUpEmptyModuleTrees(/*int numModules*/)
//{
//    int numModules = audioProcessor.getNumModulesInSampler();
//    auto valueTree = getValueTree();
//
//    if (numModules < MAX_NUM_MODULES)
//    {
//        auto modulesTree = valueTree->getChildWithName("KrumModules");
//
//        for (int i = numModules; i < MAX_NUM_MODULES; i++)
//        {
//            auto moduleTree = modulesTree.getChildWithName("Module" + juce::String(i));
//
//            moduleTree.setProperty("name", juce::var(""), nullptr);
//
//            for (int j = 0; j < moduleTree.getNumChildren(); j++)
//            {
//                auto stateTree = moduleTree.getChild(j);
//                auto id = stateTree.getProperty("id");
//
//                if (id.toString() == TreeIDs::paramModuleState_ID)
//                {
//                    stateTree.setProperty("value", juce::var(0), nullptr);
//                }
//                else if (id.toString() == TreeIDs::paramModuleFile_ID)
//                {
//                    stateTree.setProperty("value", juce::var(""), nullptr);
//                }
//                else if (id.toString() == TreeIDs::paramModuleMidiNote_ID)
//                {
//                    stateTree.setProperty("value", juce::var(0), nullptr);
//                }
//                else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID)
//                {
//                    stateTree.setProperty("value", juce::var(0), nullptr);
//                }
//                else if (id.toString() == TreeIDs::paramModuleColor_ID)
//                {
//                    stateTree.setProperty("value", juce::var(""), nullptr);
//                }
//                else if (id.toString() == TreeIDs::paramModuleDisplayIndex_ID)
//                {
//                    stateTree.setProperty("value", juce::var(""), nullptr);
//                }
//            }
//        }
//    }
//}

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

//void KrumSamplerAudioProcessorEditor::updateThumbnails()
//{
//    for (int i = 0; i < moduleContainer.getModuleDisplayOrder().size(); i++)
//    {
//        //auto modEd = moduleContainer.getEditorFromModule(sampler.getModule(i));
//        modEd->setAndDrawThumbnail();
//    }
//    needsToUpdateThumbs = false;
//}



//=========================================================================================


