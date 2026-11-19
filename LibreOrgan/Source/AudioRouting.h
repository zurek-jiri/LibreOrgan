#pragma once
// 0.9.81-0.9.83 (#236, the user's design): THE AUDIO ROUTING DIALOG - "Organ out" (the sources), "Sound cards in"
// (the cards) and the cables between them.
//   - LEFT ("Organ out"): the sources grouped by PERSPECTIVE: a perspective row ("Front") rolls the divisions up when
//     every division sends it the same way (one cable for all); "(various)" and unfolded when they differ; the arrow
//     unfolds the divisions (P., I., II. ...) so each gets its own cable. Where a source plays now is in the tooltip.
//   - RIGHT ("Sound cards in"): the cards IN USE or open as full cards on a GRID OF CELLS like the stop grid (0.9.83:
//     a card has its cell, remembered per machine; a wide card spans several cells; dragged by its title onto a free
//     cell - dashed while dragging, orange under the mouse - or onto another card to swap) - name, driver + status,
//     Sample rate / Buffer combos (ASIO too), the latency, Driver panel, Reopen, Test - with the OUTPUTS as jacks along
//     the bottom edge and the driver's channel names standing vertically above them; a card grows WIDE with its
//     outputs, never tall. Every other card of every driver is a button sized like a stop button in a COLLAPSIBLE
//     section per driver: ASIO first and unfolded, then Aggregated WASAPI, then the rest folded (the user: "to jsou
//     vesměs shity"; without ASIO the aggregate is first and unfolded); a drop on a button opens the card with the
//     pair 1+2 and it moves up. A card busy through another driver on the same hardware is grey and takes no cable.
//     Cards plugged in after the start appear as the system reports them (no scan of ours).
//   - CABLES run in the AISLES only: from the source row right into the gutter left of the grid, down (or up) the
//     gutter to the corridor under the target card's row, along the corridor to the output, up into it - never across
//     a card, side by side 8 px apart (the gutter and the corridors widen with the cables they carry). A pair = two
//     strands + a clamp over both outputs (several cables into one pair = concentric clamps, the jack striped in all
//     their colours), a single channel one strand. Drag a source onto an output: the output and its pair partner
//     glow ORANGE (the colour of a stop switched on); drop = that pair replaces the route's channels, Shift = the pair
//     is added (C / C# split: the pipes alternate between the pairs by key), Alt = one channel. PRESS A CABLE and it is
//     picked up: it leaves its output and follows the mouse, let go on an output (or card) plugs it there, anywhere
//     else it returns; a click alone selects it (the others dim), Delete or the x at its end removes it.
// Everything goes through the Divisions dialog's own calls (setRouteAudioType, applyChannelsToDivision,
// detachRouteAudioQuietly); both dialogs read the same model and refresh by timer (rate, buffer and latency too:
// a size set in the ASIO panel shows within a second).
// Dev: ORGAN_TEST_ROUTE="<division|all>|<perspective>|<card name part>|<output 1-based>[|add|single|remove|movecable
// <from output>]" or "cell|<card name part>|<column>|<row>" (several specs separated by ^, the first 1.5 s after the
// dialog opens, then 3 s apart) drives the actions without a mouse; the log says "Audio routing: ...".
// 0.9.89 (the user): the rows of "Organ out" taller with a larger font and dots, a mini audio meter and the fx
// button (grey = everything default, ORANGE = an effect set or a module set apart) on every row - the fx opens the
// Effects dialog (EffectsDialog.h) of the route (a division row) or of the perspective; the header carries the
// master volume (the console's slider, one value), the master audio meter (the console's, one state) and Measure...
// (the polyphony measurement - the Divisions dialog lost its audio tabs, this dialog is the one way to the audio).
#include <JuceHeader.h>
#include "MainComponent.h"
#include "UIControls.h"
#include "PlatformAudio.h"

class AudioRoutingContent final : public juce::Component, public juce::TooltipClient, private juce::Timer
{
public:
    explicit AudioRoutingContent(MainComponent& o) : owner(o), masterMeter(o)
    {
        getProperties().set("manualChapter", "Audio routing");
        setWantsKeyboardFocus(true);
        // 0.9.114 (the user: "ty texty okolo jsou malé, a nedají se číst. A s velikostí button size se nemění ...
        // text musí být větší ... ale zase o to stručnější"; Shift explained - a multi-channel output of a division,
        // the C / C# sides -, Alt = one channel, "mono"): short sentences at 15 px times Button size (layout()); when
        // the header has no room for it a "?" stands there instead (the text in its tooltip, a click = the manual).
        help.setText("Drag a perspective onto an output of a card: it plays through that stereo pair. "
            "Shift + drag adds another pair for the same source - a multi-channel output of a division: its pipes alternate between the pairs by key (C side / C# side). "
            "Alt + drag takes one channel (mono). "
            "Press a cable to move it to another output; Delete removes the selected cable. Drag a card by its title. "
            "fx = the effects of a row. F1 = the manual.",
            juce::dontSendNotification);
        help.setFont(juce::FontOptions(15.0f));
        help.setMinimumHorizontalScale(1.0f);
        help.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        help.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(help);
        helpButton.setButtonText("?");
        helpButton.setTooltip(help.getText());
        helpButton.onClick = [this] { juce::Component::SafePointer<MainComponent> target(&owner); juce::MessageManager::callAsync([target] { if (target != nullptr) target->showManualSection("Audio routing"); }); };
        addChildComponent(helpButton);
        showCables.setButtonText("Show cables");
        showCables.setToggleState(true, juce::dontSendNotification);
        showCables.onClick = [this] { repaint(); };
        addAndMakeVisible(showCables);
        sizeLabel.setText("Button size:", juce::dontSendNotification);
        sizeLabel.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(sizeLabel);
        sizeBox.addItem("compact", 4); sizeBox.addItem("small", 1); sizeBox.addItem("normal", 2); sizeBox.addItem("large", 3);   // 0.9.93: compact = size 3
        sizeBox.setSelectedId(juce::jlimit(0, 3, owner.audioRoutingButtonSize) + 1, juce::dontSendNotification);
        sizeBox.setTooltip("The size of the sound card buttons, the outputs and their names. Compact, for a small screen: small squares, the first ten letters of a channel's name (the whole name in its tooltip).");
        sizeBox.onChange = [this] { owner.audioRoutingButtonSize = sizeBox.getSelectedId() - 1; owner.saveSettings(); rebuild(); };
        addAndMakeVisible(sizeBox);
        // The master strip: the console's master volume (the same value, two places - never a copy), the console's
        // audio meter, and the polyphony measurement.
        masterLabel.setText("Master volume", juce::dontSendNotification);
        masterLabel.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(masterLabel);
        masterSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        masterSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
        masterSlider.setRange(owner.masterVolumeSlider.getMinimum(), owner.masterVolumeSlider.getMaximum(), 0.1);
        masterSlider.setTextValueSuffix(" dB");
        masterSlider.setScrollWheelEnabled(true);
        masterSlider.setDoubleClickReturnValue(true, owner.masterVolumeSlider.getDoubleClickReturnValue());
        masterSlider.setValue(owner.masterVolumeSlider.getValue(), juce::dontSendNotification);
        masterSlider.setTooltip("The level of the whole organ - the console's master slider itself. Right-click: MIDI learn.");
        masterSlider.getProperties().set("midiTarget", "slider:global:master");
        masterSlider.getProperties().set("manualChapter", "Master volume");
        masterSlider.addMouseListener(&owner, false);
        masterSlider.onValueChange = [this] { owner.masterVolumeSlider.setValue(masterSlider.getValue(), juce::sendNotification); };
        addAndMakeVisible(masterSlider);
        masterMeter.setTooltip("The audio meter of the console: the highest sample of every route and device before the limiter (green to -12 dB, orange to -3 dB, red = the limiter at work); the CLIP dot stays lit until you click it.");
        masterMeter.getProperties().set("manualChapter", "The polyphony display");
        addAndMakeVisible(masterMeter);
        measureButton.setButtonText("Measure Polyphony");   // 0.9.90 (the user: "Measure..." said nothing)
        measureButton.setTooltip("Measures the most voices this organ renders cleanly with the devices, buffers and effects as set now, "
            "and proposes the LIMIT of the polyphony display (the Configuration Editor has the same button).");
        measureButton.getProperties().set("manualChapter", "Measure the maximum polyphony");
        measureButton.onClick = [this] { juce::Component::SafePointer<MainComponent> target(&owner); juce::MessageManager::callAsync([target] { if (target != nullptr) target->startPolyphonyMeasurement(); }); };
        addAndMakeVisible(measureButton);
        // 0.9.94: the routing file. Save writes everything (cables and effects); Load asks once - cables only (the
        // default) or the effects too; the default serves organs that bring no audio of this computer, cables only.
        auto later = [this](void (MainComponent::*action)())
        {
            juce::Component::SafePointer<MainComponent> target(&owner);
            juce::MessageManager::callAsync([target, action] { if (target != nullptr) (target.getComponent()->*action)(); });
        };
        saveRoutingButton.setButtonText("Save routing...");
        saveRoutingButton.setTooltip("Saves the cables (sound cards, outputs, rate, buffer) and the effects of every route into a file.");
        saveRoutingButton.onClick = [later] { later(&MainComponent::saveRoutingToFile); };
        loadRoutingButton.setButtonText("Load routing...");
        loadRoutingButton.setTooltip("Loads a routing file: the cables only, or the effects too - it asks. The effects carry the volumes of the divisions and perspectives.");
        loadRoutingButton.onClick = [later] { later(&MainComponent::loadRoutingFromFile); };
        defaultRoutingButton.setButtonText("Use as default for new organs");
        defaultRoutingButton.setTooltip("These cables become this computer's default: an organ that brings no audio of this computer (a published configuration, "
            "a Hauptwerk import, folders dropped on an empty console) gets them by itself - the cables only, never the effects. "
            "To have no default, delete DefaultRouting.json in the settings folder.");
        defaultRoutingButton.onClick = [this]
        {
            owner.saveDefaultRouting();
            defaultRoutingButton.setButtonText("Saved as the default");
            juce::Component::SafePointer<AudioRoutingContent> safeThis(this);
            juce::Timer::callAfterDelay(2000, [safeThis] { if (safeThis != nullptr) safeThis->defaultRoutingButton.setButtonText("Use as default for new organs"); });
        };
        for (auto* b : { &saveRoutingButton, &loadRoutingButton, &defaultRoutingButton })
        {
            b->getProperties().set("manualChapter", "Routing files");
            addAndMakeVisible(*b);
        }
#if JUCE_LINUX
        // The Linux switch of the audio tabs (0.8.228): every ALSA device listed, the unusable ones too.
        unusableButton.setButtonText("List unusable devices too");
        unusableButton.setTooltip("An ALSA card that cannot be opened now (PipeWire or another program holds it) is left out of the list. "
            "Ticked: it is listed too, dimmed, with the reason in its tooltip - a cable dropped on it waits until the card is free.");
        unusableButton.setToggleState(owner.showUnusableAudioDevices, juce::dontSendNotification);
        unusableButton.onClick = [this]
        {
            owner.showUnusableAudioDevices = unusableButton.getToggleState();
            unusableAudioDevicesShown().store(owner.showUnusableAudioDevices);
            owner.saveSettings();
            // 0.9.95 (#242): the lists are READ AGAIN at once - the tick did nothing until the next start.
            owner.rescanAudioDeviceLists(owner.showUnusableAudioDevices ? "unusable devices listed too" : "unusable devices left out");
            rebuild();
        };
        addAndMakeVisible(unusableButton);
        // 0.9.114: where PipeWire carries the cards, their direct ALSA devices behind a tick box that shows only in the
        // UNFOLDED ALSA section, under its caption, with the warning beside it (painted: directAlsaNote).
        directAlsaButton.setButtonText("Show the cards' direct ALSA devices - NOT recommended!");
        directAlsaButton.setTooltip("Lists every card's direct ALSA device beside PipeWire's entries. NOT recommended, and mutually exclusive with PipeWire: "
            "a card opened directly is taken whole - it plays nowhere else, not through PipeWire or JACK, in this program or any other, until every cable is removed from it. "
            "A card that plays through PipeWire here cannot be opened directly before its PipeWire cables are removed (or moved over). "
            "Unticked (the default): a route left on a direct device goes back to the card's PipeWire entry the next time it opens.");
        directAlsaButton.setToggleState(owner.showDirectAlsaDevices, juce::dontSendNotification);
        directAlsaButton.onClick = [this]
        {
            owner.showDirectAlsaDevices = directAlsaButton.getToggleState();
            directAlsaDevicesShown().store(owner.showDirectAlsaDevices);
            owner.saveSettings();
            owner.rescanAudioDeviceLists(owner.showDirectAlsaDevices ? "the cards' direct ALSA devices listed too" : "the cards' direct ALSA devices left out");
            rebuild();
        };
        addChildComponent(directAlsaButton);
#endif
        rebuild();
        startTimer(100);   // the meters; the model's signature every 10th tick (1 s, as before)
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
        const auto glow = onColour();
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.setColour(juce::Colours::white.withAlpha(0.14f));   // 0.9.114: the line between the header and the two columns
        g.fillRect(10, separatorY, getWidth() - 20, 1);
        // The source column.
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(juce::Rectangle<float>(8.0f, (float)sourcesTop - 4.0f, (float)leftWidth - 8.0f, (float)juce::jmax(40, sourcesBottom - sourcesTop + 8)), 6.0f);
        g.setColour(glow);   // 0.9.85: the two titles orange and larger (the user), "Sound cards" - they are no "in"
        g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        g.drawText("Organ out", 14, sourcesTop - 30, leftWidth - 20, 24, juce::Justification::centredLeft);
        g.drawText("Sound cards", gridLeft, sourcesTop - 30, 300, 24, juce::Justification::centredLeft);
        for (size_t ri = 0; ri < rows.size(); ++ri)
        {
            const auto& row = rows[ri];
            const bool dragged = dragging && dragRow == (int)ri;
            g.setFont(juce::FontOptions(15.0f, row.perspectiveRow ? juce::Font::bold : juce::Font::plain));   // 0.9.89: 13 -> 15 (the user: "mrňavé")
            auto area = row.bounds;
            if (row.perspectiveRow)
            {
                g.setColour(juce::Colours::lightgrey);
                g.drawText(row.folded ? juce::String::charToString(0x25B8) : juce::String::charToString(0x25BE), area.removeFromLeft(18), juce::Justification::centred);
            }
            else area.removeFromLeft(28);
            g.setColour(row.colour.withAlpha(row.silent ? 0.35f : 1.0f));
            g.fillEllipse((float)area.getX(), (float)area.getCentreY() - 7.0f, 14.0f, 14.0f);   // 0.9.89: the dots 10 -> 14 px
            g.fillEllipse((float)area.getRight() - 16.0f, (float)area.getCentreY() - 7.0f, 14.0f, 14.0f);   // 0.9.85: the dot again at the right edge = where the cable is taken
            area.removeFromLeft(18);
            area.removeFromRight(20 + fxWidth + 6 + meterWidth + 6);   // the meter, the fx button and the dot sit at the right
            g.setColour(dragged ? glow : row.silent ? juce::Colours::grey : juce::Colours::white);
            g.drawFittedText(row.label, area, juce::Justification::centredLeft, 1);
            // The row's mini meter (red ONLY while the limiter works on this row's sound; 1 s hold).
            if (ri < rowMeters.size())
            {
                const auto m = rowMeters[ri];
                // 0.9.98 (the user: "udělej to úplně stejně jako velký metr, ale miniaturní ... bez ohraničení"): the
                // large meter's eight segments (4 green, 3 orange, 1 red; the same thresholds), 2 px each = the
                // height of the row's letters, no box and no gaps.
                const juce::Rectangle<int> meter(row.bounds.getRight() - 20 - fxWidth - 6 - meterWidth, row.bounds.getCentreY() - 8, meterWidth, 16);
                for (int s = 0; s < MeterStyle::kSegments; ++s)   // s = 0 the bottom segment; MeterStyle = the large meter's table
                {
                    g.setColour(row.silent ? MeterStyle::segmentColour(s).withAlpha(MeterStyle::kUnlitAlpha) : MeterStyle::segmentPaint(s, m.db, m.clipShown));
                    g.fillRect(meter.getX(), meter.getBottom() - (s + 1) * 2, meter.getWidth(), 2);
                }
            }
        }
        // 0.9.89: the signature also follows the effects (the fx buttons' colour) - see signature().
        // The drop places of a dragged card (0.9.108): the new row under the last one as a dashed strip (orange and
        // thick when it is the target), the place between two cards as the orange landing line.
        if (dragCard >= 0 && draggingCard)
            for (int i = 0; i < (int)dropPlaces.size(); ++i)
            {
                const auto& place = dropPlaces[(size_t)i];
                if (!place.newRow) { if (dropPlace == i) ArrangeFrame::landingLine(g, place.line); continue; }
                if (dropPlace != i) { ArrangeFrame::freeCell(g, place.line.toFloat().reduced(1.0f)); continue; }
                juce::Path p; p.addRoundedRectangle(place.line.toFloat().reduced(1.0f), 6.0f);
                g.setColour(glow);
                ArrangeFrame::strokeDashes(g, p, ArrangeFrame::kDash, ArrangeFrame::kGap, 0.0f, 2.5f);
            }
        // The other cards: buttons per driver under a collapsible caption.
        if (!typeSections.empty())
        {
            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
            g.drawText(otherCardsCaption(), 14, chipsTop - 4, 600, 20, juce::Justification::centredLeft);
            for (const auto& section : typeSections)
            {
                g.setColour(section.collapsed ? glow.withAlpha(0.85f) : glow);
                g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
                g.drawText((section.collapsed ? "+ " : "- ") + shownType(section.type) + (section.collapsed ? " (" + juce::String(section.count) + ")" : juce::String()), section.bounds, juce::Justification::centredLeft);
            }
           #if JUCE_LINUX
            if (!directAlsaNote.isEmpty())   // 0.9.114: the warning under the tick box of the unfolded ALSA section
            {
                g.setColour(glow);
                g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
                g.drawFittedText("Mutually exclusive with PipeWire! A card opened directly is taken whole: it plays nowhere else - not through PipeWire or JACK, "
                    "in this program or any other - until every cable is removed from it.", directAlsaNote, juce::Justification::topLeft, 2);
            }
           #endif
        }
        for (const auto& card : cards)
        {
            if (!card.chip || card.bounds.isEmpty()) continue;
            const bool target = (dragging || draggingCard) && dropCard == &card - cards.data();
            const bool grey = card.unavailable || (card.busy && !((dragging || draggingCard) && freedByDrag(card)));   // a drag that frees its hardware lights it again; a card that is not there stays grey
            const auto base = getLookAndFeel().findColour(juce::TextButton::buttonColourId);
            g.setColour(grey ? base.withAlpha(0.35f) : target ? glow : base);
            g.fillRoundedRectangle(card.bounds.toFloat(), 4.0f);
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.drawRoundedRectangle(card.bounds.toFloat().reduced(0.5f), 4.0f, 1.0f);
            // 0.9.95: a card listed although it cannot be opened now is dimmed (its tooltip says why); it takes a drop.
            g.setColour(grey ? juce::Colours::grey : target ? juce::Colours::black : card.unusableWhy.isNotEmpty() ? juce::Colours::white.withAlpha(0.45f) : juce::Colours::white);
            g.setFont(juce::FontOptions(14.0f * scale()));
            g.drawFittedText(card.name, card.bounds.reduced(8, 2), juce::Justification::centred, 2);
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
            g.setColour(card.busy || !card.open || card.jacks.empty() ? juce::Colours::grey : juce::Colours::white);   // 0.9.98: a card that is not there now (its cables wait) is grey too; 0.9.108: one left without outputs as well
            g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
            g.drawFittedText(card.name, card.bounds.getX() + 10, card.bounds.getY() + 4, card.bounds.getWidth() - 20, 20, juce::Justification::centredLeft, 1);
            // The status line; orange with the note when the device also plays through another driver (0.9.93 - the
            // figures of the status are in the card's tooltip then).
            g.setColour(card.sharedNote.isNotEmpty() ? glow : card.busy ? juce::Colours::grey : juce::Colours::lightgrey);
            g.setFont(juce::FontOptions(juce::jmax(11.5f, 13.0f * scale())));   // 0.9.114: larger, with Button size
            g.drawFittedText(card.sharedNote.isNotEmpty() ? card.sharedNote : shownType(card.type) + "  -  " + card.status, card.bounds.getX() + 10, card.bounds.getY() + 23, card.bounds.getWidth() - 20, 19, juce::Justification::centredLeft, 1);
            // The channel names, vertical, above the jacks (read from the bottom up). Compact: the first ten letters.
            if (card.nameBand > 0)
            {
                g.setFont(juce::FontOptions(compact() ? 10.0f : 12.0f * scale()));
                for (int ch = 0; ch < (int)card.jacks.size() && ch < card.jackNames.size(); ++ch)
                {
                    const auto jack = card.jacks[(size_t)ch];
                    const juce::Rectangle<float> band((float)jack.getX(), (float)jack.getY() - 6.0f - (float)card.nameBand + 8.0f, (float)jack.getWidth(), (float)card.nameBand - 8.0f);
                    g.saveState();
                    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, band.getCentreX(), band.getCentreY()));
                    const auto rotated = juce::Rectangle<float>(band.getHeight(), band.getWidth()).withCentre(band.getCentre());
                    g.setColour(card.busy ? juce::Colours::grey : juce::Colours::lightgrey);
                    const auto& full = card.jackNames[ch];
                    if (compact()) g.drawText(full.length() > 10 ? full.substring(0, 10) + "..." : full, rotated.toNearestInt(), juce::Justification::centredLeft, false);
                    else g.drawFittedText(full, rotated.toNearestInt(), juce::Justification::centredLeft, 1, 0.9f);
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
                g.setFont(juce::FontOptions(juce::jmax(8.5f, 10.5f * scale()), juce::Font::bold));
                g.drawText(juce::String(ch + 1), jack, juce::Justification::centred);
            }
            if (card.jacks.empty())
            {
                g.setColour(juce::Colours::grey);
                g.setFont(juce::FontOptions(juce::jmax(11.0f, 13.0f * scale())));
                g.drawText("outputs are known once the card is open - drop a perspective here to open it", card.bounds.withTrimmedTop(card.bounds.getHeight() - 30).reduced(10, 4), juce::Justification::centredLeft);
            }
        }
        if (showCables.getToggleState() || dragging || selectedCable >= 0) paintCables(g);
        // The cable being dragged (a source, or the lifted cable): from its row to the mouse.
        if (dragging)
        {
            juce::Point<float> from; juce::Colour colour = glow; bool pair = dropPairs;
            if (dragRow >= 0 && dragRow < (int)rows.size()) { const auto& row = rows[(size_t)dragRow]; from = { (float)row.bounds.getRight(), (float)row.bounds.getCentreY() }; colour = row.colour; }
            else if (liftedCable >= 0 && liftedCable < (int)cables.size()) { const auto& row = rows[(size_t)cables[(size_t)liftedCable].row]; from = { (float)row.bounds.getRight(), (float)row.bounds.getCentreY() }; colour = row.colour; }
            juce::Path p;
            p.startNewSubPath(from);
            p.cubicTo(from.x + 60.0f, from.y, dragPoint.x - 60.0f, dragPoint.y, dragPoint.x, dragPoint.y);
            g.setColour(colour.withAlpha(0.9f));
            if (pair)
            {
                g.strokePath(p, juce::PathStrokeType(2.5f), juce::AffineTransform::translation(-1.6f, -1.6f));
                g.strokePath(p, juce::PathStrokeType(2.5f), juce::AffineTransform::translation(1.6f, 1.6f));
            }
            else g.strokePath(p, juce::PathStrokeType(3.0f));
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
            // 0.9.103 (the user: "mělo by to mít stejnou velikost jako karta, aby člověk viděl, co přesně přetahuje"): the
            // card's own picture, dimmed, in the travelling white frame of every arrange mode - it was a small orange
            // label with the name.
            const auto frame = cardFrame();
            if (cardImage.isValid()) { g.setOpacity(0.75f); g.drawImage(cardImage, frame); g.setOpacity(1.0f); }
            ArrangeFrame::takeable(g, frame, ArrangeFrame::phase());
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
        // A driver section's caption: fold / unfold (remembered).
        for (const auto& section : typeSections)
            if (section.bounds.contains(e.getPosition()))
            {
                if (!owner.audioRoutingExpandedSet) { owner.audioRoutingExpandedSet = true; owner.audioRoutingExpanded.clear(); for (const auto& s : typeSections) if (!s.collapsed) owner.audioRoutingExpanded.add(s.type); }
                if (owner.audioRoutingExpanded.contains(section.type)) owner.audioRoutingExpanded.removeString(section.type);
                else owner.audioRoutingExpanded.add(section.type);
                owner.saveSettings();
                rebuild();
                return;
            }
        // The x of the selected cable.
        if (selectedCable >= 0 && selectedCable < (int)cables.size() && cables[(size_t)selectedCable].removeBox.contains(e.getPosition()))
        {
            const auto data = cableData(selectedCable);
            later([this, data] { removeCableData(data); });
            return;
        }
        // A cable: the selected one first where cables overlap, else the one drawn last (on top); a press selects it
        // and may pick it up (mouseDrag decides).
        {
            int hit = -1;
            if (selectedCable >= 0 && selectedCable < (int)cables.size() && cables[(size_t)selectedCable].hitPath.contains(e.getPosition().toFloat())) hit = selectedCable;
            else for (int i = (int)cables.size(); --i >= 0;) if (cables[(size_t)i].hitPath.contains(e.getPosition().toFloat())) { hit = i; break; }
            if (hit >= 0)
            {
                selectedCable = hit; pressedCable = hit; dragPoint = e.getPosition().toFloat();
                repaint();
                return;
            }
        }
        // A grey card (its device is not there): a click asks its driver again, at once.
        for (const auto& card : cards)
            if (card.unavailable && !card.faulted && card.bounds.contains(e.getPosition()))
            {
                const auto type = card.type, name = card.name;
                CrashHandler::note("Audio routing: " + name + " asked again by a click (in the helper process)");
                owner.cardChecker.ask(type, name);   // the answer comes through cardAnswered; the dialog follows its version
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
            if (!cards[(size_t)i].chip && cards[(size_t)i].bounds.withHeight(44).contains(e.getPosition()))
            {
                dragCard = i; draggingCard = false;
                cardGrab = e.getPosition().toFloat() - cards[(size_t)i].bounds.getPosition().toFloat();
                return;
            }
        selectedCable = -1;
        repaint();
    }
    // The card taken: its picture as it stands (before the drag paints anything over it), the frame's dashes kept
    // travelling while the mouse rests.
    juce::Rectangle<float> cardFrame() const
    {
        if (dragCard < 0 || dragCard >= (int)cards.size()) return {};
        return cards[(size_t)dragCard].bounds.toFloat().withPosition(dragPoint - cardGrab);
    }
    void beginCardDrag()
    {
        if (dragCard < 0 || dragCard >= (int)cards.size()) return;
        cardImage = createComponentSnapshot(cards[(size_t)dragCard].bounds, true, 1.0f);
        draggingCard = true;
        cardTicker.tick = [this] { if (draggingCard) repaint(cardFrame().toNearestInt().expanded(4)); };
        cardTicker.startTimerHz(25);
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (dragRow >= 0 || liftedCable >= 0 || pressedCable >= 0)
        {
            if (pressedCable >= 0 && liftedCable < 0 && e.getDistanceFromDragStart() > 6) { liftedCable = pressedCable; pressedCable = -1; }
            if (!dragging && e.getDistanceFromDragStart() > 6 && (dragRow >= 0 || liftedCable >= 0)) dragging = true;
            if (!dragging) return;
            dragPoint = e.getPosition().toFloat();
            dropAdd = dragRow >= 0 && e.mods.isShiftDown();
            dropPairs = liftedCable >= 0 ? cables[(size_t)liftedCable].channels.size() > 1 : !e.mods.isAltDown();
            findDropTarget(e.getPosition());
            repaint();
        }
        else if (dragCard >= 0)
        {
            if (!draggingCard && e.getDistanceFromDragStart() > 6) beginCardDrag();
            if (!draggingCard) return;
            dragPoint = e.getPosition().toFloat();
            findDropTarget(e.getPosition());   // over a pool button: every route of the card moves there (its ASIO version, for one)
            dropPlace = dropCard >= 0 ? -1 : dropPlaceAt(e.getPosition());
            repaint();
        }
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        if (dragging && dropCard >= 0 && dropChannel >= 0)
        {
            const Card card = cards[(size_t)dropCard];   // copies: the actions run after this event (later)
            std::vector<int> channels{ dropChannel };
            if (dropPairs && (card.jacks.empty() || dropChannel + 1 < (int)card.jacks.size())) channels.push_back(dropChannel + 1);
            if (card.jacks.empty()) channels = { 0, 1 };   // a card not open yet: it opens on the first pair
            const bool add = dropAdd;
            if (dragRow >= 0) { const auto routes = rows[(size_t)dragRow].routes; later([this, routes, card, channels, add] { assign(routes, card, channels, add); }); }
            else if (liftedCable >= 0)
            {
                const auto& cable = cables[(size_t)liftedCable];
                const auto routes = rows[(size_t)cable.row].routes; const auto old = cable.channels; const auto label = rows[(size_t)cable.row].label;
                later([this, routes, card, channels, old, label] { moveCable(routes, old, label, card, channels); });
            }
        }
        else if (dragCard >= 0 && draggingCard && dropCard >= 0 && cards[(size_t)dropCard].chip)
        {   // the whole card dropped on a pool button: every route of it moves there
            const auto routes = cards[(size_t)dragCard].routes; const Card card = cards[(size_t)dropCard];
            CrashHandler::note("Audio routing: every route of " + cards[(size_t)dragCard].name + " dragged onto " + card.name);
            later([this, routes, card] { assign(routes, card, { 0, 1 }, false); });
        }
        else if (dragCard >= 0 && draggingCard && dropPlace >= 0 && dropPlace < (int)dropPlaces.size())
        {   // copies (keys): the action runs after this event, the cards may have been rebuilt by then
            const auto place = dropPlaces[(size_t)dropPlace];
            const auto key = cards[(size_t)dragCard].key;
            const auto before = place.before >= 0 ? cards[(size_t)place.before].key : juce::String(), after = place.after >= 0 ? cards[(size_t)place.after].key : juce::String();
            const bool newRow = place.newRow;
            later([this, key, before, after, newRow] { placeCard(key, before, after, newRow); });
        }
        cardTicker.stopTimer(); cardImage = {};
        dragRow = -1; dragCard = -1; liftedCable = -1; pressedCable = -1; dragging = false; draggingCard = false; dropCard = -1; dropChannel = -1; dropPlace = -1;
        repaint();   // a lifted cable let go elsewhere is drawn at its output again
    }
    bool keyPressed(const juce::KeyPress& key) override
    {
        if ((key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey) && selectedCable >= 0) { const auto data = cableData(selectedCable); later([this, data] { removeCableData(data); }); return true; }
        if (key == juce::KeyPress::escapeKey && selectedCable >= 0) { selectedCable = -1; repaint(); return true; }
        return false;
    }
    juce::String getTooltip() override
    {
        const auto p = getMouseXYRelative();
        for (const auto& row : rows)
            if (row.bounds.contains(p)) return row.label + ": " + row.whereText;
        for (const auto& card : cards)
            if (card.bounds.contains(p))
            {
                // An output (its square or its name above it): the whole name of the channel (compact shows ten letters).
                for (int ch = 0; ch < (int)card.jacks.size() && ch < card.jackNames.size(); ++ch)
                {
                    const auto jack = card.jacks[(size_t)ch];
                    if (juce::Rectangle<int>(jack.getX(), jack.getY() - card.nameBand, jack.getWidth(), jack.getHeight() + card.nameBand).contains(p))
                        return "Output " + juce::String(ch + 1) + ": " + card.jackNames[ch];
                }
                return card.name + " (" + shownType(card.type) + "): " + card.status + (card.busy ? "\n" + card.busyReason : juce::String())
                    + (viaPipeWire && card.type == "JACK" ? juce::String("\nPlayed through PipeWire: the outputs are those of the card's profile - the Pro Audio profile (the system's sound settings) shows every channel.") : juce::String())
                    + (card.sharedNote.isNotEmpty() ? "\nThis device also plays through " + card.sharedWith + ": one device, two drivers. It works; for the most polyphony drag one of the two cards by its title onto the other - every cable then plays through one driver." : juce::String());
            }
        for (const auto& section : typeSections)
            if (section.bounds.contains(p)) return section.collapsed ? "Click to show the cards of this driver" : "Click to hide the cards of this driver";
        return {};
    }

private:
    static const char* otherCardsCaption() { return "Other sound cards - drop a perspective onto one to open it"; }
    struct Route { int manual = 0; juce::String perspective; };
    struct Row
    {
        juce::String label, perspective, whereText;
        std::vector<Route> routes;    // one route, or every division's (a folded perspective)
        bool perspectiveRow = false, folded = false, silent = true;
        juce::Colour colour;
        juce::Rectangle<int> bounds;
    };
    using Cell = std::pair<int, int>;   // the order in the row, the row
    struct Card
    {
        juce::String key, type, name, status, busyReason, busyBecauseOf;   // busyBecauseOf = the key of the open card of another driver on the same hardware
        juce::String sharedNote, sharedWith;                              // 0.9.93: "Also open through ASIO ..." - the orange line of a card that shares its device with another driver (sharedWith = that card's name, for the tooltip)
        bool busy = false, open = false, chip = false;   // chip = a button among the unused cards
        bool unavailable = false;                        // 0.9.105: its driver is listed, its device is not there (asked in vain) - grey, takes no cable
        bool faulted = false;                            // 0.9.108: its driver failed in this session - out of use until the next start, never asked
        juce::String unusableWhy;                        // 0.9.95 (Linux): listed although it cannot be opened now ("list unusable devices too") - why
        double rate = 0.0; int buffer = 0, latency = 0;
        std::vector<juce::Rectangle<int>> jacks;   // the outputs on the bottom edge
        juce::StringArray jackNames;               // the driver's channel names, drawn vertically above the jacks
        int nameBand = 0;                          // the height of that band (0 when the names are numbers only)
        std::vector<Route> routes;                 // the routes playing on it
        juce::AudioDeviceManager* manager = nullptr;
        juce::Rectangle<int> bounds;
        Cell cell{ -1, -1 };                       // where it stands now: its order in the line, the line (set by layout())
        Cell place{ -1, -1 };                      // its remembered place: the order in its row, the row (the machine's memory)
        int width = 0;
    };
    struct Cable
    {
        int row = -1, card = -1, ring = 0;   // ring = this cable's order among the cables into the same jacks (concentric clamps)
        std::vector<int> channels;           // one or two (a pair)
        int gutterLane = 0, corridorLane = 0;
        juce::Path path, hitPath;
        juce::Rectangle<int> removeBox;
    };
    struct CardControls
    {
        juce::ComboBox rate, buffer;
        juce::TextButton panel{ "Driver panel" }, reopen{ "Reopen" }, test{ "Test" };
    };
    struct TypeSection { juce::String type; bool collapsed = false; int count = 0; juce::Rectangle<int> bounds; };

    static constexpr int leftWidth = 340, rowHeight = 32, cardWidth = 372, cardHeight = 118, cellGap = 24, laneGap = 14;   // laneGap 8 -> 14 (0.9.85, the user: "namačkané na sobě"); 0.9.89: rows 24 -> 32, the column wider for the meter and fx
    static constexpr int fxWidth = 34, meterWidth = 12;   // 0.9.89: the fx button and the mini meter of a row (0.9.91: one letter-sized box)

    // 0.9.89: the console's audio meter in the header (one state: MainComponent::outputMeterState).
    struct MasterMeter final : public juce::Component, public juce::SettableTooltipClient
    {
        explicit MasterMeter(MainComponent& o) : owner(o) {}
        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colours::black); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 3.0f);
            g.setColour(juce::Colours::darkgrey); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 3.0f, 1.0f);
            float db; bool now, latched;
            owner.outputMeterState(db, now, latched);
            MainComponent::paintOutputMeter(g, getLocalBounds(), db, now, latched);
        }
        void mouseDown(const juce::MouseEvent&) override { owner.clearOutputClipLatch(); repaint(); }
        MainComponent& owner;
    };

    // 0.9.93: "compact" (size 3, for small screens): small squares for the outputs, ten letters of a channel's name,
    // a lower card - the grid takes little height.
    bool compact() const noexcept { return owner.audioRoutingButtonSize == 3; }
    float scale() const noexcept { return compact() ? 0.7f : owner.audioRoutingButtonSize == 0 ? 0.85f : owner.audioRoutingButtonSize == 2 ? 1.25f : 1.0f; }
    int jackWidth() const noexcept { return compact() ? 18 : juce::roundToInt(30.0f * scale()); }
    int jackHeight() const noexcept { return compact() ? 16 : juce::roundToInt(24.0f * scale()); }
    int cardBase() const noexcept { return compact() ? 92 : cardHeight; }   // a card's height without the band of channel names
    int chipHeight() const noexcept { return juce::roundToInt(36.0f * scale()); }
    int cellWidth() const noexcept { return cardWidth + cellGap; }

    // Every action that changes the routes runs after the mouse event has returned (never inside it: the rebuild
    // deletes controls and the detach rebuilds routes - the 0.9.83 vanishing without a crash file).
    void later(std::function<void()> action)
    {
        juce::Component::SafePointer<AudioRoutingContent> safe(this);
        juce::MessageManager::callAsync([safe, action = std::move(action)] { if (safe != nullptr) action(); });
    }
    // The routes the current drag carries (a source row, a lifted cable or a whole card).
    std::vector<Route> draggedRoutes() const
    {
        if (dragRow >= 0 && dragRow < (int)rows.size()) return rows[(size_t)dragRow].routes;
        if (liftedCable >= 0 && liftedCable < (int)cables.size()) return rows[(size_t)cables[(size_t)liftedCable].row].routes;
        if (dragCard >= 0 && dragCard < (int)cards.size()) return cards[(size_t)dragCard].routes;
        return {};
    }
    // A busy card (its hardware open through another driver) takes a drop when the drop moves EVERY route off the
    // card that keeps it busy - the WASAPI card's last routes may go to its ASIO version (the user's question).
    bool freedByDrag(const Card& card) const
    {
        if (!card.busy) return true;
        const int blocker = cardIndexOf(card.busyBecauseOf);
        if (blocker < 0) return false;
        const auto dragged = draggedRoutes();
        if (dragged.empty()) return false;
        for (const auto& route : cards[(size_t)blocker].routes)
            if (std::none_of(dragged.begin(), dragged.end(), [&](const Route& d) { return d.manual == route.manual && d.perspective == route.perspective; })) return false;
        return true;
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
    static juce::StringArray hardwareWords(const juce::String& name) { return MainComponent::audioHardwareWords(name); }   // 0.9.93: one rule, MainComponent's (the conflict note of the top bar uses it too)
    // The hardware a card stands for: its name's words; the aggregate by the outputs its routes use.
    juce::StringArray cardHardwareWords(const Card& card) const
    {
        if (!card.type.containsIgnoreCase("Aggregated")) return hardwareWords(card.name);
        juce::StringArray words;
        for (const auto& route : card.routes)
            for (const int channel : channelsOf(route))
                if (channel >= 0 && channel < card.jackNames.size())
                    for (const auto& word : hardwareWords(card.jackNames[channel])) words.addIfNotAlreadyThere(word);
        return words;
    }
    static juce::String driverFamily(const juce::String& type)
    {
        if (type.containsIgnoreCase("ASIO")) return "ASIO";
        if (type.containsIgnoreCase("WASAPI") || type.containsIgnoreCase("Windows Audio")) return "Windows Audio";
        return type;
    }
    // The driver sections' order: ASIO, Aggregated WASAPI, the rest as listed.
    static int typeRank(const juce::String& type)
    {
        if (type.containsIgnoreCase("ASIO")) return 0;
        if (type == "JACK") return 0;   // 0.9.96: Linux - the cards as PipeWire has them come first (and unfolded)
        if (type.containsIgnoreCase("Aggregated")) return 1;
        return 2;
    }
    // What a driver is called on the screen (0.9.96): the JACK entries of a PipeWire system ARE PipeWire's cards.
    juce::String shownType(const juce::String& type) const { return viaPipeWire && type == "JACK" ? juce::String("PipeWire") : type; }

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
        for (int i = 0; i < (int)cables.size(); ++i)
        {
            const auto& cable = cables[(size_t)i];
            if (i == liftedCable || cable.card != &card - cards.data()) continue;
            for (const int c : cable.channels) if (c == channel) colours.push_back(rows[(size_t)cable.row].colour);
        }
        return colours;
    }
    // The drop place of a dragged card under the mouse: the line of cards (or the new row) nearest in height,
    // then the nearest place of that line.
    int dropPlaceAt(juce::Point<int> p) const
    {
        auto above = [p](const juce::Rectangle<int>& r) { return p.y < r.getY() ? r.getY() - p.y : p.y >= r.getBottom() ? p.y - r.getBottom() + 1 : 0; };
        int nearest = INT_MAX;
        for (const auto& place : dropPlaces) nearest = juce::jmin(nearest, above(place.line));
        int best = -1, bestDistance = INT_MAX;
        for (int i = 0; i < (int)dropPlaces.size(); ++i)
        {
            const auto& place = dropPlaces[(size_t)i];
            if (above(place.line) != nearest) continue;
            const int distance = place.newRow ? 0 : std::abs(p.x - place.line.getCentreX());
            if (distance < bestDistance) { bestDistance = distance; best = i; }
        }
        return best;
    }

    // ---- the model -> rows, cards, cables ----
    void rebuild()
    {
       #if JUCE_LINUX
        viaPipeWire = pipeWireCarriesTheCards();   // asked here alone (it opens a JACK client), never from paint
       #endif
        std::vector<Card> fresh;
        for (auto* type : owner.deviceManager.getAvailableDeviceTypes())
        {
            if (type == nullptr) continue;
            for (const auto& name : type->getDeviceNames(false))
            {
                Card card;
                card.type = type->getTypeName(); card.name = name; card.key = card.type + "|" + name;
                card.status = "not open";
               #if JUCE_LINUX
                if (card.type == "ALSA")
                    if (const auto found = unusableAlsaDeviceReasons().find(name); found != unusableAlsaDeviceReasons().end())
                    {
                        card.unusableWhy = found->second;
                        card.status = "cannot be opened now (" + card.unusableWhy + ") - a cable dropped here waits until the card is free";
                    }
               #endif
                if (const auto gone = owner.unavailableCards.find(card.key); gone != owner.unavailableCards.end())
                {
                    card.unavailable = true;
                    card.unusableWhy = gone->second.reason;
                    card.status = gone->second.faulted ? "out of use: " + card.unusableWhy
                        : "not there now: " + card.unusableWhy + " It takes no cable; the program asks it again when Windows reports a device, a click on it asks at once.";
                    card.faulted = gone->second.faulted;
                }
                fresh.push_back(card);
            }
        }
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
                if (device == nullptr) { if (!card.open && !card.unavailable) card.status = "not available now (switched off, unplugged or taken) - tried again every 2 s"; }   // (a card known to be away keeps its reason)
                else if (!card.open)
                {
                    card.open = true; card.manager = manager;
                    card.rate = device->getCurrentSampleRate(); card.buffer = device->getCurrentBufferSizeSamples();
                    card.latency = device->getOutputLatencyInSamples();
                    card.jackNames = device->getOutputChannelNames();
                    // 0.9.108: a Windows Audio output that lists more outputs than it opens with (Realtek's on-board
                    // output lists 64 and opens with its first pair alone - MainComponent::sharedHostFor) shows the open ones.
                    if (card.type.startsWithIgnoreCase("Windows Audio"))
                        if (const int open = device->getActiveOutputChannels().getHighestBit() + 1; open > 0 && open < card.jackNames.size())
                            card.jackNames.removeRange(open, card.jackNames.size() - open);
                    const int outputs = card.jackNames.size();
                    card.jacks.assign((size_t)juce::jmax(0, outputs), {});
                    for (const auto& jackName : card.jackNames)   // a band only for real names ("ADAT (7+8) ..."), not "3" or "Output channel 3"
                        if (jackName.trim().retainCharacters("0123456789").length() != jackName.trim().length() && !jackName.trim().startsWithIgnoreCase("output channel ")) { card.nameBand = compact() ? 74 : juce::roundToInt(180.0f * scale()); break; }
                    card.status = juce::String(outputs) + " outputs, " + juce::String(juce::roundToInt(card.rate)) + " Hz, buffer " + juce::String(card.buffer)
                        + " (" + juce::String(1000.0 * card.buffer / juce::jmax(1.0, card.rate), 1) + " ms), latency " + juce::String(card.latency) + " samples";
                    // 0.9.108 (the user, Linux: a card with cables switched off - "ztratily se porty, kabel zůstal",
                    // but its name stayed white): no output left = not there now.
                    if (outputs == 0) card.status = "no outputs now (switched off or unplugged) - its cables wait";
                }
            }
        for (auto& card : fresh)
        {
            if (card.open) continue;
            const auto words = hardwareWords(card.name);
            for (const auto& other : fresh)
            {
                if (!other.open || other.type == card.type) continue;
               #if JUCE_LINUX
                // 0.9.114: a card's direct ALSA device and its PipeWire entry are ONE card (the word rule below misses
                // "HD-Audio Generic, ALC1220 Analog; Direct hardware device ..." against "ALC1220 Analog"): while one of
                // the two is open here the other is busy - its cables are moved over by dragging, never opened beside.
                {
                    const auto& alsa = card.type == "ALSA" ? card : other;
                    const auto& jack = card.type == "ALSA" ? other : card;
                    if (alsa.type == "ALSA" && jack.type == "JACK" && !FilteredAlsaDeviceType::isSoundServerDevice(alsa.name) && jack.name.isNotEmpty()
                        && (alsa.name.containsIgnoreCase(jack.name) || jack.name.containsIgnoreCase(alsa.name.upToFirstOccurrenceOf(",", false, false).trim())))
                    {
                        card.busy = true; card.busyBecauseOf = other.key;
                        card.busyReason = card.type == "ALSA"
                            ? "busy: this card plays through PipeWire (" + other.name + ") - remove its PipeWire cables first, or drag that card (or its last cable) here to move everything over"
                            : "busy: this card is open directly through ALSA (" + other.name + ") - remove its cables there first, or drag that card (or its last cable) here to move everything over";
                        break;
                    }
                }
               #endif
                // 0.9.93 (the user): on Windows a card plays through ASIO and Windows Audio at once - only an
                // exclusive-mode driver keeps the hardware to itself. Elsewhere (ALSA) the card is busy, as before.
               #if JUCE_WINDOWS
                if (!card.type.containsIgnoreCase("Exclusive") && !other.type.containsIgnoreCase("Exclusive")) continue;
               #endif
                for (const auto& word : cardHardwareWords(other))
                    if (words.contains(word)) { card.busy = true; card.busyBecauseOf = other.key; card.busyReason = "busy: " + other.name + " is open through " + other.type + " - drag that card (or its last cable) here to move everything over"; break; }
                if (card.busy) break;
            }
            if (card.busy) card.status = card.busyReason;
        }
        // 0.9.93: two OPEN cards of different drivers on one device: both say so (an orange line in the card).
        for (auto& card : fresh)
        {
            if (!card.open) continue;
            const auto words = cardHardwareWords(card);
            for (const auto& other : fresh)
            {
                if (&other == &card || !other.open || driverFamily(other.type) == driverFamily(card.type)) continue;
                bool same = false;
                for (const auto& word : cardHardwareWords(other)) if (words.contains(word)) { same = true; break; }
                if (same) { card.sharedNote = "Also open through " + other.type + ": two drivers, less polyphony"; card.sharedWith = other.name; break; }   // short: it must fit a 372 px card (the first build cut it)
            }
        }
        for (auto& card : fresh) card.chip = card.routes.empty() && !card.open;
        // Full cards first (the remembered cells decide the places, not this order), then the buttons by driver rank.
        std::stable_sort(fresh.begin(), fresh.end(), [](const Card& a, const Card& b)
        {
            if (a.chip != b.chip) return !a.chip;
            if (a.chip) return typeRank(a.type) < typeRank(b.type);
            return false;
        });
        cards = std::move(fresh);
        // The remembered cells.
        for (auto& card : cards)
        {
            if (card.chip) continue;
            for (const auto& line : owner.audioRoutingCardCells)
                if (line.fromFirstOccurrenceOf("|", false, false) == card.key)
                {
                    const auto place = line.upToFirstOccurrenceOf("|", false, false);
                    card.place = { place.upToFirstOccurrenceOf(",", false, false).getIntValue(), place.fromFirstOccurrenceOf(",", false, false).getIntValue() };
                }
        }

        rows.clear();
        std::vector<juce::String> perspectives;
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
                if (std::find(perspectives.begin(), perspectives.end(), perspective) == perspectives.end()) perspectives.push_back(perspective);
        owner.engine.sortPerspectives(perspectives);   // 0.9.84: the order of the Perspective patterns (the user), as the tabs and the voicing
        // 0.9.90 (the user: "the primary colours first - yellow, blue, red - then the mixtures; the auxiliary takes what is
        // left"): a fixed palette in that order, the ordinary perspectives first in their order, the auxiliary after them.
        static const juce::Colour palette[8]{ juce::Colour(0xffffd400), juce::Colour(0xff3d8bff), juce::Colour(0xffff3b30), juce::Colour(0xff3ddc5a),
                                              juce::Colour(0xffff4fd8), juce::Colour(0xff2ee6e6), juce::Colour(0xffff8c1a), juce::Colour(0xffffffff) };
        std::vector<size_t> colourIndex(perspectives.size(), 0);
        {
            size_t next = 0;
            for (size_t pi = 0; pi < perspectives.size(); ++pi) if (!MainComponent::isAuxiliaryPerspective(perspectives[pi])) colourIndex[pi] = next++;
            for (size_t pi = 0; pi < perspectives.size(); ++pi) if (MainComponent::isAuxiliaryPerspective(perspectives[pi])) colourIndex[pi] = next++;
        }
        for (size_t pi = 0; pi < perspectives.size(); ++pi)
        {
            const auto& perspective = perspectives[pi];
            const juce::Colour base = palette[colourIndex[pi] % 8];
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
            head.colour = base;
            head.silent = uniform ? firstKey.isEmpty() : false;
            head.whereText = uniform ? whereOf(routes.front()) : juce::String("the divisions differ - see them below");
            rows.push_back(head);
            if (!head.folded)
                for (size_t di = 0; di < routes.size(); ++di)
                {
                    const auto& route = routes[di];
                    Row row;
                    row.perspective = perspective; row.routes = { route };
                    const auto& alias = owner.manualConfigs[(size_t)route.manual].alias;   // 0.9.86: the division's alias (0.9.80) belongs here too (the user)
                    row.label = divisionName(route.manual) + (alias.isNotEmpty() ? " " + alias : juce::String()) + "  " + perspective;
                    row.colour = base.darker(0.22f * (float)std::min<size_t>(3, di));   // a division's row: its perspective's colour, a step darker per division
                    row.silent = cardKeyOf(route).isEmpty();
                    row.whereText = whereOf(route);
                    rows.push_back(row);
                }
        }
        // 0.9.89: the fx button of every row and its meter.
        fxButtons.clear();
        rowMeters.assign(rows.size(), {});
        for (size_t ri = 0; ri < rows.size(); ++ri)
        {
            const auto& row = rows[ri];
            auto* b = fxButtons.add(new FxButton());
            const bool active = row.perspectiveRow ? owner.perspectiveEffectsActive(row.perspective)
                : (!row.routes.empty() && owner.routeEffectsActive(row.routes.front().manual, row.routes.front().perspective));
            if (active)
            {
                b->setColour(juce::TextButton::buttonColourId, onColour());
                b->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            }
            b->setTooltip((row.perspectiveRow ? "The effects of the perspective " + row.perspective + " - every linked route of it follows"
                : "The effects of this route: volume, channel separation, equalizer, IR reverb - LINKED to the perspective, or its own")
                + juce::String(active ? "\n(orange: something is set here)" : "\n(grey: everything default)") + "\nRight-click: reset everything to the default.");
            b->getProperties().set("manualChapter", "Effects");
            b->getProperties().set("testName", "fx " + row.label);   // ORGAN_TEST_MOUSE finds it ("fx Front (3 divisions)", "fx I.  Front")
            const juce::String perspective = row.perspective;
            const int manual = row.perspectiveRow || row.routes.empty() ? -1 : row.routes.front().manual;
            b->onClick = [this, perspective, manual] { juce::Component::SafePointer<MainComponent> target(&owner); juce::MessageManager::callAsync([target, perspective, manual] { if (target != nullptr) target->showEffectsDialog(perspective, manual); }); };
            // 0.9.90: the right click = a menu with the reset (the user: "all fx must be switchable off").
            const juce::String label = row.label;
            b->onRightClick = [this, perspective, manual, label]
            {
                juce::PopupMenu menu;
                menu.addItem(1, "Reset the effects of " + label + " to the default" + (manual >= 0 ? " (every module LINKED to the perspective again)" : " (every route of it linked again)"));
                menu.addItem(2, "Open the effects...");
                juce::Component::SafePointer<MainComponent> target(&owner);
                menu.showMenuAsync(juce::PopupMenu::Options(), [target, perspective, manual](int result)
                {
                    if (target == nullptr || result == 0) return;
                    if (result == 1) target->resetEffects(perspective, manual);
                    else juce::MessageManager::callAsync([target, perspective, manual] { if (target != nullptr) target->showEffectsDialog(perspective, manual); });
                });
            };
            addAndMakeVisible(b);
        }
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
                // 0.9.114 (the user: Realtek ASIO's panel sets a rate its own list lacks - "Hz neukáže nic"): the rate
                // the card runs at is listed whatever the driver's list says, as the buffer is.
                auto rates = device->getAvailableSampleRates();
                bool rateListed = false;
                for (const double rate : rates) if (juce::roundToInt(rate) == juce::roundToInt(card.rate)) rateListed = true;
                if (!rateListed && card.rate > 0.0) { rates.add(card.rate); rates.sort(); }
                for (const double rate : rates) c->rate.addItem(juce::String(juce::roundToInt(rate)) + " Hz", juce::roundToInt(rate));
                c->rate.setSelectedId(juce::roundToInt(card.rate), juce::dontSendNotification);
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
    // The cables as a list (which row to which card and outputs) - the geometry comes in layout().
    void listCables()
    {
        cables.clear();
        for (int r = 0; r < (int)rows.size(); ++r)
        {
            const auto& row = rows[(size_t)r];
            if ((row.perspectiveRow && !row.folded) || row.routes.empty()) continue;
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
    }
    void layout()
    {
        const int width = juce::jmax(760, getWidth());
        // 0.9.114: the help text is as high as its words need at this width and size; the header grows with it
        // (everything below starts under the header's line). Too narrow, or more than seven lines or so: the "?".
        const float helpFont = juce::jmax(13.0f, 15.0f * scale());
        help.setFont(juce::FontOptions(helpFont));
        const int helpWidth = width - 644;
        const int helpHeight = helpWidth >= 380 ? helpHeightFor(helpWidth, helpFont) : 9999;
        const bool helpShown = helpHeight <= 132;
        help.setVisible(helpShown);
        helpButton.setVisible(!helpShown);
        help.setBounds(10, 6, juce::jmax(10, helpWidth), helpShown ? helpHeight : 10);
        helpButton.setBounds(10, 6, 30, 30);
        // 0.9.90: the right block of the header is ONE aligned strip - Master volume, the meter, Measure Polyphony -
        // with the switches on the line under it (the user: "the top right corner looks untidy").
        // 0.9.91: a block of two rows as high as the meter (72 px): Measure Polyphony and the meter stand over both
        // rows at the right; left of them row 1 = Master volume + its slider, row 2 = Show cables + Button size, the
        // two rows sharing their left and right edges (the size box ends where the slider's value box ends).
        const int stripRight = width - 10, blockTop = 6, blockHeight = 72, rowHeight2 = 28;
        measureButton.setBounds(stripRight - 150, blockTop, 150, blockHeight);
        masterMeter.setBounds(measureButton.getX() - 10 - 46, blockTop, 46, blockHeight);
        const int rowsRight = masterMeter.getX() - 12, rowsLeft = rowsRight - 390;
        const int row1 = blockTop + 4, row2 = blockTop + blockHeight - 4 - rowHeight2;
        masterLabel.setBounds(rowsLeft, row1, 112, rowHeight2);
        masterSlider.setBounds(rowsLeft + 116, row1, rowsRight - rowsLeft - 116, rowHeight2);
        showCables.setBounds(rowsLeft, row2, 120, rowHeight2);
        sizeBox.setBounds(rowsRight - 110, row2, 110, rowHeight2);
        sizeLabel.setBounds(sizeBox.getX() - 96, row2, 92, rowHeight2);
        listCables();
        gutterWidth = juce::jmax(44, 16 + laneGap * (int)cables.size());   // (the gutter widens with the cables it carries)
        gridLeft = leftWidth + 20 + gutterWidth;
        // 0.9.94: the routing file's buttons on the line of the titles, ending at the header block's right edge.
        // They span the block above them: the left edge of its rows, the right edge of Measure Polyphony.
        // 0.9.108 (the user: "nápis Sound cards koliduje s řadou buttonů" - the title stands where the cards begin,
        // which moves right with every cable, while the buttons kept their place): the buttons begin right of the
        // title when it reaches under them, and take a line of their own (the titles a line lower) when what is
        // left is too narrow for their texts.
        const int titleRight = gridLeft + 16 + juce::roundToInt(juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)), "Sound cards"));
        const bool buttonsOnOwnLine = stripRight - juce::jmax(rowsLeft, titleRight) < 470;
        // 0.9.114 (the user: "Sekce Organ Out a Sound Cards by mohla být trochu oddělena od hlavičky"): a thin line
        // under the header - whichever of its parts reaches lowest -, the two titles a little under it.
        const int headerBottom = juce::jmax(blockTop + blockHeight, helpShown ? 6 + helpHeight : 0, buttonsOnOwnLine ? blockTop + blockHeight + 4 + 24 : 0);
        separatorY = headerBottom + 8;
        sourcesTop = separatorY + 10 + 30;
        const int buttonsY = buttonsOnOwnLine ? blockTop + blockHeight + 4 : sourcesTop - 30;
        const int buttonsLeft = buttonsOnOwnLine ? juce::jmax(12, juce::jmin(rowsLeft, stripRight - 606)) : juce::jmax(rowsLeft, titleRight);
        const int fileButton = juce::jlimit(110, 150, (stripRight - buttonsLeft - 12) * 3 / 10);
        saveRoutingButton.setBounds(buttonsLeft, buttonsY, fileButton, 24);
        loadRoutingButton.setBounds(saveRoutingButton.getRight() + 6, buttonsY, fileButton, 24);
        defaultRoutingButton.setBounds(loadRoutingButton.getRight() + 6, buttonsY, stripRight - loadRoutingButton.getRight() - 6, 24);
        int y = sourcesTop;
        for (size_t ri = 0; ri < rows.size(); ++ri)
        {
            auto& row = rows[ri];
            row.bounds = { 12, y, leftWidth - 16, rowHeight };
            if ((int)ri < fxButtons.size()) fxButtons[(int)ri]->setBounds(row.bounds.getRight() - 20 - fxWidth, y + 4, fxWidth, rowHeight - 8);
            y += rowHeight;
        }
        sourcesBottom = y;
        // THE ROWS OF CARDS (0.9.108, the user: "karta se často přesune jinam, pokud přesahuje políčko gridu. Grid by
        // měl být fluidnější, nikoli přesně na políčka stejné velikosti" - the cells had one width, a wide card took
        // two or three of them, and one let go where its cells met a neighbour's was thrown to the first free place):
        // no columns. The cards of a row stand side by side, each as wide as its outputs need, in their remembered
        // order; a row too long for the window goes on in the line below; a card without a remembered place joins the
        // last row. A dragged card lands BETWEEN two cards (the others move aside) or in a new row.
        const int right = width - 10;
        const int jw0 = jackWidth(), jh = jackHeight();
        std::vector<int> order;
        int lastRow = 0;
        for (int i = 0; i < (int)cards.size(); ++i)
        {
            auto& card = cards[(size_t)i];
            if (card.chip) continue;
            const int n = (int)card.jacks.size();
            const int jw = n > 0 ? juce::jlimit(juce::jmin(18, jw0), jw0, (juce::jmax(cardWidth, right - gridLeft) - 20) / juce::jmax(1, n) - 2) : jw0;
            card.width = juce::jmax(cardWidth, 20 + n * (jw + 2));
            order.push_back(i);
            lastRow = juce::jmax(lastRow, card.place.second);
        }
        auto rowOf = [this, lastRow](int i) { return cards[(size_t)i].place.second >= 0 ? cards[(size_t)i].place.second : lastRow; };
        auto orderOf = [this](int i) { return cards[(size_t)i].place.second >= 0 ? cards[(size_t)i].place.first : 100000; };
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return rowOf(a) != rowOf(b) ? rowOf(a) < rowOf(b) : orderOf(a) < orderOf(b); });
        std::vector<int> cardX(cards.size(), gridLeft);
        {
            int line = -1, x = gridLeft, inLine = 0, rowBefore = 0;
            for (const int i : order)
            {
                auto& card = cards[(size_t)i];
                if (line < 0 || rowOf(i) != rowBefore || (inLine > 0 && x + card.width > right)) { ++line; x = gridLeft; inLine = 0; }
                rowBefore = rowOf(i);
                card.cell = { inLine++, line };
                cardX[(size_t)i] = x;
                x += card.width + cellGap;
            }
        }
        cardOrder = order;
        int maxRow = -1;
        for (const auto& card : cards) if (!card.chip) maxRow = juce::jmax(maxRow, card.cell.second);
        // The rows' heights (the tallest card) and the corridors under them (the cables that end in that row).
        std::vector<int> rowHeights((size_t)maxRow + 1, cardBase()), corridorCount((size_t)maxRow + 1, 0);
        for (const auto& card : cards) if (!card.chip) rowHeights[(size_t)card.cell.second] = juce::jmax(rowHeights[(size_t)card.cell.second], cardBase() + card.nameBand);
        for (const auto& cable : cables) ++corridorCount[(size_t)cards[(size_t)cable.card].cell.second];
        rowTop.assign((size_t)maxRow + 1, 0); rowBottom.assign((size_t)maxRow + 1, 0);
        int gy = sourcesTop - 2;
        for (int r = 0; r <= maxRow; ++r)
        {
            rowTop[(size_t)r] = gy; rowBottom[(size_t)r] = gy + rowHeights[(size_t)r];
            gy = rowBottom[(size_t)r] + 20 + laneGap * corridorCount[(size_t)r] + 12;   // the corridor
        }
        const int gridBottom = maxRow >= 0 ? gy : sourcesTop;
        for (int i = 0; i < (int)cards.size(); ++i)
        {
            auto& card = cards[(size_t)i];
            if (card.chip) continue;
            card.bounds = { cardX[(size_t)i], rowTop[(size_t)card.cell.second], card.width, cardBase() + card.nameBand };
            const int n = (int)card.jacks.size();
            const int jw = n > 0 ? juce::jlimit(juce::jmin(18, jw0), jw0, (card.width - 20) / n - 2) : jw0;
            for (int ch = 0; ch < n; ++ch)
                card.jacks[(size_t)ch] = { card.bounds.getX() + 10 + ch * (jw + 2), card.bounds.getBottom() - jh - 4, jw, jh };
            if (auto* c = controls[i])
            {
                const int cx = card.bounds.getX() + 10, cy = card.bounds.getY() + 44;
                c->rate.setBounds(cx, cy, 90, 22); c->buffer.setBounds(cx + 96, cy, 70, 22);
                c->panel.setBounds(cx + 172, cy, 84, 22); c->reopen.setBounds(cx + 260, cy, 56, 22); c->test.setBounds(cx + 320, cy, 42, 22);
            }
        }
        // The drop places of a dragged card: before each card of a line, after the line's last card, and a new row
        // under the last line.
        dropPlaces.clear();
        for (int line = 0; line <= maxRow; ++line)
        {
            std::vector<int> inLine;
            for (const int i : cardOrder) if (cards[(size_t)i].cell.second == line) inLine.push_back(i);
            const int top = rowTop[(size_t)line], h = rowHeights[(size_t)line];
            for (size_t k = 0; k < inLine.size(); ++k)
            {
                const auto& b = cards[(size_t)inLine[k]].bounds;
                dropPlaces.push_back({ { k == 0 ? gridLeft - 5 : b.getX() - cellGap / 2 - 1, top, 3, h }, inLine[k], -1, false });
                if (k + 1 == inLine.size()) dropPlaces.push_back({ { b.getRight() + cellGap / 2 - 1, top, 3, h }, -1, inLine[k], false });
            }
        }
        dropPlaces.push_back({ { gridLeft, maxRow >= 0 ? gridBottom : sourcesTop - 2, juce::jmax(cardWidth, right - gridLeft), cardBase() }, -1, -1, true });
        fullCardsBottom = (maxRow >= 0 ? gridBottom : sourcesTop - 2) + cardBase() + 10;   // the spare row stays free below the grid
        // The other cards as buttons, a collapsible section per driver (ASIO first ...), below the grid and the sources.
        chipsTop = juce::jmax(fullCardsBottom + 10, sourcesBottom + 24);
#if JUCE_LINUX
        // 0.9.95 (#242): the switch decides which cards THIS list shows - it stands right after the list's caption.
        // In the header it lay over the help text (seen on no picture: Windows has no such switch).
        unusableButton.setBounds(14 + juce::roundToInt(juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)), otherCardsCaption())) + 24,
            chipsTop - 6, 230, 24);
        unusableButton.setVisible(!viaPipeWire);   // 0.9.96: PipeWire carries the cards - nothing is "unusable" there
#endif
        int cy = chipsTop + 24, cx = 12;
       #if JUCE_LINUX
        directAlsaButton.setVisible(false);
        directAlsaNote = {};
       #endif
        juce::String lastType;
        typeSections.clear();
        bool collapsed = false;
        const int ch = chipHeight();
        juce::StringArray chipTypes;
        for (const auto& card : cards) if (card.chip) chipTypes.addIfNotAlreadyThere(card.type);
        juce::StringArray expanded = owner.audioRoutingExpanded;
        if (!owner.audioRoutingExpandedSet)
        {
            expanded.clear();
            for (const auto& type : chipTypes) if (typeRank(type) == 0) expanded.add(type);
            if (expanded.isEmpty()) for (const auto& type : chipTypes) if (typeRank(type) == 1) expanded.add(type);
            if (expanded.isEmpty() && !chipTypes.isEmpty()) expanded.add(chipTypes[0]);
        }
        for (auto& card : cards)
        {
            if (!card.chip) continue;
            if (card.type != lastType)
            {
                if (lastType.isNotEmpty()) cy += collapsed ? 8 : ch + 8;
                collapsed = !expanded.contains(card.type);
                int count = 0; for (const auto& other : cards) if (other.chip && other.type == card.type) ++count;
                typeSections.push_back({ card.type, collapsed, count, { 12, cy, 400, 22 } });
                lastType = card.type; cx = 12; cy += 26;
               #if JUCE_LINUX
                // 0.9.114: the unfolded ALSA section (PipeWire carries the cards) begins with the tick box of the
                // direct devices and its warning; the folded section shows neither.
                if (!collapsed && viaPipeWire && card.type == "ALSA")
                {
                    directAlsaButton.setBounds(12, cy, 470, 24);
                    directAlsaButton.setVisible(true);
                    directAlsaNote = { 16, cy + 26, juce::jmax(200, juce::jmin(right - 28, 980)), 36 };
                    cy += 26 + 36 + 6;
                }
               #endif
            }
            if (collapsed) { card.bounds = {}; continue; }
            const int w = juce::jlimit(juce::roundToInt(120.0f * scale()), juce::roundToInt(320.0f * scale()),
                24 + juce::roundToInt(juce::GlyphArrangement::getStringWidth(juce::Font(juce::FontOptions(14.0f * scale())), card.name)));
            if (cx + w > right) { cx = 12; cy += ch + 6; }
            card.bounds = { cx, cy, w, ch };
            cx += w + 6;
        }
        const int bottom = lastType.isNotEmpty() ? cy + (collapsed ? 4 : ch + 8) : chipsTop;
        buildCablePaths();
        const int height = juce::jmax(sourcesBottom + 20, bottom + 10);
        if (height != getHeight()) setSize(getWidth(), height);
    }
    // The cables' geometry in the aisles: the gutter (left of the grid, a lane per cable, the inner lanes for the
    // nearest row and the leftmost outputs so the runs do not cross), the corridor under the target's row (a lane per
    // cable, the upper lanes for the leftmost outputs), up into the output.
    void buildCablePaths()
    {
        std::vector<int> order((size_t)cables.size());
        for (size_t i = 0; i < order.size(); ++i) order[i] = (int)i;
        auto jackX = [this](const Cable& cable)
        {
            const auto& card = cards[(size_t)cable.card];
            const int first = cable.channels.front();
            return first < (int)card.jacks.size() ? card.jacks[(size_t)first].getCentreX() : card.bounds.getX() + 10;
        };
        std::sort(order.begin(), order.end(), [&](int a, int b)
        {
            const int ra = cards[(size_t)cables[(size_t)a].card].cell.second, rb = cards[(size_t)cables[(size_t)b].card].cell.second;
            if (ra != rb) return ra < rb;
            return jackX(cables[(size_t)a]) < jackX(cables[(size_t)b]);
        });
        std::map<int, int> corridorUse;
        for (size_t k = 0; k < order.size(); ++k)
        {
            auto& cable = cables[(size_t)order[k]];
            cable.gutterLane = (int)k;
            cable.corridorLane = corridorUse[cards[(size_t)cable.card].cell.second]++;
        }
        for (auto& cable : cables)
        {
            const auto& card = cards[(size_t)cable.card];
            const auto& row = rows[(size_t)cable.row];
            const float gx = (float)gridLeft - 10.0f - (float)laneGap * (float)cable.gutterLane;
            const float y0 = (float)row.bounds.getCentreY();
            const float x0 = (float)row.bounds.getRight();
            const float cy = (float)rowBottom[(size_t)card.cell.second] + 12.0f + (float)laneGap * (float)cable.corridorLane;
            const int first = cable.channels.front();
            const auto jack = first < (int)card.jacks.size() ? card.jacks[(size_t)first] : card.bounds.withTrimmedTop(card.bounds.getHeight() - 10).withWidth(20);
            const float jx = (float)jack.getCentreX() + (cable.channels.size() > 1 ? (float)(jack.getWidth() + 2) / 2.0f : 0.0f) + 3.0f * (float)cable.ring;
            const float r = 6.0f, down = cy > y0 ? 1.0f : -1.0f;
            juce::Path p;
            p.startNewSubPath(x0, y0);
            p.lineTo(gx - r, y0); p.quadraticTo(gx, y0, gx, y0 + r * down);
            p.lineTo(gx, cy - r * down); p.quadraticTo(gx, cy, gx + r, cy);
            p.lineTo(jx - r, cy); p.quadraticTo(jx, cy, jx, cy - r);
            p.lineTo(jx, (float)jack.getBottom() + 3.0f * (float)cable.ring);
            cable.path = p;
            cable.hitPath.clear();
            juce::PathStrokeType(10.0f).createStrokedPath(cable.hitPath, p);
            cable.removeBox = juce::Rectangle<int>((int)x0 + 6, (int)y0 - 18, 14, 14);   // ABOVE the cable at its source end - never on the path (0.9.83: the x sat on the corridor turn and a grab removed the cable)
        }
    }
    void paintCables(juce::Graphics& g)
    {
        for (int i = 0; i < (int)cables.size(); ++i)
        {
            if (i == liftedCable) continue;   // picked up: it hangs from the mouse
            const auto& cable = cables[(size_t)i];
            const auto& row = rows[(size_t)cable.row];
            const bool selected = i == selectedCable;
            const float alpha = selectedCable >= 0 && !selected ? 0.25f : 0.9f;
            g.setColour(row.colour.withAlpha(alpha));
            const float width = selected ? 3.5f : 2.5f;
            const auto& card = cards[(size_t)cable.card];
            const float ring = 3.0f * (float)cable.ring;
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
                    g.drawRoundedRectangle(card.jacks[(size_t)cable.channels.front()].toFloat().expanded(3.0f + ring), 4.0f, 2.0f);
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
            if (card.bounds.isEmpty() || !card.bounds.expanded(6).contains(p) || card.unavailable || (card.busy && !freedByDrag(card))) continue;
            if (dragCard >= 0 && (!card.chip || i == dragCard)) continue;   // a dragged card lands on a pool button only (its routes move there)
            dropCard = i;
            if (card.chip || card.jacks.empty()) { dropChannel = 0; return; }
            int best = -1; int bestDistance = 1 << 30;
            for (int ch = 0; ch < (int)card.jacks.size(); ++ch)
            {
                const int d = card.jacks[(size_t)ch].getCentre().getDistanceFrom(p);
                if (d < bestDistance) { bestDistance = d; best = ch; }
            }
            dropChannel = best;
            if (dropPairs && dropChannel + 1 >= (int)card.jacks.size() && dropChannel > 0 && (int)card.jacks.size() % 2 == 0) dropChannel -= 1;
            return;
        }
    }

    // ---- the actions ----
    void assign(const std::vector<Route>& routes, const Card& card, std::vector<int> channels, bool add, const std::vector<int>& takeAway = {})
    {
        // 0.9.105: a card that is not open is ASKED before any route is touched. Its device is not there: nothing
        // moves (the cable is back where it was), the card turns grey with the driver's reason - no six seconds of
        // waiting, no message box (the user dropped a cable on "UMC ASIO Driver" with the interface unplugged).
        if (!card.open && !card.busy && !owner.probeCard(card.type, card.name))
        {
            CrashHandler::note("Audio routing: " + card.name + " (" + shownType(card.type) + ") is not there - the cable stays where it was");
            rebuild(); repaint();
            return;
        }
        juce::StringArray done, refused;
        const double now = juce::Time::getMillisecondCounterHiRes();
        bool deviceChanged = false;
        if (card.busy)
        {   // the hardware is open through another driver by these very routes (freedByDrag): close them first, so
            // the other driver (ASIO) can open the card
            owner.audioMuted.store(true); owner.engine.noteOffAll(); owner.ignoreDeviceRestartsUntil.store(now + 8000.0); deviceChanged = true;
            for (const auto& route : routes) owner.detachRouteAudioQuietly(route.manual, route.perspective);
            CrashHandler::note("Audio routing: " + card.name + " was busy through another driver - its routes closed first");
        }
        // 0.9.90: an aggregated card opens anew for every route's outputs (up to a second each): the routes go one at a
        // time behind the "set for all" progress window, the dialog is rebuilt when the queue is done.
        // 0.9.91 (the user: "why the dialog at every cable?"): the window only for three or more routes that change
        // card at once (a folded perspective of a big organ); a cable or two are set at once, as before 0.9.90.
        int cardChanges = 0;
        for (const auto& route : routes) if (cardKeyOf(route) != card.key) ++cardChanges;
        if (card.type.equalsIgnoreCase("Aggregated WASAPI") && cardChanges >= 3)
        {
            std::vector<MainComponent::RoutingAssignStep> steps;
            for (const auto& route : routes)
            {
                const bool changeDevice = cardKeyOf(route) != card.key;
                auto wanted = channels;
                if (add && !changeDevice)
                    for (const int c : channelsOf(route))
                        if (std::find(wanted.begin(), wanted.end(), c) == wanted.end() && std::find(takeAway.begin(), takeAway.end(), c) == takeAway.end()) wanted.push_back(c);
                std::sort(wanted.begin(), wanted.end());
                steps.push_back({ route.manual, route.perspective, wanted, changeDevice });
                if (changeDevice && !deviceChanged) { owner.audioMuted.store(true); owner.engine.noteOffAll(); owner.ignoreDeviceRestartsUntil.store(now + 8000.0 + 1500.0 * (double)routes.size()); deviceChanged = true; }
            }
            selectedCable = -1;
            juce::Component::SafePointer<AudioRoutingContent> safeThis(this);
            owner.beginRoutingAssign(card.type, card.name, card.rate, card.buffer, std::move(steps), [safeThis] { if (safeThis != nullptr) safeThis->rebuild(); });
            return;
        }
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
               #if JUCE_LINUX
                // 0.9.95 (#242): a card PipeWire still holds does not open at once - the route keeps it and the
                // watcher opens it when it answers (never a wait here; before, the route fell back to its old card).
                owner.rememberWantedRouteDevice(route.manual, route.perspective, *state);
               #endif
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
    // The lifted cable let go on another output (or card): its channels leave, the new ones come; the route's other
    // cables stay (on another card the whole route moves - a route plays through one card).
    void moveCable(const std::vector<Route>& routes, const std::vector<int>& oldChannels, const juce::String& label, const Card& card, const std::vector<int>& channels)
    {
        CrashHandler::note("Audio routing: cable " + channelsText(oldChannels) + " of " + label + " moved to " + card.name + " outputs " + channelsText(channels));
        assign(routes, card, channels, true, oldChannels);
    }
    struct CableData { std::vector<Route> routes; std::vector<int> channels; juce::String label; };
    CableData cableData(int index) const
    {
        if (index < 0 || index >= (int)cables.size()) return {};
        const auto& cable = cables[(size_t)index];
        return { rows[(size_t)cable.row].routes, cable.channels, rows[(size_t)cable.row].label };
    }
    void removeCable(int index) { removeCableData(cableData(index)); }
    void removeCableData(const CableData& cable)
    {
        juce::StringArray done, refused;
        bool detached = false;
        for (const auto& route : cable.routes)
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
    // A full card dropped at a place of the rows (0.9.108): before a card, after the last card of a line, or as a
    // new row; the others move aside. The order of every full card - "<order>,<row>|<card>", the lines as they
    // stand on the screen - is the machine's memory.
    void placeCard(const juce::String& key, const juce::String& beforeKey, const juce::String& afterKey, bool newRow)
    {
        auto indexOf = [this](const juce::String& k) { if (k.isNotEmpty()) for (int i = 0; i < (int)cards.size(); ++i) if (!cards[(size_t)i].chip && cards[(size_t)i].key == k) return i; return -1; };
        const int index = indexOf(key);
        if (index < 0 || key == beforeKey || key == afterKey) return;   // (let go where it stood)
        std::vector<std::pair<int, int>> sequence;   // card, line - without the moved card
        for (const int i : cardOrder) if (i != index && i >= 0 && i < (int)cards.size() && !cards[(size_t)i].chip) sequence.push_back({ i, cards[(size_t)i].cell.second });
        int lines = 0;
        for (const auto& s : sequence) lines = juce::jmax(lines, s.second + 1);
        const int target = indexOf(beforeKey.isNotEmpty() ? beforeKey : afterKey);
        bool placed = false;
        if (!newRow && target >= 0)
            for (size_t i = 0; i < sequence.size() && !placed; ++i)
                if (sequence[i].first == target)
                {
                    const int line = sequence[i].second;
                    sequence.insert(sequence.begin() + (long)(beforeKey.isNotEmpty() ? i : i + 1), { index, line });
                    placed = true;
                }
        if (!placed) sequence.push_back({ index, newRow ? lines : juce::jmax(0, lines - 1) });
        owner.audioRoutingCardCells.clear();
        int row = -1, lineBefore = INT_MIN, position = 0;
        for (const auto& s : sequence)
        {
            if (s.second != lineBefore) { ++row; position = 0; lineBefore = s.second; }
            auto& c = cards[(size_t)s.first];
            c.place = { position++, row };
            owner.audioRoutingCardCells.add(juce::String(c.place.first) + "," + juce::String(c.place.second) + "|" + c.key);
        }
        owner.saveSettings();
        CrashHandler::note("Audio routing: card " + cards[(size_t)index].name + (newRow || target < 0 ? juce::String(" placed in a new row")
            : (beforeKey.isNotEmpty() ? " placed before " : " placed after ") + cards[(size_t)target].name)
            + " - row " + juce::String(cards[(size_t)index].place.second + 1) + ", place " + juce::String(cards[(size_t)index].place.first + 1));
        rebuild();
    }
    void reopenWith(juce::AudioDeviceManager* manager, const juce::AudioDeviceManager::AudioDeviceSetup& setup, const juce::String& what)
    {
        owner.audioMuted.store(true);
        owner.engine.noteOffAll();
        owner.ignoreDeviceRestartsUntil.store(juce::Time::getMillisecondCounterHiRes() + 4000.0);
       #if JUCE_LINUX
        ++pipeWireUserSetsClock();   // the user's own choice may change PipeWire's clock for every card
       #endif
        const auto error = manager->setAudioDeviceSetup(setup, true);
       #if JUCE_LINUX
        --pipeWireUserSetsClock();
       #endif
        CrashHandler::note("Audio routing: " + what + " of " + setup.outputDeviceName + " set to " + (what == "rate" ? juce::String(juce::roundToInt(setup.sampleRate)) + " Hz" : juce::String(setup.bufferSize)) + (error.isNotEmpty() ? " - " + error : juce::String()));
        if (error.isNotEmpty()) owner.noteMessage("Audio routing", error);
        owner.saveSettings();
        owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1500.0;
        rebuild();
    }
    void runTestSpec(const juce::String& spec)
    {
        const auto parts = juce::StringArray::fromTokens(spec, "|", "");
        // "dragcard|<card name part>|<dx>|<dy>": the card held in a drag that far from its place (for a picture; it is
        // let go by the next mouse click in the dialog).
        if (parts.size() >= 4 && parts[0].trim().equalsIgnoreCase("dragcard"))
        {
            for (int i = 0; i < (int)cards.size(); ++i)
                if (!cards[(size_t)i].chip && cards[(size_t)i].name.containsIgnoreCase(parts[1].trim()))
                {
                    dragCard = i;
                    cardGrab = { 60.0f, 14.0f };
                    dragPoint = cards[(size_t)i].bounds.getPosition().toFloat() + cardGrab + juce::Point<float>((float)parts[2].getIntValue(), (float)parts[3].getIntValue());
                    beginCardDrag();
                    dropPlace = dropPlaceAt(dragPoint.toInt());
                    repaint();
                    break;
                }
            CrashHandler::note("ORGAN_TEST_ROUTE: " + spec + (dragCard < 0 ? " - no such full card" : " - held"));
            return;
        }
        // "place|<card name part>|before|<other card>", "...|after|<other card>", "...|newrow|x": a full card moved.
        if (parts.size() >= 4 && parts[0].trim().equalsIgnoreCase("place"))
        {
            auto find = [this](const juce::String& part) { for (int i = 0; i < (int)cards.size(); ++i) if (!cards[(size_t)i].chip && cards[(size_t)i].name.containsIgnoreCase(part.trim())) return i; return -1; };
            const int index = find(parts[1]), other = find(parts[3]);
            const auto how = parts[2].trim().toLowerCase();
            CrashHandler::note("ORGAN_TEST_ROUTE: " + spec + (index < 0 ? " - no such full card" : ""));
            if (index >= 0)
                placeCard(cards[(size_t)index].key, how == "before" && other >= 0 ? cards[(size_t)other].key : juce::String(),
                    how == "after" && other >= 0 ? cards[(size_t)other].key : juce::String(), how == "newrow");
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
            if (i >= 0) { const auto data = cableData(i); moveCable(data.routes, data.channels, data.label, cards[(size_t)cardIndex], channels); }
            else CrashHandler::note("ORGAN_TEST_ROUTE: no cable to move");
            return;
        }
        assign(routes, cards[(size_t)cardIndex], channels, mode == "add");
    }
    juce::String signature() const
    {
        juce::String s;
        s << owner.unavailableCardsVersion << ";";   // a card that went grey or answers again
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
            {
                s << cardKeyOf({ manual, perspective }) << ":" << channelsText(channelsOf({ manual, perspective })) << ":" << owner.manualConfigs[(size_t)manual].alias
                  << ":" << (owner.routeEffectsActive(manual, perspective) ? "fx" : "-") << (owner.perspectiveEffectsActive(perspective) ? "p" : "-") << ";";   // 0.9.89: the fx buttons follow the effects
                if (auto* manager = owner.managerForRoute(manual, perspective))
                    if (auto* device = manager->getCurrentAudioDevice())
                        s << juce::roundToInt(device->getCurrentSampleRate()) << "/" << device->getCurrentBufferSizeSamples() << "/" << device->getOutputLatencyInSamples() << ";";
            }
        // The lists by their NAMES (0.9.95: the count alone missed a card that came while another went).
        for (auto* type : owner.deviceManager.getAvailableDeviceTypes()) if (type != nullptr) s << type->getDeviceNames(false).joinIntoString("|").hashCode64() << ",";
       #if JUCE_LINUX
        for (const auto& item : unusableAlsaDeviceReasons()) s << item.first.hashCode() << "!";
       #endif
        return s;
    }
    void timerCallback() override
    {
        // 0.9.89: the meters every tick (100 ms): the row's peak = the highest of its routes since the last tick, 1 s hold.
        {
            const double now = juce::Time::getMillisecondCounterHiRes();
            bool changed = false;
            // 0.9.92: every route's peak is TAKEN once per tick and shared by the rows that hold the route - the
            // perspective's row took (and reset) the peaks first, so the division rows under it read nothing.
            std::map<std::string, float> peaks;
            auto peakOf = [&](const Route& route)
            {
                const auto key = MainComponent::perspectiveRouteKey(route.manual, route.perspective);
                const auto found = peaks.find(key);
                if (found != peaks.end()) return found->second;
                const float peak = owner.engine.takeRoutePeak(route.manual, route.perspective);
                peaks[key] = peak;
                return peak;
            };
            for (size_t ri = 0; ri < rows.size() && ri < rowMeters.size(); ++ri)
            {
                float peak = 0.0f;
                for (const auto& route : rows[ri].routes) peak = juce::jmax(peak, peakOf(route));
                const float db = peak > 1.0e-6f ? juce::Decibels::gainToDecibels(peak) : -100.0f;
                auto& m = rowMeters[ri];
                if (db >= m.db || now >= m.holdUntil) { if (std::abs(db - m.db) > 0.5f) changed = true; m.db = db; m.holdUntil = now + 1000.0; }
                // The red part: the limiter worked on a sample of this row (the engine's own clip rule: above 0.98).
                // 0.9.100 (the user: "červený peak hold 3 s"): red stays three seconds; and it lights when the SUM on
                // the row's card passed the limiter while the row sounded (the row alone may be far below it).
                bool clipped = peak > 0.98f;
                if (!clipped && db > -40.0f)
                    for (const auto& route : rows[ri].routes)
                        if (owner.routeCardClippedSince(route.manual, route.perspective, now - 150.0)) { clipped = true; break; }
                if (clipped) m.clipUntil = now + 3000.0;
                const bool clipNow = now < m.clipUntil;
                if (clipNow != m.clipShown) { m.clipShown = clipNow; changed = true; }
            }
            if (changed) repaint(juce::Rectangle<int>(12, sourcesTop, leftWidth, juce::jmax(0, sourcesBottom - sourcesTop)));
            masterMeter.repaint();
            if (!masterSlider.isMouseButtonDown() && std::abs(masterSlider.getValue() - owner.masterVolumeSlider.getValue()) > 0.001)
                masterSlider.setValue(owner.masterVolumeSlider.getValue(), juce::dontSendNotification);
            const bool canMeasure = owner.canMeasurePolyphony();
            if (measureButton.isEnabled() != canMeasure) measureButton.setEnabled(canMeasure);
        }
        if (++ticks % 10 != 0) return;
        if (dragging || dragCard >= 0 || pressedCable >= 0) return;
        if (ticks % 20 == 0 && isShowing()) owner.refreshSoundCardsQuietly();   // 0.9.97 (#249): a card switched on or off shows within 2 s
        const auto now = signature();
        if (now != lastSignature) { lastSignature = now; rebuild(); }
    }

    MainComponent& owner;
    juce::Label help, sizeLabel, masterLabel;
    juce::TextButton helpButton;       // 0.9.114: stands for the help text when the header has no room for it
    int separatorY = 86;               // 0.9.114: the line under the header
    // The height the help text needs at a width and a font (the Label's own margins counted).
    int helpHeightFor(int width, float fontHeight) const
    {
        juce::AttributedString text;
        text.append(help.getText(), juce::Font(juce::FontOptions(fontHeight)), juce::Colours::white);
        text.setWordWrap(juce::AttributedString::byWord);
        juce::TextLayout layoutOfText;
        layoutOfText.createLayout(text, (float)juce::jmax(40, width - 14));
        return juce::roundToInt(layoutOfText.getHeight() + fontHeight * 0.5f) + 6;
    }
    juce::ToggleButton showCables;
    juce::ComboBox sizeBox;
    LearnableSlider masterSlider;      // 0.9.89: the console's master volume (the same value)
    MasterMeter masterMeter;           // 0.9.89: the console's audio meter (the same state)
    juce::TextButton measureButton;    // 0.9.89: the polyphony measurement (was on the audio tabs)
    juce::TextButton saveRoutingButton, loadRoutingButton, defaultRoutingButton;   // 0.9.94: the routing file
#if JUCE_LINUX
    juce::ToggleButton unusableButton; // 0.9.89: the ALSA "list unusable devices" switch (was on the audio tabs)
    juce::ToggleButton directAlsaButton;       // 0.9.114: "Show the cards' direct ALSA devices" (PipeWire systems; in the unfolded ALSA section)
    juce::Rectangle<int> directAlsaNote;       // where its warning is painted; empty = not shown
#endif
    // 0.9.90: the fx button takes a right click (the reset menu) - a TextButton swallows it as a press otherwise.
    struct FxButton final : public juce::TextButton
    {
        FxButton() : juce::TextButton("fx") {}
        std::function<void()> onRightClick;
        void mouseDown(const juce::MouseEvent& e) override { if (e.mods.isPopupMenu()) { if (onRightClick) onRightClick(); return; } juce::TextButton::mouseDown(e); }
        void mouseUp(const juce::MouseEvent& e) override { if (e.mods.isPopupMenu()) return; juce::TextButton::mouseUp(e); }
    };
    juce::OwnedArray<FxButton> fxButtons;   // 0.9.89: one per row
    struct RowMeter { float db = -100.0f; double holdUntil = 0.0, clipUntil = 0.0; bool clipShown = false; };
    std::vector<RowMeter> rowMeters;
    int ticks = 0;
    std::vector<Row> rows;
    std::vector<Card> cards;
    std::vector<Cable> cables;
    struct DropPlace { juce::Rectangle<int> line; int before = -1, after = -1; bool newRow = false; };   // the landing line (the strip of a new row), the card it lands before / after
    std::vector<DropPlace> dropPlaces;
    std::vector<int> cardOrder;   // the full cards in their order (row by row)
    std::vector<int> rowTop, rowBottom;   // the grid rows' top and bottom (the corridor lies under the bottom)
    juce::OwnedArray<CardControls> controls;
    std::set<juce::String> unfolded;
    std::vector<TypeSection> typeSections;
    int sourcesTop = 94, sourcesBottom = 94, fullCardsBottom = 94, chipsTop = 94, gridLeft = 320, gutterWidth = 44;
    int dragRow = -1, dragCard = -1, liftedCable = -1, pressedCable = -1, dropCard = -1, dropChannel = -1, selectedCable = -1;
    int dropPlace = -1;   // the drop place of a dragged card (an index of dropPlaces)
    bool dragging = false, draggingCard = false, dropAdd = false, dropPairs = true;
    bool viaPipeWire = false;   // 0.9.96 (Linux): PipeWire carries the cards - set in rebuild()
    juce::Point<float> dragPoint;
    juce::Point<float> cardGrab;     // where in the card the mouse took it
    juce::Image cardImage;           // the dragged card's own picture
    struct FrameTicker final : public juce::Timer { std::function<void()> tick; void timerCallback() override { if (tick) tick(); } };
    FrameTicker cardTicker;
    juce::String lastSignature;
};
