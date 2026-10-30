#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

// The "on" colour of every switchable button (stops, transmissions, couplers,
// tremulants, pistons, crescendo): orange one shade darker than before, which
// was the mouse-over shade; the look-and-feel darkens it one more shade on hover.
inline juce::Colour onColour() { return juce::Colour(0xffffa500).contrasting(0.05f); }
// The colour of a filled combination piston or a taught crescendo step (0.8.243, the user: a
// touch of the orange on the ordinary button grey, nothing conspicuous), and of a piston
// holding the same registration as the selected one (a shade more of it). 'base' is the
// look-and-feel's TextButton::buttonColourId - one grey for the pistons and the steps.
inline juce::Colour filledColour(juce::Colour base) { return base.interpolatedWith(juce::Colour(0xffffa500), 0.12f); }   // the user's row t = 0.12 (0.8.245)
inline juce::Colour sameRegistrationColour(juce::Colour base) { return base.interpolatedWith(juce::Colour(0xffffa500), 0.18f); }

// Section header that folds its section when clicked (the Configuration Editor
// style: orange bold caption with a "+ " / "- " prefix). Shared by the dialogs.
class SectionHeaderLabel : public juce::Label
{
public:
    std::function<void()> onClick;
    SectionHeaderLabel()
    {
        setFont(juce::FontOptions(16.0f, juce::Font::bold));
        setColour(juce::Label::textColourId, juce::Colours::orange);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setTooltip("Click to show or hide this section");
    }
    void setSection(const juce::String& title, bool expanded)
    {
        setText((expanded ? "- " : "+ ") + title, juce::dontSendNotification);
    }
    void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(); }
};

// A dialog's content inside a viewport: the content keeps its design size at
// least, so when the window is smaller (a large interface size on a small
// screen, a remembered small window) scrollbars appear instead of controls
// vanishing under the window's edge. A larger window lets the content grow
// with it (its own layout spreads out). Every dialog of the program uses it.
class ScrollableContent final : public juce::Component
{
public:
    explicit ScrollableContent(juce::Component* ownedContent)
        : content(ownedContent), designWidth(ownedContent->getWidth()), designHeight(ownedContent->getHeight())
    {
        viewport.setViewedComponent(content.get(), false);
        viewport.setScrollBarsShown(true, true);
        viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::never);
        addAndMakeVisible(viewport);
        setSize(designWidth, designHeight);
    }
    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        const int bar = viewport.getScrollBarThickness();
        const bool vertical = designHeight > getHeight();
        const bool horizontal = designWidth > getWidth() - (vertical ? bar : 0);
        content->setSize(juce::jmax(designWidth, getWidth() - (vertical ? bar : 0)),
                         juce::jmax(designHeight, getHeight() - (horizontal ? bar : 0)));
    }
    juce::Component& inner() noexcept { return *content; }
    int getDesignWidth() const noexcept { return designWidth; }
    int getDesignHeight() const noexcept { return designHeight; }
private:
    std::unique_ptr<juce::Component> content;
    juce::Viewport viewport;
    int designWidth, designHeight;
};

// A viewport inside a scrolling dialog: the wheel scrolls this one until its
// edge, then the dialog itself (a plain Viewport swallows the wheel at its edge
// and the row of buttons below the table never came into view - the Stop
// List, 0.8.136).
class EdgeViewport : public juce::Viewport
{
public:
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const auto before = getViewPosition();
        if (useMouseWheelMoveIfNeeded(e, wheel) && getViewPosition() != before) return;
        juce::Component::mouseWheelMove(e, wheel);   // at the edge: the parent scrolls
    }
};

// "Swipe switching": press a stop, coupler or tremulant button and drag across
// any other such button to give it the same state. All families share one
// registry, so one drag can run from the stops into the couplers and back.
namespace SwipeSwitch
{
    struct Target
    {
        juce::Component* component = nullptr;
        std::function<void(bool)> apply;   // must be idempotent
    };
    inline std::vector<Target>& targets() { static std::vector<Target> list; return list; }
    inline bool active = false;
    inline bool enable = false;

    inline void add(juce::Component* component, std::function<void(bool)> apply)
    {
        targets().push_back({ component, std::move(apply) });
    }
    inline void remove(juce::Component* component)
    {
        auto& list = targets();
        list.erase(std::remove_if(list.begin(), list.end(),
            [component](const Target& t) { return t.component == component; }), list.end());
    }
    inline void begin(bool enableState) { active = true; enable = enableState; }
    inline void end() { active = false; }
    // Applies the swipe state to the registered button under the screen position.
    inline bool applyAt(juce::Point<int> screenPosition)
    {
        if (!active) return false;
        for (auto& t : targets())
            if (t.component != nullptr && t.component->isShowing()
                && t.component->getScreenBounds().contains(screenPosition))
            {
                // Copy first: applying may rebuild the UI and change the registry.
                const auto apply = t.apply;
                if (apply) apply(enable);
                return true;
            }
        return false;
    }
}

// JUCE sliders already support wheel movement.  This small wrapper only keeps
// a popup click from also moving the control before MIDI Learn is displayed.
class LearnableSlider : public juce::Slider
{
public:
    using juce::Slider::Slider;
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (!event.mods.isPopupMenu()) juce::Slider::mouseDown(event);
    }
};

// The large stop workspace must only move when its scrollbars are dragged.
// Wheel gestures are reserved for the controls inside the workspace.
class NoWheelScrollBar final : public juce::ScrollBar
{
public:
    explicit NoWheelScrollBar(bool vertical) : juce::ScrollBar(vertical) {}
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override {}
};

class NoWheelViewport : public juce::Viewport
{
public:
    using juce::Viewport::Viewport;
    // The owner may allow the wheel for some positions (the stop workspace: over
    // a division without an open voicing the wheel scrolls; over an open voicing
    // it belongs to the sliders).
    std::function<bool(const juce::MouseEvent&)> allowWheel;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override
    {
        if (allowWheel && allowWheel(event)) juce::Viewport::mouseWheelMove(event, wheel);
    }

protected:
    juce::ScrollBar* createScrollBarComponent(bool isVertical) override
    {
        return new NoWheelScrollBar(isVertical);
    }
};

// Momentary buttons with onStateChange must not regard a right-click used for
// MIDI Learn as a physical press.
class LeftOnlyTextButton : public juce::TextButton
{
public:
    using juce::TextButton::TextButton;
    void mouseDown(const juce::MouseEvent& event) override
    {
        acceptedLeftPress = event.mods.isLeftButtonDown();
        if (acceptedLeftPress) juce::TextButton::mouseDown(event);
    }
    void mouseUp(const juce::MouseEvent& event) override
    {
        if (acceptedLeftPress) juce::TextButton::mouseUp(event);
        acceptedLeftPress = false;
    }
private:
    bool acceptedLeftPress = false;
};

// Record button: an ordinary TextButton (standard look, colours and border)
// showing a dark red disc when idle and a bright red disc while recording; the
// button is drawn in its "on" state while recording.
class RecordButton : public LeftOnlyTextButton   // the right button is MIDI learn, like the other top-bar buttons
{
public:
    RecordButton() : LeftOnlyTextButton("") { setTooltip("Record the stereo mix of all outputs to a WAV file"); }
    void setRecording(bool shouldBeRecording)
    {
        if (recording == shouldBeRecording) return;
        recording = shouldBeRecording;
        setToggleState(recording, juce::dontSendNotification);
        repaint();
    }
    bool isRecording() const noexcept { return recording; }
    void paintButton(juce::Graphics& g, bool highlighted, bool down) override
    {
        juce::TextButton::paintButton(g, highlighted, down);   // the usual background and border
        const auto area = getLocalBounds().toFloat();
        const float diameter = juce::jmin(area.getWidth(), area.getHeight()) * 0.5f;
        const auto disc = juce::Rectangle<float>(diameter, diameter).withCentre(area.getCentre());
        g.setColour(recording ? juce::Colours::red : juce::Colour(0xff8b1a1a));
        g.fillEllipse(disc);
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawEllipse(disc, 1.0f);
    }
private:
    bool recording = false;
};

// The one display look of the console (combination display, swell bar,
// polyphony display): black rounded panel, dark grey frame, lime text.
class DisplayLabel : public juce::Label
{
public:
    using juce::Label::Label;
    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(r, 3.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(r, 3.0f, 1.0f);
        g.setColour(findColour(juce::Label::textColourId));
        g.setFont(getFont());
        g.drawFittedText(getText(), getLocalBounds().reduced(4, 2), getJustificationType(), 1, 0.6f);
    }
};

class LeftOnlyToggleButton : public juce::ToggleButton
{
public:
    using juce::ToggleButton::ToggleButton;
    void mouseDown(const juce::MouseEvent& event) override
    {
        acceptedLeftPress = event.mods.isLeftButtonDown();
        if (acceptedLeftPress) juce::ToggleButton::mouseDown(event);
    }
    void mouseUp(const juce::MouseEvent& event) override
    {
        if (acceptedLeftPress) juce::ToggleButton::mouseUp(event);
        acceptedLeftPress = false;
    }
private:
    bool acceptedLeftPress = false;
};

// A golden sheen that sweeps over a few buttons now and then after the start,
// with a half-transparent golden arrow sliding towards the button and vanishing
// (0.8.175), until the first click anywhere in the window: a newcomer's eye is
// led to Stop List and Load Configuration (0.8.172). A transparent layer over
// the buttons' area that takes no mouse; a mouse listener of the window stops it.
class HintSheen final : public juce::Component, private juce::Timer   // a Component is a MouseListener already
{
public:
    HintSheen() { setInterceptsMouseClicks(false, false); }
    // arrowFromAbove: the arrow comes down onto the button (room above it, the
    // Stop List's table); otherwise it comes up from below (the console's top row).
    void follow(std::vector<juce::Component*> buttons, bool arrowFromAbove = false)
    {
        targets = std::move(buttons);
        fromAbove = arrowFromAbove;
        startTimer(2000);
    }
    void stop()
    {
        stopTimer();
        setVisible(false);
        if (auto* parent = getParentComponent()) parent->removeMouseListener(this);
    }
    bool active() const { return isTimerRunning(); }   // not yet stopped by a click
    void mouseDown(const juce::MouseEvent&) override { stop(); }
    void paint(juce::Graphics& g) override
    {
        if (phase < 0.0f || targets.empty()) return;
        // One button at a time, the buttons in turn (0.8.173).
        auto* button = targets[current % targets.size()];
        if (button == nullptr || !button->isShowing()) return;
        const auto r = getLocalArea(button, button->getLocalBounds()).toFloat();
        const float band = r.getHeight() * 1.2f;
        const float x = r.getX() - band + (r.getWidth() + 2.0f * band) * phase;   // the band sweeps left to right
        juce::ColourGradient gradient(juce::Colours::transparentWhite, x - band, r.getY(), juce::Colours::transparentWhite, x + band, r.getBottom(), false);
        gradient.addColour(0.5, juce::Colour(0xffffd36b).withAlpha(0.3f));
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(r, 4.0f);
        // The arrow: slides towards the button over the sweep, fades in and out.
        const float eased = 1.0f - (1.0f - phase) * (1.0f - phase);
        const float alpha = std::sin(phase * juce::MathConstants<float>::pi) * 0.6f;
        const float gap = 6.0f + travel * (1.0f - eased);   // the tip's distance from the button's edge
        const float dir = fromAbove ? 1.0f : -1.0f;          // +1: pointing down
        const float cx = r.getCentreX(), tipY = fromAbove ? r.getY() - gap : r.getBottom() + gap;
        const float backY = tipY - dir * arrowLength, headBackY = tipY - dir * 56.0f, headHalf = 44.0f, shaftHalf = 16.0f;   // four times the 0.8.175 size (the user: "hodně velké")
        juce::Path arrow;
        arrow.startNewSubPath(cx, tipY);
        arrow.lineTo(cx + headHalf, headBackY);
        arrow.lineTo(cx + shaftHalf, headBackY);
        arrow.lineTo(cx + shaftHalf, backY);
        arrow.lineTo(cx - shaftHalf, backY);
        arrow.lineTo(cx - shaftHalf, headBackY);
        arrow.lineTo(cx - headHalf, headBackY);
        arrow.closeSubPath();
        juce::ColourGradient shade(juce::Colour(0xfffff0b0).withAlpha(alpha), cx - headHalf, tipY, juce::Colour(0xffc9931a).withAlpha(alpha), cx + headHalf, backY, false);
        g.setGradientFill(shade);
        g.fillPath(arrow);
        g.setColour(juce::Colour(0xff7a5a08).withAlpha(alpha * 0.8f));
        g.strokePath(arrow, juce::PathStrokeType(1.0f));
    }
private:
    void timerCallback() override
    {
        if (phase < 0.0f) { phase = 0.0f; startTimer(25); }
        else if ((phase += 0.03f) > 1.0f) { phase = -1.0f; ++current; startTimer(2500); }
        if (auto* parent = getParentComponent())
        {
            juce::Rectangle<int> area;
            for (auto* button : targets)
                if (button != nullptr && button->isShowing()) area = area.getUnion(parent->getLocalArea(button, button->getLocalBounds()));
            // Room for the arrow's travel on the side it comes from, within the parent.
            const int reach = (int)(travel + arrowLength + 12.0f);
            area = fromAbove ? area.withTop(area.getY() - reach) : area.withBottom(area.getBottom() + reach);
            setBounds(area.getIntersection(parent->getLocalBounds()));
            toFront(false);
        }
        repaint();
    }
    static constexpr float travel = 180.0f, arrowLength = 120.0f;   // 0.8.179: four times the 0.8.175 arrow (30 px), twice the travel
    std::vector<juce::Component*> targets;
    bool fromAbove = false;
    size_t current = 0;    // the button of this sweep
    float phase = -1.0f;   // < 0 between the sweeps
};

// A transparent layer over the whole console (0.8.206): a button pressed with the left mouse button and a stop
// being dragged glow for a moment with the golden halo of the instruction video (the user asked for the glows in
// the program too, with a switch: the setting "consoleGlow", Configuration Editor "Glow ..."). It is a mouse
// listener of the main component (every child's events reach it) and takes no mouse itself.
class GlowLayer final : public juce::Component, private juce::Timer
{
public:
    GlowLayer() { setInterceptsMouseClicks(false, false); setOpaque(false); }
    bool enabled = true;
    void glow(juce::Component* target, double ms)
    {
        if (!enabled || target == nullptr) return;
        const double until = juce::Time::getMillisecondCounterHiRes() + ms;
        for (auto& item : items)
            if (item.target == target) { item.until = juce::jmax(item.until, until); return; }
        items.push_back({ target, until });
        toFront(false);
        if (!isTimerRunning()) startTimerHz(30);
        repaint(areaOf(target));
    }
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.mods.isLeftButtonDown() && dynamic_cast<juce::Button*>(event.eventComponent) != nullptr) glow(event.eventComponent, 900.0);
    }
    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (dynamic_cast<juce::Button*>(event.eventComponent) != nullptr) glow(event.eventComponent, 400.0);   // a dragged stop glows as long as it is dragged
    }
    void paint(juce::Graphics& g) override
    {
        const double now = juce::Time::getMillisecondCounterHiRes();
        for (const auto& item : items)
        {
            if (item.target == nullptr || !item.target->isShowing() || now >= item.until) continue;
            const auto r = getLocalArea(item.target, item.target->getLocalBounds()).toFloat();
            const float fade = (float)juce::jlimit(0.0, 1.0, (item.until - now) / 300.0);
            const float pulse = (0.55f + 0.45f * (float)std::sin(now / 110.0)) * fade;
            for (int ring = 4; ring >= 1; --ring)
            {
                g.setColour(juce::Colour(0xffffd36b).withAlpha(pulse * 0.16f * (float)(5 - ring)));
                g.drawRoundedRectangle(r.expanded(3.0f + 4.0f * (float)ring), 8.0f, 5.0f);
            }
        }
    }
private:
    struct Item { juce::Component::SafePointer<juce::Component> target; double until; };
    juce::Rectangle<int> areaOf(juce::Component* target) const
    {
        return target != nullptr && target->isShowing() ? getLocalArea(target, target->getLocalBounds()).expanded(26) : juce::Rectangle<int>();
    }
    void timerCallback() override
    {
        const double now = juce::Time::getMillisecondCounterHiRes();
        juce::Rectangle<int> dirty;
        for (auto it = items.begin(); it != items.end();)
        {
            const auto a = areaOf(it->target);
            if (!a.isEmpty()) dirty = dirty.isEmpty() ? a : dirty.getUnion(a);
            if (it->target == nullptr || now >= it->until + 40.0) it = items.erase(it); else ++it;
        }
        if (!dirty.isEmpty()) repaint(dirty);
        if (items.empty()) stopTimer();
    }
    std::vector<Item> items;
};
