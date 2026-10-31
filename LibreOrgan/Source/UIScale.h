#pragma once

#include <JuceHeader.h>
#if JUCE_LINUX
 #include "LinuxWindowPlacement.h"
#endif
#include <atomic>
#include <cmath>

//==============================================================================
// Size of the user interface: one factor for every window, button, slider and
// text, in per cent of the design size. The console was designed on a
// 2560 x 1440 screen at the system's 100 %; the first start estimates the
// factor from the screen so that the same amount of console fits a Full HD,
// QHD, 4K or Retina screen and the buttons keep a sensible size. The user
// overrides it in Configuration Editor > Console ("Interface size", settings
// key "uiScalePercent").
//
// How it is applied: every top-level window of the program (the main window,
// the second window, the modeless dialogs, the dialogs launched around the
// main component, alert boxes through the look and feel) reports this factor
// from Component::getDesktopScaleFactor(). JUCE then draws the window's
// content through a scale transform (vector text and shapes, as sharp as the
// system's own display scaling), and popup menus, tooltips and combo-box
// lists pick the factor up from the component they belong to. The global
// Desktop scale factor is NOT used: with it, JUCE 9 converts window positions
// twice on secondary screens and resizing a dialog there fails.
//
// Coordinates: a scaled window's own bounds are in its scaled space
// (screen pixels / factor); the screen areas JUCE reports (Displays) are
// unscaled. toWindowSpace() / toScreenSpace() convert between them.
//==============================================================================
namespace UIScale
{
    // The main window: every message box without a component of its own is
    // centred on it (set by Main.cpp when the window exists). 0.8.172.
    inline juce::Component::SafePointer<juce::Component> dialogAnchor;

    constexpr int minPercent = 50, maxPercent = 300;
    constexpr int estimateFloorPercent = 70;   // small laptop screens: rather scroll than shrink below this

    inline std::atomic<int>& percentRef() noexcept { static std::atomic<int> percent{ 100 }; return percent; }
    inline int clampPercent(int percent) noexcept { return juce::jlimit(minPercent, maxPercent, percent); }
    inline int percent() noexcept { return percentRef().load(std::memory_order_relaxed); }
    inline float factor() noexcept { return (float)percent() / 100.0f; }
    inline void set(int newPercent) noexcept { percentRef().store(clampPercent(newPercent), std::memory_order_relaxed); }

    // The screen the program should start on (0.8.248, the user: "on nenaskočí" - the fresh
    // start opened on the second monitor): what JUCE calls the primary display is not the
    // user's main one on every X11/Wayland desktop, so the screen under the mouse (where the
    // program was launched from) comes first, then the largest screen, then JUCE's primary.
    inline const juce::Displays::Display* startDisplay()
    {
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        const auto mouse = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().roundToInt();
        for (const auto& display : displays.displays)
            if (display.totalArea.contains(mouse)) return &display;
        const juce::Displays::Display* largest = nullptr;
        for (const auto& display : displays.displays)
            if (largest == nullptr || display.userBounds.getWidth() * display.userBounds.getHeight() > largest->userBounds.getWidth() * largest->userBounds.getHeight())
                largest = &display;
        return largest != nullptr ? largest : displays.getPrimaryDisplay();
    }

    // The percentage that makes the start screen hold what 2560 x 1440 holds at 100 %.
    inline int estimatePercent()
    {
        const auto* primary = startDisplay();
        if (primary == nullptr) return 100;
        const double width = primary->userBounds.getWidth();
        const double height = primary->userBounds.getHeight();
        if (width < 100.0 || height < 100.0) return 100;
        // The reference user area: 2560 wide, 1440 less the task bar.
        const double ratio = juce::jmin(width / 2560.0, height / 1392.0);
        const int rounded = (int)std::round(ratio * 100.0 / 5.0) * 5;
        return juce::jlimit(estimateFloorPercent, maxPercent, rounded);
    }

    // The saved value, or 0 when the settings do not have one yet (first start).
    inline int percentFromSettings(const juce::var& settings)
    {
        return settings.isObject() ? (int)settings.getProperty("uiScalePercent", 0) : 0;
    }

    inline juce::Rectangle<int> toWindowSpace(juce::Rectangle<int> screen) noexcept
    {
        return (screen.toFloat() / factor()).toNearestInt();
    }
    inline juce::Rectangle<int> toScreenSpace(juce::Rectangle<int> window) noexcept
    {
        return (window.toFloat() * factor()).toNearestInt();
    }
    inline juce::Point<int> toWindowSpace(juce::Point<int> screen) noexcept
    {
        return (screen.toFloat() / factor()).roundToInt();
    }
    inline juce::Point<int> toScreenSpace(juce::Point<int> window) noexcept
    {
        return (window.toFloat() * factor()).roundToInt();
    }

    // Alert boxes are created by the look and feel: the program's default look
    // and feel (set in Main.cpp) makes them windows with the interface size.
    struct AlertWindowScaled final : public juce::AlertWindow
    {
        using juce::AlertWindow::AlertWindow;
        float getDesktopScaleFactor() const override { return factor(); }
    };

    struct LookAndFeel final : public juce::LookAndFeel_V4
    {
        // Tooltips: the long explanations of the Configuration Editor need air
        // between the lines (the stock layout packs them), a wider box and a
        // left-aligned text.
        static juce::TextLayout layoutTooltip(const juce::String& text, juce::Colour colour)
        {
            juce::AttributedString s;
            s.setJustification(juce::Justification::left);
            s.setLineSpacing(5.0f);
            s.append(text, juce::Font(juce::FontOptions(14.0f)), colour);
            juce::TextLayout layout;
            layout.createLayoutWithBalancedLineLengths(s, 520.0f);
            return layout;
        }
        juce::Rectangle<int> getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override
        {
            const auto layout = layoutTooltip(tipText, juce::Colours::black);
            const int w = (int)(layout.getWidth() + 18.0f), h = (int)(layout.getHeight() + 12.0f);
            return juce::Rectangle<int>(screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h).constrainedWithin(parentArea);
        }
        void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
        {
            const juce::Rectangle<int> bounds(width, height);
            g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
            g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
            g.setColour(findColour(juce::TooltipWindow::outlineColourId));
            g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), 5.0f, 1.0f);
            layoutTooltip(text, findColour(juce::TooltipWindow::textColourId)).draw(g, bounds.toFloat().reduced(8.0f, 5.0f));
        }

        juce::AlertWindow* createAlertWindow(const juce::String& title, const juce::String& message,
            const juce::String& button1, const juce::String& button2, const juce::String& button3,
            juce::MessageBoxIconType iconType, int numButtons, juce::Component* associatedComponent) override
        {
            // Every box without a component of its own is centred on the main window
            // (0.8.172): JUCE centres it on the PRIMARY display otherwise, while the
            // program may stand on the second one.
            if (associatedComponent == nullptr && dialogAnchor != nullptr && dialogAnchor->isShowing()) associatedComponent = dialogAnchor.getComponent();
            // The same buttons and shortcuts as juce::LookAndFeel_V2::createAlertWindow.
            auto* aw = new AlertWindowScaled(title, message, iconType, associatedComponent);
           #if JUCE_LINUX
            // The window manager's own placement: a KDE Wayland desktop ignores where
            // a client puts its window and placed the box where it liked. As a
            // transient of the main window it is centred on it, on its display - the
            // hint must be there before the map, and the box was mapped at its birth.
            // The X window alone (the peer) is unmapped and mapped again, never the
            // JUCE component: a hidden modal component loses its modal state.
            if (dialogAnchor != nullptr && dialogAnchor->getPeer() != nullptr && aw->getPeer() != nullptr && aw->isVisible())
            {
                auto* peer = aw->getPeer();
                peer->setVisible(false);
                LinuxWindowPlacement::setTransientFor((unsigned long)(juce::pointer_sized_uint)peer->getNativeHandle(),
                                                      (unsigned long)(juce::pointer_sized_uint)dialogAnchor->getPeer()->getNativeHandle());
                peer->setVisible(true);
            }
           #endif
            if (numButtons == 1)
            {
                aw->addButton(button1, 0, juce::KeyPress(juce::KeyPress::escapeKey), juce::KeyPress(juce::KeyPress::returnKey));
            }
            else
            {
                const juce::KeyPress button1ShortCut((int)juce::CharacterFunctions::toLowerCase(button1[0]), 0, 0);
                juce::KeyPress button2ShortCut((int)juce::CharacterFunctions::toLowerCase(button2[0]), 0, 0);
                if (button1ShortCut == button2ShortCut) button2ShortCut = juce::KeyPress();
                if (numButtons == 2)
                {
                    aw->addButton(button1, 1, juce::KeyPress(juce::KeyPress::returnKey), button1ShortCut);
                    aw->addButton(button2, 0, juce::KeyPress(juce::KeyPress::escapeKey), button2ShortCut);
                }
                else if (numButtons == 3)
                {
                    aw->addButton(button1, 1, button1ShortCut);
                    aw->addButton(button2, 2, button2ShortCut);
                    aw->addButton(button3, 0, juce::KeyPress(juce::KeyPress::escapeKey));
                }
            }
            return aw;
        }
    };

    // After the factor changed: every open window keeps its place and size on
    // the screen (its content re-flows), and stays within its screen.
    inline void relayoutOpenWindows()
    {
        auto& desktop = juce::Desktop::getInstance();
        for (int i = desktop.getNumComponents(); --i >= 0;)
        {
            auto* window = desktop.getComponent(i);
            if (window == nullptr) continue;
            auto* peer = window->getPeer();
            if (peer == nullptr) continue;
            const float scale = window->getDesktopScaleFactor();
            if (scale <= 0.0f) continue;
            auto screenBounds = peer->getBounds();   // unscaled screen pixels, as Displays report them
            if (!peer->isFullScreen())
                if (const auto* display = desktop.getDisplays().getDisplayForRect(screenBounds))
                {
                    const auto area = display->userBounds.toNearestInt();
                    screenBounds.setSize(juce::jmin(screenBounds.getWidth(), area.getWidth()),
                                         juce::jmin(screenBounds.getHeight(), area.getHeight()));
                    screenBounds = screenBounds.constrainedWithin(area);
                }
            const auto wanted = (screenBounds.toFloat() / scale).toNearestInt();
            if (wanted == window->getBounds())
            {
                // Same logical size: the peer must still take the new factor.
                window->setBounds(wanted.expanded(1));
            }
            window->setBounds(wanted);
        }
    }
}
