#pragma once
// 0.9.81/0.9.82 (#236, the user's design): THE AUDIO ROUTING DIALOG - "Organ out" (the sources), "Sound cards in"
// (the cards) and the cables between them.
//   - LEFT ("Organ out"): the sources grouped by PERSPECTIVE: a perspective row ("Front") rolls the divisions up when
//     every division sends it the same way (one cable for all); "(various)" and unfolded when they differ; the arrow
//     unfolds the divisions (P., I., II. ...) so each gets its own cable. Where a source plays now is in the tooltip.
//   - RIGHT ("Sound cards in"): the cards IN USE or open as full cards - name, driver + status, Sample rate / Buffer
//     combos (ASIO too: the driver's sizes), the latency, Driver panel, Reopen, Test - with the OUTPUTS as jacks along
//     the bottom edge and the driver's channel names standing vertically above them; a card grows WIDE with its
//     outputs, never tall. Every other card of every driver (Windows Audio, Exclusive, ASIO, Aggregated WASAPI; JACK /
//     ALSA; CoreAudio) is a button sized like a stop button in a COLLAPSIBLE section per driver (the orange caption
//     folds it, remembered per machine); a drop on it opens the card with the pair 1+2 and it moves up. A card busy
//     through another driver on the same hardware is grey and takes no cable. Cards plugged in after the start appear
//     as the system reports them (no scan of ours: a rescan of ASIO with an open driver, or JACK beside ALSA, crashes).
//     The full cards are dragged by their title onto a SLOT of the grid (the slot under the mouse glows orange).
//   - CABLES: from the right edge of the source row, along its own lane to the card's gutter, down under the card and
//     up INTO the output from below, in the source's colour; a stereo cable (a pair) is two strands ending in a clamp
//     over both outputs (several cables into one pair = concentric clamps, the jack striped in all their colours), a
//     single channel one strand. Drag a source onto an output: the output and its pair partner glow ORANGE (the
//     colour of a stop switched on) while the mouse is over them; drop = that pair replaces the route's channels,
//     Shift = the pair is added (C / C# split: the pipes alternate between the pairs by key), Alt = one channel. Click
//     a cable to select it (the others dim); Delete or the x at its end removes it, dragging the selected cable moves it
//     to another output (or card); the last cable gone = the source is silent.
// Everything goes through the Divisions dialog's own calls (setRouteAudioType, applyChannelsToDivision,
// detachRouteAudioQuietly); both dialogs read the same model and refresh by timer (rate, buffer and latency too:
// a size set in the ASIO panel shows within a second).
// Dev: ORGAN_TEST_ROUTE="<division|all>|<perspective>|<card name part>|<output 1-based>[|add|single|remove|movecable
// <from output>]" or "card|<name part>|<slot>" (several specs separated by ^, the first 1.5 s after the dialog opens,
// then 3 s apart) drives the actions without a mouse; the log says "Audio routing: ...".
#include <JuceHeader.h>
#include "MainComponent.h"

class AudioRoutingContent final : public juce::Component, public juce::TooltipClient, private juce::Timer
{
public:
    explicit AudioRoutingContent(MainComponent& o) : owner(o)
    {
        getProperties().set("manualChapter", "Audio routing");
        setWantsKeyboardFocus(true);
        help.setText("Drag a perspective (left) onto an output of a card: the output and its partner glow where the stereo pair will land; "
            "Shift adds a second pair to the same source (the pipes alternate between the pairs by key, a C / C# split), Alt takes one channel. "
            "Click a cable to select it: Delete (or its x) removes it, dragging it moves it to another output. A grey card is busy through "
            "another driver; the arrow before a perspective splits it into its divisions; drag a card by its title onto another place of the grid.",
            juce::dontSendNotification);
        help.setFont(juce::FontOptions(12.5f));
        help.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        help.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(help);
        showCables.setButtonText("Show cables");
        showCables.setToggleState(true, juce::dontSendNotification);
        showCables.onClick = [this] { repaint(); };
        addAndMakeVisible(showCables);
        sizeLabel.setText("Button size:", juce::dontSendNotification);
        sizeLabel.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(sizeLabel);
        sizeBox.addItem("small", 1); sizeBox.addItem("normal", 2); sizeBox.addItem("large", 3);
        sizeBox.setSelectedId(juce::jlimit(0, 2, owner.audioRoutingButtonSize) + 1, juce::dontSendNotification);
        sizeBox.setTooltip("The size of the sound card buttons, the outputs and their names");
        sizeBox.onChange = [this] { owner.audioRoutingButtonSize = sizeBox.getSelectedId() - 1; owner.saveSettings(); rebuild(); };
        addAndMakeVisible(sizeBox);
        rebuild();
        startTimer(1000);
        if (const char* env = std::getenv("ORGAN_TEST_ROUTE"))
        {
            const auto specs = juce::StringArray::fromTokens(juce::String::fromUTF8(env), "^", "");
            juce::Component::SafePointer<AudioRoutingContent> safeThis(this);
            for (int i = 0; i < specs.size(); ++i)
                juce::Timer::callAfterDelay(1500 + 3000 * i, [safeThis, spec = specs[i]] { if (safeThis != nullptr) safeThis->runTestSpec(spec); });
        }
    }

    void resized() override { layout(); }

    void paint(juce::Graphics& g) override
    {
        const auto glow = juce::Colours::orange;
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        // The source column.
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(juce::Rectangle<float>(8.0f, (float)sourcesTop - 4.0f, (float)leftWidth - 8.0f, (float)juce::jmax(40, sourcesBottom - sourcesTop + 8)), 6.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText("Organ out", 14, sourcesTop - 26, leftWidth - 20, 20, juce::Justification::centredLeft);
        g.drawText("Sound cards in", leftWidth + 20 + gutter, sourcesTop - 26, 300, 20, juce::Justification::centredLeft);
        for (const auto& row : rows)
        {
            const bool dragged = dragging && dragRow == &row - rows.data();
            g.setFont(juce::FontOptions(13.0f, row.perspectiveRow ? juce::Font::bold : juce::Font::plain));
            auto area = row.bounds;
            if (row.perspectiveRow)
            {
                g.setColour(juce::Colours::lightgrey);
                g.drawText(row.folded ? juce::String::charToString(0x25B8) : juce::String::charToString(0x25BE), area.removeFromLeft(16), juce::Justification::centred);
            }
            else area.removeFromLeft(26);
            g.setColour(row.colour.withAlpha(row.silent ? 0.35f : 1.0f));
            g.fillEllipse((float)area.getX(), (float)area.getCentreY() - 5.0f, 10.0f, 10.0f);
            area.removeFromLeft(14);
            g.setColour(dragged ? glow : row.silent ? juce::Colours::grey : juce::Colours::white);
            g.drawFittedText(row.label, area.withTrimmedRight(4), juce::Justification::centredLeft, 1);
        }
        // The other cards: buttons per driver under a collapsible caption.
        if (!typeSections.empty())
        {
            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
            g.drawText("Other sound cards - drop a perspective onto one to open it", 14, chipsTop - 4, 600, 20, juce::Justification::centredLeft);
            for (const auto& section : typeSections)
            {
                g.setColour(section.collapsed ? glow.withAlpha(0.85f) : glow);
                g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
                g.drawText((section.collapsed ? "+ " : "- ") + section.type, section.bounds, juce::Justification::centredLeft);
            }
        }
        for (const auto& card : cards)
        {
            if (!card.chip || card.bounds.isEmpty()) continue;
            const bool target = dragging && dropCard == &card - cards.data();
            const auto base = getLookAndFeel().findColour(juce::TextButton::buttonColourId);
            g.setColour(card.busy ? base.withAlpha(0.35f) : target ? glow : base);
            g.fillRoundedRectangle(card.bounds.toFloat(), 4.0f);
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.drawRoundedRectangle(card.bounds.toFloat().reduced(0.5f), 4.0f, 1.0f);
            g.setColour(card.busy ? juce::Colours::grey : target ? juce::Colours::black : juce::Colours::white);
            g.setFont(juce::FontOptions(14.0f * scale()));
            g.drawFittedText(card.name, card.bounds.reduced(8, 2), juce::Justification::centred, 2);
        }
        // The slots while a card is dragged.
        if (dragCard >= 0 && draggingCard)
            for (int i = 0; i < (int)slots.size(); ++i)
            {
                const bool under = i == dropSlot;
                g.setColour(under ? glow : juce::Colours::lightgrey.withAlpha(0.5f));
                const float dashes[]{ 6.0f, 4.0f };
                juce::Path p; p.addRoundedRectangle(slots[(size_t)i].toFloat().reduced(1.0f), 6.0f);
                juce::Path dashed; juce::PathStrokeType(under ? 2.5f : 1.5f).createDashedStroke(dashed, p, dashes, 2);
                g.fillPath(dashed);
            }
        // The cards in use / open.
        for (const auto& card : cards)
        {
            if (card.chip) continue;
            const auto b = card.bounds.toFloat();
            g.setColour(card.busy ? juce::Colours::black.withAlpha(0.12f) : juce::Colours::black.withAlpha(0.28f));
            g.fillRoundedRectangle(b, 6.0f);
            if (dragging && dropCard == &card - cards.data())
            {
                g.setColour(glow.withAlpha(0.6f));
                g.drawRoundedRectangle(b.reduced(1.0f), 6.0f, 2.0f);
            }
            g.setColour(card.busy ? juce::Colours::grey : juce::Colours::white);
            g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
            g.drawFittedText(card.name, card.bounds.getX() + 10, card.bounds.getY() + 4, card.bounds.getWidth() - 20, 20, juce::Justification::centredLeft, 1);
            g.setColour(card.busy ? juce::Colours::grey : juce::Colours::lightgrey);
            g.setFont(juce::FontOptions(11.5f));
            g.drawFittedText(card.type + "  -  " + card.status, card.bounds.getX() + 10, card.bounds.getY() + 24, card.bounds.getWidth() - 20, 16, juce::Justification::centredLeft, 1);
            // The channel names, vertical, above the jacks (read from the bottom up).
            if (card.nameBand > 0)
            {
                g.setFont(juce::FontOptions(10.5f * scale()));
                for (int ch = 0; ch < (int)card.jacks.size() && ch < card.jackNames.size(); ++ch)
                {
                    const auto jack = card.jacks[(size_t)ch];
                    const juce::Rectangle<float> band((float)jack.getX(), (float)jack.getY() - 6.0f - (float)card.nameBand + 8.0f, (float)jack.getWidth(), (float)card.nameBand - 8.0f);
                    g.saveState();
                    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, band.getCentreX(), band.getCentreY()));
                    const auto rotated = juce::Rectangle<float>(band.getHeight(), band.getWidth()).withCentre(band.getCentre());
                    g.setColour(card.busy ? juce::Colours::grey : juce::Colours::lightgrey);
                    g.drawFittedText(card.jackNames[ch], rotated.toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
                    g.restoreState();
                }
            }
            // The outputs along the bottom edge: striped in the colours of every cable that enters them.
            for (int ch = 0; ch < (int)card.jacks.size(); ++ch)
            {
                const auto jack = card.jacks[(size_t)ch];
                const bool lit = dragging && dropCard == &card - cards.data() && (ch == dropChannel || (dropPairs && ch == dropChannel + 1));
                const auto held = jackColours(card, ch);
                if (lit) g.setColour(glow);
                else g.setColour(held.empty() ? juce::Colours::darkgrey.darker(0.3f) : held.front().darker(0.2f));
                g.fillRoundedRectangle(jack.toFloat(), 3.0f);
                if (!lit && held.size() > 1)
                {
                    g.saveState();
                    juce::Path clip; clip.addRoundedRectangle(jack.toFloat(), 3.0f); g.reduceClipRegion(clip);
                    const float stripe = (float)jack.getWidth() / (float)held.size();
                    for (size_t s = 0; s < held.size(); ++s)
                    {
                        g.setColour(held[s].darker(0.2f));
                        g.fillRect(juce::Rectangle<float>((float)jack.getX() + stripe * (float)s, (float)jack.getY(), stripe + 0.5f, (float)jack.getHeight()));
                    }
                    g.restoreState();
                }
                if (lit) { g.setColour(glow.withAlpha(0.35f)); g.fillRoundedRectangle(jack.toFloat().expanded(4.0f), 5.0f); }
                g.setColour(card.busy ? juce::Colours::grey : lit || !held.empty() ? juce::Colours::black : juce::Colours::lightgrey);
                g.setFont(juce::FontOptions(10.5f * scale(), juce::Font::bold));
                g.drawText(juce::String(ch + 1), jack, juce::Justification::centred);
            }
            if (card.jacks.empty())
            {
                g.setColour(juce::Colours::grey);
                g.setFont(juce::FontOptions(11.0f));
                g.drawText("outputs are known once the card is open - drop a perspective here to open it", card.bounds.withTrimmedTop(card.bounds.getHeight() - 30).reduced(10, 4), juce::Justification::centredLeft);
            }
        }
        if (showCables.getToggleState() || dragging || selectedCable >= 0) paintCables(g);
        // The cable being dragged (a source, or the selected cable on the move): from its start to the mouse.
        if (dragging)
        {
            juce::Point<float> from; juce::Colour colour = glow;
            if (dragRow >= 0 && dragRow < (int)rows.size()) { const auto& row = rows[(size_t)dragRow]; from = { (float)row.bounds.getRight(), (float)row.bounds.getCentreY() }; colour = row.colour; }
            else if (cableDrag >= 0 && cableDrag < (int)cables.size()) { const auto& row = rows[(size_t)cables[(size_t)cableDrag].row]; from = { (float)row.bounds.getRight(), (float)row.bounds.getCentreY() }; colour = row.colour; }
            juce::Path p;
            p.startNewSubPath(from);
            p.cubicTo(from.x + 60.0f, from.y, dragPoint.x - 60.0f, dragPoint.y, dragPoint.x, dragPoint.y);
            g.setColour(colour.withAlpha(0.9f));
            g.strokePath(p, juce::PathStrokeType(3.0f));
            if (dropCard >= 0)
            {
                const auto& target = cards[(size_t)dropCard];
                const juce::String what = dropPairs && (target.jacks.empty() || dropChannel + 1 < (int)target.jacks.size())
                    ? juce::String(dropChannel + 1) + "+" + juce::String(dropChannel + 2) : juce::String(dropChannel + 1);
                const juce::String text = (dropAdd ? "+ " : "") + what;
                g.setColour(juce::Colours::black.withAlpha(0.75f));
                g.fillRoundedRectangle(dragPoint.x + 14.0f, dragPoint.y - 12.0f, 24.0f + 8.0f * (float)text.length(), 22.0f, 4.0f);
                g.setColour(glow);
                g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
                g.drawText(text, (int)dragPoint.x + 16, (int)dragPoint.y - 12, 24 + 8 * text.length(), 22, juce::Justification::centredLeft);
            }
        }
        // The ghost of a dragged card.
        if (dragCard >= 0 && draggingCard && dragCard < (int)cards.size())
        {
            const auto& card = cards[(size_t)dragCard];
            const juce::Rectangle<float> ghost(dragPoint.x - 90.0f, dragPoint.y - 14.0f, 180.0f, 28.0f);
            g.setColour(juce::Colours::black.withAlpha(0.7f)); g.fillRoundedRectangle(ghost, 5.0f);
            g.setColour(glow); g.drawRoundedRectangle(ghost, 5.0f, 1.5f);
            g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            g.drawFittedText(card.name, ghost.toNearestInt().reduced(6, 0), juce::Justification::centred, 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        grabKeyboardFocus();
        pressedCable = -1;
        // A fold arrow of a perspective.
        for (auto& row : rows)
            if (row.perspectiveRow && row.bounds.withWidth(18).contains(e.getPosition()))
            {
                if (unfolded.count(row.perspective) != 0) unfolded.erase(row.perspective); else unfolded.insert(row.perspective);
                rebuild();
                return;
            }
        // A driver section's caption: fold / unfold.
        for (const auto& section : typeSections)
            if (section.bounds.contains(e.getPosition()))
            {
                if (owner.audioRoutingCollapsed.contains(section.type)) owner.audioRoutingCollapsed.removeString(section.type);
                else owner.audioRoutingCollapsed.add(section.type);
                owner.saveSettings();
                rebuild();
                return;
            }
        // The x of the selected cable.
        if (selectedCable >= 0 && selectedCable < (int)cables.size() && cables[(size_t)selectedCable].removeBox.contains(e.getPosition()))
        {
            removeCable(selectedCable);
            return;
        }
        // A cable: a click selects it; a press on the SELECTED one may start its move (mouseDrag decides).
        for (int i = 0; i < (int)cables.size(); ++i)
            if (cables[(size_t)i].hitPath.contains(e.getPosition().toFloat()))
            {
                if (selectedCable == i) { pressedCable = i; dragPoint = e.getPosition().toFloat(); }
                else { selectedCable = i; repaint(); }
                return;
            }
        // A source row: the start of a drag.
        for (int i = 0; i < (int)rows.size(); ++i)
            if (rows[(size_t)i].bounds.contains(e.getPosition()))
            {
                dragRow = i; dragging = false; dragPoint = e.getPosition().toFloat();
                return;
            }
        // A card's title: the start of a card drag.
        for (int i = 0; i < (int)cards.size(); ++i)
            if (!cards[(size_t)i].chip && cards[(size_t)i].bounds.withHeight(44).contains(e.getPosition())) { dragCard = i; draggingCard = false; return; }
        selectedCable = -1;
        repaint();
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (dragRow >= 0 || cableDrag >= 0 || pressedCable >= 0)
        {
            if (pressedCable >= 0 && cableDrag < 0 && e.getDistanceFromDragStart() > 6) { cableDrag = pressedCable; pressedCable = -1; }
            if (!dragging && e.getDistanceFromDragStart() > 6 && (dragRow >= 0 || cableDrag >= 0)) dragging = true;
            if (!dragging) return;
            dragPoint = e.getPosition().toFloat();
            dropAdd = dragRow >= 0 && e.mods.isShiftDown();
            dropPairs = cableDrag >= 0 ? cables[(size_t)cableDrag].channels.size() > 1 : !e.mods.isAltDown();
            findDropTarget(e.getPosition());
            repaint();
        }
        else if (dragCard >= 0)
        {
            if (!draggingCard && e.getDistanceFromDragStart() > 6) draggingCard = true;
            if (!draggingCard) return;
            dragPoint = e.getPosition().toFloat();
            dropSlot = -1; int best = 1 << 30;
            for (int i = 0; i < (int)slots.size(); ++i)
            {
                const int d = slots[(size_t)i].getCentre().getDistanceFrom(e.getPosition());
                if (slots[(size_t)i].contains(e.getPosition())) { dropSlot = i; break; }
                if (d < best) { best = d; dropSlot = i; }
            }
            repaint();
        }
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (dragging && dropCard >= 0 && dropChannel >= 0)
        {
            const auto& card = cards[(size_t)dropCard];
            std::vector<int> channels{ dropChannel };
            if (dropPairs && (card.jacks.empty() || dropChannel + 1 < (int)card.jacks.size())) channels.push_back(dropChannel + 1);
            if (card.jacks.empty()) channels = { 0, 1 };   // a card not open yet: it opens on the first pair
            if (dragRow >= 0) assign(rows[(size_t)dragRow].routes, card, channels, dropAdd);
            else if (cableDrag >= 0) moveCable(cableDrag, card, channels);
        }
        else if (dragCard >= 0 && draggingCard && dropSlot >= 0) moveCard(dragCard, dropSlot);
        dragRow = -1; dragCard = -1; cableDrag = -1; pressedCable = -1; dragging = false; draggingCard = false; dropCard = -1; dropChannel = -1; dropSlot = -1;
        juce::ignoreUnused(e);
        repaint();
    }
    bool keyPressed(const juce::KeyPress& key) override
    {
        if ((key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey) && selectedCable >= 0) { removeCable(selectedCable); return true; }
        if (key == juce::KeyPress::escapeKey && selectedCable >= 0) { selectedCable = -1; repaint(); return true; }
        return false;
    }
    juce::String getTooltip() override
    {
        const auto p = getMouseXYRelative();
        for (const auto& row : rows)
            if (row.bounds.contains(p)) return row.label + ": " + row.whereText;
        for (const auto& card : cards)
            if (card.bounds.contains(p)) return card.name + " (" + card.type + "): " + card.status + (card.busy ? "\n" + card.busyReason : juce::String());
        for (const auto& section : typeSections)
            if (section.bounds.contains(p)) return section.collapsed ? "Click to show the cards of this driver" : "Click to hide the cards of this driver";
        return {};
    }

private:
    struct Route { int manual = 0; juce::String perspective; };
    struct Row
    {
        juce::String label, perspective, whereText;
        std::vector<Route> routes;    // one route, or every division's (a folded perspective)
        bool perspectiveRow = false, folded = false, silent = true;
        juce::Colour colour;
        juce::Rectangle<int> bounds;
    };
    struct Card
    {
        juce::String key, type, name, status, busyReason;
        bool busy = false, open = false, chip = false;   // chip = a button among the unused cards
        double rate = 0.0; int buffer = 0, latency = 0;
        std::vector<juce::Rectangle<int>> jacks;   // the outputs on the bottom edge
        juce::StringArray jackNames;               // the driver's channel names, drawn vertically above the jacks (the user: numbers alone say nothing)
        int nameBand = 0;                          // the height of that band (0 when the names are numbers only)
        std::vector<Route> routes;                 // the routes playing on it
        juce::AudioDeviceManager* manager = nullptr;
        juce::Rectangle<int> bounds;
        int gutterUse = 0;
    };
    struct Cable
    {
        int row = -1, card = -1, ring = 0;   // ring = this cable's order among the cables into the same jacks (concentric clamps)
        std::vector<int> channels;           // one or two (a pair)
        juce::Path path, hitPath;
        juce::Rectangle<int> removeBox;
    };
    struct CardControls
    {
        juce::ComboBox rate, buffer;
        juce::TextButton panel{ "Driver panel" }, reopen{ "Reopen" }, test{ "Test" };
    };
    struct TypeSection { juce::String type; bool collapsed = false; juce::Rectangle<int> bounds; };

    static constexpr int leftWidth = 250, rowHeight = 24, cardWidth = 372, cardHeight = 118, gutter = 44, cardGap = 28;

    float scale() const noexcept { return owner.audioRoutingButtonSize == 0 ? 0.85f : owner.audioRoutingButtonSize == 2 ? 1.25f : 1.0f; }
    int jackWidth() const noexcept { return juce::roundToInt(30.0f * scale()); }
    int jackHeight() const noexcept { return juce::roundToInt(24.0f * scale()); }
    int chipHeight() const noexcept { return juce::roundToInt(36.0f * scale()); }

    static juce::Colour colourOf(int manual, const juce::String& perspective)
    {
        const float hue = (float)(((perspective.toLowerCase().hashCode() & 0xffff) + 97 * manual) % 360) / 360.0f;
        return juce::Colour::fromHSV(hue, 0.6f, 0.95f, 1.0f);
    }
    static const juce::String& divisionName(int manual)
    {
        static const juce::String names[8]{ "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
        return names[juce::jlimit(0, 7, manual)];
    }
    static juce::String channelsText(const std::vector<int>& channels)
    {
        juce::StringArray parts;
        for (const int c : channels) parts.add(juce::String(c + 1));
        return parts.joinIntoString("+");
    }
    // A word of a device name that identifies the hardware ("Fireface", "Komplete"): letters only, 5+ long, not generic.
    static juce::StringArray hardwareWords(const juce::String& name)
    {
        static const juce::StringArray generic{ "audio", "output", "outputs", "input", "speakers", "definition", "device", "driver", "default", "digital", "analog", "aggregated", "wasapi", "windows", "all", "high", "realtek", "nvidia", "intel" };
        juce::StringArray words;
        for (const auto& raw : juce::StringArray::fromTokens(name, " ()[]-_/,.:", ""))
        {
            const auto word = raw.retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ").toLowerCase();
            if (word.length() >= 5 && !generic.contains(word)) words.addIfNotAlreadyThere(word);
        }
        return words;
    }

    const std::vector<int>& channelsOf(const Route& route) const
    {
        const auto& config = owner.manualConfigs[(size_t)route.manual];
        if (route.perspective.equalsIgnoreCase("default")) return config.outputChannels;
        const auto found = config.perspectiveAudio.find(MainComponent::perspectiveRouteKey(route.manual, route.perspective));
        static const std::vector<int> none;
        return found != config.perspectiveAudio.end() ? found->second.outputChannels : none;
    }
    // The card a route is SET to: the open device, else the device it asks for (0.8.250: a route keeps its card while
    // the card is off) - "" for none.
    juce::String cardKeyOf(const Route& route, juce::String* typeOut = nullptr, juce::String* nameOut = nullptr) const
    {
        auto* manager = owner.managerForRoute(route.manual, route.perspective);
        if (manager == nullptr) return {};
        if (auto* device = manager->getCurrentAudioDevice())
        {
            if (typeOut) *typeOut = manager->getCurrentAudioDeviceType();
            if (nameOut) *nameOut = device->getName();
            return manager->getCurrentAudioDeviceType() + "|" + device->getName();
        }
        // Not open: the stored state of the route says what it asks for (as the audio tab's savedDeviceName does -
        // the manager's own setup is empty after a failed open).
        const auto& config = owner.manualConfigs[(size_t)route.manual];
        const auto key = MainComponent::perspectiveRouteKey(route.manual, route.perspective);
        const auto foundAudio = config.perspectiveAudio.find(key);
        const juce::String xmlText = route.perspective.equalsIgnoreCase("default") ? config.audioDeviceStateXml
            : foundAudio != config.perspectiveAudio.end() ? foundAudio->second.audioDeviceStateXml : juce::String();
        const auto state = xmlText.isNotEmpty() ? juce::parseXML(xmlText) : nullptr;
        const auto name = state != nullptr ? state->getStringAttribute("audioOutputDeviceName") : juce::String();
        if (name.isEmpty()) return {};
        const auto type = state->getStringAttribute("deviceType", manager->getCurrentAudioDeviceType());
        if (typeOut) *typeOut = type;
        if (nameOut) *nameOut = name;
        return type + "|" + name;
    }
    int cardIndexOf(const juce::String& key) const
    {
        for (int i = 0; i < (int)cards.size(); ++i) if (cards[(size_t)i].key == key) return i;
        return -1;
    }
    std::vector<juce::Colour> jackColours(const Card& card, int channel) const
    {
        std::vector<juce::Colour> colours;
        for (const auto& cable : cables)
            if (cable.card == &card - cards.data())
                for (const int c : cable.channels) if (c == channel) colours.push_back(rows[(size_t)cable.row].colour);
        return colours;
    }

    // ---- the model -> rows, cards, cables ----
    void rebuild()
    {
        // The cards: every device name of every driver type (the start's scan plus what the system reported since -
        // never a scan of ours), the open ones with their figures.
        std::vector<Card> fresh;
        for (auto* type : owner.deviceManager.getAvailableDeviceTypes())
        {
            if (type == nullptr) continue;
            for (const auto& name : type->getDeviceNames(false))
            {
                Card card;
                card.type = type->getTypeName(); card.name = name; card.key = card.type + "|" + name;
                card.status = "not open";
                fresh.push_back(card);
            }
        }
        // The routes on them (a route's card may be absent from the lists: added).
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
            {
                Route route{ manual, perspective };
                juce::String type, name;
                const auto key = cardKeyOf(route, &type, &name);
                if (key.isEmpty()) continue;
                int index = -1;
                for (int i = 0; i < (int)fresh.size(); ++i) if (fresh[(size_t)i].key == key) index = i;
                if (index < 0) { Card card; card.type = type; card.name = name; card.key = key; fresh.push_back(card); index = (int)fresh.size() - 1; }
                auto& card = fresh[(size_t)index];
                card.routes.push_back(route);
                auto* manager = owner.managerForRoute(manual, perspective);
                auto* device = manager != nullptr ? manager->getCurrentAudioDevice() : nullptr;
                if (device == nullptr) { if (!card.open) card.status = "not available now (switched off, unplugged or taken) - tried again every 2 s"; }
                else if (!card.open)
                {
                    card.open = true; card.manager = manager;
                    card.rate = device->getCurrentSampleRate(); card.buffer = device->getCurrentBufferSizeSamples();
                    card.latency = device->getOutputLatencyInSamples();
                    card.jackNames = device->getOutputChannelNames();
                    const int outputs = card.jackNames.size();
                    card.jacks.assign((size_t)juce::jmax(0, outputs), {});
                    for (const auto& jackName : card.jackNames)   // a band only for real names ("ADAT (7+8) ..."), not "3" or "Output channel 3"
                        if (jackName.trim().retainCharacters("0123456789").length() != jackName.trim().length() && !jackName.trim().startsWithIgnoreCase("output channel ")) { card.nameBand = juce::roundToInt(150.0f * scale()); break; }
                    card.status = juce::String(outputs) + " outputs, " + juce::String(juce::roundToInt(card.rate)) + " Hz, buffer " + juce::String(card.buffer)
                        + " (" + juce::String(1000.0 * card.buffer / juce::jmax(1.0, card.rate), 1) + " ms), latency " + juce::String(card.latency) + " samples";
                }
            }
        // Busy: a card of one driver whose hardware word is in an OPEN device of another driver.
        for (auto& card : fresh)
        {
            if (card.open) continue;
            const auto words = hardwareWords(card.name);
            for (const auto& other : fresh)
            {
                if (!other.open || other.type == card.type) continue;
                for (const auto& word : hardwareWords(other.name))
                    if (words.contains(word)) { card.busy = true; card.busyReason = "busy: " + other.name + " is open through " + other.type; break; }
                if (card.busy) break;
            }
            if (card.busy) card.status = card.busyReason;
        }
        // A FULL card (outputs along its bottom edge) only for a card that plays something or is open; the rest are
        // buttons (the user: cards with many outputs would eat the height and send every cable far away).
        for (auto& card : fresh) card.chip = card.routes.empty() && !card.open;
        // The order: the user's (dragged) order first, then the lists' order.
        std::vector<Card> ordered;
        for (const auto& key : owner.audioRoutingCardOrder)
            for (auto& card : fresh) if (card.key == key && !card.chip && std::none_of(ordered.begin(), ordered.end(), [&](const Card& c) { return c.key == key; })) ordered.push_back(card);
        for (auto& card : fresh)
            if (!card.chip && std::none_of(ordered.begin(), ordered.end(), [&](const Card& c) { return c.key == card.key; })) ordered.push_back(card);
        for (auto& card : fresh) if (card.chip) ordered.push_back(card);
        cards = std::move(ordered);

        // The rows: per perspective a header row (folded when every division sends it the same way).
        rows.clear();
        std::vector<juce::String> perspectives;
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
                if (std::find(perspectives.begin(), perspectives.end(), perspective) == perspectives.end()) perspectives.push_back(perspective);
        for (const auto& perspective : perspectives)
        {
            std::vector<Route> routes;
            for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
                for (const auto& p : owner.engine.getPerspectivesForManual(manual)) if (p == perspective) routes.push_back({ manual, p });
            bool uniform = true;
            juce::String firstKey; std::vector<int> firstChannels;
            for (size_t i = 0; i < routes.size(); ++i)
            {
                const auto key = cardKeyOf(routes[i]);
                if (i == 0) { firstKey = key; firstChannels = channelsOf(routes[i]); }
                else if (key != firstKey || channelsOf(routes[i]) != firstChannels) { uniform = false; break; }
            }
            Row head;
            head.perspectiveRow = true; head.perspective = perspective; head.routes = routes;
            head.folded = uniform && unfolded.count(perspective) == 0;
            head.label = perspective + (routes.size() > 1 ? " (" + juce::String(routes.size()) + (uniform ? " divisions)" : " divisions, various)") : juce::String());
            head.colour = colourOf(0, perspective);
            head.silent = uniform ? firstKey.isEmpty() : false;
            head.whereText = uniform ? whereOf(routes.front()) : juce::String("the divisions differ - see them below");
            rows.push_back(head);
            if (!head.folded)
                for (const auto& route : routes)
                {
                    Row row;
                    row.perspective = perspective; row.routes = { route };
                    row.label = divisionName(route.manual) + "  " + perspective;
                    row.colour = colourOf(route.manual + 1, perspective);
                    row.silent = cardKeyOf(route).isEmpty();
                    row.whereText = whereOf(route);
                    rows.push_back(row);
                }
        }
        // The card controls.
        controls.clear();
        for (int i = 0; i < (int)cards.size(); ++i)
        {
            const auto& card = cards[(size_t)i];
            if (card.chip) { controls.add(nullptr); continue; }
            auto* c = controls.add(new CardControls());
            auto* manager = card.manager;
            auto* device = manager != nullptr ? manager->getCurrentAudioDevice() : nullptr;
            if (device != nullptr)
            {
                for (const double rate : device->getAvailableSampleRates()) c->rate.addItem(juce::String(juce::roundToInt(rate)) + " Hz", juce::roundToInt(rate));
                c->rate.setSelectedId(juce::roundToInt(card.rate), juce::dontSendNotification);
                // The buffer sizes the driver offers (ASIO: its range as JUCE reads it from the driver); the current
                // size is listed even when the driver's list lacks it (a size set in the ASIO panel).
                auto sizes = device->getAvailableBufferSizes();
                if (!sizes.contains(card.buffer) && card.buffer > 0) sizes.add(card.buffer);
                sizes.sort();
                for (const int size : sizes) c->buffer.addItem(juce::String(size), size);
                c->buffer.setSelectedId(card.buffer, juce::dontSendNotification);
                c->rate.onChange = [this, manager, box = &c->rate] { auto setup = manager->getAudioDeviceSetup(); setup.sampleRate = box->getSelectedId(); reopenWith(manager, setup, "rate"); };
                c->buffer.onChange = [this, manager, box = &c->buffer] { auto setup = manager->getAudioDeviceSetup(); setup.bufferSize = box->getSelectedId(); reopenWith(manager, setup, "buffer"); };
                c->panel.setEnabled(device->hasControlPanel());
                c->panel.onClick = [device] { device->showControlPanel(); };
                c->reopen.onClick = [this, manager] { owner.reopenAudioDeviceFresh(*manager, "Audio routing: Reopen"); };
            }
            else { c->rate.setEnabled(false); c->buffer.setEnabled(false); c->panel.setEnabled(false); c->reopen.setEnabled(false); }
            c->rate.setTooltip("The sample rate of this card (every route on it reopens)"); c->buffer.setTooltip("The buffer size of this card (every route on it reopens); a size set in the driver's panel shows here within a second");
            c->panel.setTooltip("The driver's own control panel (ASIO)"); c->reopen.setTooltip("Closes and opens the card again");
            c->test.setTooltip("A soft tone for a second on every route of this card - you hear where it goes");
            c->test.setEnabled(!card.routes.empty() && card.open);
            const auto routes = card.routes;
            c->test.onClick = [this, routes] { for (const auto& route : routes) owner.startRouteTestTone(route.manual, route.perspective); };
            for (auto* comp : { (juce::Component*)&c->rate, (juce::Component*)&c->buffer, (juce::Component*)&c->panel, (juce::Component*)&c->reopen, (juce::Component*)&c->test })
                addAndMakeVisible(comp);
        }
        layout();
        repaint();
    }
    juce::String whereOf(const Route& route) const
    {
        juce::String type, name;
        if (cardKeyOf(route, &type, &name).isEmpty()) return "no sound card (silent)";
        return name + " (" + type + "), outputs " + channelsText(channelsOf(route)) + (channelsOf(route).size() > 2 ? " - the pipes alternate between the pairs by key" : "");
    }
    void layout()
    {
        const int width = juce::jmax(700, getWidth());
        help.setBounds(10, 6, width - 330, 56);
        showCables.setBounds(width - 310, 8, 110, 24);
        sizeLabel.setBounds(width - 200, 8, 90, 24);
        sizeBox.setBounds(width - 106, 8, 96, 24);
        sourcesTop = 94;
        int y = sourcesTop;
        for (auto& row : rows) { row.bounds = { 12, y, leftWidth - 16, rowHeight }; y += rowHeight; }
        sourcesBottom = y;
        // The full cards (in use / open) flow right of the sources, each as wide as its outputs need (32 outputs =
        // a wide card, never a taller one); a row of cards leaves room below for the cable runs.
        const int gridLeft = leftWidth + 20 + gutter;
        const int right = width - 10;
        int x = gridLeft, y0 = sourcesTop - 2, rowBottom = y0;
        slots.clear();
        const int jw0 = jackWidth(), jh = jackHeight();
        for (int i = 0; i < (int)cards.size(); ++i)
        {
            auto& card = cards[(size_t)i];
            if (card.chip) continue;
            const int n = (int)card.jacks.size();
            const int jw = n > 0 ? juce::jlimit(16, jw0, (right - gridLeft - 20) / juce::jmax(1, n) - 2) : jw0;
            const int w = juce::jmax(cardWidth, 20 + n * (jw + 2));
            if (x + w > right && x > gridLeft) { x = gridLeft; y0 = rowBottom + 60; }
            card.bounds = { x, y0, w, cardHeight + card.nameBand };
            slots.push_back(card.bounds);
            for (int ch = 0; ch < n; ++ch)
                card.jacks[(size_t)ch] = { card.bounds.getX() + 10 + ch * (jw + 2), card.bounds.getBottom() - jh - 4, jw, jh };
            if (auto* c = controls[i])
            {
                const int cx = card.bounds.getX() + 10, cy = card.bounds.getY() + 44;
                c->rate.setBounds(cx, cy, 90, 22); c->buffer.setBounds(cx + 96, cy, 70, 22);
                c->panel.setBounds(cx + 172, cy, 84, 22); c->reopen.setBounds(cx + 260, cy, 56, 22); c->test.setBounds(cx + 320, cy, 42, 22);
            }
            x += w + gutter + cardGap;
            rowBottom = juce::jmax(rowBottom, card.bounds.getBottom());
        }
        if (!slots.empty()) slots.push_back(juce::Rectangle<int>(x > right - cardWidth ? gridLeft : x, x > right - cardWidth ? rowBottom + 60 : y0, cardWidth, cardHeight));   // the place after the last card
        fullCardsBottom = rowBottom + 60;
        // The other cards as buttons, a collapsible section per driver, below the full cards and the sources.
        chipsTop = juce::jmax(fullCardsBottom, sourcesBottom + 24);
        int cy = chipsTop + 24, cx = 12;
        juce::String lastType;
        typeSections.clear();
        bool collapsed = false;
        const int ch = chipHeight();
        for (auto& card : cards)
        {
            if (!card.chip) continue;
            if (card.type != lastType)
            {
                if (lastType.isNotEmpty()) cy += collapsed ? 8 : ch + 8;   // past the last row of the previous driver
                collapsed = owner.audioRoutingCollapsed.contains(card.type);
                typeSections.push_back({ card.type, collapsed, { 12, cy, 400, 22 } });
                lastType = card.type; cx = 12; cy += 26;
            }
            if (collapsed) { card.bounds = {}; continue; }
            const int w = juce::jlimit(juce::roundToInt(120.0f * scale()), juce::roundToInt(320.0f * scale()),
                24 + juce::roundToInt(juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(14.0f * scale())), card.name)));
            if (cx + w > right) { cx = 12; cy += ch + 6; }
            card.bounds = { cx, cy, w, ch };
            cx += w + 6;
        }
        const int bottom = lastType.isNotEmpty() ? cy + (collapsed ? 4 : ch + 8) : chipsTop;
        buildCables();
        const int height = juce::jmax(sourcesBottom + 20, bottom + 10);
        if (height != getHeight()) setSize(getWidth(), height);
    }
    // The cables from the rows to the jacks: a lane per row (its y), a gutter per card (vertical runs side by side),
    // under the card to the jack, up into it. Hit paths for the mouse.
    void buildCables()
    {
        cables.clear();
        for (auto& card : cards) card.gutterUse = 0;
        for (int r = 0; r < (int)rows.size(); ++r)
        {
            const auto& row = rows[(size_t)r];
            if (row.perspectiveRow && !row.folded) continue;   // the divisions below carry the cables
            if (row.routes.empty()) continue;
            const auto& route = row.routes.front();
            const int cardIndex = cardIndexOf(cardKeyOf(route));
            if (cardIndex < 0) continue;
            auto channels = channelsOf(route);
            std::sort(channels.begin(), channels.end());
            for (size_t i = 0; i < channels.size();)
            {
                Cable cable;
                cable.row = r; cable.card = cardIndex;
                cable.channels = { channels[i] };
                if (i + 1 < channels.size() && channels[i + 1] == channels[i] + 1) { cable.channels.push_back(channels[i + 1]); i += 2; } else ++i;
                for (const auto& other : cables) if (other.card == cable.card && other.channels.front() == cable.channels.front()) ++cable.ring;
                cables.push_back(cable);
            }
        }
        for (auto& cable : cables)
        {
            auto& card = cards[(size_t)cable.card];
            const auto& row = rows[(size_t)cable.row];
            const int lane = card.gutterUse++;
            const float gx = (float)card.bounds.getX() - 8.0f - 4.0f * (float)lane;
            const float y0 = (float)row.bounds.getCentreY();
            const float x0 = (float)row.bounds.getRight();
            const float under = (float)card.bounds.getBottom() + 10.0f + 4.0f * (float)lane;
            const int first = cable.channels.front();
            const auto jack = first < (int)card.jacks.size() ? card.jacks[(size_t)first] : card.bounds.withTrimmedTop(card.bounds.getHeight() - 10).withWidth(20);
            const float jx = (float)jack.getCentreX() + (cable.channels.size() > 1 ? (float)(jack.getWidth() + 2) / 2.0f : 0.0f) + 3.0f * (float)cable.ring;
            juce::Path p;
            p.startNewSubPath(x0, y0);
            p.lineTo(gx - 6.0f, y0); p.quadraticTo(gx, y0, gx, y0 + 6.0f);
            p.lineTo(gx, under - 6.0f); p.quadraticTo(gx, under, gx + 6.0f, under);
            p.lineTo(jx - 6.0f, under); p.quadraticTo(jx, under, jx, under - 6.0f);
            p.lineTo(jx, (float)jack.getBottom() + 3.0f * (float)cable.ring);
            cable.path = p;
            cable.hitPath.clear();
            juce::PathStrokeType(10.0f).createStrokedPath(cable.hitPath, p);
            cable.removeBox = juce::Rectangle<int>((int)jx - 7, (int)under - 7, 14, 14);
        }
    }
    void paintCables(juce::Graphics& g)
    {
        for (int i = 0; i < (int)cables.size(); ++i)
        {
            const auto& cable = cables[(size_t)i];
            const auto& row = rows[(size_t)cable.row];
            const bool selected = i == selectedCable;
            const float alpha = selectedCable >= 0 && !selected ? 0.25f : 0.9f;
            g.setColour(row.colour.withAlpha(alpha));
            const float width = selected ? 3.5f : 2.5f;
            const auto& card = cards[(size_t)cable.card];
            const float ring = 3.0f * (float)cable.ring;   // concentric clamps for several cables into the same outputs
            if (cable.channels.size() > 1)
            {
                g.strokePath(cable.path, juce::PathStrokeType(width, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), juce::AffineTransform::translation(-1.6f, -1.6f));
                g.strokePath(cable.path, juce::PathStrokeType(width, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), juce::AffineTransform::translation(1.6f, 1.6f));
                if (cable.channels.back() < (int)card.jacks.size())
                {
                    const auto a = card.jacks[(size_t)cable.channels.front()], b = card.jacks[(size_t)cable.channels.back()];
                    g.drawRoundedRectangle(juce::Rectangle<float>((float)a.getX() - ring, (float)a.getY() - 3.0f - ring, (float)(b.getRight() - a.getX()) + 2.0f * ring, (float)a.getHeight() + 6.0f + 2.0f * ring), 4.0f, 2.0f);
                }
            }
            else
            {
                g.strokePath(cable.path, juce::PathStrokeType(width, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                if (cable.channels.front() < (int)card.jacks.size() && cable.ring > 0)
                {
                    const auto a = card.jacks[(size_t)cable.channels.front()];
                    g.drawRoundedRectangle(a.toFloat().expanded(3.0f + ring), 4.0f, 2.0f);
                }
            }
            if (selected)
            {
                g.setColour(juce::Colours::red);
                g.fillEllipse(cable.removeBox.toFloat());
                g.setColour(juce::Colours::white);
                g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                g.drawText("x", cable.removeBox, juce::Justification::centred);
            }
        }
    }
    void findDropTarget(juce::Point<int> p)
    {
        dropCard = -1; dropChannel = -1;
        for (int i = 0; i < (int)cards.size(); ++i)
        {
            const auto& card = cards[(size_t)i];
            if (card.bounds.isEmpty() || !card.bounds.expanded(6).contains(p) || card.busy) continue;
            dropCard = i;
            if (card.chip || card.jacks.empty()) { dropChannel = 0; return; }
            int best = -1; int bestDistance = 1 << 30;
            for (int ch = 0; ch < (int)card.jacks.size(); ++ch)
            {
                const int d = card.jacks[(size_t)ch].getCentre().getDistanceFrom(p);
                if (d < bestDistance) { bestDistance = d; best = ch; }
            }
            dropChannel = best;
            if (dropPairs && dropChannel + 1 >= (int)card.jacks.size() && dropChannel > 0 && (int)card.jacks.size() % 2 == 0) dropChannel -= 1;   // the last output alone: the pair below it
            return;
        }
    }

    // ---- the actions ----
    void assign(const std::vector<Route>& routes, const Card& card, std::vector<int> channels, bool add, const std::vector<int>& takeAway = {})
    {
        juce::StringArray done, refused;
        const double now = juce::Time::getMillisecondCounterHiRes();
        bool deviceChanged = false;
        for (const auto& route : routes)
        {
            juce::String type, name;
            const auto key = cardKeyOf(route, &type, &name);
            if (key != card.key)
            {
                if (!deviceChanged) { owner.audioMuted.store(true); owner.engine.noteOffAll(); owner.ignoreDeviceRestartsUntil.store(now + 8000.0); deviceChanged = true; }
                auto state = std::make_unique<juce::XmlElement>("DEVICESETUP");
                state->setAttribute("deviceType", card.type);
                state->setAttribute("audioOutputDeviceName", card.name);
                state->setAttribute("audioInputDeviceName", "");
                if (card.rate > 0.0) state->setAttribute("audioDeviceRate", card.rate);
                if (card.buffer > 0) state->setAttribute("audioDeviceBufferSize", card.buffer);
                owner.setRouteAudioType(route.manual, route.perspective, card.type, state.get(), card.name);
                add = false;   // a route moved to another card starts with the dropped pair alone
            }
            auto wanted = channels;
            if (add)
                for (const int c : channelsOf(route))
                    if (std::find(wanted.begin(), wanted.end(), c) == wanted.end() && std::find(takeAway.begin(), takeAway.end(), c) == takeAway.end()) wanted.push_back(c);
            std::sort(wanted.begin(), wanted.end());
            if (owner.applyChannelsToDivision(route.manual, route.perspective, wanted, done, refused))
                CrashHandler::note("Audio routing: " + divisionName(route.manual) + " " + route.perspective + " -> " + card.name + " (" + card.type + ") outputs " + channelsText(wanted));
            else CrashHandler::note("Audio routing: " + divisionName(route.manual) + " " + route.perspective + " -> " + card.name + " outputs " + channelsText(wanted) + " REFUSED (the card has no such output)");
        }
        if (!refused.isEmpty()) owner.noteMessage("Audio routing", "Not set: " + refused.joinIntoString("; "));
        owner.saveSettings();
        if (deviceChanged) owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1500.0;
        selectedCable = -1;
        rebuild();
    }
    // The selected cable dragged onto another output (or card): its channels leave, the new ones come; the route's
    // other cables stay (on another card the whole route moves - a route plays through one card).
    void moveCable(int index, const Card& card, const std::vector<int>& channels)
    {
        if (index < 0 || index >= (int)cables.size()) return;
        const auto cable = cables[(size_t)index];
        const auto routes = rows[(size_t)cable.row].routes;
        CrashHandler::note("Audio routing: cable " + channelsText(cable.channels) + " of " + rows[(size_t)cable.row].label + " moved to " + card.name + " outputs " + channelsText(channels));
        assign(routes, card, channels, true, cable.channels);
    }
    void removeCable(int index)
    {
        if (index < 0 || index >= (int)cables.size()) return;
        const auto cable = cables[(size_t)index];
        const auto& row = rows[(size_t)cable.row];
        juce::StringArray done, refused;
        bool detached = false;
        for (const auto& route : row.routes)
        {
            auto channels = channelsOf(route);
            for (const int c : cable.channels) channels.erase(std::remove(channels.begin(), channels.end(), c), channels.end());
            if (channels.empty())
            {
                if (!detached) { owner.audioMuted.store(true); owner.engine.noteOffAll(); owner.ignoreDeviceRestartsUntil.store(juce::Time::getMillisecondCounterHiRes() + 4000.0); detached = true; }
                owner.detachRouteAudioQuietly(route.manual, route.perspective);
                CrashHandler::note("Audio routing: " + divisionName(route.manual) + " " + route.perspective + " -> no sound card (its last cable removed)");
            }
            else
            {
                owner.applyChannelsToDivision(route.manual, route.perspective, channels, done, refused);
                CrashHandler::note("Audio routing: " + divisionName(route.manual) + " " + route.perspective + " outputs " + channelsText(channels) + " (cable " + channelsText(cable.channels) + " removed)");
            }
        }
        owner.saveSettings();
        if (detached) owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1200.0;
        selectedCable = -1;
        rebuild();
    }
    // A full card dragged onto a slot of the grid: the order is the machine's memory.
    void moveCard(int from, int slot)
    {
        std::vector<juce::String> order;
        for (const auto& card : cards) if (!card.chip) order.push_back(card.key);
        if (from < 0 || from >= (int)order.size()) return;
        const auto key = order[(size_t)from];
        order.erase(order.begin() + from);
        slot = juce::jlimit(0, (int)order.size(), slot > from ? slot - 1 : slot);
        order.insert(order.begin() + slot, key);
        owner.audioRoutingCardOrder.clear();
        for (const auto& k : order) owner.audioRoutingCardOrder.add(k);
        owner.saveSettings();
        CrashHandler::note("Audio routing: card " + key + " moved to place " + juce::String(slot + 1));
        rebuild();
    }
    void reopenWith(juce::AudioDeviceManager* manager, const juce::AudioDeviceManager::AudioDeviceSetup& setup, const juce::String& what)
    {
        owner.audioMuted.store(true);
        owner.engine.noteOffAll();
        owner.ignoreDeviceRestartsUntil.store(juce::Time::getMillisecondCounterHiRes() + 4000.0);
        const auto error = manager->setAudioDeviceSetup(setup, true);
        CrashHandler::note("Audio routing: " + what + " of " + setup.outputDeviceName + " set to " + (what == "rate" ? juce::String(juce::roundToInt(setup.sampleRate)) + " Hz" : juce::String(setup.bufferSize)) + (error.isNotEmpty() ? " - " + error : juce::String()));
        if (error.isNotEmpty()) owner.noteMessage("Audio routing", error);
        owner.saveSettings();
        owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1500.0;
        rebuild();
    }
    void runTestSpec(const juce::String& spec)
    {
        const auto parts = juce::StringArray::fromTokens(spec, "|", "");
        if (parts.size() >= 3 && parts[0].trim().equalsIgnoreCase("card"))
        {
            int index = -1, full = 0;
            for (int i = 0; i < (int)cards.size(); ++i) { if (cards[(size_t)i].chip) continue; if (index < 0 && cards[(size_t)i].name.containsIgnoreCase(parts[1].trim())) index = full; ++full; }
            CrashHandler::note("ORGAN_TEST_ROUTE: " + spec + (index < 0 ? " - no such full card" : ""));
            if (index >= 0) moveCard(index, parts[2].getIntValue() - 1);
            return;
        }
        if (parts.size() < 4) { CrashHandler::note("ORGAN_TEST_ROUTE: bad spec " + spec); return; }
        std::vector<Route> routes;
        const auto perspective = parts[1].trim();
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            if (parts[0].trim().equalsIgnoreCase("all") || parts[0].getIntValue() == manual)
                for (const auto& p : owner.engine.getPerspectivesForManual(manual)) if (p.equalsIgnoreCase(perspective)) routes.push_back({ manual, p });
        int cardIndex = -1;
        for (int i = 0; i < (int)cards.size(); ++i) if (cards[(size_t)i].name.containsIgnoreCase(parts[2].trim())) { cardIndex = i; break; }
        if (routes.empty() || cardIndex < 0) { CrashHandler::note("ORGAN_TEST_ROUTE: no route or card for " + spec); return; }
        const int output = parts[3].getIntValue() - 1;
        const auto mode = parts.size() >= 5 ? parts[4].trim().toLowerCase() : juce::String();
        CrashHandler::note("ORGAN_TEST_ROUTE: " + spec);
        auto cableOf = [&](int fromOutput)
        {
            for (int i = 0; i < (int)cables.size(); ++i)
                if (cables[(size_t)i].channels.front() == fromOutput && rows[(size_t)cables[(size_t)i].row].routes.front().manual == routes.front().manual
                    && rows[(size_t)cables[(size_t)i].row].routes.front().perspective.equalsIgnoreCase(perspective)) return i;
            return -1;
        };
        std::vector<int> channels{ output };
        if (mode != "single" && (cards[(size_t)cardIndex].jacks.empty() || output + 1 < (int)cards[(size_t)cardIndex].jacks.size())) channels.push_back(output + 1);
        if (mode == "remove")
        {
            const int i = cableOf(output);
            if (i >= 0) removeCable(i); else CrashHandler::note("ORGAN_TEST_ROUTE: no cable to remove");
            return;
        }
        if (mode == "movecable")
        {
            const int i = cableOf(parts.size() >= 6 ? parts[5].getIntValue() - 1 : 0);
            if (i >= 0) moveCable(i, cards[(size_t)cardIndex], channels); else CrashHandler::note("ORGAN_TEST_ROUTE: no cable to move");
            return;
        }
        assign(routes, cards[(size_t)cardIndex], channels, mode == "add");
    }
    juce::String signature() const
    {
        juce::String s;
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
            {
                s << cardKeyOf({ manual, perspective }) << ":" << channelsText(channelsOf({ manual, perspective })) << ";";
                if (auto* manager = owner.managerForRoute(manual, perspective))
                    if (auto* device = manager->getCurrentAudioDevice())   // rate / buffer / latency: a size set in the ASIO panel shows within a second
                        s << juce::roundToInt(device->getCurrentSampleRate()) << "/" << device->getCurrentBufferSizeSamples() << "/" << device->getOutputLatencyInSamples() << ";";
            }
        for (auto* type : owner.deviceManager.getAvailableDeviceTypes()) if (type != nullptr) s << type->getDeviceNames(false).size() << ",";
        return s;
    }
    void timerCallback() override
    {
        if (dragging || dragCard >= 0 || pressedCable >= 0) return;
        const auto now = signature();
        if (now != lastSignature) { lastSignature = now; rebuild(); }
    }

    MainComponent& owner;
    juce::Label help, sizeLabel;
    juce::ToggleButton showCables;
    juce::ComboBox sizeBox;
    std::vector<Row> rows;
    std::vector<Card> cards;
    std::vector<Cable> cables;
    std::vector<juce::Rectangle<int>> slots;   // the places of the full cards (+ one after the last) while a card is dragged
    juce::OwnedArray<CardControls> controls;
    std::set<juce::String> unfolded;
    std::vector<TypeSection> typeSections;
    int sourcesTop = 94, sourcesBottom = 94, fullCardsBottom = 94, chipsTop = 94;
    int dragRow = -1, dragCard = -1, cableDrag = -1, pressedCable = -1, dropCard = -1, dropChannel = -1, dropSlot = -1, selectedCable = -1;
    bool dragging = false, draggingCard = false, dropAdd = false, dropPairs = true;
    juce::Point<float> dragPoint;
    juce::String lastSignature;
};
