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

    // 0.9.134 BAR SLIDERS (the user: a track with a thumb and "a text frame somewhere beside it" looked poor; "one
    // element instead of three"). Every HORIZONTAL slider of the program is one bar: its name on the left (the
    // property "barLabel"; without it the value alone, centred - a table's column header or a label beside it names
    // it), its value on the right, the fill running from the slider's default (its double-click value; without one:
    // from the minimum) to the value, and a bright line where the value is - the "thumb" that tells a slider from a
    // display. A row too narrow for that sets "barLines" = 2 on ALL its sliders alike (the name small above the
    // value) and "barUseShort" for the short name ("barShort"). VERTICAL sliders are the same bar upright, without
    // text (their value box stays below them). The gestures: BarValueTyping in UIControls.h.
    inline bool isBarSlider(const juce::Slider& slider) noexcept
    {
        const auto style = slider.getSliderStyle();
        return style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar;
    }
    inline juce::Rectangle<int> barAreaOf(const juce::Slider& slider) noexcept
    {
        const bool twoLines = (int)slider.getProperties().getWithDefault("barLines", 1) == 2;
        const auto bounds = slider.getLocalBounds();
        // "barHeight": a row that wants its bars as tall as its neighbours (the top row: the height of the display).
        const int tall = (int)slider.getProperties().getWithDefault("barHeight", twoLines ? 44 : 28);
        return bounds.withSizeKeepingCentre(bounds.getWidth(), juce::jmin(bounds.getHeight(), tall));
    }

    // 0.9.136 THE WHEEL IN A SCROLLING AREA (the user: scrolling the Configuration Editor with the wheel changed
    // whatever slider came under the mouse; a modifier key and an extra click were both refused): inside anything
    // that scrolls - the Editor, the scrolled dialogs, the stop workspace with its open voicings - THE WHEEL
    // SCROLLS, also when a slider comes under the mouse. A control takes the wheel only when the MOUSE ITSELF moved
    // onto it since the last scroll (a control the page carried under a resting mouse stays deaf), has been on it
    // for 0.4 s, and the page has stood for 0.4 s. A control that listens shows it: a thin orange frame = "the wheel sets this one now".
    // One object for the whole program (a member of MainComponent) listening to the mouse of every window. A slider
    // in a scrolling area is made deaf when it is PAINTED (LookAndFeel::drawLinearSlider calls prepare), so no
    // place that creates a slider can forget it; a control that handles the wheel itself asks listens().
    class WheelGate final : public juce::MouseListener, private juce::Timer
    {
    public:
        WheelGate() { instance() = this; juce::Desktop::getInstance().addGlobalMouseListener(this); }
        ~WheelGate() override
        {
            juce::Desktop::getInstance().removeGlobalMouseListener(this);
            stopTimer();
            instance() = nullptr;
        }
        static WheelGate*& instance() { static WheelGate* gate = nullptr; return gate; }
        static constexpr double kStandMs = 400.0, kHandOnWheelMs = 300.0;
        static constexpr int kMovePixels = 4;
        // The control the wheel would act on at this component: a slider, or a component marked "wheelGated".
        static juce::Component* targetOf(juce::Component* c)
        {
            for (; c != nullptr; c = c->getParentComponent())
                if (dynamic_cast<juce::Slider*>(c) != nullptr || c->getProperties().contains("wheelGated")) return c;
            return nullptr;
        }
        static bool inScrollingArea(const juce::Component& c)
        {
            for (auto* p = c.getParentComponent(); p != nullptr; p = p->getParentComponent())
                if (dynamic_cast<juce::Viewport*>(p) != nullptr) return true;
            return false;
        }
        // For a control that handles the wheel itself: may it take the wheel now?
        static bool listens(const juce::Component& c)
        {
            if (!inScrollingArea(c)) return true;
            auto* gate = instance();
            return gate == nullptr || gate->armed.getComponent() == &c;
        }
        // Called whenever a slider is painted.
        static void prepare(juce::Slider& slider)
        {
            auto* gate = instance();
            if (gate == nullptr || !inScrollingArea(slider)) return;
            const bool on = gate->armed.getComponent() == &slider;
            if (slider.isScrollWheelEnabled() != on) slider.setScrollWheelEnabled(on);
        }
        void mouseMove(const juce::MouseEvent& e) override { moved(e); }
        void mouseEnter(const juce::MouseEvent& e) override { moved(e); }
        void mouseDrag(const juce::MouseEvent& e) override { moved(e); }
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails&) override
        {
            auto* target = targetOf(e.eventComponent);
            if (target != nullptr && (target == armed.getComponent() || !inScrollingArea(*target))) return;   // the wheel set a control
            lastScrollMs = juce::Time::getMillisecondCounterHiRes();   // a scroll
            movedSinceScroll = false;
            lastPosition = e.getScreenPosition();
            disarm();
        }
    private:
        void moved(const juce::MouseEvent& e)
        {
            // The mouse ITSELF: a page that moves under a resting mouse sends moves at the same place.
            // 0.9.137 (the user: a slider under the resting mouse listened 0.4 s after the scroll stopped): turning
            // the wheel shakes the mouse by a few pixels, and 3 px counted as a move. What comes within 0.3 s of a
            // wheel notch is the hand on the wheel (it only moves the place the distance is counted from); after
            // that a move of 4 px or more is the user's.
            const auto place = e.getScreenPosition();
            const double now = juce::Time::getMillisecondCounterHiRes();
            if (now - lastScrollMs < kHandOnWheelMs) lastPosition = place;
            else if (place.getDistanceFrom(lastPosition) >= kMovePixels)
            {
                // (the user: "často člověk trochu myší pohne než začne scrollovat" and "když se scrolling zastaví nad
                // sliderem, a myš se trochu pohne - čekat 0.4 s": every move, a small one too, starts the 0.4 s anew -
                // the control listens once the mouse has come to rest on it)
                overSinceMs = now;
                movedSinceScroll = true;
                lastPosition = place;
            }
            look(targetOf(e.eventComponent));
        }
        void look(juce::Component* target)
        {
            // 0.9.137 (the user: "after mouse move over slider, it should wait 0.4s before activating the slider"): the
            // mouse must have been on the control for 0.4 s as well - crossing a slider on the way and turning the
            // wheel at once scrolls.
            if (target != under.getComponent()) { disarm(); under = target; overSinceMs = juce::Time::getMillisecondCounterHiRes(); }
            if (under == nullptr) { stopTimer(); return; }
            if (armed.getComponent() != under.getComponent())
            {
                const bool gated = inScrollingArea(*under);
                auto* slider = dynamic_cast<juce::Slider*>(under.getComponent());
                if (!gated) { if (slider == nullptr || slider->isScrollWheelEnabled()) arm(under, false); }
                else if (const double now = juce::Time::getMillisecondCounterHiRes(); movedSinceScroll && now - lastScrollMs >= kStandMs && now - overSinceMs >= kStandMs) arm(under, true);
            }
            if (!isTimerRunning()) startTimer(80);
        }
        void timerCallback() override
        {
            // (the component under the mouse as JUCE keeps it from the mouse events - no question to the system)
            look(targetOf(juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse()));
        }
        void arm(juce::Component* c, bool gated)
        {
            armed = c;
            armedGated = gated;
            c->getProperties().set("wheelArmed", true);
            if (gated) if (auto* slider = dynamic_cast<juce::Slider*>(c)) slider->setScrollWheelEnabled(true);
            c->repaint();
        }
        void disarm()
        {
            if (auto* c = armed.getComponent())
            {
                c->getProperties().remove("wheelArmed");
                if (armedGated) if (auto* slider = dynamic_cast<juce::Slider*>(c)) slider->setScrollWheelEnabled(false);
                c->repaint();
            }
            armed = nullptr;
        }
        juce::Component::SafePointer<juce::Component> under, armed;
        juce::Point<int> lastPosition;
        double lastScrollMs = 0.0, overSinceMs = 0.0;
        bool movedSinceScroll = true, armedGated = false;
    };

    // 0.9.136 (the user: "Tool tip se zobrazí těsně u myši, takže většinou zakryje ovládací prvek"): the tip is
    // placed by the CONTROL, not by the pointer - under its bottom edge, and above its top edge when there is no
    // room below. The tooltip window sets this before it shows a tip: the position it passes is the tip's top left
    // corner (under the control), 'above' the way up from there to where the tip's BOTTOM edge would stand.
    struct TipPlacement { bool own = false; int above = 0; };
    inline TipPlacement& tipPlacement() { static TipPlacement placement; return placement; }

    // The value typed into the bar itself: ONE left click without a drag opens a field over the bar once the
    // double-click time has passed (so a double click still puts the default back); Enter or a click elsewhere takes
    // the value, Escape leaves it. One object for the whole program (a member of MainComponent): it listens to the
    // mouse of every window.
    class BarValueTyping final : public juce::MouseListener, private juce::Timer
    {
    public:
        BarValueTyping() { juce::Desktop::getInstance().addGlobalMouseListener(this); }
        ~BarValueTyping() override
        {
            juce::Desktop::getInstance().removeGlobalMouseListener(this);
            stopTimer();
            if (editor != nullptr) { editor->onFocusLost = nullptr; editor.reset(); }
        }
        void mouseDown(const juce::MouseEvent& event) override
        {
            stopTimer();
            pending = nullptr;
            if (editor != nullptr && event.eventComponent != editor.get() && !editor->isParentOf(event.eventComponent)) finish(true);
        }
        void mouseUp(const juce::MouseEvent& event) override
        {
            auto* slider = dynamic_cast<juce::Slider*>(event.eventComponent);
            if (slider == nullptr || !isBarSlider(*slider) || !slider->isEnabled() || !slider->isTextBoxEditable()) return;
            if (!event.mods.isLeftButtonDown() || event.mods.isPopupMenu() || event.mods.isAnyModifierKeyDown()) return;
            if (!event.mouseWasClicked() || event.getNumberOfClicks() != 1) return;
            if (slider->getProperties().contains("barNoTyping")) return;
            pending = slider;
            startTimer(juce::MouseEvent::getDoubleClickTimeout() + 40);
        }
        // For a test hook: the field of this slider now (what the click does after its wait).
        void openFor(juce::Slider& slider) { stopTimer(); pending = nullptr; open(slider); if (editor != nullptr) editor->onFocusLost = nullptr; }   // a test's window has no focus: the field stays
        bool isOpen() const noexcept { return editor != nullptr; }
        void typeForTest(const juce::String& text) { if (editor != nullptr) { editor->setText(text, false); finish(true); } }   // ... and Enter

    private:
        void timerCallback() override
        {
            stopTimer();
            if (auto* slider = pending.getComponent(); slider != nullptr && slider->isShowing() && slider->isEnabled()) open(*slider);
            pending = nullptr;
        }
        void open(juce::Slider& slider)
        {
            finish(true);
            target = &slider;
            startText = slider.getTextFromValue(slider.getValue());
            editor = std::make_unique<juce::TextEditor>();
            editor->setFont(juce::FontOptions(juce::jlimit(9.0f, 16.0f, (float)(double)slider.getProperties().getWithDefault("barValueSize", 13.0)), juce::Font::bold));
            editor->setJustification(juce::Justification::centred);
            editor->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xffffa500));
            editor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffffa500));
            editor->setText(startText, false);
            editor->setBounds(barAreaOf(slider).reduced(1));
            slider.getProperties().set("barTyping", true);
            slider.addAndMakeVisible(*editor);
            editor->onReturnKey = [this] { finish(true); };
            editor->onEscapeKey = [this] { finish(false); };
            editor->onFocusLost = [this] { finish(true); };
            editor->selectAll();
            editor->grabKeyboardFocus();
            slider.repaint();
        }
        void finish(bool take)
        {
            if (editor == nullptr) return;
            // The field goes first (its own callbacks may have called this): hidden now, deleted after the event.
            std::shared_ptr<juce::TextEditor> old(std::move(editor));
            old->onReturnKey = nullptr; old->onEscapeKey = nullptr; old->onFocusLost = nullptr;
            const juce::String text = old->getText();
            juce::Component::SafePointer<juce::Slider> slider = target;
            target = nullptr;
            if (slider != nullptr)
            {
                slider->getProperties().remove("barTyping");
                slider->removeChildComponent(old.get());
            }
            juce::MessageManager::callAsync([old] { juce::ignoreUnused(old); });
            if (take && slider != nullptr && text.trim().isNotEmpty() && text != startText)
            {
                const double value = slider->getValueFromText(text);
                if (std::isfinite(value) && value != slider->getValue())
                {
                    // As the value box did: a typed value is a drag of its own (start, value, end).
                    if (slider->onDragStart) slider->onDragStart();
                    if (slider != nullptr) slider->setValue(value, juce::sendNotificationSync);
                    if (slider != nullptr && slider->onDragEnd) slider->onDragEnd();
                }
            }
            if (slider != nullptr) slider->repaint();
        }
        juce::Component::SafePointer<juce::Slider> pending, target;
        std::unique_ptr<juce::TextEditor> editor;
        juce::String startText;
    };

    struct LookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
        {
            if (!isBarSlider(slider)) return juce::LookAndFeel_V4::getSliderLayout(slider);
            slider.setSliderSnapsToMousePosition(false);   // a drag moves the value from where it is; a click alone changes nothing
            juce::Slider::SliderLayout layout;
            layout.sliderBounds = barAreaOf(slider).reduced(3, 0);   // no value box: the value is written in the bar
            slider.setMouseDragSensitivity(juce::jmax(120, layout.sliderBounds.getWidth()));   // the bright line follows the mouse
            return layout;
        }
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos,
                              juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            const bool upright = style == juce::Slider::LinearVertical;
            WheelGate::prepare(slider);   // 0.9.136: in a scrolling area the wheel scrolls; only the slider that listens takes it
            if (!upright && !isBarSlider(slider)) { juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider); return; }
            const float alpha = slider.isEnabled() ? 1.0f : 0.45f;
            // The thin orange frame (the program's "on" colour, onColour() of UIControls.h): the wheel sets this slider now.
            const bool wheelListens = slider.isEnabled() && slider.getProperties().contains("wheelArmed");
            const juce::Colour wheelFrame = juce::Colour(0xffffa500).contrasting(0.05f);
            const auto ground = slider.findColour(juce::ComboBox::backgroundColourId);
            const auto thumb = slider.findColour(juce::Slider::thumbColourId);
            const auto track = slider.findColour(juce::Slider::trackColourId);
            const bool ownTrack = track != findColour(juce::Slider::trackColourId);   // set by the program: a sounding key (green), a pipe that is off (grey)
            const bool hasDefault = slider.isDoubleClickReturnEnabled() && slider.getDoubleClickReturnValue() >= slider.getMinimum() && slider.getDoubleClickReturnValue() <= slider.getMaximum();
            // The fill starts at the default; a slider without one: at zero when its range has both signs, else at its minimum.
            const double from = hasDefault ? slider.getDoubleClickReturnValue() : slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0 ? 0.0 : slider.getMinimum();
            // The fill - how far the value stands from the default - is dark and quiet (the user chose it for the
            // texts it runs under); the bright line alone is the value.
            // Always the look's own blue: a slider's own thumb colour (green = a replaced pipe) colours its LINE alone.
            const juce::Colour fillColour = findColour(juce::Slider::thumbColourId).withMultipliedSaturation(0.8f).withMultipliedBrightness(0.50f).withMultipliedAlpha(alpha);
            if (upright)
            {
                // A per-key slider: a narrow bar (the column is 21 px), the value in the box below it. A track colour
                // set by the program - the key SOUNDS (orange, the program's "on"), the pipe is off (grey) - tints the
                // whole bar and frames it, so it shows at the default value too; the colour of the line is the thumb's
                // (green: a pipe replaced by its neighbour).
                const float w = juce::jlimit(8.0f, 22.0f, (float)width - 5.0f);
                const float corner = juce::jmin(barCorner, w / 3.0f);
                const juce::Rectangle<float> box((float)x + ((float)width - w) * 0.5f, (float)y, w, (float)height);
                g.setColour((ownTrack ? ground.interpolatedWith(track, 0.6f) : ground).withMultipliedAlpha(alpha));
                g.fillRoundedRectangle(box, corner);
                const float zero = (float)y + (float)height * (1.0f - (float)slider.valueToProportionOfLength(from));
                g.setColour(fillColour);
                if (std::abs(sliderPos - zero) > 1.0f) g.fillRect(juce::Rectangle<float>(box.getX() + 2.0f, juce::jmin(sliderPos, zero), w - 4.0f, std::abs(sliderPos - zero)));
                g.setColour(slider.findColour(juce::ComboBox::outlineColourId).withMultipliedAlpha(alpha * 0.6f));
                g.fillRect(juce::Rectangle<float>(box.getRight() - 4.0f, zero - 0.5f, 3.0f, 1.0f));   // where the default lies
                // 0.9.136 (the user): a sounding key is orange INSIDE alone; the orange frame is the wheel's.
                g.setColour((wheelListens ? wheelFrame : slider.findColour(juce::ComboBox::outlineColourId)).withMultipliedAlpha(alpha));
                g.drawRoundedRectangle(box.reduced(wheelListens ? 0.75f : 0.5f), corner, wheelListens ? 1.5f : 1.0f);
                g.setColour(thumb.withMultipliedAlpha(alpha));
                g.fillRect(juce::Rectangle<float>(box.getX() + 1.5f, juce::jlimit((float)y + 1.0f, (float)(y + height) - 3.5f, sliderPos - 1.25f), w - 3.0f, 2.5f));
                return;
            }
            const auto area = barAreaOf(slider).toFloat();
            g.setColour(ground.withMultipliedAlpha(alpha));
            g.fillRoundedRectangle(area, barCorner);
            const float zero = (float)x + (float)width * (float)slider.valueToProportionOfLength(from);
            g.setColour(fillColour);
            if (std::abs(sliderPos - zero) > 1.0f)
                g.fillRect(juce::Rectangle<float>(juce::jmin(sliderPos, zero), area.getY() + 2.0f, std::abs(sliderPos - zero), area.getHeight() - 4.0f));
            g.setColour(wheelListens ? wheelFrame : slider.findColour(juce::ComboBox::outlineColourId).withMultipliedAlpha(alpha * (slider.isMouseOverOrDragging() ? 1.0f : 0.75f)));
            g.drawRoundedRectangle(area.reduced(wheelListens ? 0.75f : 0.5f), barCorner, wheelListens ? 1.5f : 1.0f);
            // The texts: the name and the value (never cut). The bright line of the value is drawn whole where it
            // crosses no text, else as two marks above and below the text, as long as the bar's height allows - a
            // text is never struck through.
            const bool typing = slider.getProperties().contains("barTyping");   // the value is being typed over the bar: no texts
            const auto& properties = slider.getProperties();
            const bool twoLines = (int)properties.getWithDefault("barLines", 1) == 2;
            juce::String name = properties.getWithDefault("barLabel", juce::String()).toString();
            if ((bool)properties.getWithDefault("barUseShort", false) && properties.contains("barShort")) name = properties["barShort"].toString();
            const juce::String value = slider.getTextFromValue(slider.getValue());
            const auto text = slider.findColour(juce::Slider::textBoxTextColourId).withMultipliedAlpha(alpha);
            // The sizes: 13 px by default; a row beside buttons asks for the buttons' size ("barNameSize",
            // "barValueSize"; two lines: "barSmallNameSize" above the value) - the user: the texts of the top row's
            // sliders must not be smaller than the texts of the buttons above them.
            const float nameSize = twoLines ? juce::jlimit(9.0f, 13.0f, (float)(double)properties.getWithDefault("barSmallNameSize", 11.0))
                                            : juce::jlimit(9.0f, 16.0f, (float)(double)properties.getWithDefault("barNameSize", 13.0));
            const float valueSize = juce::jlimit(9.0f, 16.0f, (float)(double)properties.getWithDefault("barValueSize", 13.0));
            const juce::Font valueFont(juce::FontOptions(valueSize, juce::Font::bold)), nameFont{ juce::FontOptions(nameSize) };
            const auto inner = area.reduced(7.0f, 0.0f).toNearestInt();
            const int valueWidth = (int)std::ceil(juce::GlyphArrangement::getStringWidth(valueFont, value));
            const int nameWidth = (int)std::ceil(juce::GlyphArrangement::getStringWidth(nameFont, name));
            const bool centredValue = twoLines || name.isEmpty();
            const auto valueSpan = centredValue ? juce::Range<int>(inner.getCentreX() - valueWidth / 2, inner.getCentreX() + (valueWidth + 1) / 2)
                                                : juce::Range<int>(inner.getRight() - valueWidth, inner.getRight());
            const auto nameSpan = name.isEmpty() ? juce::Range<int>()
                : twoLines ? juce::Range<int>(inner.getCentreX() - juce::jmin(nameWidth, inner.getWidth()) / 2, inner.getCentreX() + (juce::jmin(nameWidth, inner.getWidth()) + 1) / 2)
                           : juce::Range<int>(inner.getX(), inner.getX() + juce::jmin(nameWidth, inner.getWidth() - valueWidth - 8));
            const float lineX = juce::jlimit(area.getX() + 1.0f, area.getRight() - 3.5f, sliderPos - 1.25f);
            const auto crosses = [lineX](juce::Range<int> span) { return !span.isEmpty() && lineX + 2.5f > (float)span.getStart() - 2.0f && lineX < (float)span.getEnd() + 2.0f; };
            // Two lines stand close together in the middle of the bar (the name 12 px, the value 15 px).
            const int centreY = inner.getCentreY();
            const float textBlock = twoLines ? 27.0f : valueSize + 2.0f;
            g.setColour(thumb.withMultipliedAlpha(alpha));
            if (!typing && (crosses(valueSpan) || crosses(nameSpan)))
            {
                const float mark = juce::jmax(3.0f, (area.getHeight() - textBlock) * 0.5f - 2.5f);
                g.fillRect(juce::Rectangle<float>(lineX, area.getY() + 1.5f, 2.5f, mark));
                g.fillRect(juce::Rectangle<float>(lineX, area.getBottom() - 1.5f - mark, 2.5f, mark));
            }
            else g.fillRect(juce::Rectangle<float>(lineX, area.getY() + 1.5f, 2.5f, area.getHeight() - 3.0f));
            if (typing) return;
            if (twoLines)
            {
                g.setFont(nameFont);
                g.setColour(text.withMultipliedAlpha(0.78f));
                g.drawText(name, juce::Rectangle<int>(inner.getX(), centreY - 14, inner.getWidth(), 13), juce::Justification::centred, true);
                g.setFont(valueFont);
                g.setColour(text);
                g.drawText(value, juce::Rectangle<int>(inner.getX(), centreY - 2, inner.getWidth(), 16), juce::Justification::centred, false);
                return;
            }
            g.setFont(valueFont);
            g.setColour(text);
            g.drawText(value, inner, centredValue ? juce::Justification::centred : juce::Justification::centredRight, false);
            if (name.isNotEmpty())
            {
                g.setFont(nameFont);
                g.drawText(name, inner.withTrimmedRight(valueWidth + 8), juce::Justification::centredLeft, true);
            }
        }
        // A bar has the corners of the buttons and the displays beside it (the user: 3 px, the text boxes' corners,
        // was a mistake - "zaoblení rohů musí být stejné").
        static constexpr float barCorner = 6.0f;
        // A slider with the property "smallTextBox" - the per-key sliders of the voicing, 22 px apart - has a plain
        // number under it, no box (0.9.135, the user's choice): 9 px bold, a zero greyed so the set values stand out
        // (drawLabel below); the frame comes only while the number is typed.
        juce::Label* createSliderTextBox(juce::Slider& slider) override
        {
            auto* label = juce::LookAndFeel_V4::createSliderTextBox(slider);
            if (slider.getProperties().contains("smallTextBox"))
            {
                label->setFont(juce::FontOptions(9.0f, juce::Font::bold));
                label->setBorderSize(juce::BorderSize<int>(1, 0, 1, 0));
                label->setMinimumHorizontalScale(0.7f);
                label->getProperties().set("plainNumber", true);
            }
            return label;
        }
        // 0.9.91 (the user: "why has a combo box round corners and a text box square ones?"): every text box - the
        // value box of a slider (a Label with a background) and every TextEditor - has the combo box's 3 px corners.
        static constexpr float boxCorner = 3.0f;
        void drawLabel(juce::Graphics& g, juce::Label& label) override
        {
            if (label.getProperties().contains("plainNumber"))
            {
                if (label.isBeingEdited()) return;   // its editor draws itself (with its frame)
                const juce::String text = label.getText();
                const bool zero = text.containsOnly("-+0., ") && text.containsAnyOf("0");
                g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(!label.isEnabled() ? 0.3f : zero ? 0.42f : 1.0f));
                g.setFont(getLabelFont(label));
                g.drawFittedText(text, label.getLocalBounds(), juce::Justification::centred, 1, label.getMinimumHorizontalScale());
                return;
            }
            const auto bounds = label.getLocalBounds().toFloat();
            g.setColour(label.findColour(juce::Label::backgroundColourId));
            g.fillRoundedRectangle(bounds, boxCorner);
            if (!label.isBeingEdited())
            {
                const float alpha = label.isEnabled() ? 1.0f : 0.5f;
                const juce::Font font(getLabelFont(label));
                g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
                g.setFont(font);
                const auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
                g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                    juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())), label.getMinimumHorizontalScale());
                g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
            }
            else if (label.isEnabled()) g.setColour(label.findColour(juce::Label::outlineColourId));
            else return;
            g.drawRoundedRectangle(bounds.reduced(0.5f), boxCorner, 1.0f);
        }
        void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
        {
            if (dynamic_cast<juce::AlertWindow*>(editor.getParentComponent()) != nullptr) { juce::LookAndFeel_V4::fillTextEditorBackground(g, width, height, editor); return; }
            g.setColour(editor.findColour(juce::TextEditor::backgroundColourId));
            g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, boxCorner);
        }
        void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
        {
            if (dynamic_cast<juce::AlertWindow*>(editor.getParentComponent()) != nullptr) { juce::LookAndFeel_V4::drawTextEditorOutline(g, width, height, editor); return; }
            if (!editor.isEnabled()) return;
            const bool focused = editor.hasKeyboardFocus(true) && !editor.isReadOnly();
            g.setColour(editor.findColour(focused ? juce::TextEditor::focusedOutlineColourId : juce::TextEditor::outlineColourId));
            g.drawRoundedRectangle(juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(focused ? 1.0f : 0.5f), boxCorner, focused ? 2.0f : 1.0f);
        }
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
            if (tipPlacement().own)
            {
                const int tx = juce::jlimit(parentArea.getX(), juce::jmax(parentArea.getX(), parentArea.getRight() - w), screenPos.x);
                int ty = screenPos.y;                                                       // under the control
                if (ty + h > parentArea.getBottom()) ty = screenPos.y - tipPlacement().above - h;   // no room: above it
                return juce::Rectangle<int>(tx, juce::jmax(parentArea.getY(), ty), w, h);
            }
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
