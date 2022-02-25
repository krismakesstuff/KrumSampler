/*
  ==============================================================================

    KrumLookAndFeel.h
    Created: 10 Mar 2021 1:48:30pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/*
* 
* Handles the drawing of many objects.
* 
*/


class KrumLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KrumLookAndFeel() 
    {
        juce::Typeface::Ptr tface = juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratLight_ttf, BinaryData::MontserratLight_ttfSize);
        setDefaultSansSerifTypeface(tface);
    }

  /*  juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override
    {
        static juce::Typeface::Ptr tface = juce::Typeface::createSystemTypefaceFor(BinaryData::MontserratRegular_ttf,
                                            BinaryData::MontserratRegular_ttfSize);

        DBG("Font: " + juce::String(wacky.get() ? "valid" : "null"));
        
        return wacky;
    }*/


    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour(slider.findColour(juce::Slider::trackColourId));
            g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, sliderPos - (float)x, (float)height - 1.0f)
                 /*vertical*/ : juce::Rectangle<float>((float)x + 0.5f, sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));
        }
        else
        {
            drawLinearSliderBackground(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
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
        juce::Colour gradCol1(juce::Colours::black);
        juce::Colour gradCol2(trackColour.overlaidWith(juce::Colour(0x06000000)));
        juce::Path indent;

        float cornerSize = 4.0f;

        if (slider.isHorizontal())
        {
            auto iy = height * 0.25f;
            juce::Rectangle<float> trackRect ((float)x, iy, (float)width , height * 0.50f);

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
            
            g.setColour(gradCol1);
            g.fillRoundedRectangle(trackRect, cornerSize);

        }
        else //vertical 
        {
            float trackWidth = width;// * 0.35f;
            //auto ix = /*(float)x + */(float)width * 0.5f;// -(sliderThumbRadius * 0.5f);
            float ix = bounds.getCentreX() - (trackWidth / 2);
            juce::Rectangle<float> trackRect (x + 5, (float)y - 5, trackWidth, (float)height + 5);

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
        float cornerSize = 2.0f;
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

            line.setStart({ (float)thumbX , (float)thumbY +(thumbH / 2) });
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
        
        juce::DropShadow ds{juce::Colours::black, dropRad, dropPoint};
        ds.drawForRectangle(g, thumb/*.withBottom(thumb.getBottom() + 3)*/);
        
        g.setColour(thumbColor);
        g.fillRoundedRectangle(thumb.toFloat(),cornerSize);

        
        g.setColour(thumbColor.darker(0.9f)/*.withAlpha(0.2f)*/);
        g.drawLine(line);

    }

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();

        /*if (auto pitchSlider = static_cast<KrumModuleEditor::PitchSlider*>(&slider))
        {
            layout.sliderBounds = bounds.withX(bounds.getX() - 20).withWidth(bounds.getWidth() + 40);
        }
        else*/
        {
            if (slider.isHorizontal())
            {
                layout.sliderBounds = bounds;//bounds.reduced(5, 0);
            }
            else if (slider.isBar())
            {
                layout.sliderBounds = bounds;
            }
            else
            {
                layout.sliderBounds = bounds;// .reduced(0, 10); //thumbnail height
            }
        }
        
        return layout;
    }

    //int getSliderThumbRadius(juce::Slider& slider) override
    //{
    //    if (slider.isHorizontal())
    //    {
    //        return 10;
    //    }

    //    return 20; // thumbW for now

    //}

    int getScrollbarButtonSize(juce::ScrollBar& scrollbar) override
    {
        return 20;
    }

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        juce::Rectangle<int> thumbBounds;

        if (isScrollbarVertical)
            thumbBounds = { x, thumbStartPosition, width, thumbSize };
        else
            thumbBounds = { thumbStartPosition, y, thumbSize, height };

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
        int, int, int, int, juce::ComboBox& box)
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


    void positionComboBoxText(juce::ComboBox& box, juce::Label& label)
    {
        label.setJustificationType(juce::Justification::centredLeft);
        label.setBounds(1, 1, box.getWidth() - 10, box.getHeight() - 2);
        label.setFont(getComboBoxFont(box));
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
            auto textColour = (textColourToUse == nullptr ? findColour(juce::PopupMenu::textColourId) : *textColourToUse);

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
                auto tick = getTickShape(1.0f);
                g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
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

        p.addRoundedRectangle(body.reduced(4), 1.0f);

        g.setColour(juce::Colours::black/*.withAlpha(0.5f)*/);
        g.fillPath(p);

        g.setColour(juce::Colours::white);
        g.strokePath(p, juce::PathStrokeType(0.5f));

    }

    void drawDrawableButton(juce::Graphics& g, juce::DrawableButton& button, bool highlighted, bool down) override
    {
        bool toggleState = button.getToggleState();

        float cornerSize = 3.0f;

        auto color = button.findColour(toggleState ? juce::DrawableButton::backgroundOnColourId
            : juce::DrawableButton::backgroundColourId);

        g.setColour(color);
        g.fillRoundedRectangle(button.getBounds().toFloat(), cornerSize);
        //g.fillAll();

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
