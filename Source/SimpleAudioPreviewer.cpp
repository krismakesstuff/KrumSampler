/*
  ==============================================================================

    SimpleAudioPreviewer.cpp
    Created: 2 Apr 2021 3:29:00pm
    Author:  krisc

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SimpleAudioPreviewer.h"

//==============================================================================
SimpleAudioPreviewer::SimpleAudioPreviewer(juce::AudioFormatManager* fm, juce::ValueTree& vt)
    : formatManager(fm), valueTree(vt)
{
    addAndMakeVisible(autoPlayToggle);
    autoPlayToggle.setButtonText("Auto-Play");
    autoPlayToggle.setToggleState(getSavedToggleState(), juce::dontSendNotification);
    autoPlayToggle.setTooltip("double-click files to preview, auto-play will preview as it's selected");
    autoPlayToggle.onStateChange = [this] { saveToggleState(); };

    addAndMakeVisible(volumeSlider);
    volumeSlider.setAlwaysOnTop(true);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setRange(0.01, 1.0);
    volumeSlider.setSkewFactor(0.7);
    volumeSlider.setDoubleClickReturnValue(true, 0.75f);
    
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setNumDecimalPlacesToDisplay(2);

    volumeSlider.setValue(getSavedPreviewerGainState(), juce::dontSendNotification);
    volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::cadetblue);
    volumeSlider.setTooltip("double-click files to preview, auto-play will preview as it's selected");
    
    volumeSlider.onValueChange = [this] { updateBubbleComp(&volumeSlider, volumeSlider.getCurrentPopupDisplay()); };
    volumeSlider.onDragEnd = [this] { setGain(); };
    volumeSlider.valueFromTextFunction = [this](juce::String(text)) { return fromText(text); };
    volumeSlider.textFromValueFunction = [this](double value) { return toText(value); };

    setPaintingIsUnclipped(true);
}

SimpleAudioPreviewer::~SimpleAudioPreviewer()
{
}

void SimpleAudioPreviewer::paint (juce::Graphics& g)
{
}

void SimpleAudioPreviewer::resized()
{
    auto area = getLocalBounds();

    autoPlayToggle.setBounds(area.withRight(area.getWidth() / 3));
    volumeSlider.setBounds(area.withLeft(area.getWidth() / 3).withY(area.getHeight()/3).withHeight(area.getHeight()/2));
}

bool SimpleAudioPreviewer::isAutoPlayActive()
{
    return autoPlayToggle.getToggleState();
}

void SimpleAudioPreviewer::renderPreviewer(juce::AudioBuffer<float>& outputBuffer)
{
    if (currentAudioFileSource)
    {
        juce::AudioBuffer<float> tempBuffer{ outputBuffer.getNumChannels(), outputBuffer.getNumSamples() };
        const juce::AudioSourceChannelInfo tempChannelInfo{ tempBuffer };

        currentAudioFileSource->getNextAudioBlock(tempChannelInfo);

        const juce::AudioSourceChannelInfo channelInfo{ outputBuffer };

        auto gain = getGain();

        for (int i = 0; i < outputBuffer.getNumChannels(); i++)
        {
            channelInfo.buffer->addFrom(i, 0, tempBuffer, i, 0, tempBuffer.getNumSamples(), gain);
        }

        if (currentAudioFileSource->getNextReadPosition() >= currentAudioFileSource->getTotalLength())
        {
            readyToPlayFile = false;
        }
    }
}

void SimpleAudioPreviewer::setGain()
{
    double newGain = volumeSlider.getValue();
    
    savePreviewerGainState();
    
    DBG("Previewer Gain: " + juce::String(newGain));
}

double SimpleAudioPreviewer::getGain()
{
    return volumeSlider.getValue();
}

juce::String SimpleAudioPreviewer::toText(double value)
{
    float db = juce::Decibels::gainToDecibels(value);
    juce::String retString;
    if (db > -0.05)
    {
        retString = "0 dB";
    }
    else if (db < -38)
    {
        retString = "-inf";
    }
    else 
    {
        retString = juce::String(db).dropLastCharacters(3) + " dB";
    }

    return retString;
}

float SimpleAudioPreviewer::fromText(juce::String text)
{
    if(text.compare("0 dB") != 0)
    {
        text = text.dropLastCharacters(3);
    }

    return juce::Decibels::decibelsToGain(text.getFloatValue());
}

void SimpleAudioPreviewer::updateBubbleComp(juce::Slider* slider, juce::Component* comp)
{
    auto bubbleComp = static_cast<juce::BubbleComponent*>(comp);
    if (bubbleComp != nullptr)
    {
        juce::Point<int> pos;
        juce::BubbleComponent::BubblePlacement bubblePlacement = juce::BubbleComponent::above;
        auto sliderStyle = slider->getSliderStyle();

        if (sliderStyle == juce::Slider::LinearVertical)
        {
            pos = { getLocalBounds().getCentreX() /*+ 6*/, getMouseXYRelative().getY() - 5 };
        }
        else if (sliderStyle == juce::Slider::LinearHorizontal)
        {
            int mouseX = getMouseXYRelative().getX();
            pos = {  mouseX, slider->getBounds().getCentreY() - 10 };
            bubblePlacement = mouseX > getLocalBounds().getRight() - 50 ? juce::BubbleComponent::left : juce::BubbleComponent::right;
        }

        bubbleComp->setAllowedPlacement(bubblePlacement);
        bubbleComp->setPosition(pos, 0);
        bubbleComp->setPaintingIsUnclipped(true);
    }

    slider->setTooltip(slider->getTextFromValue(slider->getValue()));
}

void SimpleAudioPreviewer::loadFile(juce::File& fileToPreview)
{
    if (fileToPreview.existsAsFile())
    {
        currentFormatReader.reset(formatManager->createReaderFor(fileToPreview));
    }

    if (currentFormatReader)
    {
        currentAudioFileSource.reset(new juce::AudioFormatReaderSource(currentFormatReader.get(), false));
        currentAudioFile = fileToPreview;
    }
    else
    {
        DBG("reader is NULL");
    }
}

juce::AudioFormatManager* SimpleAudioPreviewer::getFormatManager()
{
    return formatManager;
}

void SimpleAudioPreviewer::saveToggleState()
{
    auto globalTree = valueTree.getChildWithName("GlobalSettings");
    auto autoPTree = globalTree.getChildWithName("PreviewerAutoPlay");

    autoPTree.setProperty("value", autoPlayToggle.getToggleState() ? juce::var(1) : juce::var(0), nullptr);
}

bool SimpleAudioPreviewer::getSavedToggleState()
{
    auto globalTree = valueTree.getChildWithName("GlobalSettings");
    auto autoPTree = globalTree.getChildWithName("PreviewerAutoPlay");
    bool state = (int)autoPTree.getProperty("value") > 0;
    return state;
}

void SimpleAudioPreviewer::savePreviewerGainState()
{
    auto globalTree = valueTree.getChildWithName("GlobalSettings");
    auto prevTree = globalTree.getChildWithName("PreviewerGain");

    prevTree.setProperty("value", juce::var(volumeSlider.getValue()), nullptr);
}

float SimpleAudioPreviewer::getSavedPreviewerGainState()
{
    auto globalTree = valueTree.getChildWithName("GlobalSettings");
    auto prevTree = globalTree.getChildWithName("PreviewerGain");

    return (float)prevTree.getProperty("value");
}

void SimpleAudioPreviewer::refreshSettings()
{
    volumeSlider.setValue(getSavedPreviewerGainState(), juce::dontSendNotification);
    autoPlayToggle.setToggleState(getSavedToggleState(), juce::dontSendNotification);
}

bool SimpleAudioPreviewer::wantsToPlayFile()
{
    return readyToPlayFile;
}

void SimpleAudioPreviewer::setWantsToPlayFile(bool wantsToPlay)
{
    readyToPlayFile = wantsToPlay;
}
