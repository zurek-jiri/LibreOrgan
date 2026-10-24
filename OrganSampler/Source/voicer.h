#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#include "UIControls.h"
#include <atomic>
#include <set>

struct DivisionCouplerInfo
{
    juce::String id;
    juce::String label;
    bool enabled = false;
};

//==============================================================================
// One stop row: button, global slider, and a row of per-key vertical sliders
class StopRow : public juce::Component, private juce::Timer
{
public:
    // transmissionIndex >= 0: the row is one of the stop's transmissions (its own
    // button, transposition, volume and per-key voicing; the pipes are the stop's).
    StopRow(Stop* stop, SamplerEngine& engine, int noteMin, int noteMax, int sliderWidth,
        std::function<void(const juce::String&)> midiLearnCallback, std::function<void(Stop*)> relayout,
        int activeManualCount, std::function<void()> stateChanged, int transmissionIndex = -1);
    ~StopRow() override;
    bool isTransmission() const noexcept { return transmissionIndex >= 0; }
    int getTransmissionIndex() const noexcept { return transmissionIndex; }
    Stop::Transmission* transmission() const noexcept
    {
        return transmissionIndex >= 0 && transmissionIndex < (int)stop->transmissions.size()
            ? &stop->transmissions[(size_t)transmissionIndex] : nullptr;
    }
    bool isExpanded() const noexcept { auto* t = transmission(); return t != nullptr ? t->voicingExpanded : stop->voicingExpanded; }
    bool routeEnabled() const noexcept { auto* t = transmission(); return t != nullptr ? t->enabled : stop->enabled; }
    int divisionIndex() const noexcept { auto* t = transmission(); return t != nullptr ? t->manual : stop->primaryManual; }
    int pipeForKey(int note) const noexcept { auto* t = transmission(); return t != nullptr ? note + t->shift : note; }
    juce::String caption() const;   // button text: alias or the automatic name

    void resized() override;
    void paint(juce::Graphics& g) override;   // the field of an expanded row: a slightly darker rounded panel (0.8.179)
    void paintOverChildren(juce::Graphics& g) override;
    // Repaints while the stop's filled-pipe copies are being built (the progress
    // strip on the button) and once more when the build is over.
    void pollBuildProgress();
    bool buildingSeen = false;
    void updateFromStop();           // sync UI with stop's current per-key gains and global db
    void refreshEnabledColour();     // the button's colour from the stop's (or transmission's) on/off state
    void setNoteActive(int note, bool active);   // highlight slider when note is held
    Stop* getStop() const noexcept { return stop; }

    // "Arrange mode" (Android-like): a 3 s press on a stop button starts it,
    // then stops are dragged to other grid cells of their division.
    // Shared by StopRow and TransmissionRow (both are grid items).
    struct ArrangeHooks
    {
        std::function<bool(const juce::Component*)> active;            // is this row in arrange mode?
        std::function<float()> phase;                                   // marching-border animation phase
        std::function<void(juce::Component*)> begin;                    // long press detected
        // Row dropped: the screen position is the centre of the dragged button (or
        // of its travelling copy); duplicate (right button or Shift) makes a
        // transmission there instead of moving the item.
        std::function<void(juce::Component*, juce::Point<int>, bool duplicate)> drop;
        std::function<void(juce::Component*)> remove;   // the "x" in the button's corner: the item leaves the organ
        std::function<void(juce::Component*, juce::Point<int>, bool duplicate)> hover;   // while dragging: shows where an indent would happen (duplicate: the drag makes a copy)
    };
    void setArrangeHooks(ArrangeHooks hooks) { arrange = std::move(hooks); }
    // The stop's Transposition chosen in the voicing (0.8.158): the owner sets it
    // on the Stop List row and the engine, which loads the stop's samples again.
    std::function<void(Stop*, int)> transpositionRequest;
    // The One-shot switch of a stop (0.8.158): on/off, overriding the One-shot Patterns.
    std::function<void(Stop*, bool)> oneShotRequest;
    juce::Rectangle<int> getStopButtonBounds() const { return stopButton.getBounds(); }
    // Development (ORGAN_DUMP_GRID): where the first line's widgets are and whether they show.
    juce::String describeLineOne() const
    {
        auto one = [](const juce::String& name, const juce::Component& c)
        { return name + (c.isVisible() ? " " : " hidden ") + c.getBounds().toString(); };
        return "row " + getBounds().toString() + " | " + one("transposition", shiftBox) + " | " + one("diapason", diapasonLabel) + " | " + one("alias", aliasBox)
            + " | " + one("oneShot", oneShotToggle) + (oneShotToggle.getToggleState() ? " on" : " off") + " | " + one("rankCount", rankCountToggle);
    }
    // Expanded rows carry two lines of controls above the per-key sliders.
    static constexpr int expandedControlsHeight = 104;
    // Height of the per-key area below them: the "Volume per key" and "Treble per
    // key" headers and, when unfolded, their rows of vertical sliders.
    // Volume row labels: MIDI number, note name and the pipe on/off checkbox.
    static constexpr int keyHeaderHeight = 22, keySliderHeight = 180, keyLabelsHeight = 40, toneLabelsHeight = 12;
    static int keyAreaHeight(bool volumeExpanded, bool toneExpanded) noexcept
    {
        return keyHeaderHeight + (volumeExpanded ? keyLabelsHeight + keySliderHeight : 0)
            + keyHeaderHeight + (toneExpanded ? toneLabelsHeight + keySliderHeight : 0);
    }
    // Folding of the two per-key rows, per stop (or transmission): the owner
    // relayouts and saves when a header is clicked; section 0 = volume, 1 = tone.
    void setVoicingFolds(std::function<void(int)> toggle) { toggleSection = std::move(toggle); }
    // A stop folder without pipes of its own shows one line of information and
    // the alias only; every other expanded row has the controls and the key rows.
    static constexpr int containerExpandedHeight = 58;
    // The "Velocity layers:" boxes follow the sliders; in a column too narrow for
    // them they wrap onto a third line and the row grows by velocityLineHeight.
    static constexpr int velocityLineHeight = 30;
    static int velocityRowWidth(const Stop* stop) noexcept
    {
        const auto layers = stop->velocityLayerValues();
        return layers.empty() ? 0 : 104 + 66 * ((int)layers.size() + 1);   // label, every layer, "_128"
    }
    static constexpr int slidersRowWidth() noexcept   // margin, the widest inset and the six sliders as resized() places them
    {
        return 5 + 26 + 134 + gainSliderWidth + attackSliderWidth
            + randomPitchSliderWidth + undulationSliderWidth + formantSliderWidth + 6 * voicingSliderGap;
    }
    static bool velocityRowWraps(const Stop* stop, int rowWidth) noexcept
    {
        const int needed = velocityRowWidth(stop);
        return needed > 0 && slidersRowWidth() + needed > rowWidth;
    }
    // rowWidth: the width the row will get; a narrow column adds the wrapped velocity line.
    static int expandedHeightFor(const Stop* stop, int transmissionIndex, int rowWidth = 1 << 30) noexcept
    {
        if (stop == nullptr) return containerExpandedHeight;
        if (transmissionIndex < 0 && stop->container) return containerExpandedHeight;
        bool volume = stop->volumeKeysExpanded, tone = stop->toneKeysExpanded;
        if (transmissionIndex >= 0 && transmissionIndex < (int)stop->transmissions.size())
        {
            volume = stop->transmissions[(size_t)transmissionIndex].volumeKeysExpanded;
            tone = stop->transmissions[(size_t)transmissionIndex].toneKeysExpanded;
        }
        const int velocityLine = transmissionIndex < 0 && velocityRowWraps(stop, rowWidth) ? velocityLineHeight : 0;
        return expandedControlsHeight + velocityLine + 4 + keyAreaHeight(volume, tone) + 4;
    }
    // Appends the recorded temperament to the "Orig.: a1 = ..." caption.
    void setTemperamentText(const juce::String& text)
    {
        if (stop->container) return;   // no pipes, no temperament
        diapasonLabel.setText(origText + "; temperament = " + text.replace("Temperament: ", ""), juce::dontSendNotification);
        resized();   // the label is as wide as its text
    }
    // Called when the transmission count of a division is lowered: (stop, index
    // into stop->transmissions). The owner removes it and its grid/pool entries.
    void setTransmissionRemover(std::function<void(Stop*, int)> remover) { transmissionRemover = std::move(remover); }
    // Stop folders: the children of a master stop (they sound with it, their
    // voicing opens under its own).
    void setChildrenGetter(std::function<std::vector<Stop*>(const Stop*)> getter,
        std::function<std::vector<std::pair<Stop*, int>>(const Stop*)> transmissionGetter = nullptr)
    {
        childrenGetter = std::move(getter);
        childTransmissionsGetter = std::move(transmissionGetter);
        stopButton.setButtonText(caption());   // "[N]" on a stop folder's button
        // The rank-count switch is enabled where the button can carry a count: known
        // only now, the getters come after construction (0.8.153: it stayed greyed).
        const bool folder = (childrenGetter && !childrenGetter(stop).empty())
            || (childTransmissionsGetter && !childTransmissionsGetter(stop).empty());
        rankCountToggle.setEnabled(folder);
        rankCountToggle.setVisible(isExpanded() && !isTransmission() && folder);   // shown on stop folders only (0.8.158)
        resized();
    }

private:
    std::function<std::vector<Stop*>(const Stop*)> childrenGetter;
    std::function<std::vector<std::pair<Stop*, int>>(const Stop*)> childTransmissionsGetter;   // transmissions indented under a master
    ArrangeHooks arrange;
    bool longPressCandidate = false;
    bool arrangeDragging = false;
    bool arrangeDuplicate = false;   // the drag makes a transmission (right button or Shift)
    std::unique_ptr<juce::Label> dragGhost;   // the copy that travels with the mouse while duplicating (the row stays)
    juce::Point<int> pressScreenPosition;
    juce::Point<int> dragStartTopLeft;
    void timerCallback() override;
    Stop* stop;
    SamplerEngine& engine;
    int transmissionIndex = -1;
    int noteMin;
    int noteMax;
    int sliderWidth;

    // Layout constants
    int margin = 5;
    int buttonWidth = 200;
    int globalSliderWidth = 134;
    int topRowHeight = 50;
    int sliderHeight = 180;
    int labelHeight = 32;

    // New constants for the attack‑envelope sliders
    // Voicing sliders: caption above, slider below (value box right of it),
    // slightly shorter than before so that the gaps between them are wider.
    static constexpr int gainSliderWidth = 194;
    static constexpr int attackSliderWidth = 194;
    static constexpr int randomPitchSliderWidth = 214;
    static constexpr int undulationSliderWidth = 234;
    static constexpr int formantSliderWidth = 170;
    static constexpr int sliderSpacing = 8;
    static constexpr int voicingSliderGap = 27;

    // UI elements
    LeftOnlyTextButton stopButton;
    juce::TextButton voicingToggle{ "v" };
    LeftOnlyTextButton removeButton{ "x" };   // arrange mode only: removes the stop (or transmission) from the organ
    juce::Label divisionLabel;
    juce::Label transmissionLabel;
    std::array<juce::ToggleButton, SamplerEngine::maxManuals> primaryManualButtons;
    std::array<juce::ComboBox, SamplerEngine::maxManuals> transmissionBoxes;   // how many transmissions (new stop buttons) per division
    std::function<void(Stop*, int)> transmissionRemover;
    void transmissionCountChanged(int manual);
    // Transposition: a transmission's shift, or (0.8.158) the stop's own; the
    // list holds the pure intervals with the footage each gives. Transmission
    // rows show the source stop instead of the pitch information.
    juce::Label transpositionLabel, sourceLabel;
    juce::ComboBox shiftBox;
    std::unique_ptr<juce::LookAndFeel> shiftMenuLookAndFeel;   // bold octave steps in the list
    // Alias: the caption shown instead of the automatic name ("Automatic" = none).
    juce::Label aliasLabel;
    juce::ComboBox aliasBox;
    void aliasChanged();
    juce::ToggleButton rankCountToggle;   // a stop folder's "[II]" on its button (0.8.146; shown on stop folders only)
    juce::ToggleButton oneShotToggle;     // the stop plays each sample once through (0.8.158; overrides the One-shot Patterns)
    juce::Label ranksLabel;
    juce::OwnedArray<juce::ToggleButton> rankButtons;
    // Velocity layers ("_070" files): one tick box per layer of the stop; unticked,
    // its velocities play the next layer up or the plain samples.
    juce::Label velocityLabel;
    juce::OwnedArray<juce::ToggleButton> velocityButtons;
    LearnableSlider globalSlider;
    juce::Label globalLabel;

    LearnableSlider gainSlider;            // Attack boost gain
    juce::Label gainLabel;
    LearnableSlider attackSlider;          // Attack time: the hold and the fade of the attack gain, half each
    juce::Label attackLabel;
    LearnableSlider randomPitchSlider;
    juce::Label randomPitchLabel;
    juce::Label diapasonLabel;                      // "Orig.: a1 = ...; temperament = ..."
    juce::String origText;                          // the a1 part of that caption
    LearnableSlider undulationSlider;               // celeste: beats per second at a1 (only with 440 Hz)
    juce::Label undulationLabel;
    LearnableSlider formantSlider;                  // pitch shift: formants follow the shift of the filled pipes (per stop / transmission)
    juce::Label formantLabel;
    juce::Label temperamentLabel;                   // estimated recorded temperament (text from the owner)
public:
    void refreshUndulationLabel();                  // cents shown for the current beat ratio
private:

    juce::OwnedArray<LearnableSlider> keySliders;
    juce::OwnedArray<juce::Label> noteNameLabels;
    juce::OwnedArray<juce::Label> midiNumLabels;
    juce::OwnedArray<juce::ToggleButton> pipeBoxes;   // under the note name: the pipe is on (unchecked = silent, samples freed)
    // Second row of per-key sliders: the tone (high shelf, -9..9 dB) per key.
    juce::OwnedArray<LearnableSlider> toneSliders;
    juce::OwnedArray<juce::Label> toneNameLabels;
    SectionHeaderLabel volumeHeader, toneHeader;          // "Volume per key" / "Treble per key"
    juce::TextButton volumeReset{ "Reset to 0 dB" }, toneReset{ "Reset to 0 dB" };
    std::function<void(int)> toggleSection;
    bool volumeRowShown() const { auto* t = transmission(); return t != nullptr ? t->volumeKeysExpanded : stop->volumeKeysExpanded; }
    bool toneRowShown() const { auto* t = transmission(); return t != nullptr ? t->toneKeysExpanded : stop->toneKeysExpanded; }
    void toneSliderChanged(int note, float value);
    void resetKeyRow(bool tone);
    juce::OwnedArray<LearnableSlider>* paintRow = nullptr;   // the row being painted with the right button

    // For note highlighting
    std::unordered_map<int, int> activeNoteCount;          // note -> active count
    std::unordered_map<juce::Slider*, juce::Colour> originalTrackColours;
    bool keySliderPaintActive = false;
    int lastPaintedKeySlider = -1;
    double lastPaintedValue = 0.0;
    double verticalPaintAnchorValue = 0.0;
    std::array<double, 4> verticalNeighbourAnchors{};

    void globalSliderChanged();
    void keySliderChanged(int note, float value);
    void gainSliderChanged();
    void attackSliderChanged();
    void randomPitchSliderChanged();
    void formantSliderChanged();
    void applyStopState(bool enabled);
    int keySliderIndexForComponent(const juce::Component* component, juce::OwnedArray<LearnableSlider>** row = nullptr) const;
    void paintKeySlidersAt(juce::Point<int> screenPosition);
    std::function<void(const juce::String&)> midiLearnCallback;
    std::function<void(Stop*)> relayout;
    std::function<void()> stateChanged;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
};


//==============================================================================
// Main voicer class: manages the UI and state for per-key volume adjustments
class Voicer
{
public:
    Voicer(SamplerEngine& engine, std::function<void(const juce::String&)> midiLearnCallback = {},
        std::function<void(int)> folderCallback = {}, std::function<void(int)> unloadCallback = {},
        std::function<void(int)> settingsCallback = {}, std::function<void(int)> keyboardLearn = {},
        std::function<float(int)> swellGetter = {},
        std::function<void(int, float)> swellSetter = {}, std::function<void(int)> swellLearn = {},
        std::function<bool(int)> swellLearning = {}, std::function<void()> stateChanged = {},
        std::function<std::vector<DivisionCouplerInfo>(int)> couplerGetter = {},
        std::function<void(const juce::String&, bool)> couplerSetter = {},
        std::function<bool(int)> tremulantGetter = {},
        std::function<void(int, bool)> tremulantSetter = {},
        std::function<float(int)> columnWeightGetter = {},
        std::function<void(int, float, int, float)> columnWeightSetter = {},
        std::function<void(int)> stopBuildCallback = {});
    ~Voicer();

    void rebuildUI(juce::Component& container, juce::Viewport& viewport);
    void saveState(const juce::File& file);
    juce::var createState() const;
    void loadState(const juce::File& file);
    // Stops of the loaded state the engine does not hold (their voicing is kept, see retainedStopStates).
    int retainedStopCount() const noexcept { return (int)retainedStopStates.size(); }
    // The Stop List as a Markdown table for a configuration's README: one column
    // per division, the rows in the order of the grid (row by row, left to
    // right) with the captions of the buttons; the rows nested in a stop folder
    // are not listed (the folder's caption carries their number).
    juce::String stopListMarkdown() const;
    // Every button of the grid, in grid order (row by row, left to right): the
    // stops, the transmissions and extensions, and the stop folders that have
    // at least one sound (their own or a child's). Rows nested in a stop folder
    // are not items (the folder is). THIS is what "a stop" means wherever stops
    // are counted or switched: never only the stops with samples of their own.
    struct GridItem { int manual = 0; Stop* stop = nullptr; int transmission = -1; bool folder = false; bool enabled = false; juce::String caption; };
    std::vector<GridItem> gridItems() const;
    void resetAll(bool resetGrid = true);   // reset voicing; optionally also stop grids and the stop pool
    // Forgets the grid cells of a division's own stops (transmissions keep theirs):
    // the next layout places them in load order (the Stop building table's order).
    void forgetGridCells(int manual);
    // Stop folders (Stop List): the stops indented under this master (same division).
    std::vector<Stop*> groupChildren(const Stop* master) const;
    // ... and the transmissions indented under it (received in the master's division).
    std::vector<std::pair<Stop*, int>> groupChildTransmissions(const Stop* master) const;
    // The grid cell (column, row) an item of a division was placed in, if any.
    bool gridCellOf(int manual, const juce::String& key, int& column, int& row) const;
    // The Stop List applied a new order for a division: every item (stops and
    // transmissions, by grid key) is laid out afresh in this order, row by row.
    void setArrangeOrder(int manual, std::vector<juce::String> keys);
    // The grid cells of every transmission of a stop are forgotten (their keys
    // changed with the receiving division); the next layout places them again.
    void forgetTransmissionCells(const Stop* stop);
    // Before a stop is removed from the engine: nothing of the voicer may still
    // point at it (transmission cells, the scroll target, the drag state).
    void forgetStop(Stop* stop);
    void refreshUndulationLabels();         // after the global celeste beat ratio changed
    void updateStopStates(); // sync UI after external changes (e.g., memory recall)
    void setNoteActive(int note, bool active);   // forward to all rows
    void pulseManualActivity(int manual);        // thread-safe (called from the MIDI thread)
    void setActiveManualCount(int count) { activeManualCount = juce::jlimit(2, SamplerEngine::maxManuals, count); }
    double getManualActivityUntil(int manual) const noexcept
    {
        return manual >= 0 && manual < SamplerEngine::maxManuals ? manualActivityUntil[(size_t)manual].load(std::memory_order_relaxed) : 0.0;
    }
    // Coupler order changed by dragging in a division header (manual, coupler ids in new order).
    void setCouplerOrderSetter(std::function<void(int, const std::vector<juce::String>&)> setter)
    {
        couplerOrderSetter = std::move(setter);
    }
    bool isArrangeModeActive() const noexcept { return arrangeMode; }
    void exitArrangeMode();
    // Before the engine retires stops (a division reload): nothing here may point
    // at them afterwards (the drag and dwell state, the scroll target); the rows
    // are rebuilt by the caller right after.
    void dropStopReferences();
    // Arrange mode: a click on a free grid cell may also place a ready-made stop
    // that the loaded folder trees hold but the division does not: the owner
    // lists their names and adds the chosen one (its folders of every perspective).
    std::function<std::vector<juce::String>(int manual)> readyStopNames;
    std::function<void(int manual, int column, int row, const juce::String& name)> newStopRequest;
    // Arrange mode: a stop dropped into another division's grid. The owner moves
    // its table row (children included) and reloads both divisions; the grid cell
    // and the voicing state are already retargeted here.
    std::function<void(Stop*, int targetManual)> moveStopRequest;
    // Arrange mode: the "x" of a stop. The owner removes its Stop List row (after a
    // warning when it has transmissions) and reloads the division.
    std::function<void(Stop*)> removeStopRequest;
    // Arrange mode: a stop dropped on the open voicing of a stop folder is indented
    // under it. The owner moves its Stop List row there and reloads.
    std::function<void(Stop*, Stop* master)> indentStopRequest;
    // Arrange mode: a stop folder's child dropped on a grid cell leaves the folder
    // (the owner un-indents its Stop List row and reloads); its cell is set already.
    std::function<void(Stop*)> outdentStopRequest;
    // Arrange mode: a click on a free grid cell asked for a new empty stop (a stop
    // folder) there. The owner adds its Stop List row and reloads; the cell is set.
    std::function<void(int manual, int column, int row)> newStopFolderRequest;
    std::function<void(Stop*, int)> transpositionRequest;   // a stop's Transposition chosen in its voicing (0.8.158)
    std::function<void(Stop*, bool)> oneShotRequest;        // a stop's One-shot switch (0.8.158)
    // Arrange mode: a stop folder dropped under a stop (after the question was
    // answered "Merge"): the owner moves its children under that stop, the folder
    // itself too when it has pipes, otherwise the empty folder row is removed.
    std::function<void(Stop* folder, Stop* target)> mergeStopFolderRequest;
    // The question asked before a stop folder dissolves into another stop (grid and Stop List alike).
    static juce::String mergeFolderText(const juce::String& folderName, const juce::String& folderAlias, bool folderHasPipes,
        const juce::StringArray& children, const juce::String& targetName);
    // Arrange mode: the grid cell an item of a division will take at the next layout.
    void setGridCell(int manual, const juce::String& key, int column, int row) { stopCells[manual][key] = { column, row }; }
    // The next loadState reads the saved state of (name, from) as belonging to
    // division 'to' (a stop moved between divisions keeps its voicing).
    void retargetSavedStop(const juce::String& name, int from, int to) { stateRetargets.push_back({ name, from, to }); }
    // Is the screen position over a division column with an open voicing? (The
    // mouse wheel then belongs to the sliders, elsewhere it scrolls the workspace.)
    bool wheelBlockedAt(juce::Point<int> screenPosition) const;
    // Forgets the grid cells and pool of a division that no longer exists.
    void forgetDivision(int manual) { stopCells.erase(manual); hiddenStops.erase(manual); }
    // The stops of two divisions were exchanged (SamplerEngine::swapDivisions):
    // grid cells and pools follow, transmission keys carry the new home division.
    void swapDivisions(int a, int b);
    // Two program windows: divisions are laid out on the window ("screen") the
    // owner assigns them; without a second screen everything is on the first.
    void setSecondScreen(juce::Component* container, juce::Viewport* viewport)
    {
        screenContainers[1] = container;
        screenViewports[1] = viewport;
    }
    std::function<int(int)> screenGetter;   // manual -> 0 or 1
    // Compass (lowest, highest MIDI key) of a division: the per-key voicing
    // sliders of its stops cover exactly this range.
    void setCompassGetter(std::function<std::pair<int, int>(int)> getter) { compassGetter = std::move(getter); }
    std::pair<int, int> compassOf(int manual) const
    {
        if (compassGetter)
        {
            auto range = compassGetter(manual);
            range.first = juce::jlimit(0, 127, range.first);
            range.second = juce::jlimit(range.first, 127, range.second);
            return range;
        }
        return { noteMin, noteMax };
    }
    // Text describing the recorded temperament of a stop (matched by the owner
    // against its temperament tables); shown in the expanded voicing row.
    void setTemperamentDescriber(std::function<juce::String(const Stop*)> describer)
    {
        temperamentDescriber = std::move(describer);
    }
    // "Stop pool": stops taken off the grid of their division (their
    // transmissions in other divisions stay visible).
    bool isStopHidden(int manual, const juce::String& name) const;
    void setStopHidden(int manual, const juce::String& name, bool hidden);
    // Deletes transmission 'index' of 'stop' (same as lowering the count in the
    // stop's voicing); the pool of the target division offers this too. The UI
    // is rebuilt asynchronously (callers may sit inside a control of that UI).
    void removeTransmission(Stop* stop, int index);
    // Caption of a transmission button: its alias, or the stop name with the
    // footage of the transposition and the mark: "Principal 16' (Ext.)" for an 8' stop shifted
    // by -12 in its own division, "Principal 16' (Trsm.)" in another division; an alias gets the same mark.
    static juce::String transmissionLabel(const Stop* stop, int index);
    // JSON of a stop's transmissions ([{manual, shift, enabled}, ...]) and back;
    // older states with transmissionMask / transmissionEnabledMask are converted.
    static juce::var transmissionsToVar(const Stop* stop);
    static std::vector<Stop::Transmission> transmissionsFromVar(const juce::var& stopObj, int primaryManual);

private:
    std::map<int, std::set<juce::String>> hiddenStops;
    // Stop arrangement: per division a grid cell (column, row) for every stop the
    // user has placed. Unplaced stops fill the free cells column by column.
    std::map<int, std::map<juce::String, std::pair<int, int>>> stopCells;
    bool arrangeMode = false;
    int arrangeManual = -1;
    int couplerArrangeManual = -1;   // division whose coupler buttons are being arranged (survives rebuilds)
    float arrangeDashPhase = 0.0f;
    struct CompactCell
    {
        juce::String key;                // grid item key (stop name or transmission key); empty = free cell
        int manual = 0, column = 0, row = 0;
        juce::Rectangle<int> bounds;     // container coordinates
    };
    std::vector<CompactCell> compactCells;           // every grid cell of the last layout
    struct GridGeometry { int x = 0, drawWidth = 1, cellHeight = 1, columns = 1, top = 0, bottom = 0, rows = 0; bool valid = false; };
    std::array<GridGeometry, SamplerEngine::maxManuals> gridGeometry{};   // per division, container coordinates
    juce::OwnedArray<juce::Component> arrangeOverlays;   // faint cell outlines while arranging
    juce::Component::SafePointer<juce::Component> uiContainer;
    juce::Viewport* uiViewport = nullptr;
    std::unique_ptr<juce::Timer> arrangeAnimator;
    std::unique_ptr<juce::Timer> progressAnimator;   // 5 Hz: the copy-build strips on the stop buttons
    std::unique_ptr<juce::MouseListener> arrangeExitListener;
    std::function<void(int, const std::vector<juce::String>&)> couplerOrderSetter;
    std::function<juce::String(const Stop*)> temperamentDescriber;
    std::function<std::pair<int, int>(int)> compassGetter;
    std::array<juce::Rectangle<int>, SamplerEngine::maxManuals> columnBounds{};   // division columns of the last layout (container coordinates)
    std::array<bool, SamplerEngine::maxManuals> columnHasVoicing{};                 // ... and whether a voicing is open in them
    std::array<int, SamplerEngine::maxManuals> columnScreen{};                       // ... and the screen (container) they are on
    std::array<juce::Component*, 2> screenContainers{ nullptr, nullptr };
    std::array<juce::Viewport*, 2> screenViewports{ nullptr, nullptr };
    int screenOf(int manual) const
    {
        return screenContainers[1] != nullptr && screenGetter && screenGetter(manual) == 1 ? 1 : 0;
    }
    juce::Component* containerOf(int manual) const { return screenContainers[(size_t)screenOf(manual)]; }
    void layoutScreen(int screen, juce::Component& container, juce::Viewport& viewport);
    std::vector<Stop*> orderedDivisionStops(int manual) const;
    void enterArrangeMode(int manual);
    void dropItem(int manual, const juce::String& key, juce::Component* row, juce::Point<int> screenPosition, bool duplicate);
    void removeItem(int manual, const juce::String& key);
    // The open stop folder under a screen point that could take the dragged item as
    // a child (nullptr when there is none); the indicator follows the mouse.
    StopRow* indentTargetAt(int manual, const juce::String& key, juce::Point<int> screenPosition) const;
    void updateIndentIndicator(int manual, const juce::String& key, juce::Point<int> screenPosition);
    std::unique_ptr<juce::Component> indentMarker;   // the orange bar under a stop folder's button while a drop would indent
    // Indenting under any stop (it becomes a stop folder): the dragged item rests
    // over the stop's button for a moment - a dim bar appears under the button -
    // then is pulled a little down onto the bar, which lights up: the drop indents.
    // Pulled back up, the bar dims again and the drop is an ordinary move/swap.
    struct Dwell
    {
        juce::Component::SafePointer<StopRow> row;   // the stop rested over
        double since = 0.0;                          // when the rest began
        juce::Point<int> anchor;                     // where the item was when the bar appeared
        bool shown = false, armed = false;
    } dwell;
    struct Hover { bool active = false; int manual = 0; juce::String key; juce::Point<int> position; } lastHover;   // re-evaluated by the animator while the mouse rests
    bool hoverCopy = false;   // the current drag makes a copy: a folder's own child may be copied into the folder again
    StopRow* dwellCandidateAt(int manual, const juce::String& key, juce::Point<int> screenPosition) const;
    void resetDwell();
    // Arrange mode: a click on a free cell - a menu offers a new stop folder there and the division's pooled items.
    void freeCellClicked(int manual, int column, int row, juce::Point<int> screenPosition);
    // The stop (and transmission index, -1 for the stop itself) a grid key of a division stands for.
    bool resolveItem(int manual, const juce::String& key, Stop*& stop, int& transmissionIndex) const;
    struct StateRetarget { juce::String name; int from, to; };
    std::vector<StateRetarget> stateRetargets;
    // The voicing of stops the state names but the engine does not hold right
    // now (folders not found on this machine, a tree not loaded yet): kept
    // verbatim and written back by createState, so that a load that could not
    // build every stop never loses what the user voiced - including the
    // transmissions grouped under a stop folder, which live in these entries.
    std::vector<juce::var> retainedStopStates;
    std::map<int, std::vector<juce::String>> arrangeOrders;   // per division: the order the next layout places every item in

public:
    // Grid key of a transmission of 'stop' in its receiving division: the first
    // one into a division keeps the old key, further ones carry "#ordinal".
    static juce::String transmissionKey(const Stop* stop, int index);
    SamplerEngine& engine;
    juce::OwnedArray<StopRow> stopRows;   // stops and transmissions alike
    juce::OwnedArray<juce::Component> sectionDividers;
    int noteMin = 128, noteMax = 0;
    int sliderWidth = 30;               // per-key sliders sit close together; "-9.0" still fits the value box
    // Folding of the per-key rows in every expanded voicing (remembered by the owner).
    int stopRowTopHeight = 50;
    int stopRowBottomHeight = 180;      // sliders only (labels are on top of them)
    int margin = 12;
    std::function<void(const juce::String&)> midiLearnCallback;
    std::function<void(int)> folderCallback;
    std::function<void(int)> unloadCallback;
    std::function<void(int)> settingsCallback;
    std::function<void(int)> keyboardLearn;
    std::function<float(int)> swellGetter;
    std::function<void(int, float)> swellSetter;
    std::function<void(int)> swellLearn;
    std::function<bool(int)> swellLearning;
    std::function<void()> stateChanged;
    std::function<std::vector<DivisionCouplerInfo>(int)> couplerGetter;
    std::function<void(const juce::String&, bool)> couplerSetter;
    std::function<bool(int)> tremulantGetter;
    std::function<void(int, bool)> tremulantSetter;
    std::function<float(int)> columnWeightGetter;
    std::function<void(int, float, int, float)> columnWeightSetter;
    std::function<void(int)> stopBuildCallback;   // "Stops..." in the division header: the Stop building table
public:
    // The division header's notice of sample folder trees that are not on this
    // computer ("" = none): read when the columns are built, so the main
    // component rebuilds the columns when the set of missing trees changes.
    std::function<juce::String(int)> missingTreesGetter;
    std::function<juce::String(int)> missingTreesDetail;   // the tooltip: the paths and what to do
    std::function<void(int)> missingTreesAction;           // a click on the notice: replace a tree / open the Stop List
    std::function<void(int)> divisionMenu;                 // right-click on a division header's background: its feature settings
private:
    Stop* scrollTarget = nullptr;
    int activeManualCount = 2;
    juce::OwnedArray<juce::Component> sectionHeaders;
    std::array<std::atomic<double>, SamplerEngine::maxManuals> manualActivityUntil{};

    void computeNoteRange();
    void updateUI();        // rebuild all rows
};
