#pragma once
// 0.9.89 (the user's design, 17 Nov 2026): THE EFFECTS DIALOG of one ROUTE ("Effects: I. Front") or of one
// PERSPECTIVE ("Effects: Front"), opened by the fx button of its row in the Audio routing dialog. One window at a
// time (modeless; another fx button retargets it).
//   - LEFT the chain in the order the engine applies it: Division volume (a route's only - it belongs to the
//     division, not to the perspective), Volume, Channel separation, Equalizer, IR reverb; RIGHT the panel of the
//     chosen module.
//   - LINKED (a route's dialog, one button per module, in the module's header): ORANGE = the module follows the
//     perspective - its sliders are locked and show the perspective's values; press it and it goes grey, the
//     values are the route's own from now on; press again = the perspective's values come back. Nothing is
//     summed: the perspective's settings are what its linked routes play, each route alone (N routes = N times
//     the DSP - the perspective's dialog says so).
//   - The perspective's dialog names the routes set apart ("3 of 4 routes follow this - set apart: II. Front")
//     and opens any of them.
//   - Every slider has MIDI learn (right-click) and F1 (the Effects chapters of the manual).
// Everything reads the model on a 10 Hz timer (the 0.9.81 rule: a field mirrors the model, never a copy of it).
#include <JuceHeader.h>
#include "MainComponent.h"
#include "UIControls.h"
#include "SampleFiles.h"

// The parametric EQ as a picture: the response over 20 Hz - 20 kHz, +-18 dB, a point per band - drag it (frequency
// and gain), the wheel over it changes Q; locked while LINKED.
class EqGraph final : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<SamplerEngine::EqSettings()> get;
    std::function<void(const SamplerEngine::EqSettings&)> set;
    double sampleRate = 48000.0;
    bool locked = false;

    void paint(juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(area, 4.0f);
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(area, 4.0f, 1.0f);
        // The grid: decades and the +-6 / +-12 dB lines.
        g.setColour(juce::Colours::darkgrey.withAlpha(0.45f));
        for (const float hz : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
        {
            const float x = xOf(hz);
            g.drawVerticalLine((int)x, area.getY(), area.getBottom());
        }
        for (const float db : { -12.0f, -6.0f, 0.0f, 6.0f, 12.0f })
        {
            const float y = yOf(db);
            g.setColour(db == 0.0f ? juce::Colours::grey : juce::Colours::darkgrey.withAlpha(0.45f));
            g.drawHorizontalLine((int)y, area.getX(), area.getRight());
        }
        g.setColour(juce::Colours::grey);
        g.setFont(juce::FontOptions(10.0f));
        for (const float hz : { 100.0f, 1000.0f, 10000.0f })
            g.drawText(hz >= 1000.0f ? juce::String((int)hz / 1000) + " kHz" : juce::String((int)hz) + " Hz", (int)xOf(hz) + 3, (int)area.getBottom() - 14, 50, 12, juce::Justification::centredLeft);
        g.drawText("+12", (int)area.getX() + 3, (int)yOf(12.0f) - 12, 30, 12, juce::Justification::centredLeft);
        g.drawText("-12", (int)area.getX() + 3, (int)yOf(-12.0f) + 1, 30, 12, juce::Justification::centredLeft);
        if (!get) return;
        const auto eq = get();
        // The response.
        juce::Path curve;
        const int w = juce::jmax(2, getWidth() - 2);
        for (int px = 0; px <= w; ++px)
        {
            const float hz = hzAt(1.0f + (float)px);
            const float db = SamplerEngine::eqResponseDb(eq, hz, sampleRate);
            const float y = yOf(juce::jlimit(-18.0f, 18.0f, db));
            if (px == 0) curve.startNewSubPath(1.0f + (float)px, y); else curve.lineTo(1.0f + (float)px, y);
        }
        g.setColour(locked ? juce::Colours::lightgrey : juce::Colours::orange);
        g.strokePath(curve, juce::PathStrokeType(2.0f));
        // The bands' points.
        for (int b = 0; b < 5; ++b)
        {
            const auto& band = eq.bands[b];
            const juce::Point<float> p(xOf(band.hz), yOf(band.gainDb));
            const bool held = b == dragBand;
            g.setColour(!band.on ? juce::Colours::grey.withAlpha(0.6f) : locked ? juce::Colours::lightgrey : held ? juce::Colours::white : juce::Colours::orange);
            g.fillEllipse(p.x - 7.0f, p.y - 7.0f, 14.0f, 14.0f);
            g.setColour(juce::Colours::black);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(juce::String(b + 1), (int)p.x - 7, (int)p.y - 7, 14, 14, juce::Justification::centred);
        }
    }
    void mouseDown(const juce::MouseEvent& e) override
    {
        dragBand = bandAt(e.getPosition().toFloat());
        repaint();
    }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (dragBand < 0 || locked || !get || !set) return;
        auto eq = get();
        auto& band = eq.bands[dragBand];
        band.hz = juce::jlimit(20.0f, 20000.0f, hzAt((float)e.x));
        band.gainDb = juce::jlimit(-18.0f, 18.0f, dbAt((float)e.y));
        set(eq);
        repaint();
    }
    void mouseUp(const juce::MouseEvent&) override { dragBand = -1; repaint(); }
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (locked || !get || !set) return;
        const int b = bandAt(e.getPosition().toFloat());
        if (b < 0) return;
        auto eq = get();
        eq.bands[b].q = juce::jlimit(0.1f, 10.0f, eq.bands[b].q * std::pow(2.0f, wheel.deltaY * 2.0f));
        set(eq);
        repaint();
    }

private:
    int dragBand = -1;
    float xOf(float hz) const noexcept { return 1.0f + (float)(getWidth() - 2) * std::log(juce::jlimit(20.0f, 20000.0f, hz) / 20.0f) / std::log(1000.0f); }
    float hzAt(float x) const noexcept { return 20.0f * std::pow(1000.0f, juce::jlimit(0.0f, 1.0f, (x - 1.0f) / (float)juce::jmax(1, getWidth() - 2))); }
    float yOf(float db) const noexcept { return (float)getHeight() * 0.5f - db * ((float)getHeight() * 0.5f - 10.0f) / 18.0f; }
    float dbAt(float y) const noexcept { return ((float)getHeight() * 0.5f - y) * 18.0f / ((float)getHeight() * 0.5f - 10.0f); }
    int bandAt(juce::Point<float> p) const
    {
        if (!get) return -1;
        const auto eq = get();
        int best = -1; float bestDistance = 14.0f;
        for (int b = 0; b < 5; ++b)
        {
            const float d = p.getDistanceFrom({ xOf(eq.bands[b].hz), yOf(eq.bands[b].gainDb) });
            if (d < bestDistance) { bestDistance = d; best = b; }
        }
        return best;
    }
};

class EffectsContent final : public juce::Component, private juce::Timer
{
public:
    using Module = MainComponent::EffectModule;

    EffectsContent(MainComponent& o, juce::String perspectiveIn, int manualIn)
        : owner(o), perspective(std::move(perspectiveIn)), manual(manualIn)
    {
        getProperties().set("manualChapter", "Effects");
        static const juce::String names[8]{ "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
        routeName = manual >= 0 ? names[juce::jlimit(0, 7, manual)] + " " + perspective : perspective;
        title.setText(manual >= 0 ? "Effects of the route " + routeName : "Effects of the perspective " + perspective + " - every linked route of it", juce::dontSendNotification);
        title.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(title);
        note.setFont(juce::FontOptions(12.5f));
        note.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        note.setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(note);
        // The chain (0.9.101): built from the perspective's order - see rebuildChain().
        rebuildChain();
        // 0.9.90: everything of this row to the default (the user: "a reset button for all the effects in the dialog").
        resetAll.setButtonText("Reset all effects");
        resetAll.setTooltip(manual >= 0 ? "Every module of this route LINKED to the perspective again (the route's own values are dropped)"
            : "Every module of the perspective to its default, every route of it linked again");
        resetAll.getProperties().set("manualChapter", "Effects");
        resetAll.onClick = [this] { owner.resetEffects(perspective, manual); refresh(); };
        addAndMakeVisible(resetAll);
        buildPanels();
        buildPluginPanel();
        addPlugin.setButtonText("+");
        addPlugin.setTooltip(manual >= 0 ? "Plug-ins belong to the perspective: add them in the perspective's effects (the fx button of the perspective's row)."
            : "Add a plug-in (VST3; Audio Units too on macOS) to this perspective's chain - it works on every route of the perspective, right before the IR reverb unless you arrange it elsewhere.");
        addPlugin.setEnabled(manual < 0);
        addPlugin.getProperties().set("manualChapter", "Plug-ins in the chain");
        addPlugin.getProperties().set("testName", "add plugin");
        addPlugin.onClick = [this] { addPluginMenu(); };
        addAndMakeVisible(addPlugin);
        addChildComponent(arranger);
        // 0.9.90: the module shown last is shown again when the dialog is retargeted (the user: "not Division volume every time").
        int wanted = manual >= 0 ? 1 : 0;
        for (size_t i = 0; i < chain.size(); ++i)
            if ((owner.effectsDialogModule < 0 && chain[i].divisionVolume) || (owner.effectsDialogModule >= 0 && !chain[i].divisionVolume && (int)chain[i].module == owner.effectsDialogModule))
                wanted = (int)i;
        select(wanted);
        refresh();
        startTimer(100);
        // ORGAN_TEST_FX_ARRANGE="<id>|<before id>": 1.5 s after the dialog opens the module is moved as a drag does
        // and the arrange mode is left on (for a picture).
        if (const char* env = std::getenv("ORGAN_TEST_FX_ARRANGE"))
        {
            const auto parts = juce::StringArray::fromTokens(juce::String::fromUTF8(env), "|", "");
            juce::Component::SafePointer<EffectsContent> safeThis(this);
            juce::Timer::callAfterDelay(1500, [safeThis, parts]
            {
                if (safeThis == nullptr || parts.size() < 1) return;
                safeThis->owner.moveEffectInChain(safeThis->perspective, parts[0], parts.size() > 1 ? parts[1] : juce::String());
                safeThis->rebuildChain(); safeThis->resized(); safeThis->select(safeThis->selected);
                safeThis->beginArranging();
            });
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        auto titleLine = area.removeFromTop(26);
        resetAll.setBounds(titleLine.removeFromRight(150).reduced(0, 1));
        title.setBounds(titleLine);
        note.setBounds(area.removeFromTop(44));
        area.removeFromTop(6);
        auto list = area.removeFromLeft(214);
        for (int i = 0; i < chainButtons.size(); ++i)
        {
            auto line = list.removeFromTop(34).reduced(0, 3);
            // The ON / OFF switch before the name (the division volume has none: its place stays empty).
            auto switchArea = line.removeFromLeft(44);
            if (auto* s = switchButtons[i]) s->setBounds(switchArea);
            line.removeFromLeft(4);
            chainButtons[i]->setBounds(line);
        }
        addPlugin.setBounds(list.removeFromTop(34).reduced(0, 3).removeFromLeft(44));
        area.removeFromLeft(14);
        for (auto* p : panels) p->setBounds(area);
        arranger.setBounds(getLocalBounds());
    }

private:
    struct ChainItem { Module module; juce::String name; bool divisionVolume; int panel; juce::String id; };   // id: "volume" / "separation" / "eq" (movable), "division", "ir"
    // A button that also answers the right mouse button (the MIDI learn menu of a switch, the chain's own menu).
    struct RightClickButton final : public juce::TextButton
    {
        using juce::TextButton::TextButton;
        std::function<void()> onRightClick;
        double downSinceMs = 0.0;      // a left press began then (the dialog's timer makes two seconds of it the arrange mode)
        bool swallowRelease = false;   // ... and its release is then no click
        bool rightPress = false;
        void mouseDown(const juce::MouseEvent& e) override
        {
            rightPress = e.mods.isPopupMenu() || e.mods.isRightButtonDown();
            if (rightPress) { if (onRightClick) onRightClick(); return; }
            downSinceMs = juce::Time::getMillisecondCounterHiRes();
            juce::TextButton::mouseDown(e);
        }
        void mouseUp(const juce::MouseEvent& e) override
        {
            downSinceMs = 0.0;
            if (rightPress) { rightPress = false; return; }
            if (swallowRelease) { swallowRelease = false; setState(juce::Button::buttonNormal); return; }
            juce::TextButton::mouseUp(e);
        }
    };
    static const char* moduleKey(Module m) { return m == Module::Volume ? "volume" : m == Module::Separation ? "separation" : m == Module::Eq ? "eq" : "ir"; }
    juce::String switchTarget(Module m) const { return "fxon:" + juce::String(manual) + ":" + moduleKey(m) + ":" + perspective; }
    static void styleSwitch(juce::TextButton& b)
    {
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
        b.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
    }
    void setupSwitch(RightClickButton& b, Module m, const juce::String& moduleName)
    {
        styleSwitch(b);
        b.getProperties().set("manualChapter", "Effects");
        b.getProperties().set("testName", "ONOFF " + moduleName);
        b.onClick = [this, m] { owner.setEffectOn(perspective, manual, m, !owner.effectOn(perspective, manual, m)); refresh(); };
        b.onRightClick = [this, m] { owner.showMidiLearnMenu(switchTarget(m)); };
    }
    // The chain's rows in the perspective's order: Division volume (a route), the movable modules, the IR reverb.
    void rebuildChain()
    {
        const juce::String selectedId = (size_t)selected < chain.size() ? chain[(size_t)selected].id : juce::String();
        chain.clear(); chainButtons.clear(); switchButtons.clear();
        const int base = manual >= 0 ? 1 : 0;   // the panels: [division], volume, separation, eq, ir
        if (manual >= 0) chain.push_back({ Module::Volume, "Division volume", true, 0, "division" });
        for (const auto& id : owner.effectChain(perspective))
        {
            if (id == "volume") chain.push_back({ Module::Volume, "Volume", false, base, id });
            else if (id == "separation") chain.push_back({ Module::Separation, "Channel separation", false, base + 1, id });
            else if (id == "eq") chain.push_back({ Module::Eq, "Equalizer", false, base + 2, id });
            else if (auto* slot = owner.pluginSlot(perspective, id.fromFirstOccurrenceOf("vst:", false, false)))
                chain.push_back({ Module::Ir, slot->description.name, false, base + 4, id });   // 0.9.102: a plug-in (its panel is the last one)
        }
        chain.push_back({ Module::Ir, "IR reverb", false, base + 3, "ir" });
        for (size_t i = 0; i < chain.size(); ++i)
        {
            auto* b = new RightClickButton(chain[i].name);
            chainButtons.add(b);
            b->setClickingTogglesState(false);
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
            b->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
            b->setTooltip("The modules in the order the engine applies them. Click one to set it; right-click to arrange the order.");
            b->getProperties().set("manualChapter", chain[i].name == "Division volume" ? "Volume" : chain[i].name);
            b->onClick = [this, i] { select((int)i); };
            b->onRightClick = [this] { chainMenu(); };
            addAndMakeVisible(b);
            if (chain[i].divisionVolume) { switchButtons.add(nullptr); continue; }
            auto* s = new RightClickButton();
            switchButtons.add(s);
            if (isPlugin(i))
            {
                const auto slotId = pluginIdOf(i);
                styleSwitch(*s);
                s->getProperties().set("manualChapter", "Plug-ins in the chain");
                s->getProperties().set("testName", "ONOFF plugin " + chain[i].name);
                s->onClick = [this, slotId] { if (auto* slot = owner.pluginSlot(perspective, slotId)) owner.setPluginOn(perspective, slotId, !slot->on); refresh(); };
                b->getProperties().set("manualChapter", "Plug-ins in the chain");
            }
            else setupSwitch(*s, chain[i].module, chain[i].name);
            addAndMakeVisible(s);
        }
        for (size_t i = 0; i < chain.size(); ++i) if (chain[i].id == selectedId) selected = (int)i;
        arranger.toFront(false);
    }
    bool isPlugin(size_t i) const { return i < chain.size() && chain[i].id.startsWith("vst:"); }
    juce::String pluginIdOf(size_t i) const { return i < chain.size() ? chain[i].id.fromFirstOccurrenceOf("vst:", false, false) : juce::String(); }
    // The "+" under the chain: the effect plug-ins found by the scan, by maker.
    void addPluginMenu()
    {
        juce::PopupMenu menu;
        const auto effects = PluginHost::effects();
        juce::String maker;
        juce::PopupMenu sub;
        int index = 0;
        auto flush = [&] { if (sub.getNumItems() > 0) menu.addSubMenu(maker.isEmpty() ? juce::String("(no maker named)") : maker, sub); sub = juce::PopupMenu(); };
        for (const auto& type : effects)
        {
            if (type.manufacturerName != maker) { flush(); maker = type.manufacturerName; }
            sub.addItem(1000 + index, type.name + "   [" + type.pluginFormatName + "]");
            ++index;
        }
        flush();
        if (effects.isEmpty()) menu.addItem(1, "no plug-in found yet - scan first", false);
        menu.addSeparator();
        menu.addItem(2, PluginHost::scanner().running() ? "Scanning now: " + PluginHost::scanner().progress() : juce::String("Scan for plug-ins (new and changed files)"), !PluginHost::scanner().running());
        menu.addItem(3, "Plug-in folders and the list (Configuration Editor)...");
        menu.addItem(4, "Manual: Plug-ins in the chain");
        juce::Component::SafePointer<EffectsContent> safeThis(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addPlugin), [safeThis, effects](int result)
        {
            if (safeThis == nullptr) return;
            if (result == 2) safeThis->owner.startPluginScan(false, 8, false);
            else if (result == 3) safeThis->owner.showPluginSettings();
            else if (result == 4) safeThis->owner.showManualSection("Plug-ins in the chain");
            else if (result >= 1000 && result - 1000 < effects.size())
            {
                const auto id = safeThis->owner.addPlugin(safeThis->perspective, effects[result - 1000]);
                if (id.isEmpty()) return;
                safeThis->rebuildChain(); safeThis->resized();
                for (size_t i = 0; i < safeThis->chain.size(); ++i) if (safeThis->chain[i].id == "vst:" + id) safeThis->select((int)i);
            }
        });
    }
    void chainMenu()
    {
        CrashHandler::note("Menu: the effects chain of " + perspective);
        juce::PopupMenu menu;
        menu.addItem(1, "Arrange the chain (drag the modules and plug-ins to their places)...");
        menu.addSeparator();
        menu.addItem(2, "Manual: Effects");
        juce::Component::SafePointer<EffectsContent> safeThis(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withMousePosition(), [safeThis](int result)
        {
            if (safeThis == nullptr) return;
            if (result == 1) safeThis->beginArranging();
            else if (result == 2) safeThis->owner.showManualSection("Effects");
        });
    }
    void beginArranging() { arranger.setVisible(true); arranger.toFront(true); arranger.repaint(); CrashHandler::note("Effects: arranging the chain of " + perspective); }
    void endArranging() { arranger.setVisible(false); }
    // The arrange mode of the chain (as on the top bar): a layer over the dialog - the movable modules framed, a
    // press takes one, the orange line shows where it lands, a click anywhere else ends the mode. The order is the
    // PERSPECTIVE's: every route of it shares it. Division volume stays first, the IR reverb last.
    struct Arranger final : public juce::Component, private juce::Timer
    {
        explicit Arranger(EffectsContent& c) : content(c) { startTimerHz(25); }
        void timerCallback() override   // the dashes travel: the chain's column alone is painted anew
        {
            juce::Rectangle<int> all;
            for (int i = 0; i < content.chainButtons.size(); ++i) { const auto r = rowOf(i); all = all.isEmpty() ? r : all.getUnion(r); }
            if (!all.isEmpty()) repaint(all.expanded(8));
        }
        bool movable(int i) const { const auto& id = content.chain[(size_t)i].id; return id != "division" && id != "ir"; }
        juce::Rectangle<int> rowOf(int i) const
        {
            auto r = content.chainButtons[i]->getBounds();
            if (auto* s = content.switchButtons[i]) r = r.getUnion(s->getBounds());
            return r;
        }
        void paint(juce::Graphics& g) override
        {
            const auto orange = juce::Colours::orange;
            juce::Rectangle<int> all;
            for (int i = 0; i < content.chainButtons.size(); ++i)
            {
                const auto r = rowOf(i);
                all = all.isEmpty() ? r : all.getUnion(r);
                if (!movable(i)) continue;
                ArrangeFrame::takeable(g, r.toFloat().expanded(2.0f), ArrangeFrame::phase(), ArrangeFrame::kThickness, i == dragIndex ? 0.25f : 0.8f);   // the frame of every arrange mode
            }
            const juce::Rectangle<int> note(all.getX(), all.getBottom() + 10, juce::jmax(all.getWidth(), 214), 88);
            g.setColour(juce::Colours::black.withAlpha(0.85f));
            g.fillRoundedRectangle(note.toFloat(), 5.0f);
            g.setColour(orange);
            g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            g.drawFittedText("Drag a framed module to its place. Division volume stays first, the IR reverb last. The order is the perspective's - all its routes share it. Click anywhere else to finish.",
                note.reduced(8, 4), juce::Justification::centredLeft, 6);
            if (dragIndex >= 0 && insertY >= 0) ArrangeFrame::landingLine(g, { all.getX() - 3, insertY - 1, all.getWidth() + 6, 3 });
        }
        void mouseDown(const juce::MouseEvent& e) override
        {
            dragIndex = -1; insertY = -1; moved = false;
            for (int i = 0; i < content.chainButtons.size(); ++i)
                if (movable(i) && rowOf(i).contains(e.getPosition())) { dragIndex = i; repaint(); return; }
            juce::Component::SafePointer<EffectsContent> safe(&content);
            juce::MessageManager::callAsync([safe] { if (safe != nullptr) safe->endArranging(); });
        }
        void mouseDrag(const juce::MouseEvent& e) override
        {
            if (dragIndex < 0) return;
            beforeId.clear(); insertY = -1;
            int lastBottom = -1;
            for (int i = 0; i < content.chainButtons.size(); ++i)
            {
                if (!movable(i) || i == dragIndex) continue;
                const auto r = rowOf(i);
                if (beforeId.isEmpty() && e.y < r.getCentreY()) { beforeId = content.chain[(size_t)i].id; insertY = r.getY() - 3; }
                lastBottom = r.getBottom() + 3;
            }
            if (beforeId.isEmpty()) insertY = lastBottom;
            moved = moved || e.getDistanceFromDragStart() > 5;
            repaint();
        }
        void mouseUp(const juce::MouseEvent&) override
        {
            if (dragIndex >= 0 && moved)
            {
                const auto id = content.chain[(size_t)dragIndex].id;
                const auto before = beforeId;
                juce::Component::SafePointer<EffectsContent> safe(&content);
                juce::MessageManager::callAsync([safe, id, before]   // after the mouse event: the rows are rebuilt
                {
                    if (safe == nullptr) return;
                    safe->owner.moveEffectInChain(safe->perspective, id, before);
                    safe->rebuildChain(); safe->resized(); safe->select(safe->selected);
                });
            }
            dragIndex = -1; insertY = -1; beforeId.clear(); moved = false;
            repaint();
        }
        EffectsContent& content;
        int dragIndex = -1, insertY = -1;
        juce::String beforeId;
        bool moved = false;
    };
    Arranger arranger{ *this };
    juce::OwnedArray<RightClickButton> switchButtons;   // one per chain row (nullptr for the division volume)
    MainComponent& owner;
    const juce::String perspective;
    const int manual;   // -1 = the perspective's dialog
    juce::String routeName;
    juce::Label title, note;
    juce::TextButton resetAll;   // 0.9.90
    std::vector<ChainItem> chain;
    juce::OwnedArray<RightClickButton> chainButtons;
    juce::TextButton addPlugin;        // 0.9.102: "+" under the chain
    juce::String selectedPlugin;       // the slot id the plug-in panel shows
    int statusTick = 0;
    juce::OwnedArray<juce::Component> panels;
    int selected = 0;

    // ---- the model ----
    std::string routeKey() const { return MainComponent::perspectiveRouteKey(juce::jmax(0, manual), perspective); }
    MainComponent::ManualConfig::PerspectiveAudioConfig& routeConfig() { return owner.manualConfigs[(size_t)juce::jmax(0, manual)].perspectiveAudio[routeKey()]; }
    MainComponent::PerspectiveEffects& perspectiveConfig() { return owner.effectsOfPerspective(perspective); }
    bool linked(Module m) { return manual >= 0 && owner.routeEffectLinked(manual, perspective, m); }
    float volumeDb() { return manual >= 0 ? routeConfig().volumeDb : perspectiveConfig().volumeDb; }
    float separation() { return manual >= 0 ? routeConfig().channelSeparation : perspectiveConfig().channelSeparation; }
    SamplerEngine::EqSettings eq() { return manual >= 0 ? routeConfig().eq : perspectiveConfig().eq; }
    juce::String irPath() { return manual >= 0 ? routeConfig().reverbIrPath : perspectiveConfig().reverbIrPath; }
    float wet() { return manual >= 0 ? routeConfig().reverbWet : perspectiveConfig().reverbWet; }
    SamplerEngine::ReverbTone tone() { return manual >= 0 ? routeConfig().reverbTone : perspectiveConfig().reverbTone; }
    // A change made in this dialog: a route's own value (its sliders are enabled only while not linked) is applied
    // and saved; a perspective's value goes to every linked route.
    void changed(Module m)
    {
        if (manual >= 0) { owner.applyRouteEffect(manual, perspective, m); owner.saveSettings(); }
        else owner.perspectiveEffectChanged(perspective, m);
    }
    juce::String midiTarget(const juce::String& what)
    {
        if (manual >= 0)
        {
            if (what == "volume") return "slider:perspective:" + juce::String(manual) + ":" + perspective;
            if (what == "separation") return "slider:separation:" + juce::String(manual) + ":" + perspective;
            if (what == "reverb") return "slider:reverb:" + juce::String(manual) + ":" + perspective;
            return "slider:eq:" + juce::String(manual) + ":" + what + ":" + perspective;   // what = "<band>:<param>"
        }
        if (what == "volume" || what == "separation" || what == "reverb") return "slider:pfx:" + what + ":" + perspective;
        return "slider:eq:-1:" + what + ":" + perspective;
    }

    // ---- the panels ----
    // The header of a module panel: its name, LINKED (a route) or the linked count (a perspective).
    struct Header final : public juce::Component
    {
        juce::Label name, info;
        juce::TextButton linkedButton{ "LINKED" };
        juce::TextButton openButton{ "Open a route set apart..." };
        juce::TextButton resetButton{ "Reset" };   // 0.9.90: this module to the default
        RightClickButton onButton;                  // 0.9.101: ON / OFF of the module (the same switch as in the chain)
        void resized() override
        {
            auto r = getLocalBounds();
            name.setBounds(r.removeFromLeft(onButton.isVisible() ? 172 : 220));
            if (onButton.isVisible()) { onButton.setBounds(r.removeFromLeft(44).reduced(0, 3)); r.removeFromLeft(4); }
            if (linkedButton.isVisible()) linkedButton.setBounds(r.removeFromLeft(90).reduced(0, 3));
            resetButton.setBounds(r.removeFromRight(70).reduced(0, 3));
            r.removeFromRight(8);
            if (openButton.isVisible()) openButton.setBounds(r.removeFromRight(200).reduced(0, 3));
            info.setBounds(r.reduced(8, 0));
        }
    };
    std::vector<Header*> headers;   // one per panel (owned by the panels)

    Header* makeHeader(juce::Component& panel, const juce::String& moduleName, Module m, bool divisionVolume)
    {
        auto* h = new Header();
        panel.addAndMakeVisible(h);
        h->name.setText(moduleName, juce::dontSendNotification);
        h->name.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        h->addAndMakeVisible(h->name);
        h->info.setFont(juce::FontOptions(12.5f));
        h->info.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        h->addAndMakeVisible(h->info);
        h->linkedButton.setClickingTogglesState(true);
        h->linkedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
        h->linkedButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        h->linkedButton.getProperties().set("manualChapter", "LINKED");
        h->linkedButton.getProperties().set("testName", "LINKED " + moduleName);   // ORGAN_TEST_MOUSE ("LINKED Volume"); ORGAN_TEST_CLICK by text finds a hidden one first
        if (!divisionVolume)   // the division volume has no LINKED (its header's button stays hidden and inert)
            h->linkedButton.onClick = [this, m, h]
            {
                owner.setRouteEffectLinked(manual, perspective, m, h->linkedButton.getToggleState());
                refresh();
            };
        h->linkedButton.setVisible(manual >= 0 && !divisionVolume);
        h->addChildComponent(h->linkedButton);
        h->onButton.setVisible(!divisionVolume);
        if (!divisionVolume) setupSwitch(h->onButton, m, moduleName + " (card)");
        h->addChildComponent(h->onButton);
        h->openButton.setVisible(manual < 0);
        h->openButton.getProperties().set("manualChapter", "LINKED");
        h->openButton.onClick = [this, m]
        {
            juce::StringArray apart;
            owner.routesLinked(perspective, m, &apart);
            juce::PopupMenu menu;
            if (apart.isEmpty()) menu.addItem(1, "every route follows this perspective", false);
            for (int i = 0; i < apart.size(); ++i) menu.addItem(100 + i, apart[i]);
            juce::Component::SafePointer<MainComponent> target(&owner);
            const juce::String p = perspective;
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this), [target, p, apart](int result)
            {
                if (target == nullptr || result < 100 || result - 100 >= apart.size()) return;
                static const juce::String names[8]{ "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
                int m = -1;
                for (int i = 0; i < 8; ++i) if (apart[result - 100].startsWith(names[i] + " ")) m = i;
                if (m >= 0) juce::MessageManager::callAsync([target, p, m] { if (target != nullptr) target->showEffectsDialog(p, m); });   // after the menu: the window is remade
            });
        };
        h->addChildComponent(h->openButton);
        // 0.9.90: Reset - a route's module LINKED again (the division volume: 0 dB), a perspective's module to its default.
        h->resetButton.setTooltip(divisionVolume ? "The division volume back to 0 dB"
            : manual >= 0 ? "This module LINKED to the perspective again (the route's own values are dropped)" : "This module to its default; every route of the perspective follows it again");
        h->resetButton.getProperties().set("manualChapter", "Effects");
        h->resetButton.getProperties().set("testName", "Reset " + moduleName);
        h->resetButton.onClick = [this, m, divisionVolume]
        {
            if (divisionVolume)
            {
                auto& config = owner.manualConfigs[(size_t)juce::jmax(0, manual)];
                config.divisionVolumeDb = 0.0f;
                owner.engine.setManualDivisionGain(manual, 1.0f);
                owner.saveSettings();
            }
            else owner.resetEffectModule(perspective, manual, m);
            refresh();
        };
        h->addAndMakeVisible(h->resetButton);
        headers.push_back(h);
        return h;
    }
    static void setupSlider(LearnableSlider& s, MainComponent& o, double minimum, double maximum, double step, const juce::String& suffix, double defaultValue, const juce::String& target, const juce::String& chapter)
    {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 78, 22);
        s.setRange(minimum, maximum, step);
        s.setTextValueSuffix(suffix);
        s.setScrollWheelEnabled(true);
        s.setDoubleClickReturnValue(true, defaultValue);
        s.getProperties().set("midiTarget", target);
        s.getProperties().set("manualChapter", chapter);
        s.addMouseListener(&o, false);
    }

    // Division volume (a route's dialog only): the level of the whole division - every perspective of it.
    struct DivisionVolumePanel final : public juce::Component
    {
        juce::Label label; LearnableSlider slider; Header* header = nullptr;
        void resized() override
        {
            auto r = getLocalBounds();
            if (header) header->setBounds(r.removeFromTop(32));
            r.removeFromTop(8);
            auto line = r.removeFromTop(34);
            label.setBounds(line.removeFromLeft(170));
            slider.setBounds(line.reduced(0, 4));
        }
    };
    struct VolumePanel final : public juce::Component
    {
        juce::Label label; LearnableSlider slider; Header* header = nullptr;
        void resized() override
        {
            auto r = getLocalBounds();
            if (header) header->setBounds(r.removeFromTop(32));
            r.removeFromTop(8);
            auto line = r.removeFromTop(34);
            label.setBounds(line.removeFromLeft(170));
            slider.setBounds(line.reduced(0, 4));
        }
    };
    struct EqPanel final : public juce::Component
    {
        EqGraph graph; Header* header = nullptr;
        struct BandRow { juce::ToggleButton on; juce::Label name; LearnableSlider hz, gain, q; };
        BandRow rows[5];
        void resized() override
        {
            auto r = getLocalBounds();
            if (header) header->setBounds(r.removeFromTop(32));
            r.removeFromTop(6);
            graph.setBounds(r.removeFromTop(juce::jmax(120, r.getHeight() - 5 * 34 - 10)));
            r.removeFromTop(8);
            for (auto& row : rows)
            {
                auto line = r.removeFromTop(34).reduced(0, 4);
                row.on.setBounds(line.removeFromLeft(26));
                row.name.setBounds(line.removeFromLeft(30));   // 0.9.90: "1".."5"
                const int w = (line.getWidth() - 16) / 3;
                row.hz.setBounds(line.removeFromLeft(w)); line.removeFromLeft(8);
                row.gain.setBounds(line.removeFromLeft(w)); line.removeFromLeft(8);
                row.q.setBounds(line);
            }
        }
    };
    struct IrPanel final : public juce::Component
    {
        Header* header = nullptr;
        juce::TextButton load{ "Load IR..." }, clear{ "Clear" };
        juce::Label file, wetLabel, toneNote;
        LearnableSlider wet;
        juce::Label toneLabels[6]; juce::Slider tones[6];
        void resized() override
        {
            auto r = getLocalBounds();
            if (header) header->setBounds(r.removeFromTop(32));
            r.removeFromTop(8);
            auto line = r.removeFromTop(30);
            load.setBounds(line.removeFromLeft(110).reduced(0, 2)); line.removeFromLeft(6);
            clear.setBounds(line.removeFromLeft(70).reduced(0, 2)); line.removeFromLeft(10);
            file.setBounds(line);
            line = r.removeFromTop(34);
            wetLabel.setBounds(line.removeFromLeft(170));
            wet.setBounds(line.reduced(0, 4));
            toneNote.setBounds(r.removeFromTop(40));
            for (int i = 0; i < 6; ++i)
            {
                line = r.removeFromTop(32);
                toneLabels[i].setBounds(line.removeFromLeft(170));
                tones[i].setBounds(line.reduced(0, 3));
            }
        }
    };
    DivisionVolumePanel* divisionPanel = nullptr;
    VolumePanel* volumePanel = nullptr;
    VolumePanel* separationPanel = nullptr;
    EqPanel* eqPanel = nullptr;
    IrPanel* irPanel = nullptr;

    void buildPanels()
    {
        if (manual >= 0)
        {
            auto* p = new DivisionVolumePanel();
            panels.add(p); divisionPanel = p;
            p->header = makeHeader(*p, "Division volume", Module::Volume, true);
            p->header->info.setText("the level of the whole division (every perspective of it) - a division's own, never a perspective's", juce::dontSendNotification);
            p->label.setText("Division volume", juce::dontSendNotification);
            p->addAndMakeVisible(p->label);
            setupSlider(p->slider, owner, -60.0, 12.0, 0.1, " dB", 0.0, "slider:division:" + juce::String(manual) + ":volume", "Volume");
            p->slider.setTooltip("The level of the whole division (-60 to +12 dB), for balancing the divisions against each other. Right-click: MIDI learn.");
            p->slider.onValueChange = [this]
            {
                auto& config = owner.manualConfigs[(size_t)manual];
                config.divisionVolumeDb = (float)divisionPanel->slider.getValue();
                owner.engine.setManualDivisionGain(manual, juce::Decibels::decibelsToGain(config.divisionVolumeDb));
                owner.saveSettings();
            };
            p->addAndMakeVisible(p->slider);
            addChildComponent(p);
        }
        {
            auto* p = new VolumePanel();
            panels.add(p); volumePanel = p;
            p->header = makeHeader(*p, "Volume", Module::Volume, false);
            p->label.setText("Volume", juce::dontSendNotification);
            p->addAndMakeVisible(p->label);
            setupSlider(p->slider, owner, -31.0, 6.0, 0.1, " dB", 0.0, midiTarget("volume"), "Volume");
            p->slider.textFromValueFunction = [](double value) { return value < -30.5 ? juce::String("-inf") : juce::String(value, 1); };
            p->slider.setTooltip("The level of this " + juce::String(manual >= 0 ? "route" : "perspective") + " (-30 to +6 dB; the far left is silence). Right-click: MIDI learn.");
            p->slider.onValueChange = [this]
            {
                auto db = (float)volumePanel->slider.getValue();
                if (db > -31.0f && db < -30.0f) { db = db < -30.5f ? -31.0f : -30.0f; volumePanel->slider.setValue(db, juce::dontSendNotification); }
                if (manual >= 0) routeConfig().volumeDb = db; else perspectiveConfig().volumeDb = db;
                changed(Module::Volume);
            };
            p->addAndMakeVisible(p->slider);
            addChildComponent(p);
        }
        {
            auto* p = new VolumePanel();
            panels.add(p); separationPanel = p;
            p->header = makeHeader(*p, "Channel separation", Module::Separation, false);
            p->label.setText("Channel separation", juce::dontSendNotification);
            p->addAndMakeVisible(p->label);
            setupSlider(p->slider, owner, 0.0, 100.0, 1.0, " %", 100.0, midiTarget("separation"), "Channel separation");
            p->slider.setTooltip("100 % = every pipe only on its own channel pair (mono pipes are spread over the pairs); lower, every channel also carries "
                "that share of the others - the stereo width narrows, 0 % = the same sum everywhere. Right-click: MIDI learn.");
            p->slider.onValueChange = [this]
            {
                const float v = (float)(separationPanel->slider.getValue() / 100.0);
                if (manual >= 0) routeConfig().channelSeparation = v; else perspectiveConfig().channelSeparation = v;
                changed(Module::Separation);
            };
            p->addAndMakeVisible(p->slider);
            addChildComponent(p);
        }
        {
            auto* p = new EqPanel();
            panels.add(p); eqPanel = p;
            p->header = makeHeader(*p, "Equalizer", Module::Eq, false);
            p->graph.get = [this] { return eq(); };
            p->graph.set = [this](const SamplerEngine::EqSettings& e) { if (manual >= 0) routeConfig().eq = e; else perspectiveConfig().eq = e; changed(Module::Eq); refreshEqRows(); };
            p->graph.setTooltip("The response of the five bands. Drag a point: its frequency and gain; the wheel over a point: its Q (width).");
            p->graph.getProperties().set("manualChapter", "Equalizer");
            p->addAndMakeVisible(p->graph);
            // 0.9.90: the rows are numbered 1-5 like the points of the graph (the user: "the numbers on the thumbs did not
            // match the rows"); the band's kind is in the tooltip.
            static const char* const bandKinds[5]{ "Low shelf", "Peak", "Peak", "Peak", "High shelf" };
            for (int b = 0; b < 5; ++b)
            {
                auto& row = p->rows[b];
                row.on.setTooltip("Band " + juce::String(b + 1) + " (" + bandKinds[b] + ") on or off");
                row.on.getProperties().set("manualChapter", "Equalizer");
                row.on.onClick = [this, b] { auto e = eq(); e.bands[b].on = eqPanel->rows[b].on.getToggleState(); storeEq(e); };
                p->addAndMakeVisible(row.on);
                row.name.setText(juce::String(b + 1), juce::dontSendNotification);
                row.name.setTooltip(juce::String(bandKinds[b]) + " - point " + juce::String(b + 1) + " of the graph");
                row.name.setJustificationType(juce::Justification::centred);
                p->addAndMakeVisible(row.name);
                setupSlider(row.hz, owner, 20.0, 20000.0, 1.0, " Hz", (double)SamplerEngine::EqSettings().bands[b].hz, midiTarget(juce::String(b) + ":hz"), "Equalizer");
                row.hz.setSkewFactorFromMidPoint(630.0);
                row.hz.setTooltip("The band's frequency. Right-click: MIDI learn.");
                row.hz.onValueChange = [this, b] { auto e = eq(); e.bands[b].hz = (float)eqPanel->rows[b].hz.getValue(); storeEq(e); };
                p->addAndMakeVisible(row.hz);
                setupSlider(row.gain, owner, -18.0, 18.0, 0.1, " dB", 0.0, midiTarget(juce::String(b) + ":gain"), "Equalizer");
                row.gain.setTooltip("The band's gain (+-18 dB). Right-click: MIDI learn.");
                row.gain.onValueChange = [this, b] { auto e = eq(); e.bands[b].gainDb = (float)eqPanel->rows[b].gain.getValue(); storeEq(e); };
                p->addAndMakeVisible(row.gain);
                setupSlider(row.q, owner, 0.1, 10.0, 0.01, " Q", (double)SamplerEngine::EqSettings().bands[b].q, midiTarget(juce::String(b) + ":q"), "Equalizer");
                row.q.setSkewFactorFromMidPoint(1.0);
                row.q.setTooltip("The band's width (a peak) or slope (a shelf): higher = narrower. Right-click: MIDI learn.");
                row.q.onValueChange = [this, b] { auto e = eq(); e.bands[b].q = (float)eqPanel->rows[b].q.getValue(); storeEq(e); };
                p->addAndMakeVisible(row.q);
            }
            addChildComponent(p);
        }
        {
            auto* p = new IrPanel();
            panels.add(p); irPanel = p;
            p->header = makeHeader(*p, "IR reverb", Module::Ir, false);
            p->load.setTooltip("An impulse response (mono, stereo or true stereo WAV) convolved with this " + juce::String(manual >= 0 ? "route" : "perspective"));
            p->load.getProperties().set("manualChapter", "IR reverb");
            p->load.onClick = [this]
            {
                juce::Component::SafePointer<MainComponent> target(&owner);
                const int m = manual; const juce::String pn = perspective;
                juce::MessageManager::callAsync([target, m, pn] { if (target == nullptr) return; if (m >= 0) target->chooseReverbIr(m, pn); else target->chooseReverbIrForPerspective(pn); });
            };
            p->addAndMakeVisible(p->load);
            p->clear.setTooltip("No reverb on this " + juce::String(manual >= 0 ? "route" : "perspective"));
            p->clear.onClick = [this]
            {
                if (manual >= 0) routeConfig().reverbIrPath.clear(); else perspectiveConfig().reverbIrPath.clear();
                changed(Module::Ir);
            };
            p->addAndMakeVisible(p->clear);
            p->file.setFont(juce::FontOptions(13.0f));
            p->addAndMakeVisible(p->file);
            p->wetLabel.setText("Dry / wet", juce::dontSendNotification);
            p->addAndMakeVisible(p->wetLabel);
            setupSlider(p->wet, owner, 0.0, 100.0, 1.0, " %", 30.0, midiTarget("reverb"), "IR reverb");
            p->wet.setTooltip("0 % = dry only (the convolution is skipped), 100 % = reverb only. Right-click: MIDI learn.");
            p->wet.onValueChange = [this]
            {
                const float v = (float)(irPanel->wet.getValue() / 100.0);
                if (manual >= 0) { routeConfig().reverbWet = v; owner.engine.setPerspectiveReverbWet(manual, perspective, v); owner.saveSettings(); }
                else { perspectiveConfig().reverbWet = v; changed(Module::Ir); }
            };
            p->addAndMakeVisible(p->wet);
            p->toneNote.setText("The wet (reverberated) signal: the cuts grow with the dry/wet ratio (nothing is cut at 0 % wet, the set frequencies apply at 100 %); "
                "Decay shortens the IR, Fade-out lets the shortened IR die away over that share of its end. An IR costs the DSP a fixed amount per route.", juce::dontSendNotification);
            p->toneNote.setFont(juce::FontOptions(12.0f));
            p->toneNote.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
            p->toneNote.setJustificationType(juce::Justification::topLeft);
            p->addAndMakeVisible(p->toneNote);
            static const char* const toneNames[6]{ "High cut at 100 % wet", "Low cut at 100 % wet", "Pre-delay", "Wet volume", "Decay (IR length)", "Fade-out (of used length)" };
            static const double toneRanges[6][4]{ { 500.0, 20000.0, 50.0, 2000.0 }, { 0.0, 500.0, 5.0, 195.0 }, { 0.0, 250.0, 1.0, 0.0 }, { -30.0, 6.0, 0.5, -4.0 }, { 10.0, 100.0, 1.0, 100.0 }, { 0.0, 100.0, 1.0, 0.0 } };
            static const char* const toneSuffixes[6]{ " Hz", " Hz", " ms", " dB", " %", " %" };
            for (int i = 0; i < 6; ++i)
            {
                p->toneLabels[i].setText(toneNames[i], juce::dontSendNotification);
                p->addAndMakeVisible(p->toneLabels[i]);
                auto& s = p->tones[i];
                s.setSliderStyle(juce::Slider::LinearHorizontal);
                s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 78, 22);
                s.setRange(toneRanges[i][0], toneRanges[i][1], toneRanges[i][2]);
                s.setTextValueSuffix(toneSuffixes[i]);
                s.setDoubleClickReturnValue(true, toneRanges[i][3]);
                s.setScrollWheelEnabled(true);
                s.getProperties().set("manualChapter", "IR reverb");
                if (i == 0) s.setSkewFactorFromMidPoint(3000.0);
                s.onValueChange = [this, i]
                {
                    auto t = tone();
                    const double v = irPanel->tones[i].getValue();
                    switch (i)
                    {
                        case 0: t.highCutHz = (float)v; break;
                        case 1: t.lowCutHz = (float)v; break;
                        case 2: t.preDelayMs = (float)v; break;
                        case 3: t.wetDb = (float)v; break;
                        case 4: t.decayPercent = (float)v; t.fadePercent = v >= 99.9 ? 0.0f : (float)juce::jmax(5.0, 100.0 - v); break;   // the fade follows the decay (0.8.xx rule of the tone dialog)
                        default: t.fadePercent = (float)v; break;
                    }
                    if (manual >= 0) { routeConfig().reverbTone = t; owner.engine.setPerspectiveReverbTone(manual, perspective, t); owner.saveSettings(); }
                    else { perspectiveConfig().reverbTone = t; changed(Module::Ir); }
                };
                p->addAndMakeVisible(s);
            }
            addChildComponent(p);
        }
    }
    // 0.9.102: the card of a plug-in - its name and maker, the ON / OFF switch, its own editor (always a window of
    // its own), Remove, and what it does right now.
    struct PluginPanel final : public juce::Component
    {
        juce::Label name, maker, status, warning, note;
        RightClickButton onButton;
        juce::TextButton open{ "Open editor" }, remove{ "Remove" };
        void resized() override
        {
            auto r = getLocalBounds();
            auto head = r.removeFromTop(30);
            name.setBounds(head.removeFromLeft(juce::jmin(200, head.getWidth() / 3)));
            onButton.setBounds(head.removeFromLeft(44).reduced(0, 3));
            head.removeFromLeft(8);
            remove.setBounds(head.removeFromRight(90).reduced(0, 3));
            head.removeFromRight(8);
            open.setBounds(head.removeFromRight(120).reduced(0, 3));
            maker.setBounds(head.reduced(8, 0));
            r.removeFromTop(10);
            // One line or two: the warning stands right under the status, no empty line between them.
            // (A label squeezes a text too wide down to 0.7 of its width before it wraps it - the line pictured was
            // such a squeezed one.)
            const bool twoLines = juce::GlyphArrangement::getStringWidth(status.getFont(), status.getText()) * 0.7f > (float)(r.getWidth() - 10);
            status.setBounds(r.removeFromTop(warning.getText().isNotEmpty() && !twoLines ? 20 : 44));
            if (warning.getText().isNotEmpty()) { r.removeFromTop(2); warning.setBounds(r.removeFromTop(warning.getText().containsChar('\n') ? 40 : 22)); }
            else warning.setBounds({});
            r.removeFromTop(6);
            note.setBounds(r.removeFromTop(90));
        }
    };
    PluginPanel* pluginPanel = nullptr;
    void buildPluginPanel()
    {
        auto* p = new PluginPanel();
        panels.add(p); pluginPanel = p;
        p->name.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        p->addAndMakeVisible(p->name);
        p->warning.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        p->warning.setColour(juce::Label::textColourId, juce::Colours::orange);
        p->warning.setJustificationType(juce::Justification::topLeft);
        p->warning.getProperties().set("testName", "plugin warning");
        p->addAndMakeVisible(p->warning);
        for (auto* l : { &p->maker, &p->status, &p->note })
        {
            l->setFont(juce::FontOptions(12.5f));
            l->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
            l->setJustificationType(juce::Justification::topLeft);
            p->addAndMakeVisible(*l);
        }
        p->maker.setJustificationType(juce::Justification::centredLeft);
        p->status.setColour(juce::Label::textColourId, juce::Colours::white);
        p->note.setText(manual >= 0 ? "A plug-in belongs to the perspective: it works on every route of it with the same setting. Switch, set and remove it in the perspective's effects."
            : "ON: the plug-in works on every route of this perspective (an instance for each route, all with this one setting). OFF: the sound passes untouched and the setting is kept. "
              "Its place in the chain: right-click the chain, Arrange the chain. The plug-in is foreign code - one that takes too long is switched OFF by the program, "
              "and after a crash the next start loads every plug-in switched OFF.", juce::dontSendNotification);
        styleSwitch(p->onButton);
        p->onButton.getProperties().set("testName", "ONOFF plugin (card)");
        p->onButton.onClick = [this] { if (auto* slot = owner.pluginSlot(perspective, selectedPlugin)) owner.setPluginOn(perspective, selectedPlugin, !slot->on); refresh(); };
        p->addAndMakeVisible(p->onButton);
        p->open.setTooltip("The plug-in's own window (always a window of its own). What you set there goes to every route of the perspective.");
        p->open.getProperties().set("testName", "plugin editor");
        p->open.onClick = [this] { owner.showPluginEditor(perspective, selectedPlugin); };
        p->addAndMakeVisible(p->open);
        p->remove.setTooltip("Takes the plug-in out of this perspective's chain; its setting is dropped.");
        p->remove.getProperties().set("testName", "plugin remove");
        p->remove.onClick = [this]
        {
            const auto id = selectedPlugin;
            juce::Component::SafePointer<EffectsContent> safeThis(this);
            juce::MessageManager::callAsync([safeThis, id]   // after the click: the rows are rebuilt
            {
                if (safeThis == nullptr) return;
                safeThis->owner.removePlugin(safeThis->perspective, id);
                safeThis->rebuildChain(); safeThis->resized(); safeThis->select(juce::jmax(0, safeThis->selected - 1));
            });
        };
        p->addAndMakeVisible(p->remove);
        for (auto* c : std::initializer_list<juce::Component*>{ p, &p->open, &p->remove, &p->status }) c->getProperties().set("manualChapter", "Plug-ins in the chain");
        addChildComponent(p);
    }
    void storeEq(const SamplerEngine::EqSettings& e)
    {
        if (manual >= 0) routeConfig().eq = e; else perspectiveConfig().eq = e;
        changed(Module::Eq);
        eqPanel->graph.repaint();
    }
    void select(int index)
    {
        selected = juce::jlimit(0, (int)chain.size() - 1, index);
        if ((size_t)selected < chain.size() && !isPlugin((size_t)selected)) owner.effectsDialogModule = chain[(size_t)selected].divisionVolume ? -1 : (int)chain[(size_t)selected].module;   // 0.9.90: remembered for the next target
        const int shownPanel = (size_t)selected < chain.size() ? chain[(size_t)selected].panel : 0;
        selectedPlugin = isPlugin((size_t)selected) ? pluginIdOf((size_t)selected) : juce::String();
        for (int i = 0; i < panels.size(); ++i) panels[i]->setVisible(i == shownPanel);
        for (int i = 0; i < chainButtons.size(); ++i) chainButtons[i]->setToggleState(i == selected, juce::dontSendNotification);
        refresh();
    }
    void refreshEqRows()
    {
        const auto e = eq();
        for (int b = 0; b < 5; ++b)
        {
            auto& row = eqPanel->rows[b];
            row.on.setToggleState(e.bands[b].on, juce::dontSendNotification);
            if (!row.hz.isMouseButtonDown()) row.hz.setValue(e.bands[b].hz, juce::dontSendNotification);
            if (!row.gain.isMouseButtonDown()) row.gain.setValue(e.bands[b].gainDb, juce::dontSendNotification);
            if (!row.q.isMouseButtonDown()) row.q.setValue(e.bands[b].q, juce::dontSendNotification);
        }
    }
    // Everything from the model (10 Hz): the values, the LINKED buttons, the locked sliders, the counts.
    void refresh()
    {
        if (manual < 0)
        {
            int routes = 0;
            for (int m = 0; m <= owner.manualCount && m < SamplerEngine::maxManuals; ++m)
                for (const auto& p : owner.engine.getPerspectivesForManual(m)) if (p.equalsIgnoreCase(perspective)) ++routes;
            note.setText("What you set here is what every LINKED route of the perspective plays - each route on its own (" + juce::String(routes) + " route(s): "
                + juce::String(routes) + " times the DSP of an effect). A route set apart (its LINKED off) keeps its own values; open it from the module's header.", juce::dontSendNotification);
        }
        else note.setText("The effects of this route in the order the engine applies them. LINKED (orange) = the module follows the perspective \"" + perspective
            + "\" and its sliders are locked; press it to set the route apart and the sliders are yours; press it again and the perspective's values are back.", juce::dontSendNotification);
        // 0.9.101: the ON / OFF switches (the chain's and the cards'): a linked route shows the perspective's and is locked.
        auto showSwitch = [this](juce::TextButton& b, Module m)
        {
            const bool on = owner.effectOn(perspective, manual, m);
            const bool isLinked = linked(m);
            if (b.getToggleState() != on) b.setToggleState(on, juce::dontSendNotification);
            if (b.getButtonText() != (on ? "ON" : "OFF")) b.setButtonText(on ? "ON" : "OFF");
            if (b.isEnabled() == isLinked) b.setEnabled(!isLinked);
            b.setTooltip(isLinked ? "Follows the perspective \"" + perspective + "\" - switch it there, or set this module apart (LINKED off) first."
                : "ON: the module works. OFF: the sound passes untouched and every setting is kept. Right-click: learn a MIDI control for it.");
        };
        // A plug-in's switch: the perspective's (a route's dialog shows it locked).
        auto showPluginSwitch = [this](juce::TextButton& b, const juce::String& slotId)
        {
            auto* slot = owner.pluginSlot(perspective, slotId);
            const bool on = slot != nullptr && slot->on;
            if (b.getToggleState() != on) b.setToggleState(on, juce::dontSendNotification);
            if (b.getButtonText() != (on ? "ON" : "OFF")) b.setButtonText(on ? "ON" : "OFF");
            if (b.isEnabled() != (manual < 0)) b.setEnabled(manual < 0);
            b.setTooltip(manual >= 0 ? "A plug-in is the perspective's - switch it in the effects of the perspective \"" + perspective + "\"."
                : "ON: the plug-in works. OFF: the sound passes untouched and its setting is kept.");
            return on;
        };
        if (pluginPanel != nullptr && selectedPlugin.isNotEmpty())
        {
            if (auto* slot = owner.pluginSlot(perspective, selectedPlugin))
            {
                pluginPanel->name.setText(slot->description.name, juce::dontSendNotification);
                pluginPanel->maker.setText(slot->description.manufacturerName + "  -  " + slot->description.pluginFormatName, juce::dontSendNotification);
                showPluginSwitch(pluginPanel->onButton, selectedPlugin);
                if (++statusTick % 10 == 1)   // once a second: it takes the engine's counters
                {
                    juce::String warning;
                    pluginPanel->status.setText(owner.pluginStatus(perspective, selectedPlugin, &warning), juce::dontSendNotification);
                    if (pluginPanel->warning.getText() != warning) { pluginPanel->warning.setText(warning, juce::dontSendNotification); pluginPanel->resized(); }
                }
                pluginPanel->open.setEnabled(manual < 0);
                pluginPanel->remove.setEnabled(manual < 0);
            }
        }
        for (int i = 0; i < chainButtons.size() && (size_t)i < chain.size(); ++i)
            if (auto* s = switchButtons[i])
            {
                if (isPlugin((size_t)i))
                {
                    const auto colour = showPluginSwitch(*s, pluginIdOf((size_t)i)) ? juce::Colours::white : juce::Colours::grey;
                    if (chainButtons[i]->findColour(juce::TextButton::textColourOffId) != colour) chainButtons[i]->setColour(juce::TextButton::textColourOffId, colour);
                    continue;
                }
                showSwitch(*s, chain[(size_t)i].module);
                const auto colour = owner.effectOn(perspective, manual, chain[(size_t)i].module) ? juce::Colours::white : juce::Colours::grey;
                if (chainButtons[i]->findColour(juce::TextButton::textColourOffId) != colour) chainButtons[i]->setColour(juce::TextButton::textColourOffId, colour);
            }
        auto lockFor = [this, &showSwitch](Module m, Header* h, std::initializer_list<juce::Component*> controls)
        {
            const bool isLinked = linked(m);
            if (h != nullptr) showSwitch(h->onButton, m);
            if (h != nullptr)
            {
                if (manual >= 0)
                {
                    if (h->linkedButton.getToggleState() != isLinked) h->linkedButton.setToggleState(isLinked, juce::dontSendNotification);
                    h->linkedButton.setTooltip(isLinked ? "Follows the perspective \"" + perspective + "\": the sliders show its values and are locked. Switch off to set this route apart."
                        : "This route has its own settings. Switch on to follow \"" + perspective + "\" again (the own values are dropped).");
                    h->info.setText(isLinked ? "follows the perspective \"" + perspective + "\"" : "set apart - this route's own", juce::dontSendNotification);
                }
                else
                {
                    juce::StringArray apart;
                    const int n = owner.routesLinked(perspective, m, &apart);
                    h->info.setText(juce::String(n) + " of " + juce::String(n + apart.size()) + " route(s) follow this" + (apart.isEmpty() ? juce::String() : " - set apart: " + apart.joinIntoString(", ")), juce::dontSendNotification);
                }
            }
            for (auto* c : controls) c->setEnabled(!isLinked);
        };
        if (divisionPanel != nullptr && !divisionPanel->slider.isMouseButtonDown())
            divisionPanel->slider.setValue(owner.manualConfigs[(size_t)manual].divisionVolumeDb, juce::dontSendNotification);
        if (!volumePanel->slider.isMouseButtonDown()) volumePanel->slider.setValue(volumeDb(), juce::dontSendNotification);
        lockFor(Module::Volume, volumePanel->header, { &volumePanel->slider });
        if (!separationPanel->slider.isMouseButtonDown()) separationPanel->slider.setValue(separation() * 100.0, juce::dontSendNotification);
        lockFor(Module::Separation, separationPanel->header, { &separationPanel->slider });
        refreshEqRows();
        {
            lockFor(Module::Eq, eqPanel->header, { &eqPanel->graph });
            const bool isLinked = linked(Module::Eq);
            eqPanel->graph.locked = isLinked;
            for (auto& row : eqPanel->rows) { row.on.setEnabled(!isLinked); row.hz.setEnabled(!isLinked); row.gain.setEnabled(!isLinked); row.q.setEnabled(!isLinked); }
            if (manual >= 0) { if (auto* r = owner.managerForRoute(manual, perspective)) if (auto* d = r->getCurrentAudioDevice()) eqPanel->graph.sampleRate = d->getCurrentSampleRate(); }
            eqPanel->graph.repaint();
        }
        {
            const auto path = irPath();
            juce::String text = path.isEmpty() ? juce::String("IR: none") : "IR: " + juce::File(path).getFileName();
            if (manual >= 0 && path.isNotEmpty())
            {
                const auto info = owner.engine.getPerspectiveReverbInfo(manual, perspective);
                if (info.error.isNotEmpty()) text << "  - " << info.error;
                else if (info.channels > 0) text << "  (" << info.channels << " ch, " << juce::String(info.seconds, 2) << " s" << (info.light ? ", Light" : "") << (info.sharedWith.isNotEmpty() ? ", shared" : "") << ")";
                else if (!SampleFiles::existsAsFile(juce::File(path))) text << "  - FILE NOT FOUND";
            }
            irPanel->file.setText(text, juce::dontSendNotification);
            irPanel->file.setTooltip(path);
            if (!irPanel->wet.isMouseButtonDown()) irPanel->wet.setValue(wet() * 100.0, juce::dontSendNotification);
            const auto t = tone();
            const double values[6]{ t.highCutHz, t.lowCutHz, t.preDelayMs, t.wetDb, t.decayPercent, t.fadePercent };
            for (int i = 0; i < 6; ++i) if (!irPanel->tones[i].isMouseButtonDown()) irPanel->tones[i].setValue(values[i], juce::dontSendNotification);
            lockFor(Module::Ir, irPanel->header, { &irPanel->load, &irPanel->clear, &irPanel->wet, &irPanel->tones[0], &irPanel->tones[1], &irPanel->tones[2], &irPanel->tones[3], &irPanel->tones[4], &irPanel->tones[5] });
        }
    }
    void timerCallback() override
    {
        refresh();
        // 0.9.102 (the user: as the stops - "2 s stisk"): a chain row held down for two seconds enters the arrange mode.
        if (!arranger.isVisible())
            for (auto* b : chainButtons)
                if (b->downSinceMs > 0.0 && b->isDown() && juce::Time::getMillisecondCounterHiRes() - b->downSinceMs >= 2000.0)
                {
                    b->swallowRelease = true;
                    b->downSinceMs = 0.0;
                    beginArranging();
                    break;
                }
    }
    // A right-click anywhere in the chain's column (beside the buttons too) opens the chain's menu.
    void mouseDown(const juce::MouseEvent& e) override
    {
        if ((e.mods.isPopupMenu() || e.mods.isRightButtonDown()) && e.x < 12 + 214 + 8) chainMenu();
    }
};
