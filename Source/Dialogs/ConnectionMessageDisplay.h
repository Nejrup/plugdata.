/*
 // Copyright (c) 2023 Timothy Schoen and Alex Mitchell
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Constants.h"
#include "LookAndFeel.h"
#include "../Connection.h"
#include "../PluginEditor.h"
#include "../CanvasViewport.h"

class ConnectionMessageDisplay
    : public Component
    , public MultiTimer {
public:
    ConnectionMessageDisplay()
    {
        setSize(36, 36);
        setVisible(false);
        // needed to stop the component from gaining mouse focus
        setInterceptsMouseClicks(false, false);
    }

    ~ConnectionMessageDisplay()
    {
    }

    /** Activate the current connection info display overlay, to hide give it a nullptr
     */
    void setConnection(Connection* connection, Point<int> screenPosition = { 0, 0 })
    {
        // multiple events can hide the display, so we don't need to do anything
        // if this object has already been set to null
        if (activeConnection == nullptr && connection == nullptr)
            return;

        activeConnection = SafePointer<Connection>(connection);
        if (activeConnection.getComponent()) {
            mousePosition = screenPosition;
            startTimer(MouseHoverDelay, mouseDelay);
            stopTimer(MouseHoverExitDelay);
            startTimer(RepaintTimer, 1000 / 60);
            updateTextString(true);
        } else {
            hideDisplay();
            // to copy tooltip behaviour, any successful interaction will cause the next interaction to have no delay
            mouseDelay = 0;
            stopTimer(MouseHoverDelay);
            startTimer(MouseHoverExitDelay, 500);
        }
    }

    CriticalSection& getLock()
    {
        return connectionMessageLock;
    }

private:
    void updateTextString(bool isHoverEntered = false)
    {
        messageItemsWithFormat.clear();

        auto haveMessage = true;
        auto textString = activeConnection->getMessageFormated();

        if (textString[0].isEmpty()) {
            haveMessage = false;
            textString = StringArray("no message yet");
        }

        auto fontStyle = haveMessage ? FontStyle::Semibold : FontStyle::Regular;
        auto textFont = Font(haveMessage ? Fonts::getSemiBoldFont() : Fonts::getCurrentFont());
        textFont.setSizeAndStyle(14, FontStyle::Regular, 1.0f, 0.0f);
        int stringWidth;
        int totalStringWidth = (8 * 2) + 4;
        String stringItem;
        bool firstOrLast = false;
        for (int i = 0; i < textString.size(); i++) {
            firstOrLast = i == 0 || i == textString.size() - 1 ? true : false;
            stringItem = textString[i];
            stringItem += firstOrLast ? "" : ",";
            // first item uses system font
            stringWidth = textFont.getStringWidth(stringItem);

            messageItemsWithFormat.add(TextStringWithMetrics(stringItem, fontStyle, stringWidth));

            // set up font for next item/s
            fontStyle = FontStyle::Monospace;
            textFont = Font(Fonts::getMonospaceFont());

            // calculate total needed width
            totalStringWidth += stringWidth + 4;
        }

        Rectangle<int> proposedPosition;
        // only make the size wider, to fit changing size of values
        if (totalStringWidth > getWidth() || isHoverEntered) {
            proposedPosition.setSize(totalStringWidth, 36);
            // make sure the proposed position is inside the viewport area
            proposedPosition.setCentre(getParentComponent()->getLocalPoint(nullptr, mousePosition).translated(0, -(getHeight() * 0.5)));
            auto activeCanvas = static_cast<PluginEditor*>(getParentComponent())->getCurrentCanvas();
            auto viewArea = getParentComponent()->getLocalArea(activeCanvas, activeCanvas->viewport->getViewArea() / getValue<float>(activeCanvas->zoomScale));
            constrainedBounds = proposedPosition.constrainedWithin(viewArea);
        }

        if (getBounds() != constrainedBounds)
            setBounds(constrainedBounds);

        repaint();
    }

    void hideDisplay()
    {
        stopTimer(RepaintTimer);
        setVisible(false);
    }

    void timerCallback(int timerID) override
    {
        switch (timerID) {
        case RepaintTimer: {
            if (activeConnection.getComponent()) {
                updateTextString();
            } else {
                hideDisplay();
            }
            break;
        }
        case MouseHoverDelay: {
            if (activeConnection.getComponent()) {
                updateTextString();
                setVisible(true);
            } else {
                hideDisplay();
            }
            break;
        }
        case MouseHoverExitDelay: {
            mouseDelay = 500;
            stopTimer(MouseHoverExitDelay);
            break;
        }
        }
    }

    void paint(Graphics& g) override
    {
        Path messageDisplay;
        auto internalBounds = getLocalBounds().reduced(8).toFloat();
        messageDisplay.addRoundedRectangle(internalBounds, Corners::defaultCornerRadius);

        if (cachedImage.isNull() || previousBounds != getBounds()) {
            cachedImage = { Image::ARGB, getWidth(), getHeight(), true };
            Graphics g2(cachedImage);

            StackShadow::renderDropShadow(g2, messageDisplay, Colour(0, 0, 0).withAlpha(0.3f), 6);
        }

        g.setColour(Colours::black);
        g.drawImageAt(cachedImage, 0, 0);

        g.setColour(findColour(PlugDataColour::outlineColourId));
        g.fillRoundedRectangle(internalBounds.expanded(1), Corners::defaultCornerRadius);
        g.setColour(findColour(PlugDataColour::dialogBackgroundColourId));
        g.fillRoundedRectangle(internalBounds, Corners::defaultCornerRadius);

        // indicator - TODO
        // if(activeConnection.getComponent()) {
        //    Path indicatorPath;
        //    indicatorPath.addPieSegment(circlePosition.x - circleRadius,
        //                          circlePosition.y - circleRadius,
        //                          circleRadius * 2.0f,
        //                          circleRadius * 2.0f, 0, (activeConnection->messageActivity * (1.0f / 12.0f)) * MathConstants<float>::twoPi, 0.5f);
        //    g.setColour(findColour(PlugDataColour::panelTextColourId));
        //    g.fillPath(indicatorPath);
        //}

        int startPostionX = 8 + 4;
        for (auto item : messageItemsWithFormat) {
            Fonts::drawStyledText(g, item.text, startPostionX, 0, item.width, getHeight(), findColour(PlugDataColour::panelTextColourId), item.fontStyle, 14, Justification::centredLeft);
            startPostionX += item.width + 4;
        }

        // used for cached background shadow
        previousBounds = getBounds();
    }

    static inline bool isShowing = false;

    struct TextStringWithMetrics {
        TextStringWithMetrics(String text, FontStyle fontStyle, int width)
            : text(text)
            , fontStyle(fontStyle)
            , width(width) {};
        String text;
        FontStyle fontStyle;
        int width;
    };
    Array<TextStringWithMetrics> messageItemsWithFormat;

    Component::SafePointer<Connection> activeConnection;
    int mouseDelay = 500;
    Point<int> mousePosition;
    enum TimerID { RepaintTimer,
        MouseHoverDelay,
        MouseHoverExitDelay };
    Rectangle<int> constrainedBounds = { 0, 0, 0, 0 };

    Point<float> circlePosition = { 8 + 4, 36 / 2 };
    float circleRadius = 3.0f;

    Image cachedImage;
    Rectangle<int> previousBounds;

    CriticalSection connectionMessageLock;
};