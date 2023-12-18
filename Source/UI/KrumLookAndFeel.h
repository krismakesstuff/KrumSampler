/*
  ==============================================================================

    KrumLookAndFeel.h
    Created: 10 Mar 2021 1:48:30pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//#include "../UI/FileBrowser/KrumFileBrowser.h"

/*
* 
* Handles the drawing of many objects.
* 
* I have a couple classes that inherit from KrumLookAndFeel. These are mainly used to draw the Module's Sliders different from the rest of the sliders
* 
*/

//FileBrowser Dimensions
namespace Dimensions
{
    const int rowHeight = 22;
    const float rowTextScalar = 0.70f;
    const int rowHeaderHeight = 32;
    const int panelHeaderCornerSize = 2.0f;
    const int titleH = 21;
    const int fileIconSize = 19;
    const float fileIconAlpha = 0.6f;
    const int locationTabDepth = 22;
    const int currentPathHeight = 25;
    const int previewerH = 45;
}


namespace Colors
{
    //general app 
    inline juce::Colour getbgColor(){return juce::Colours::black.brighter(0.12f);}

    inline juce::Colour getFontColor(){return juce::Colours::white.withAlpha(0.5f);}
    inline juce::Colour getHighlightFontColor(){return getFontColor().darker(0.1f); }
    inline juce::Colour getHighlightColor(){return juce::Colours::black.brighter(0.15f);}

    inline juce::Colour getSectionOutlineColor(){return juce::Colours::grey;}
    inline juce::Colour getSectionBGColor(){return juce::Colours::black.brighter(0.075f);}
    
    inline juce::Colour getOutputSliderBGColor() { return getSectionBGColor(); }
    inline juce::Colour getOutputThumbColor() { return juce::Colours::cadetblue; }
    inline juce::Colour getOutputTrackColor() { return juce::Colours::darkgrey; }

    inline juce::Colour getDropShadowColor() { return juce::Colours::black; }

    //browser
    inline juce::Colour getBrowserFontColor() { return juce::Colours::white.withAlpha(0.5f); }
    inline juce::Colour getBrowserBGColor() { return Colors::getSectionBGColor(); }
    inline juce::Colour getBrowserPathBoxColor() { return juce::Colours::black.brighter(0.15f); }


    //contextual
    inline juce::Colour getAddAnimationColor() { return juce::Colour::fromRGB(67, 170, 139); } //zomp
    inline juce::Colour getRemoveAnimationColor() { return juce::Colour::fromRGB(249, 65, 68); } //redSalsa
    inline juce::Colour getCanDropFileColor() { return getAddAnimationColor(); }

    
    //concertina panel header 
    inline juce::Colour getPanelHeaderBGColor() { return juce::Colours::grey.darker(0.8); }
    inline juce::Colour getPanelHeaderMouseOverBGColor() { return getPanelHeaderBGColor().brighter(0.19f); }
    inline juce::Colour getPanelHeaderFontColor() { return juce::Colours::black; }
    inline juce::Colour getPanelHeaderLineColor() { return juce::Colours::darkgrey; }


    //modules 
    inline juce::Colour getModuleOutlineColor() { return juce::Colours::white; }
    inline juce::Colour getModuleDefaultColor() { return juce::Colours::grey.brighter(0.1f); }
    inline juce::Colour getModuleBGColor() { return juce::Colours::black; }
    inline juce::Colour getModuleSelectedOutlineColor() { return juce::Colours::white.withAlpha(0.65f); }
    inline juce::Colour getModuleHoverOutlineColor() { return juce::Colours::white.withAlpha(0.79f); }
    inline juce::Colour getModuleMultiControlAcitveColor() { return juce::Colours::red.brighter().withAlpha(0.9f); }
    inline juce::Colour getModuleMidiListeningColor() { return getModuleMultiControlAcitveColor(); }
    inline juce::Colour getModuleMuteActiveColor() { return juce::Colours::red.withAlpha(0.7f); }
    inline juce::Colour getModulePlayingHightlightColor() { return  juce::Colours::white.withAlpha(0.4f); }
    inline juce::Colour getModuleOutputMenuBG() { return juce::Colours::black.withAlpha(0.7f); }
    inline juce::Colour getBackOutlineColor() { return juce::Colours::darkgrey; }


}

namespace ColorPaletteColors
{
    const juce::Colour redSalsa = juce::Colour::fromRGB(249, 65, 68);
    const juce::Colour orangePeel = juce::Colour::fromRGB(248, 164, 65);
    const  juce::Colour mangoTango = juce::Colour::fromRGB(249, 132, 74);
    const juce::Colour zomp = juce::Colour::fromRGB(67, 170, 139);
    const juce::Colour cgBlue = juce::Colour::fromRGB(39, 125, 161);
    const juce::Colour defaultColor = Colors::getModuleDefaultColor();

    const juce::Array<juce::Colour> colorArray{ redSalsa, orangePeel, mangoTango, zomp, cgBlue, defaultColor };
    
}



class KrumLookAndFeel : public juce::LookAndFeel_V4
{
public:

    KrumLookAndFeel() 
    {
        //setDefaultSansSerifTypeface(getMontRegularTypeface());
    }

//    juce::Typeface::Ptr getMontLightTypeface()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratLight_ttf, BinaryData::MontserratLight_ttfSize);
//    }
//
//    juce::Typeface::Ptr getMontMediumTypeface()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratMedium_ttf, BinaryData::MontserratMedium_ttfSize);
//    }
//
//    juce::Typeface::Ptr getMontRegularTypeface()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratRegular_ttf, BinaryData::MontserratRegular_ttfSize);
//    }
//
//    juce::Typeface::Ptr getMontBoldTypeface()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize);
//    }
//
//    juce::Typeface::Ptr getMontBlackTypeface()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratBlack_ttf, BinaryData::MontserratBlack_ttfSize);
//    }
//
//    juce::Typeface::Ptr getMontExtraBoldTypeFace()
//    {
//        return juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratExtraBold_ttf, BinaryData::MontserratExtraBold_ttfSize);
//    }

    //juce::Typeface::Ptr getFileBrowserFont()
//    {
//        return getMontRegularTypeface();
//    }

  /*  juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override
    {
        static juce::Typeface::Ptr tface = juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratRegular_ttf,
                                            BinaryData::MontserratRegular_ttfSize);

        DBG("Font: " + juce::String(wacky.get() ? "valid" : "null"));
        
        return wacky;
    }*/

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();
        layout.sliderBounds = bounds.reduced(getSliderThumbRadius(slider));
        return layout;
    }

    int getSliderThumbRadius(juce::Slider& slider) override
    {

        if (slider.isBar())
        {
            return 2;
        }

        if (slider.isHorizontal())
        {
            //return slider.getHeight() * 0.9f;
            return 5;
        }
        else
        {
            return 8;
        }
    }

    //lineStart is x or y, depending on slider orientation
    void drawVolumeLines(juce::Graphics& g, float lineStart, float lineDistance, float startDecibel, float endDecibel, float decibelDistance, juce::Slider& slider)
    {

        float lineThickness = 0.5f;

        if (decibelDistance > 0)
        {
            decibelDistance *= -1;
        }

        //currentdb is an index value, not the slider position. 
        for (float currentdb = startDecibel; currentdb >= endDecibel; currentdb += decibelDistance)
        {
            float currentLinePos = getSliderDecibelPosition(slider, currentdb);
            
            if (slider.isHorizontal())
            {
                g.drawLine({ currentLinePos, lineStart, currentLinePos, lineStart + lineDistance }, lineThickness);
            }      
            else
            {
                g.drawLine({ lineStart, currentLinePos, lineStart + lineDistance, currentLinePos }, lineThickness);

            }
        }
    }

    float getSliderDecibelPosition(juce::Slider& slider, float decibel)
    {
        return slider.getPositionOfValue(juce::Decibels::decibelsToGain(decibel));
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.isBar()) 
        {
            
            //if (slider.getSliderStyle() == juce::Slider::LinearBar) //pitchSlider
            //{

            //}
            //else if (slider.getSliderStyle() == juce::Slider::LinearBarVertical) // moduleClipGainSlider
            //{
                g.setColour(slider.findColour(juce::Slider::trackColourId));
                //g.setColour(juce::Colours::white);
                g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, sliderPos - (float)x, (float)height - 1.0f)
                            /*vertical*/ : juce::Rectangle<float>((float)x + 0.5f, sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));

            //}
        }
        else
        {
            drawLinearSliderBackground(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
    
    //Vertical is Output Slider, Horizontal is Previewer Slider
    void drawLinearSliderBackground(juce::Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, float minSliderPos, float maxSliderPos,
                                    const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        //const float sliderThumbRadius = (float)(getSliderThumbRadius(slider) - 2);
        juce::Rectangle<int> bounds{ x, y, width, height };

        float sliderPorp;
        if (slider.isHorizontal())
        {
            sliderPorp = sliderPos / (width);
        }
        else
        {
           sliderPorp = sliderPos / (maxSliderPos - (y + 10));
        }

        if (sliderPorp < 0.0f)
        {
            sliderPorp = 0.01f;
        }
        else if (sliderPorp > 1.0f)
        {
            sliderPorp = 0.9999f;
        }

        const juce::Colour trackColour(slider.findColour(juce::Slider::trackColourId));
        //juce::Colour gradCol1(juce::Colours::black.withAlpha(0.3f));
        juce::Colour gradCol1(juce::Colours::transparentBlack);
        juce::Colour gradCol2(trackColour.overlaidWith(juce::Colour(0x06000000)));
        juce::Path indent;

        float cornerSize = 4.0f;

        if (slider.isHorizontal())
        {
            auto iy = height * 0.25f;
            juce::Rectangle<float> trackRect ((float)x, iy, (float)width , height /** 0.50f*/);

            /*juce::ColourGradient horzRGrade (gradCol1, trackRect.getCentreX(), iy, gradCol2, trackRect.getRight()-2, iy, false);
            juce::ColourGradient horzLGrade (gradCol2, trackRect.getX(), iy, gradCol1, trackRect.getCentreX(), iy, false);
            
            indent.addRoundedRectangle(trackRect, cornerSize);
            g.setColour(gradCol1);
            g.fillPath(indent);

            horzRGrade.addColour(sliderPorp, gradCol1);
            g.setGradientFill(horzRGrade);
            g.fillRoundedRectangle(trackRect.withLeft(trackRect.getCentreX()), cornerSize);

            horzLGrade.addColour(sliderPorp, gradCol1);
            g.setGradientFill(horzLGrade);
            g.fillRoundedRectangle(trackRect.withRight(trackRect.getCentreX()), cornerSize);*/
            
            //g.setColour(Colors::browserBGColor);
            //g.fillRoundedRectangle(trackRect, cornerSize);

        }
        else //vertical 
        {
            float trackWidth = width;// * 0.35f;
            //auto ix = /*(float)x + */(float)width * 0.5f;// -(sliderThumbRadius * 0.5f);
            float ix = bounds.getCentreX() - (trackWidth / 2);
            juce::Rectangle<float> trackRect (x , (float)y, trackWidth, (float)height );

            /*juce::ColourGradient vertGrade(gradCol1, ix, y, gradCol2, ix, trackRect.getBottom(), false);
            vertGrade.addColour(sliderPorp, gradCol1);
            
            g.setGradientFill(vertGrade);
            indent.addRoundedRectangle(trackRect, cornerSize);
            g.fillPath(indent);*/

            g.setColour(gradCol1);
            g.fillRoundedRectangle(trackRect, cornerSize);
        }

        //g.setColour(trackColour.contrasting(0.6f));
        //g.strokePath(indent, juce::PathStrokeType(0.5f));
    }

    //Vertical is Output Slider, Horizontal is Previewer Slider
    void drawLinearSliderThumb(juce::Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        float cornerSize = 1.0f;
        juce::Line<float> line;
        juce::Point<int> dropPoint;

        int dropRad = 5;
        int thumbX, thumbY, thumbH, thumbW;
        
        //Output Slider
        if (style == juce::Slider::LinearVertical)
        {
            thumbH = getSliderThumbRadius(slider); //height * 0.07;// : height * 0.085f;
            thumbW = width * 0.85f;

            thumbX = x - 4;
            thumbY = sliderPos - thumbH /2;

            auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
            juce::Rectangle<int> thumb(thumbX, thumbY, thumbW, thumbH);

            //juce::Rectangle<int> dbLineRect{ thumbX + thumbW + 5, y, width - thumbW, height };
            juce::Rectangle<int> dbLineRect{ thumbW + 10, (int)maxSliderPos , width + 5, (int)minSliderPos - getSliderThumbRadius(slider) };
            float textHeight = 11.0f;

            //g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.setColour(Colors::getFontColor().darker(0.5f));
            //drawVolumeLines(g, thumbX + thumbW , width - thumbW, 2.0f, -60.0f, 0.5f, slider)
            drawVolumeLines(g, (float)dbLineRect.getX() - 2, (float)dbLineRect.getWidth() - 4, 2.0f, -60.0f, -0.5f, slider);

            //g.setColour(juce::Colours::white);
            g.setColour(Colors::getFontColor());
            g.setFont(textHeight);

            auto twoDbPos = getSliderDecibelPosition(slider, 2.0f);  //+ thumbOffset;
            g.drawFittedText("+2", dbLineRect.withY(twoDbPos - textHeight / 2).withX(dbLineRect.getX() + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto zeroDbPos = getSliderDecibelPosition(slider, 0.0f); //+ thumbOffset;
            g.drawFittedText("0", dbLineRect.withY(zeroDbPos - textHeight / 2).withX(dbLineRect.getX() + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n5DbPos = getSliderDecibelPosition(slider, -5.0f); //+thumbOffset;
            g.drawFittedText("-5", dbLineRect.withY(n5DbPos - textHeight / 2).withX(dbLineRect.getX() + 0).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n10DbPos = getSliderDecibelPosition(slider, -10.0f);// +thumbOffset;
            g.drawFittedText("-10", dbLineRect.withY(n10DbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n20DbPos = getSliderDecibelPosition(slider, -20.0f);// +thumbOffset;
            g.drawFittedText("-20", dbLineRect.withY(n20DbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);

            g.setColour(thumbColor);
            g.fillRoundedRectangle(thumb.toFloat(), cornerSize);
        }
        else // horizontal, preview slider
        {
            thumbH = height + 1 ; //height - 3;
            thumbW = getSliderThumbRadius(slider);

            thumbX = sliderPos;
            thumbY = y - 5;

            auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
            juce::Rectangle<int> thumb(thumbX, thumbY, thumbW, thumbH);

            juce::Rectangle<int> dbLineRect{ x, y + thumbH , width, (int)(height * 0.5f)};
            float textHeight = 9.0f;

            g.setColour(Colors::getFontColor().darker(0.99f));
            drawVolumeLines(g, dbLineRect.getY(), dbLineRect.getHeight(), 2.0f, -60.0f, 0.5f, slider);

            g.setColour(Colors::getFontColor());
            g.setFont(textHeight);

            auto twoDbPos = getSliderDecibelPosition(slider, 2.0f);  //+ thumbOffset;
            g.drawFittedText("+2", dbLineRect.withX(twoDbPos - 5).withY(height + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto zeroDbPos = getSliderDecibelPosition(slider, 0.0f); //+ thumbOffset;
            g.drawFittedText("0", dbLineRect.withX(zeroDbPos).withY(height + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n5DbPos = getSliderDecibelPosition(slider, -5.0f); //+thumbOffset;
            g.drawFittedText("-5", dbLineRect.withX(n5DbPos - 3).withY(height + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n10DbPos = getSliderDecibelPosition(slider, -10.0f);// +thumbOffset;
            g.drawFittedText("-10", dbLineRect.withX(n10DbPos - 6).withY(height + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);

            auto n20DbPos = getSliderDecibelPosition(slider, -20.0f);// +thumbOffset;
            g.drawFittedText("-20", dbLineRect.withX(n20DbPos - 6).withY(height + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);


            g.setColour(thumbColor);
            g.fillRoundedRectangle(thumb.toFloat(), cornerSize);

            /*g.setColour(juce::Colours::red);
            g.fillRect(x, y, width, height);*/
        }

    }

    int getScrollbarButtonSize(juce::ScrollBar& scrollbar) override
    {
        return 20;
    }

    int getDefaultScrollbarWidth() override
    {
        return 50;
    }

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        juce::Rectangle<int> thumbBounds;

        if (isScrollbarVertical)
            thumbBounds = { x, thumbStartPosition, width, thumbSize };
        else
            thumbBounds = { thumbStartPosition, y + 2, thumbSize, height -4 };

        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRect(x, y, width, height);
        
        auto c = juce::Colours::darkgrey/*.darker()*/;
        g.setColour(isMouseOver ? c.brighter(0.25f) : c);
        g.fillRect(thumbBounds.reduced(1));


    }

    void drawScrollbarButton(juce::Graphics& g, juce::ScrollBar& scrollbar, int width, int height, int buttonDirection, bool isScrollbarVertical,
                                bool isMouseOverButton, bool isButtonDown) override
    {
        g.setColour(juce::Colours::black);
        g.fillRect(0, 0, width, height);

        switch (buttonDirection)
        {
        case (0):
        {
            //up
            break;
        }
        case(1):
        {
            //right
            juce::Point<float> topPoint{ width / 3.0f, 0.0f };
            juce::Point<float> bottomPoint{width/3.0f, float(height)};
            juce::Point<float> rightSidePoint{float(width), height/2.0f};


            juce::Path triPath;
            triPath.addTriangle(topPoint, rightSidePoint, bottomPoint);
            
            g.setColour(juce::Colours::darkgrey);
            g.fillPath(triPath);

            break;
        }
        case(2):
        {
            //down
            break;
        }
        case(3):
        {
            //left
            break;
        }

        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
        int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        juce::Rectangle<int> boxBounds(0, 0, width, height);

        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

        bool popUp = box.isPopupActive();

        int arrowW = width * 0.2f;
        juce::Rectangle<int> arrowZone(width - arrowW, 0, arrowW, height);
        juce::Path path;
        path.startNewSubPath((float)arrowZone.getX() + 3.0f, (float)arrowZone.getCentreY() - 2.0f);
        path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f);
        path.lineTo((float)arrowZone.getRight() - 3.0f, (float)arrowZone.getCentreY() - 2.0f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));

        if (popUp)
        {
            auto pathCentre = path.getBounds().getCentre();
            float rotation = juce::MathConstants<float>::pi;

            g.fillPath(path, juce::AffineTransform::rotation(rotation, pathCentre.getX(), pathCentre.getY()));
        }
        else
        {
            g.fillPath(path);
        }
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setJustificationType(juce::Justification::centredLeft);
        label.setBounds(0, 0, box.getWidth(), box.getHeight());
        //label.setFont(getComboBoxFont(box));
        //label.setFont(getMontMediumTypeface());
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        //g.setColour(findColour(juce::ComboBox::ColourIds::backgroundColourId));
        g.setColour(juce::Colours::black);
        g.fillRect(0, 0, width, height);

        g.setColour(juce::Colours::darkgrey);
        g.drawRect(0, 0, width, height);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        const bool isSeparator, const bool isActive,
        const bool isHighlighted, const bool isTicked,
        const bool hasSubMenu, const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* const textColourToUse) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));

            g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
            g.fillRect(r.removeFromTop(1));
        }
        else
        {
            auto textColour = findColour(juce::PopupMenu::textColourId);

            auto r = area.reduced(1);
            if (isHighlighted && isActive)
            {
                g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
                g.fillRect(r);

                g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
            }
            else
            {
                g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
            }

            r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

            auto font = getPopupMenuFont();
            auto maxFontHeight = (float)r.getHeight() / 1.3f;

            if (font.getHeight() > maxFontHeight)
                font.setHeight(maxFontHeight);

            g.setFont(font);

            auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

            if (icon != nullptr)
            {
                icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
                r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
            }
            else if (isTicked)
            {
                //auto tick = getTickShape(1.0f);
                juce::Path circlePath;
                iconArea.reduce(3, 3);
                circlePath.addEllipse(iconArea.getX() - 3, iconArea.getY() + 1.5f, iconArea.getWidth(), iconArea.getWidth());
                
                g.fillPath(circlePath);
                //g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
            }

            if (hasSubMenu)
            {
                auto arrowH = 0.6f * getPopupMenuFont().getAscent();

                auto x = static_cast<float> (r.removeFromRight((int)arrowH).getX());
                auto halfH = static_cast<float> (r.getCentreY());

                juce::Path path;
                path.startNewSubPath(x, halfH - arrowH * 0.5f);
                path.lineTo(x + arrowH * 0.6f, halfH);
                path.lineTo(x, halfH + arrowH * 0.5f);

                g.strokePath(path, juce::PathStrokeType(2.0f));
            }

            r.removeFromRight(3);
            g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

            /*g.setColour(juce::Colours::white);
            juce::Line<float> bottomLine { area.getBottomLeft().toFloat(), area.getBottomRight().toFloat() };
            g.drawLine(bottomLine);*/

            if (shortcutKeyText.isNotEmpty())
            {
                auto f2 = font;
                f2.setHeight(f2.getHeight() * 0.75f);
                f2.setHorizontalScale(0.95f);
                g.setFont(f2);

                g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
            }
        }
    }

    void drawBubble(juce::Graphics& g, juce::BubbleComponent& bubble, const juce::Point<float>& tip, const juce::Rectangle<float>& body) override
    {
        juce::Path p;

        p.addRoundedRectangle(body.reduced(4), 5.0f);

        g.setColour(juce::Colours::black/*.withAlpha(0.5f)*/);
        g.fillPath(p);

        g.setColour(bubble.findColour(juce::BubbleComponent::outlineColourId));
        g.strokePath(p, juce::PathStrokeType(0.75f));

    }

    void drawDrawableButton(juce::Graphics& g, juce::DrawableButton& button, bool highlighted, bool down) override
    {
        bool toggleState = button.getToggleState();

        float cornerSize = 3.0f;

        auto color = button.findColour(toggleState ? juce::DrawableButton::backgroundOnColourId
                     : juce::DrawableButton::backgroundColourId);
        
        button.getLookAndFeel().drawButtonBackground(g, button, color, highlighted, down);

        g.setColour(color);
        //g.setColour(juce::Colours::red);

        g.fillRoundedRectangle(button.getBounds().toFloat(), cornerSize);

        const int textH = (button.getStyle() == juce::DrawableButton::ImageAboveTextLabel)
                            ? juce::jmin(16, button.proportionOfHeight(0.25f)) : 0;

        if (textH > 0)
        {
            g.setFont((float)textH);

            g.setColour(button.findColour(toggleState ? juce::DrawableButton::textColourOnId
                : juce::DrawableButton::textColourId)
                .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.4f));

            g.drawFittedText(button.getButtonText(),
                2, button.getHeight() - textH - 1,
                button.getWidth() - 4, textH,
                juce::Justification::centred, 1);
        }
    }

   /* void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

        juce::Colour baseColour;

        auto textButton = static_cast<juce::TextButton*>(&button);

        if (textButton != nullptr)
        {
            baseColour = juce::Colours::black.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);
        }
        else
        {
            auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);
        }

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour(baseColour);

        auto flatOnLeft = button.isConnectedOnLeft();
        auto flatOnRight = button.isConnectedOnRight();
        auto flatOnTop = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
        {
            juce::Path path;
            path.addRoundedRectangle(bounds.getX(), bounds.getY(),
                bounds.getWidth(), bounds.getHeight(),
                cornerSize, cornerSize,
                !(flatOnLeft || flatOnTop),
                !(flatOnRight || flatOnTop),
                !(flatOnLeft || flatOnBottom),
                !(flatOnRight || flatOnBottom));

            g.fillPath(path);

            g.setColour(button.findColour(juce::ComboBox::outlineColourId));
            g.strokePath(path, juce::PathStrokeType(1.0f));
        }
        else
        {
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(juce::Colours::white);
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }



    }*/

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumLookAndFeel)
};

//===========================================================================================================

class VolumeLookAndFeel : public KrumLookAndFeel
{
public:
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();
        layout.sliderBounds = bounds.reduced(getSliderThumbRadius(slider));
        return layout;
    }

    int getSliderThumbRadius(juce::Slider& slider) override
    {
        if (slider.isHorizontal())
        {
            return 5;
        }
        else
        {
            return 7;
        }
    }


    void drawLinearSliderBackground(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        //const float sliderThumbRadius = (float)(getSliderThumbRadius(slider) - 2);
        juce::Rectangle<int> bounds{ x, y, width, height };

        float sliderPorp;
        if (slider.isHorizontal())
        {
            sliderPorp = sliderPos / (width);
        }
        else
        {
            sliderPorp = sliderPos / (maxSliderPos/* - (y + 10)*/);
        }

        if (sliderPorp < 0.0f)
        {
            sliderPorp = 0.01f;
        }
        else if (sliderPorp > 1.0f)
        {
            sliderPorp = 0.9999f;
        }

        const juce::Colour trackColour(slider.findColour(juce::Slider::trackColourId));
        bool defColor = trackColour == Colors::getModuleDefaultColor();
            
        juce::Colour gradCol1(defColor ? trackColour.darker(0.5f) : trackColour.darker().withSaturation(0.65f));
        juce::Colour gradCol2(defColor ? trackColour : trackColour.darker(0.7).withSaturation(0.65f));

        juce::Path indent;

        float cornerSize = 4.0f;

        if (slider.isHorizontal()) 
        {
            auto iy = height * 0.25f;
            juce::Rectangle<float> trackRect((float)x, y, (float)width, height);

            g.setColour(juce::Colours::grey);
            g.fillRoundedRectangle(trackRect, cornerSize);

        }
        else //vertical , module gain 
        {
            float trackWidth = width;// * 0.35f;
            //auto ix = /*(float)x + */(float)width * 0.5f;// -(sliderThumbRadius * 0.5f);
            float ix = bounds.getCentreX() - (trackWidth/* / 2*/);
            juce::Rectangle<float> trackRect(x, (float)y, trackWidth, (float)height);


            juce::ColourGradient vertGrade(gradCol1, ix, y, gradCol2, ix, trackRect.getBottom(), false);
            //vertGrade.addColour(0.5, trackColour.brighter(0.2f));

            g.setGradientFill(vertGrade);
            //indent.addRoundedRectangle(trackRect, cornerSize);
            //g.fillPath(indent);

            //g.setColour(gradCol2);
            //g.setColour(trackColour);
            //g.drawRoundedRectangle(bounds.expanded(getSliderThumbRadius(slider)).toFloat(), cornerSize, 2.5);
            //g.fillRoundedRectangle(bounds.expanded(getSliderThumbRadius(slider)).toFloat(), cornerSize);
            g.fillRoundedRectangle(bounds.expanded(getSliderThumbRadius(slider)).withTrimmedRight(21).toFloat().withTop(sliderPos), cornerSize);
        }

        //g.setColour(trackColour.contrasting(0.6f));
        //g.strokePath(indent, juce::PathStrokeType(0.5f));

    }

    

    void drawLinearSliderThumb(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        float cornerSize = 2.0f;
        juce::Line<float> line;
        juce::Point<int> dropPoint;

        int dropRad = 5;
        int thumbX, thumbY, thumbH, thumbW;

        if (style == juce::Slider::LinearVertical)
        {
            thumbH = getSliderThumbRadius(slider) + 1; //height * 0.07;// : height * 0.085f;
            //thumbH = 5; //height * 0.07;// : height * 0.085f;
            thumbW = 42; //width * 0.65f;
            //thumbW = width + 10/** 0.5f*/;
            //thumbX = (x + width * 0.5f) - (thumbW * 0.5f);
            //thumbX = (x - thumbH/ 2);
            thumbX = (x - 8);
            thumbY = sliderPos - thumbH / 2;

            line.setStart({ (float)thumbX , (float)thumbY + (thumbH / 2) });
            line.setEnd({ (float)thumbX + thumbW , (float)thumbY + (thumbH / 2) });

            dropPoint.setXY(2, 2);
        }
        else // horizontal
        {
            thumbH = height * 0.7f; //height - 3;
            thumbW = 5;

            thumbX = sliderPos - 5;
            thumbY = y + 5;

            line.setStart({ (float)thumbX + (thumbW / 2), (float)thumbY });
            line.setEnd({ (float)thumbX + (thumbW / 2), (float)thumbY + thumbH });

            dropPoint.setXY(1, 1);
            dropRad = 3;
        }

        auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
        juce::Rectangle<int> thumb(thumbX, thumbY, thumbW, thumbH);

        //juce::DropShadow ds{ juce::Colours::black, dropRad, dropPoint };
        //ds.drawForRectangle(g, thumb/*.withBottom(thumb.getBottom() + 3)*/);

        //g.setColour(thumbColor);


        /*g.setColour(juce::Colours::black);
        g.fillRect(dbLineRect);*/

        
        //vertical
        //juce::Rectangle<int> dbLineRect{thumbW + 10, (int)maxSliderPos , width + 5, (int)minSliderPos - getSliderThumbRadius(slider)};
        juce::Rectangle<int> dbLineRect{48, (int)maxSliderPos , width + 5, (int)minSliderPos/* - getSliderThumbRadius(slider)*/};
        
        g.setColour(slider.findColour(juce::Slider::ColourIds::trackColourId).darker(0.99f));
        //drawVolumeLines(g, (float)dbLineRect.getX() - 5, (float)dbLineRect.getWidth() - 5, 2.0f, -50.0f, -0.5f, slider);
        drawVolumeLines(g, (float)dbLineRect.getX() - 5, (float)dbLineRect.getWidth() + 7, 20.0f, -50.0f, -0.5f, slider);
        
        g.setColour(thumbColor);
        //g.setColour(juce::Colours::black);
        
        g.fillRoundedRectangle(thumb.toFloat(), cornerSize);

        //float thumbOffset = thumbH / 2;
        float thumbOffset = 0;//getSliderThumbRadius(slider) / 2;
        int textHeight = 12;
        g.setFont( (float)textHeight );
        
        auto twentyDbPos = getSliderDecibelPosition(slider, 20.0f) + thumbOffset;
        g.drawFittedText("+20", dbLineRect.withY(twentyDbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);
        
        auto fifteenDbPos = getSliderDecibelPosition(slider, 15.0f) + thumbOffset;
        g.drawFittedText("+15", dbLineRect.withY(fifteenDbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);
        
        auto tenDbPos = getSliderDecibelPosition(slider, 10.0f) + thumbOffset;
        g.drawFittedText("+10", dbLineRect.withY(tenDbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);
        
        auto fiveDbPos = getSliderDecibelPosition(slider, 5.0f) + thumbOffset;
        g.drawFittedText("+5", dbLineRect.withY(fiveDbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);

        auto zeroDbPos = getSliderDecibelPosition(slider, 0.0f) + thumbOffset;
        g.drawFittedText("0", dbLineRect.withY(zeroDbPos - textHeight / 2).withX(dbLineRect.getX() + 1).withHeight(textHeight), juce::Justification::centredLeft, 1);
       
        auto n5DbPos = getSliderDecibelPosition(slider, -5.0f) + thumbOffset;
        g.drawFittedText("-5", dbLineRect.withY(n5DbPos - textHeight / 2).withX(dbLineRect.getX() - 2).withHeight(textHeight), juce::Justification::centredLeft, 1);

        auto n10DbPos = getSliderDecibelPosition(slider, -10.0f) + thumbOffset;
        g.drawFittedText("-10", dbLineRect.withY(n10DbPos - textHeight / 2).withX(dbLineRect.getX() - 3).withHeight(textHeight), juce::Justification::centredLeft, 1);

        auto n20DbPos = getSliderDecibelPosition(slider, -20.0f) + thumbOffset;
        g.drawFittedText("-20", dbLineRect.withY(n20DbPos - textHeight / 2).withX(dbLineRect.getX() - 3).withHeight(textHeight), juce::Justification::centredLeft, 1);
        
        auto n50DbPos = getSliderDecibelPosition(slider, -50.0f) + thumbOffset;
        g.drawFittedText("-50", dbLineRect.withY(n50DbPos - textHeight / 2).withX(dbLineRect.getX() - 3).withHeight(textHeight), juce::Justification::centredLeft, 1);

    }
};


//int decibelDistance = 0.2f;

//int startPos = slider.getPositionOfValue(juce::Decibels::decibelsToGain(1.8f));
//int endPos = slider.getPositionOfValue(juce::Decibels::decibelsToGain(0.8f));

////int lineDistance = endPos + startPos / ;
//int currentLine = startPos;

//for (; currentLine > endPos; currentLine += decibelDistance)
//{
//    //drawLine
//    juce::Line<float> line{ (float)dbLineRect.getX(), (float)currentLine, (float)dbLineRect.getWidth(), (float)currentLine };
//    g.drawLine(line, lineThickness);
//}
//===========================================================================================================

class PanLookAndFeel : public KrumLookAndFeel
{
public:

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();
        layout.sliderBounds = bounds.reduced(getSliderThumbRadius(slider), 0);
        return layout;
    }

    int getSliderThumbRadius(juce::Slider& slider) override
    {
        if (slider.isHorizontal())
        {
            //return slider.getHeight() * 0.9f;
            return 2;
        }
        else
        {
            return 7;
        }
    }

    void drawLinearSliderBackground(juce::Graphics& g, int x, int y, int width, int height,
                                    float sliderPos, float minSliderPos, float maxSliderPos,
                                    const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        //const float sliderThumbRadius = (float)(getSliderThumbRadius(slider) - 2);
        juce::Rectangle<int> bounds{ x, y, width, height };

        //float sliderPorp;
        /*if (slider.isHorizontal())
        {
            sliderPorp = sliderPos / (width);
        }
        else
        {
            sliderPorp = sliderPos / (maxSliderPos - (y + 10));
        }

        if (sliderPorp < 0.0f)
        {
            sliderPorp = 0.01f;
        }
        else if (sliderPorp > 1.0f)
        {
            sliderPorp = 0.9999f;
        }*/

        const juce::Colour trackColour(slider.findColour(juce::Slider::trackColourId));
        juce::Colour textColor = slider.findColour(juce::Slider::ColourIds::textBoxTextColourId);
        //juce::Colour gradCol1(juce::Colours::black.withAlpha(0.4f));
        //juce::Colour gradCol1(juce::Colours::red);
        
        
        juce::Colour gradCol1(trackColour.darker(0.7));
        juce::Colour gradCol2(trackColour.darker());
        juce::Path indent;

        juce::Rectangle<float> trackRect{};
        int mid = width / 2;
        float midLineThick = 1.0f;
        
        
        if (sliderPos < mid)
        {
            trackRect = bounds.withLeft(sliderPos).withRight(mid).toFloat();
            juce::Colour temp{ gradCol1 };
            gradCol1 = gradCol2;
            gradCol2 = temp;
            
        }
        else if(sliderPos > mid)
        {
            trackRect = bounds.withLeft(mid - midLineThick).withRight(sliderPos).toFloat();
        }


        float cornerSize = 4.0f;

        if (slider.isHorizontal()) //module pan
        {
            //auto iy = height * 0.25f;
            //juce::Rectangle<float> trackRect((float)x, iy, (float)width, height * 0.50f);
            //juce::Rectangle<float> trackRect = bounds.toFloat();

            juce::ColourGradient hGrade(gradCol1, trackRect.getX(), y, gradCol2, trackRect.getRight(), y, false);
            //g.setColour(trackColour);
            g.setGradientFill(hGrade);
            g.fillRoundedRectangle(trackRect, cornerSize);
             
            //int zeroX = slider.getPositionOfValue(0);
            int zeroX = bounds.getCentreX() - 3;
            juce::Line<int> zeroLine{ zeroX, y + 2, zeroX, height - 4 };
            

            g.setColour(textColor);
                     
            //g.drawLine(zeroLine.toFloat(), midLineThick);
            
            g.setFont( 12.0f );
            g.drawFittedText("L", {x + getSliderThumbRadius(slider), y, 10,height}, juce::Justification::centred, 1);
            g.drawFittedText("R", { width - 10, y, 10,height }, juce::Justification::centred, 1);
            //g.drawFittedText("L", trackRect.withX(cornerSize).withY(cornerSize / 2).withWidth(cornerSize * 2).withHeight(trackRect.getHeight() * 0.55f).toNearestInt(), juce::Justification::centred, 1);
            //g.drawFittedText("R", trackRect.withX(trackRect.getRight() - (cornerSize * 3)).withY(cornerSize / 2).withWidth(cornerSize * 2).withHeight(trackRect.getHeight() * 0.55f).toNearestInt(), juce::Justification::centred, 1);
        }
        else //vertical 
        {
            float trackWidth = width;// * 0.35f;
            //auto ix = /*(float)x + */(float)width * 0.5f;// -(sliderThumbRadius * 0.5f);
            //float ix = bounds.getCentreX() - (trackWidth / 2);
            trackRect = juce::Rectangle<float>{(float) x - 2, (float)y - 5, (float)trackWidth + 4, (float)height + 5 };

            /*juce::ColourGradient vertGrade(gradCol1, ix, y, gradCol2, ix, trackRect.getBottom(), false);
            vertGrade.addColour(sliderPorp, gradCol1);

            g.setGradientFill(vertGrade);
            indent.addRoundedRectangle(trackRect, cornerSize);
            g.fillPath(indent);*/

            g.setColour(gradCol1);
            g.fillRoundedRectangle(trackRect, cornerSize);
        }

        //g.setColour(trackColour.contrasting(0.6f));
        //g.strokePath(indent, juce::PathStrokeType(0.5f));

    }


    void drawLinearSliderThumb(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        //TODO: make EditorDimensions::cornerSize namespace accessible from here? review
        float cornerSize = 3.0f;
        juce::Line<float> line;
        juce::Point<int> dropPoint;

        int dropRad = 5;
        int thumbX, thumbY, thumbH, thumbW;

        if (style == juce::Slider::LinearVertical)
        {
            thumbH = 7; //height * 0.07;// : height * 0.085f;
            thumbW = 33; //width * 0.65f;

            thumbX = (x + width * 0.5f) - (thumbW * 0.5f);
            thumbY = sliderPos;

            line.setStart({ (float)thumbX , (float)thumbY + (thumbH / 2) });
            line.setEnd({ (float)thumbX + thumbW , (float)thumbY + (thumbH / 2) });

            dropPoint.setXY(2, 2);
        }
        else // horizontal
        {
            thumbH = height /** 0.9f*/; //height - 3;
            thumbW = 5;

            thumbX = sliderPos - thumbW / 2;
            thumbY = y;

            line.setStart({ (float)thumbX + (thumbW / 2), (float)thumbY });
            line.setEnd({ (float)thumbX + (thumbW / 2), (float)thumbY + thumbH });

            dropPoint.setXY(1, 1);
            dropRad = 3;
        }

        auto thumbColor = slider.findColour(juce::Slider::ColourIds::thumbColourId);
        juce::Rectangle<int> thumb(thumbX, thumbY, thumbW, thumbH);

        //juce::DropShadow ds{ Colors::getDropShadowColor(), dropRad, dropPoint};
        //ds.drawForRectangle(g, thumb/*.withBottom(thumb.getBottom() + 3)*/);

        g.setColour(thumbColor);
        g.fillRoundedRectangle(thumb.toFloat(), cornerSize);

        //g.setColour(thumbColor.darker(0.9f)/*.withAlpha(0.2f)*/);
        //g.drawLine(line);

    }
};

class PitchSliderLookAndFeel : public KrumLookAndFeel
{
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        //g.setColour(juce::Colours::white);
        float cornerSize = 2.0f;

        juce::Rectangle<int> backgroundRect{ x, y, width, height };

        float mid = (width/2) + 2;

        if (sliderPos > mid)
        {
            backgroundRect.setTop(mid);
            backgroundRect.setBottom(sliderPos);
        }
        else
        {
            backgroundRect.setTop(sliderPos);
            backgroundRect.setBottom(mid);
        }
            
    
        g.fillRoundedRectangle(backgroundRect.toFloat(), cornerSize);
        //g.fillRect(backgroundRect.toFloat());

    }


};

class FileBrowserLookAndFeel : public KrumLookAndFeel
{
public:

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
        int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        juce::Rectangle<int> boxBounds(0, 0, width, height);

        //g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.setColour(Colors::getBrowserPathBoxColor());
        g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

        //g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        //g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

        bool popUp = box.isPopupActive();

        int arrowW = width * 0.085f;
        juce::Rectangle<int> arrowZone(width - arrowW, 0, arrowW, height);
        juce::Path path;
        path.startNewSubPath((float)arrowZone.getX() + 3.0f, (float)arrowZone.getCentreY() - 2.0f);
        path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f);
        path.lineTo((float)arrowZone.getRight() - 3.0f, (float)arrowZone.getCentreY() - 2.0f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));

        if (popUp)
        {
            auto pathCentre = path.getBounds().getCentre();
            float rotation = juce::MathConstants<float>::pi;

            g.fillPath(path, juce::AffineTransform::rotation(rotation, pathCentre.getX(), pathCentre.getY()));
        }
        else
        {
            g.fillPath(path);
        }
    }

    int getDefaultScrollbarWidth() override
    {
        return 5;
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setJustificationType(juce::Justification::centred);
        label.setBounds(1, 0, box.getWidth() - 10, box.getHeight());
        //label.setFont(getMontBoldTypeface());
        label.setFont(label.getHeight() * 0.8f);
        //label.setFont(getMontBoldTypeface());
        //label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::lightgrey.darker(0.3f).withAlpha(0.7f));
        label.setColour(juce::Label::ColourIds::textColourId, Colors::getBrowserFontColor());
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        //g.setColour(findColour(juce::ComboBox::ColourIds::backgroundColourId));
        g.setColour(Colors::getBrowserBGColor());
        g.fillRect(0, 0, width, height);

        g.setColour(juce::Colours::darkgrey);
        g.drawRect(0, 0, width, height);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        const bool isSeparator, const bool isActive,
        const bool isHighlighted, const bool isTicked,
        const bool hasSubMenu, const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* const textColourToUse) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));

            //g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
            g.setColour(Colors::getFontColor());
            g.fillRect(r.removeFromTop(1));
        }
        else
        {
            auto textColour = findColour(juce::PopupMenu::textColourId);

            auto r = area.reduced(1);
            if (isHighlighted && isActive)
            {
                //g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
                g.setColour(juce::Colours::grey.withAlpha(0.2f));
                g.fillRect(r);

                g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
                
                //g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
                //g.setColour(juce::Colours::red);
            }
            else
            {
                g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
            }

            r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

            auto font = getPopupMenuFont();
            auto maxFontHeight = (float)r.getHeight() - 3;

            if (font.getHeight() > maxFontHeight)
                font.setHeight(maxFontHeight);

            g.setFont(font);

            auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

            if (icon != nullptr)
            {
                icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
                r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
            }
            else if (isTicked)
            {
                //auto tick = getTickShape(1.0f);
                juce::Path circlePath;
                iconArea.reduce(3, 3);
                circlePath.addEllipse(iconArea.getX() - 3, iconArea.getY() + 1.5f, iconArea.getWidth(), iconArea.getWidth());

                g.fillPath(circlePath);
                //g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
            }

            if (hasSubMenu)
            {
                auto arrowH = 0.6f * getPopupMenuFont().getAscent();

                auto x = static_cast<float> (r.removeFromRight((int)arrowH).getX());
                auto halfH = static_cast<float> (r.getCentreY());

                juce::Path path;
                path.startNewSubPath(x, halfH - arrowH * 0.5f);
                path.lineTo(x + arrowH * 0.6f, halfH);
                path.lineTo(x, halfH + arrowH * 0.5f);

                g.strokePath(path, juce::PathStrokeType(2.0f));
            }

            r.removeFromRight(3);
            g.drawFittedText(text, r, juce::Justification::centredLeft, 1, 1.0f);

            /*g.setColour(juce::Colours::white);
            juce::Line<float> bottomLine { area.getBottomLeft().toFloat(), area.getBottomRight().toFloat() };
            g.drawLine(bottomLine);*/

            if (shortcutKeyText.isNotEmpty())
            {
                auto f2 = font;
                f2.setHeight(f2.getHeight() * 0.75f);
                f2.setHorizontalScale(0.95f);
                g.setFont(f2);

                g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
            }
        }
    }

    void getIdealPopupMenuItemSize(const juce::String& text, const bool isSeparator,
                                int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
    {
        if (isSeparator)
        {
            idealWidth = 50;
            idealHeight = 8;
        }
        else
        {
            auto font = getPopupMenuFont();

            //if (standardMenuItemHeight > 0 && font.getHeight() > (float)standardMenuItemHeight / 1.3f)
              //  font.setHeight((float)standardMenuItemHeight / 1.3f);

            //idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : juce::roundToInt(font.getHeight() * 1.3f);
            //idealWidth = font.getStringWidth(text) + idealHeight * 2;

            idealHeight = 17;
            idealWidth = 50;
            //font.setHeight(10.0f);
        }
    }

    void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName) override
    {
        //g.setColour(juce::Colours::darkgrey.darker(0.99f));
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRect(area);

        g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
        g.drawFittedText(sectionName, area.reduced(2), juce::Justification::centredLeft, 1, 1.0f);

    }

    

    void drawFileBrowserRow(juce::Graphics& g, int width, int height,
        const juce::File& file, const juce::String& filename, juce::Image* icon,
        const juce::String& fileSizeDescription,
        const juce::String& fileTimeDescription,
        bool isDirectory, bool isItemSelected,
        int /*itemIndex*/, juce::DirectoryContentsDisplayComponent& dcc) override
    {
        //auto fileListComp = dynamic_cast<juce::Component*> (&dcc);

        bool audioFile = file.getFileExtension().compareIgnoreCase(".wav") == 0;

        juce::Rectangle<int> area = { 0, 0, width, height };

        if (isItemSelected)
        {
            g.setColour(Colors::getHighlightColor());
        }
        else
        {
            g.setColour(Colors::getBrowserBGColor());
        }

        g.fillRect(area);
        
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        
        if (auto* d = isDirectory ? getDefaultFolderImage()
                      : audioFile ? getAudioFileImage() : getDefaultDocumentFileImage())
        {

            d->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(),
                juce::RectanglePlacement::stretchToFit, Dimensions::fileIconAlpha);
        }

        if (isItemSelected)
            g.setColour(Colors::getHighlightFontColor());
        else
            g.setColour(Colors::getBrowserFontColor());

        //g.setFont(getFileBrowserFont());
        //g.setFont(getMontRegularTypeface());
        g.setFont((float)height * Dimensions::rowTextScalar);
       // g.drawFittedText(filename, area.withX(Dimensions::fileIconSize + 5), juce::Justification::centredLeft, 1);
        g.drawText(filename, area.withX(Dimensions::fileIconSize + 5), juce::Justification::centredLeft, true);

        
    }

    void drawTreeviewPlusMinusBox(juce::Graphics& g, const juce::Rectangle<float>& bounds,
        juce::Colour backgroundColour, bool isOpen, bool isMouseOver) override
    {

    /*    Path p;
        p.addTriangle(0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);

        g.setColour(backgroundColour.contrasting().withAlpha(isMouseOver ? 0.5f : 0.3f));
        g.fillPath(p, p.getTransformToScaleToFit(area.reduced(2, area.getHeight() / 4), true));*/

        g.setColour(Colors::getBrowserBGColor());
        g.fillRect(bounds);
        
        juce::Path path;
        auto area = bounds.reduced(7, 5);
        g.setColour(juce::Colours::grey);

        if (!isOpen)
        {
            path.addTriangle(area.getTopLeft(), { area.getRight(), area.getCentreY() }, area.getBottomLeft());
            g.strokePath(path, juce::PathStrokeType(1.0f));
        }
        else
        {
            path.addTriangle(area.getBottomLeft(), { area.getRight(), area.getCentreY() - 2 }, area.getBottomRight());
            g.strokePath(path, juce::PathStrokeType(1.0f));
            g.fillPath(path);
        }


    }

    const juce::Drawable* getDefaultDocumentFileImage() override
    {
        return fileIconImage.get();
    }
    
    const juce::Drawable* getDefaultFolderImage() override
    {
        return folderIconImage.get();
    }

    const juce::Drawable* getAudioFileImage()
    {
        return audioFileIconImage.get();
    }

    std::unique_ptr<juce::Drawable> fileIconImage = juce::Drawable::createFromImageData(BinaryData::file_white_24dp_svg, BinaryData::file_white_24dp_svgSize);
    std::unique_ptr<juce::Drawable> folderIconImage = juce::Drawable::createFromImageData(BinaryData::folder_white_24dp_svg, BinaryData::folder_white_24dp_svgSize);
    std::unique_ptr<juce::Drawable> audioFileIconImage = juce::Drawable::createFromImageData(BinaryData::audio_file_white_24dp_svg, BinaryData::audio_file_white_24dp_svgSize);

};





//===========================================================================================================
