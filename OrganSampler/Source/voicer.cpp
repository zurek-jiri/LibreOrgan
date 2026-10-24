#include "voicer.h"
#include "CrashHandler.h"

static juce::String canonicalPerspectiveName(juce::String name)
{
    return name.equalsIgnoreCase("misc.") ? juce::String("default") : name;
}

juce::var Voicer::transmissionsToVar(const Stop* stop)
{
    juce::Array<juce::var> list;
    for (const auto& t : stop->transmissions)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("manual", t.manual);
        obj->setProperty("shift", t.shift);
        obj->setProperty("db", t.db);
        obj->setProperty("enabled", t.enabled);
        if (t.alias.isNotEmpty()) obj->setProperty("alias", t.alias);
        if (t.volumeKeysExpanded) obj->setProperty("volumeKeysExpanded", true);
        if (t.toneKeysExpanded) obj->setProperty("toneKeysExpanded", true);
        if (t.groupMaster.isNotEmpty()) obj->setProperty("groupMaster", t.groupMaster);
        obj->setProperty("formantFollowPct", juce::roundToInt(t.formantFollow * 100.0f));
        if (t.keyDisabled.any())
        {
            juce::Array<juce::var> off;
            for (int key = 0; key < 128; ++key) if (t.keyDisabled.test((size_t)key)) off.add(key);
            obj->setProperty("disabledKeys", off);
        }
        juce::DynamicObject::Ptr perKey = new juce::DynamicObject();
        bool anyKey = false;
        for (int key = 0; key < 128; ++key)
            if (std::abs(t.keyDb[(size_t)key]) > 1.0e-4f)
            {
                perKey->setProperty(juce::Identifier(juce::String(key)), t.keyDb[(size_t)key]);
                anyKey = true;
            }
        if (anyKey) obj->setProperty("perKeyDb", juce::var(perKey.get()));
        juce::DynamicObject::Ptr perKeyTone = new juce::DynamicObject();
        bool anyTone = false;
        for (int key = 0; key < 128; ++key)
            if (std::abs(t.keyTone[(size_t)key]) > 1.0e-4f)
            {
                perKeyTone->setProperty(juce::Identifier(juce::String(key)), t.keyTone[(size_t)key]);
                anyTone = true;
            }
        if (anyTone) obj->setProperty("perKeyTone", juce::var(perKeyTone.get()));
        list.add(juce::var(obj.get()));
    }
    return list;
}

std::vector<Stop::Transmission> Voicer::transmissionsFromVar(const juce::var& stopObj, int primaryManual)
{
    std::vector<Stop::Transmission> list;
    if (const auto* array = stopObj.getProperty("transmissions", juce::var()).getArray())
    {
        for (const auto& item : *array)
        {
            if (!item.isObject()) continue;
            Stop::Transmission t;
            t.manual = juce::jlimit(0, SamplerEngine::maxManuals - 1, (int)item.getProperty("manual", 0));
            t.shift = juce::jlimit(-Stop::Transmission::maxShift, Stop::Transmission::maxShift, (int)item.getProperty("shift", 0));
            t.db = juce::jlimit(-10.0f, 10.0f, (float)(double)item.getProperty("db", 0.0));
            t.enabled = (bool)item.getProperty("enabled", false);
            t.alias = item.getProperty("alias", "").toString();
            t.volumeKeysExpanded = (bool)item.getProperty("volumeKeysExpanded", false);
            t.toneKeysExpanded = (bool)item.getProperty("toneKeysExpanded", false);
            t.groupMaster = item.getProperty("groupMaster", "").toString();
            t.formantFollow = juce::jlimit(0.0f, 1.0f, (float)(double)item.getProperty("formantFollowPct", 100.0 * SamplerEngine::defaultFormantFollow) / 100.0f);
            if (auto* off = item.getProperty("disabledKeys", juce::var()).getArray())
                for (const auto& key : *off)
                    if ((int)key >= 0 && (int)key < 128) t.keyDisabled.set((size_t)(int)key);
            if (auto* perKey = item.getProperty("perKeyDb", juce::var()).getDynamicObject())
                for (const auto& prop : perKey->getProperties())
                {
                    const int key = prop.name.toString().getIntValue();
                    if (key >= 0 && key < 128) t.keyDb[(size_t)key] = juce::jlimit(-24.0f, 24.0f, (float)(double)prop.value);
                }
            if (auto* perKeyTone = item.getProperty("perKeyTone", juce::var()).getDynamicObject())
                for (const auto& prop : perKeyTone->getProperties())
                {
                    const int key = prop.name.toString().getIntValue();
                    if (key >= 0 && key < 128) t.keyTone[(size_t)key] = juce::jlimit(-9.0f, 9.0f, (float)(double)prop.value);
                }
            list.push_back(t);
        }
        return list;
    }
    // Older states: one plain transmission per bit of the masks.
    const int legacyMask = (int)stopObj.getProperty("manualMask", 0);
    const int mask = (int)stopObj.getProperty("transmissionMask", legacyMask & ~(1 << juce::jlimit(0, SamplerEngine::maxManuals - 1, primaryManual)));
    const int enabledMask = (int)stopObj.getProperty("transmissionEnabledMask", 0);
    for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual)
        if ((mask & (1 << manual)) != 0 && manual != primaryManual)
        {
            Stop::Transmission t;
            t.manual = manual;
            t.enabled = (enabledMask & (1 << manual)) != 0;
            list.push_back(t);
        }
    return list;
}

namespace
{
    bool stopSwipeActive = false;
    bool stopSwipeEnable = false;
    std::vector<StopRow*> stopSwipeRows;
}

//==============================================================================
namespace
{
    // Popup menu of the transposition combobox: the octave steps (0, +-12, +-24)
    // are drawn bold so they stand out in the long list.
    struct BoldOctavesLookAndFeel final : public juce::LookAndFeel_V4
    {
        std::set<juce::String> boldTexts;
        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
            const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour) override
        {
            if (isSeparator || boldTexts.count(text) == 0)
            {
                juce::LookAndFeel_V4::drawPopupMenuItem(g, area, isSeparator, isActive, isHighlighted, isTicked,
                    hasSubMenu, text, shortcutKeyText, icon, textColour);
                return;
            }
            auto r = area.reduced(1);
            if (isHighlighted && isActive)
            {
                g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
                g.fillRect(r);
                g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
            }
            else
                g.setColour(textColour != nullptr ? *textColour : findColour(juce::PopupMenu::textColourId));
            if (!isActive) g.setOpacity(0.3f);
            auto font = getPopupMenuFont().boldened();
            g.setFont(font);
            r.reduce(juce::jmin(5, area.getWidth() / 20), 0);
            auto iconArea = r.removeFromLeft(juce::roundToInt(font.getHeight() * 1.2f)).toFloat();
            if (isTicked)
            {
                const auto tick = getTickShape(1.0f);
                g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5.0f, 0.0f), true));
            }
            r.removeFromRight(3);
            g.drawFittedText(text, r, juce::Justification::centredLeft, 1);
        }
    };

    std::unique_ptr<BoldOctavesLookAndFeel> makeBoldOctavesLookAndFeel()
    {
        auto lnf = std::make_unique<BoldOctavesLookAndFeel>();
        // The menu look follows the application's look and feel.
        auto& base = juce::LookAndFeel::getDefaultLookAndFeel();
        for (const int colourId : { (int)juce::PopupMenu::backgroundColourId, (int)juce::PopupMenu::textColourId,
            (int)juce::PopupMenu::highlightedBackgroundColourId, (int)juce::PopupMenu::highlightedTextColourId,
            (int)juce::ComboBox::backgroundColourId, (int)juce::ComboBox::textColourId, (int)juce::ComboBox::outlineColourId,
            (int)juce::ComboBox::buttonColourId, (int)juce::ComboBox::arrowColourId, (int)juce::ComboBox::focusedOutlineColourId })
            lnf->setColour(colourId, base.findColour(colourId));
        return lnf;
    }

    const juce::String divisionNames[8]{ "Pedal", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
}

juce::String StopRow::caption() const
{
    if (auto* t = transmission()) return Voicer::transmissionLabel(stop, transmissionIndex);
    // A stop folder shows how many ranks sound with it (a Roman numeral: the
    // children, plus the master itself when it has pipes of its own).
    if (childrenGetter)
    {
        const auto children = childrenGetter(stop);
        const int transmissions = childTransmissionsGetter ? (int)childTransmissionsGetter(stop).size() : 0;
        if ((!children.empty() || transmissions > 0) && stop->showRankCount)
        {
            bool sounds = false;
            for (int note = 0; note < 128 && !sounds; ++note) sounds = stop->hasNoteInAnyRank(note);
            return stop->displayName() + "  [" + romanNumeral((int)children.size() + transmissions + (sounds ? 1 : 0)) + "]";
        }
    }
    return stop->displayName();
}

StopRow::StopRow(Stop* stop, SamplerEngine& engine, int noteMin, int noteMax, int sliderWidth,
    std::function<void(const juce::String&)> midiLearnCallback, std::function<void(Stop*)> relayout,
    int activeManualCount, std::function<void()> stateChanged, int transmissionIndexIn)
    : stop(stop), engine(engine), transmissionIndex(transmissionIndexIn), noteMin(noteMin), noteMax(noteMax),
      sliderWidth(sliderWidth), midiLearnCallback(std::move(midiLearnCallback)), relayout(std::move(relayout)),
      stateChanged(std::move(stateChanged))
{
    stopSwipeRows.push_back(this);
    SwipeSwitch::add(&stopButton, [this](bool enabled) { applyStopState(enabled); });
    // Stop button
    stopButton.setButtonText(caption());
    stopButton.setColour(juce::TextButton::buttonColourId,
        routeEnabled() ? onColour() : juce::Colours::darkgrey);
    addAndMakeVisible(stopButton);
    stopButton.addMouseListener(this, false);
    voicingToggle.setTooltip("Show/hide voicing controls");
    voicingToggle.setButtonText(isExpanded() ? "-" : "+");
    // Arrange mode: the "x" in the button's top-left corner removes the item.
    addChildComponent(removeButton);
    removeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);   // the red of PANIC and GLOBAL RESET
    removeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    removeButton.setTooltip(transmissionIndexIn >= 0 ? "Remove this transmission"
        : "Remove this stop from the organ (its Stop List row goes; a warning names its transmissions first)");
    removeButton.onClick = [this] { if (arrange.remove) arrange.remove(this); };
    voicingToggle.onClick = [this]
    {
        const bool expand = !isExpanded();
        if (expand)
        {
            // One division's voicing at a time: the other divisions fold up.
            const int division = divisionIndex();
            for (auto* other : this->engine.getStops())
            {
                if (other->primaryManual != division) other->voicingExpanded = false;
                for (auto& t : other->transmissions) if (t.manual != division) t.voicingExpanded = false;
            }
        }
        if (auto* t = transmission()) t->voicingExpanded = expand;
        else
        {
            this->stop->voicingExpanded = expand;
            // A stop folder lists its children under its open voicing (each opens
            // its own voicing when asked). Folding the master up folds them too and
            // they take the master's state again, so none keeps sounding behind a
            // master that is off.
            if (childrenGetter)
            {
                const auto children = childrenGetter(this->stop);
                if (!expand) for (auto* child : children) child->voicingExpanded = false;
                if (!expand)
                    for (auto* child : children)
                        if (child->enabled != this->stop->enabled) this->engine.setStopEnabled(child, this->stop->enabled);
                if (!expand && childTransmissionsGetter)
                    for (const auto& [source, index] : childTransmissionsGetter(this->stop))
                    {
                        source->transmissions[(size_t)index].voicingExpanded = false;
                        if (source->transmissions[(size_t)index].enabled != this->stop->enabled)
                            this->engine.setTransmissionEnabled(source, index, this->stop->enabled);
                    }
                if (!expand) for (auto* row : stopSwipeRows) row->refreshEnabledColour();
                if (!expand && this->stateChanged) this->stateChanged();
            }
        }
        if (this->relayout)
        {
            juce::Component::SafePointer<StopRow> safeThis(this);
            auto callback = this->relayout;
            auto* target = this->stop;
            juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
            {
                if (safeThis != nullptr) callback(target);
            });
        }
    };
    addAndMakeVisible(voicingToggle);

    static const juce::String manualNames[8] { "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
    transmissionLabel.setText("Transmissions:", juce::dontSendNotification);
    transmissionLabel.setJustificationType(juce::Justification::centredRight);
    // Transmissions are added and removed in the Stop List; the boxes stay hidden.
    addChildComponent(transmissionLabel);

    ranksLabel.setText("Perspectives:", juce::dontSendNotification);
    ranksLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(ranksLabel);
    std::vector<Stop*> rankSources{ stop };
    for (auto& rank : stop->ranks) rankSources.push_back(rank.get());
    for (auto* rank : rankSources)
    {
        auto* button = rankButtons.add(new juce::ToggleButton(rank->perspective));
        button->setToggleState(rank->rankEnabled, juce::dontSendNotification);
        button->onClick = [this, rank, button]
        {
            this->engine.setRankEnabled(this->stop, rank, button->getToggleState());
            if (this->stateChanged) this->stateChanged();
        };
        addAndMakeVisible(button);
    }
    // Velocity layers: files "_NNN" after the key play for velocities up to NNN.
    velocityLabel.setText("Velocity layers:", juce::dontSendNotification);
    velocityLabel.setFont(juce::FontOptions(12.5f));
    velocityLabel.setJustificationType(juce::Justification::centredRight);
    addChildComponent(velocityLabel);
    if (!isTransmission())
    {
        const auto layers = stop->velocityLayerValues();
        for (const int maxVelocity : layers)
        {
            const juce::String suffix = "_" + juce::String(maxVelocity).paddedLeft('0', 3);
            auto* button = velocityButtons.add(new juce::ToggleButton(suffix));   // the suffix alone; the tooltip and the manual explain
            button->setToggleState(!stop->velocityLayerDisabled.test((size_t)maxVelocity), juce::dontSendNotification);
            button->setTooltip("Sample files named with \"" + suffix + "\" after the key (e.g. 060-C" + suffix + ".wav) play for key "
                "velocities up to " + juce::String(maxVelocity) + "; above that the plain file plays (or the next layer up). "
                "Unticked, these velocities use the next layer up or the plain samples as well.\n\n"
                "Special use: when a stop has plain samples and most of its keys also carry " + suffix + " files, the "
                + suffix + " files are usually the original recordings and the plain ones the fine-tuned versions (the "
                "harmonics tuned inside the audio - a rare treatment for detuned mixtures). Untick " + suffix
                + " to hear only the fine-tuned samples.");
            button->onClick = [this, maxVelocity, button]
            {
                this->engine.setVelocityLayerEnabled(this->stop, maxVelocity, button->getToggleState());
                if (this->stateChanged) this->stateChanged();
            };
            addChildComponent(button);
        }
        if (!layers.empty())
        {
            // The plain files are the top layer ("_128"): they can be switched off too.
            auto* button = velocityButtons.add(new juce::ToggleButton("_128"));
            button->setToggleState(!stop->plainLayerDisabled(), juce::dontSendNotification);
            button->setTooltip("The sample files without a \"_NNN\" suffix: the top velocity layer, played for key velocities above "
                + juce::String(layers.back()) + ". Unticked, those velocities play the highest layer that is ticked instead (its "
                "release too). When nothing is ticked the plain files play anyway - a key never falls silent.");
            button->onClick = [this, button]
            {
                this->engine.setVelocityLayerEnabled(this->stop, 128, button->getToggleState());
                if (this->stateChanged) this->stateChanged();
            };
            addChildComponent(button);
        }
    }

    // Recorded diapason of the stop (the 440 Hz normalisation is a global switch
    // in the Temperaments dialog).
    {
        if (stop->container)
            origText = juce::String("Stop folder (no pipes of its own). Each subordinate stop in its branch is voiced separately below ")
                + juce::String(juce::CharPointer_UTF8("\xe2\x86\x93"));
        else if (!stop->tuningInfoAvailable) origText = "Orig. Pitch: no pitch data in the samples";
        else
        {
            origText = "Orig. Pitch: a1 = " + juce::String(stop->measuredDiapasonHz, 1) + " Hz";
            for (const auto& rank : stop->ranks)
                if (rank->tuningInfoAvailable && std::abs(rank->measuredDiapasonHz - stop->measuredDiapasonHz) > 0.3f)
                    origText << " / " << juce::String(rank->measuredDiapasonHz, 1);
            // The smpl footage (per break segment), and in brackets the footage the
            // pipes really sound at when a compound stop's fundamental is missing ("8' (2 2/3')").
            if (stop->footageSounding.isNotEmpty()) origText << "; " << stop->footageSounding;
            else if (stop->footageBreaks.isNotEmpty()) origText << "; " << stop->footageBreaks;   // rank with breaks
            else if (stop->footageKnown) origText << "; " << SamplerEngine::footageName(stop->footageSemitones);
            float celeste = stop->detectedCelesteBeats;
            for (const auto& rank : stop->ranks)
                if (std::abs(celeste) < 1.0e-3f) celeste = rank->detectedCelesteBeats;
            if (std::abs(celeste) >= 1.0e-3f)
                origText << "; celeste " << (celeste > 0.0f ? "+" : "") << juce::String(celeste, 1) << " beats/s at a1";
        }
        diapasonLabel.setText(origText, juce::dontSendNotification);   // same font size as "Perspectives:"
        diapasonLabel.setJustificationType(juce::Justification::centredLeft);
        diapasonLabel.setVisible(stop->voicingExpanded);
        addAndMakeVisible(diapasonLabel);
    }
    // Undulation (celeste): beats per second at a1; only meaningful once the
    // stop is tuned to 440 Hz, so it is greyed out otherwise.
    undulationSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    undulationSlider.setRange(-7.0, 7.0, 0.1);
    undulationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    undulationSlider.setValue(stop->undulationBeats, juce::dontSendNotification);
    undulationSlider.setDoubleClickReturnValue(true, 0.0);
    undulationSlider.setScrollWheelEnabled(true);
    undulationSlider.addMouseListener(this, false);   // right-click: MIDI Learn like the other voicing sliders
    undulationSlider.setAlpha(engine.getTuneAllTo440() ? 1.0f : 0.4f);   // greyed out until the 440 Hz diapason is on
    undulationSlider.setTooltip("Celeste: beats per second against an in-tune rank at a1 (negative = flat). "
        "Beats grow with pitch; how much faster the top of the compass beats than the bottom is set in "
        "Configuration Editor > Advanced global audio (Celeste beats).");
    undulationSlider.onValueChange = [this]
    {
        this->stop->undulationManual = true;   // from now on the saved value rules, not the detection
        this->engine.setStopUndulation(this->stop, (float)undulationSlider.getValue());
        refreshUndulationLabel();
        if (this->stateChanged) this->stateChanged();
    };
    undulationSlider.setEnabled(engine.getTuneAllTo440());
    undulationSlider.setVisible(stop->voicingExpanded);
    addAndMakeVisible(undulationSlider);
    undulationLabel.setFont(juce::FontOptions(12.5f));
    undulationLabel.setJustificationType(juce::Justification::centredLeft);
    undulationLabel.setVisible(stop->voicingExpanded);
    addAndMakeVisible(undulationLabel);
    refreshUndulationLabel();

    // Transposition: a transmission's shift (its pipes borrowed from the source
    // stop), or (0.8.158) the stop's own - the key plays the pipe meant for the key
    // this many semitones away, its transmissions add their shift on top. The list
    // holds the pure intervals of the harmonic series with the footage each gives.
    {
        auto* t = transmission();
        transpositionLabel.setText("Transposition:", juce::dontSendNotification);
        transpositionLabel.setJustificationType(juce::Justification::centredRight);
        transpositionLabel.setTooltip(t != nullptr
            ? "Extension: the key sounds the pipe this many semitones away, as a pure interval of the key (harmonic series). "
              "-12 makes a 16' extension of an 8' stop, +12 a 4' one."
            : "The stop's transposition against its files on their own numbers (0 = the file 036 on key 36, at the footage the samples' pitch says), "
              "as a pure interval: -12 puts the file 024 on key 36 - an octave down (Doesburg: files sounding 8' on their numbers make a 16' this way), "
              "+12 an octave up. Found by the program from the samples' pitch and the footage in the folder's name, or from a Hauptwerk definition; "
              "its transmissions and extensions add their own on top. A change loads the samples again on their keys.");
        addAndMakeVisible(transpositionLabel);
        auto lnf = makeBoldOctavesLookAndFeel();
        const int base = t != nullptr ? SamplerEngine::stopFootageSemitones(stop) : SamplerEngine::untransposedFootageSemitones(stop);
        const int current = t != nullptr ? t->shift : stop->sampleOffset;
        // Every semitone from -24 to +24, each a pure interval with the fraction it
        // gives (octaves printed bold), as the transmission rows had it since 0.8.111.
        std::vector<int> steps;
        for (int shift = -Stop::Transmission::maxShift; shift <= Stop::Transmission::maxShift; ++shift) steps.push_back(shift);
        if (std::find(steps.begin(), steps.end(), current) == steps.end()) { steps.push_back(current); std::sort(steps.begin(), steps.end()); }
        for (const int shift : steps)
        {
            juce::String text = (shift > 0 ? "+" : "") + juce::String(shift) + "  ->  " + SamplerEngine::footageLabel(base + shift);
            if (shift == 0) text = "0  ->  " + SamplerEngine::footageLabel(base) + " (as the original)";
            if (shift % 12 == 0) lnf->boldTexts.insert(text);
            shiftBox.addItem(text, shift + 100);
        }
        shiftMenuLookAndFeel = std::move(lnf);
        shiftBox.setLookAndFeel(shiftMenuLookAndFeel.get());
        shiftBox.setSelectedId(current + 100, juce::dontSendNotification);
        shiftBox.onChange = [this]
        {
            const int shift = shiftBox.getSelectedId() - 100;
            if (!isTransmission())
            {
                if (shift == this->stop->sampleOffset) return;
                if (transpositionRequest) transpositionRequest(this->stop, shift);   // the owner reloads the samples and rebuilds the rows
                return;
            }
            this->engine.setTransmissionShift(this->stop, transmissionIndex, shift);
            stopButton.setButtonText(caption());
            if (this->stateChanged) this->stateChanged();
            // The per-key sliders map to other pipes now: rebuild the row.
            if (this->relayout)
            {
                juce::Component::SafePointer<StopRow> safeThis(this);
                auto callback = this->relayout;
                auto* target = this->stop;
                juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
                {
                    if (safeThis != nullptr) callback(target);
                });
            }
        };
        addAndMakeVisible(shiftBox);
        if (t != nullptr)
        {
            sourceLabel.setText("Taken from: " + stop->displayName() + " (" + divisionNames[juce::jlimit(0, SamplerEngine::maxManuals - 1, stop->primaryManual)]
                + "), " + (stop->footageKnown ? "smpl pitch: " : "name: ") + SamplerEngine::footageLabel(base),
                juce::dontSendNotification);
            sourceLabel.setJustificationType(juce::Justification::centredLeft);
            addAndMakeVisible(sourceLabel);
        }
    }
    // Alias: the caption shown on the button instead of the automatic name.
    aliasLabel.setText("Alias:", juce::dontSendNotification);
    aliasLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(aliasLabel);
    aliasBox.setEditableText(true);
    aliasBox.addItem("Automatic", 1);
    {
        const juce::String alias = transmission() != nullptr ? transmission()->alias : stop->alias;
        if (alias.isNotEmpty()) aliasBox.setText(alias, juce::dontSendNotification);
        else aliasBox.setSelectedId(1, juce::dontSendNotification);
    }
    aliasBox.setTooltip("Name shown on the button. Automatic = the folder name (transmissions: with the footage of the "
        "transposition and \"Trsm.\"). Type any text to replace it; the stop keeps its identity for saved states.");
    aliasBox.onChange = [this] { aliasChanged(); };
    addAndMakeVisible(aliasBox);
    // A stop folder: whether its button carries the rank count "[II]" (0.8.146).
    rankCountToggle.setButtonText("Rank count [II]");
    rankCountToggle.setToggleState(this->stop->showRankCount, juce::dontSendNotification);
    rankCountToggle.setTooltip("The button of a stop folder shows how many ranks sound with it, as a Roman numeral. Off: the name alone.");
    rankCountToggle.onClick = [this]
    {
        this->stop->showRankCount = rankCountToggle.getToggleState();
        stopButton.setButtonText(caption());
        if (this->relayout) this->relayout(this->stop);
    };
    addAndMakeVisible(rankCountToggle);
    // One-shot (0.8.158): the stop plays each sample once through, whenever the
    // key goes up (chimes, harp). The switch starts as the One-shot Patterns say
    // and overrides them once touched.
    oneShotToggle.setButtonText("One-shot");
    oneShotToggle.setToggleState(this->stop->isOneShot, juce::dontSendNotification);
    oneShotToggle.setTooltip("The stop plays each sample once from start to end, whenever the key is released (chimes, harp, celesta). "
        "Set by the One-shot Patterns from the folder's name; this switch overrides them for this stop.");
    oneShotToggle.onClick = [this] { if (oneShotRequest) oneShotRequest(this->stop, oneShotToggle.getToggleState()); };
    addAndMakeVisible(oneShotToggle);

    for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual)
    {
        auto& primary = primaryManualButtons[(size_t)manual];
        primary.setButtonText(manualNames[manual]);
        primary.setToggleState(stop->primaryManual == manual, juce::dontSendNotification);
        primary.onClick = [this, manual]
        {
            {
                juce::ScopedLock sl(this->engine.getLock());
                this->stop->primaryManual = manual;
            }
            for (int i = 0; i < SamplerEngine::maxManuals; ++i)
                primaryManualButtons[(size_t)i].setToggleState(i == manual, juce::dontSendNotification);
            if (this->stateChanged) this->stateChanged();
            if (this->relayout)
            {
                juce::Component::SafePointer<StopRow> safeThis(this);
                auto callback = this->relayout;
                auto* target = this->stop;
                juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
                {
                    if (safeThis != nullptr) callback(target);
                });
            }
        };

        // Number of transmissions (extra stop buttons) into this division.
        auto& box = transmissionBoxes[(size_t)manual];
        for (int count = 0; count <= 6; ++count)
            box.addItem(manualNames[manual] + " " + (count == 0 ? juce::String("-") : juce::String(count)), count + 1);
        box.setSelectedId(juce::jlimit(0, 6, stop->transmissionCount(manual)) + 1, juce::dontSendNotification);
        box.onChange = [this, manual] { transmissionCountChanged(manual); };
        addChildComponent(box);   // hidden: the Stop List adds and removes transmissions
        juce::ignoreUnused(activeManualCount);
    }

    // Global volume slider (transmissions: their volume relative to the parent stop)
    globalSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    globalSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    if (auto* t = transmission())
    {
        globalSlider.setRange(-10.0, 10.0, 0.1);
        globalSlider.setValue(t->db, juce::dontSendNotification);
        globalSlider.setTooltip("Volume of this transmission relative to the parent stop. A pipe held through several "
            "routes sounds at the loudest one; when that route lets go the volume glides to the next loudest.");
    }
    else
    {
        globalSlider.setRange(-24.0, 12.0, 0.1);
        globalSlider.setValue(stop->db, juce::dontSendNotification);
    }
    globalSlider.setTextValueSuffix(" dB");
    globalSlider.setScrollWheelEnabled(true);
    globalSlider.setDoubleClickReturnValue(true, 0.0);
    globalSlider.addMouseListener(this, false);
    globalSlider.onValueChange = [this] { globalSliderChanged(); };
    addAndMakeVisible(globalSlider);
    globalLabel.setText(isTransmission() ? "Volume vs. parent stop" : "Stop volume", juce::dontSendNotification);
    globalLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(globalLabel);

    // Attack gain slider (boost)
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    gainSlider.setRange(-6.0, 6.0, 0.1);
    gainSlider.setValue(stop->gainDb, juce::dontSendNotification);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setScrollWheelEnabled(true);
    gainSlider.setDoubleClickReturnValue(true, 0.0);
    gainSlider.addMouseListener(this, false);
    gainSlider.onValueChange = [this] { gainSliderChanged(); };
    addAndMakeVisible(gainSlider);
    gainLabel.setText("Attack Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(gainLabel);

    // Attack time slider
    attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    // One slider for the whole attack envelope: the attack gain holds for the
    // first half of this time and fades back to the normal level over the second.
    attackSlider.setRange(0.0, 500.0, 1.0);
    attackSlider.setValue(stop->attackMs + stop->attackFadeoutMs, juce::dontSendNotification);
    attackSlider.setTextValueSuffix(" ms");
    attackSlider.setScrollWheelEnabled(true);
    attackSlider.setDoubleClickReturnValue(true, 150.0);
    attackSlider.setTooltip("How long the Attack Gain acts at the start of every note: it holds for the first half of this "
        "time and fades back to the normal level over the second half (0-500 ms, default 150).");
    attackSlider.addMouseListener(this, false);
    attackSlider.onValueChange = [this] { attackSliderChanged(); };
    addAndMakeVisible(attackSlider);
    attackLabel.setText("Attack Time", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(attackLabel);

    randomPitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    randomPitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    randomPitchSlider.setRange(0.0, 40.0, 0.1);
    randomPitchSlider.setValue(stop->randomPitchErrorCents, juce::dontSendNotification);
    randomPitchSlider.setTextValueSuffix(" cents");
    randomPitchSlider.setScrollWheelEnabled(true);
    randomPitchSlider.setDoubleClickReturnValue(true, 4.0);
    randomPitchSlider.addMouseListener(this, false);
    randomPitchSlider.onValueChange = [this] { randomPitchSliderChanged(); };
    addAndMakeVisible(randomPitchSlider);
    randomPitchLabel.setText("Random pitch error", juce::dontSendNotification);
    randomPitchLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(randomPitchLabel);

    // Pitch shift of the filled pipes: how far their spectral envelope follows the
    // shift. Per stop; a transmission has its own for the pipes only it asks for.
    formantSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    formantSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
    formantSlider.setRange(0.0, 100.0, 1.0);
    formantSlider.setValue(100.0 * (transmission() != nullptr ? transmission()->formantFollow : stop->formantFollow), juce::dontSendNotification);
    formantSlider.setTextValueSuffix(" %");
    formantSlider.setScrollWheelEnabled(true);
    formantSlider.setDoubleClickReturnValue(true, 100.0 * SamplerEngine::defaultFormantFollow);
    formantSlider.addMouseListener(this, false);
    formantSlider.setTooltip("Filled pipes of this stop (pitch shift method): how far the spectral envelope - the formants, "
        "the body of the sound - follows the pitch shift of a pipe made from a neighbour's sample. 0 % keeps the envelope "
        "where the recording had it (brightest), 100 % moves it fully with the pitch (plain transposition, veiled two "
        "octaves down). The value is for shifts downwards up to an octave; shifts upwards follow 30 points more; beyond an "
        "octave every further octave in either direction adds 15 points (the phase vocoder 25): the farther the shift, the "
        "more the envelope goes with it. A pipe is one copy: "
        "inside the stop's own compass the stop's value applies; a pipe beyond it (asked for by transmissions only) "
        "takes the value of the first transmission in the stop's list whose shifted compass holds it - two "
        "transmissions asking for the same pipe share that copy. Changing it rebuilds this stop's copies of the pipes "
        "concerned in the background; the cache keeps the others.");
    formantSlider.onValueChange = [this] { formantSliderChanged(); };
    addAndMakeVisible(formantSlider);
    formantLabel.setText("Formants (filled pipes)", juce::dontSendNotification);
    formantLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(formantLabel);

    // Headers of the two per-key rows (fold like the Configuration Editor's
    // sections) with a small "Reset to 0 dB" beside each.
    volumeHeader.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    toneHeader.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    volumeHeader.onClick = [this] { if (toggleSection) toggleSection(0); };
    toneHeader.onClick = [this] { if (toggleSection) toggleSection(1); };
    toneHeader.setTooltip("Tone per key: a high shelf of this many dB above the tone start frequency (Configuration "
        "Editor > Advanced global audio, default 1800 Hz, or the pipe's own fundamental when higher; the transition "
        "band is about an octave wide). A pipe held through several routes takes the highest setting. Click to fold.");
    addAndMakeVisible(volumeHeader);
    addAndMakeVisible(toneHeader);
    for (auto* reset : { &volumeReset, &toneReset })
    {
        reset->setTooltip("Sets every slider of this row back to 0 dB");
        reset->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        addAndMakeVisible(reset);
    }
    volumeReset.onClick = [this] { resetKeyRow(false); };
    toneReset.onClick = [this] { resetKeyRow(true); };

    // Per-key sliders and labels
    for (int note = noteMin; note <= noteMax; ++note)
    {
        // Vertical slider: the volume of this key (-9..9 dB)
        auto* slider = new LearnableSlider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        slider->setRange(-9.0, 9.0, 0.1);
        if (auto* t = transmission()) slider->setValue(t->keyDb[(size_t)note], juce::dontSendNotification);
        else slider->setValue(stop->getNoteDb(note), juce::dontSendNotification);
        slider->setTextValueSuffix("");
        slider->setScrollWheelEnabled(true);
        slider->setDoubleClickReturnValue(true, 0.0);
        slider->addMouseListener(this, false);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, sliderWidth, 18);
        slider->onValueChange = [this, note, slider] {
            keySliderChanged(note, (float)slider->getValue());
            };
        keySliders.add(slider);
        addAndMakeVisible(slider);

        // Second row: the tone of this key (high shelf, -9..9 dB)
        auto* tone = new LearnableSlider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
        tone->setRange(-9.0, 9.0, 0.1);
        if (auto* t = transmission()) tone->setValue(t->keyTone[(size_t)note], juce::dontSendNotification);
        else tone->setValue(stop->getNoteToneDb(note), juce::dontSendNotification);
        tone->setTextValueSuffix("");
        tone->setScrollWheelEnabled(true);
        tone->setDoubleClickReturnValue(true, 0.0);
        tone->addMouseListener(this, false);
        tone->setTextBoxStyle(juce::Slider::TextBoxBelow, false, sliderWidth, 18);
        tone->onValueChange = [this, note, tone] { toneSliderChanged(note, (float)tone->getValue()); };
        toneSliders.add(tone);
        addAndMakeVisible(tone);

        // Note name label (above slider)
        juce::String noteName = juce::MidiMessage::getMidiNoteName(note, true, true, 3);
        noteName = noteName.upToFirstOccurrenceOf(" ", false, false);
        auto* nameLabel = new juce::Label();
        nameLabel->setText(noteName, juce::dontSendNotification);
        nameLabel->setJustificationType(juce::Justification::centred);
        nameLabel->setFont(juce::FontOptions(10.0f));
        noteNameLabels.add(nameLabel);
        addAndMakeVisible(nameLabel);
        auto* toneNameLabel = new juce::Label();
        toneNameLabel->setText(noteName, juce::dontSendNotification);
        toneNameLabel->setJustificationType(juce::Justification::centred);
        toneNameLabel->setFont(juce::FontOptions(10.0f));
        toneNameLabels.add(toneNameLabel);
        addAndMakeVisible(toneNameLabel);

        // MIDI number label (above note name)
        auto* numLabel = new juce::Label();
        numLabel->setText(juce::String(note), juce::dontSendNotification);
        numLabel->setJustificationType(juce::Justification::centred);
        numLabel->setFont(juce::FontOptions(9.0f));
        midiNumLabels.add(numLabel);
        addAndMakeVisible(numLabel);

        // Disable slider if the stop has no sample for this key's pipe (a
        // transmission's key plays the pipe shifted by its transposition). A pipe
        // filled from a neighbouring pipe gets a green thumb.
        const int pipe = pipeForKey(note);
        // A transmission switches its keys off on its own; the stop's own row its pipes.
        auto* thisTransmission = transmission();
        const bool pipeOff = thisTransmission != nullptr ? thisTransmission->keyDisabled.test((size_t)note)
            : (pipe >= 0 && pipe < 128 && stop->isPipeDisabled(pipe));
        // A faulty pipe (beyond the retune limit, 0.8.165) is switched off and plays
        // its neighbour's sample: shown as a filled key, not as a pipe of its own
        // (0.8.172: the dot stayed blue and the tooltip named the own sample).
        bool faulty = false;
        if (pipe >= 0 && pipe < 128)
            for (const auto* rank : rankSources)
                if (rank != nullptr && rank->faultyPipes.test((size_t)pipe)) { faulty = true; break; }
        const bool hasSample = pipe >= 0 && pipe < 128 && stop->hasNoteInAnyRank(pipe) && !faulty;
        const bool filled = !hasSample && pipe >= 0 && pipe < 128 && stop->isFilledInAnyRank(pipe);
        // A pipe switched off is greyed out like a pipe that does not exist,
        // whether it is a real pipe or an extension.
        const bool usable = (hasSample || filled) && !pipeOff;
        slider->setEnabled(usable);
        tone->setEnabled(usable);

        // Under the note name: the pipe on/off box. Off = the key stays silent
        // (no extension for a cornet); an extension's copies are freed.
        auto* box = new juce::ToggleButton();
        box->setToggleState(!pipeOff, juce::dontSendNotification);
        box->setEnabled(pipe >= 0 && pipe < 128);
        if (thisTransmission != nullptr)
            box->setTooltip(pipeOff ? "Key switched off in this transmission: silent here only. Tick to play it again."
                : "Key on in this transmission. Untick to silence it here only: the stop's own button and its other "
                  "transmissions keep playing it.");
        else
            box->setTooltip(pipeOff ? "Pipe switched off: silent on the stop's own button. Tick to play it again."
                : "Pipe on. Untick to switch this pipe off on the stop's own button: it stays silent and is never filled "
                  "from a neighbour (an extension a stop must not have, a cornet's missing bass). The stop's transmissions "
                  "still play it (they switch their keys off on their own). Without transmissions an extension's copies are "
                  "freed from RAM; a real pipe keeps its samples.");
        box->onClick = [this, pipe, note, box]
        {
            if (this->isTransmission()) this->engine.setTransmissionKeyEnabled(this->stop, this->transmissionIndex, note, box->getToggleState());
            else this->engine.setPipeEnabled(this->stop, pipe, box->getToggleState());
            if (this->stateChanged) this->stateChanged();
            if (this->relayout)   // sliders, colours and fills of the row change
            {
                juce::Component::SafePointer<StopRow> safeThis(this);
                auto callback = this->relayout;
                auto* target = this->stop;
                juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
                {
                    if (safeThis != nullptr) callback(target);
                });
            }
        };
        pipeBoxes.add(box);
        addAndMakeVisible(box);

        const juce::Colour green(0xff4cd964);
        if (!usable)
        {
            slider->setColour(juce::Slider::trackColourId, juce::Colours::grey);
            tone->setColour(juce::Slider::trackColourId, juce::Colours::grey);
            nameLabel->setColour(juce::Label::textColourId, juce::Colours::grey);
            toneNameLabel->setColour(juce::Label::textColourId, juce::Colours::grey);
            numLabel->setColour(juce::Label::textColourId, juce::Colours::grey);
        }
        else
        {
            // Store original track colour for later restoration (both rows light up while the key sounds)
            originalTrackColours[slider] = slider->findColour(juce::Slider::trackColourId);
            originalTrackColours[tone] = tone->findColour(juce::Slider::trackColourId);
            int source = pipe;
            if (filled)
            {
                // Only the thumb is green: this key has no pipe of its own.
                const Stop* filledRank = nullptr;
                for (const auto* rank : rankSources)
                    if (rank->isFilled(pipe)) { source = rank->sourceNoteFor(pipe); filledRank = rank; break; }
                slider->setColour(juce::Slider::thumbColourId, green);
                tone->setColour(juce::Slider::thumbColourId, green);
                // What sounds here: the parent stop, the file of the sample used, the
                // shift - and the key of this very row that plays that sample itself,
                // when the row has it: "HW Tromp 16', sample 039-D#.wav, -12 st (from key 51)".
                juce::String file;
                if (filledRank != nullptr && source >= 0 && source < 128)
                    for (const auto& sample : filledRank->samplesByNote[(size_t)source])
                        if (sample != nullptr && sample->fileName.isNotEmpty()) { file = sample->fileName; break; }
                const int shift = pipe - source;
                const int sourceKey = thisTransmission != nullptr ? source - thisTransmission->shift : source;   // the key of this row that plays the source pipe
                juce::String tip = stop->displayName() + ", sample " + (file.isNotEmpty() ? file : "of pipe " + juce::String(source)) + ", "
                    + (shift > 0 ? "+" : "") + juce::String(shift) + " st";
                if (sourceKey >= noteMin && sourceKey <= noteMax) tip << " (from key " << sourceKey << ")";
                // A faulty pipe (0.8.165): its own sample lies beyond the retune limit, so it
                // was switched off and this neighbour's sample plays instead - said here.
                if (filledRank != nullptr && pipe >= 0 && pipe < 128 && filledRank->faultyPipes.test((size_t)pipe))
                {
                    juce::String own;
                    for (const auto& sample : filledRank->samplesByNote[(size_t)pipe])
                        if (sample != nullptr && sample->fileName.isNotEmpty()) { own = sample->fileName; break; }
                    tip << ". Faulty pipe: its own sample " << (own.isNotEmpty() ? own : juce::String("here")) << " sounds "
                        << juce::String(-filledRank->rawTuningCents[(size_t)pipe], 0) << " cents off its key, beyond the retune limit ("
                        << juce::String(engine.getRetuneLimitCents(), 0) << " cents, Configuration Editor) - switched off and taken from the neighbour";
                }
                slider->setTooltip(tip);
                tone->setTooltip(tip);
                nameLabel->setTooltip(tip);
                toneNameLabel->setTooltip(tip);
                numLabel->setTooltip(tip);
            }
            else
            {
                // A pipe of its own: the file it plays (0.8.156, "Files vs keys" made the
                // number in the name and the key two different things).
                juce::String file;
                for (const auto* rank : rankSources)
                {
                    if (rank == nullptr || pipe < 0 || pipe >= 128) continue;
                    for (const auto& sample : rank->samplesByNote[(size_t)pipe])
                        if (sample != nullptr && sample->fileName.isNotEmpty()) { file = sample->fileName; break; }
                    if (file.isNotEmpty()) break;
                }
                if (file.isNotEmpty())
                {
                    const juce::String tip = stop->displayName() + ", sample " + file + " on key " + juce::String(pipe);
                    slider->setTooltip(tip);
                    tone->setTooltip(tip);
                    nameLabel->setTooltip(tip);
                    toneNameLabel->setTooltip(tip);
                    numLabel->setTooltip(tip);
                }
            }
            // An orange note name (the shade of every emphasised text): the sample
            // that sounds here has no loop in its smpl chunk.
            if (!stop->noteHasLoopInAnyRank(source))
            {
                nameLabel->setColour(juce::Label::textColourId, juce::Colours::orange);
                nameLabel->setTooltip("No loop in the smpl chunk of this sample");
                toneNameLabel->setColour(juce::Label::textColourId, juce::Colours::orange);
                toneNameLabel->setTooltip("No loop in the smpl chunk of this sample");
            }
        }
    }

    const bool showVoicing = isExpanded();
    // A stop folder without pipes: information and alias only, no sliders or key rows.
    const bool withPipes = showVoicing && !stop->container;
    const bool showVolumeRow = withPipes && volumeRowShown();
    const bool showToneRow = withPipes && toneRowShown();
    volumeHeader.setVisible(withPipes);
    toneHeader.setVisible(withPipes);
    volumeHeader.setSection("Volume per key", volumeRowShown());
    toneHeader.setSection("Treble per key", toneRowShown());
    volumeReset.setVisible(showVolumeRow);   // the reset belongs to an unfolded row
    toneReset.setVisible(showToneRow);
    for (auto* s : toneSliders) s->setVisible(showToneRow);
    for (auto* label : toneNameLabels) label->setVisible(showToneRow);
    const bool stopControls = withPipes && !isTransmission();     // pipe properties: the stop's own row only
    const bool transmissionControls = showVoicing && isTransmission();
    transmissionLabel.setVisible(stopControls);
    ranksLabel.setVisible(stopControls);
    for (auto* button : rankButtons) button->setVisible(stopControls);
    for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual)
        transmissionBoxes[(size_t)manual].setVisible(stopControls && manual < activeManualCount);
    diapasonLabel.setVisible(showVoicing && !isTransmission());
    transpositionLabel.setVisible(transmissionControls || stopControls);
    shiftBox.setVisible(transmissionControls || stopControls);
    sourceLabel.setVisible(transmissionControls);
    aliasLabel.setVisible(showVoicing);
    aliasBox.setVisible(showVoicing);
    // The rank-count switch only where the button can show a count (0.8.158): a
    // stop folder, or a stop with items indented under it.
    rankCountToggle.setVisible(showVoicing && !isTransmission() && ((childrenGetter && !childrenGetter(stop).empty())
        || (childTransmissionsGetter && !childTransmissionsGetter(stop).empty())));
    if (rankCountToggle.isVisible())
    {
        // The switch names the count the button would show (0.8.180: "[II]" always before - Cornet is "[III]").
        bool sounds = false;
        for (int note = 0; note < 128 && !sounds; ++note) sounds = stop->hasNoteInAnyRank(note);
        const int count = (int)childrenGetter(stop).size() + (childTransmissionsGetter ? (int)childTransmissionsGetter(stop).size() : 0) + (sounds ? 1 : 0);
        rankCountToggle.setButtonText("Rank count [" + romanNumeral(count) + "]");
    }
    oneShotToggle.setVisible(stopControls);
    globalSlider.setVisible(withPipes); globalLabel.setVisible(withPipes);
    gainSlider.setVisible(stopControls); gainLabel.setVisible(stopControls);
    attackSlider.setVisible(stopControls); attackLabel.setVisible(stopControls);
    randomPitchSlider.setVisible(stopControls); randomPitchLabel.setVisible(stopControls);
    undulationSlider.setVisible(stopControls); undulationLabel.setVisible(stopControls);
    formantSlider.setVisible(withPipes); formantLabel.setVisible(withPipes);   // stops and transmissions alike
    for (auto* slider : keySliders) slider->setVisible(showVolumeRow);
    for (auto* label : noteNameLabels) label->setVisible(showVolumeRow);
    for (auto* label : midiNumLabels) label->setVisible(showVolumeRow);
}

void StopRow::aliasChanged()
{
    juce::String text = aliasBox.getText().trim();
    if (text.equalsIgnoreCase("Automatic")) text.clear();
    {
        juce::ScopedLock sl(engine.getLock());
        if (auto* t = transmission()) t->alias = text;
        else stop->alias = text;
    }
    if (text.isEmpty()) aliasBox.setSelectedId(1, juce::dontSendNotification);
    stopButton.setButtonText(caption());
    if (stateChanged) stateChanged();
    // The transmissions of an aliased stop carry its name too: relabel the grid.
    if (!isTransmission() && relayout)
    {
        juce::Component::SafePointer<StopRow> safeThis(this);
        auto callback = relayout;
        auto* target = stop;
        juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
        {
            if (safeThis != nullptr) callback(target);
        });
    }
}

StopRow::~StopRow()
{
    stopSwipeRows.erase(std::remove(stopSwipeRows.begin(), stopSwipeRows.end(), this),
        stopSwipeRows.end());
    SwipeSwitch::remove(&stopButton);
    shiftBox.setLookAndFeel(nullptr);
}

void StopRow::transmissionCountChanged(int manual)
{
    const int wanted = juce::jlimit(0, 6, transmissionBoxes[(size_t)manual].getSelectedId() - 1);
    int have = stop->transmissionCount(manual);
    bool reloadSamples = false;
    while (have < wanted)
    {
        engine.addTransmission(stop, manual, 0);
        reloadSamples = reloadSamples || stop->samplesUnloaded;
        ++have;
    }
    // Lowering the count removes the last transmissions into this division
    // (switching them off first); the owner also drops their grid cells.
    while (have > wanted)
    {
        const int index = stop->transmissionIndex(manual, have - 1);
        if (index < 0) break;
        if (transmissionRemover) transmissionRemover(stop, index);
        else engine.removeTransmission(stop, index);
        --have;
    }
    // A transmission must sound even while the stop sits in its division's
    // pool: bring the samples back.
    if (reloadSamples) engine.reloadStopSamples(stop);
    if (stateChanged) stateChanged();
    if (relayout)
    {
        juce::Component::SafePointer<StopRow> safeThis(this);
        auto callback = relayout;
        auto* target = stop;
        juce::MessageManager::callAsync([safeThis, callback = std::move(callback), target]() mutable
        {
            if (safeThis != nullptr) callback(target);
        });
    }
}

void StopRow::mouseDown(const juce::MouseEvent& event)
{
    juce::OwnedArray<LearnableSlider>* row = nullptr;
    const int keySliderIndex = keySliderIndexForComponent(event.eventComponent, &row);
    if (keySliderIndex >= 0 && event.mods.isRightButtonDown())
    {
        keySliderPaintActive = true;
        paintRow = row;   // painting stays within the row (volume or tone) it started in
        lastPaintedKeySlider = -1;
        paintKeySlidersAt(event.getScreenPosition());
        return;
    }

    if (event.mods.isPopupMenu() && midiLearnCallback && !isTransmission())
    {
        juce::String parameter;
        int note = -1;
        if (event.eventComponent == &globalSlider) parameter = "volume";
        else if (event.eventComponent == &gainSlider) parameter = "attackGain";
        else if (event.eventComponent == &attackSlider) parameter = "attackTime";
        else if (event.eventComponent == &randomPitchSlider) parameter = "randomPitch";
        else if (event.eventComponent == &undulationSlider) parameter = "undulation";
        else if (event.eventComponent == &formantSlider) parameter = "formantFollow";
        if (parameter.isNotEmpty())
        {
            midiLearnCallback("slider:voicing:" + juce::String(stop->primaryManual) + ":"
                + parameter + ":" + juce::String(note) + ":" + stop->perspective
                + ":" + stop->name);
            return;
        }
    }
    if (event.eventComponent == &stopButton && arrange.active && arrange.active(this)
        && (event.mods.isLeftButtonDown() || event.mods.isRightButtonDown()))
    {
        // Arrange mode: the button drags the whole row instead of switching. The
        // right button (or Shift) drags a copy: a transmission is made where it lands.
        arrangeDragging = true;
        arrangeDuplicate = event.mods.isRightButtonDown() || event.mods.isShiftDown();
        pressScreenPosition = event.getScreenPosition();
        dragStartTopLeft = getPosition();
        if (arrangeDuplicate)
        {
            // The row stays where it is; a translucent copy of its button travels
            // with the mouse, so the duplication is visible.
            if (auto* parent = getParentComponent())
            {
                dragGhost = std::make_unique<juce::Label>();
                // The copy will be a transmission: say so already while it travels.
                dragGhost->setText(isTransmission() ? caption() : Voicer::transmissionLabel(stop, -1), juce::dontSendNotification);
                dragGhost->setJustificationType(juce::Justification::centred);
                dragGhost->setFont(juce::FontOptions(15.0f));
                dragGhost->setColour(juce::Label::backgroundColourId, onColour().withAlpha(0.75f));
                dragGhost->setColour(juce::Label::textColourId, juce::Colours::black);
                dragGhost->setInterceptsMouseClicks(false, false);
                dragGhost->setBounds(stopButton.getBounds().translated(getX(), getY()));
                parent->addAndMakeVisible(dragGhost.get());
                dragGhost->toFront(false);
                dragStartTopLeft = dragGhost->getPosition();
            }
        }
        else toFront(false);
        return;
    }
    if (event.eventComponent == &stopButton && event.mods.isPopupMenu() && midiLearnCallback)
    {
        if (auto* t = transmission())
            midiLearnCallback("transmission:" + juce::String(t->manual) + ":" + juce::String(stop->primaryManual) + ":"
                + stop->name + ":" + juce::String(stop->transmissionOrdinal(transmissionIndex)));
        else
            midiLearnCallback("stop:" + stop->name);
    }
    else if (event.eventComponent == &stopButton && event.mods.isLeftButtonDown())
    {
        stopSwipeActive = true;
        stopSwipeEnable = !routeEnabled();
        SwipeSwitch::begin(stopSwipeEnable);
        applyStopState(stopSwipeEnable);
        // A press held for 3 s without moving starts arrange mode.
        longPressCandidate = true;
        pressScreenPosition = event.getScreenPosition();
        startTimer(2000);   // the long press that opens the arrange mode
    }
}

void StopRow::timerCallback()
{
    stopTimer();
    if (!longPressCandidate) return;
    longPressCandidate = false;
    // The press was meant as "arrange", not as a switch: take the switch back.
    applyStopState(!stopSwipeEnable);
    stopSwipeActive = false;
    SwipeSwitch::end();
    if (arrange.begin) arrange.begin(this);
}

namespace
{
    // Dashed border whose dashes travel around the rectangle ("marching ants").
    void drawMarchingBorder(juce::Graphics& g, juce::Rectangle<float> r, float phase, juce::Colour colour)
    {
        const float dash = 9.0f, gap = 6.0f, period = dash + gap;
        const float w = r.getWidth(), h = r.getHeight(), perimeter = 2.0f * (w + h);
        if (perimeter < 4.0f) return;
        auto pointAt = [&](float d)
        {
            d = std::fmod(std::fmod(d, perimeter) + perimeter, perimeter);
            if (d < w) return juce::Point<float>(r.getX() + d, r.getY());
            d -= w;
            if (d < h) return juce::Point<float>(r.getRight(), r.getY() + d);
            d -= h;
            if (d < w) return juce::Point<float>(r.getRight() - d, r.getBottom());
            d -= w;
            return juce::Point<float>(r.getX(), r.getBottom() - d);
        };
        juce::Path path;
        for (float start = std::fmod(phase, period) - period; start < perimeter; start += period)
        {
            const float end = start + dash;
            path.startNewSubPath(pointAt(start));
            for (float d = start + 2.0f; d < end; d += 2.0f) path.lineTo(pointAt(d));
            path.lineTo(pointAt(end));
        }
        g.setColour(colour);
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }
}

void StopRow::paint(juce::Graphics& g)
{
    // An expanded row: the whole field of this stop's voicing - the button line and
    // the sliders under it - lies on a slightly darker rounded panel, so its extent
    // is plain (0.8.179). Collapsed rows stay on the bare grid.
    if (!isExpanded()) return;
    g.setColour(juce::Colours::black.withAlpha(0.16f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 6.0f);
}

void StopRow::paintOverChildren(juce::Graphics& g)
{
    // Filled-pipe copies being built for this stop: a thin orange strip along the
    // bottom of the button grows with the pipes done (the Voicer repaints at 5 Hz
    // while any stop builds). Transmissions show their source stop's progress.
    // Queued (waiting for its turn): the dark track alone; building: the orange
    // part grows; done: nothing.
    if (stop != nullptr && (stop->copiesQueued.load(std::memory_order_acquire) || stop->copiesBuilding.load(std::memory_order_acquire)))
    {
        const auto button = stopButton.getBounds().toFloat().reduced(6.0f, 0.0f);
        const float y = button.getBottom() - 5.0f;
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(button.getX(), y, button.getWidth(), 3.0f, 1.5f);
        if (stop->copiesBuilding.load(std::memory_order_acquire))
        {
            const int planned = stop->copiesPlanned.load(std::memory_order_relaxed);
            const int done = stop->copiesDone.load(std::memory_order_relaxed);
            g.setColour(onColour());
            g.fillRoundedRectangle(button.getX(), y, button.getWidth() * (planned > 0 ? juce::jlimit(0.0f, 1.0f, (float)done / (float)planned) : 0.0f), 3.0f, 1.5f);
        }
    }
    if (arrange.active && arrange.active(this))
    {
        // Inside the button's own edge, so every stop shows its own frame; the
        // per-stop phase offset keeps neighbouring frames from merging visually.
        const float offset = (float)(std::abs((stop->name + juce::String(transmissionIndex)).hashCode()) % 15);
        drawMarchingBorder(g, stopButton.getBounds().toFloat().reduced(2.5f),
            (arrange.phase ? arrange.phase() : 0.0f) + offset, juce::Colours::white.withAlpha(0.9f));
    }
}

void StopRow::mouseEnter(const juce::MouseEvent& event)
{
    if (stopSwipeActive && event.eventComponent == &stopButton
        && event.mods.isLeftButtonDown())
        applyStopState(stopSwipeEnable);
}

void StopRow::mouseDrag(const juce::MouseEvent& event)
{
    if (keySliderPaintActive)
    {
        if (event.mods.isRightButtonDown())
            paintKeySlidersAt(event.getScreenPosition());
        return;
    }
    if (arrangeDragging)
    {
        const auto position = dragStartTopLeft + (event.getScreenPosition() - pressScreenPosition);
        if (dragGhost != nullptr) dragGhost->setTopLeftPosition(position);
        else setTopLeftPosition(position);
        if (arrange.hover)
            arrange.hover(this, dragGhost != nullptr ? dragGhost->getScreenBounds().getCentre() : stopButton.getScreenBounds().getCentre(), arrangeDuplicate);
        return;
    }
    if (longPressCandidate && event.getScreenPosition().getDistanceFrom(pressScreenPosition) > 8)
    {
        longPressCandidate = false;   // moved: this is a swipe, not a long press
        stopTimer();
    }
    if (!stopSwipeActive || !event.mods.isLeftButtonDown()) return;
    const auto screenPosition = event.getScreenPosition();
    for (auto* row : stopSwipeRows)
        if (row != nullptr
            && row->stopButton.getScreenBounds().contains(screenPosition))
        {
            row->applyStopState(stopSwipeEnable);
            return;
        }
    // Not over a stop: couplers and tremulants take part in the same swipe.
    SwipeSwitch::applyAt(screenPosition);
}

void StopRow::mouseUp(const juce::MouseEvent& event)
{
    if (keySliderPaintActive)
    {
        keySliderPaintActive = false;
        lastPaintedKeySlider = -1;
        return;
    }
    if (arrangeDragging)
    {
        arrangeDragging = false;
        const bool duplicate = arrangeDuplicate;
        arrangeDuplicate = false;
        // The centre of what was dragged (the row's button, or the copy) decides the target cell.
        const auto anchor = dragGhost != nullptr ? dragGhost->getScreenBounds().getCentre() : stopButton.getScreenBounds().getCentre();
        dragGhost.reset();   // removes itself from the parent
        if (arrange.drop) arrange.drop(this, anchor, duplicate);   // may rebuild the UI
        return;
    }
    stopTimer();
    longPressCandidate = false;
    if (event.mods.isLeftButtonDown() || event.eventComponent == &stopButton)
    {
        stopSwipeActive = false;
        SwipeSwitch::end();
    }
}

int StopRow::keySliderIndexForComponent(const juce::Component* component, juce::OwnedArray<LearnableSlider>** row) const
{
    for (int index = 0; index < keySliders.size(); ++index)
        if (component == keySliders[index])
        {
            if (row != nullptr) *row = const_cast<juce::OwnedArray<LearnableSlider>*>(&keySliders);
            return index;
        }
    for (int index = 0; index < toneSliders.size(); ++index)
        if (component == toneSliders[index])
        {
            if (row != nullptr) *row = const_cast<juce::OwnedArray<LearnableSlider>*>(&toneSliders);
            return index;
        }
    return -1;
}

void StopRow::toneSliderChanged(int note, float value)
{
    if (isTransmission()) engine.setTransmissionKeyTone(stop, transmissionIndex, note, value);
    else engine.setStopNoteTone(stop, note, value);
    if (stateChanged) stateChanged();
}

void StopRow::resetKeyRow(bool tone)
{
    auto& row = tone ? toneSliders : keySliders;
    for (auto* slider : row)
        if (slider != nullptr && slider->isEnabled()) slider->setValue(0.0, juce::sendNotificationSync);
}

void StopRow::paintKeySlidersAt(juce::Point<int> screenPosition)
{
    // The row being painted (volume or tone); the sliders of both rows share indices.
    auto& sliders = paintRow != nullptr ? *paintRow : this->keySliders;
    if (sliders.isEmpty()) return;

    int index = -1;
    for (int candidate = 0; candidate < sliders.size(); ++candidate)
    {
        auto* slider = sliders[candidate];
        if (slider != nullptr && slider->isEnabled())
        {
            const auto bounds = slider->getScreenBounds();
            if (screenPosition.x >= bounds.getX() && screenPosition.x < bounds.getRight())
            {
                index = candidate;
                break;
            }
        }
    }
    if (index < 0) return;

    auto* targetSlider = sliders[index];
    const auto targetBounds = targetSlider->getScreenBounds();
    const int trackTop = targetBounds.getY() + 4;
    const int trackBottom = targetBounds.getBottom() - 20;
    const double vertical = juce::jlimit(0.0, 1.0,
        (screenPosition.y - trackTop) / (double)juce::jmax(1, trackBottom - trackTop));
    const double targetValue = targetSlider->proportionOfLengthToValue(1.0 - vertical);

    auto setSliderValue = [&sliders](int sliderIndex, double value)
    {
        if (sliderIndex < 0 || sliderIndex >= sliders.size()) return;
        if (auto* slider = sliders[sliderIndex]; slider != nullptr && slider->isEnabled())
            slider->setValue(value, juce::sendNotificationSync);
    };

    const bool enteredNewSlider = lastPaintedKeySlider != index;

    // Interpolate every skipped slider when the mouse moves quickly, so the
    // painted curve has no holes.
    if (lastPaintedKeySlider >= 0 && lastPaintedKeySlider != index)
    {
        const int distance = std::abs(index - lastPaintedKeySlider);
        const int direction = index > lastPaintedKeySlider ? 1 : -1;
        for (int step = 1; step <= distance; ++step)
        {
            const double amount = step / (double)distance;
            setSliderValue(lastPaintedKeySlider + step * direction,
                lastPaintedValue + (targetValue - lastPaintedValue) * amount);
        }
    }
    else
    {
        setSliderValue(index, targetValue);
    }

    // Apply the very light smoothing brush only once when entering a new
    // slider. Reapplying it during a vertical drag would cumulatively pull the
    // neighbours almost all the way to the directly painted value.
    static constexpr std::array<int, 4> neighbourOffsets{ -2, -1, 1, 2 };
    if (enteredNewSlider)
    {
        for (int offset : neighbourOffsets)
        {
            const int neighbour = index + offset;
            if (neighbour < 0 || neighbour >= sliders.size()) continue;
            if (auto* slider = sliders[neighbour]; slider != nullptr && slider->isEnabled())
            {
                const double weight = std::abs(offset) == 1 ? 0.10 : 0.025;
                setSliderValue(neighbour, slider->getValue()
                    + (targetValue - slider->getValue()) * weight);
            }
        }
        verticalPaintAnchorValue = targetValue;
        for (size_t neighbourIndex = 0; neighbourIndex < neighbourOffsets.size(); ++neighbourIndex)
        {
            const int neighbour = index + neighbourOffsets[neighbourIndex];
            verticalNeighbourAnchors[neighbourIndex] = neighbour >= 0 && neighbour < sliders.size()
                && sliders[neighbour] != nullptr ? sliders[neighbour]->getValue() : 0.0;
        }
    }
    else
    {
        // During a vertical right-drag, neighbours follow only a small fraction
        // of the main movement. Anchoring the calculation prevents repeated
        // mouse events from accumulating until all sliders become equal.
        const double movement = targetValue - verticalPaintAnchorValue;
        for (size_t neighbourIndex = 0; neighbourIndex < neighbourOffsets.size(); ++neighbourIndex)
        {
            const int offset = neighbourOffsets[neighbourIndex];
            const double weight = std::abs(offset) == 1 ? 0.08 : 0.02;
            setSliderValue(index + offset,
                verticalNeighbourAnchors[neighbourIndex] + movement * weight);
        }
    }

    lastPaintedKeySlider = index;
    lastPaintedValue = targetValue;
}

void StopRow::refreshUndulationLabel()
{
    // Greyed out (like the key sliders without a sample) until the 440 Hz diapason is on.
    undulationLabel.setColour(juce::Label::textColourId,
        stop->tuneTo440 ? juce::Colours::lightgrey : juce::Colours::grey);
    undulationSlider.setEnabled(stop->tuneTo440);
    undulationSlider.setAlpha(stop->tuneTo440 ? 1.0f : 0.4f);
    if (!stop->tuneTo440)
    {
        undulationLabel.setText("Undulation (beats/s at a1) - needs 440 Hz", juce::dontSendNotification);
        return;
    }
    if (std::abs(stop->undulationBeats) < 1.0e-3f)
    {
        undulationLabel.setText("Undulation (beats/s at a1): off", juce::dontSendNotification);
        return;
    }
    int lowest = 128, highest = -1;
    for (int note = 0; note < 128; ++note)
        if (stop->hasNote(note)) { lowest = juce::jmin(lowest, note); highest = juce::jmax(highest, note); }
    juce::String text = "Undulation (beats/s at a1)";
    if (highest >= 0)
        text << ": " << juce::String(stop->undulationCents[(size_t)lowest], 1) << " c bottom, "
             << juce::String(stop->undulationCents[(size_t)highest], 1) << " c top";
    undulationLabel.setText(text, juce::dontSendNotification);
}

void StopRow::refreshEnabledColour()
{
    stopButton.setColour(juce::TextButton::buttonColourId,
        routeEnabled() ? onColour() : juce::Colours::darkgrey);
}

void StopRow::applyStopState(bool enabled)
{
    if (routeEnabled() == enabled) return;
    if (isTransmission()) engine.setTransmissionEnabled(stop, transmissionIndex, enabled);
    else
    {
        engine.setStopEnabled(stop, enabled);
        // A stop folder switches its children with it - stops and transmissions alike.
        if (childrenGetter) for (auto* child : childrenGetter(stop)) engine.setStopEnabled(child, enabled);
        if (childTransmissionsGetter)
            for (const auto& [source, index] : childTransmissionsGetter(stop)) engine.setTransmissionEnabled(source, index, enabled);
        // ... and their buttons light up or go out with it (they are on the grid
        // under the master's open voicing); each can still be switched alone.
        if (childrenGetter || childTransmissionsGetter)
            for (auto* row : stopSwipeRows)
                if (row != this) row->refreshEnabledColour();
    }
    refreshEnabledColour();
    if (stateChanged) stateChanged();
}

void StopRow::resized()
{
    auto area = getLocalBounds();

    // Line 1: stop button, transmissions, ranks, 440 Hz / diapason / temperament,
    // alias. Transmission rows: button, transposition, source stop, alias.
    stopButton.setBounds(margin, 0, buttonWidth, topRowHeight);
    voicingToggle.setBounds(margin + 1, topRowHeight - 19, 18, 18);   // bottom-left corner of every button
    removeButton.setBounds(margin + 1, 1, 18, 18);                     // top-left corner, arrange mode only
    removeButton.setVisible(arrange.active && arrange.active(this));
    int x = margin + buttonWidth + sliderSpacing;
    // The Transposition box right after the button (a transmission's shift, or the
    // stop's own - 0.8.158); a transmission row shows its source stop after it.
    if (isTransmission() || !stop->container)
    {
        transpositionLabel.setBounds(x, 13, 100, 23);
        x += 104;
        shiftBox.setBounds(x, 13, 250, 23);
        x += 258;
        // The One-shot switch of a stop with pipes right after its transposition
        // (0.8.179; it stood after the alias): what the stop IS belongs together.
        if (!isTransmission())
        {
            oneShotToggle.setBounds(x, 13, 92, 23);
            x += 96;
        }
    }
    if (isTransmission())
    {
        sourceLabel.setBounds(x, 13, 520, 23);
        x += 528;
    }
    else
    {
        // The line's parts sit close together: the rank boxes only when there are
        // pipes, the pitch information as wide as its text.
        if (!stop->container && !rankButtons.isEmpty())
        {
            ranksLabel.setBounds(x, 13, 92, 23);
            x += 92;
            for (auto* button : rankButtons)
            {
                button->setBounds(x, 13, 80, 23);
                x += 82;
            }
            x += sliderSpacing;
        }
        const int textWidth = (int)std::ceil(juce::GlyphArrangement::getStringWidth(diapasonLabel.getFont(), diapasonLabel.getText()));
        const int diapasonWidth = juce::jlimit(120, 900, textWidth + 12);
        diapasonLabel.setBounds(x, 13, diapasonWidth, 23);   // "Orig. Pitch: a1 = ...; temperament = ..."
        x += diapasonWidth + 10;
    }
    aliasLabel.setBounds(x, 13, 46, 23);
    x += 48;
    aliasBox.setBounds(x, 13, 190, 23);
    x += 196;
    if (!isTransmission())
    {
        // After the alias: the rank count of a stop folder (shown for stop folders
        // only) - it belongs to the caption, like the alias.
        if (rankCountToggle.isVisible())
        {
            rankCountToggle.setBounds(x, 13, 140, 23);
            x += 146;
        }
    }

    // A stop folder with pipes of its own: its voicing steps right of the tree
    // lines that join its children to it.
    const int inset = (!isTransmission() && ((childrenGetter && !childrenGetter(stop).empty())
        || (childTransmissionsGetter && !childTransmissionsGetter(stop).empty()))) ? 26 : 0;

    // Line 2 (expanded rows only): the voicing sliders, each with its caption
    // above, the slider below it and the value box right of the slider (the
    // same arrangement as the sliders of the Division settings dialog).
    const int line2 = topRowHeight + 4;
    x = margin + inset;
    auto placeSlider = [&](juce::Slider& slider, juce::Label& label, int width)
    {
        label.setBounds(x, line2, width, 16);
        slider.setBounds(x, line2 + 17, width, 26);
        x += width + voicingSliderGap;
    };
    placeSlider(globalSlider, globalLabel, globalSliderWidth);
    if (isTransmission()) placeSlider(formantSlider, formantLabel, formantSliderWidth);   // the only other slider of a transmission
    placeSlider(gainSlider, gainLabel, gainSliderWidth);
    placeSlider(attackSlider, attackLabel, attackSliderWidth);
    placeSlider(randomPitchSlider, randomPitchLabel, randomPitchSliderWidth);
    placeSlider(undulationSlider, undulationLabel, undulationSliderWidth);
    if (!isTransmission()) placeSlider(formantSlider, formantLabel, formantSliderWidth);
    // Velocity layers after the sliders (only stops that have "_NNN" files). A
    // column too narrow for them: the boxes wrap onto a third line (the owner
    // sized the row for it, see expandedHeightFor).
    int velocityLine = 0;
    {
        const bool showLayers = isExpanded() && !velocityButtons.isEmpty();
        const int needed = 104 + 66 * velocityButtons.size();
        if (showLayers && x + needed > getWidth()) { x = margin + inset; velocityLine = velocityLineHeight; }
        const int layerY = line2 + 17 + velocityLine;
        velocityLabel.setVisible(showLayers);
        velocityLabel.setBounds(x, layerY, 100, 26);
        x += 104;
        for (auto* button : velocityButtons)
        {
            button->setVisible(showLayers);
            button->setBounds(x, layerY, 62, 26);
            x += 66;
        }
    }

    // Bottom area: the two per-key rows, each under its folding header
    // ("Volume per key": MIDI number, note name, slider; "Treble per key": note
    // name, slider). A folded row shows its header only. The folding hooks are
    // set after construction, so the visibility is refreshed here.
    {
        const bool showVoicing = isExpanded() && !stop->container;
        const bool showVolumeRow = showVoicing && volumeRowShown();
        const bool showToneRow = showVoicing && toneRowShown();
        volumeHeader.setSection("Volume per key", volumeRowShown());
        toneHeader.setSection("Treble per key", toneRowShown());
        volumeHeader.setVisible(showVoicing); volumeReset.setVisible(showVolumeRow);
        toneHeader.setVisible(showVoicing); toneReset.setVisible(showToneRow);
        for (auto* slider : keySliders) slider->setVisible(showVolumeRow);
        for (auto* label : noteNameLabels) label->setVisible(showVolumeRow);
        for (auto* label : midiNumLabels) label->setVisible(showVolumeRow);
        for (auto* box : pipeBoxes) box->setVisible(showVolumeRow);
        for (auto* slider : toneSliders) slider->setVisible(showToneRow);
        for (auto* label : toneNameLabels) label->setVisible(showToneRow);
    }
    int y = expandedControlsHeight + velocityLine + 4;
    volumeHeader.setBounds(margin + inset, y, 170, keyHeaderHeight);
    volumeReset.setBounds(margin + inset + 172, y + 2, 92, keyHeaderHeight - 4);
    y += keyHeaderHeight;
    if (volumeRowShown())
    {
        x = margin + inset;
        for (int note = noteMin; note <= noteMax; ++note)
        {
            const int idx = note - noteMin;
            if (auto* slider = keySliders[idx])
            {
                slider->setBounds(x, y + keyLabelsHeight, sliderWidth, keySliderHeight);
                if (auto* numLabel = midiNumLabels[idx]) numLabel->setBounds(x, y, sliderWidth, 12);
                if (auto* nameLabel = noteNameLabels[idx]) nameLabel->setBounds(x, y + 12, sliderWidth, 12);
                // The tick box is drawn 4 px into the button: shift it so the box is centred.
                if (auto* box = pipeBoxes[idx]) box->setBounds(x + sliderWidth / 2 - 10, y + 24, sliderWidth - 4, 16);
            }
            x += sliderWidth;
        }
        y += keyLabelsHeight + keySliderHeight;
    }
    toneHeader.setBounds(margin + inset, y, 170, keyHeaderHeight);
    toneReset.setBounds(margin + inset + 172, y + 2, 92, keyHeaderHeight - 4);
    y += keyHeaderHeight;
    if (toneRowShown())
    {
        x = margin + inset;
        for (int note = noteMin; note <= noteMax; ++note)
        {
            const int idx = note - noteMin;
            if (auto* slider = toneSliders[idx])
            {
                slider->setBounds(x, y + toneLabelsHeight, sliderWidth, keySliderHeight);
                if (auto* nameLabel = toneNameLabels[idx]) nameLabel->setBounds(x, y, sliderWidth, 12);
            }
            x += sliderWidth;
        }
    }
}


void StopRow::updateFromStop()
{
    if (auto* t = transmission())
    {
        globalSlider.setValue(t->db, juce::dontSendNotification);
        for (int note = noteMin; note <= noteMax; ++note)
        {
            if (auto* slider = keySliders[note - noteMin])
                slider->setValue(t->keyDb[(size_t)note], juce::dontSendNotification);
            if (auto* slider = toneSliders[note - noteMin])
                slider->setValue(t->keyTone[(size_t)note], juce::dontSendNotification);
        }
        shiftBox.setSelectedId(t->shift + Stop::Transmission::maxShift + 1, juce::dontSendNotification);
        formantSlider.setValue(100.0 * t->formantFollow, juce::dontSendNotification);
        stopButton.setButtonText(caption());
        stopButton.setColour(juce::TextButton::buttonColourId,
            t->enabled ? onColour() : juce::Colours::darkgrey);
        return;
    }
    globalSlider.setValue(stop->db, juce::sendNotification);
    gainSlider.setValue(stop->gainDb, juce::sendNotification);
    attackSlider.setValue(stop->attackMs + stop->attackFadeoutMs, juce::sendNotification);
    randomPitchSlider.setValue(stop->randomPitchErrorCents, juce::dontSendNotification);
    formantSlider.setValue(100.0 * stop->formantFollow, juce::dontSendNotification);
    for (int note = noteMin; note <= noteMax; ++note)
    {
        int idx = note - noteMin;
        if (auto* slider = keySliders[idx])
            slider->setValue(stop->getNoteDb(note), juce::sendNotification);
        if (auto* slider = toneSliders[idx])
            slider->setValue(stop->getNoteToneDb(note), juce::dontSendNotification);
    }
    stopButton.setButtonText(caption());
    stopButton.setColour(juce::TextButton::buttonColourId,
        stop->enabled ? onColour() : juce::Colours::darkgrey);
    std::vector<Stop*> rankSources{ stop };
    for (auto& rank : stop->ranks) rankSources.push_back(rank.get());
    for (int i = 0; i < rankButtons.size() && i < (int)rankSources.size(); ++i)
        rankButtons[i]->setToggleState(rankSources[(size_t)i]->rankEnabled,
            juce::dontSendNotification);
    for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual)
    {
        primaryManualButtons[(size_t)manual].setToggleState(
            stop->primaryManual == manual, juce::dontSendNotification);
        transmissionBoxes[(size_t)manual].setSelectedId(
            juce::jlimit(0, 6, stop->transmissionCount(manual)) + 1, juce::dontSendNotification);
    }
}

void StopRow::globalSliderChanged()
{
    if (isTransmission()) engine.setTransmissionDb(stop, transmissionIndex, (float)globalSlider.getValue());
    else stop->setDb((float)globalSlider.getValue());
    if (stateChanged) stateChanged();
}

void StopRow::gainSliderChanged()
{
    juce::ScopedLock sl(engine.getLock());
    stop->gainDb = (float)gainSlider.getValue();
    if (stateChanged) stateChanged();
}

void StopRow::attackSliderChanged()
{
    // Half of the time the attack gain holds, half it fades back.
    juce::ScopedLock sl(engine.getLock());
    stop->attackMs = (float)attackSlider.getValue() * 0.5f;
    stop->attackFadeoutMs = stop->attackMs;
    if (stateChanged) stateChanged();
}

void StopRow::formantSliderChanged()
{
    const float share = (float)(formantSlider.getValue() / 100.0);
    if (isTransmission()) engine.setTransmissionFormantFollow(stop, transmissionIndex, share);
    else engine.setStopFormantFollow(stop, share);
    if (stateChanged) stateChanged();
}

void StopRow::randomPitchSliderChanged()
{
    engine.setStopRandomPitchError(stop, (float)randomPitchSlider.getValue(), true);
    if (stateChanged) stateChanged();
}

void StopRow::keySliderChanged(int note, float value)
{
    if (isTransmission()) engine.setTransmissionKeyDb(stop, transmissionIndex, note, value);
    else
    {
        juce::ScopedLock sl(engine.getLock());
        stop->setNoteDb(note, value);
    }
    if (stateChanged) stateChanged();
}

void StopRow::setNoteActive(int note, bool active)
{
    // Only process if this stop actually sounds this key (its own pipe or a filled one)
    const int pipe = pipeForKey(note);
    if (pipe < 0 || pipe >= 128 || (!stop->hasNoteInAnyRank(pipe) && !stop->isFilledInAnyRank(pipe))) return;

    int idx = note - noteMin;
    if (idx < 0 || idx >= keySliders.size()) return;
    auto* slider = keySliders[idx];
    if (!slider) return;
    auto* tone = idx < toneSliders.size() ? toneSliders[idx] : nullptr;   // the treble row lights up as well

    if (active)
    {
        int& count = activeNoteCount[note];
        if (count == 0)
        {
            // First activation: highlight (the original colours are in originalTrackColours)
            for (auto* s : { slider, tone })
                if (s != nullptr && originalTrackColours.find(s) != originalTrackColours.end())
                    s->setColour(juce::Slider::trackColourId, juce::Colours::lightgreen);
        }
        ++count;
    }
    else
    {
        auto it = activeNoteCount.find(note);
        if (it != activeNoteCount.end() && it->second > 0)
        {
            if (--(it->second) == 0)
            {
                // Last release: restore original colours
                for (auto* s : { slider, tone })
                {
                    if (s == nullptr) continue;
                    auto colIt = originalTrackColours.find(s);
                    if (colIt != originalTrackColours.end()) s->setColour(juce::Slider::trackColourId, colIt->second);
                }
                activeNoteCount.erase(it);
            }
        }
    }
}

//==============================================================================
Voicer::Voicer(SamplerEngine& engine, std::function<void(const juce::String&)> midiLearnCallback,
    std::function<void(int)> folderCallback, std::function<void(int)> unloadCallback,
    std::function<void(int)> settingsCallback, std::function<void(int)> keyboardLearn,
    std::function<float(int)> swellGetter, std::function<void(int, float)> swellSetter,
    std::function<void(int)> swellLearn, std::function<bool(int)> swellLearning,
    std::function<void()> stateChanged,
    std::function<std::vector<DivisionCouplerInfo>(int)> couplerGetter,
    std::function<void(const juce::String&, bool)> couplerSetter,
    std::function<bool(int)> tremulantGetter,
    std::function<void(int, bool)> tremulantSetter,
    std::function<float(int)> columnWeightGetter,
    std::function<void(int, float, int, float)> columnWeightSetter,
    std::function<void(int)> stopBuildCallback)
    : engine(engine), midiLearnCallback(std::move(midiLearnCallback)),
      folderCallback(std::move(folderCallback)), unloadCallback(std::move(unloadCallback)),
      settingsCallback(std::move(settingsCallback)), keyboardLearn(std::move(keyboardLearn)),
      swellGetter(std::move(swellGetter)), swellSetter(std::move(swellSetter)),
      swellLearn(std::move(swellLearn)), swellLearning(std::move(swellLearning)),
      stateChanged(std::move(stateChanged)), couplerGetter(std::move(couplerGetter)),
      couplerSetter(std::move(couplerSetter)), tremulantGetter(std::move(tremulantGetter)),
      tremulantSetter(std::move(tremulantSetter)),
      columnWeightGetter(std::move(columnWeightGetter)),
      columnWeightSetter(std::move(columnWeightSetter)),
      stopBuildCallback(std::move(stopBuildCallback))
{
    computeNoteRange();
    // The copy-build strips: every row looks at its stop five times a second.
    struct ProgressAnimator final : public juce::Timer
    {
        explicit ProgressAnimator(Voicer& v) : voicer(v) {}
        void timerCallback() override
        {
            for (auto* row : voicer.stopRows) row->pollBuildProgress();
            // Every five seconds, when no build runs: a build that ended before its
            // last pipe and was not launched again is resumed, so that no filled
            // pipe stays without its copy.
            if (++tick % 25 == 0 && voicer.engine.pendingBackgroundWorkers() == 0)
            {
                const int resumed = voicer.engine.resumeIncompleteCopyBuilds();
                if (resumed > 0) CrashHandler::note("Copy builds resumed for " + juce::String(resumed) + " stop(s)");
            }
        }
        Voicer& voicer;
        int tick = 0;
    };
    progressAnimator = std::make_unique<ProgressAnimator>(*this);
    progressAnimator->startTimerHz(5);
}

void StopRow::pollBuildProgress()
{
    const bool building = stop != nullptr
        && (stop->copiesBuilding.load(std::memory_order_acquire) || stop->copiesQueued.load(std::memory_order_acquire));
    if (building || buildingSeen) repaint(stopButton.getBounds());
    buildingSeen = building;
}

Voicer::~Voicer()
{
}

void Voicer::computeNoteRange()
{
    noteMin = 128;
    noteMax = 0;
    for (auto* stop : engine.getStops())
        for (int note = 0; note < 128; ++note)
            if (stop->hasNoteInAnyRank(note))
            {
                noteMin = juce::jmin(noteMin, note);
                noteMax = juce::jmax(noteMax, note);
            }
    if (noteMin > noteMax)
    {
        noteMin = 24;
        noteMax = 96;
    }
}

void Voicer::rebuildUI(juce::Component& container, juce::Viewport& viewport)
{
    // The first screen is the main window's workspace; a second one (another
    // program window) receives the divisions assigned to it.
    stopRows.clear();
    sectionHeaders.clear();
    sectionDividers.clear();
    compactCells.clear();
    arrangeOverlays.clear();
    uiContainer = &container;
    uiViewport = &viewport;
    layoutScreen(0, container, viewport);
    if (screenContainers[1] != nullptr && screenViewports[1] != nullptr)
        layoutScreen(1, *screenContainers[1], *screenViewports[1]);
    // Development (0.8.153): ORGAN_DUMP_GRID=1 logs every grid item's button and
    // every free cell in SCREEN coordinates once the layout has settled - a test
    // script drives the mouse by them (the drag between divisions, #77).
    if (std::getenv("ORGAN_DUMP_GRID") != nullptr)
        juce::Timer::callAfterDelay(1500, [this]
        {
            for (auto* row : stopRows)
            {
                if (row == nullptr || row->getStop() == nullptr) continue;
                const auto b = row->getStopButtonBounds().translated(row->getX(), row->getY());
                auto* parent = row->getParentComponent();
                const auto s = parent != nullptr ? parent->localAreaToGlobal(b) : b;
                int firstNote = -1, lastNote = -1;
                for (int note = 0; note < 128; ++note)
                    if (row->getStop()->hasNoteInAnyRank(note)) { if (firstNote < 0) firstNote = note; lastNote = note; }
                CrashHandler::note("Grid item: manual " + juce::String(row->getStop()->primaryManual) + (row->isTransmission() ? " transmission " : " stop ")
                    + row->getStop()->name + " button " + s.toString() + " notes " + juce::String(firstNote) + ".." + juce::String(lastNote)
                    + (row->getStop()->sampleOffset != 0 ? " transposition " + juce::String(row->getStop()->sampleOffset) : juce::String())
                    + (row->getStop()->footageKnown ? " footage " + juce::String(row->getStop()->footageSemitones) : juce::String(" footage unknown"))
                    + " caption \"" + row->getStop()->displayName() + "\""
                    + (row->getStop()->isOneShot ? " one-shot" : "")
                    + (row->isExpanded() ? " EXPANDED " + row->describeLineOne() : juce::String()));
                if (const char* png = std::getenv("ORGAN_TEST_SNAPSHOT"); png != nullptr && row->isExpanded())
                {
                    const juce::Image snapshot = row->createComponentSnapshot(row->getLocalBounds(), false, 1.0f);
                    juce::Image image(juce::Image::RGB, snapshot.getWidth(), snapshot.getHeight(), true);
                    {
                        juce::Graphics g(image);   // the window's dark background under the white texts
                        g.fillAll(juce::Colour(0xff2b2b2b));
                        g.drawImageAt(snapshot, 0, 0);
                    }
                    const juce::File file = juce::File(juce::String(png) + ".row.png");
                    file.deleteFile();
                    juce::FileOutputStream stream(file);
                    if (stream.openedOk()) juce::PNGImageFormat().writeImageToStream(image, stream);
                }
            }
            for (const auto& cell : compactCells)
            {
                if (cell.key.isNotEmpty()) continue;
                auto* cellContainer = containerOf(cell.manual);
                if (cellContainer == nullptr) cellContainer = uiContainer;
                if (cellContainer == nullptr) continue;
                CrashHandler::note("Grid free cell: manual " + juce::String(cell.manual) + " cell " + juce::String(cell.column) + "," + juce::String(cell.row)
                    + " " + cellContainer->localAreaToGlobal(cell.bounds).toString());
            }
            // ORGAN_TEST_DROP="<manual>|<stop name>|<target manual>|<column>|<row>": the stop
            // is dropped on that cell exactly as the mouse would drop it (once per run);
            // ORGAN_TEST_NEW_FOLDER="<manual>|<column>|<row>": a new stop folder there.
            static bool dropDone = false, folderDone = false;
            auto cellCentre = [this](int target, int column, int row, juce::Point<int>& where)
            {
                for (const auto& cell : compactCells)
                    if (cell.manual == target && cell.column == column && cell.row == row)
                    {
                        auto* c = containerOf(cell.manual);
                        if (c == nullptr) c = uiContainer;
                        if (c == nullptr) return false;
                        where = c->localAreaToGlobal(cell.bounds).getCentre();
                        return true;
                    }
                return false;
            };
            if (const char* drop = std::getenv("ORGAN_TEST_DROP"); drop != nullptr && !dropDone && !compactCells.empty())
            {
                const auto parts = juce::StringArray::fromTokens(juce::String(drop), "|", "");
                if (parts.size() == 5)
                {
                    const int manual = parts[0].getIntValue(), target = parts[2].getIntValue();
                    StopRow* dragged = nullptr;
                    for (auto* r : stopRows)
                        if (r != nullptr && !r->isTransmission() && r->getStop() != nullptr && r->getStop()->primaryManual == manual && r->getStop()->name == parts[1]) dragged = r;
                    juce::Point<int> where;
                    const bool found = cellCentre(target, parts[3].getIntValue(), parts[4].getIntValue(), where);
                    CrashHandler::note("ORGAN_TEST_DROP: " + parts[1] + " (row " + (dragged != nullptr ? "found" : "MISSING") + ") from division " + juce::String(manual)
                        + " to division " + juce::String(target) + " cell " + parts[3] + "," + parts[4] + (found ? " at " + where.toString() : juce::String(" - cell MISSING")));
                    if (dragged != nullptr && found)
                    {
                        dropDone = true;
                        enterArrangeMode(manual);
                        dropItem(manual, parts[1], dragged, where, false);
                    }
                }
            }
            // ORGAN_TEST_EXPAND="<manual>|<stop name>" opens that stop's voicing (once);
            // ORGAN_TEST_SNAPSHOT=<png> writes a picture of the grid after every layout.
            static bool expandDone = false;
            if (const char* expand = std::getenv("ORGAN_TEST_EXPAND"); expand != nullptr && !expandDone && !compactCells.empty())
            {
                const auto parts = juce::StringArray::fromTokens(juce::String(expand), "|", "");
                for (auto* r : stopRows)
                    if (parts.size() == 2 && r != nullptr && !r->isTransmission() && r->getStop() != nullptr
                        && r->getStop()->primaryManual == parts[0].getIntValue() && r->getStop()->name.startsWith(parts[1]))
                    {
                        expandDone = true;
                        r->getStop()->voicingExpanded = true;
                        CrashHandler::note("ORGAN_TEST_EXPAND: " + parts[1]);
                        juce::MessageManager::callAsync([this] { if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport); });
                    }
            }
            // ORGAN_TEST_OFFSET="<manual>|<stop name>|<offset>": "Files vs keys" as the voicing box would set it (once).
            // ORGAN_TEST_TRANSPOSITION="<manual>|<stop prefix>|<semitones>" (tests): the voicing's Transposition path.
            static bool offsetDone = false;
            if (const char* test = std::getenv("ORGAN_TEST_TRANSPOSITION"); test != nullptr && !offsetDone && !compactCells.empty())
            {
                const auto parts = juce::StringArray::fromTokens(juce::String(test), "|", "");
                for (auto* r : stopRows)
                    if (parts.size() == 3 && r != nullptr && !r->isTransmission() && r->getStop() != nullptr
                        && r->getStop()->primaryManual == parts[0].getIntValue() && r->getStop()->name.startsWith(parts[1]))
                    {
                        offsetDone = true;
                        CrashHandler::note("ORGAN_TEST_TRANSPOSITION: " + r->getStop()->name + " -> " + parts[2]);
                        if (transpositionRequest) transpositionRequest(r->getStop(), parts[2].getIntValue());
                        break;
                    }
            }
            // ORGAN_TEST_RECORD="<manual>|<key>|<velocity>|<seconds>|<out.wav>|<stop prefix>" (dev, 0.8.162):
            // once the copies are built, every stop and transmission is switched off but the
            // stops named like the prefix, the engine records its routes' output to the file
            // while the key is played on that division and released (1.5 s of tail) - what
            // the user hears, through the whole GUI path (device, voicing, settings).
            static bool recordDone = false;
            if (const char* rec = std::getenv("ORGAN_TEST_RECORD"); rec != nullptr && !recordDone && !compactCells.empty())
            {
                recordDone = true;
                const auto parts = juce::StringArray::fromTokens(juce::String(rec), "|", "");
                if (parts.size() >= 6)
                {
                    const int manual = parts[0].getIntValue(), key = parts[1].getIntValue(), velocity = juce::jlimit(1, 127, parts[2].getIntValue());
                    const double seconds = parts[3].getDoubleValue();
                    const juce::File file = juce::File(parts[4]);
                    const juce::String prefix = parts[5];
                    const bool noTune = parts.size() > 6 && parts[6] == "notune";   // |notune: the stop's tune-to-440 off, random detune 0
                    SamplerEngine* eng = &engine;
                    auto step = std::make_shared<std::function<void()>>();
                    *step = [eng, manual, key, velocity, seconds, file, prefix, noTune, step]
                    {
                        if (eng->pendingBackgroundWorkers() > 0) { juce::Timer::callAfterDelay(500, [step] { if (*step) (*step)(); }); return; }
                        int on = 0;
                        for (auto* stop : eng->getStops())
                        {
                            const bool wanted = stop->name.startsWithIgnoreCase(prefix);
                            eng->setStopEnabled(stop, wanted);
                            if (wanted) ++on;
                            if (wanted && noTune) { stop->tuneTo440 = false; stop->randomPitchErrorCents = 0.0f; }
                            for (int i = 0; i < (int)stop->transmissions.size(); ++i) eng->setTransmissionEnabled(stop, i, false);
                        }
                        CrashHandler::note("ORGAN_TEST_RECORD: copies built, " + juce::String(on) + " stop(s) \"" + prefix + "\" on, recording " + file.getFullPathName()
                            + ": key " + juce::String(key) + " on division " + juce::String(manual) + " at velocity " + juce::String(velocity) + " for " + juce::String(seconds, 1) + " s");
                        juce::Timer::callAfterDelay(1000, [eng, manual, key, velocity, seconds, file, step]
                        {
                            const bool started = eng->startRecording(file);
                            CrashHandler::note(juce::String("ORGAN_TEST_RECORD: recording ") + (started ? "started" : "FAILED to start"));
                            juce::Timer::callAfterDelay(800, [eng, manual, key, velocity, seconds, file, step]
                            {
                                eng->noteOn(key, velocity, manual);
                                juce::Timer::callAfterDelay((int)(seconds * 1000.0), [eng, manual, key, file, step]
                                {
                                    eng->noteOff(key, manual);
                                    juce::Timer::callAfterDelay(1500, [eng, file, step]
                                    {
                                        eng->stopRecording();
                                        CrashHandler::note("ORGAN_TEST_RECORD: done, " + juce::String(file.getSize()) + " bytes in " + file.getFullPathName());
                                        juce::MessageManager::callAsync([step] { *step = nullptr; });
                                    });
                                });
                            });
                        });
                    };
                    (*step)();
                }
            }
            if (const char* png = std::getenv("ORGAN_TEST_SNAPSHOT"); png != nullptr && uiContainer != nullptr)
            {
                const juce::Image image = uiContainer->createComponentSnapshot(uiContainer->getLocalBounds(), false, 1.0f);
                const juce::File file = juce::File(juce::String(png));   // not File file(String(x)): the vexing parse
                file.deleteFile();
                juce::FileOutputStream stream(file);
                if (stream.openedOk()) juce::PNGImageFormat().writeImageToStream(image, stream);
                CrashHandler::note("ORGAN_TEST_SNAPSHOT: " + file.getFullPathName() + " " + juce::String(image.getWidth()) + "x" + juce::String(image.getHeight()));
            }
            if (const char* folder = std::getenv("ORGAN_TEST_NEW_FOLDER"); folder != nullptr && !folderDone && !compactCells.empty())
            {
                folderDone = true;
                const auto parts = juce::StringArray::fromTokens(juce::String(folder), "|", "");
                if (parts.size() == 3 && newStopFolderRequest)
                {
                    CrashHandler::note("ORGAN_TEST_NEW_FOLDER: division " + parts[0] + " cell " + parts[1] + "," + parts[2]);
                    enterArrangeMode(parts[0].getIntValue());
                    newStopFolderRequest(parts[0].getIntValue(), parts[1].getIntValue(), parts[2].getIntValue());
                }
            }
        });
}

void Voicer::layoutScreen(int screen, juce::Component& container, juce::Viewport& viewport)
{
    const auto oldViewPosition = viewport.getViewPosition();
    class DivisionDivider final : public juce::Component
    {
    public:
        DivisionDivider(int leftManual, int rightManual, int leftWidth, int rightWidth,
            float leftWeight, float rightWeight,
            std::function<void(int, float, int, float)> setter)
            : leftManual(leftManual), rightManual(rightManual), leftWidth(leftWidth),
              rightWidth(rightWidth), leftWeight(leftWeight), rightWeight(rightWeight),
              setter(std::move(setter))
        {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        }

        void paint(juce::Graphics& g) override
        {
            g.setColour(isMouseOverOrDragging() ? juce::Colours::orange.withAlpha(0.85f)
                                                : juce::Colours::grey.withAlpha(0.65f));
            g.fillRect(getLocalBounds().withSizeKeepingCentre(2, getHeight()));
        }

        void mouseDown(const juce::MouseEvent&) override
        {
            startX = getX();
            dragDelta = 0;
        }

        void mouseDrag(const juce::MouseEvent& event) override
        {
            dragDelta = juce::jlimit(220 - leftWidth, rightWidth - 220,
                event.getDistanceFromDragStartX());
            setTopLeftPosition(startX + dragDelta, getY());
            repaint();
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (!setter || dragDelta == 0) return;
            const float combinedWeight = juce::jmax(0.3f, leftWeight + rightWeight);
            const float fraction = juce::jlimit(0.05f, 0.95f,
                (float)(leftWidth + dragDelta) / (float)juce::jmax(1, leftWidth + rightWidth));
            auto callback = setter;
            const int left = leftManual, right = rightManual;
            const float newLeft = combinedWeight * fraction;
            const float newRight = combinedWeight - newLeft;
            juce::MessageManager::callAsync([callback, left, right, newLeft, newRight]
            {
                callback(left, newLeft, right, newRight);
            });
        }

    private:
        int leftManual, rightManual, leftWidth, rightWidth;
        int startX = 0, dragDelta = 0;
        float leftWeight, rightWeight;
        std::function<void(int, float, int, float)> setter;
    };

    class SectionHeader final : public juce::Component, private juce::Timer
    {
    public:
        class SwellBar final : public juce::Component
        {
        public:
            SwellBar(int manual, std::function<float(int)> getter,
                std::function<void(int, float)> setter, std::function<void(int)> learn,
                std::function<bool(int)> learning)
                : manual(manual), getter(std::move(getter)), setter(std::move(setter)),
                  learn(std::move(learn)), learning(std::move(learning)) {}
            void paint(juce::Graphics& g) override
            {
                // Console display look: black rounded panel with a dark grey frame, lime content.
                const auto panel = getLocalBounds().toFloat().reduced(1.0f);
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(panel, 3.0f);
                g.setColour(juce::Colours::darkgrey);
                g.drawRoundedRectangle(panel, 3.0f, 1.0f);
                if (learning && learning(manual))
                {
                    g.setColour(juce::Colours::lime);
                    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
                    g.drawText("LEARN", getLocalBounds(), juce::Justification::centred);
                    return;
                }
                const int segments = 18;
                const int lit = juce::roundToInt(juce::jlimit(0.0f, 1.0f,
                    getter ? getter(manual) : 1.0f) * segments);
                const float gap = 3.0f;
                const float width = juce::jmax(2.0f,
                    (getWidth() - gap * (segments + 1)) / segments);
                for (int i = 0; i < segments; ++i)
                {
                    g.setColour(i < lit ? juce::Colours::lime
                                        : juce::Colours::darkgreen.darker(0.7f));
                    g.fillRect(gap + i * (width + gap), 5.0f, width, getHeight() - 10.0f);
                }
            }
            void mouseDown(const juce::MouseEvent& event) override
            {
                if (event.mods.isRightButtonDown())
                {
                    if (learn) learn(manual);
                    return;
                }
                if (event.mods.isLeftButtonDown()) setFromMouse(event.position.x);
            }
            void mouseDrag(const juce::MouseEvent& event) override
            {
                if (event.mods.isLeftButtonDown()) setFromMouse(event.position.x);
            }
            void mouseWheelMove(const juce::MouseEvent&,
                const juce::MouseWheelDetails& wheel) override
            {
                const float current = getter ? getter(manual) : 1.0f;
                if (setter) setter(manual, juce::jlimit(0.0f, 1.0f,
                    current + wheel.deltaY * 0.12f));
                repaint();
            }
            void mouseDoubleClick(const juce::MouseEvent& event) override
            {
                if (event.mods.isLeftButtonDown() && setter) setter(manual, 1.0f);
                repaint();
            }
        private:
            void setFromMouse(float x)
            {
                if (setter) setter(manual, juce::jlimit(0.0f, 1.0f, x / juce::jmax(1.0f, (float)getWidth())));
                repaint();
            }
            int manual;
            std::function<float(int)> getter;
            std::function<void(int, float)> setter;
            std::function<void(int)> learn;
            std::function<bool(int)> learning;
        };

        SectionHeader(int index, std::function<void(int)> folder, std::function<void(int)> unloadFolder,
            std::function<void(int)> settings, std::function<void(int)> keyboardLearn,
            std::function<float(int)> swellGetter, std::function<void(int, float)> swellSetter,
            std::function<void(int)> swellLearn, std::function<bool(int)> swellLearning,
            std::function<std::vector<DivisionCouplerInfo>(int)> couplerGetter,
            std::function<void(const juce::String&, bool)> couplerSetter,
            std::function<void(const juce::String&)> midiLearn,
            std::function<bool(int)> tremulantGetter,
            std::function<void(int, bool)> tremulantSetter,
            std::function<double(int)> activityGetter)
            : manual(index), keyboardLearn(std::move(keyboardLearn)),
              couplerGetter(std::move(couplerGetter)), couplerSetter(std::move(couplerSetter)),
              midiLearn(std::move(midiLearn)), tremulantGetter(std::move(tremulantGetter)),
              tremulantSetter(std::move(tremulantSetter)), activityGetter(std::move(activityGetter)),
              swell(index, std::move(swellGetter), std::move(swellSetter),
                std::move(swellLearn), std::move(swellLearning))
        {
            static const juce::String names[8]{ "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
            title.setText(names[index], juce::dontSendNotification);
            title.setFont(juce::FontOptions(24.0f, juce::Font::bold));
            addAndMakeVisible(title);
            // "Load Stops" opens the Stop building table of the division, where the
            // folder trees are loaded and unloaded and the stops are arranged.
            juce::ignoreUnused(folder, unloadFolder);
            // The stops and folders of every division live in the Stop List (top bar).
            addChildComponent(load);
            setup.setButtonText("Settings");
            setup.onClick = [settings, index] { if (settings) settings(index); };
            addAndMakeVisible(setup);
            addAndMakeVisible(swell);
            tremulant.setButtonText("Tremulant");
            tremulant.getProperties().set("tremulantManual", index);
            tremulant.setColour(juce::TextButton::buttonOnColourId, onColour());
            // Switched on mouse-down (like the stops) so that one drag can sweep
            // across stops, couplers and tremulants; see SwipeSwitch.
            tremulant.addMouseListener(this, false);
            SwipeSwitch::add(&tremulant, [this](bool enabled) { applyTremulant(enabled); });
            addAndMakeVisible(tremulant);
            missing.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            missing.setColour(juce::TextButton::textColourOffId, juce::Colours::orange);
            missing.onClick = [this] { if (missingAction) missingAction(); else if (load.onClick) load.onClick(); };
            addChildComponent(missing);
            rebuildCouplers();
            startTimerHz(25);
        }
        ~SectionHeader() override
        {
            SwipeSwitch::remove(&tremulant);
            for (auto* button : couplerButtons) SwipeSwitch::remove(button);
        }
        void setStopsAction(std::function<void()> action) { load.onClick = std::move(action); }
        // A sample folder tree of this division is not on this computer: an
        // orange line under the header; a click opens the Stop List, where the
        // folder is loaded again (a tree of the same name takes the missing
        // one's place) or unloaded. The line goes when no tree is missing.
        void setMissingNotice(const juce::String& text, const juce::String& detail)
        {
            missing.setButtonText(text);
            missing.setTooltip(detail);
            missing.setVisible(text.isNotEmpty());
        }
        int getPreferredHeight(int width) const
        {
            return basePreferredHeight(width) + (missing.isVisible() ? 34 : 0);
        }
        int basePreferredHeight(int width) const
        {
            if (couplerButtons.isEmpty() && !couplerArrange) return 42;
            int x = 5, rows = 1;
            const int usable = juce::jmax(90, width - 10);
            for (auto* button : couplerButtons)
            {
                const int w = juce::jlimit(86, 150, 28 + button->getButtonText().length() * 8);
                if (x > 5 && x + w > usable) { ++rows; x = 5; }
                x += w + 5;
            }
            if (couplerArrange)   // the spare slot at the end
            {
                if (x > 5 && x + 100 > usable) ++rows;
            }
            return 40 + rows * 41;
        }
        void paint(juce::Graphics& g) override
        {
            const double until = activityGetter ? activityGetter(manual) : 0.0;
            const bool active = juce::Time::getMillisecondCounterHiRes() < until;
            const auto indicator = juce::Rectangle<float>(5.0f, 7.0f, 34.0f, 24.0f);
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(indicator, 2.0f);
            const float keyWidth = indicator.getWidth() / 3.0f;
            for (int key = 0; key < 3; ++key)
            {
                g.setColour(active ? juce::Colours::red : juce::Colours::white);
                g.fillRect(indicator.getX() + key * keyWidth + 0.7f,
                    indicator.getY() + 1.5f, keyWidth - 1.4f, indicator.getHeight() - 3.0f);
            }
            // Three white keys with the two intervening semitones.
            g.setColour(juce::Colours::black);
            for (const int gap : { 1, 2 })
                g.fillRect(indicator.getX() + gap * keyWidth - keyWidth * 0.23f,
                    indicator.getY() + 1.5f, keyWidth * 0.46f, indicator.getHeight() * 0.58f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(indicator, 2.0f, 1.0f);
        }
        void mouseDown(const juce::MouseEvent& event) override
        {
            if (event.mods.isPopupMenu()
                && juce::Rectangle<float>(5.0f, 7.0f, 34.0f, 24.0f).contains(event.position)
                && keyboardLearn)
                keyboardLearn(manual);
            else if (event.mods.isPopupMenu() && event.eventComponent == this && headerMenu)
                headerMenu(manual);   // the header's own background: the division's feature settings
            else if (event.mods.isPopupMenu() && midiLearn && event.eventComponent != this)
            {
                if (event.eventComponent == &tremulant)
                {
                    midiLearn("tremulant:" + juce::String(manual));
                    return;
                }
                const auto id = event.eventComponent->getProperties()
                    .getWithDefault("couplerId", {}).toString();
                if (id.isNotEmpty()) midiLearn("coupler:" + id);
            }
            else if (event.mods.isLeftButtonDown() && event.eventComponent == this && couplerArrange)
            {
                couplerArrange = false;   // click on the background ends arranging
                if (arrangeStateChanged) arrangeStateChanged(manual, false);
                resized();
                repaint();
            }
            else if (event.mods.isLeftButtonDown() && event.eventComponent != this)
            {
                if (event.eventComponent == &tremulant)
                {
                    SwipeSwitch::begin(!tremulant.getToggleState());
                    applyTremulant(SwipeSwitch::enable);
                }
                else if (auto* button = dynamic_cast<LeftOnlyTextButton*>(event.eventComponent))
                {
                    if (couplerButtons.contains(button))
                    {
                        pressScreenPosition = event.getScreenPosition();
                        if (couplerArrange)
                        {
                            draggedCoupler = button;
                            dragStartTopLeft = button->getPosition();
                            button->toFront(false);
                            return;
                        }
                        SwipeSwitch::begin(!button->getToggleState());
                        applyCoupler(button, SwipeSwitch::enable);
                        pressedCoupler = button;   // a 3 s hold starts arranging
                        pressedAtMs = juce::Time::getMillisecondCounterHiRes();
                    }
                }
            }
        }
        void mouseDrag(const juce::MouseEvent& event) override
        {
            if (draggedCoupler != nullptr)
            {
                draggedCoupler->setTopLeftPosition(dragStartTopLeft + (event.getScreenPosition() - pressScreenPosition));
                return;
            }
            if (pressedCoupler != nullptr && event.getScreenPosition().getDistanceFrom(pressScreenPosition) > 8)
                pressedCoupler = nullptr;   // moved: a swipe, not a long press
            if (event.mods.isLeftButtonDown()) SwipeSwitch::applyAt(event.getScreenPosition());
        }
        void mouseUp(const juce::MouseEvent& event) override
        {
            pressedCoupler = nullptr;
            if (draggedCoupler != nullptr)
            {
                auto* dragged = draggedCoupler;
                draggedCoupler = nullptr;
                dropCoupler(dragged, event.getScreenPosition());
                return;
            }
            SwipeSwitch::end();
        }
        void paintOverChildren(juce::Graphics& g) override
        {
            if (!couplerArrange) return;
            for (auto* button : couplerButtons)
                drawMarchingBorder(g, button->getBounds().toFloat().expanded(2.0f), dashPhase,
                    juce::Colours::white.withAlpha(0.85f));
            if (!spareSlot.isEmpty())   // the free place at the end
            {
                juce::Path outline;
                outline.addRoundedRectangle(spareSlot.toFloat().reduced(3.0f), 4.0f);
                juce::Path dashed;
                const float dashes[]{ 4.0f, 4.0f };
                juce::PathStrokeType(1.0f).createDashedStroke(dashed, outline, dashes, 2);
                g.setColour(juce::Colours::white.withAlpha(0.35f));
                g.fillPath(dashed);
            }
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced(5);
            auto top = r.removeFromTop(30);
            top.removeFromLeft(40);
            title.setBounds(top.removeFromLeft(50));
            // One line: keyboard indicator, division name, swell display, Tremulant,
            // Settings. A narrow column shrinks the three controls proportionally.
            const int natural = 104 + 94 + 80;
            const float scale = juce::jlimit(0.35f, 1.0f, (float)top.getWidth() / (float)natural);
            swell.setBounds(top.removeFromLeft(juce::roundToInt(104 * scale)).reduced(2, 0));
            tremulant.setBounds(top.removeFromLeft(juce::roundToInt(94 * scale)).reduced(2));
            setup.setBounds(top.removeFromLeft(juce::roundToInt(80 * scale)).reduced(2));
            int x = 5, y = 40;
            const int right = juce::jmax(90, getWidth() - 5);
            for (auto* button : couplerButtons)
            {
                const int w = juce::jlimit(86, 150, 28 + button->getButtonText().length() * 8);
                if (x > 5 && x + w > right) { x = 5; y += 41; }
                button->setBounds(x, y, w, 36);
                x += w + 5;
            }
            spareSlot = {};
            if (couplerArrange)
            {
                if (x > 5 && x + 100 > right) { x = 5; y += 41; }
                spareSlot = { x, y, 100, 36 };
            }
            if (missing.isVisible()) missing.setBounds(5, getHeight() - 34, juce::jmax(60, getWidth() - 10), 30);
        }
    private:
        void applyTremulant(bool enabled)
        {
            if (tremulant.getToggleState() == enabled) return;
            tremulant.setToggleState(enabled, juce::dontSendNotification);
            tremulant.setColour(juce::TextButton::buttonColourId,
                enabled ? onColour() : juce::Colours::darkgrey);
            if (tremulantSetter) tremulantSetter(manual, enabled);
        }
        void applyCoupler(LeftOnlyTextButton* button, bool enabled)
        {
            if (button == nullptr || button->getToggleState() == enabled) return;
            const auto id = button->getProperties().getWithDefault("couplerId", {}).toString();
            button->setToggleState(enabled, juce::dontSendNotification);
            button->setColour(juce::TextButton::buttonColourId,
                enabled ? onColour() : juce::Colours::darkgrey);
            if (couplerSetter && id.isNotEmpty()) couplerSetter(id, enabled);
        }
        void rebuildCouplers()
        {
            for (auto* button : couplerButtons) SwipeSwitch::remove(button);
            couplerButtons.clear();
            if (!couplerGetter) return;
            for (const auto& info : couplerGetter(manual))
            {
                auto* button = couplerButtons.add(new LeftOnlyTextButton(info.label));
                button->getProperties().set("couplerId", info.id);
                button->setToggleState(info.enabled, juce::dontSendNotification);
                button->setColour(juce::TextButton::buttonColourId,
                    info.enabled ? onColour() : juce::Colours::darkgrey);
                button->setColour(juce::TextButton::buttonOnColourId, onColour());
                // Switched on mouse-down through the swipe registry (see mouseDown).
                button->addMouseListener(this, false);
                SwipeSwitch::add(button, [this, button](bool enabled) { applyCoupler(button, enabled); });
                addAndMakeVisible(button);
            }
        }
        void timerCallback() override
        {
            if (pressedCoupler != nullptr && pressedAtMs > 0.0
                && juce::Time::getMillisecondCounterHiRes() - pressedAtMs >= 3000.0)
            {
                // Long press: the switch was not meant, take it back and arrange.
                auto* button = pressedCoupler;
                pressedCoupler = nullptr;
                applyCoupler(button, !button->getToggleState());
                SwipeSwitch::end();
                couplerArrange = true;
                if (arrangeStateChanged) arrangeStateChanged(manual, true);
                resized();   // makes room for the spare slot
            }
            if (couplerArrange)
            {
                dashPhase += 1.2f;
                if (dashPhase > 1000.0f) dashPhase -= 990.0f;
                repaint();
            }
            repaint(0, 5, 40, 28);
            swell.repaint();
            const bool tremEnabled = tremulantGetter && tremulantGetter(manual);
            tremulant.setToggleState(tremEnabled, juce::dontSendNotification);
            tremulant.setColour(juce::TextButton::buttonColourId,
                tremEnabled ? onColour() : juce::Colours::darkgrey);
            if (couplerGetter)
                for (const auto& info : couplerGetter(manual))
                    for (auto* button : couplerButtons)
                        if (button->getProperties().getWithDefault("couplerId", {}).toString() == info.id)
                        {
                            button->setToggleState(info.enabled, juce::dontSendNotification);
                            button->setColour(juce::TextButton::buttonColourId,
                                info.enabled ? onColour() : juce::Colours::darkgrey);
                        }
        }
        int manual;
        std::function<void(int)> keyboardLearn;
        std::function<std::vector<DivisionCouplerInfo>(int)> couplerGetter;
        std::function<void(const juce::String&, bool)> couplerSetter;
        std::function<void(const juce::String&)> midiLearn;
        std::function<bool(int)> tremulantGetter;
        std::function<void(int, bool)> tremulantSetter;
        std::function<double(int)> activityGetter;
        juce::Label title;
        juce::TextButton load, setup, missing;
        LeftOnlyTextButton tremulant;
        juce::OwnedArray<LeftOnlyTextButton> couplerButtons;
        SwellBar swell;
        // Coupler arrangement: a 3 s press on a coupler starts it, couplers are
        // then dragged to new places; a click on the header background ends it.
        bool couplerArrange = false;
        LeftOnlyTextButton* pressedCoupler = nullptr;
        double pressedAtMs = -1.0;
        LeftOnlyTextButton* draggedCoupler = nullptr;
        juce::Point<int> pressScreenPosition, dragStartTopLeft;
        float dashPhase = 0.0f;
        juce::Rectangle<int> spareSlot;   // free place after the last coupler while arranging

        void dropCoupler(LeftOnlyTextButton* dragged, juce::Point<int> screenPosition)
        {
            const auto centre = getLocalPoint(nullptr, screenPosition)
                + (dragged->getLocalBounds().getCentre() - dragged->getLocalPoint(nullptr, screenPosition));
            std::vector<LeftOnlyTextButton*> order;
            for (auto* button : couplerButtons) order.push_back(button);
            LeftOnlyTextButton* target = nullptr;
            for (auto* button : order)
                if (button != dragged && button->getBounds().contains(centre)) { target = button; break; }
            if (target == nullptr && !spareSlot.contains(centre)) { resized(); return; }   // dropped outside: snap back
            order.erase(std::remove(order.begin(), order.end(), dragged), order.end());
            if (target == nullptr)
                order.push_back(dragged);   // the spare slot: move to the end
            else
            {
                std::vector<LeftOnlyTextButton*> original;
                for (auto* button : couplerButtons) original.push_back(button);
                const bool forward = std::find(original.begin(), original.end(), target)
                    > std::find(original.begin(), original.end(), dragged);
                auto at = std::find(order.begin(), order.end(), target);
                order.insert(forward ? at + 1 : at, dragged);
            }
            std::vector<juce::String> ids;
            for (auto* button : order) ids.push_back(button->getProperties().getWithDefault("couplerId", {}).toString());
            if (!couplerReorder) { resized(); return; }
            // The owner rebuilds the headers; do it after this mouse event.
            auto callback = couplerReorder;
            const int m = manual;
            juce::MessageManager::callAsync([callback, m, ids] { callback(m, ids); });
        }
    public:
        std::function<void(int, const std::vector<juce::String>&)> couplerReorder;
        std::function<void(int, bool)> arrangeStateChanged;
        std::function<void()> missingAction;   // the notice under the header was clicked   // (manual, arranging) - remembered by the owner
        std::function<void(int)> headerMenu;   // right-click on the header's background
        void setCouplerArranging(bool on) { couplerArrange = on; }
    };

    // The lists are cleared by rebuildUI (both screens share them); this screen's
    // container starts empty.
    container.removeAllChildren();
    screenContainers[(size_t)screen] = &container;
    screenViewports[(size_t)screen] = &viewport;
    if (arrangeExitListener == nullptr)
    {
        // A click on empty workspace ends arrange mode.
        struct ExitListener final : public juce::MouseListener
        {
            explicit ExitListener(Voicer& v) : voicer(v) {}
            void mouseDown(const juce::MouseEvent&) override { if (voicer.isArrangeModeActive()) voicer.exitArrangeMode(); }
            Voicer& voicer;
        };
        arrangeExitListener = std::make_unique<ExitListener>(*this);
    }
    container.addMouseListener(arrangeExitListener.get(), false);   // ignored when already added
    // The divisions shown on this screen, in console order.
    std::vector<int> manuals;
    for (int manual = 0; manual < activeManualCount; ++manual)
        if (screenOf(manual) == screen) manuals.push_back(manual);
    const int columnsHere = (int)manuals.size();
    // Arrange hooks for one grid item (a stop or a transmission) of a division.
    auto hooksFor = [this](int manual, juce::String key)
    {
        StopRow::ArrangeHooks hooks;
        hooks.active = [this](const juce::Component*) { return arrangeMode; };   // every division arranges at once
        juce::ignoreUnused(manual);
        hooks.phase = [this] { return arrangeDashPhase; };
        hooks.begin = [this, manual](juce::Component*) { enterArrangeMode(manual); };
        hooks.drop = [this, manual, key](juce::Component* row, juce::Point<int> screenPosition, bool duplicate)
        {
            dropItem(manual, key, row, screenPosition, duplicate);
        };
        hooks.remove = [this, manual, key](juce::Component*) { removeItem(manual, key); };
        hooks.hover = [this, manual, key](juce::Component*, juce::Point<int> screenPosition, bool duplicate)
        { hoverCopy = duplicate; updateIndentIndicator(manual, key, screenPosition); };
        return hooks;
    };

    computeNoteRange();

    // Stop folders: a child's voicing opens and closes on its own, but only
    // while its master's is open (a folded master shows no children).
    for (auto* stop : engine.getStops())
        if (stop->groupMaster.isNotEmpty())
            for (auto* master : engine.getStops())
                if (master->primaryManual == stop->primaryManual && master->name == stop->groupMaster && !master->voicingExpanded)
                    stop->voicingExpanded = false;

    std::array<bool, SamplerEngine::maxManuals> manualHasExpanded{};
    bool anyExpanded = false;
    for (auto* stop : engine.getStops())
    {
        if (stop->voicingExpanded && stop->primaryManual >= 0 && stop->primaryManual < SamplerEngine::maxManuals)
        {
            manualHasExpanded[(size_t)stop->primaryManual] = true;
            anyExpanded = true;
        }
        for (const auto& t : stop->transmissions)   // an open transmission voicing widens its receiving division
            if (t.voicingExpanded && t.manual >= 0 && t.manual < SamplerEngine::maxManuals)
            {
                manualHasExpanded[(size_t)t.manual] = true;
                anyExpanded = true;
            }
    }
    const int narrowWidth = 335;
    const int availableExpanded = viewport.getWidth() - margin * 2
        - narrowWidth * juce::jmax(0, columnsHere - 1);
    int widestRankCount = 1;
    for (auto* stop : engine.getStops())
        if (stop->voicingExpanded)
            widestRankCount = juce::jmax(widestRankCount, 1 + (int)stop->ranks.size());
    // Two lines of controls: the wider one decides the expanded width.
    const int line1Width = margin + 200 + 8 + 55 + widestRankCount * 82 + 8 + 200 + 320 + 8 + 240 + margin;   // ... + alias
    const int line2Width = margin + 150 + 8 + 210 + 8 + 210 + 8 + 210 + 8 + 230 + 8 + 250 + margin;
    const int topControlsWidth = juce::jmax(line1Width, line2Width);
    int widestCompass = noteMax - noteMin + 1;
    for (const int manual : manuals)
    {
        const auto compass = compassOf(manual);
        widestCompass = juce::jmax(widestCompass, compass.second - compass.first + 1);
        // ... and the keys beyond it that a stop's folder holds (its rows show them).
        for (auto* stop : engine.getStops())
            if (stop != nullptr && stop->primaryManual == manual)
            {
                int first = compass.first, last = compass.second;
                for (int note = 0; note < 128; ++note)
                    if (stop->hasNoteInAnyRank(note)) { first = juce::jmin(first, note); last = juce::jmax(last, note); }
                widestCompass = juce::jmax(widestCompass, last - first + 1);
            }
    }
    const int voicingWidth = juce::jmax(topControlsWidth, margin * 2 + widestCompass * sliderWidth);
    const int wideWidth = juce::jmax(availableExpanded, voicingWidth);
    std::array<int, SamplerEngine::maxManuals> configuredWidths{};
    float totalColumnWeight = 0.0f;
    for (const int manual : manuals)
        totalColumnWeight += columnWeightGetter
            ? juce::jmax(0.15f, columnWeightGetter(manual)) : 1.0f;
    const int availableColumnWidth = juce::jmax(220 * columnsHere,
        viewport.getWidth() - margin * (columnsHere + 1));
    for (const int manual : manuals)
    {
        const float weight = columnWeightGetter
            ? juce::jmax(0.15f, columnWeightGetter(manual)) : 1.0f;
        configuredWidths[(size_t)manual] = juce::jmax(220,
            juce::roundToInt((float)availableColumnWidth * weight
                / juce::jmax(0.15f, totalColumnWeight)));
    }
    // A voicing open on the other screen does not narrow the columns here.
    bool anyExpandedHere = false;
    for (const int manual : manuals) anyExpandedHere = anyExpandedHere || manualHasExpanded[(size_t)manual];
    anyExpanded = anyExpandedHere;
    int x = margin;
    int totalHeight = 0;
    std::array<int, SamplerEngine::maxManuals> usedWidths{};   // the widths actually laid out (dividers sit between them)
    for (const int manual : manuals)
    {
        const int columnWidth = !anyExpanded ? configuredWidths[(size_t)manual]
            : (manualHasExpanded[(size_t)manual] ? wideWidth : narrowWidth);
        usedWidths[(size_t)manual] = columnWidth;
        columnBounds[(size_t)manual] = juce::Rectangle<int>(x, 0, columnWidth, 1 << 20);   // the wheel test only needs the x range
        columnHasVoicing[(size_t)manual] = manualHasExpanded[(size_t)manual];
        columnScreen[(size_t)manual] = screen;
        int y = margin;
        auto header = std::make_unique<SectionHeader>(manual, folderCallback, unloadCallback,
            settingsCallback, keyboardLearn,
            swellGetter, swellSetter, swellLearn, swellLearning,
            couplerGetter, couplerSetter, midiLearnCallback,
            tremulantGetter, tremulantSetter,
            [this](int m) { return getManualActivityUntil(m); });
        header->setStopsAction([this, manual] { if (stopBuildCallback) stopBuildCallback(manual); });
        header->couplerReorder = couplerOrderSetter;
        // Arranging couplers survives the rebuild that every reorder triggers.
        header->arrangeStateChanged = [this](int m, bool on) { couplerArrangeManual = on ? m : -1; };
        header->setCouplerArranging(couplerArrangeManual == manual);
        if (missingTreesGetter)
            header->setMissingNotice(missingTreesGetter(manual), missingTreesDetail ? missingTreesDetail(manual) : juce::String());
        header->missingAction = [this, manual] { if (missingTreesAction) missingTreesAction(manual); };
        header->headerMenu = divisionMenu;
        const int headerHeight = header->getPreferredHeight(columnWidth);
        header->setBounds(x, y, columnWidth, headerHeight);
        container.addAndMakeVisible(header.get());
        sectionHeaders.add(std::move(header));
        y += headerHeight + 4;
        const std::vector<Stop*> divisionStops = orderedDivisionStops(manual);
        const int drawWidth = 215;
        const int gridColumns = juce::jmax(1, columnWidth / drawWidth);
        // Grid items of this division: its own compact stops and the transmissions
        // it receives. Both are placed, dragged and pooled the same way.
        struct Item { Stop* stop = nullptr; bool transmission = false; juce::String key; int index = -1; };
        const auto compass = compassOf(manual);
        auto makeStopRow = [&](Stop* stop, const juce::String& key, int transmissionIndex)
        {
            // The per-key rows of a stop cover the division's compass AND every key
            // the folder holds a sample for (0.8.156: a six-octave folder on a
            // five-octave keyboard - the couplers reach those keys, so they are voiced).
            auto range = compass;
            if (transmissionIndex < 0 && stop != nullptr)
                for (int note = 0; note < 128; ++note)
                    if (stop->hasNoteInAnyRank(note)) { range.first = juce::jmin(range.first, note); range.second = juce::jmax(range.second, note); }
            auto row = std::make_unique<StopRow>(stop, engine, range.first, range.second, sliderWidth,
                midiLearnCallback, [this](Stop* target)
                { scrollTarget = target; if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport); },
                activeManualCount, stateChanged, transmissionIndex);
            row->setArrangeHooks(hooksFor(manual, key));
            row->transpositionRequest = [this](Stop* target, int semitones) { if (transpositionRequest) transpositionRequest(target, semitones); };
            row->oneShotRequest = [this](Stop* target, bool on) { if (oneShotRequest) oneShotRequest(target, on); };
            row->setTransmissionRemover([this](Stop* target, int index) { removeTransmission(target, index); });
            row->setChildrenGetter([this](const Stop* master) { return groupChildren(master); },
                [this](const Stop* master) { return groupChildTransmissions(master); });
            // Folding of the per-key rows: each stop (and transmission) remembers its own.
            row->setVoicingFolds([this, stop, transmissionIndex](int section)
                {
                    bool* flag = section == 0 ? &stop->volumeKeysExpanded : &stop->toneKeysExpanded;
                    if (transmissionIndex >= 0 && transmissionIndex < (int)stop->transmissions.size())
                    {
                        auto& t = stop->transmissions[(size_t)transmissionIndex];
                        flag = section == 0 ? &t.volumeKeysExpanded : &t.toneKeysExpanded;
                    }
                    *flag = !*flag;
                    if (stateChanged) stateChanged();   // the folds are part of the saved voicing state
                    juce::MessageManager::callAsync([this]
                    {
                        if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
                    });
                });
            if (temperamentDescriber && transmissionIndex < 0) row->setTemperamentText(temperamentDescriber(stop));
            return row;
        };
        auto itemExpanded = [](const Item* item)
        {
            if (item->transmission)
                return item->index >= 0 && item->index < (int)item->stop->transmissions.size()
                    && item->stop->transmissions[(size_t)item->index].voicingExpanded;
            return item->stop->voicingExpanded;
        };
        // Every stop keeps its grid cell; an expanded (voicing) stop opens as a
        // full-width band at its own row, parting the rows below.
        std::vector<Item> items;
        for (auto* stop : divisionStops)
        {
            if (isStopHidden(manual, stop->name)) continue;   // in the stop pool, not on the grid
            if (stop->groupMaster.isNotEmpty()) continue;      // a stop folder's child: under its master's voicing
            // A stop folder with nothing under it because its children were left out
            // of a load stopped for memory (0.8.144): not on the grid - an empty button
            // is a stop that seems loaded. Its row stays in the Stop List. Only after
            // such a load (0.8.153): the rule hid every NEW stop folder created on the
            // grid - empty until the user puts something into it (#77).
            if (stop->container && groupChildren(stop).empty() && groupChildTransmissions(stop).empty()
                && engine.lastLoadWasMemoryLimited()) continue;
            items.push_back({ stop, false, stop->name });
        }
        for (auto* stop : engine.getStops())
            for (int index = 0; index < (int)stop->transmissions.size(); ++index)
            {
                const auto& t = stop->transmissions[(size_t)index];
                if (t.manual != manual || isStopHidden(manual, transmissionKey(stop, index))) continue;
                // A transmission indented under a stop folder of this division sits
                // under the master's voicing, not on the grid.
                bool underMaster = false;
                if (t.groupMaster.isNotEmpty())
                    for (auto* master : divisionStops)
                        if (master->name == t.groupMaster && master->groupMaster.isEmpty()) underMaster = true;
                if (underMaster) continue;
                items.push_back({ stop, true, transmissionKey(stop, index), index });
            }
        // The Stop List applied a new order: every item is laid out afresh in it
        // (the cells were cleared with the order; unknown keys follow at the end).
        if (const auto order = arrangeOrders.find(manual); order != arrangeOrders.end())
        {
            const auto& keys = order->second;
            auto rank = [&keys](const Item& item)
            {
                const auto found = std::find(keys.begin(), keys.end(), item.key);
                return found == keys.end() ? (int)keys.size() : (int)(found - keys.begin());
            };
            std::stable_sort(items.begin(), items.end(), [&rank](const Item& a, const Item& b) { return rank(a) < rank(b); });
            arrangeOrders.erase(order);
        }
        // A real grid: items the user has placed keep their cell (column, row),
        // empty cells are allowed.
        const int cellHeight = stopRowTopHeight + 4;
        auto& cellsOfManual = stopCells[manual];
        // Items are placed over the division's natural width (its configured
        // column, not the width narrowed while a voicing is open elsewhere), so a
        // placement made beside an open voicing never ends up in a single column.
        const int placementColumns = juce::jmax(1, configuredWidths[(size_t)manual] / drawWidth);
        // A division whose every item sits in the first column while two or more
        // fit (placed once beside an open voicing, or in a narrow window) is laid
        // out afresh across the columns; a layout the user made keeps other columns.
        if (placementColumns >= 2 && items.size() >= 2)
        {
            bool allFirstColumn = true;
            size_t stored = 0;
            for (const auto& item : items)
                if (auto it = cellsOfManual.find(item.key); it != cellsOfManual.end())
                {
                    ++stored;
                    if (it->second.first != 0) allFirstColumn = false;
                }
            if (stored == items.size() && allFirstColumn)
                for (const auto& item : items) cellsOfManual.erase(item.key);
        }
        // 1. Stored cells go into a virtual grid that may be wider than the window.
        int storedColumns = juce::jmax(gridColumns, placementColumns);
        for (const auto& item : items)
            if (auto it = cellsOfManual.find(item.key); it != cellsOfManual.end())
                storedColumns = juce::jmax(storedColumns, it->second.first + 1);
        std::vector<std::vector<const Item*>> columns((size_t)storedColumns);   // [column][row], nullptr = empty
        auto put = [&](int column, int row, const Item* item)
        {
            auto& col = columns[(size_t)column];
            while (row < (int)col.size() && col[(size_t)row] != nullptr) ++row;   // occupied: below it
            if (row >= (int)col.size()) col.resize((size_t)row + 1, nullptr);
            col[(size_t)row] = item;
            return row;
        };
        std::vector<const Item*> unplaced;
        for (const auto& item : items)
        {
            const auto it = cellsOfManual.find(item.key);
            if (it == cellsOfManual.end()) unplaced.push_back(&item);
            else put(juce::jmax(0, it->second.first), juce::jmax(0, it->second.second), &item);
        }
        // 2. Items never placed by the user take the first free cells, row by row
        //    (the first row across all columns, then the second, in load order,
        //    which is the Stop building table's order), and remember them: a cell
        //    freed later stays empty.
        {
            int rowsNeeded = 0;
            for (const auto& col : columns) rowsNeeded = juce::jmax(rowsNeeded, (int)col.size());
            rowsNeeded = juce::jmax(rowsNeeded, ((int)items.size() + placementColumns - 1) / placementColumns);
            size_t next = 0;
            for (int row = 0; row < rowsNeeded && next < unplaced.size(); ++row)
                for (int column = 0; column < placementColumns && next < unplaced.size(); ++column)
                {
                    const auto& col = columns[(size_t)column];
                    if (row < (int)col.size() && col[(size_t)row] != nullptr) continue;
                    put(column, row, unplaced[next]);
                    cellsOfManual[unplaced[next]->key] = { column, row };
                    ++next;
                }
            while (next < unplaced.size())   // grid full: new rows at the bottom
            {
                for (int column = 0; column < placementColumns && next < unplaced.size(); ++column)
                {
                    const int row = put(column, rowsNeeded, unplaced[next]);
                    cellsOfManual[unplaced[next]->key] = { column, row };
                    ++next;
                }
                ++rowsNeeded;
            }
        }
        // 3. A narrower window: the rightmost column merges into its neighbour like
        //    a zipper (left 1, right 1, left 2, right 2, ...). The stored cells stay
        //    untouched, so a wider window restores the layout.
        while ((int)columns.size() > gridColumns && columns.size() > 1)
        {
            // Empty cells take part in the zip like any other entry (left 1, right 1,
            // left 2, right 2, ...) and are only dropped from the merged column.
            const auto& left = columns[columns.size() - 2];
            const auto& right = columns.back();
            std::vector<const Item*> merged;
            for (size_t i = 0; i < juce::jmax(left.size(), right.size()); ++i)
            {
                if (i < left.size() && left[i] != nullptr) merged.push_back(left[i]);
                if (i < right.size() && right[i] != nullptr) merged.push_back(right[i]);
            }
            columns.pop_back();
            columns.back() = merged;
        }
        columns.resize((size_t)gridColumns);
        int gridRows = 0;
        for (const auto& col : columns)
            for (int row = 0; row < (int)col.size(); ++row)
                if (col[(size_t)row] != nullptr) gridRows = juce::jmax(gridRows, row + 1);
        // One spare row in every division while arranging: items can be dropped
        // (moved or copied) into another division's grid too.
        const int visibleRows = gridRows + (arrangeMode ? 1 : 0);
        // Row positions: an expanded stop opens as a full-width voicing band right
        // above its own row (the rows below are parted vertically), its compact
        // cell stays reserved in the grid.
        // Rows are laid out in zipper order (left to right inside a row). An
        // expanded stop opens as a full-width voicing band exactly at its place
        // in that order: the cells left of it stay above the band, the cells
        // right of it continue below it. Its own compact cell stays reserved.
        std::vector<juce::Rectangle<int>> cellRects;   // for the arrange overlay
        int cursor = y;
        for (int row = 0; row < visibleRows; ++row)
        {
            bool lineHasCells = false;   // cells already placed on the current line
            for (int column = 0; column < gridColumns; ++column)
            {
                const auto& col = columns[(size_t)column];
                const Item* item = row < (int)col.size() ? col[(size_t)row] : nullptr;
                if (item != nullptr && itemExpanded(item))
                {
                    // Stops and transmissions alike open as a full-width voicing band.
                    if (lineHasCells) { cursor += cellHeight; lineHasCells = false; }
                    // The band's height follows this row's own folds (a stop folder
                    // without pipes is one line).
                    const int expandedHeight = StopRow::expandedHeightFor(item->stop, item->transmission ? item->index : -1, columnWidth);
                    const int masterTop = cursor;   // the band's top: the tree lines of a stop folder start under its button
                    auto stopRow = makeStopRow(item->stop, item->key, item->transmission ? item->index : -1);
                    stopRow->setBounds(x, cursor, columnWidth, expandedHeight);
                    container.addAndMakeVisible(stopRow.get());
                    stopRows.add(std::move(stopRow));
                    cursor += expandedHeight + margin;
                    // A stop folder: its children stay stacked under it in this column
                    // while it is open - each as a compact row (button only) or, when
                    // its own voicing is open, as a full band. They return to the grid
                    // only when the master folds up (they have no cell of their own).
                    if (!item->transmission)
                    {
                        // Indented, joined to the master by tree lines (a trunk down the
                        // left, a branch to every child), like a folder tree.
                        const auto children = groupChildren(item->stop);
                        const auto childTransmissions = groupChildTransmissions(item->stop);
                        // The trunk starts right under the master's button and runs to the last branch.
                        const int indent = 28, branchTop = masterTop + stopRowTopHeight;
                        std::vector<int> branchTicks;
                        for (auto* child : children)
                        {
                            const int childHeight = child->voicingExpanded ? StopRow::expandedHeightFor(child, -1, juce::jmax(220, columnWidth - indent)) : stopRowTopHeight;
                            auto childRow = makeStopRow(child, child->name, -1);
                            childRow->setBounds(x + indent, cursor, juce::jmax(220, columnWidth - indent), childHeight);
                            container.addAndMakeVisible(childRow.get());
                            stopRows.add(std::move(childRow));
                            branchTicks.push_back(cursor - branchTop + 25);   // the centre of the child's button
                            cursor += childHeight + margin;
                        }
                        // Transmissions indented under the master: the same, with their own voicing rows.
                        for (const auto& [source, index] : childTransmissions)
                        {
                            const bool open = source->transmissions[(size_t)index].voicingExpanded;
                            const int childHeight = open ? StopRow::expandedHeightFor(source, index) : stopRowTopHeight;
                            auto childRow = makeStopRow(source, transmissionKey(source, index), index);
                            childRow->setBounds(x + indent, cursor, juce::jmax(220, columnWidth - indent), childHeight);
                            container.addAndMakeVisible(childRow.get());
                            stopRows.add(std::move(childRow));
                            branchTicks.push_back(cursor - branchTop + 25);
                            cursor += childHeight + margin;
                        }
                        if (!children.empty() || !childTransmissions.empty())
                        {
                            struct BranchLines final : public juce::Component
                            {
                                std::vector<int> ticks;
                                int trunkX = 12, indent = 28;
                                void paint(juce::Graphics& g) override
                                {
                                    g.setColour(juce::Colours::white.withAlpha(0.5f));
                                    const int trunkEnd = ticks.empty() ? 0 : ticks.back();
                                    g.fillRect(trunkX, 0, 2, trunkEnd + 1);
                                    // The branch runs up to the button's edge (the row starts at 'indent', its button 5 px in).
                                    for (const int y : ticks) g.fillRect(trunkX, y, indent + 4 - trunkX, 2);
                                }
                            };
                            auto lines = std::make_unique<BranchLines>();
                            lines->ticks = branchTicks;
                            lines->indent = indent;
                            lines->setInterceptsMouseClicks(false, false);
                            lines->setBounds(x, branchTop, indent + 6, juce::jmax(1, branchTicks.back() + 2));
                            container.addAndMakeVisible(lines.get());
                            lines->toBack();   // under the master's band, which is transparent there anyway
                            sectionDividers.add(std::move(lines));
                        }
                    }
                }
                const juce::Rectangle<int> cell(x + column * drawWidth, cursor, drawWidth, stopRowTopHeight);
                cellRects.push_back(cell);
                compactCells.push_back({ item != nullptr ? item->key : juce::String(), manual, column, row, cell });
                lineHasCells = true;
                if (item == nullptr) continue;
                if (!itemExpanded(item))   // expanded ones sit in their band
                {
                    auto stopRow = makeStopRow(item->stop, item->key, item->transmission ? item->index : -1);
                    stopRow->setBounds(cell);
                    container.addAndMakeVisible(stopRow.get());
                    stopRows.add(std::move(stopRow));
                }
            }
            if (lineHasCells) cursor += cellHeight;
        }
        // The grid's geometry: a drop below the last row (or on any free spot of
        // the column) computes its cell from it, so a stop can be put anywhere.
        gridGeometry[(size_t)manual] = { x, drawWidth, cellHeight, gridColumns, y, cursor, visibleRows, true };
        if (arrangeMode)
        {
            // Faint outlines of every cell (behind the rows) show where stops can go - in every division.
            // A click on a free cell offers a new stop folder there (and the division's pooled items).
            struct GridOverlay final : public juce::Component
            {
                std::vector<juce::Rectangle<int>> cells;
                struct FreeCell { juce::Rectangle<int> bounds; int column, row; };
                std::vector<FreeCell> freeCells;
                std::function<void(int column, int row, juce::Point<int> screenPosition)> onFreeCellClick;
                juce::Point<int> pressedAt;
                void paint(juce::Graphics& g) override
                {
                    g.setColour(juce::Colours::white.withAlpha(0.18f));
                    for (const auto& cell : cells)
                    {
                        juce::Path outline;
                        outline.addRoundedRectangle(cell.toFloat().reduced(6.0f), 4.0f);
                        juce::Path dashed;
                        const float dashes[]{ 4.0f, 4.0f };
                        juce::PathStrokeType(1.0f).createDashedStroke(dashed, outline, dashes, 2);
                        g.fillPath(dashed);
                    }
                }
                bool hitTest(int px, int py) override
                {
                    for (const auto& cell : freeCells) if (cell.bounds.contains(px, py)) return true;
                    return false;   // occupied cells and the space between belong to the rows / the workspace
                }
                void mouseDown(const juce::MouseEvent& event) override { pressedAt = event.getPosition(); }
                void mouseUp(const juce::MouseEvent& event) override
                {
                    // Left or right button alike: a click (not a drag) on a free cell opens the menu.
                    if (event.getPosition().getDistanceFrom(pressedAt) > 6) return;
                    if (!event.mods.isLeftButtonDown() && !event.mods.isRightButtonDown()) return;
                    for (const auto& cell : freeCells)
                        if (cell.bounds.contains(event.getPosition()) && onFreeCellClick)
                        {
                            onFreeCellClick(cell.column, cell.row, event.getScreenPosition());
                            return;
                        }
                }
            };
            auto overlay = std::make_unique<GridOverlay>();
            overlay->setBounds(x, y, gridColumns * drawWidth, juce::jmax(1, cursor - y));
            for (const auto& cell : cellRects) overlay->cells.push_back(cell.translated(-x, -y));
            for (const auto& cell : compactCells)
                if (cell.manual == manual && cell.key.isEmpty())
                    overlay->freeCells.push_back({ cell.bounds.translated(-x, -y).reduced(6), cell.column, cell.row });
            overlay->onFreeCellClick = [this, manual](int column, int row, juce::Point<int> screenPosition) { freeCellClicked(manual, column, row, screenPosition); };
            container.addAndMakeVisible(overlay.get());
            overlay->toBack();
            arrangeOverlays.add(std::move(overlay));
        }
        y = cursor;
        totalHeight = juce::jmax(totalHeight, y);
        x += columnWidth + margin;
    }

    const int displayHeight = juce::jmax(totalHeight, viewport.getHeight());
    // The dividers between the division columns stay visible while a voicing is
    // expanded too; a drag then adjusts the weights the compact layout uses.
    if (columnWeightSetter)
    {
        int dividerX = margin;
        for (size_t i = 0; i + 1 < manuals.size(); ++i)
        {
            const int manual = manuals[i], right = manuals[i + 1];
            dividerX += usedWidths[(size_t)manual];
            const float leftWeight = columnWeightGetter
                ? columnWeightGetter(manual) : 1.0f;
            const float rightWeight = columnWeightGetter
                ? columnWeightGetter(right) : 1.0f;
            auto divider = std::make_unique<DivisionDivider>(manual, right,
                configuredWidths[(size_t)manual], configuredWidths[(size_t)right],
                leftWeight, rightWeight, columnWeightSetter);
            divider->setBounds(dividerX, 0, margin, juce::jmax(1, displayHeight - 5));
            container.addAndMakeVisible(divider.get());
            divider->toFront(false);
            sectionDividers.add(std::move(divider));
            dividerX += margin;
        }
    }

    const int totalWidth = x;
    container.setSize(totalWidth, displayHeight);
    viewport.setViewedComponent(&container, false);
    viewport.setScrollBarsShown(true, true);
    viewport.setViewPosition(oldViewPosition);
    if (scrollTarget != nullptr && scrollTarget->voicingExpanded)
    {
        for (auto* row : stopRows)
            if (row->getStop() == scrollTarget)
            {
                const auto rowBounds = row->getBounds();
                const auto visible = viewport.getViewArea();
                int wantedY = visible.getY();
                if (rowBounds.getY() < visible.getY() + 8)
                    wantedY = juce::jmax(0, rowBounds.getY() - 8);
                else if (rowBounds.getBottom() > visible.getBottom() - 8)
                    wantedY = juce::jmax(0, rowBounds.getBottom() - visible.getHeight() + 8);
                viewport.setViewPosition(viewport.getViewPositionX(), wantedY);
                break;
            }
    }
    scrollTarget = nullptr;

    container.repaint();
}

std::vector<Stop*> Voicer::orderedDivisionStops(int manual) const
{
    // Load order; the grid cells decide the display positions.
    std::vector<Stop*> result;
    for (auto* stop : engine.getStops())
        if (stop->primaryManual == manual) result.push_back(stop);
    return result;
}

void Voicer::enterArrangeMode(int manual)
{
    arrangeMode = true;
    arrangeManual = manual;
    if (arrangeAnimator == nullptr)
    {
        struct Animator final : public juce::Timer
        {
            explicit Animator(Voicer& v) : voicer(v) {}
            void timerCallback() override
            {
                voicer.arrangeDashPhase += 1.5f;
                if (voicer.arrangeDashPhase > 1000.0f) voicer.arrangeDashPhase -= 990.0f;
                for (auto* row : voicer.stopRows) row->repaint();
                // A dragged item resting still sends no mouse events: the rest is timed here.
                if (voicer.lastHover.active)
                    voicer.updateIndentIndicator(voicer.lastHover.manual, voicer.lastHover.key, voicer.lastHover.position);
            }
            Voicer& voicer;
        };
        arrangeAnimator = std::make_unique<Animator>(*this);
    }
    arrangeAnimator->startTimerHz(30);
    // The free cells (and the spare row) appear at once, not only after the
    // first drop: relayout after the long-press timer has finished.
    juce::MessageManager::callAsync([this]
    {
        if (arrangeMode && uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
    });
}

void Voicer::exitArrangeMode()
{
    if (!arrangeMode) return;
    arrangeMode = false;
    arrangeManual = -1;
    resetDwell();
    lastHover.active = false;
    if (indentMarker != nullptr) indentMarker->setVisible(false);
    if (arrangeAnimator != nullptr) arrangeAnimator->stopTimer();
    if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);   // rows snap to the grid
    if (stateChanged) stateChanged();   // the order is part of the saved state
}

bool Voicer::resolveItem(int manual, const juce::String& key, Stop*& stop, int& transmissionIndex) const
{
    // Keys: the stop's name, or "tr:<home division>:<name>[#ordinal]" for a transmission.
    stop = nullptr;
    transmissionIndex = -1;
    if (key.startsWith("tr:"))
    {
        const auto rest = key.fromFirstOccurrenceOf("tr:", false, false);
        const int home = rest.upToFirstOccurrenceOf(":", false, false).getIntValue();
        auto nameAndOrdinal = rest.fromFirstOccurrenceOf(":", false, false);
        const int ordinal = nameAndOrdinal.contains("#") ? nameAndOrdinal.fromLastOccurrenceOf("#", false, false).getIntValue() : 0;
        const auto name = nameAndOrdinal.contains("#") ? nameAndOrdinal.upToLastOccurrenceOf("#", false, false) : nameAndOrdinal;
        for (auto* candidate : engine.getStops())
            if (candidate->primaryManual == home && candidate->name == name) { stop = candidate; break; }
        if (stop != nullptr) transmissionIndex = stop->transmissionIndex(manual, ordinal);
        if (transmissionIndex < 0) stop = nullptr;
        return stop != nullptr;
    }
    for (auto* candidate : engine.getStops())
        if (candidate->primaryManual == manual && candidate->name == key) { stop = candidate; break; }
    return stop != nullptr;
}

StopRow* Voicer::indentTargetAt(int manual, const juce::String& key, juce::Point<int> screenPosition) const
{
    Stop* dragged = nullptr;
    int draggedIndex = -1;
    if (!resolveItem(manual, key, dragged, draggedIndex)) return nullptr;
    // (A stop folder can be dragged too: it dissolves into the target after a question, see dropItem.)
    for (auto* row : stopRows)
    {
        if (row == nullptr || row->isTransmission() || !row->isExpanded()) continue;
        auto* master = row->getStop();
        if (master == nullptr || master == dragged || master->groupMaster.isNotEmpty()) continue;
        // An open master: it has children already (an existing stop folder).
        if (groupChildren(master).empty() && groupChildTransmissions(master).empty() && !master->container) continue;
        if (!row->getScreenBounds().contains(screenPosition)) continue;
        // Already its child: nothing to do (a copy of a child may still go into the folder).
        if (!hoverCopy)
        {
            if (draggedIndex < 0 && dragged->groupMaster == master->name && dragged->primaryManual == master->primaryManual) return nullptr;
            if (draggedIndex >= 0)
            {
                const auto& t = dragged->transmissions[(size_t)draggedIndex];
                if (t.groupMaster == master->name && t.manual == master->primaryManual) return nullptr;
            }
        }
        return row;
    }
    return nullptr;
}

StopRow* Voicer::dwellCandidateAt(int manual, const juce::String& key, juce::Point<int> screenPosition) const
{
    Stop* dragged = nullptr;
    int draggedIndex = -1;
    if (!resolveItem(manual, key, dragged, draggedIndex)) return nullptr;
    auto acceptable = [&](StopRow* row)
    {
        if (row == nullptr || row->isTransmission()) return false;
        auto* master = row->getStop();
        if (master == nullptr || master == dragged || master->groupMaster.isNotEmpty()) return false;   // one level only
        if (!hoverCopy)   // already its child (a copy of a child may still go into the folder)
        {
            if (draggedIndex < 0 && dragged->groupMaster == master->name && dragged->primaryManual == master->primaryManual) return false;
            if (draggedIndex >= 0)
            {
                const auto& t = dragged->transmissions[(size_t)draggedIndex];
                if (t.groupMaster == master->name && t.manual == master->primaryManual) return false;
            }
        }
        return true;
    };
    // The stop rested over stays the candidate while the item is pulled down onto its bar.
    if (auto* current = dwell.row.getComponent(); current != nullptr && acceptable(current))
    {
        auto zone = current->localAreaToGlobal(current->getStopButtonBounds());
        zone.setHeight(zone.getHeight() + 30);
        if (zone.contains(screenPosition)) return current;
    }
    for (auto* row : stopRows)
        if (acceptable(row) && row->localAreaToGlobal(row->getStopButtonBounds()).contains(screenPosition)) return row;
    return nullptr;
}

void Voicer::resetDwell()
{
    dwell.row = nullptr;
    dwell.shown = dwell.armed = false;
    dwell.since = 0.0;
}

void Voicer::updateIndentIndicator(int manual, const juce::String& key, juce::Point<int> screenPosition)
{
    lastHover = { true, manual, key, screenPosition };
    auto showMarker = [this](StopRow* target, bool bright)
    {
        if (indentMarker == nullptr)
        {
            struct Marker final : public juce::Component
            {
                void paint(juce::Graphics& g) override { g.fillAll(onColour()); }
            };
            indentMarker = std::make_unique<Marker>();
            indentMarker->setInterceptsMouseClicks(false, false);
        }
        // An orange bar right under the master's button: "the item will be indented here".
        auto* parent = target->getParentComponent();
        if (parent == nullptr) return;
        if (indentMarker->getParentComponent() != parent) parent->addAndMakeVisible(indentMarker.get());
        const auto button = target->getStopButtonBounds().translated(target->getX(), target->getY());
        indentMarker->setBounds(button.getX() + 28, button.getBottom() + 2, button.getWidth() + 40, bright ? 5 : 3);
        indentMarker->setAlpha(bright ? 1.0f : 0.4f);
        indentMarker->setVisible(true);
        indentMarker->toFront(false);
    };
    // The open voicing of an existing stop folder takes the item at once.
    if (auto* target = indentTargetAt(manual, key, screenPosition))
    {
        resetDwell();
        showMarker(target, true);
        return;
    }
    // Any other stop's button: resting there for a moment shows a dim bar under it;
    // pulling the item down onto the bar arms the indent (bright), pulling it back up disarms.
    auto* candidate = dwellCandidateAt(manual, key, screenPosition);
    if (candidate == nullptr)
    {
        resetDwell();
        if (indentMarker != nullptr) indentMarker->setVisible(false);
        return;
    }
    const double now = juce::Time::getMillisecondCounterHiRes();
    if (dwell.row.getComponent() != candidate)
    {
        resetDwell();
        dwell.row = candidate;
        dwell.since = now;
        dwell.anchor = screenPosition;
        if (indentMarker != nullptr) indentMarker->setVisible(false);
        return;
    }
    if (!dwell.shown)
    {
        // Still moving across the button: the rest starts again.
        if (screenPosition.getDistanceFrom(dwell.anchor) > 6) { dwell.since = now; dwell.anchor = screenPosition; }
        if (now - dwell.since < 800.0) { if (indentMarker != nullptr) indentMarker->setVisible(false); return; }
        dwell.shown = true;
        dwell.anchor = screenPosition;
    }
    const int pulledDown = screenPosition.y - dwell.anchor.y;
    if (!dwell.armed && pulledDown >= 12) dwell.armed = true;
    else if (dwell.armed && pulledDown <= 4) dwell.armed = false;
    showMarker(candidate, dwell.armed);
}

void Voicer::freeCellClicked(int manual, int column, int row, juce::Point<int> screenPosition)
{
    if (!arrangeMode) return;
    juce::PopupMenu menu;
    menu.addItem(1, "New stop folder here");
    // Ready-made stops the loaded folder trees hold but this division does not:
    // placed here whole (every perspective of theirs).
    std::vector<juce::String> ready;
    if (readyStopNames) ready = readyStopNames(manual);
    static constexpr int readyBase = 1000;
    if (!ready.empty())
    {
        juce::PopupMenu stops;
        for (size_t i = 0; i < ready.size(); ++i) stops.addItem(readyBase + (int)i, ready[i]);
        menu.addSubMenu("New stop here (from the loaded folders)", stops);
    }
    // The division's pooled items (stops and transmissions taken off its grid).
    std::vector<juce::String> pooled;
    if (auto found = hiddenStops.find(manual); found != hiddenStops.end())
        for (const auto& key : found->second)
        {
            Stop* stop = nullptr;
            int index = -1;
            if (!resolveItem(manual, key, stop, index)) continue;
            pooled.push_back(key);
            menu.addItem((int)pooled.size() + 1, "Place \"" + (index >= 0 ? transmissionLabel(stop, index) : stop->displayName()) + "\" here (from the pool)");
        }
    juce::Component::SafePointer<juce::Component> safeContainer(uiContainer.getComponent());
    // The target component gives the menu the window's interface size; the area then places it at the mouse.
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(uiContainer.getComponent())
            .withTargetScreenArea({ screenPosition.x, screenPosition.y, 1, 1 }),
        [this, safeContainer, manual, column, row, pooled, ready](int result)
        {
            if (safeContainer == nullptr || result <= 0 || !arrangeMode) return;
            // The arrange mode goes on (the owner adds the stop in place and rebuilds
            // the grid); only the drag state is dropped.
            if (result == 1)
            {
                dropStopReferences();
                if (newStopFolderRequest)
                    juce::MessageManager::callAsync([this, manual, column, row] { if (newStopFolderRequest) newStopFolderRequest(manual, column, row); });
                return;
            }
            if (result >= readyBase && result - readyBase < (int)ready.size())
            {
                const juce::String name = ready[(size_t)(result - readyBase)];
                dropStopReferences();
                if (newStopRequest)
                    juce::MessageManager::callAsync([this, manual, column, row, name] { if (newStopRequest) newStopRequest(manual, column, row, name); });
                return;
            }
            const size_t which = (size_t)(result - 2);
            if (which >= pooled.size()) return;
            stopCells[manual][pooled[which]] = { column, row };
            setStopHidden(manual, pooled[which], false);   // loads its samples again when it had none
            if (stateChanged) stateChanged();
            if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
        });
}

void Voicer::removeItem(int manual, const juce::String& key)
{
    Stop* stop = nullptr;
    int index = -1;
    if (!resolveItem(manual, key, stop, index)) return;
    if (index >= 0) { removeTransmission(stop, index); return; }   // a transmission simply goes (rebuilds asynchronously)
    stopCells[manual].erase(key);
    hiddenStops[manual].erase(key);
    // The arrange mode stays: the owner removes the stop alone (nothing else is
    // reloaded) and rebuilds the grid; the drag state must not point at the row.
    resetDwell();
    lastHover.active = false;
    if (indentMarker != nullptr) indentMarker->setVisible(false);
    if (removeStopRequest)
    {
        Stop* leaving = stop;
        juce::MessageManager::callAsync([this, leaving] { if (removeStopRequest) removeStopRequest(leaving); });
    }
}

void Voicer::forgetStop(Stop* stop)
{
    if (stop == nullptr) return;
    forgetTransmissionCells(stop);
    if (scrollTarget == stop) scrollTarget = nullptr;
    resetDwell();
    lastHover.active = false;
}

void Voicer::dropStopReferences()
{
    scrollTarget = nullptr;
    resetDwell();
    lastHover.active = false;
    if (indentMarker != nullptr) indentMarker->setVisible(false);
}

void Voicer::dropItem(int manual, const juce::String& key, juce::Component* row, juce::Point<int> screenPosition, bool duplicate)
{
    if (row == nullptr || key.isEmpty() || uiContainer == nullptr || uiViewport == nullptr) return;
    // screenPosition is the centre of the dragged button (or of its travelling
    // copy); it decides the target cell - in any division, on either screen
    // (every cell lies in the container of its division's screen).
    const CompactCell* target = nullptr;
    const CompactCell* origin = nullptr;
    for (const auto& cell : compactCells)
    {
        auto* cellContainer = containerOf(cell.manual);
        if (cellContainer == nullptr) cellContainer = uiContainer;
        const auto centre = cellContainer->getLocalPoint(nullptr, screenPosition);
        if (cell.bounds.contains(centre)) target = &cell;
        if (cell.manual == manual && cell.key == key) origin = &cell;
    }
    // No cell there: any spot inside a division's column still counts - below
    // the last row the cell is computed from the grid's geometry (empty rows
    // above it are allowed), so an item can be put anywhere and stays there.
    CompactCell virtualCell;
    if (target == nullptr)
        for (int m = 0; m < SamplerEngine::maxManuals && target == nullptr; ++m)
        {
            const auto& geo = gridGeometry[(size_t)m];
            if (!geo.valid || geo.columns <= 0) continue;
            auto* cellContainer = containerOf(m);
            if (cellContainer == nullptr) cellContainer = uiContainer;
            const auto local = cellContainer->getLocalPoint(nullptr, screenPosition);
            const bool inColumns = local.x >= geo.x && local.x < geo.x + geo.columns * geo.drawWidth;
            if (!inColumns || local.y < geo.bottom) continue;
            virtualCell.key = {};
            virtualCell.manual = m;
            virtualCell.column = juce::jlimit(0, geo.columns - 1, (local.x - geo.x) / juce::jmax(1, geo.drawWidth));
            virtualCell.row = geo.rows + (local.y - geo.bottom) / juce::jmax(1, geo.cellHeight);
            target = &virtualCell;
        }
    auto rebuildLater = [this]
    {
        // After the mouse event has finished (the dragged row is destroyed by the
        // rebuild); this also snaps the row back when dropped outside the grid.
        juce::MessageManager::callAsync([this]
        {
            if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
        });
    };
    // The dragged item: a stop of this division, or one of its transmissions.
    Stop* source = nullptr;
    int sourceIndex = -1;   // transmission index when the item is a transmission
    if (!resolveItem(manual, key, source, sourceIndex)) { rebuildLater(); return; }
    // Dropped on the open voicing of a stop folder, or armed by the rest-and-pull-down
    // gesture over any stop's button: the item is indented under that stop (this
    // comes before the cells: the band is no cell).
    // A copy (a new extension or transmission) dropped there is created straight
    // under that stop; a stop folder cannot be copied, so a dragged folder is no copy.
    hoverCopy = duplicate;
    StopRow* masterRow = indentTargetAt(manual, key, screenPosition);
    if (masterRow == nullptr && dwell.armed) masterRow = dwell.row.getComponent();
    resetDwell();
    lastHover.active = false;
    if (masterRow != nullptr)
    {
        Stop* master = masterRow->getStop();
        if (indentMarker != nullptr) indentMarker->setVisible(false);
        if (duplicate)
        {
            // A new transmission of the stop into the master's division, indented under
            // the master at once (with the transposition and volume of a dragged transmission).
            const int shift = sourceIndex >= 0 ? source->transmissions[(size_t)sourceIndex].shift : 0;
            const int index = engine.addTransmission(source, master->primaryManual, shift);
            if (index >= 0)
            {
                std::vector<Stop::Transmission> list = source->transmissions;
                auto& t = list[(size_t)index];
                if (sourceIndex >= 0) t.db = source->transmissions[(size_t)sourceIndex].db;
                t.groupMaster = master->name;
                engine.replaceTransmissions(source, list);
                if (source->samplesUnloaded) engine.reloadStopSamples(source);   // a pooled stop must sound through it
                CrashHandler::note("Grid: new transmission of " + source->name + " (shift " + juce::String(shift)
                    + ") indented under \"" + master->name + "\" in division " + juce::String(master->primaryManual));
            }
            if (stateChanged) stateChanged();
            rebuildLater();
            return;
        }
        if (sourceIndex < 0 && (!groupChildren(source).empty() || !groupChildTransmissions(source).empty()))
        {
            // A stop folder dropped under a stop: its items go under that stop and the
            // folder dissolves (one level only) - after a question, since an empty
            // folder is removed by it. "Cancel" leaves everything as it was.
            juce::StringArray names;
            for (auto* child : groupChildren(source)) names.add(child->displayName());
            for (const auto& [s, idx] : groupChildTransmissions(source)) names.add(transmissionLabel(s, idx));
            Stop* folder = source;
            juce::Component* parent = uiContainer != nullptr ? uiContainer->getTopLevelComponent() : nullptr;
            juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon, "Merge stop folder",
                mergeFolderText(folder->name, folder->alias, !folder->container, names, master->displayName()), "Merge", "Cancel", parent,
                juce::ModalCallbackFunction::create([this, folder, master, manual, key, rebuildLater](int result)
                {
                    bool folderAlive = false, masterAlive = false;
                    for (auto* s : engine.getStops()) { if (s == folder) folderAlive = true; if (s == master) masterAlive = true; }
                    if (result == 0 || !folderAlive || !masterAlive) { rebuildLater(); return; }
                    // The folder's child transmissions move under the target (their division too when it differs).
                    for (const auto& [s, idx] : groupChildTransmissions(folder))
                    {
                        std::vector<Stop::Transmission> list = s->transmissions;
                        if (idx < 0 || idx >= (int)list.size()) continue;
                        const bool moved = list[(size_t)idx].manual != master->primaryManual;
                        list[(size_t)idx].groupMaster = master->name;
                        list[(size_t)idx].manual = master->primaryManual;
                        engine.replaceTransmissions(s, list);
                        if (moved) forgetTransmissionCells(s);
                    }
                    stopCells[manual].erase(key);
                    hiddenStops[manual].erase(key);
                    if (master->primaryManual != manual)
                    {
                        retargetSavedStop(folder->name, manual, master->primaryManual);
                        for (auto* child : groupChildren(folder)) retargetSavedStop(child->name, manual, master->primaryManual);
                    }
                    if (stateChanged) stateChanged();
                    dropStopReferences();   // the arrange mode goes on; the owner merges in place and rebuilds the grid
                    if (mergeStopFolderRequest)
                        juce::MessageManager::callAsync([this, folder, master] { if (mergeStopFolderRequest) mergeStopFolderRequest(folder, master); });
                    else rebuildLater();
                }));
            return;
        }
        if (sourceIndex >= 0)
        {
            // A transmission: into the master's division when needed, then under the master.
            std::vector<Stop::Transmission> list = source->transmissions;
            auto& t = list[(size_t)sourceIndex];
            const bool moved = t.manual != master->primaryManual;
            t.manual = master->primaryManual;
            t.groupMaster = master->name;
            engine.replaceTransmissions(source, list);
            if (moved) forgetTransmissionCells(source);
            else { stopCells[manual].erase(key); hiddenStops[manual].erase(key); }
            if (stateChanged) stateChanged();
            rebuildLater();
            return;
        }
        stopCells[manual].erase(key);
        hiddenStops[manual].erase(key);
        if (master->primaryManual != manual) retargetSavedStop(source->name, manual, master->primaryManual);
        dropStopReferences();   // the arrange mode goes on; the owner indents in place and rebuilds the grid
        if (indentStopRequest)
        {
            Stop* moving = source;
            juce::MessageManager::callAsync([this, moving, master] { if (indentStopRequest) indentStopRequest(moving, master); });
        }
        else rebuildLater();
        return;
    }
    if (indentMarker != nullptr) indentMarker->setVisible(false);
    if (target == nullptr) { rebuildLater(); return; }
    // A narrow window shows fewer columns than the stored layout has (the
    // rightmost columns are zipped into their neighbours). A drop then refers to
    // the columns and rows on the screen, so the stored cells of the divisions
    // concerned are rewritten to what is visible first; otherwise the dropped
    // item would land in the hidden wide layout and seem to jump back.
    for (const int m : { manual, target->manual })
        for (const auto& cell : compactCells)
            if (cell.manual == m && cell.key.isNotEmpty()) stopCells[m][cell.key] = { cell.column, cell.row };
    const std::pair<int, int> cell{ target->column, target->row };
    const bool cellFree = target->key.isEmpty();
    CrashHandler::note("Grid drop: " + key + " from division " + juce::String(manual) + " at screen " + screenPosition.toString()
        + " -> division " + juce::String(target->manual) + " cell " + juce::String(cell.first) + "," + juce::String(cell.second)
        + (target == &virtualCell ? " (below the last row)" : cellFree ? " (free)" : " (occupied by " + target->key + ")") + (duplicate ? ", a copy" : ""));

    // A stop folder's child dropped on a cell leaves the folder and takes the cell.
    if (!duplicate && sourceIndex >= 0 && source->transmissions[(size_t)sourceIndex].groupMaster.isNotEmpty())
    {
        std::vector<Stop::Transmission> list = source->transmissions;
        auto& t = list[(size_t)sourceIndex];
        const bool moved = t.manual != target->manual;
        t.groupMaster.clear();
        t.manual = target->manual;
        engine.replaceTransmissions(source, list);
        if (moved) forgetTransmissionCells(source);
        else if (cellFree) stopCells[target->manual][key] = cell;
        if (stateChanged) stateChanged();
        rebuildLater();
        return;
    }
    if (!duplicate && sourceIndex < 0 && source->groupMaster.isNotEmpty() && target->manual == manual)
    {
        if (cellFree) stopCells[manual][key] = cell;
        hiddenStops[manual].erase(key);
        dropStopReferences();   // the arrange mode goes on; the owner un-indents in place and rebuilds the grid
        if (outdentStopRequest)
        {
            Stop* leaving = source;
            juce::MessageManager::callAsync([this, leaving] { if (outdentStopRequest) outdentStopRequest(leaving); });
        }
        else rebuildLater();
        return;
    }

    if (duplicate)
    {
        // A copy: a new transmission of the stop into the target division (its own
        // division included), with the transposition of a dragged transmission.
        const int shift = sourceIndex >= 0 ? source->transmissions[(size_t)sourceIndex].shift : 0;
        const int index = engine.addTransmission(source, target->manual, shift);
        if (index >= 0)
        {
            if (sourceIndex >= 0)
            {
                const juce::ScopedLock lock(engine.getLock());
                source->transmissions[(size_t)index].db = source->transmissions[(size_t)sourceIndex].db;
            }
            if (source->samplesUnloaded) engine.reloadStopSamples(source);   // a pooled stop must sound through it
            if (cellFree) stopCells[target->manual][transmissionKey(source, index)] = cell;
        }
        if (stateChanged) stateChanged();
        rebuildLater();
        return;
    }
    if (target->manual == manual)
    {
        if (target->key != key)
        {
            auto& cells = stopCells[manual];
            // An occupied cell swaps: the other item takes the dragged item's old cell.
            if (target->key.isNotEmpty() && origin != nullptr)
                cells[target->key] = { origin->column, origin->row };
            cells[key] = cell;
        }
        rebuildLater();
        return;
    }
    // Another division.
    if (sourceIndex >= 0)
    {
        // A transmission moves: the same one made in the new division, the old one removed.
        const auto old = source->transmissions[(size_t)sourceIndex];
        const int index = engine.addTransmission(source, target->manual, old.shift);
        if (index >= 0)
        {
            {
                const juce::ScopedLock lock(engine.getLock());
                auto& moved = source->transmissions[(size_t)index];
                moved.db = old.db; moved.keyDb = old.keyDb; moved.keyTone = old.keyTone; moved.alias = old.alias;
            }
            removeTransmission(source, sourceIndex);   // rebuilds asynchronously; later indices move down
            const int newIndex = index > sourceIndex ? index - 1 : index;
            if (cellFree) stopCells[target->manual][transmissionKey(source, newIndex)] = cell;
            if (old.enabled) engine.setTransmissionEnabled(source, newIndex, true);
        }
        if (stateChanged) stateChanged();
        rebuildLater();
        return;
    }
    // A stop moves house: its grid cell and saved voicing go with it, the owner
    // moves its table row and reloads both divisions.
    stopCells[manual].erase(key);
    hiddenStops[manual].erase(key);
    if (cellFree) stopCells[target->manual][key] = cell;
    retargetSavedStop(source->name, manual, target->manual);
    for (auto* child : groupChildren(source)) retargetSavedStop(child->name, manual, target->manual);
    // The arrange mode goes on (0.8.153): the owner moves the stop in place and
    // rebuilds the grid - the exit dated from the days the move reloaded both
    // divisions, and the user arranging several stops lost the mode at each.
    dropStopReferences();
    if (moveStopRequest)
    {
        Stop* moving = source;
        const int destination = target->manual;
        juce::MessageManager::callAsync([this, moving, destination] { if (moveStopRequest) moveStopRequest(moving, destination); });
    }
    else rebuildLater();
}

juce::String Voicer::mergeFolderText(const juce::String& folderName, const juce::String& folderAlias, bool folderHasPipes,
    const juce::StringArray& children, const juce::String& targetName)
{
    const juce::String button = "\"" + folderName + "\"" + (folderAlias.isNotEmpty() ? " (alias \"" + folderAlias + "\")" : juce::String());
    juce::String text = button + " is a stop folder with " + juce::String(children.size()) + " item(s):\n  " + children.joinIntoString("\n  ")
        + "\n\nIndented under \"" + targetName + "\", these become children of \"" + targetName + "\" and the folder dissolves (folders have one level only).\n";
    if (folderHasPipes)
        text << "\"" << folderName << "\" keeps its own pipes and becomes a child of \"" << targetName << "\" as well.";
    else
        text << "The folder button " << button << " has no pipes of its own and would be left with nothing: it is removed.";
    text << "\n\nMerge?";
    return text;
}

juce::String Voicer::transmissionKey(const Stop* stop, int index)
{
    // Distinct from any stop name of the receiving division. The first
    // transmission into a division keeps the key older layouts used.
    const int ordinal = juce::jmax(0, stop->transmissionOrdinal(index));
    return "tr:" + juce::String(stop->primaryManual) + ":" + stop->name
        + (ordinal > 0 ? "#" + juce::String(ordinal) : juce::String());
}

juce::String Voicer::transmissionLabel(const Stop* stop, int index)
{
    if (stop == nullptr) return {};
    if (index < 0 || index >= (int)stop->transmissions.size()) return stop->displayName() + " (Trsm.)";
    const auto& t = stop->transmissions[(size_t)index];
    const auto suffix = SamplerEngine::transmissionSuffix(stop, t);   // " (Ext.)" in the stop's own division, " (Trsm.)" elsewhere
    if (t.alias.isNotEmpty())
    {
        // An alias without a footage gets the transmission's footage, like every
        // other caption; the mark follows the alias as it follows the automatic name.
        int written = 0;
        if (SamplerEngine::footageFromName(t.alias, written)) return footageDisplayName(t.alias) + suffix;   // the foot mark on a bare number ("Plockfloett 4" -> "4'")
        return t.alias + " " + SamplerEngine::footageLabel(SamplerEngine::displayedFootageSemitones(stop) + t.shift) + suffix;
    }
    return SamplerEngine::transmissionCaption(stop, t);
}

void Voicer::removeTransmission(Stop* stop, int index)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    const int targetManual = stop->transmissions[(size_t)index].manual;
    const int ordinal = stop->transmissionOrdinal(index);
    const int count = stop->transmissionCount(targetManual);
    // Forget its grid cell and pool entry in the target division; the later
    // transmissions into that division move down one ordinal.
    auto& cells = stopCells[targetManual];
    auto& pool = hiddenStops[targetManual];
    auto keyFor = [stop, targetManual](int ord)
    {
        return "tr:" + juce::String(stop->primaryManual) + ":" + stop->name
            + (ord > 0 ? "#" + juce::String(ord) : juce::String());
    };
    cells.erase(keyFor(ordinal));
    pool.erase(keyFor(ordinal));
    for (int ord = ordinal + 1; ord < count; ++ord)
    {
        if (const auto it = cells.find(keyFor(ord)); it != cells.end())
        {
            const auto cell = it->second;
            cells.erase(it);
            cells[keyFor(ord - 1)] = cell;
        }
        if (pool.erase(keyFor(ord)) > 0) pool.insert(keyFor(ord - 1));
    }
    engine.removeTransmission(stop, index);   // switches it off first (pipes released properly)
    // Callers may sit inside a control of the current UI: rebuild afterwards.
    juce::MessageManager::callAsync([this]
    {
        if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
    });
    if (stateChanged) stateChanged();
}

bool Voicer::isStopHidden(int manual, const juce::String& name) const
{
    const auto it = hiddenStops.find(manual);
    return it != hiddenStops.end() && it->second.count(name) != 0;
}

void Voicer::setStopHidden(int manual, const juce::String& name, bool hidden)
{
    auto& pool = hiddenStops[manual];
    if (hidden) pool.insert(name); else pool.erase(name);
    // RAM: a pooled stop without transmissions releases its samples; a stop put
    // back on the grid loads them again. Stops with transmissions stay loaded
    // because their transmissions keep sounding.
    for (auto* stop : engine.getStops())
    {
        if (stop->primaryManual != manual || stop->name != name) continue;
        if (hidden && stop->transmissions.empty()) engine.unloadStopSamples(stop);
        else if (!hidden && stop->samplesUnloaded) engine.reloadStopSamples(stop);
    }
    // Keep the loaders informed for future folder loads.
    std::map<int, std::set<juce::String>> pooled;
    for (const auto& division : hiddenStops)
        for (const auto& pooledName : division.second)
        {
            bool hasTransmissions = false;
            for (auto* stop : engine.getStops())
                if (stop->primaryManual == division.first && stop->name == pooledName && !stop->transmissions.empty())
                    hasTransmissions = true;
            if (!hasTransmissions && !pooledName.startsWith("tr:")) pooled[division.first].insert(pooledName);
        }
    engine.setPooledStopNames(std::move(pooled));
    if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
    if (stateChanged) stateChanged();
}

juce::var Voicer::createState() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    juce::Array<juce::var> stopsArray;

    for (auto* stop : engine.getStops())
    {
        juce::DynamicObject::Ptr stopObj = new juce::DynamicObject();
        stopObj->setProperty(juce::Identifier("name"), stop->name);
        stopObj->setProperty("perspective", stop->perspective);
        stopObj->setProperty("alias", stop->alias);
        if (!stop->showRankCount) stopObj->setProperty("showRankCount", false);   // a stop folder's "[II]" switched off (0.8.146)
        stopObj->setProperty(juce::Identifier("globalDb"), stop->db);
        stopObj->setProperty(juce::Identifier("gainDb"), stop->gainDb);
        stopObj->setProperty(juce::Identifier("attackMs"), stop->attackMs);
        stopObj->setProperty(juce::Identifier("attackFadeoutMs"), stop->attackFadeoutMs);
        stopObj->setProperty("randomPitchErrorCents", stop->randomPitchErrorCents);
        stopObj->setProperty("formantFollowPct", juce::roundToInt(stop->formantFollow * 100.0f));
        // Undulation is saved only when the user set it; otherwise the value the
        // loader detects from the samples applies (also to celestes recognised later).
        if (stop->undulationManual)
        {
            stopObj->setProperty("undulationBeats", stop->undulationBeats);
            stopObj->setProperty("undulationManual", true);
        }
        stopObj->setProperty(juce::Identifier("primaryManual"), stop->primaryManual);
        stopObj->setProperty("transmissions", transmissionsToVar(stop));
        stopObj->setProperty(juce::Identifier("voicingExpanded"), stop->voicingExpanded);
        stopObj->setProperty("volumeKeysExpanded", stop->volumeKeysExpanded);
        stopObj->setProperty("toneKeysExpanded", stop->toneKeysExpanded);
        juce::Array<juce::var> ranks;
        auto addRankState = [&ranks](const Stop* rank)
        {
            juce::DynamicObject::Ptr state = new juce::DynamicObject();
            state->setProperty("perspective", rank->perspective);
            state->setProperty("enabled", rank->rankEnabled);
            ranks.add(juce::var(state.get()));
        };
        addRankState(stop);
        for (const auto& rank : stop->ranks) addRankState(rank.get());
        stopObj->setProperty("ranks", ranks);

        juce::DynamicObject::Ptr perNoteObj = new juce::DynamicObject();
        juce::DynamicObject::Ptr perNoteToneObj = new juce::DynamicObject();
        for (const int note : stop->voicedNotes())
        {
            perNoteObj->setProperty(juce::Identifier(juce::String(note)), stop->getNoteDb(note));
            perNoteToneObj->setProperty(juce::Identifier(juce::String(note)), stop->getNoteToneDb(note));
        }
        stopObj->setProperty(juce::Identifier("perNoteDb"), juce::var(perNoteObj.get()));
        stopObj->setProperty("perNoteTone", juce::var(perNoteToneObj.get()));
        {
            juce::Array<juce::var> off;
            for (int pipe = 0; pipe < 128; ++pipe) if (stop->pipeDisabled.test((size_t)pipe)) off.add(pipe);
            if (!off.isEmpty()) stopObj->setProperty("disabledPipes", off);
        }
        {
            // Velocity layers ("_NNN" files) switched off in the voicing, by their maxVelocity; 128 = the plain files.
            juce::Array<juce::var> off;
            for (int v = 1; v < 128; ++v) if (stop->velocityLayerDisabled.test((size_t)v)) off.add(v);
            if (stop->plainLayerDisabled()) off.add(128);
            if (!off.isEmpty()) stopObj->setProperty("disabledVelocityLayers", off);
        }
        stopsArray.add(juce::var(stopObj.get()));
    }
    // The stops of the loaded state the engine does not hold: their entries go
    // back unchanged (a stop that exists now wins over its kept entry).
    for (const auto& kept : retainedStopStates)
    {
        const auto name = kept.getProperty("name", "").toString();
        const int manual = (int)kept.getProperty("primaryManual", 0);
        bool present = false;
        for (auto* stop : engine.getStops())
            if (stop->name == name && stop->primaryManual == manual) { present = true; break; }
        if (!present) stopsArray.add(kept);
    }
    root->setProperty(juce::Identifier("stops"), stopsArray);
    // Grid cells of the stops per division (arranged by dragging): "column,row".
    juce::DynamicObject::Ptr gridObj = new juce::DynamicObject();
    for (const auto& division : stopCells)
    {
        if (division.second.empty()) continue;
        juce::DynamicObject::Ptr cellsObj = new juce::DynamicObject();
        for (const auto& cell : division.second)
            cellsObj->setProperty(juce::Identifier(cell.first),
                juce::String(cell.second.first) + "," + juce::String(cell.second.second));
        gridObj->setProperty(juce::Identifier(juce::String(division.first)), juce::var(cellsObj.get()));
    }
    root->setProperty("stopGrid", juce::var(gridObj.get()));
    // Stops taken off the grid ("stop pool") per division.
    juce::DynamicObject::Ptr poolObj = new juce::DynamicObject();
    for (const auto& division : hiddenStops)
    {
        if (division.second.empty()) continue;
        juce::Array<juce::var> names;
        for (const auto& name : division.second) names.add(name);
        poolObj->setProperty(juce::Identifier(juce::String(division.first)), names);
    }
    root->setProperty("stopPool", juce::var(poolObj.get()));
    return juce::var(root.get());
}

void Voicer::saveState(const juce::File& file)
{
    file.replaceWithText(juce::JSON::toString(createState(), true));
}

void Voicer::loadState(const juce::File& file)
{
    if (!file.existsAsFile()) return;

    auto text = file.loadFileAsString();
    auto parsed = juce::JSON::parse(text);
    if (!parsed.isObject()) return;

    if (auto* gridObj = parsed.getProperty("stopGrid", juce::var()).getDynamicObject())
    {
        stopCells.clear();
        for (const auto& division : gridObj->getProperties())
        {
            auto& cells = stopCells[division.name.toString().getIntValue()];
            if (auto* cellsObj = division.value.getDynamicObject())
                for (const auto& cell : cellsObj->getProperties())
                {
                    const auto parts = juce::StringArray::fromTokens(cell.value.toString(), ",", "");
                    if (parts.size() == 2)
                        cells[cell.name.toString()] = { parts[0].getIntValue(), parts[1].getIntValue() };
                }
        }
        // Callers usually rebuild the UI before loading the state; lay the
        // stops out again in the loaded order once this call has finished.
        if (uiContainer != nullptr && uiViewport != nullptr)
            juce::MessageManager::callAsync([this]
            {
                if (uiContainer != nullptr && uiViewport != nullptr) rebuildUI(*uiContainer, *uiViewport);
            });
    }

    if (auto* poolObj = parsed.getProperty("stopPool", juce::var()).getDynamicObject())
    {
        hiddenStops.clear();
        for (const auto& division : poolObj->getProperties())
            if (auto* array = division.value.getArray())
                for (const auto& name : *array)
                    hiddenStops[division.name.toString().getIntValue()].insert(name.toString());
    }
    // Pooled stops without transmissions are not loaded into RAM at all. The
    // transmission masks come from this state too (the stops may not exist yet).
    {
        std::map<int, std::set<juce::String>> pooled;
        if (auto* stopsInState = parsed.getProperty("stops", juce::var()).getArray())
            for (const auto& stopVar : *stopsInState)
            {
                const auto name = stopVar.getProperty("name", "").toString();
                const int manual = juce::jlimit(0, SamplerEngine::maxManuals - 1, (int)stopVar.getProperty("primaryManual", 0));
                if (transmissionsFromVar(stopVar, manual).empty() && isStopHidden(manual, name)) pooled[manual].insert(name);
            }
        engine.setPooledStopNames(std::move(pooled));
    }

    auto stopsArray = parsed.getProperty(juce::Identifier("stops"), juce::var());
    if (!stopsArray.isArray()) return;
    // Stops moved between divisions since the state was saved: their saved
    // entries are read as belonging to the new division (once).
    const std::vector<StateRetarget> retargets = std::move(stateRetargets);
    stateRetargets.clear();
    retainedStopStates.clear();
    juce::StringArray retainedNames;

    for (const auto& stopVar : *stopsArray.getArray())
    {
        if (!stopVar.isObject()) continue;
        auto stopObj = stopVar.getDynamicObject();
        if (!stopObj) continue;

        juce::String name;
        if (stopObj->hasProperty(juce::Identifier("name")))
            name = stopObj->getProperty(juce::Identifier("name")).toString();
        const auto savedPerspective = stopObj->hasProperty("perspective")
            ? canonicalPerspectiveName(stopObj->getProperty("perspective").toString()) : juce::String();

        double globalDb = 0.0;
        if (stopObj->hasProperty(juce::Identifier("globalDb")))
            globalDb = stopObj->getProperty(juce::Identifier("globalDb"));

        double gainDb = 0.0;
        if (stopObj->hasProperty(juce::Identifier("gainDb")))
            gainDb = stopObj->getProperty(juce::Identifier("gainDb"));

        double attackMs = 0.0;
        if (stopObj->hasProperty(juce::Identifier("attackMs")))
            attackMs = stopObj->getProperty(juce::Identifier("attackMs"));

        double attackFadeoutMs = 0.0;
        if (stopObj->hasProperty(juce::Identifier("attackFadeoutMs")))
            attackFadeoutMs = stopObj->getProperty(juce::Identifier("attackFadeoutMs"));
        const float randomPitchErrorCents = stopObj->hasProperty("randomPitchErrorCents")
            ? (float)(double)stopObj->getProperty("randomPitchErrorCents") : 4.0f;
        int legacyMask = stopObj->hasProperty(juce::Identifier("manualMask"))
            ? (int)stopObj->getProperty(juce::Identifier("manualMask")) : 1;
        int primaryManual = stopObj->hasProperty(juce::Identifier("primaryManual"))
            ? (int)stopObj->getProperty(juce::Identifier("primaryManual"))
            : [legacyMask]
              {
                  for (int i = 0; i < SamplerEngine::maxManuals; ++i)
                      if ((legacyMask & (1 << i)) != 0) return i;
                  return 0;
              }();
        for (const auto& retarget : retargets)
            if (retarget.name == name && retarget.from == primaryManual) { primaryManual = retarget.to; break; }
        juce::DynamicObject::Ptr perNoteObj = nullptr;
        if (stopObj->hasProperty(juce::Identifier("perNoteDb")))
        {
            auto perNoteVar = stopObj->getProperty(juce::Identifier("perNoteDb"));
            if (perNoteVar.isObject())
                perNoteObj = perNoteVar.getDynamicObject();
        }

        // The stop the entry belongs to: by name and division; among several of
        // that name the one of the saved primary perspective, else any - the
        // primary perspective may have changed since the save, the voicing and
        // the transmissions are still this stop's.
        Stop* matchedStop = nullptr;
        {
            const int wantedManual = juce::jlimit(0, SamplerEngine::maxManuals - 1, primaryManual);
            Stop* byName = nullptr;
            for (auto* stop : engine.getStops())
                if (stop->name == name && stop->primaryManual == wantedManual)
                {
                    if (savedPerspective.isEmpty() || stop->perspective.equalsIgnoreCase(savedPerspective)) { matchedStop = stop; break; }
                    if (byName == nullptr) byName = stop;
                }
            if (matchedStop == nullptr) matchedStop = byName;
        }
        bool matched = false;
        for (auto* stop : engine.getStops())
        {
            if (stop == matchedStop)
            {
                matched = true;
                stop->setDb((float)globalDb);
                stop->gainDb = (float)gainDb;
                stop->attackMs = (float)attackMs;
                stop->attackFadeoutMs = (float)attackFadeoutMs;
                stop->alias = stopObj->getProperty("alias").toString();
                stop->showRankCount = !stopObj->hasProperty("showRankCount") || (bool)stopObj->getProperty("showRankCount");
                engine.setStopRandomPitchError(stop, randomPitchErrorCents, true);
                // The formant share of the filled pipes: set before any copy is built (no rebuild here).
                {
                    juce::ScopedLock sl(engine.getLock());
                    stop->formantFollow = juce::jlimit(0.0f, 1.0f, stopObj->hasProperty("formantFollowPct")
                        ? (float)(double)stopObj->getProperty("formantFollowPct") / 100.0f : SamplerEngine::defaultFormantFollow);
                }
                // A saved undulation overrides the detected one only when the user set
                // it ("undulationManual"); older states saved every value, so a saved
                // 0 from them is taken as "automatic" and the detection stays.
                if (stopObj->hasProperty("undulationBeats"))
                {
                    const float saved = (float)(double)stopObj->getProperty("undulationBeats");
                    const bool manual = (bool)stopObj->getProperty("undulationManual") || std::abs(saved) > 1.0e-3f;
                    if (manual)
                    {
                        stop->undulationManual = true;
                        engine.setStopUndulation(stop, saved);
                    }
                }
                engine.replaceTransmissions(stop, transmissionsFromVar(stopVar, stop->primaryManual));
                // Loading a folder/state always starts with a compact stop list; the
                // per-key folds are remembered (folded when the state does not say).
                stop->voicingExpanded = false;
                stop->volumeKeysExpanded = (bool)stopObj->getProperty("volumeKeysExpanded");
                stop->toneKeysExpanded = (bool)stopObj->getProperty("toneKeysExpanded");
                const auto ranks = stopObj->hasProperty("ranks")
                    ? stopObj->getProperty("ranks") : juce::var();
                if (ranks.isArray())
                    for (const auto& state : *ranks.getArray())
                    {
                        const auto perspective = canonicalPerspectiveName(
                            state.getProperty("perspective", "").toString());
                        auto apply = [&](Stop* rank)
                        {
                            if (rank->perspective.equalsIgnoreCase(perspective))
                                rank->rankEnabled = state.getProperty("enabled", true);
                        };
                        apply(stop);
                        for (auto& rank : stop->ranks) apply(rank.get());
                    }
                if (perNoteObj)
                {
                    for (const auto& prop : perNoteObj->getProperties())
                    {
                        int note = prop.name.toString().getIntValue();
                        float db = (float)(double)prop.value;
                        {
                            juce::ScopedLock sl(engine.getLock());
                            stop->setNoteDb(note, db);
                        }
                    }
                }
                {
                    juce::ScopedLock sl(engine.getLock());
                    stop->resetNoteTone();
                    if (auto* toneObj = stopObj->getProperty("perNoteTone").getDynamicObject())
                        for (const auto& prop : toneObj->getProperties())
                            stop->setNoteToneDb(prop.name.toString().getIntValue(), (float)(double)prop.value);
                }
                // Pipes switched off in the voicing.
                {
                    std::bitset<128> wanted;
                    if (auto* off = stopObj->getProperty("disabledPipes").getArray())
                        for (const auto& pipe : *off)
                            if ((int)pipe >= 0 && (int)pipe < 128) wanted.set((size_t)(int)pipe);
                    for (int pipe = 0; pipe < 128; ++pipe)
                        if (wanted.test((size_t)pipe) != stop->pipeDisabled.test((size_t)pipe))
                            engine.setPipeEnabled(stop, pipe, !wanted.test((size_t)pipe));
                }
                // Velocity layers switched off in the voicing.
                {
                    std::bitset<128> wanted;   // bit 0 = the plain files (saved as 128)
                    if (auto* off = stopObj->getProperty("disabledVelocityLayers").getArray())
                        for (const auto& v : *off)
                        {
                            if ((int)v == 128) wanted.set(Stop::plainLayerBit);
                            else if ((int)v >= 1 && (int)v < 128) wanted.set((size_t)(int)v);
                        }
                    for (int v = 0; v < 128; ++v)
                        if (wanted.test((size_t)v) != stop->velocityLayerDisabled.test((size_t)v))
                            engine.setVelocityLayerEnabled(stop, v == 0 ? 128 : v, !wanted.test((size_t)v));
                }
                break;
            }
        }
        if (!matched && name.isNotEmpty())
        {
            // No such stop right now (its folders were not found, its tree is not
            // loaded): the entry is kept as it is - under the division it was moved
            // to, if it was - and written back by createState. Dropping it here used
            // to lose the stop's voicing and the transmissions it carried (a stop
            // folder's grouped transmissions among them) on the next save.
            juce::var kept = stopVar;
            if (primaryManual != (int)stopVar.getProperty("primaryManual", -1))
            {
                juce::DynamicObject::Ptr copy = new juce::DynamicObject(*stopObj);
                copy->setProperty("primaryManual", primaryManual);
                kept = juce::var(copy.get());
            }
            retainedStopStates.push_back(kept);
            retainedNames.add(name + " (division " + juce::String(primaryManual) + ")");
        }
    }
    if (!retainedNames.isEmpty())
        CrashHandler::note("Voicing state: " + juce::String(retainedNames.size()) + " stop(s) not loaded, their voicing and transmissions kept: "
            + retainedNames.joinIntoString(", "));

    for (auto* row : stopRows)
        row->updateFromStop();
}

juce::String Voicer::stopListMarkdown() const
{
    struct Item { int y, x; juce::String caption; };
    std::map<int, std::vector<Item>> divisions;
    for (auto* row : stopRows)
    {
        if (row == nullptr || !row->isVisible()) continue;
        const Stop* stop = row->getStop();
        if (stop == nullptr) continue;
        if (auto* t = row->transmission()) { if (t->groupMaster.isNotEmpty()) continue; }   // nested in a stop folder
        else if (stop->groupMaster.isNotEmpty()) continue;
        divisions[row->divisionIndex()].push_back({ row->getY(), row->getX(), row->caption() });
    }
    size_t longest = 0;
    for (auto& [manual, items] : divisions)
    {
        std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.y != b.y ? a.y < b.y : a.x < b.x; });
        longest = std::max(longest, items.size());
    }
    auto cell = [](juce::String text)
    {
        text = text.replace("|", "\\|").trim();
        while (text.contains("  ")) text = text.replace("  ", " ");   // "Cornet  [III]" on the button
        return text;
    };
    juce::String header = "|", rule = "|";
    for (const auto& [manual, items] : divisions)
    {
        header << " " << (manual == 0 ? juce::String("Pedal") : romanNumeral(manual)) << " |";
        rule << "---|";
    }
    juce::String table = header + "\n" + rule + "\n";
    for (size_t i = 0; i < longest; ++i)
    {
        table << "|";
        for (const auto& [manual, items] : divisions)
            table << " " << (i < items.size() ? cell(items[i].caption) : juce::String()) << " |";
        table << "\n";
    }
    int total = 0;
    for (const auto& [manual, items] : divisions) total += (int)items.size();
    return "Stop List rows: " + juce::String(total) + "\n\n" + table;
}

std::vector<Voicer::GridItem> Voicer::gridItems() const
{
    struct Placed { int y, x; GridItem item; };
    std::vector<Placed> placed;
    for (auto* row : stopRows)
    {
        if (row == nullptr || !row->isVisible()) continue;
        Stop* stop = row->getStop();
        if (stop == nullptr) continue;
        GridItem item;
        item.manual = row->divisionIndex();
        item.stop = stop;
        item.caption = row->caption();
        item.enabled = row->routeEnabled();
        if (auto* t = row->transmission())
        {
            if (t->groupMaster.isNotEmpty()) continue;   // nested in a stop folder: the folder is the item
            item.transmission = row->getTransmissionIndex();
        }
        else
        {
            if (stop->groupMaster.isNotEmpty()) continue;
            if (stop->container)
            {
                // A stop folder counts when something under it sounds.
                item.folder = true;
                if (groupChildren(stop).empty() && groupChildTransmissions(stop).empty()) continue;
            }
        }
        placed.push_back({ row->getY(), row->getX(), item });
    }
    std::stable_sort(placed.begin(), placed.end(), [](const Placed& a, const Placed& b)
    {
        if (a.item.manual != b.item.manual) return a.item.manual < b.item.manual;
        return a.y != b.y ? a.y < b.y : a.x < b.x;
    });
    std::vector<GridItem> items;
    items.reserve(placed.size());
    for (auto& p : placed) items.push_back(p.item);
    return items;
}

bool Voicer::wheelBlockedAt(juce::Point<int> screenPosition) const
{
    // The wheel belongs to the sliders only inside the rectangle of an open
    // voicing (a stop's or a transmission's row with its per-key sliders);
    // everywhere else - the rest of the division's column included - it
    // scrolls the workspace, as it does with every voicing folded.
    for (auto* row : stopRows)
        if (row != nullptr && row->isShowing() && row->isExpanded() && row->getScreenBounds().contains(screenPosition))
            return true;
    return false;
}

std::vector<Stop*> Voicer::groupChildren(const Stop* master) const
{
    std::vector<Stop*> children;
    if (master == nullptr || master->groupMaster.isNotEmpty()) return children;   // one level only
    for (auto* stop : engine.getStops())
        if (stop != master && stop->primaryManual == master->primaryManual && stop->groupMaster == master->name)
            children.push_back(stop);
    return children;
}

std::vector<std::pair<Stop*, int>> Voicer::groupChildTransmissions(const Stop* master) const
{
    std::vector<std::pair<Stop*, int>> children;
    if (master == nullptr || master->groupMaster.isNotEmpty()) return children;
    for (auto* stop : engine.getStops())
        for (int index = 0; index < (int)stop->transmissions.size(); ++index)
        {
            const auto& t = stop->transmissions[(size_t)index];
            if (t.manual == master->primaryManual && t.groupMaster == master->name) children.push_back({ stop, index });
        }
    return children;
}

bool Voicer::gridCellOf(int manual, const juce::String& key, int& column, int& row) const
{
    const auto division = stopCells.find(manual);
    if (division == stopCells.end()) return false;
    const auto cell = division->second.find(key);
    if (cell == division->second.end()) return false;
    column = cell->second.first;
    row = cell->second.second;
    return true;
}

void Voicer::setArrangeOrder(int manual, std::vector<juce::String> keys)
{
    stopCells[manual].clear();   // every item of the division is placed afresh
    arrangeOrders[manual] = std::move(keys);
}

void Voicer::forgetTransmissionCells(const Stop* stop)
{
    if (stop == nullptr) return;
    const juce::String prefix = "tr:" + juce::String(stop->primaryManual) + ":" + stop->name;
    for (auto& division : stopCells)
        for (auto it = division.second.begin(); it != division.second.end();)
            it = it->first == prefix || it->first.startsWith(prefix + "#") ? division.second.erase(it) : std::next(it);
    for (auto& division : hiddenStops)
        for (auto it = division.second.begin(); it != division.second.end();)
            it = *it == prefix || it->startsWith(prefix + "#") ? division.second.erase(it) : std::next(it);
}

void Voicer::forgetGridCells(int manual)
{
    auto found = stopCells.find(manual);
    if (found == stopCells.end()) return;
    for (auto it = found->second.begin(); it != found->second.end();)
        it = it->first.startsWith("tr:") ? std::next(it) : found->second.erase(it);
}

void Voicer::swapDivisions(int a, int b)
{
    if (a == b) return;
    std::swap(stopCells[a], stopCells[b]);
    std::swap(hiddenStops[a], hiddenStops[b]);
    // Transmission keys name the home division of their stop ("tr:<manual>:<name>").
    const juce::String keyA = "tr:" + juce::String(a) + ":", keyB = "tr:" + juce::String(b) + ":";
    auto renamed = [&](const juce::String& key)
    {
        if (key.startsWith(keyA)) return keyB + key.substring(keyA.length());
        if (key.startsWith(keyB)) return keyA + key.substring(keyB.length());
        return key;
    };
    for (auto& division : stopCells)
    {
        std::map<juce::String, std::pair<int, int>> cells;
        for (const auto& cell : division.second) cells[renamed(cell.first)] = cell.second;
        division.second = std::move(cells);
    }
    for (auto& division : hiddenStops)
    {
        std::set<juce::String> pool;
        for (const auto& key : division.second) pool.insert(renamed(key));
        division.second = std::move(pool);
    }
}

void Voicer::refreshUndulationLabels()
{
    for (auto* row : stopRows) row->refreshUndulationLabel();
}

void Voicer::resetAll(bool resetGrid)
{
    if (resetGrid)
    {
        stopCells.clear();   // stops return to their load order
        hiddenStops.clear(); // and the stop pool is emptied
        engine.setPooledStopNames({});
        for (auto* stop : engine.getStops())
            if (stop->samplesUnloaded) engine.reloadStopSamples(stop);
    }
    exitArrangeMode();
    for (auto* stop : engine.getStops())
    {
        for (int pipe = 0; pipe < 128; ++pipe)
            if (stop->pipeDisabled.test((size_t)pipe)) engine.setPipeEnabled(stop, pipe, true);
        {
            juce::ScopedLock sl(engine.getLock());
            stop->resetNoteDb();
            stop->resetNoteTone();
            for (auto& t : stop->transmissions) { t.keyDb.fill(0.0f); t.keyTone.fill(0.0f); }
            stop->gainDb = 0.0f;
            stop->attackMs = 75.0f;
            stop->attackFadeoutMs = 75.0f;
        }
    }
    for (auto* row : stopRows)
        row->updateFromStop();
}

void Voicer::updateStopStates()
{
    for (auto* row : stopRows)
        row->updateFromStop();
}

void Voicer::setNoteActive(int note, bool active)
{
    // Use MessageManager::callAsync to ensure UI updates happen on the main thread
    juce::MessageManager::callAsync([this, note, active] {
        for (auto* row : stopRows)
            row->setNoteActive(note, active);
        });
}

void Voicer::pulseManualActivity(int manual)
{
    // Called from the MIDI thread: only touch atomics here. The section
    // headers poll this value from their own GUI timer.
    if (manual < 0 || manual >= SamplerEngine::maxManuals) return;
    manualActivityUntil[(size_t)manual].store(juce::Time::getMillisecondCounterHiRes() + 120.0,
        std::memory_order_relaxed);
}
