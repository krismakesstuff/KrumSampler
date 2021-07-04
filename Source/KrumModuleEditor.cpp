/*
  ==============================================================================

    KrumModuleEditor.cpp
    Created: 30 Apr 2021 10:21:42am
    Author:  krisc

  ==============================================================================
*/


#include "KrumModuleEditor.h"
#include "KrumModuleProcessor.h"
#include "KrumModule.h"
#include "PluginEditor.h"
#include "ModuleSettingsOverlay.h"

class DragHandle : public juce::DrawableButton
{
public:
    DragHandle(KrumModule& owner, const juce::String& buttonName, juce::DrawableButton::ButtonStyle buttonStyle)
        : parentModule(owner), juce::DrawableButton(buttonName, buttonStyle)
    {}

    ~DragHandle() override
    {}

    void mouseDrag(const juce::MouseEvent& e) override
    {
        parentModule.info.moduleDragging = true;
        //DBG("Module Dragging: " + parentModule->getModuleName());
        parentModule.startDragging("ModuleDragAndDrop", parentModule.getCurrentModuleEditor(), juce::Image(), true);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        parentModule.info.moduleDragging = false;
    }


    KrumModule& parentModule;

};


//===============================================================================================//
//===============================================================================================//


KrumModuleEditor::KrumModuleEditor(KrumModule& o, KrumModuleProcessor& p, KrumSamplerAudioProcessorEditor& e)
    :   parent(o), moduleProcessor(p), editor(e),
        thumbnailCache(5), thumbnail(512, moduleProcessor.sampler.getFormatManager(), thumbnailCache)
{
    setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);
    setVisible(true);
    setPaintingIsUnclipped(true);

    if (parent.info.midiNote == 0 || parent.info.midiChannel == 0)
    {
        //settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
        settingsOverlay.setOwned(new ModuleSettingsOverlay(getLocalBounds(), parent));
        showSettingsOverlay();
    }
    else
    {
        buildModule();
    }

}

KrumModuleEditor::~KrumModuleEditor()
{
    //parent.setEditorVisibility(false);
}

void KrumModuleEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(bgColor);
    juce::Colour c = parent.info.moduleColor.withAlpha(0.5f);
    /*ca = c.withAlpha(0.5f);*/
    if (settingsOverlay != nullptr)
    {
        c = settingsOverlay->getSelectedColor().withAlpha(0.5f);

    }
    else
    {
        /*juce::DropShadow ds{ juce::Colours::black, 1, {1,1} };
        ds.drawForRectangle(g, area);*/

        g.setColour(parent.info.modulePlaying ? c.brighter() : c);
        g.fillRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize);

        if (thumbnail.getNumChannels() == 0)
        {
            paintIfNoFileLoaded(g, thumbnailBG);
        }
        else
        {
            paintIfFileLoaded(g, thumbnailBG);
        }

        
        juce::Rectangle<int> leftLabelRect{ area.getX() + 2, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> rightLabelRect{ area.getRight() - 22, panSlider.getBottom() - 5, 20, 20 };
        juce::Rectangle<int> midiNoteRect{ 10, thumbnailBG.getBottom() + 5, area.getWidth() - 20, 20};
        juce::Rectangle<int> midiChanRect{ 10, midiNoteRect.getBottom() - 5, area.getWidth() - 20, 20 };

        juce::Rectangle<int> labelsBGRect = midiNoteRect.withBottom(midiChanRect.getBottom()).withX(thumbnailBG.getX()).withWidth(thumbnailBG.getWidth());

        g.setColour(c.darker(0.5f));
        g.fillRect(labelsBGRect);
        
        g.setColour(c);
        //g.drawRect(labelsBGRect);
        //g.setColour(fontColor);

        g.setFont(11.0f);
        g.drawFittedText("L", leftLabelRect, juce::Justification::centred, 1);
        g.drawFittedText("R", rightLabelRect, juce::Justification::centred, 1);

        /*juce::Rectangle<int> gainLabelRect{ area.getCentreX() - 20, volumeSlider.getBottom() - 25, 40, 40 };
        g.drawFittedText("Gain", gainLabelRect, juce::Justification::centred, 1);*/
        g.setColour(fontColor);
        g.setFont(14.0f);

        g.drawFittedText("Note:", midiNoteRect, juce::Justification::centredLeft, 1);
        g.drawFittedText(getModuleMidiNoteString(true), midiNoteRect, juce::Justification::centredRight, 1);

        g.drawFittedText("Channel:", midiChanRect, juce::Justification::centredLeft, 1);
        g.drawFittedText(juce::String(getModuleMidiChannel()), midiChanRect, juce::Justification::centredRight, 1);

    }


    //g.setColour(juce::Colours::white);

    auto sliderBounds = volumeSlider.getBoundsInParent().toFloat();
    auto sliderLineBounds = sliderBounds.withTrimmedTop(22).withBottom(sliderBounds.getBottom() - 6).withWidth(sliderBounds.getWidth() + 10).withX(sliderBounds.getX() - 5);
    paintVolumeSliderLines(g, sliderLineBounds);

    auto panSliderBounds = panSlider.getBoundsInParent().toFloat();
    auto panSliderAdBounds = panSliderBounds.withY(panSlider.getY() - 5);
    paintPanSliderLines(g, panSliderAdBounds);
    

   /* juce::Line<int> midLine { {area.getCentreX(), area.getY()}, { area.getCentreX(), area.getBottom() } };
    g.setColour(juce::Colours::red);
    g.drawLine(midLine.toFloat());*/
}

void KrumModuleEditor::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(bgColor);
    g.fillRect(thumbnailBounds);
    g.setColour(bgColor.contrasting());
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void KrumModuleEditor::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    // g.setColour(thumbBgColor);
    g.setColour(parent.info.moduleColor.darker(0.8f));
    g.fillRect(thumbnailBounds);

    g.setColour(parent.info.moduleColor);

    thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
}

void KrumModuleEditor::paintVolumeSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    int numLines = 20;
    int spaceBetweenLines = bounds.getHeight() / numLines;
   

    g.setColour(parent.info.moduleColor.withAlpha(0.5f));
    juce::Line<float> firstLine{ {bounds.getX(), bounds.getY()}, {bounds.getCentreX() - 5, bounds.getY()} };
    juce::Point<int> firstPoint = firstLine.getStart().toInt();
    g.drawLine(firstLine);
    juce::Point<int> zeroLine;
    juce::Line<float> line;
    for (int i = 1; i < numLines; i ++)
    {
        float startX = bounds.getX();
        float endX = bounds.getCentreX() - 5;
        if (i % 2)
        {
            startX += 5;
            endX -= 5;
        }

        line.setStart({ startX, bounds.getY() + ( i * spaceBetweenLines)});
        line.setEnd({ endX,  bounds.getY() + (i * spaceBetweenLines)});
        g.drawLine(line);

        if (i == 7)
        {
            zeroLine = line.getStart().toInt();
        }
    }

    g.drawFittedText("+2", { firstPoint.getX() - 15, firstPoint.getY() - 8 , 15, 15 }, juce::Justification::centredLeft, 1);
    g.drawFittedText("0", { zeroLine.getX() - 15, zeroLine.getY() + 3, 15, 15 }, juce::Justification::centredLeft, 1);


    //juce::Line<float> midPointLine{ 10.0f, height * 0.6f, float(width - 10.f), height * 0.6f };
    //g.drawLine(midPointLine);
}

void KrumModuleEditor::paintPanSliderLines(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    //int numLines = 11;
    //int spaceBetweenLines = bounds.getWidth() / numLines;


    g.setColour(parent.info.moduleColor.withAlpha(0.5f));
    juce::Line<float> midLine{ {bounds.getCentreX() - 2, bounds.getY()},{bounds.getCentreX() - 2 , bounds.getCentreY() } };
    g.drawLine(midLine, 2.0f);

    /*g.setColour(juce::Colours::red);
    g.drawRect(bounds);*/
    /*juce::Line<float> firstLine{ {bounds.getX(), bounds.getBottom()}, {bounds.getX(), bounds.getY()} };
    g.drawLine(firstLine);

    juce::Point<int> zeroLine;
    juce::Line<float> line;
    for (int i = 1; i < numLines; i++)
    {
        float startY = bounds.getY();
        float endY = bounds.getBottom();
        if (i % 2)
        {
            startY -= 4;
            endY += 4;
        }

        line.setStart({ bounds.getX() + (i * spaceBetweenLines), startY });
        line.setEnd({ bounds.getX() + (i * spaceBetweenLines), endY });
        g.drawLine(line);

        if (i == 4)
        {
            zeroLine = line.getStart().toInt();
        }
    }*/

}

void KrumModuleEditor::resized()
{
    auto area = getLocalBounds();

    int titleHeight = 32;

    int spacer = 5;
    int thumbnailH = 120;

    //int dragHandleH = 30;
    //int dragHandleW = area.getWidth() - spacer;

    int panSliderH = 25;
    int panSliderW = area.getWidth();

    int volumeSliderH = 260;
    int volumeSliderW = area.getWidth() / 2.5;

    int statusButtonH = 40;
    //int deleteButtonH = 30;
    //int comboBoxH = 40;

    titleBox.setBounds(area.withBottom(titleHeight).reduced(spacer));
    thumbnailBG = area.withBottom(thumbnailH).withTop(titleBox.getBottom()).reduced(spacer);


    panSlider.setBounds(area.withTop(thumbnailBG.getBottom() + (spacer * 10)).withBottom(thumbnailBG.getBottom() + panSliderH + (spacer * 7)).withWidth(panSliderW).withLeft(area.getCentreX() - (panSliderW/2)).withHeight(panSliderH/* - spacer*/)/*.reduced(spacer)*/);
    volumeSlider.setBounds(area.withTop(panSlider.getBottom()/* + spacer*/).withBottom(panSlider.getBottom() + volumeSliderH).withLeft(area.getCentreX() - (volumeSliderW / 2)).withWidth(volumeSliderW)/*.reduced(spacer)*/);

    playButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withWidth(area.getWidth() / 2).reduced(spacer));
    editButton.setBounds(area.withTop(volumeSlider.getBottom() - (spacer * 2)).withHeight(statusButtonH).withLeft(playButton.getRight() + spacer).withWidth(area.getWidth() / 2).reduced(spacer));

    if (dragHandle != nullptr)
    {
        dragHandle->setBounds(area.withTop(editButton.getBottom() /*+ spacer*/)/*.reduced(spacer)*/);
    }

}

void KrumModuleEditor::mouseDown(const juce::MouseEvent& e)
{
    if (settingsOverlay != nullptr)
    {
        editor.moduleContainer.setModuleSelected(&parent);
        
    }
    else
    {
        juce::Component::mouseDown(e);
    }

}

void KrumModuleEditor::buildModule()
{

    juce::String i = " " + juce::String(parent.info.index);
    auto seperatorString = juce::File::getSeparatorString();
    juce::File appDataFolder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory);
    DBG("App Data Folder: " + appDataFolder.getFullPathName());
    //juce::String dragHandleStringFile = "C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\drag_handle-black-18dp.svg";
    
    juce::String dragHandleStringFile = "KrumSampler"+ seperatorString +"Resources"+ seperatorString +"drag_handle-black-18dp.svg";
    juce::File dragHandleFile = appDataFolder.getChildFile(dragHandleStringFile);
    DBG("Drag Handle File Path: " + dragHandleFile.getFullPathName());
    auto dragHandelIm = juce::Drawable::createFromSVGFile(dragHandleFile);

   // dragHandle.reset(new DragHandle{ parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
    dragHandle.setOwned(new DragHandle{ parent, "Drag Handle", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground });
    dragHandle->setImages(dragHandelIm.get());
    addAndMakeVisible(dragHandle.get());
    dragHandle->setTooltip("Future Kris will make this drag and drop to re-arrange modules");

    addAndMakeVisible(titleBox);
    titleBox.setText(parent.info.name, juce::NotificationType::dontSendNotification);
    titleBox.setFont({ 17.0f });
    titleBox.setColour(juce::Label::ColourIds::textColourId, fontColor);
    titleBox.setJustificationType(juce::Justification::centred);
    titleBox.setEditable(false, true, false);
    titleBox.setTooltip("double-click to change name");
    
    titleBox.onTextChange = [this] { updateName(); };

    thumbnail.setSource(new juce::FileInputSource(parent.info.audioFile));

    addAndMakeVisible(volumeSlider);
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setNumDecimalPlacesToDisplay(2);
    volumeSlider.setDoubleClickReturnValue(true, 1.0f);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTooltip(volumeSlider.getTextFromValue(volumeSlider.getValue()));
    volumeSlider.onValueChange = [this] { updateBubbleComp(&volumeSlider, volumeSlider.getCurrentPopupDisplay()); };
    
    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + i, volumeSlider));
    moduleProcessor.moduleGain = parent.parameters->getRawParameterValue(TreeIDs::paramModuleGain_ID + i);

    addAndMakeVisible(panSlider);
    panSlider.setScrollWheelEnabled(false);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panSlider.setNumDecimalPlacesToDisplay(2);
    panSlider.setDoubleClickReturnValue(true, 1.0f);
    panSlider.setPopupDisplayEnabled(true, false, this);
    panSlider.setTooltip(panSlider.getTextFromValue(panSlider.getValue()));

    panSlider.onValueChange = [this] { updateBubbleComp(&panSlider, panSlider.getCurrentPopupDisplay()); };

    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + i, panSlider));
    moduleProcessor.modulePan = parent.parameters->getRawParameterValue(TreeIDs::paramModulePan_ID + i);

    //juce::String playButtonFileString = "C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\play_arrow-black-18dp.svg";
    
    
    
    //juce::File playButtonImFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile(playButtonFileString);
    juce::String playButtonFileString = "KrumSampler" + seperatorString + "Resources" + seperatorString + "play_arrow-black-18dp.svg";
    juce::File playButtonImFile = appDataFolder.getChildFile(playButtonFileString);
    auto playButtonImage = juce::Drawable::createFromSVGFile(playButtonImFile);
    //DBG("File Path: " + playButtonImFile.getFullPathName());
    
    addAndMakeVisible(playButton);
    playButton.setImages(playButtonImage.get());
    playButton.onClick = [this] { triggerNoteOnInParent(); };
    
    //juce::String editButtonFileString = "C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\settings-black-18dp.svg";

    juce::String editButtonFileString = "KrumSampler"+ seperatorString +"Resources"+ seperatorString +"settings-black-18dp.svg";
    juce::File editButtonImFile = appDataFolder.getChildFile(editButtonFileString);
    auto editButtonImage = juce::Drawable::createFromSVGFile(editButtonImFile);
    
    addAndMakeVisible(editButton);
    editButton.setImages(editButtonImage.get());
    editButton.onClick = [this] { showSettingsMenu(); };
    

    /*addAndMakeVisible(deleteButton);
    juce::File deleteButtonImFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\clear-black-18dp.svg");
    auto deleteButtonImage = juce::Drawable::createFromSVGFile(deleteButtonImFile);

    deleteButton.setImages(deleteButtonImage.get());
    deleteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);
    deleteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkred.brighter());
    deleteButton.onClick = [this] {};*/

    parent.setModuleActive(true);

    /*if (parent.info.midiNote == 0)
    {
        settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
        showSettingsOverlay();
    }*/

    setChildCompColors();
    editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor);

    resized();

    //setSize(EditorDimensions::moduleW, EditorDimensions::moduleH);


}



void KrumModuleEditor::setChildCompColors()
{
    auto moduleColor = parent.info.moduleColor;

    dragHandle->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::transparentBlack);
    dragHandle->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
    dragHandle->setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);

    panSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    panSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());

    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, moduleColor);
    volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, moduleColor.darker());
    volumeSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, moduleColor.darker());

    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.7f));
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));

    editButton.setColour(juce::TextButton::ColourIds::buttonColourId, moduleColor.darker(0.7f));
    editButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, moduleColor.brighter(0.2f));


    titleBox.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::black);
    titleBox.setColour(juce::TextEditor::ColourIds::textColourId, moduleColor.contrasting());

    titleBox.setColour(juce::Label::ColourIds::backgroundColourId, moduleColor.darker(0.7f));
    //titleBox.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::black);


    const juce::MessageManagerLock lock;
    repaint();
}


void KrumModuleEditor::showSettingsMenu()
{
    juce::PopupMenu settingsMenu;

    settingsMenu.addItem(KrumModule::moduleReConfig_Id, "Re-Config");
    settingsMenu.addItem(KrumModule::moduleDelete_Id, "Delete Module");

    auto result = settingsMenu.showAt(&editButton);

    if (result == KrumModule::moduleReConfig_Id)
    {
        //settingsOverlay.reset(new ModuleSettingsOverlay(getLocalBounds(), parent));
        settingsOverlay.setOwned(new ModuleSettingsOverlay(getLocalBounds(), parent));
        settingsOverlay->setMidi(parent.info.midiNote, parent.info.midiChannel);
        settingsOverlay->keepCurrentColor(true);
        showSettingsOverlay(true);
    }
    else if (result == KrumModule::moduleDelete_Id)
    {
        removeFromDisplay();
        parent.deleteEntireModule();
        
        //editor.cleanUpEmptyModuleTrees(/*numModules*/);
    }
}


void KrumModuleEditor::setModuleSelected(bool isModuleSelected)
{
    if (settingsOverlay != nullptr)
    {
        if (isModuleSelected)
        {
            editor.addKeyboardListener(&parent);
        }
        else
        {
            editor.removeKeyboardListener(&parent);
        }

        settingsOverlay->setOverlaySelected(isModuleSelected);
        //setInterceptsMouseClicks(!isModuleSelected, true);
    }
}

void KrumModuleEditor::removeSettingsOverlay(bool keepSettings)
{
    editor.removeKeyboardListener(&parent);
    //settingsOverlay = nullptr;
    settingsOverlay.reset();
    cleanUpOverlay(keepSettings);
}


void KrumModuleEditor::showSettingsOverlay(bool selectOverlay)
{
    if (settingsOverlay != nullptr)
    {
        addAndMakeVisible(settingsOverlay.get());
        //addMouseListener(this, true);
        //setInterceptsMouseClicks(false, true);
        if (selectOverlay)
        {
            settingsOverlay->setOverlaySelected(true);
        }
        
        setModuleButtonsClickState(false);
        grabKeyboardFocus();
    }
    else
    {
        removeMouseListener(this);
    }

}

void KrumModuleEditor::cleanUpOverlay(bool keepSettings)
{
    buildModule();
    
    if (keepSettings)
    {
        editor.setKeyboardNoteColor(parent.info.midiNote, parent.info.moduleColor);
        moduleProcessor.sampler.updateModule(&parent);
    }

    setModuleButtonsClickState(true);
}

void KrumModuleEditor::setModuleButtonsClickState(bool isClickable)
{
    int numChildren = getNumChildComponents();

    for (int i = 0; i < numChildren; i++)
    {
        getChildComponent(i)->setInterceptsMouseClicks(isClickable, isClickable);
    }

}

int KrumModuleEditor::getModuleIndex()
{
    return parent.getModuleIndex();
}

void KrumModuleEditor::setModuleIndex(int newIndex)
{
    parent.setModuleIndex(newIndex);
}


int KrumModuleEditor::getModuleDisplayIndex()
{
    return parent.getModuleDisplayIndex();
}

void KrumModuleEditor::setModuleDisplayIndex(int newDisplayIndex)
{
    parent.setModuleDisplayIndex(newDisplayIndex);
}

void KrumModuleEditor::setModuleColor(juce::Colour newColor)
{
    parent.info.moduleColor = newColor;
}

juce::Colour KrumModuleEditor::getModuleColor()
{
    return parent.info.moduleColor;
}

int KrumModuleEditor::getModuleMidiNote()
{
    return parent.info.midiNote;
}

juce::String KrumModuleEditor::getModuleMidiNoteString(bool noteName)
{
    int noteNum = parent.info.midiNote;
    if (noteName)
    {
        return juce::MidiMessage::getMidiNoteName(noteNum, true, true, 3);
    }
    else
    {
        return juce::String(noteNum);
    }
}

void KrumModuleEditor::setModuleMidiNote(int newMidiNote)
{
    parent.setMidiTriggerNote(newMidiNote);
}

int KrumModuleEditor::getModuleMidiChannel()
{
    return parent.info.midiChannel;
}

void KrumModuleEditor::setModuleMidiChannel(int newMidiChannel)
{
    parent.setMidiTriggerChannel(newMidiChannel);
}

void KrumModuleEditor::setModulePlaying(bool isPlaying)
{
    parent.setModulePlaying(isPlaying);
}

bool KrumModuleEditor::isModulePlaying()
{
    return parent.info.modulePlaying;
}

void KrumModuleEditor::updateName()
{
    juce::String name = titleBox.getText();
    parent.setModuleName(name);
}

void KrumModuleEditor::reassignSliderAttachments()
{
    juce::String i = " " + juce::String(parent.info.index);

    volumeSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModuleGain_ID + i, volumeSlider));
    panSliderAttachment.reset(new SliderAttachment(*parent.parameters, TreeIDs::paramModulePan_ID + i, panSlider));
}

void KrumModuleEditor::updateBubbleComp(juce::Slider* slider, juce::Component* comp)
{
    auto bubbleComp = static_cast<juce::BubbleComponent*>(comp);
    if (bubbleComp != nullptr)
    {
        juce::Point<int> pos; 
        juce::BubbleComponent::BubblePlacement bubblePlacement = juce::BubbleComponent::above;
        auto area = getLocalBounds();
        if (slider->getSliderStyle() == juce::Slider::LinearVertical) 
        {
            pos = { area.getCentreX() /*+ 6*/, getMouseXYRelative().getY() - 5 };
            //bubblePlacement = juce::BubbleComponent::right;
        }
        else if (slider->getSliderStyle() == juce::Slider::LinearHorizontal)
        {
            int mouseX = getMouseXYRelative().getX();
            int leftBound = area.getX() + 20;
            int rightBound = area.getRight() - 20;
            if (mouseX < leftBound)
            {
                mouseX = leftBound;
            }
            if (mouseX > rightBound) 
            {
                mouseX = rightBound;
            }
            pos = {  mouseX, slider->getBounds().getCentreY() - 10 };
            //bubblePlacement = juce::BubbleComponent::above;
        }

        bubbleComp->setAllowedPlacement(bubblePlacement);
        bubbleComp->setPosition(pos, 0);
        
    }
    slider->setTooltip(slider->getTextFromValue(slider->getValue()));

}

int KrumModuleEditor::getAudioFileLengthInMs()
{
    return thumbnail.getTotalLength() * 1000;
}


void KrumModuleEditor::setKeyboardColor()
{
    setChildCompColors();
    int midiNote = parent.getMidiTriggerNote();

    if (editor.keyboard.isMidiNoteAssigned(midiNote))
    {
        editor.keyboard.removeMidiNoteColorAssignment(midiNote);
    }

    editor.setKeyboardNoteColor(midiNote, parent.info.moduleColor);
}

bool KrumModuleEditor::doesEditorWantMidi()
{
    return settingsOverlay != nullptr;
}

void KrumModuleEditor::handleMidi(int midiChannel, int midiNote)
{
    if (settingsOverlay != nullptr)
    {
        settingsOverlay->handleMidiInput(midiChannel, midiNote);
    }

}

void KrumModuleEditor::removeFromDisplay()
{
    editor.removeKeyboardListener(&parent);
    editor.keyboard.removeMidiNoteColorAssignment(parent.info.midiNote);
    editor.getModuleContainer().removeModuleEditor(this);
}

void KrumModuleEditor::triggerNoteOnInParent()
{
    parent.triggerNoteOn();
}

void KrumModuleEditor::triggerNoteOffInParent()
{
    parent.triggerNoteOff();
}
