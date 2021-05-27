/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//juce::ValueTree& getFileBrowserTree(juce::ValueTree& valueTree)
//{
//    auto fileBrowserTree = valueTree.getChildWithName("FileBrowserTree");
//    
//    if (fileBrowserTree.isValid())
//    {
//        DBG(fileBrowserTree.toXmlString());
//    }
//    else
//    {
//        DBG("BrowserTree NOT valid");
//    }
//
//    return fileBrowserTree;
//}

//juce::ValueTree& getPreviewerGainTree(juce::ValueTree& valueTree)
//{
//    auto globalTree = valueTree.getChildWithName("GlobalSettings");
//    auto prevGainTree = globalTree.getChildWithName("PreviewerGain");
//    DBG("Previewer Gain Tree: " + prevGainTree.toXmlString());
//    return prevGainTree;
//}

//==============================================================================
KrumSamplerAudioProcessorEditor::KrumSamplerAudioProcessorEditor (KrumSamplerAudioProcessor& p, KrumSampler& s, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& valueTree, juce::ValueTree& fileBrowserTree)
    : AudioProcessorEditor (&p), audioProcessor (p), sampler(s), parameters(apvts), fileBrowser(valueTree, fileBrowserTree, *audioProcessor.getFormatManager())
{

    auto titleImageFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\KrumSamplerTitle.png");
    //auto titleImageFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\KrumSamplerTitleAirborne.png");
    

    titleImage = juce::ImageFileFormat::loadFrom(titleImageFile);

    juce::String validString = (titleImage.isValid() ? "Valid" : "INValid");
    DBG("TitleImage: " + validString);



    getLookAndFeel().setDefaultLookAndFeel(&kLaf);
    //getLookAndFeel().setDefaultSansSerifTypefaceName(defaultFont.getTypefaceName());
    //getLookAndFeel().setDefaultSansSerifTypeface(getWackyFont().getTypeface());
    getLookAndFeel().setDefaultSansSerifTypefaceName("Calibri");
    toolTipWindow->setMillisecondsBeforeTipAppears(1000);

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

    addAndMakeVisible(fileDrop);
    fileDrop.setRepaintsOnMouseActivity(true);

    addAndMakeVisible(fileBrowser);

    juce::File collapseLeftImFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\chevron_left_black_24dp.svg");
    juce::File collapseRightImFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\chevron_right_black_24dp.svg");
    
    auto collapseLeftIm = juce::Drawable::createFromSVGFile(collapseLeftImFile);
    auto collapseRightIm = juce::Drawable::createFromSVGFile(collapseRightImFile);

    collapseBrowserButton.setImages(collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(), collapseRightIm.get(),
                                    collapseLeftIm.get());

    collapseBrowserButton.setClickingTogglesState(true);
    collapseBrowserButton.setToggleState(getSavedFileBrowserHiddenState(), juce::dontSendNotification);
    collapseBrowserButton.onStateChange = [this] { collapseBrowserButton.getToggleState() ? hideFileBrowser() : showFileBrowser(); };
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::darkgrey.darker());
    collapseBrowserButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::darkgrey);
    addAndMakeVisible(collapseBrowserButton);

    
    //this was used to print out midi messages as a log
    //audioProcessor.addMidiKeyboardListener(this);

    
    addAndMakeVisible(keyboard);
    
    addAndMakeVisible(modulesViewport);
    
    modulesViewport.setViewedComponent(&moduleContainer);
    modulesViewport.setSingleStepSizes(10, 10);
    modulesViewport.setInterceptsMouseClicks(true, true);
    
    //modulesViewport.setScrollOnDragEnabled(true);
    modulesViewport.setScrollBarsShown(false, true, false, false);
    //modulesViewport.setViewPosition(200, 200);
    
    if (sampler.getNumModules() > 0)
    {
        createModuleEditors();
    }

    //moduleContainer.refreshModuleLayout(true);
   
    setPaintingIsUnclipped(true);
    setOpaque(true);

    //modulesViewport.setRepaintsOnMouseActivity(true);
    setInterceptsMouseClicks(false, true);
    //startTimerHz(10);
   // setResizable(true, true);
    //resized();
    if (collapseBrowserButton.getToggleState())
    {
        setSize(900, 700);
    }
    else
    {
        setSize (1200, 700);
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
    g.fillRect(area);
    
    auto titleRect = area.withHeight(EditorDimensions::topBar).withY(7).withX(10).toFloat();

    //g.setColour(mainFontColor);
    //g.setFont(20.f);
    //g.drawFittedText("KRUM SAMPLER", area.withHeight(EditorDimensions::topBar), juce::Justification::centred, 1);
    g.drawImage(titleImage, titleRect, juce::RectanglePlacement::centred);
    //g.drawImageWithin(titleImage, area.getX(), area.getY(), area.getWidth(), EditorDimensions::topBar, juce::RectanglePlacement::fillDestination, true);

    g.setColour(modulesBGColor);
    g.fillRoundedRectangle(modulesBG.toFloat(), EditorDimensions::cornerSize);
    
    g.setColour(outlineColor);
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

    //moduleContainer.refreshModuleLayout(false);

}

void KrumSamplerAudioProcessorEditor::paintOutputVolumeLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 40;
    int spaceBetweenLines = bounds.getHeight() / numLines;


    g.setColour(outputTrackColor/*.withAlpha(0.5f)*/);
    juce::Line<float> firstLine{ {bounds.getX(), bounds.getY()}, {bounds.getCentreX(), bounds.getY()} };
    g.drawLine(firstLine);
    juce::Point<int> zeroLine;
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

        line.setStart({ startX, bounds.getY() + (i * spaceBetweenLines) });
        line.setEnd({ endX,  bounds.getY() + (i * spaceBetweenLines) });
        g.drawLine(line);

        if (i == 16)
        {
            zeroLine = line.getStart().toInt();
        }
    }

    g.drawFittedText("0", { zeroLine.getX() - 5, zeroLine.getY() + 4 , 15, 15 }, juce::Justification::centredLeft, 1);


}

void KrumSamplerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();


    if (!collapseBrowserButton.getToggleState())
    {
        //File Browser is Visible

        modulesBG = area.withTop(EditorDimensions::topBar).withLeft(area.getX()  + EditorDimensions::emptyAreaMinW/* - dimensions.outputW*/).withRight(area.getRight() - EditorDimensions::outputW).reduced(EditorDimensions::extraShrinkage());
        //addModuleButton.setBounds(area.withTop(EditorDimensions::shrinkage * 2).withHeight(EditorDimensions::addButtonH).withWidth(EditorDimensions::addButtonW).reduced(EditorDimensions::shrinkage));
        
        fileDrop.setBounds(area.withTop(titleImage.getBounds().getBottom()).withRight(modulesBG.getX()).withBottom(area.getBottom() - EditorDimensions::infoH).reduced(EditorDimensions::extraShrinkage()));
        fileBrowser.setBounds(area.withTop(fileDrop.getBottom()+ EditorDimensions::shrinkage).withRight(modulesBG.getX()).reduced(EditorDimensions::extraShrinkage(3)));
       
        modulesViewport.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH)/*.withLeft(modulesBG.getX() + dimensions.shrinkage)*/.withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
        moduleContainer.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH)/*.withLeft(modulesBG.getX() + dimensions.shrinkage)*/.withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
        moduleContainer.refreshModuleLayout(true);

        outputGainSlider.setBounds(area.withTop(modulesBG.getY()).withBottom(area.getBottom() - EditorDimensions::extraShrinkage(10)).withLeft(modulesBG.getRight() + EditorDimensions::extraShrinkage()).withRight(area.getRight() - EditorDimensions::extraShrinkage()));
        keyboard.setBounds(modulesBG.withTop(modulesBG.getBottom() - EditorDimensions::keyboardH).withRight(modulesBG.getRight()).reduced(EditorDimensions::extraShrinkage()));
        
        collapseBrowserButton.setBounds(area.withTop(area.getHeight()/2).withRight(area.getX() + EditorDimensions::collapseButtonW).withHeight(EditorDimensions::collapseButtonH));
    }
    else
    {
        //File Browser is Hidden

        modulesBG = area.withTop(EditorDimensions::topBar).withRight(area.getRight() - EditorDimensions::outputW).reduced(EditorDimensions::extraShrinkage());
        //addModuleButton.setBounds(area.withTop(EditorDimensions::shrinkage * 2).withHeight(EditorDimensions::addButtonH).withWidth(EditorDimensions::addButtonW).reduced(EditorDimensions::shrinkage));

        modulesViewport.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH)/*.withLeft(modulesBG.getX() + dimensions.shrinkage)*/.withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
        moduleContainer.setBounds(modulesBG.withBottom(area.getBottom() - EditorDimensions::keyboardH)/*.withLeft(modulesBG.getX() + dimensions.shrinkage)*/.withRight(area.getRight() - EditorDimensions::outputW - EditorDimensions::extraShrinkage()).reduced(EditorDimensions::extraShrinkage()));
        moduleContainer.refreshModuleLayout(true);

        outputGainSlider.setBounds(area.withTop(modulesBG.getY()).withBottom(area.getBottom() - EditorDimensions::extraShrinkage(10)).withLeft(modulesBG.getRight() + EditorDimensions::extraShrinkage()).withRight(area.getRight() - EditorDimensions::extraShrinkage()));
        keyboard.setBounds(modulesBG.withTop(modulesBG.getBottom() - EditorDimensions::keyboardH).withRight(modulesBG.getRight()).reduced(EditorDimensions::extraShrinkage()));

        collapseBrowserButton.setBounds(area.withTop(area.getHeight()/ 2).withRight(area.getX() + EditorDimensions::collapseButtonW).withHeight(EditorDimensions::collapseButtonH));

    }


}

//void KrumSamplerAudioProcessorEditor::mouseMove(const juce::MouseEvent& e)
//{
//    auto activeBounds = getLocalBounds().withRight(fileBrowser.getBounds().getRight());
//    auto mousePos = e.getPosition();
//
//    if (activeBounds.contains(mousePos.getX(), mousePos.getY()))
//    {
//        collapseBrowserButton.setVisible(true);
//    }
//    else
//    {
//        collapseBrowserButton.setVisible(false);
//    }
//    
//    juce::MessageManagerLock lock;
//    repaint();
//    
//
//
//}
//
//
//void KrumSamplerAudioProcessorEditor::mouseExit(const juce::MouseEvent& e)
//{
//    juce::MessageManagerLock lock;
//    collapseBrowserButton.setVisible(false);
//    repaint();
//
//}



void KrumSamplerAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState* keyState, int midiChannel, int midiNote, float velocity)
{
    auto m = juce::MidiMessage::noteOn(midiChannel, midiNote, velocity);
    postMessageToList(m, juce::String());
}

void KrumSamplerAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState* keyState, int midiChannel, int midiNote, float velocity)
{
    auto m = juce::MidiMessage::noteOff(midiChannel, midiNote, velocity);
    postMessageToList(m, juce::String());
}

void KrumSamplerAudioProcessorEditor::createModule(juce::String& moduleName, int index, juce::File& file)
{
    auto mod = new KrumModule(moduleName, index, file, sampler, audioProcessor.getValueTree(), &parameters);
    auto modEd = mod->createModuleEditor(*this);

    if (mod != nullptr && modEd != nullptr)
    {
        addKeyboardListener(mod);
        sampler.addModule(mod, false);
        moduleContainer.addModuleEditor(modEd);
    }
    else
    {
        //module creation failed
    }
}

void KrumSamplerAudioProcessorEditor::createModuleEditors()
{
    for (int i = 0; i < sampler.getNumModules(); i++)
    {
        auto mod = sampler.getModule(i);
        auto modEd = mod->createModuleEditor(*this);
        if (mod->hasEditor())
        {
            moduleContainer.addModuleEditor(modEd, false);
        }
    }
}

KrumModuleContainer& KrumSamplerAudioProcessorEditor::getModuleContainer()
{
    return moduleContainer;
}

//void KrumSamplerAudioProcessorEditor::addModuleToContainer(KrumModule* newModule, bool addVoice)
//{
//    //moduleContainer.addModule(newModule, addVoice);
//}


//void KrumSamplerAudioProcessorEditor::addModuleToDisplayOrder(int displayIndex, int moduleIndex)
//{
//   
//    //moduleDisplayOrder.insert(displayIndex, moduleIndex);
//
//    //audioProcessor.updateValueTreeState();
//    //printModuleDisplayOrder();
//    //printModules();
//
//}

//void KrumSamplerAudioProcessorEditor::removeModule(KrumModule* moduleToDelete)
//{
//
//    int samplerIndex = moduleToDelete->getModuleIndex();
//    int displayIndex = moduleToDelete->getModuleDisplayIndex();
//
//    audioProcessor.getMidiState().removeListener(moduleToDelete);
//    keyboard.removeMidiNoteColorAssignment(moduleToDelete->getMidiTriggerNote());
//    sampler.removeModule(moduleToDelete);
//    container.moduleDisplayOrder.remove(displayIndex);
//
//    //updated the Modules knowledge of it's own display position
//    for (int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        auto& currentdisplayIndex = moduleDisplayOrder.getReference(i);
//        if (currentdisplayIndex >= samplerIndex)
//        {
//            --currentdisplayIndex;
//            sampler.getModule(i)->setModuleDisplayIndex(currentdisplayIndex);
//        }
//    }
//
//    audioProcessor.updateValueTreeState();
////    printModuleDisplayOrder();
////    printModules();
//}



//juce::Array<int>& KrumSamplerAudioProcessorEditor::getModuleDisplayOrder()
//{
//    return moduleDisplayOrder;
//}

//void KrumSamplerAudioProcessorEditor::printModuleDisplayOrder()
//{
//    DBG("----Module Display Order----");
//    for (int i = 0; i < moduleDisplayOrder.size(); i++)
//    {
//        DBG("Module Display Position: " + juce::String(i) + " Module Index: " + juce::String(moduleDisplayOrder[i]));
//        
//    }
//}


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
        DBG("Module Index: " + juce::String(mod->getModuleIndex()));
        DBG("Module Display Index: " + juce::String(mod->getModuleDisplayIndex()));
        DBG("Module Name: " + juce::String(mod->getModuleName()));
        DBG("Midi Note: " + juce::String(mod->getMidiTriggerNote()));
        //DBG("Module Bounds: " + mod->getBounds().toString());
    }
}

void KrumSamplerAudioProcessorEditor::hideFileBrowser()
{
    juce::MessageManagerLock lock;

    fileDrop.setVisible(false);
    fileBrowser.setVisible(false);
    setSize(900, 700);

    saveFileBrowserHiddenState();
    repaint();
}

void KrumSamplerAudioProcessorEditor::showFileBrowser()
{
    juce::MessageManagerLock lock;

    fileDrop.setVisible(true);
    fileBrowser.setVisible(true);
    setSize(1200, 700);

    saveFileBrowserHiddenState();
    repaint();

}

void KrumSamplerAudioProcessorEditor::saveFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName("GlobalSettings");
    auto hiddenTree = globalTree.getChildWithName("BrowserHidden");

    hiddenTree.setProperty("value", collapseBrowserButton.getToggleState() ? juce::var(1) : juce::var(0), nullptr);
}

bool KrumSamplerAudioProcessorEditor::getSavedFileBrowserHiddenState()
{
    auto globalTree = getValueTree()->getChildWithName("GlobalSettings");
    auto hiddenTree = globalTree.getChildWithName("BrowserHidden");

    return (int)hiddenTree.getProperty("value") > 0;
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


void KrumSamplerAudioProcessorEditor::setKeyboardNoteColor(int midiNoteNumber, juce::Colour color)
{
    keyboard.assignMidiNoteColor(midiNoteNumber, color);
}

void KrumSamplerAudioProcessorEditor::addKeyboardListener(juce::MidiKeyboardStateListener* listener)
{
    audioProcessor.addMidiKeyboardListener(listener);
}

void KrumSamplerAudioProcessorEditor::removeKeyboardListener(juce::MidiKeyboardStateListener* listenerToRemove)
{
    audioProcessor.removeMidiKeyboardListener(listenerToRemove);
}

void KrumSamplerAudioProcessorEditor::postMessageToList(const juce::MidiMessage& message, const juce::String& source)
{
    (new IncomingMessageCallback(this, message, source))->post();
}

void KrumSamplerAudioProcessorEditor::addMessageToList(const juce::MidiMessage& message, const juce::String& source)
{
    auto description = getMidiInfo(message);
    setTextBox(description);
}

juce::String KrumSamplerAudioProcessorEditor::getMidiInfo(const juce::MidiMessage& midiMessage)
{
    return midiMessage.getDescription() + " Note Number: " + juce::String(midiMessage.getNoteNumber());
}

void KrumSamplerAudioProcessorEditor::setTextBox(juce::String message)
{
    //textBox.moveCaretToEndOfLine(false);
    //textBox.insertTextAtCaret(message + juce::NewLine::getDefault());
}

void KrumSamplerAudioProcessorEditor::cleanUpEmptyModuleTrees(/*int numModules*/)
{
    int numModules = audioProcessor.getNumModulesInSampler();
    auto valueTree = getValueTree();

    if (numModules < TreeIDs::maxNumModules)
    {
        auto modulesTree = valueTree->getChildWithName("KrumModules");

        for (int i = numModules; i < TreeIDs::maxNumModules; i++)
        {
            auto moduleTree = modulesTree.getChildWithName("Module" + juce::String(i));

            moduleTree.setProperty("name", juce::var(""), nullptr);

            for (int j = 0; j < moduleTree.getNumChildren(); j++)
            {
                auto stateTree = moduleTree.getChild(j);
                auto id = stateTree.getProperty("id");

                if (id.toString() == TreeIDs::paramModuleActive_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleFile_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiNote_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleMidiChannel_ID)
                {
                    stateTree.setProperty("value", juce::var(0), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleColor_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
                else if (id.toString() == TreeIDs::paramModuleDisplayIndex_ID)
                {
                    stateTree.setProperty("value", juce::var(""), nullptr);
                }
            }
        }
    }
}

KrumSampler& KrumSamplerAudioProcessorEditor::getSampler()
{
    return sampler;
}

juce::AudioFormatManager* KrumSamplerAudioProcessorEditor::getAudioFormatManager()
{
    return audioProcessor.getFormatManager();
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