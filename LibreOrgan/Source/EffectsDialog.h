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
        // The chain.
        if (manual >= 0) chain.push_back({ Module::Volume, "Division volume", true });
        chain.push_back({ Module::Volume, "Volume", false });
        chain.push_back({ Module::Separation, "Channel separation", false });
        chain.push_back({ Module::Eq, "Equalizer", false });
        chain.push_back({ Module::Ir, "IR reverb", false });
        for (size_t i = 0; i < chain.size(); ++i)
        {
            auto* b = chainButtons.add(new juce::TextButton(chain[i].name));
            b->setClickingTogglesState(false);
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
            b->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
            b->setTooltip("The modules in the order the engine applies them. Click one to set it.");
            b->getProperties().set("manualChapter", chain[i].name == "Division volume" ? "Volume" : chain[i].name);
            b->onClick = [this, i] { select((int)i); };
            addAndMakeVisible(b);
        }
        buildPanels();
        select(manual >= 0 ? 1 : 0);
        refresh();
        startTimer(100);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(12);
        title.setBounds(area.removeFromTop(26));
        note.setBounds(area.removeFromTop(44));
        area.removeFromTop(6);
        auto list = area.removeFromLeft(190);
        for (auto* b : chainButtons) { b->setBounds(list.removeFromTop(34).reduced(0, 3)); }
        area.removeFromLeft(14);
        for (auto* p : panels) p->setBounds(area);
    }

private:
    struct ChainItem { Module module; juce::String name; bool divisionVolume; };
    MainComponent& owner;
    const juce::String perspective;
    const int manual;   // -1 = the perspective's dialog
    juce::String routeName;
    juce::Label title, note;
    std::vector<ChainItem> chain;
    juce::OwnedArray<juce::TextButton> chainButtons;
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
        void resized() override
        {
            auto r = getLocalBounds();
            name.setBounds(r.removeFromLeft(220));
            if (linkedButton.isVisible()) linkedButton.setBounds(r.removeFromLeft(90).reduced(0, 3));
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
                row.name.setBounds(line.removeFromLeft(84));
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
            static const char* const bandNames[5]{ "Low shelf", "Peak 1", "Peak 2", "Peak 3", "High shelf" };
            for (int b = 0; b < 5; ++b)
            {
                auto& row = p->rows[b];
                row.on.setTooltip("This band on or off");
                row.on.getProperties().set("manualChapter", "Equalizer");
                row.on.onClick = [this, b] { auto e = eq(); e.bands[b].on = eqPanel->rows[b].on.getToggleState(); storeEq(e); };
                p->addAndMakeVisible(row.on);
                row.name.setText(bandNames[b], juce::dontSendNotification);
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
    void storeEq(const SamplerEngine::EqSettings& e)
    {
        if (manual >= 0) routeConfig().eq = e; else perspectiveConfig().eq = e;
        changed(Module::Eq);
        eqPanel->graph.repaint();
    }
    void select(int index)
    {
        selected = juce::jlimit(0, (int)panels.size() - 1, index);
        for (int i = 0; i < panels.size(); ++i) panels[i]->setVisible(i == selected);
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
        auto lockFor = [this](Module m, Header* h, std::initializer_list<juce::Component*> controls)
        {
            const bool isLinked = linked(m);
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
    void timerCallback() override { refresh(); }
};
