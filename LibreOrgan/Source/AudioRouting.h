#pragma once
// 0.9.79 (#236): THE AUDIO ROUTING DIALOG - the sound cards as blocks, the routes as chips in channel cells, in
// the language of the stop grid (the user: "udržovat určitý estetický kód"):
//   - a BLOCK = one sound card (type + name, its rate and buffer in the header), the "No sound card" block last;
//     "Add sound card" makes an empty block for another card (the device opens when the first route lands there);
//   - the block's GRID: the columns are the card's output channels (1 ... N), the rows the routes on that card
//     ("I. / Front" - a perspective of a division); a CHIP in a cell = the route plays on that channel, several
//     chips of one row = several channels, the pipes spread over them in turn (the multi-channel rule stays);
//   - a perspective that every division sends to the same card and channels is ONE folded row "All divisions /
//     Front" (a stop folder, [N] = the divisions); the arrow unfolds it into the division rows;
//   - DRAG a chip into a free cell = the route moves to that channel (an occupied cell: the two swap); with
//     Shift or the right button = a COPY: one more channel for the route (the transmission gesture); a chip
//     dragged out of every grid = that channel taken away (a route keeps one at least); a chip dropped on
//     ANOTHER block = the route moves to that card with that channel;
//   - "Test" plays a soft tone for a second on the route's channels alone, so the user hears where it goes.
// Everything goes through the same calls the Divisions dialog uses (setRouteAudioType, applyChannelsToDivision,
// detachRouteAudioQuietly): both dialogs show the same data, the Divisions dialog stays as the second way.
#include <JuceHeader.h>
#include "MainComponent.h"

class AudioRoutingContent final : public juce::Component, private juce::Timer
{
public:
    explicit AudioRoutingContent(MainComponent& o) : owner(o)
    {
        getProperties().set("manualChapter", "Audio routing");
        addButton.setButtonText("Add sound card...");
        addButton.setTooltip("Another sound card as a new block: drag a route's chip onto it and the card opens for that route.");
        addButton.onClick = [this] { chooseCardToAdd(); };
        addAndMakeVisible(addButton);
        hint.setText("Drag a chip into a cell to change the channel (Shift or the right button: a copy = one more channel); "
            "drag it onto another card to move the route there; drag it out of the grids to take the channel away. "
            "The pipes of a route with several channels are spread over them in turn.", juce::dontSendNotification);
        hint.setFont(juce::FontOptions(13.0f));
        hint.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        hint.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(hint);
        rebuild();
        startTimer(1000);   // devices come and go: the blocks follow (no device call - the routes' managers are read)
    }

    void resized() override { layout(); }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        for (const auto& block : blocks)
        {
            g.setColour(juce::Colours::black.withAlpha(0.25f));
            g.fillRoundedRectangle(block.bounds.toFloat(), 6.0f);
            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
            g.drawText(block.title, block.bounds.getX() + 10, block.bounds.getY() + 4, block.bounds.getWidth() - 20, 24, juce::Justification::centredLeft);
            g.setColour(juce::Colours::lightgrey);
            g.setFont(juce::FontOptions(12.0f));
            g.drawText(block.subtitle, block.bounds.getX() + 10, block.bounds.getY() + 26, block.bounds.getWidth() - 20, 16, juce::Justification::centredLeft);
            // The column headers (channel numbers) and the cells.
            for (int channel = 0; channel < block.channels; ++channel)
            {
                const auto head = cellRect(block, -1, channel);
                g.setColour(juce::Colours::grey);
                g.setFont(juce::FontOptions(12.0f));
                g.drawText(juce::String(channel + 1), head, juce::Justification::centred);
            }
            for (int row = 0; row < (int)block.rows.size(); ++row)
            {
                const auto& r = block.rows[(size_t)row];
                g.setColour(r.all ? juce::Colours::white : juce::Colours::lightgrey);
                g.setFont(juce::FontOptions(13.0f, r.all ? juce::Font::bold : juce::Font::plain));
                const auto labelArea = rowLabelRect(block, row);
                g.drawFittedText(r.label, labelArea.withTrimmedLeft(r.all ? 0 : 14), juce::Justification::centredLeft, 1);
                for (int channel = 0; channel < block.channels; ++channel)
                {
                    const auto cell = cellRect(block, row, channel);
                    const bool held = std::find(r.channels.begin(), r.channels.end(), channel) != r.channels.end();
                    if (held)
                    {
                        const bool dragging = dragChip.block == &block - blocks.data() && dragChip.row == row && dragChip.channel == channel && dragChip.active;
                        g.setColour(chipColour(r.perspective).withAlpha(dragging ? 0.35f : 1.0f));
                        g.fillRoundedRectangle(cell.toFloat().reduced(2.0f), 4.0f);
                        g.setColour(juce::Colours::black);
                        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                        g.drawFittedText(cell.getWidth() < 40 ? r.perspective.substring(0, 3) : r.perspective, cell.reduced(2, 0), juce::Justification::centred, 1);
                    }
                    else
                    {
                        juce::Path outline;
                        outline.addRoundedRectangle(cell.toFloat().reduced(3.0f), 4.0f);
                        juce::Path dashed;
                        const float dashes[]{ 4.0f, 4.0f };
                        juce::PathStrokeType(1.0f).createDashedStroke(dashed, outline, dashes, 2);
                        g.setColour(juce::Colours::white.withAlpha(0.25f));
                        g.fillPath(dashed);
                    }
                }
            }
            if (block.rows.empty())
            {
                g.setColour(juce::Colours::grey);
                g.setFont(juce::FontOptions(13.0f));
                g.drawText(block.none ? "Every route has a sound card." : "No route plays on this card yet - drag a chip here.",
                    block.bounds.getX() + 10, block.bounds.getY() + 48, block.bounds.getWidth() - 20, 24, juce::Justification::centredLeft);
            }
        }
        if (dragChip.active)
        {
            const auto& block = blocks[(size_t)dragChip.block];
            const auto& r = block.rows[(size_t)dragChip.row];
            const auto cell = cellRect(block, dragChip.row, dragChip.channel).withCentre(dragPosition);
            g.setColour(chipColour(r.perspective).withAlpha(0.85f));
            g.fillRoundedRectangle(cell.toFloat().reduced(2.0f), 4.0f);
            g.setColour(juce::Colours::black);
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawFittedText(dragCopy ? r.perspective + " +" : r.perspective, cell.reduced(3, 0), juce::Justification::centred, 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        dragChip = {};
        for (int b = 0; b < (int)blocks.size(); ++b)
        {
            const auto& block = blocks[(size_t)b];
            for (int row = 0; row < (int)block.rows.size(); ++row)
                for (const int channel : block.rows[(size_t)row].channels)
                    if (cellRect(block, row, channel).contains(e.getPosition()))
                    {
                        dragChip = { b, row, channel, false };
                        dragCopy = e.mods.isShiftDown() || e.mods.isRightButtonDown();
                        dragPosition = e.getPosition();
                        return;
                    }
            for (int row = 0; row < (int)block.rows.size(); ++row)
                if (block.rows[(size_t)row].all && foldRect(block, row).contains(e.getPosition()))
                {
                    const auto& perspective = block.rows[(size_t)row].perspective;
                    if (unfolded.count(perspective) != 0) unfolded.erase(perspective); else unfolded.insert(perspective);
                    rebuild();
                    return;
                }
        }
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (dragChip.block < 0) return;
        if (!dragChip.active && e.getDistanceFromDragStart() > 6) dragChip.active = true;
        dragPosition = e.getPosition();
        if (dragChip.active) repaint();
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (!dragChip.active) { dragChip = {}; return; }
        const auto chip = dragChip;
        dragChip = {};
        repaint();
        // The target: a cell of any block, or nowhere.
        for (int b = 0; b < (int)blocks.size(); ++b)
        {
            const auto& block = blocks[(size_t)b];
            if (!block.bounds.contains(e.getPosition())) continue;
            if (block.none) { moveRoutesToNone(chip); return; }
            for (int channel = 0; channel < block.channels; ++channel)
                for (int row = -1; row <= (int)block.rows.size(); ++row)   // any row of the block (a new row below too)
                    if (cellRect(block, row, channel).contains(e.getPosition()))
                    {
                        if (b == chip.block) moveChipWithinBlock(chip, channel);
                        else moveRoutesToBlock(chip, b, channel);
                        return;
                    }
            return;   // on the block, beside the cells: nothing
        }
        removeChipChannel(chip);   // dropped out of every grid: the channel goes
    }

private:
    struct Route { int manual = 0; juce::String perspective; };
    struct Row
    {
        juce::String label, perspective;
        std::vector<Route> routes;    // one route, or every division's route of the perspective (a folded "All divisions")
        std::vector<int> channels;    // the channels (the same for every route of a folded row)
        bool all = false;
    };
    struct Block
    {
        juce::String type, name, title, subtitle;
        bool none = false;            // the "No sound card" block
        int channels = 0;
        double rate = 0.0; int buffer = 0;
        std::vector<Row> rows;
        juce::Rectangle<int> bounds;
    };
    struct DragChip { int block = -1, row = -1, channel = -1; bool active = false; };

    static constexpr int labelWidth = 170, cellWidth = 46, cellHeight = 30, cellGap = 4, headerHeight = 46, blockGap = 12, testWidth = 48;
    static const juce::String& divisionName(int manual)
    {
        static const juce::String names[8]{ "Pedal", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
        return names[juce::jlimit(0, 7, manual)];
    }
    static juce::Colour chipColour(const juce::String& perspective)
    {
        // One hue per perspective name (the same in every block).
        const float hue = (float)((perspective.toLowerCase().hashCode() & 0xffff) % 360) / 360.0f;
        return juce::Colour::fromHSV(hue, 0.55f, 0.95f, 1.0f);
    }
    // The cells shrink so every channel of a wide card (32 on an aggregate) fits the block's width.
    int cellWidthFor(const Block& block) const
    {
        const int available = block.bounds.getWidth() - 10 - labelWidth - testWidth - 10;
        return juce::jlimit(22, cellWidth, available / juce::jmax(1, block.channels) - cellGap);
    }
    juce::Rectangle<int> cellRect(const Block& block, int row, int channel) const
    {
        const int w = cellWidthFor(block);
        return { block.bounds.getX() + 10 + labelWidth + testWidth + channel * (w + cellGap),
                 block.bounds.getY() + headerHeight + (row + 1) * (cellHeight + cellGap), w, cellHeight };
    }
    juce::Rectangle<int> rowLabelRect(const Block& block, int row) const
    {
        return { block.bounds.getX() + 10, block.bounds.getY() + headerHeight + (row + 1) * (cellHeight + cellGap), labelWidth - 4, cellHeight };
    }
    juce::Rectangle<int> foldRect(const Block& block, int row) const { return rowLabelRect(block, row).removeFromLeft(14); }
    static int blockHeight(const Block& block)
    {
        return headerHeight + (juce::jmax(1, (int)block.rows.size()) + 1) * (cellHeight + cellGap) + 8;
    }

    const std::vector<int>& channelsOf(const Route& route) const
    {
        const auto& config = owner.manualConfigs[(size_t)route.manual];
        if (route.perspective.equalsIgnoreCase("default")) return config.outputChannels;
        const auto key = MainComponent::perspectiveRouteKey(route.manual, route.perspective);
        const auto found = config.perspectiveAudio.find(key);
        static const std::vector<int> none;
        return found != config.perspectiveAudio.end() ? found->second.outputChannels : none;
    }

    // The blocks from the live routes: one per (type, name) of an open device, the empty ones added by the
    // user kept, "No sound card" last.
    void rebuild()
    {
        std::vector<Block> fresh;
        fresh.reserve(256);   // blockFor hands out references into it: no reallocation while they live
        auto blockFor = [&](const juce::String& type, const juce::String& name, juce::AudioIODevice* device) -> Block&
        {
            for (auto& block : fresh) if (!block.none && block.type == type && block.name == name) return block;
            Block block;
            block.type = type; block.name = name;
            block.title = name;
            if (device != nullptr)
            {
                block.channels = device->getOutputChannelNames().size();
                block.rate = device->getCurrentSampleRate();
                block.buffer = device->getCurrentBufferSizeSamples();
                block.subtitle = type + "  -  " + juce::String(block.channels) + " output channels, " + juce::String(block.rate, 0) + " Hz, buffer " + juce::String(block.buffer);
            }
            else { block.channels = 8; block.subtitle = type + "  -  not open yet (the card opens when a route lands here)"; }
            fresh.push_back(block);
            return fresh.back();
        };
        for (const auto& added : addedCards) blockFor(added.first, added.second, nullptr);
        Block none;
        none.none = true; none.title = "No sound card"; none.subtitle = "routes that play through no device (silent) - drag a chip here to silence a route";
        std::map<juce::String, std::vector<Route>> routesOfPerspective;   // for the "All divisions" folding
        std::vector<std::pair<Route, Block*>> placed;
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
            {
                Route route{ manual, perspective };
                auto* manager = owner.managerForRoute(manual, perspective);
                auto* device = manager != nullptr ? manager->getCurrentAudioDevice() : nullptr;
                if (device == nullptr) { none.rows.push_back(Row{ divisionName(manual) + " / " + perspective, perspective, { route }, {}, false }); continue; }
                auto& block = blockFor(manager->getCurrentAudioDeviceType(), device->getName(), device);
                if (block.channels <= 0 || block.rate <= 0.0) { block.channels = device->getOutputChannelNames().size(); block.rate = device->getCurrentSampleRate(); block.buffer = device->getCurrentBufferSizeSamples();
                    block.subtitle = block.type + "  -  " + juce::String(block.channels) + " output channels, " + juce::String(block.rate, 0) + " Hz, buffer " + juce::String(block.buffer); }
                routesOfPerspective[perspective].push_back(route);
                placed.push_back({ route, &block });
            }
        // Folding: a perspective every division sends to ONE card with the same channels = one row there.
        std::set<juce::String> foldedPerspectives;
        for (const auto& [perspective, routes] : routesOfPerspective)
        {
            if ((int)routes.size() != owner.manualCount + 1 || unfolded.count(perspective) != 0) continue;
            Block* card = nullptr; bool same = true; std::vector<int> channels;
            for (const auto& route : routes)
            {
                Block* b = nullptr;
                for (const auto& p : placed) if (p.first.manual == route.manual && p.first.perspective == route.perspective) b = p.second;
                if (card == nullptr) { card = b; channels = channelsOf(route); }
                else if (b != card || channelsOf(route) != channels) { same = false; break; }
            }
            if (same && card != nullptr)
            {
                card->rows.push_back(Row{ "All divisions / " + perspective + " [" + juce::String(routes.size()) + "]", perspective, routes, channels, true });
                foldedPerspectives.insert(perspective);
            }
        }
        for (const auto& [route, block] : placed)
        {
            if (foldedPerspectives.count(route.perspective) != 0) continue;
            block->rows.push_back(Row{ divisionName(route.manual) + " / " + route.perspective, route.perspective, { route }, channelsOf(route), false });
        }
        fresh.push_back(none);
        blocks = std::move(fresh);
        // The "Test" buttons: one per row.
        testButtons.clear();
        for (int b = 0; b < (int)blocks.size(); ++b)
            for (int row = 0; row < (int)blocks[(size_t)b].rows.size(); ++row)
            {
                auto* button = testButtons.add(new juce::TextButton("Test"));
                button->setTooltip("Plays a soft tone for a second on this route's channels alone - you hear where it goes.");
                const auto routes = blocks[(size_t)b].rows[(size_t)row].routes;
                button->onClick = [this, routes] { for (const auto& route : routes) owner.startRouteTestTone(route.manual, route.perspective); };
                button->setEnabled(!blocks[(size_t)b].none);
                addAndMakeVisible(button);
            }
        layout();
        repaint();
    }
    void layout()
    {
        int y = 8;
        const int width = juce::jmax(300, getWidth() - 16);
        addButton.setBounds(8, y, 150, 26);
        hint.setBounds(170, y - 2, width - 170, 44);
        y += 50;
        int testIndex = 0;
        for (auto& block : blocks)
        {
            block.bounds = { 8, y, width, blockHeight(block) };
            for (int row = 0; row < (int)block.rows.size(); ++row)
                if (auto* button = testButtons[testIndex++])
                    button->setBounds(block.bounds.getX() + 10 + labelWidth, block.bounds.getY() + headerHeight + (row + 1) * (cellHeight + cellGap) + 3, testWidth - 6, cellHeight - 6);
            y += block.bounds.getHeight() + blockGap;
        }
        if (y != getHeight()) setSize(getWidth(), y);   // the ScrollableContent follows
    }
    juce::String signature() const
    {
        juce::String s;
        for (int manual = 0; manual <= owner.manualCount && manual < SamplerEngine::maxManuals; ++manual)
            for (const auto& perspective : owner.engine.getPerspectivesForManual(manual))
            {
                auto* manager = owner.managerForRoute(manual, perspective);
                auto* device = manager != nullptr ? manager->getCurrentAudioDevice() : nullptr;
                s << manual << "|" << perspective << "|" << (device != nullptr ? device->getName() : juce::String("-")) << "|";
                for (const int c : channelsOf({ manual, perspective })) s << c << ",";
                s << ";";
            }
        return s;
    }
    void timerCallback() override
    {
        if (dragChip.active) return;
        const auto now = signature();
        if (now != lastSignature) { lastSignature = now; rebuild(); }
    }

    // ---- the actions: every one through the Divisions dialog's own calls ----
    void applyChannels(const Route& route, std::vector<int> channels)
    {
        std::sort(channels.begin(), channels.end());
        channels.erase(std::unique(channels.begin(), channels.end()), channels.end());
        if (channels.empty()) return;   // a route keeps one channel at least
        juce::StringArray done, refused;
        owner.applyChannelsToDivision(route.manual, route.perspective, channels, done, refused);
        if (!refused.isEmpty()) owner.noteMessage("Audio routing", "Not set: " + refused.joinIntoString("; "));
        CrashHandler::note("Audio routing: " + divisionName(route.manual) + " / " + route.perspective + " channels " + [&] { juce::StringArray l; for (const int c : channels) l.add(juce::String(c + 1)); return l.joinIntoString("+"); }());
    }
    void moveChipWithinBlock(const DragChip& chip, int targetChannel)
    {
        const auto& row = blocks[(size_t)chip.block].rows[(size_t)chip.row];
        if (targetChannel == chip.channel && !dragCopy) return;
        for (const auto& route : row.routes)
        {
            auto channels = channelsOf(route);
            if (!dragCopy) std::replace(channels.begin(), channels.end(), chip.channel, targetChannel);
            else channels.push_back(targetChannel);
            applyChannels(route, channels);
        }
        owner.saveSettings();
        rebuild();
    }
    void removeChipChannel(const DragChip& chip)
    {
        const auto& row = blocks[(size_t)chip.block].rows[(size_t)chip.row];
        for (const auto& route : row.routes)
        {
            auto channels = channelsOf(route);
            if (channels.size() <= 1) { owner.noteMessage("Audio routing", "A route keeps one channel at least - drag it onto \"No sound card\" to silence it."); return; }
            channels.erase(std::remove(channels.begin(), channels.end(), chip.channel), channels.end());
            applyChannels(route, channels);
        }
        owner.saveSettings();
        rebuild();
    }
    void moveRoutesToBlock(const DragChip& chip, int targetBlock, int targetChannel)
    {
        const auto& row = blocks[(size_t)chip.block].rows[(size_t)chip.row];
        const auto& target = blocks[(size_t)targetBlock];
        auto state = std::make_unique<juce::XmlElement>("DEVICESETUP");
        state->setAttribute("deviceType", target.type);
        state->setAttribute("audioOutputDeviceName", target.name);
        state->setAttribute("audioInputDeviceName", "");
        if (target.rate > 0.0) state->setAttribute("audioDeviceRate", target.rate);
        if (target.buffer > 0) state->setAttribute("audioDeviceBufferSize", target.buffer);
        const double now = juce::Time::getMillisecondCounterHiRes();
        owner.audioMuted.store(true);
        owner.engine.noteOffAll();
        owner.ignoreDeviceRestartsUntil.store(now + 8000.0);
        for (const auto& route : row.routes)
        {
            CrashHandler::note("Audio routing: " + divisionName(route.manual) + " / " + route.perspective + " moved to " + target.name + " (" + target.type + "), channel " + juce::String(targetChannel + 1));
            owner.setRouteAudioType(route.manual, route.perspective, target.type, state.get(), target.name);
            applyChannels(route, { targetChannel });
        }
        owner.saveSettings();
        owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1500.0;
        rebuild();
    }
    void moveRoutesToNone(const DragChip& chip)
    {
        const auto& row = blocks[(size_t)chip.block].rows[(size_t)chip.row];
        owner.audioMuted.store(true);
        owner.engine.noteOffAll();
        owner.ignoreDeviceRestartsUntil.store(juce::Time::getMillisecondCounterHiRes() + 4000.0);
        for (const auto& route : row.routes)
        {
            CrashHandler::note("Audio routing: " + divisionName(route.manual) + " / " + route.perspective + " set to no sound card");
            owner.detachRouteAudioQuietly(route.manual, route.perspective);
        }
        owner.saveSettings();
        owner.audioDeviceSelectionUnmuteAt = juce::Time::getMillisecondCounterHiRes() + 1200.0;
        rebuild();
    }
    void chooseCardToAdd()
    {
        juce::PopupMenu menu;
        std::vector<std::pair<juce::String, juce::String>> choices;
        for (auto* type : owner.deviceManager.getAvailableDeviceTypes())
        {
            if (type == nullptr) continue;
            type->scanForDevices();
            for (const auto& name : type->getDeviceNames(false))
            {
                bool shown = false;
                for (const auto& block : blocks) if (!block.none && block.type == type->getTypeName() && block.name == name) shown = true;
                if (shown) continue;
                choices.push_back({ type->getTypeName(), name });
                menu.addItem((int)choices.size(), type->getTypeName() + ": " + name);
            }
        }
        if (choices.empty()) { owner.noteMessage("Audio routing", "Every sound card found is shown already."); return; }
        juce::Component::SafePointer<AudioRoutingContent> safeThis(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addButton), [safeThis, choices](int result)
        {
            if (safeThis == nullptr || result <= 0 || result > (int)choices.size()) return;
            safeThis->addedCards.push_back(choices[(size_t)result - 1]);
            safeThis->rebuild();
        });
    }

    MainComponent& owner;
    std::vector<Block> blocks;
    std::vector<std::pair<juce::String, juce::String>> addedCards;   // (type, name) blocks the user added, empty until a route lands
    std::set<juce::String> unfolded;                                   // perspectives whose "All divisions" row is unfolded
    juce::OwnedArray<juce::TextButton> testButtons;
    juce::TextButton addButton;
    juce::Label hint;
    DragChip dragChip;
    bool dragCopy = false;
    juce::Point<int> dragPosition;
    juce::String lastSignature;
};
