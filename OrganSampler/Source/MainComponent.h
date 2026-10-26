#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#include "voicer.h"
#include "HauptwerkOdf.h"
#include "UIControls.h"
#include <array>
#include <atomic>
#include <set>
#include <climits>
#include <string>
#include <unordered_map>

struct MemorySlot
{
    double masterDb = 0.0;
    juce::Array<juce::var> stopData; // will contain name, enabled, db, gainDb, attackMs, attackFadeoutMs
    juce::Array<juce::var> couplerData;
    juce::Array<juce::var> tremulantData; // { manual, enabled } per division
};

// A step of "Compiling the Stop List" in the loading dialog: its text, the
// percentage shown while it runs, and the work (one message-loop turn each).
struct LoadStep
{
    juce::String text;
    double progress = 0.0;
    std::function<void()> work;
};

class MainComponent : public juce::AudioAppComponent,
    private juce::MidiInputCallback,
    private juce::MidiKeyboardStateListener,
    private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    // Called when the user closes the application: asks whether changed settings
    // should be saved into the current configuration, then calls proceed().
    void requestQuit(std::function<void()> proceed);
    // Before quitting, loading another configuration or shutting down: unsaved
    // changes are offered for saving (into the open configuration file, or into
    // a new file when none is open); proceed() runs afterwards, never silently over them.
    void askToKeepChanges(const juce::String& title, const juce::String& continueVerb, std::function<void()> proceed);
    // Places the main window where it was last time (or maximised / centred); called by MainWindow.
    void placeMainWindow(juce::ResizableWindow& window);
   #if JUCE_LINUX
    void showMainWindow(juce::ResizableWindow& window);   // right after setVisible(true): the first paint at once (0.8.170)
   #endif
    void setUiScalePercent(int percent);   // the interface size (UIScale.h): applied to every open window, saved

private:
    class DivisionAudioCallback;
    SamplerEngine engine;
    std::unique_ptr<Voicer> voicer;

    juce::TextButton selectFolderButton{ "Select Top Folder Containing Stops" };
    // Top-bar buttons answer the left button only: the right button is MIDI learn.
    LeftOnlyTextButton panicButton{ "PANIC" };
    juce::TextButton keyboardsButton{ "Keyboards" };
    juce::Label manualCountLabel{ {}, "Divisions:" };
    juce::ComboBox manualCountBox;
    LeftOnlyTextButton toggleAllVoicingButton{ "Hide all voicing" };
    bool allVoicingVisible = false;
    LeftOnlyTextButton resetButton{ "GLOBAL RESET" };
    LeftOnlyTextButton enableAllButton{ "Enable All Stops" };
    LeftOnlyTextButton disableAllButton{ "Disable All Stops" };
    LeftOnlyTextButton saveStopSettingsButton{ "Save Voicing" };
    LeftOnlyTextButton loadStopSettingsButton{ "Load Voicing" };
    LeftOnlyTextButton mergeStopsButton{ "Stop-Merging" };
    LeftOnlyTextButton loadConfigurationButton{ "Load Configuration" };
    LeftOnlyTextButton saveConfigurationButton{ "Save Configuration" };
    // ========== PŘIDANÉ TLAČÍTKO ==========
    LeftOnlyTextButton oneShotPatternsButton{ "One-shot Patterns..." };
    // ========== KONEC PŘIDANÉ ČÁSTI ==========
    // ========== PŘIDANÉ TLAČÍTKO PRO IGNORED PATTERNS ==========
    LeftOnlyTextButton ignoredPatternsButton{ "Ignored Patterns..." };
    LeftOnlyTextButton perspectivePatternsButton{ "Perspective Patterns..." };
    LeftOnlyTextButton couplersButton{ "Couplers..." };
    LeftOnlyTextButton temperamentsButton{ "Temperaments" };
    LeftOnlyTextButton configurationEditorButton{ "Configuration Editor" };
    LeftOnlyTextButton stopListButton{ "Stop List" };   // the table of every division's stops, folders and transmissions
    LeftOnlyTextButton guidedTourButton{ "Guided tour" };   // shown on a teaching organ only (0.8.206)
    HintSheen hintSheen;   // the golden sheen over Stop List and Load Configuration until the first click (UIControls.h)
    GlowLayer glowLayer;   // 0.8.206: a pressed button and a dragged stop glow for a moment (setting "consoleGlow")
    bool consoleGlow = true;
    bool stopListHintShown = false;        // the Stop List's own sheen and arrow: the first opening of the session only (0.8.176)
    bool ramLogPending = false;            // "RAM held after the load" to be logged once the copies are built (0.8.184)
    bool loadConfigurationAgain = false;   // the test hook ORGAN_LOAD_CONFIGURATION_AGAIN is the file to take now (0.8.184)
    juce::File loadConfigurationAgainFile;
    double ramLogAfterMs = 0.0;
    int copyCacheHitsAtLoad = 0;
    void logRamAfterLoadIfDue();
    juce::StringArray volumeSetsAnnounced;   // multi-part archives whose other parts were announced (once per session, 0.8.176)
    LeftOnlyTextButton shutdownButton{ "Shut down PC" };   // saves, quits and shuts the computer down (hidden by default)
    LeftOnlyTextButton unpackArchiveButton{ "Unpack archive" };   // RAR sample archives unpacked by libarchive (hidden by default)
    void unpackArchiveDialog();
    void startUnpack(const juce::Array<juce::File>& archives, const juce::File& destination,
        std::function<void(const std::vector<juce::File>& topFolders, bool ok)> afterwards = {});   // the work (one archive after another) with its progress window, then the report (or 'afterwards')
    // Load folder... of the Stop List (0.8.157): a folder tree, or RAR archives read as
    // the folders they hold (a Hauptwerk definition inside is offered for import), or
    // the archives unpacked first on request.
    void loadChosenIntoDivision(const std::vector<juce::File>& folders, const std::vector<juce::File>& archives, bool unpackFirst, const juce::File& destination, int manual);
    // The archives' indexes built on a thread with a progress window, then 'then' on the message thread.
    void indexArchivesThen(const std::vector<juce::File>& archives, std::function<void()> then);
    void loadIndexedIntoDivision(const std::vector<juce::File>& folders, const std::vector<juce::File>& archives, int manual);
    void loadTreesIntoDivision(const std::vector<juce::File>& trees, int manual);
    bool addTreeToDivisionTable(const juce::File& tree, int manual);   // the tree into the pool and the division's table; false = nothing to reload
    juce::Component::SafePointer<juce::DialogWindow> unpackWindow;   // the Unpack archive window (not modal: a file chooser beside it must keep the focus)
    std::unique_ptr<juce::FileChooser> archiveChooser;
    // Every button of the grid (stops, transmissions, extensions, stop folders
    // with a sound) in grid order - what "a stop" means when stops are counted or
    // switched (never only the stops with samples of their own). From the Stop
    // List when it exists, from the engine otherwise.
    std::vector<Voicer::GridItem> gridStopItems() const;
    void setGridItemEnabled(const Voicer::GridItem& item, bool enabled);   // a stop folder with its children
    // ========== KONEC PŘIDANÉ ČÁSTI ==========
    // Which buttons the top bar shows (Configuration Editor > Console). Missing
    // keys use the defaults: everything on except Stop-Merging, Load Voicing and
    // Save Voicing.
    std::map<juce::String, bool> headerButtonShown;
    bool isHeaderButtonShown(const juce::String& key) const;
    struct HeaderButtonInfo { juce::Button* button; int weight; const char* key; const char* label; bool defaultShown; };
    std::vector<HeaderButtonInfo> headerButtons();

    static constexpr double kDefaultMasterDb = -18.0;   // the master level a fresh start, a reset and a configuration without one get (0.8.128; -20 before)
    LearnableSlider masterVolumeSlider;
    juce::Label masterLabel{ "Master: -20.0 dB" };
    LearnableSlider pitchSlider;
    juce::Label pitchLabel{ "Pitch: 0 cents" };
    std::unique_ptr<juce::Component> polyphonyDisplay;

    LearnableSlider releaseFadeOutSlider;
    juce::Label releaseFadeOutLabel{ "Sustain fade-out (ms)" };
    LearnableSlider releaseFadeInSlider;
    juce::Label releaseFadeInLabel{ "Release fade-in (ms)" };
    LearnableSlider releaseScalingSlider;
    juce::Label releaseScalingLabel{ "Release scaling (dB)" };
    LearnableSlider attackMinimumSlider;
    juce::Label attackMinimumLabel{ "Attack min. time (ms)" };
    LearnableSlider attackMinimumDelaySlider;
    juce::Label attackMinimumDelayLabel{ "Attack min. delay (ms)" };
    LearnableSlider releaseCutSlider;          // release cutting (% of the release sample length)
    juce::Label releaseCutLabel{ "Release cut: 100 %" };

    juce::Component stopsContent;
    NoWheelViewport stopsViewport;
    // A picture of the grid as it was, shown dimmed over the workspace while a
    // load (a merge, a reload) replaces the instrument (0.8.154: the grid went
    // blank for the minutes a second organ took to load).
    std::unique_ptr<juce::ImageComponent> frozenGrid;
    // 0.8.206: the picture is composed per division - the old stops of a division under
    // the NEW header and coupler rows (0.8.204's picture showed them through each other).
    juce::Image frozenPicture;
    std::vector<std::pair<int, juce::Rectangle<int>>> frozenAreas;   // the grid areas of the picture (container coordinates)
    juce::Point<int> frozenViewPosition;
    // The guided tour of a teaching organ (0.8.206, GuidedTour.h): offered when the
    // configuration carries "guidedTour": true (or is the Rural Organ demo).
    std::unique_ptr<class GuidedTour> guidedTour;
    bool guidedTourAvailable = false;
    juce::StringArray guidedTourDismissed;   // configuration names whose tour was closed with "Don't show again" (machine-specific)
    void startGuidedTour();
    // ORGAN_VIDEO_SCRIPT (0.8.206): an instruction video's frames and sound, driven by a script file.
    struct VideoScript;
    std::shared_ptr<VideoScript> video;   // shared_ptr: the struct is defined late in the cpp, after the destructor
    void startVideoScript(const juce::File& script, const juce::File& folder);
    void freezeGrid();
    void thawGrid();
    // The empty instrument (no folder tree, no stop): a hint in the stop
    // workspace with a button that opens the user manual; gone once anything loads.
    // The hint sits on a panel of the window's own background colour, so that
    // the dividers between the division columns do not run through its text.
    struct HintPanel final : public juce::Component
    {
        void paint(juce::Graphics& g) override { g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId)); }
    };
    HintPanel emptyHintPanel;
    juce::Label emptyHintLabel;
    juce::TextButton emptyHintButton;
    void updateEmptyHint();
    // Sample folder trees of a division that are not on this computer (a
    // configuration from elsewhere): the notice under the division header
    // ("" = none) and its tooltip; the columns are rebuilt when the set changes.
    juce::String missingTreeNotice(int manual, bool detail) const;
    void updateMissingTreeNotices();
    juce::String lastMissingTreeSummary;
    double missingTreeCheckAt = 0.0;
    // Shows the tooltips of every control in the application (there was no
    // TooltipWindow before, so none of the setTooltip texts ever appeared).
    juce::TooltipWindow tooltipWindow{ nullptr, 600 };

    LeftOnlyToggleButton combinationSetButton{ "Set" };
    LeftOnlyTextButton previousCombinationButton{ "<<" };
    LeftOnlyTextButton nextCombinationButton{ ">>" };
    LeftOnlyTextButton bankDownButton{ juce::String(juce::CharPointer_UTF8("Bank \xe2\x96\xbc")) };   // Bank ▼
    LeftOnlyTextButton bankUpButton{ juce::String(juce::CharPointer_UTF8("Bank \xe2\x96\xb2")) };     // Bank ▲
    juce::Label bankLabel;
    DisplayLabel combinationDisplay;   // same look as the swell bar and the polyphony display
    juce::Label pistonCountLabel{ {}, "Pistons:" };
    juce::ComboBox pistonCountBox;
    juce::OwnedArray<juce::TextButton> pistonButtons;
    LeftOnlyTextButton saveCombinationsButton{ "Save" };
    LeftOnlyTextButton loadCombinationsButton{ "Load" };
    juce::Label couplerRailLabel{ {}, "Couplers:" };
    juce::Viewport couplerViewport;
    juce::Component couplerContent;
    juce::OwnedArray<juce::TextButton> couplerButtons;

    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSettings;
    // Pool of ASIO hosts: one AudioDeviceManager per open ASIO driver, shared by
    // every route (division or perspective) that plays through that driver. A
    // driver may be opened only once per process, but different drivers (several
    // audio interfaces) run side by side, each with its own host.
    // Since 0.8.47 the pool holds Windows Audio (WASAPI shared mode) hosts as
    // well: every route of one Windows Audio device plays through ONE stream and
    // one callback thread, rendered by the whole worker pool - as on ASIO. Before,
    // every route opened its own stream with its own thread: a division on one
    // route was bound to one core, and ten device threads starved against the
    // workers. Aggregated WASAPI and "none" keep private managers.
    std::vector<std::unique_ptr<juce::AudioDeviceManager>> asioHosts;
    std::array<juce::AudioDeviceManager*, SamplerEngine::maxManuals> manualAsioHost{};   // default perspective of each division: its host, or nullptr
    static bool isSharedHostType(const juce::String& typeName)
    {
        return typeName.equalsIgnoreCase("ASIO") || typeName.equalsIgnoreCase("Windows Audio")
            || typeName.equalsIgnoreCase("CoreAudio") || typeName.equalsIgnoreCase("ALSA") || typeName.equalsIgnoreCase("JACK");
    }
    // The system's own driver type: the default output of this type is where a
    // route without a saved device plays - and every route of a portable
    // configuration (its states say deviceType="default", whatever the platform).
    static juce::String defaultSharedTypeName()
    {
       #if JUCE_MAC
        return "CoreAudio";
       #elif JUCE_LINUX
        return "ALSA";
       #else
        return "Windows Audio";
       #endif
    }
    juce::AudioDeviceManager* findSharedHost(const juce::String& typeName, const juce::String& deviceName) const;
    juce::AudioDeviceManager* sharedHostFor(const juce::String& typeName, const juce::String& deviceName, const juce::XmlElement* state = nullptr);
    juce::String defaultDeviceNameOfType(const juce::String& typeName);   // the system's default output of a type ("" when none)
    juce::AudioDeviceManager* findAsioHost(const juce::String& deviceName) const { return findSharedHost("ASIO", deviceName); }
    juce::AudioDeviceManager* asioHostFor(const juce::String& deviceName, const juce::XmlElement* state = nullptr) { return sharedHostFor("ASIO", deviceName, state); }
    bool isAsioHost(const juce::AudioDeviceManager* manager) const;   // "is a shared host" (any type in the pool)
    int routesOnManager(const juce::AudioDeviceManager* manager) const;
    void pruneUnusedAsioHosts();
    // Choosing another device in a tab of a shared host moves only that route to the device's host.
    void installAsioDeviceInterceptor(juce::AudioDeviceSelectorComponent& selector, int manualIndex,
        const juce::String& perspective);
    void moveRouteToSharedDevice(int manualIndex, const juce::String& perspective, const juce::String& typeName, const juce::String& deviceName);
    void moveRouteToAsioDevice(int manualIndex, const juce::String& perspective, const juce::String& deviceName)
    {
        moveRouteToSharedDevice(manualIndex, perspective, "ASIO", deviceName);
    }
    // The division's own route joins a shared host (type, device, saved state) or
    // keeps its private manager without a device (none).
    void attachDivisionRoute(int manualIndex, const juce::String& typeName, const juce::String& deviceName, const juce::XmlElement* state);
    // Aggregated WASAPI: the route's configured channels become the device's
    // open endpoints - called whenever such a device starts for a route (so every
    // path applies them) and when the user ticks channels. Endpoints that cannot
    // be opened are refused: their channels drop out of the configuration (none,
    // as the Windows Audio type refuses such a device) and the log names them.
    // Returns true when the configured channels had to change.
    bool applyAggregateSelection(int manualIndex, const juce::String& perspective, juce::AudioIODevice* device);
    // "<< none >>" chosen for a route: only this route lets go of its device (a
    // shared ASIO host keeps playing the other routes). The quiet variant does the
    // work without muting, dialog hooks or saving (for "set for all").
    void detachRouteAudio(int manualIndex, const juce::String& perspective);
    void detachRouteAudioQuietly(int manualIndex, const juce::String& perspective);
    std::unique_ptr<DivisionAudioCallback> primaryAudioCallback;
    // Divisions 1.. (the pedal uses deviceManager): one entry per manual.
    std::array<std::unique_ptr<juce::AudioDeviceManager>, SamplerEngine::maxManuals - 1> extraAudioManagers;
    std::array<std::unique_ptr<juce::AudioDeviceSelectorComponent>, SamplerEngine::maxManuals - 1> extraAudioSettings;
    std::array<std::unique_ptr<DivisionAudioCallback>, SamplerEngine::maxManuals - 1> extraAudioCallbacks;
    struct PerspectiveAudioRoute
    {
        int manual = 0;
        juce::String perspective;
        std::unique_ptr<juce::AudioDeviceManager> manager;
        juce::AudioDeviceManager* managerInUse = nullptr;
        std::unique_ptr<juce::AudioDeviceSelectorComponent> selector;
        std::unique_ptr<DivisionAudioCallback> callback;
    };
    std::map<std::string, std::unique_ptr<PerspectiveAudioRoute>> perspectiveAudioRoutes;
    std::unique_ptr<juce::ThreadWithProgressWindow> activeMerger;
    std::unique_ptr<juce::ThreadWithProgressWindow> activeFolderLoader;

    std::array<juce::MidiKeyboardState, SamplerEngine::maxManuals> manualKeyboardStates;

    // ========== PŘIDANÉ: ZAMEZENÍ DVOJÍHO SPUŠTĚNÍ TÓNU ==========
    // processNextMidiEvent() v handleIncomingMidiMessage() vyvolá listener
    // (handleNoteOn/handleNoteOff), který by tón spustil podruhé.
    bool suppressKeyboardStateNotes = false;
    // ========== KONEC PŘIDANÉ ČÁSTI ==========

    struct ManualConfig
    {
        struct PerspectiveAudioConfig
        {
            std::vector<int> outputChannels{ 0, 1 };
            juce::String audioDeviceStateXml;
            float volumeDb = 0.0f;
            float channelSeparation = 1.0f;   // 1 = pipes only on their own channel pair, 0 = the same sum everywhere
            juce::String reverbIrPath;     // impulse response WAV of this perspective's IR reverb (empty = none)
            float reverbWet = 0.3f;        // dry/wet ratio 0..1
            SamplerEngine::ReverbTone reverbTone;   // high/low cut, pre-delay, wet level, decay
        };
        juce::String deviceIdentifier;
        int midiChannel = 1;
        bool disableVelocity = true;
        int bassOctaveLayout = 0; // 0 standard, 1 short octave, 2 CDD#, 3 CDE
        int midiTranspose = 0;    // the MIDI keyboard plays this many semitones away (-24..24)
        juce::String expressionDeviceIdentifier;
        int expressionChannel = 1;
        int expressionController = -1;
        int expressionMinimum = 127;
        int expressionMaximum = 0;
        float expressionPosition = 1.0f;
        float expressionMaxAttenuationDb = 13.0f;
        float expressionInertiaMs = 300.0f;
        juce::String folderPath;                     // first folder tree of the division
        std::vector<juce::String> extraFolderPaths;  // further trees loaded into the same division
        std::vector<int> outputChannels{ 0, 1 };
        float divisionVolumeDb = 0.0f;
        bool allowRandomTuningError = true;
        bool allowPipeCoupling = true;
        bool tremulantEnabled = false;
        float tremulantRateHz = 4.4f;
        float tremulantDepthCents = 6.0f;
        float tremulantRankDriftDegrees = 260.0f;
        float windDepthCents = 5.0f;
        float windFrequencyHz = 7.5f;
        float windOscillations = 7.0f;
        float windOscillationStrength = 23.3f;
        float windSmoothingMs = 170.0f;
        uint8_t windTargetMask = 0;
        // Compass of the division (MIDI keys): the per-key voicing sliders cover it.
        // Pedal defaults to 36-67 (set in the constructor), manuals to 36-96.
        int compassLow = 36, compassHigh = 96;
        juce::String audioDeviceStateXml;
        std::map<std::string, PerspectiveAudioConfig> perspectiveAudio;
        // Stop building (the table of the Stop building dialog): which sample
        // folder of which tree forms which perspective of which stop. While
        // inactive the division is built by the automatic rules (folder names).
        struct StopBuild
        {
            struct FolderRef
            {
                int tree = 0; juce::String relativePath;   // tree 0 = folderPath, 1.. = extraFolderPaths
                juce::StringArray morePaths;               // further folders of the same perspective in the same tree: the L and R halves of a stereo stop (0.8.160)
            };
            struct Entry
            {
                juce::String name, alias;
                bool customName = false;                      // typed by the user, not derived from the folders
                bool child = false;                           // indented under the previous non-child row (stop folder)
                int sampleOffset = 0;                         // the stop's transposition: the files' numbers minus the keys they land on (0.8.155; Stop List "Transp.")
                bool sampleOffsetSet = false;                 // known ("sampleOffset" in the file: a definition, the user, or found before); unset: the loader finds it from the smpl pitches (0.8.158)
                std::map<juce::String, FolderRef> folders;    // perspective -> folder
                // A transmission shown as a row of its receiving division (Stop List
                // "Show Trsm."): never written into the table, applied to the engine.
                juce::String trSource;                        // the source stop's name (empty = a stop row)
                int trSourceManual = -1;                      // ... and its division
                int trOriginalManual = -1, trOrdinal = 0;     // its identity when the dialog opened (division, ordinal there)
                int trShift = 0;
                juce::String trMaster;                        // the stop folder it was indented under when the dialog opened
                bool isTransmission() const noexcept { return trSource.isNotEmpty(); }
            };
            bool active = false;
            std::vector<juce::String> perspectives;
            std::vector<Entry> entries;
            std::vector<juce::String> legacyTrees;   // the division's tree list as read from the settings (migration to the pool)
        };
        StopBuild stopBuild;
    };
    std::array<ManualConfig, SamplerEngine::maxManuals> manualConfigs;
    // The tree pool: every loaded sample folder tree, once, for the whole organ.
    // FolderRef.tree indexes this list; the per-division folderPath/extraFolderPaths
    // are derived from the tables (syncDivisionTreeLists) for the older code paths.
    std::vector<juce::String> sampleTrees;
    int poolIndexOf(const juce::String& path) const;
    int addTreeToPool(const juce::String& path);           // returns its index
    void syncDivisionTreeLists();
    void dropNestedTrees();   // a tree inside another loaded tree goes: its rows point into the outer one (0.8.143)
    void loadTreeIntoDivision(const juce::File& tree, int manual);   // a new tree: its stops join the division's table
    // The pool tree at that index is replaced by another folder, wherever it is
    // and whatever its name: the table rows keep their folders (relative to the
    // tree), every division that uses it is rebuilt. "Replace ... with another
    // folder" of the Stop List and of the missing-tree notice.
    void relocateTree(int poolIndex, const juce::File& newFolder);
    // 0.8.202: the tree found from ANY folder of the set the user chose (the tree, a folder above
    // it, a neighbour), its content checked against the Stop List rows, the other missing trees
    // looked for around it, all loaded at once, the place remembered as a sample location.
    void relocateTreeSmart(int poolIndex, const juce::File& chosen);
    bool applyTreeRelocation(int poolIndex, const juce::File& newFolder);   // the path replaced in the pool and the divisions, no reload
    juce::StringArray treeRowFolders(int poolIndex) const;                  // the folders the Stop List rows expect inside the tree (relative)
    int treeFoldersFound(int poolIndex, const juce::File& folder, juce::StringArray* missing = nullptr) const;   // how many of them lie under the folder; -1 = no row names one
    void chooseFolderForMissingTree(int poolIndex);                          // the folder chooser of the notice's menu
    void showMissingTreesMenu(int manual);   // the notice under a division header was clicked
    // Linux: a .desktop entry and the icon in every size under ~/.local/share, so
    // the program has its icon in the menu, the dock and the file manager.
    void installLinuxDesktopEntry();
    void unloadTreeFromPool(const juce::String& path);     // the tree and every stop built from it, in every division
    void unloadAllTreesFromPool();                           // every tree: the rows stay as empty stops, every division reloads (empty)
    std::shared_ptr<std::vector<SamplerEngine::FolderAssignment>> stopMapForDivisions(const std::set<int>& divisions) const;
    // Stop building helpers (see ManualConfig::StopBuild).
    std::vector<juce::String> divisionTrees(int manual) const;   // the pool trees the division's table uses (or its legacy lists)
    juce::File resolveStopBuildFolder(int manual, const ManualConfig::StopBuild::FolderRef& ref) const;
    // Every stop without an alias gets the bare name of its sample folder as its
    // alias (no parent folders, no perspective prefix); applied to the tables,
    // the loaded stops and the grid at once. Returns how many were filled.
    int fillAliasesFromFolders();
    // "Report a problem...": the dialog that bundles the logs, settings, a
    // description of the computer and the audio devices and screenshots into a
    // folder on the Desktop and opens the GitHub bug report with the fields filled in.
    void showProblemReportDialog();
    void showLongMessage(const juce::String& title, const juce::String& body);   // a message that may not fit an alert box: read-only text with scrollbars and OK
    juce::File writeProblemReport(const juce::String& what, const juce::String& steps, bool windowShots,
        const juce::Array<juce::File>& extraShots, juce::URL& issueUrl);
    // After the report: the message with "Open the GitHub form" (again) and "Open the report folder".
    static void showAfterReport(const juce::URL& url, const juce::File& folder, const juce::String& title = "Report a problem",
        const juce::String& message = {}, const juce::String& formButton = "Open the GitHub form", const juce::String& folderButton = "Open the report folder");
    // After a portable save (0.8.194): the same window as after a report - "Publish on GitHub"
    // opens the issue form prefilled and the Publish folder with the zip and the README beside it.
    void showPublishWindow(const juce::File& folder, const juce::File& zip, const juce::File& readme, bool launch);
    static void arrangeReportWindows(const juce::File& folder);   // the browser on the left half of the screen, the file manager with the report folder on the right (0.8.139)
    static const juce::Displays::Display* displayOfMainWindow();   // the screen the main window is on (0.8.146)
    std::shared_ptr<std::vector<SamplerEngine::FolderAssignment>> stopBuildMap(int manual) const;   // nullptr while inactive
    ManualConfig::StopBuild deriveStopBuildFromEngine(int manual) const;   // the table as the loaded stops are
    void extendStopBuildWithTree(int manual, const juce::File& tree, int treeIndex);   // a tree added to a built division
    void applyStopBuild(int manual, const ManualConfig::StopBuild& build, bool arrangeGrid = false);
    // The Stop List applies the tables of several divisions at once (one reload).
    void applyStopLists(const std::array<ManualConfig::StopBuild, SamplerEngine::maxManuals>& builds,
        const std::set<int>& changed, const std::set<int>& arrangeGrid);
    void resetStopBuild(int manual);
    // Arrange mode: a click on a free grid cell makes a new empty stop (a stop
    // folder) there; its Stop List row is added and the division reloads.
    void createStopFolderOnGrid(int manual, int column, int row);
    void setStopSampleOffset(Stop* stop, int offset);   // the stop's Transposition from the voicing (0.8.155 "Files vs keys", 0.8.158)
    void takeDetectedSampleOffsets();   // the offsets the loader found from the smpl pitches go into the Stop List rows (0.8.158)
    // RAR archives elsewhere (0.8.158): the folders whose archives serve the
    // configuration's paths - its own folder, the folders in its ArchiveFolders.txt,
    // OrganRarPackages beside it, and the program-wide list in the settings folder.
    void refreshArchiveSearchFolders(const juce::File& configurationFile);
    void registerArchive(const juce::File& archive);   // an archive the user chose: serves paths and is remembered for the next start (0.8.177)
    void loadConfigurationFromArchives(const std::vector<juce::File>& archives, bool merge);   // Load Configuration / Merge given .rar files: one configuration or definition inside them, chosen
    // A choice out of a list in a proper window (centred, modal, Cancel): 'chosen' gets the index, -1 for Cancel (0.8.160).
    void chooseFromList(const juce::String& title, const juce::String& message, const juce::StringArray& items, const juce::String& okText, std::function<void(int)> chosen);
    // A ready-made stop the loaded folder trees hold: its name and the folder of
    // every perspective, as a Stop List row would carry them.
    struct ReadyStop
    {
        juce::String name;
        std::map<juce::String, ManualConfig::StopBuild::FolderRef> folders;   // perspective -> folder
    };
    // The ready-made stops of the loaded trees that no division uses yet - none
    // of their folders is in any row of any division - so that a whole stop,
    // every perspective, can be added once. 'entries' are the rows of 'manual'
    // as they are now (a draft); the other divisions count as saved.
    std::vector<ReadyStop> readyStopsNotIn(int manual, const std::vector<ManualConfig::StopBuild::Entry>& entries);
    // The same with every division's rows given (the Stop List's drafts), by division.
    std::vector<ReadyStop> readyStopsNotIn(const std::vector<std::vector<ManualConfig::StopBuild::Entry>>& allEntries);
    // Adds one of them to a division (its row with the folders, the grid cell when
    // given) and loads the division again - only the new stop is decoded, the
    // others keep their samples and copies.
    void addReadyStopToDivision(int manual, const ReadyStop& stop, int column = -1, int row = -1);
    // Arrange mode, after the question: a stop folder dropped under a stop dissolves
    // into it - its children (and itself when it has pipes) become that stop's
    // children; an empty folder row is removed. The divisions reload.
    void mergeStopFolderInto(Stop* folder, Stop* target);
    // Arrange mode: a stop dragged into another division's grid moves there
    // (its table row, children included, and its voicing follow; both divisions reload).
    void moveStopToDivision(Stop* stop, int targetManual);
    // Arrange mode "x": the stop leaves the organ - a warning lists its
    // transmissions first; its Stop List row goes and the division reloads.
    void removeStopFromGrid(Stop* stop);
    // After a table edit done in place (indent, outdent, folder, move): groups,
    // fills of the named divisions' stops, settings, grid and voicing state -
    // nothing is reloaded, no copy built that exists.
    void finishInPlaceStopEdit(const std::set<int>& manuals);
    // The engine stop named like the table row of a division (nullptr if none).
    Stop* stopNamed(int manual, const juce::String& name) const;
    // Arrange mode: a stop dropped on an open stop folder becomes its child (its
    // Stop List row moves under the master, indented; the divisions reload).
    void indentStopUnder(Stop* stop, Stop* master);
    // Arrange mode: a child dropped on the grid leaves its stop folder (its row
    // follows the folder's block, no longer indented; the division reloads).
    void outdentStop(Stop* stop);
    // Stop List: what happens to one transmission on Apply (identity as when the
    // dialog opened; the new division, master and alias, or its removal).
    struct TransmissionEdit
    {
        int sourceManual = -1; juce::String source;   // the source stop
        int originalManual = -1, ordinal = 0;         // identity at the time the dialog opened
        int newManual = -1; juce::String master, alias;
        bool remove = false;
        juce::String newKey;                          // filled by applyTransmissionPlan: the grid key afterwards
    };
    void applyTransmissionPlan(std::vector<TransmissionEdit>& plan);
    bool stopListShowTransmissions = false;   // Stop List: transmissions listed as rows
    void applyStopGroups();   // Stop::groupMaster of every stop from the tables' indented rows
    void reloadDivisionTrees(int manual);   // unloads the division and loads its trees again (with the map when active)
    // onlyTree: load just that pool tree into the divisions (nothing is unloaded first).
    void reloadDivisions(const std::vector<int>& manuals, const juce::String& onlyTree = {});
    void finishDivisionLoad(int manual);    // routes, voicing, buttons after the division's stops changed
    void finishDivisionsLoad(const std::vector<int>& manuals);
    // Its three parts, run as the phases of the loading dialog ("Compiling the Stop List" with a percentage).
    void finishDivisionsLoadRoutes(const std::vector<int>& manuals);
    void finishDivisionsLoadRoutesBegin(const std::vector<int>& manuals);   // the part before the routes are opened
    void finishDivisionsLoadGrid(const std::vector<int>& manuals);
    void finishDivisionsLoadEnd(const std::vector<int>& manuals);
    // The audio routes still missing, one loading step each (opening a device
    // takes up to seconds): the ASIO drivers first, then the routes, then a
    // closing step; their percentages run from 'first' to 'last'.
    std::vector<LoadStep> audioRouteSteps(double first, double last, std::function<void()> afterwards);
    void showStopBuildingDialog(int manual);
    int reopenStopBuildManual = -1;         // the table was open when a load/unload/swap began: open it again after
    std::set<int> pendingGridArrange;       // divisions whose grid follows the table order after the reload
    static juce::var stopBuildToVar(const ManualConfig::StopBuild& build);
    static ManualConfig::StopBuild stopBuildFromVar(const juce::var& value);
    static juce::String stopBuildKey(const juce::String& name);   // comparable form of a stop name
    static juce::String uniqueStopBuildName(const std::vector<ManualConfig::StopBuild::Entry>& entries,
        const juce::String& wanted, const ManualConfig::StopBuild::Entry* except);
    std::unique_ptr<juce::DialogWindow> stopBuildWindow;
    int stopBuildWindowManual = -1;
    // Resets everything remembered about a division (its configuration, column
    // weight, coupler order, grid and pool): used when the console shrinks.
    void forgetDivision(int manual);
    // Exchanges the stops of two divisions (folders, voicing, grid, pool,
    // transmissions, combinations, crescendo, MIDI bindings follow).
    void swapDivisionStops(int a, int b);
    void offerCombinationFileConversion(int a, int b);   // saved combination files name divisions too
    std::unique_ptr<juce::FileChooser> swapFileChooser;
    // Two program windows: divisions choose the window they are shown in.
    bool twoWindows = false;
    std::array<int, SamplerEngine::maxManuals> divisionScreen{};   // 0 = main window, 1 = second window
    class SecondWindow;
    std::unique_ptr<SecondWindow> secondWindow;
    void setTwoWindows(bool on);
    void updateSecondWindow();
    std::array<float, SamplerEngine::maxManuals> divisionColumnWeights{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<std::atomic<float>, SamplerEngine::maxManuals> expressionTargetPositions;
    std::array<float, SamplerEngine::maxManuals> expressionInertiaStages{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    double lastExpressionInertiaUpdateMs = 0.0;
    int manualCount = 2; // number of manuals in addition to the always-present pedal
    std::vector<std::unique_ptr<juce::MidiInput>> midiInputs;
    std::vector<juce::String> midiInputIdentifiers;
    juce::MidiDeviceListConnection midiDeviceListConnection;   // MIDI devices plugged in or out while running
    void midiDevicesChanged();
    std::atomic<int> listeningManual{ -1 };
    std::atomic<int> listeningExpressionManual{ -1 };
    std::atomic<bool> applicationReady{ false };
    std::atomic<bool> audioMuted{ true };
    // The polyphony measurement renders everything as usual but hands the
    // devices the sound at -42 dB (the callbacks keep their real timing). The
    // callbacks glide to this target over about two seconds, so neither the
    // start nor the end (after the report's OK) jumps in level.
    std::atomic<float> outputGainTarget{ 1.0f };
    std::atomic<double> outputGainHoldUntilMs{ 0.0 };   // the level stays down until then after a measurement (0.8.147; 0 = no hold since 0.8.187), then glides up 26 dB a second
    // One clock per audio device: the routes of a device are called one after
    // another inside one driver callback, so the first route of each callback
    // marks when the driver really called. A driver callback later than half a
    // period beyond its due time is a late callback (a dropout on ASIO).
    struct DeviceClock
    {
        std::atomic<void*> device{ nullptr };
        std::atomic<bool> asio{ false };                  // ASIO: a callback 1.5 periods late is a dropout; Windows Audio buffers more (3 periods + 2 ms)
        std::atomic<int64_t> lastFirst{ 0 }, lastEnd{ 0 };
        std::atomic<int> late{ 0 };                       // late callbacks by the device type's rule
        std::atomic<int> over15{ 0 }, over2{ 0 };         // gaps beyond 1.5 and 2 periods (diagnostics)
        std::atomic<double> maxGapMs{ 0.0 };
        // The longest driver callback (first route in -> last route out, with the
        // note bookkeeping, every route's render, IRs and copies) as a share of
        // the period: the true load of the device thread.
        std::atomic<float> maxCallbackShare{ 0.0f };
        std::atomic<double> periodMs{ 0.0 };
    };
    static constexpr int maxDeviceClocks = 16;
    DeviceClock deviceClocks[maxDeviceClocks];
    DeviceClock* deviceClockFor(void* device, bool asio) noexcept;
    // Late driver callbacks since the last call: ASIO devices and the others separately, with the longest gaps.
    struct DeviceLateness
    {
        int asioLate = 0, asioOver15 = 0, asioOver2 = 0, otherLate = 0;
        double asioMaxGapMs = 0.0, otherMaxGapMs = 0.0;
        float asioMaxCallbackShare = 0.0f, otherMaxCallbackShare = 0.0f;   // the longest driver callback / its period
    };
    DeviceLateness takeDeviceLateCallbacks() noexcept;
    std::atomic<float> masterGainLinear{ 0.1f };
    // Snapshot of the MIDI routing configuration for the MIDI callback thread.
    // Rebuilt by refreshMidiRouting() whenever settings change, so the MIDI
    // thread never reads juce::Strings that the GUI thread may be rewriting.
    struct MidiRouting
    {
        juce::String device;
        int channel = 1;
        bool disableVelocity = true;
        int bassOctaveLayout = 0;
        int midiTranspose = 0;
        juce::String expressionDevice;
        int expressionChannel = 1;
        int expressionController = -1;
        int expressionMinimum = 127;
        int expressionMaximum = 0;
    };
    std::array<MidiRouting, SamplerEngine::maxManuals> midiRouting;
    int midiRoutingManualCount = 2;
    juce::CriticalSection midiRoutingLock;
    void refreshMidiRouting();
    static int mapNoteForLayout(int layout, int note) noexcept;
    // The division's MIDI transposition: -1 when the key lands outside the MIDI range.
    static int transposedNote(int note, int semitones) noexcept
    {
        const int result = note + semitones;
        return result < 0 || result > 127 ? -1 : result;
    }
    bool sampleStorageFloat = false;
    bool resampleOnLoad = true;   // samples of other rates are resampled to 48 kHz while loading
    double storageSampleRate = 48000.0;   // the rate the samples are stored at when resampled (48 000 / 36 000 / 32 000; 0.8.143)
    bool releaseHalfRate = false;         // release samples at half the storage rate (0.8.143)
    bool compressSamples = false;         // lossless packing of the 16-bit samples in RAM (0.8.145, off by default)
    int pendingMaxPolyphony = 0;          // the limit a measurement decided, set in the engine once the run's tails have died (0.8.146)
    juce::StringArray nestedTreesDropped; // folder trees dropped from the pool for lying inside another (the notice after the load)
    juce::StringArray nestedTreesNoticed; // the ones the user was told about in this session (settings.json and the configuration file both carried the tree: two boxes, 0.8.144)
    // IR processing (Configuration Editor > Engine): "automatic" (Light on a
    // computer that cannot afford Full, see applyIrProcessing), "full", "light".
    // irLightAuto: the automatic choice made on this computer, kept over starts.
    juce::String irProcessing{ "automatic" };
    bool irLightAuto = false;
    bool irLightNoticeShown = false;
    int irEvaluatedRoutes = -1;             // the reverbs the automatic choice last looked at (timer: a new build re-evaluates)
    double irEvaluatedCost = -1.0;
    int irEvaluateDivider = 0;
    void applyIrProcessing(bool fromLoad);
    juce::String irProcessingStatus() const;
    int renderThreads = 0;                          // custom worker count, 0 = use the preset
    bool renderThreadPresetChosen = false;          // set in the Configuration Editor on this computer (a preset that was never chosen goes back to the default)
    juce::String renderKernel = "auto";             // interpolation kernel: "auto" or "scalar" / "sse2" / "avx2" / "neon" (machine-specific, 0.8.124)
    juce::String renderThreadPreset{ "allMinusOne" }; // all | allMinusOne | physical | physicalMinusOne
    int workersForCurrentPreset() const;
    void applyRenderThreadSetting();
    void showConfigurationEditor();
    // About, user manual & licences: one dialog, a list on the left (the manual
    // first, then every licence text), the text on the right. Everything is
    // embedded (Source/ManualData.h, Source/LicencesData.h). With a manual section
    // the dialog opens on the manual, scrolled to that heading, the heading orange.
    void showAboutDialog(const juce::String& manualSection = {}, const juce::String& fallbackSection = {});
    std::unique_ptr<juce::DialogWindow> aboutWindow;
    std::function<void(const juce::String&, const juce::String&)> aboutOpenChapter;   // 0.8.212: the kept manual window jumps to a chapter

    // "Set for all perspectives & divisions" in the audio tab: the source
    // route's audio device is copied everywhere; any other device choice cancels it.
    // Two scopes: the ordinary perspectives of all divisions (auxiliary routes
    // excluded) and the auxiliary routes of all divisions.
    std::atomic<bool> applyAudioToAll{ false };        // one state shared by every ordinary audio tab
    std::atomic<bool> applyAudioToAllAux{ false };     // ... and one by every auxiliary tab
    bool applyAllAuxScope = false;                     // which scope the running "set for all" copies to
    // "Set effects for all": IR, dry/wet, IR settings, perspective volume and
    // channel separation stay linked (a change on one tab goes to all) until the
    // box is cleared or the routes stop agreeing. Same two scopes.
    std::atomic<bool> effectsLinked{ false }, effectsLinkedAux{ false };
    bool applyingEffectsToAll = false;   // while "set effects for all" copies the values (they must not untick it)
    static bool isAuxiliaryPerspective(const juce::String& perspective);
    void propagateEffectsIfLinked(int manual, const juce::String& perspective);
    struct ApplyAllStep { int manual; juce::String perspective; };
    std::vector<ApplyAllStep> applyAllQueue;           // processed one route per few GUI ticks
    std::unique_ptr<juce::XmlElement> applyAllState;   // device state being copied
    juce::String applyAllType, applyAllDeviceName;
    // Aggregated WASAPI: the source's output channels (endpoints) are copied too -
    // every route of the scope then plays through the same endpoints.
    bool applyAllCopyChannels = false;
    std::vector<int> applyAllChannels;
    int applyAllTotal = 0, applyAllReopenManual = -1, applyAllCheckDivider = 0, applyAllTickDivider = 0;
    bool applyAllRunning = false;
    juce::Component::SafePointer<juce::DialogWindow> applyAllWindow;
    juce::Component::SafePointer<juce::Label> applyAllLabel;

    // Set by the open Divisions dialog: detach(true) before selectors are
    // recreated, rebind(false) afterwards, so the dialog can stay open.
    std::function<void(bool detach)> keyboardsDialogSelectorHook;

    // Name of the portable configuration in use: window title and default recording name.
    juce::String currentConfigurationName;
    juce::File currentConfigurationFile;      // portable configuration last saved or loaded
    bool configurationDirty = false;          // settings changed since that configuration was saved/loaded
    bool configurationUsedThisSession = false; // a configuration was loaded or saved since start-up (else quit never asks)
    double startupCompleteMs = 0.0;           // saves before this moment are start-up bookkeeping, not user changes
    void setCurrentConfigurationName(const juce::String& name);
    void updateWindowTitle();

    // Recording of the stereo mix of all outputs (see SamplerEngine::startRecording).
    RecordButton recordButton;
    std::unique_ptr<juce::FileChooser> recordChooser;
    // Stopping a recording first fades it out for 500 ms; the GUI timer closes
    // the file once the fade has been written (or after a safety timeout).
    double recordingFadeOutStartedMs = -1.0;
    void toggleRecording();

    // Refuses to load sample folders that would not fit into physical RAM.
    bool checkMemoryForFolders(const std::vector<juce::File>& folders);
    juce::AudioDeviceManager* managerForRoute(int manualIndex, const juce::String& perspective);
    // IR reverb per perspective: file dialog, and pushing the stored setting into the engine.
    std::unique_ptr<juce::FileChooser> irChooser;
    void chooseReverbIr(int manualIndex, const juce::String& perspective);
    void applyPerspectiveReverb(int manualIndex, const juce::String& perspective);
    void showReverbToneDialog(int manualIndex, const juce::String& perspective);
    // "Set effects for all": this route's IR, dry/wet, IR settings, volume and
    // channel separation for every route of its scope (ordinary or auxiliary).
    void applyEffectsToAllPerspectives(int sourceManual, const juce::String& sourcePerspective);
    bool allRoutesShareEffects(bool auxiliary);
    // asioDeviceName: for ASIO, the driver to join (empty = the first open ASIO
    // host, or the default ASIO device when none is open yet).
    juce::AudioDeviceManager* setRouteAudioType(int manualIndex, const juce::String& perspective,
        const juce::String& typeName, const juce::XmlElement* deviceState = nullptr,
        const juce::String& asioDeviceName = {});
    void applyAudioSetupEverywhere(int sourceManual, const juce::String& sourcePerspective);
    void continueApplyAll();
    bool allRoutesShareAudioDevice(bool auxiliary = false);
    // Windows Audio / WASAPI callbacks render with this many threads (1 = the
    // driver thread alone, which measured best); ASIO uses the full pool.
    std::atomic<int> wasapiRenderLanes{ 1 };
    std::atomic<int> aggregatedRoutesRunning{ 0 };   // routes with an Aggregated WASAPI device started (0.8.148: the lanes cap cores / routes)
    int renderThreadsWasapi = 1;
    bool showUnusableAudioDevices = false;   // Linux: list also the ALSA devices of the cards the sound server holds
    // Swell shutter curve: fast gain rise near closed, flattening when open.
    static float swellCurve(float position) noexcept;
    float expressionGainFor(const ManualConfig& config) const noexcept;
    // ASIO drivers restart themselves when their control panel changes the
    // latency. Such a restart is detected and the device is reopened cleanly
    // with the driver's new preferred buffer size.
    std::atomic<double> ignoreDeviceRestartsUntil{ 0.0 };
    std::atomic<double> unexpectedDeviceStartMs{ -1.0 };
    std::atomic<double> asioPreferredMismatchMs{ -1.0 };   // device runs with a non-preferred buffer size
    double lastAsioReconcileMs = -1.0;
    void reconcileAsioDevices();
    // Opens the driver's control panel and afterwards reopens a fresh device
    // instance at the driver's preferred buffer size and sample rate.
    void openAsioControlPanel(juce::AudioDeviceManager& manager);
    void reopenAudioDeviceFresh(juce::AudioDeviceManager& manager, const juce::String& reason);
    // Linux, JACK through PipeWire: one route's device changed PipeWire's buffer
    // size or rate (shared by every JACK client); the other JACK routes reopen.
    void reopenJackRoutesAfterClockChange(juce::AudioIODevice* changedBy);
    struct AsioPanelWatch
    {
        bool active = false;
        double startedMs = 0.0;
        juce::AudioDeviceManager* manager = nullptr;
        std::vector<void*> before, panelWindows;   // HWNDs
        double closedAtMs = -1.0;                  // when the panel window disappeared
        juce::String deviceName;                   // for matching the panel by its title
    };
    // Dialog positions remembered across reopenings (shared by all divisions);
    // Global Reset clears them.
    std::map<juce::String, juce::Rectangle<int>> dialogBounds;
    std::vector<std::pair<juce::String, juce::Component::SafePointer<juce::Component>>> trackedDialogs;
    void placeDialog(juce::Component& window, const juce::String& key, int defaultWidth, int defaultHeight);
    void rememberDialogBounds();
    int dialogBoundsDivider = 0;
    AsioPanelWatch asioPanelWatch;
    void pollAsioPanelWatch();
    // Once a second: log audio callbacks that disagree with their device (rate, block size, gaps).
    void checkAudioCallbackHealth();
    int audioHealthDivider = 0;
    // The RME Fireface USB driver cannot change its hardware period while a
    // Windows Audio (WDM) stream runs on the same interface: ASIO then reports
    // the new buffer size but keeps calling back in the old period ("digitally
    // broken" sound). Around every fresh ASIO reopen the Windows Audio devices
    // of this process are therefore closed and restored afterwards.
    struct SuspendedDevice
    {
        juce::AudioDeviceManager* manager = nullptr;
        juce::AudioDeviceManager::AudioDeviceSetup setup;
    };
    std::vector<juce::AudioDeviceManager*> allAudioManagers();
    std::vector<SuspendedDevice> suspendWindowsAudioDevices();
    void resumeWindowsAudioDevices(const std::vector<SuspendedDevice>& suspended);
    // Windows Audio devices closed around an ASIO reopen are restored from the
    // GUI timer, one per tick, so the audio tab shows the new buffer size at once.
    std::vector<SuspendedDevice> pendingWindowsAudioResume;
    // Recovery when an ASIO device measurably calls back in a period that does
    // not match its buffer size (buffer changed in the driver's own panel).
    int asioRateMismatchSeconds = 0;
    int asioRecoveriesSinceUserAction = 0;
    double audioDeviceSelectionUnmuteAt = -1.0;
    juce::ToneGeneratorAudioSource quietTestTone;
    juce::AudioSourcePlayer quietTestPlayer;
    juce::AudioDeviceManager* quietTestManager = nullptr;
    double quietTestStopAt = -1.0;
    std::unique_ptr<juce::DialogWindow> keyboardsWindow;
    std::unique_ptr<juce::DialogWindow> temperamentsWindow;
    std::unique_ptr<juce::DialogWindow> configurationWindow;
    // Set by the open Configuration Editor: rebuilds its rows. Called (deferred)
    // after every action that changes the values it shows.
    std::function<void()> configEditorRefresh;
    void refreshConfigurationEditor()
    {
        juce::Component::SafePointer<MainComponent> safeThis(this);
        juce::MessageManager::callAsync([safeThis] { if (safeThis != nullptr && safeThis->configEditorRefresh) safeThis->configEditorRefresh(); });
    }
    int keyboardsWindowManual = -2;
    juce::String keyboardsWindowAudioTab;

    juce::File topFolder;
    juce::File memoriesJsonFile;
    juce::File voicerStateFile;
    juce::File lastDirFile;
    juce::File settingsFile;
    std::unique_ptr<juce::FileChooser> folderChooser;
    std::unique_ptr<juce::FileChooser> combinationsFileChooser;
    std::unique_ptr<juce::FileChooser> configurationFileChooser;
    std::shared_ptr<HauptwerkOdf> hauptwerkImport;   // the organ definition being imported (until its voicing is applied)
    juce::File hauptwerkImportReport;                // ImportReports/<organ>-<date>.txt in the application data (0.8.192): the plan, then the voicing result
    // The extensions/transmissions of the definition (stops playing another
    // stop's ranks shifted): made as transmissions once the owner stops are loaded.
    struct HauptwerkExtension { int manual = 0, ownerManual = 0; juce::String ownerName; int shift = 0; juce::String alias; int column = -1, row = -1; int lowKey = -1, highKey = -1; };   // column/row: the grid cell from the jamb page (0.8.142), -1 none; lowKey/highKey: the keys of its rows (0.8.159)
    std::vector<HauptwerkExtension> hauptwerkExtensions;
    std::map<std::pair<int, juce::String>, std::pair<int, int>> hauptwerkCompass;   // (division, row key) -> the keys of the stop's own rows in the definition (0.8.159)
    std::unique_ptr<juce::FileChooser> configurationRootChooser;
    juce::File lastSettingsDirectory;

    double currentSampleRate = 48000.0;
    int savedWindowWidth = 1680;
    int savedWindowHeight = 900;
    int observedWindowWidth = 0;
    int observedWindowHeight = 0;
    double windowResizeSaveAt = -1.0;
    double voicingSaveAt = -1.0;
    int dialogRepaintDivider = 0;
    int combinationLightDivider = 0;
    // After a factory reset, do not immediately put the still-running audio
    // device back into the freshly cleared JSON.  The next deliberate click
    // in an audio selector makes device-state persistence active again.
    bool ignoreCurrentAudioStateUntilUserSelection = false;

    static constexpr int maxPistons = 10;
    static constexpr int maxBank = 999;
    std::unordered_map<int, MemorySlot> combinations;

    // General crescendo: 10..60 taught steps (stops and couplers), driven by a
    // MIDI controller or by the crescendo display on the combination row.
    // Step 0 = off: the registration from before the crescendo engaged returns.
    class CrescendoDisplay;
    static constexpr int minCrescendoSteps = 10, maxCrescendoSteps = 60;
    std::unique_ptr<CrescendoDisplay> crescendoDisplay;
    std::unique_ptr<juce::DialogWindow> crescendoWindow;
    std::unique_ptr<juce::FileChooser> crescendoFileChooser;
    std::function<void()> crescendoDialogRefresh;   // set by the open dialog
    std::function<void()> stopPoolRefresh;          // set by the open Division dialog: "Stops on the grid" lists
    int crescendoStepCount = 30;
    std::vector<MemorySlot> crescendoSteps;         // [step - 1]; empty stopData = not taught
    float crescendoPosition = 0.0f;                 // 0..1
    int crescendoStep = 0;                          // 0 = everything off (or the hand registration)
    // "Cresc." button: on remembers the hand registration and lets the steps act,
    // off brings that registration back.
    juce::TextButton crescendoButton{ "Cresc." };
    bool crescendoEnabled = false;
    bool crescendoCancelsHandStops = true;          // off: steps only add to the hand registration
    MemorySlot crescendoRestore;                    // hand registration from when Cresc. was switched on
    bool crescendoRestoreValid = false;
    // Non-cancelling mode: the items (stops, transmissions, couplers) the crescendo
    // itself switched on; only these are ever switched off again.
    std::set<std::string> crescendoAdded;
    void setCrescendoEnabled(bool enabled);
    void allStopsAndCouplersOff();
    void applyRegistrationAdditive(const MemorySlot& memory);   // switches on what the slot has on
    std::set<std::string> registrationOnSet(const MemorySlot& memory) const;   // keys of everything the slot has on
    bool isRegistrationItemOn(const std::string& key) const;
    void setRegistrationItem(const std::string& key, bool on);
    void setCrescendoPosition(float normalized);
    void applyCrescendoStep(int step);
    void storeCrescendoStep(int step);
    void setCrescendoStepCount(int count);
    void voidCrescendo();
    bool isCrescendoStepStored(int step) const;
    juce::var crescendoToVar() const;
    void crescendoFromVar(const juce::var& data);
    void showCrescendoDialog();
    void showCrescendoMenu();
    void beginMidiLearn(const juce::String& target, bool toggle, bool continuous);
    void clearMidiBinding(const juce::String& target);
    std::unique_ptr<MemorySlot> pendingConfigurationRegistration;
    int pistonCount = 10;
    int currentBank = 0;
    int activePiston = -1;
    int heldBankDirection = 0;
    double bankHoldStartMs = 0.0;
    double lastBankRepeatMs = 0.0;

    struct MidiButtonBinding
    {
        juce::String deviceIdentifier;
        int channel = 1;
        int messageType = 0; // MIDI status high nibble (note, CC, program change...)
        int number = 0;
        bool toggleBehaviour = false;
        bool continuous = false;
        bool lastOn = false;
    };
    std::unordered_map<std::string, MidiButtonBinding> midiButtonBindings;
    juce::CriticalSection midiButtonBindingLock;
    juce::String pendingButtonLearnTarget;
    bool pendingButtonLearnToggle = false;
    bool pendingButtonLearnContinuous = false;
    juce::Component::SafePointer<juce::AlertWindow> midiLearnWindow;
    juce::Component::SafePointer<juce::AlertWindow> keyboardLearnWindow;

    struct Temperament
    {
        juce::String name;
        std::array<float, 12> cents{};
        int rotation = 0;
        bool builtIn = true;
        juce::String description;   // last column of temperaments.csv, shown with the details
    };
    std::vector<Temperament> temperaments;
    // Recorded temperament of a stop matched against the tables above.
    // Returns the description for the voicing row; bestName/bestRotation/bestError
    // receive the closest table (empty name when nothing fits).
    juce::String describeRecordedTemperament(const Stop* stop, juce::String* bestName = nullptr,
        int* bestRotation = nullptr, float* bestError = nullptr) const;
    // After loading: when most stops agree on one recorded temperament, it
    // becomes the base ("black") temperament; otherwise nothing changes.
    void autoDetectBaseTemperament();
    float undulationBeatRatio = 12.0f;   // celeste: top-of-compass beats vs bottom (1..20)
    bool equalAt440 = false;            // global: every stop normalised to equal temperament at a1 = 440 Hz
    juce::String baseTemperamentName;
    juce::String selectedTemperamentName;
    bool coincidentTemperamentShowsTarget = true;

    // unisonOff: a per-division switch that silences the division's own pitch
    // (its keys then sound only through couplers, e.g. the 4' coupler alone).
    enum class CouplerType { unison, subOctave, superOctave, melody, bass, unisonOff };
    struct Coupler
    {
        juce::String id, label;
        int sourceManual = 0; // keyboard being played (denominator)
        int targetManual = 0; // division being pulled (numerator)
        CouplerType type = CouplerType::unison;
        bool enabled = false;
        bool visible = true;
        std::map<int, int> activeNotes; // target MIDI note -> velocity
    };
    std::vector<Coupler> couplers;
    // Display order of the coupler buttons per source division (arranged by dragging).
    std::map<int, std::vector<juce::String>> couplerOrder;
    // Global sound sliders on the main panel: each may be shown or hidden
    // (Configuration Editor). With none shown the combination row and the stops
    // move up and the polyphony display shrinks into the combination row.
    // Enum values are the storage indices (new sliders are appended so saved
    // flags keep their meaning); the main panel shows them in kGlobalSliderOrder.
    enum GlobalSlider { gsAttackDelay = 0, gsAttackMin, gsReleaseFadeOut, gsReleaseFadeIn, gsReleaseScaling,
        gsPitch, gsMaster, gsReleaseCut, gsCount };
    static constexpr GlobalSlider kGlobalSliderOrder[gsCount]{ gsAttackDelay, gsAttackMin, gsReleaseFadeOut, gsReleaseFadeIn,
        gsReleaseScaling, gsReleaseCut, gsPitch, gsMaster };
    std::array<bool, gsCount> globalSliderShown{ true, true, true, true, true, true, true, false };
    bool anyGlobalSliderShown() const noexcept
    {
        for (const bool shown : globalSliderShown) if (shown) return true;
        return false;
    }
    // Advanced global audio (Configuration Editor), pushed into the engine.
    // The polyphony measurement's limit for the longest driver callback as a share
    // of its period (0.6..1.0, default 0.85; Configuration Editor, machine-specific).
    float measurementShareLimit = 0.85f;
    float windLevelDbPerCent = 0.2f;
    float tremulantLevelDbPerCent = 0.187f;
    float windOneStopShare = 0.2f, windOnePipeShare = 0.5f, windTwoPipesShare = 0.5f;
    float releaseCutPercent = 100.0f;
    float windMaxEffect = 1.0f;      // wind model strength 0..1 (each key stroke uses a random 40..100 % of it)
    float toneStartHz = 1800.0f;     // per-key tone shelf: start of the transition band
    float sincKernelFromHz = 3000.0f;   // the sinc playback kernel for pipes sounding from here up (0.8.164; 0 = all, 20000 = none); machine-specific
    float retuneLimitCents = 200.0f;   // a pipe further off its key than this is faulty: off, filled from the neighbours (0.8.165); travels with the organ
    // What the published README says about the organ (0.8.179): asked once at the portable
    // save, kept in the configuration ("publication") - no "(fill in: ...)" places any more.
    juce::String publicationOrgan, publicationProducer, publicationProduct, publicationLink, publicationAuthor;
    bool publicationDetailsAsked = false;
    void askPublicationDetails(std::function<void()> then);
    void applyAdvancedAudioSettings();
    // Harmonic filter depth (dB at full attenuation) following swell, wind and tremulant; 0 = off.
    float harmonicSwellDb = 8.0f, harmonicWindDb = 2.0f, harmonicTremulantDb = 4.0f;   // harmonic filter depths
    float tremulantShelfPhaseDegrees = 90.0f;   // the tremulant's tone oscillation behind its volume oscillation
    float velocityFadeInMs = 100.0f;            // key velocity on stops without velocity layers: fade-in at the lowest velocity (0.8.196: 100 ms, was 40)
    int velocityFullFrom = 80;                  // ... and the velocity from which a stroke is a full one (no fade-in, no treble cut); the ramp runs below it (0.8.196)
    bool velocityPlainStrict = false;           // velocity layers: an unticked "_128" never plays (0.8.192; off = the plain files fill the gaps)
    float velocityShelfDb = 9.0f;               // ... and the high-shelf cut on the attack at the lowest velocity (the chiff blunted)
    float velocityShelfMs = 180.0f;             // ... opening to nothing over this time (0.8.196: 180 ms, was 80)
    int fillGroupSize = 12;   // missing pipes: how many adjacent pipes are borrowed as a group (1..12)
    int transposeQuality = 2; // resampling of filled pipes: 0 linear, 1 cubic, 2 cubic + anti-alias copies
    int uiScalePercent = 100; // interface size (UIScale.h); 0 in the settings = not chosen yet, the screen's estimate is used
    int transposeMethod = 2;  // filled pipes: 0 resampling (length changes), 1 pitch shift (resampling + timbre EQ + release reshaped), 2 pitch shift + time stretch (default), 3 phase vocoder
    float transposeAttackTimingPct = 50.0f;     // method 2: share of the attack's length change given back (100 = source duration)
    float transposeReleaseTimingPct = 55.0f;    // methods 2-3: the same for the release (vocoder: upwards only)
    float transposeReleaseTrimMs = 140.0f;      // every pitch-shift method: a release copy is played from here (playback, no rebuild)
    bool transposeReleaseLevelMatch = false;    // release copies at the source's level step (playback, no rebuild)
    // The "?" buttons: the About dialog on the manual, at the chapter of the option.
    void showManualSection(const juce::String& section, const juce::String& fallbackSection = {});
    juce::String copyCacheFolder;               // the cache folder chosen by the user (empty = FilledPipesCache in the application data)
    std::unique_ptr<juce::FileChooser> cacheFolderChooser;
    float transposeReleaseLengthPct = 100.0f;   // pitch shift: a release copy's duration as % of its source's (both directions)
    float transposeLoopCrossfadePct = 40.0f;    // filled pipes: loop splice as % of the loop length (0 = the ordinary 50 ms)
    bool transposeKeepLoudness = true;          // pitch shift: copies at the loudness of their source
    bool copyCacheEnabled = true;               // the generated copies kept on disk (FilledPipesCache in the application data); on by default since 0.8.124
    juce::File copyCacheDirectory() const;      // copyCacheFolder when set and usable, else the default
    // The one folder for the organ configurations of this computer (0.8.190; machine-specific):
    // every import writes there, Save and Load start there. Empty = Documents/OrganSampler/Configurations.
    juce::String configurationsFolder;
    juce::File configurationsDirectory() const;   // created on demand
    std::unique_ptr<juce::FileChooser> configurationsFolderChooser, sampleLocationChooser;
    // Folders under which sample trees and archives live (0.8.190; machine-specific): a
    // tree a configuration names is looked for under each (<location>/<library root
    // name>/<relative path>, <location>/<relative path>) before anybody is asked.
    // Filled by the program: the library root's parent at a portable save, the root
    // found at a load, the folder of a chosen archive; and by hand in the Editor.
    juce::StringArray sampleLocations;
    void noteSampleLocation(const juce::File& folder);   // remembered once, saved with the settings
    bool loadLastConfigurationAtStartup = false;   // re-open the last configuration file when the program starts
    // Remembered UI folding: Configuration Editor sections (title -> collapsed) and
    // the Division dialog's "Tremulant +" / "Wind model +" groups.
    std::map<juce::String, bool> configEditorCollapsed;
    bool divisionTremulantExpanded = false, divisionWindExpanded = false;
    // Folding of the per-key rows ("Volume per key", "Treble per key") in the voicing.
    // Audio devices missing at start-up: collected and reported in one message.
    juce::StringArray missingAudioDevices;
    juce::StringArray reportedMissingAudioDevices;        // reported once per session, then silence
    std::map<juce::String, double> audioAnomalyLastLogMs;  // callback anomalies: logged when they appear, then once a minute
    double audioLevelsLastLogMs = -1.0e9;                  // "Audio levels" at most every 10 s
    std::map<juce::String, double> missingDeviceNextTryMs; // failed reopen attempts: next silent try
    bool missingAudioDevicesReportPending = false;
    double missingDevicesReportAtMs = -1.0;             // the list is shown when the wait below is over
    // Devices register late at start-up (an RME interface switches its sample
    // rate first): a missing device is waited for up to 6 s with a "Waiting for
    // audio devices" box in the window, checked every second, and reported only
    // when it is still absent afterwards.
    static constexpr double kAudioDevicesWaitMs = 6000.0;
    double audioDevicesWaitDeadlineMs = -1.0;
    // A notice over the stop area, drawn like the message boxes (their background,
    // outline and text colours, rounded corners) so the two keep one identity.
    struct NoticeBox final : public juce::Label
    {
        void paint(juce::Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            const auto r = getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(lf.findColour(juce::AlertWindow::backgroundColourId));
            g.fillRoundedRectangle(r, 6.0f);
            g.setColour(lf.findColour(juce::AlertWindow::outlineColourId));
            g.drawRoundedRectangle(r, 6.0f, 2.0f);
            g.setColour(lf.findColour(juce::AlertWindow::textColourId));
            g.setFont(getFont());
            g.drawText(getText(), getLocalBounds().reduced(12, 2), juce::Justification::centred, true);
        }
    };
    NoticeBox waitingForAudioLabel;
    // "Compiling the Stop List ..." box: shown while loaded divisions are committed
    // and the stop list and grid are built (seconds on a big organ).
    NoticeBox generatingLabel;
    void showGeneratingMessage(bool on);
    bool userTouchedAudioSelector = false;               // a substitute device counts as the user's choice only after that
    void updateWaitingForAudioLabel();
    void forgetMissingAudioDevice(const juce::String& deviceName);
    bool isAudioDeviceOpenSomewhere(const juce::String& deviceName);
    bool isAudioDeviceAvailableNow(const juce::String& typeName, const juce::String& deviceName, juce::String* error);
    void showMissingAudioDevicesIfDue();
    // Safe start: a marker file while a device is being opened; if it is still
    // there at the next start, that device is opened with the default buffer and
    // rate once (a buffer the driver crashes on must not lock the program out).
    juce::File deviceOpenMarker() const;
    void beginDeviceOpen(const juce::XmlElement& setup);
    void endDeviceOpen();
    void applySafeStart(juce::XmlElement& setup);
    void showSafeStartNoticeIfDue();
    juce::String crashedWhileOpening;   // "type|device|buffer|rate" of the open that crashed last time (this session only)
    juce::String safeStartNotice;       // the message to show once the loading is over
    void noteIfDeviceMissing(juce::AudioDeviceManager& manager, const juce::XmlElement* wanted);
    // Main window placement: remembered position and maximised state.
    int savedWindowX = INT_MIN, savedWindowY = INT_MIN;
    bool savedWindowMaximised = false;
    // Configuration "changed" detection: the settings without their volatile
    // parts (window geometry, dialog positions) compared with the state at the
    // last configuration load or save.
    juce::String lastSettingsFingerprint, configurationFingerprint;
    void markConfigurationClean() { configurationFingerprint = lastSettingsFingerprint; configurationDirty = false; }
    void noteMissingAudioDevice(const juce::String& description);
    // Routes whose saved device is not open: when the device appears (plugged in,
    // driver started) the route is reopened with its saved state, without the user.
    void watchMissingAudioDevices();
    int missingDeviceWatchDivider = 0;
    bool melodyBassNewKeysOnly = true;
    bool melodyBassGraceTime = true;
    std::array<std::map<int, int>, SamplerEngine::maxManuals> physicallyHeldNotes;
    std::array<std::map<int, double>, SamplerEngine::maxManuals> physicalNotePressTimes;
    std::array<std::array<int, 128>, SamplerEngine::maxManuals> routedNoteRefCounts{};
    juce::CriticalSection couplerLock;

    void selectTopFolder();
    void applyDefaultWindProfile(int manualIndex);
    void applyDefaultTremulantProfile(int manualIndex);
    void setExpressionTarget(int manualIndex, float position);
    void updateExpressionInertia();
    void selectManualFolder(int manualIndex);
    void unloadManualFolder(int manualIndex);        // menu of the loaded folder trees
    void unloadWholeManualFolder(int manualIndex);   // removes the whole division
    void beginStartupFolderLoading();
    void panic();
    void playQuietTestTone(juce::AudioDeviceManager& manager);
    void showKeyboardsDialog(int onlyManual = -1);
    void loadBuiltInTemperaments();
    void showTemperamentsDialog();
    int findTemperament(const juce::String& name) const;
    void selectTemperament(const juce::String& name);
    void setBaseTemperament(const juce::String& name);
    void applyTemperamentTuning();
    float temperamentCentsAt(const Temperament& temperament, int pitchClass) const noexcept;
    void deleteCustomTemperament(const juce::String& name);
    void beginKeyboardLearn(int manualIndex);
    void showKeyboardLearnMenu(int manualIndex);
    void saveMemories();
    void loadMemories();
    bool saveMemoriesToFile(const juce::File& file);
    bool loadMemoriesFromFile(const juce::File& file);
    // Combination banks as JSON (memories.json, combination files and saved configurations).
    juce::var combinationsToVar() const;
    bool combinationsFromVar(const juce::var& parsed);
    void chooseCombinationsFileToSave();
    void chooseCombinationsFileToLoad();
    void chooseConfigurationToSave(std::function<void(bool saved)> then = nullptr);
    void launchConfigurationLoadChooser();
    void loadConfigurationFile(const juce::File& file);   // the chosen file: parsed, checked, applied
    // Before a configuration's samples load (0.8.146): the sample storage, the
    // packing and the rate on loading, shown with the file's own values (else
    // this computer's last choice) - the user confirms or changes them, they go
    // into the configuration and are saved with it. Cancel abandons the load.
    // 'file' (0.8.191): the configuration's own file - lying outside the configurations
    // folder, the box offers to copy it there (the var gets "copyIntoConfigurationsFolder").
    // 'importing' (0.8.193): the file is the import's target in the folder - exists it, the box asks replace / keep.
    void askLoadOptions(juce::var configuration, const juce::String& what, std::function<void(juce::var)> proceed, const juce::File& file = {}, bool importing = false);
    void adoptCopyInConfigurationsFolder(const juce::File& original, const juce::String& textFromZip = {});   // the copy becomes the open file; the original stays (a zip: its .json text)
    juce::File configurationsCopyOf(const juce::File& file) const;   // the file's place in the configurations folder, ".portable" dropped (0.8.201)
    static void copyLoadOptions(const juce::var& from, juce::var& to);   // the five storage choices of the load box
    void chooseConfigurationToLoad();
    // portable: for another computer - no MIDI, no audio devices or channels,
    // no machine settings, no unused trees (see the manual, "Portable
    // configuration (to share with others)").
    void savePortableConfiguration(const juce::File& file, bool includeMidi, bool portable = false, bool publishFiles = true);
    // A Hauptwerk organ definition (ODF) chosen in Load Configuration: read for
    // its divisions, stops, sample folders, perspectives and per-pipe levels, a
    // configuration made of them (written beside the samples, in the organ's
    // installation package) and loaded; the voicing is applied once the folders
    // are loaded. See HauptwerkOdf.h.
    void importHauptwerkOdf(const juce::File& odf, bool mergeIntoLoaded = false);
    void applyHauptwerkVoicing(std::shared_ptr<HauptwerkOdf> odf, const juce::File& configurationFile, const juce::String& summary, bool saveIntoPackage = true);
    // Merging another configuration (or a Hauptwerk organ definition) into the
    // loaded organ: right-click Load Configuration. MainComponentMerge.cpp.
    void mergeConfigurationDialog();
    void mergeConfigurationFile(const juce::File& file);
    void mergeConfiguration(juce::var added, const juce::File& addedFile, const juce::String& addedName, std::function<void()> afterLoad);
    friend class PolyphonyDisplay;   // the RAM column reads the two memory figures below
    // Before a configuration loads: the WAV bytes of its trees against the free
    // memory, a warning (log + message) when the samples will not fit - the
    // system then pages them out and every stroke stalls (#16, #17). Counted on
    // a thread of its own; the load goes on meanwhile.
    void warnIfMemoryShortForTrees(std::vector<juce::String> trees, const juce::String& configurationName);
    static juce::int64 availablePhysicalMemoryBytes();   // free physical memory now (the samples must fit)
    static juce::int64 processMemoryBytes();             // the RAM this program holds now (resident), 0 when unknown
    // The CPU clocks for the polyphony measurement: this process's CPU time (ms,
    // all threads) and the system's busy and total times (ms, all CPUs); false
    // where the platform offers none. MainComponentMerge.cpp.
    static bool readCpuClocks(double& processMs, double& systemBusyMs, double& systemTotalMs);
    // The power setting worth a word in the measurement's report: the Linux CPU
    // governor, the Windows power plan; empty when unknown.
    static juce::String powerSettingDescription();
    // The README of a configuration for the configurations folder on GitHub, in
    // the published form: written beside every portable configuration as
    // "<name>.README.md" (the user fills in the organ, the samples and the author).
    juce::String configurationReadme(const juce::var& configuration, const juce::File& file) const;
    // Development hooks: runs the action once the organ is loaded (polled every
    // 5 s, then 5 s of quiet) - the loading of a large organ takes its time.
    void runWhenLoaded(std::function<void()> action);
    // A portable configuration was loaded: once the organ is up, a message says
    // what to set on this computer (MIDI, audio tabs, measure, save).
    bool portableNoticePending = false;
    bool suppressPortableNotice = false;   // set by the Hauptwerk import: its configuration is made here, no "from another computer" notice
    void showPortableNoticeIfDue();
    void resolveAndApplyPortableConfiguration(juce::var configuration,
        const juce::File& configurationFile, const juce::File& selectedLibraryRoot = {});
    void applyPortableConfiguration(juce::var configuration,
        const juce::File& configurationFile, const juce::File& libraryRoot);
    void pressPiston(int piston);
    void storeCombination(int piston);
    void restoreCombination(int piston);
    void applyRegistrationState(const MemorySlot& memory, bool includeMasterVolume = true);
    void stepCombination(int direction);
    void changeBank(int direction, bool persist = true);
    void bankButtonStateChanged(int direction, bool isDown);
    void updateHeldBankNavigation();
    void updateCombinationControls();
    void updateCombinationLights();
    MemorySlot captureCurrentState() const;
    bool memoryMatchesCurrent(const MemorySlot& memory) const;
    static int combinationKey(int bank, int piston) { return bank * maxPistons + piston; }
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void showMidiLearnMenu(const juce::String& target);
    void showSwellLearnMenu(int manualIndex);
    // Right-click on a division header's background: the division's swell,
    // tremulant and wind settings, the Divisions dialog, the Configuration Editor.
    void showDivisionMenu(int manualIndex);
    // The settings of one feature of a division ("swell", "tremulant", "wind")
    // in a small dialog: the same rows as the Configuration Editor's.
    void showFeatureSettings(int manualIndex, const juce::String& feature);
    bool processMidiButtonMessage(const juce::String& sourceIdentifier, const juce::MidiMessage& message);
    void activateMidiTarget(const juce::String& target, bool isOn, bool toggleBehaviour,
        bool pulseMessage = false);
    bool applyMidiSliderValue(const juce::String& target, float normalizedValue);
    juce::String targetForComponent(const juce::Component* component) const;
    void rebuildCouplerCatalog();
    void rebuildCouplerRail();
    // Removes one folder tree from a composite division (all stops that came from it).
    void unloadManualTree(int manualIndex, const juce::String& treePath);
    void refreshCouplerRailState();
    std::vector<DivisionCouplerInfo> getDivisionCouplers(int manualIndex);
    void showCouplersDialog();
    void setCouplerEnabled(const juce::String& id, bool enabled);
    void routeKeyboardNote(int manual, int note, int velocity, bool isOn);
    int mapKeyboardNote(int manual, int note) const;
    int newestHeldKey(int manual, int& velocity) const;   // the key pressed last (-1: none) and its MIDI velocity, for the Settings keyboard
    void recomputeCouplersForSource(int sourceManual, int changedNote = -1,
        bool changedNoteWasPressed = false);
    void applyRoutedNoteDelta(int manual, int note, int velocity, int delta);
    Coupler* findCoupler(const juce::String& id);
    void performGlobalReset();
    void executeGlobalReset(bool clearMidiLearn, bool resetAudioOutputs,
        bool removeTransmissions, bool clearFolderPatterns, bool resetCouplers, bool resetStopGrid,
        bool unloadEverything = false);
    // Forgets the sample library in memory: the tree pool, every division's table
    // and tree list (nothing is loaded at start-up unless a configuration is).
    void clearSampleLibraryInMemory();
    // "Shut down PC": saves the configuration silently, quits and shuts Windows down.
    void shutdownComputer();
    void resetAudioOutputsToDefault();
    void enableAllStops();
    void disableAllStops();
    // "Measure the maximum polyphony" (Configuration Editor): with the loaded set,
    // every stop engaged and the keys struck repeatedly on every division, the
    // limit is raised step by step until a block exceeds its budget or a callback
    // comes late; the maximum polyphony is then set with headroom. Needs a set.
    struct PolyphonyMeasurement;
    std::shared_ptr<PolyphonyMeasurement> polyphonyMeasurement;
    // No hidden routes, nothing stale: a division's own ("default") route that
    // no stop plays through, and every route of a division beyond the division
    // count, keeps no device (none) and no settings; a route that no longer
    // exists loses its settings; an auxiliary route without a device keeps none
    // (no IR of an old set waiting for a device). The configuration holds only
    // what is in use now - the user's fundamental rule.
    void closeHiddenRoutes();
    bool canMeasurePolyphony() const;
    void startPolyphonyMeasurement();
    void cancelPolyphonyMeasurement();
    void finishPolyphonyMeasurement(bool cancelled);
    int takeLateCallbacksOfAllRoutes();
    void saveStopSettings();
    void loadStopSettings();
    void saveLastDirectory();
    void mergeStops();
    void loadSettings();
    void saveSettings();
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void reopenMidiInputs();
    juce::AudioDeviceSelectorComponent* getAudioSettingsForManual(int manualIndex);
    juce::AudioDeviceSelectorComponent* getAudioSettingsForPerspective(int manualIndex,
        const juce::String& perspective);
    void rebuildPerspectiveAudioRoutes();
    std::vector<std::pair<int, juce::String>> missingPerspectiveRoutes() const;   // perspectives without a route yet
    // The ASIO driver a route will ask for (empty: none) and its saved state, for opening the driver as its own loading step.
    std::unique_ptr<juce::XmlElement> requestedAsioStateForRoute(int manual, const juce::String& perspective, juce::String& driverName);
    void buildPerspectiveAudioRoute(int manual, const juce::String& perspective);   // one route (its device opened)
    void removePerspectiveAudioRoutesForManual(int manualIndex);
    void installAudioTypeInterceptor(juce::AudioDeviceSelectorComponent&, int manualIndex,
        const juce::String& perspective);
    void switchRouteAudioType(int manualIndex, const juce::String& perspective,
        const juce::String& typeName);
    static std::string perspectiveRouteKey(int manualIndex, const juce::String& perspective);

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

    // ========== PŘIDANÁ METODA ==========
    void editOneShotPatterns();
    // ========== KONEC PŘIDANÉ ČÁSTI ==========

    // ========== PŘIDANÁ METODA PRO IGNORED PATTERNS ==========
    void editFolderPatterns(bool perspectives);
    // ========== KONEC PŘIDANÉ ČÁSTI ==========

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
