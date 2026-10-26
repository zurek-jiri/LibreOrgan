#pragma once

#include <JuceHeader.h>

//==============================================================================
// The guided tour of a teaching organ (0.8.206, the Rural Organ demo): a caption
// box with an arrow to the element it speaks of and a golden frame around it,
// Back / Next / Close, a link to the manual chapter that explains the feature,
// and "Don't show this tour again for this organ". The steps come from the owner
// (MainComponent builds them from the live grid); a step whose element is not on
// the screen is skipped. The overlay itself takes no mouse - the console stays
// usable while the tour is shown - only the box does.
class GuidedTour final : public juce::Component, private juce::Timer
{
public:
    struct Step
    {
        std::function<juce::Component*()> target;   // the element the step speaks of (nullptr: skipped)
        juce::String text;                          // the caption
        juce::String chapter;                       // the manual chapter named as its heading (without the number)
    };

    GuidedTour(std::vector<Step> stepsIn, std::function<void(const juce::String&)> openManual, std::function<void(bool dontShowAgain)> closed)
        : steps(std::move(stepsIn)), openManualChapter(std::move(openManual)), onClosed(std::move(closed))
    {
        setInterceptsMouseClicks(false, true);
        addAndMakeVisible(box);
        box.back.setButtonText("Back");
        box.next.setButtonText("Next");
        box.close.setButtonText("Close");
        box.manual.setButtonText("Manual");
        box.again.setButtonText("Don't show this tour again for this organ");
        box.back.onClick = [this] { move(-1); };
        box.next.onClick = [this] { if (current + 1 >= (int)order.size()) finish(); else move(1); };
        box.close.onClick = [this] { finish(); };
        box.manual.onClick = [this] { if (openManualChapter && current >= 0 && current < (int)order.size()) openManualChapter(steps[(size_t)order[(size_t)current]].chapter); };
        // Only the steps whose element is on the screen now.
        for (size_t i = 0; i < steps.size(); ++i)
            if (steps[i].target && steps[i].target() != nullptr) order.push_back((int)i);
        current = order.empty() ? -1 : 0;
        startTimerHz(30);
        updateBox();
    }
    ~GuidedTour() override { stopTimer(); }

    int stepCount() const noexcept { return (int)order.size(); }
    bool finished = false;

    void paint(juce::Graphics& g) override
    {
        auto* target = currentTarget();
        if (target == nullptr || !target->isShowing()) return;
        const auto r = getLocalArea(target, target->getLocalBounds()).toFloat().expanded(5.0f);
        // The frame around the element, breathing.
        const float pulse = 0.55f + 0.35f * std::sin(phase);
        g.setColour(juce::Colour(0xffffd36b).withAlpha(pulse));
        g.drawRoundedRectangle(r, 6.0f, 3.0f);
        // The arrow from the box to the frame: from the box's nearest edge centre
        // to the frame's nearest point, a thick golden line with a head.
        const auto b = box.getBounds().toFloat();
        juce::Point<float> from = b.getCentre(), to = r.getCentre();
        auto clampTo = [](const juce::Rectangle<float>& rect, juce::Point<float> p)
        { return juce::Point<float>(juce::jlimit(rect.getX(), rect.getRight(), p.x), juce::jlimit(rect.getY(), rect.getBottom(), p.y)); };
        from = clampTo(b, to);
        to = clampTo(r, from);
        const auto d = to - from;
        const float length = d.getDistanceFromOrigin();
        if (length < 12.0f) return;
        const auto unit = d / length;
        const auto normal = juce::Point<float>(-unit.y, unit.x);
        const auto headBase = to - unit * 18.0f;
        juce::Path arrow;
        arrow.startNewSubPath(from + normal * 3.0f);
        arrow.lineTo(headBase + normal * 3.0f);
        arrow.lineTo(headBase + normal * 11.0f);
        arrow.lineTo(to);
        arrow.lineTo(headBase - normal * 11.0f);
        arrow.lineTo(headBase - normal * 3.0f);
        arrow.lineTo(from - normal * 3.0f);
        arrow.closeSubPath();
        g.setColour(juce::Colour(0xffffd36b).withAlpha(0.9f));
        g.fillPath(arrow);
        g.setColour(juce::Colour(0xff7a5a08).withAlpha(0.8f));
        g.strokePath(arrow, juce::PathStrokeType(1.0f));
    }
    void resized() override { placeBox(); }

private:
    struct Box final : public juce::Component
    {
        Box()
        {
            setInterceptsMouseClicks(true, true);
            for (auto* b : { &back, &next, &close, &manual }) addAndMakeVisible(*b);
            addAndMakeVisible(again);
            again.setColour(juce::ToggleButton::textColourId, juce::Colours::white.withAlpha(0.8f));
        }
        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colour(0xf0202a33));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
            g.setColour(juce::Colour(0xffffd36b));
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
            g.setColour(juce::Colour(0xffffd36b));
            g.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
            g.drawText(title, 16, 10, getWidth() - 32, 20, juce::Justification::centredLeft);
            layout.draw(g, juce::Rectangle<float>(16.0f, 36.0f, (float)getWidth() - 32.0f, textHeight));
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced(12);
            auto buttons = r.removeFromBottom(28);
            close.setBounds(buttons.removeFromRight(70));
            buttons.removeFromRight(6);
            next.setBounds(buttons.removeFromRight(70));
            buttons.removeFromRight(6);
            back.setBounds(buttons.removeFromRight(70));
            manual.setBounds(buttons.removeFromLeft(juce::jmin(170, buttons.getWidth())));
            r.removeFromBottom(6);
            again.setBounds(r.removeFromBottom(24));
        }
        void setText(const juce::String& heading, const juce::String& text, int width)
        {
            title = heading;
            juce::AttributedString attributed;
            attributed.append(text, juce::Font(juce::FontOptions(16.0f)), juce::Colours::white);
            attributed.setLineSpacing(2.0f);
            layout.createLayout(attributed, (float)width - 32.0f);
            textHeight = layout.getHeight() + 4.0f;
        }
        int neededHeight() const { return 36 + (int)std::ceil(textHeight) + 8 + 24 + 6 + 28 + 12; }
        juce::String title;
        juce::TextLayout layout;
        float textHeight = 0.0f;
        juce::TextButton back, next, close, manual;
        juce::ToggleButton again;
    } box;

    juce::Component* currentTarget() const
    {
        if (current < 0 || current >= (int)order.size()) return nullptr;
        return steps[(size_t)order[(size_t)current]].target ? steps[(size_t)order[(size_t)current]].target() : nullptr;
    }
    void move(int delta)
    {
        if (order.empty()) return;
        current = juce::jlimit(0, (int)order.size() - 1, current + delta);
        updateBox();
    }
    void finish()
    {
        if (finished) return;
        finished = true;
        stopTimer();
        setVisible(false);
        if (onClosed) onClosed(box.again.getToggleState());
    }
    void updateBox()
    {
        if (current < 0) { finish(); return; }
        const auto& step = steps[(size_t)order[(size_t)current]];
        box.setText("Tour " + juce::String(current + 1) + " of " + juce::String((int)order.size()), step.text, boxWidth);
        box.manual.setButtonText("Manual: " + step.chapter);
        box.manual.setTooltip("Opens the manual at \"" + step.chapter + "\"");
        box.back.setEnabled(current > 0);
        box.next.setButtonText(current + 1 >= (int)order.size() ? "Finish" : "Next");
        placeBox();
        repaint();
    }
    void placeBox()
    {
        const int height = box.neededHeight();
        auto* target = currentTarget();
        juce::Rectangle<int> r(0, 0, boxWidth, height);
        if (target != nullptr && target->isShowing())
        {
            const auto t = getLocalArea(target, target->getLocalBounds());
            // Above the element when there is room (what the step speaks of - a voicing row, the ranks of a
            // stop folder - unfolds BELOW the element and must stay in view; the user: "tipy zezhora"), else
            // below it, else beside it; always inside.
            if (t.getY() - 40 - height >= 0) r.setPosition(t.getCentreX() - boxWidth / 2, t.getY() - 40 - height);
            else if (t.getBottom() + 40 + height <= getHeight()) r.setPosition(t.getCentreX() - boxWidth / 2, t.getBottom() + 40);
            else r.setPosition(t.getRight() + 40, t.getCentreY() - height / 2);
        }
        else r.setPosition((getWidth() - boxWidth) / 2, (getHeight() - height) / 2);
        r.setX(juce::jlimit(8, juce::jmax(8, getWidth() - boxWidth - 8), r.getX()));
        r.setY(juce::jlimit(8, juce::jmax(8, getHeight() - height - 8), r.getY()));
        box.setBounds(r);
    }
    void timerCallback() override
    {
        phase += 0.12f;
        if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;
        // The element may have moved (a layout, a scroll): the box follows.
        if (auto* target = currentTarget(); target != nullptr && target->isShowing())
        {
            const auto t = getLocalArea(target, target->getLocalBounds());
            if (t != lastTargetBounds) { lastTargetBounds = t; placeBox(); }
        }
        repaint();
    }

    static constexpr int boxWidth = 460;
    std::vector<Step> steps;
    std::vector<int> order;   // the steps with an element on the screen
    int current = -1;
    float phase = 0.0f;
    juce::Rectangle<int> lastTargetBounds;
    std::function<void(const juce::String&)> openManualChapter;
    std::function<void(bool)> onClosed;
};
