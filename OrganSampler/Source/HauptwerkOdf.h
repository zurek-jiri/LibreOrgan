#pragma once

#include <JuceHeader.h>
#include <map>
#include <set>
#include <vector>
#include <functional>

//==============================================================================
// Hauptwerk organ definition files (ODF, "<name>.Organ_Hauptwerk_xml"): read
// for what LibreOrgan Sampler needs and nothing more - the divisions with their
// keyboards, the stops with their ranks, the ranks' sample files (which give
// the folder trees, the stop folders and the perspectives) and the per-pipe
// level adjustments (the voicing). Couplers, switches, displays, tremulants,
// wind, combinations and everything else of the ODF is ignored: in
// LibreOrgan Sampler the folder structure decides the organ.
//
// An ODF is an XML of tables: <ObjectList ObjectType="Stop"> holding one row
// per record, each a flat list of <Field>value</Field> pairs. A compacted ODF
// (the usual form shipped with sample sets) names the rows <o> and the fields
// with letters (<a>, <b>, ... <a1>); the letters below are the ones Hauptwerk
// 4-8 write (the same table the OdfEdit tool and the Masterpiece project use).
// Both forms are read; a field at its default is simply absent.
//
// The sample files: Sample records name a file relative to an installation
// package ("pipe/UW/UW_Quintadena8/03-rearMk4-UW_Quintadena8/036-C.wav" in
// package 3136); the packages live in OrganInstallationPackages/<id as six
// digits> beside the OrganDefinitions folder - the Hauptwerk standard layout.
// Encrypted samples (.hbw/.hbx) cannot be played and make the import refuse.
// Tremmed samples (alternate "tremmed" ranks, the "trem" folders, further
// pipe layers) are left out: LibreOrgan Sampler has its own tremulant.
struct HauptwerkOdf
{
    struct Pipe
    {
        int id = 0, rankId = 0, note = 60;
        float levelDb = 0.0f;           // Pipe_SoundEngine01_Layer AmpLvl_LevelAdjustDecibels of the pipe's first layer
        int packageId = 0;
        juce::String file;              // the attack sample, relative to the package ("pipe/UW/.../036-C.wav")
    };
    struct Rank
    {
        int id = 0;
        juce::String name;
        bool tremmed = false;           // an alternate (tremulant) rank, or named "(tremmed)"
        bool oneShot = false;           // its pipes have no release samples at all (chimes, harp, carillon): a one-shot stop
        bool noise = false;             // key/stop action noises, blower, tremulant noise: no stop of ours
        juce::String perspective;       // "Direct", "Diffuse", "Rear", "Front" ... from the name; empty when the name says nothing
        std::vector<Pipe> pipes;        // the first layer's attack sample of every pipe (tremmed layers and "trem" folders excluded)
        int sampleOffset = 0;           // the files' numbers minus the pipes' notes, the most common value (Doesburg: -12 - "024" for note 36; 0.8.155)
        juce::File folder;              // the folder most of the pipes lie in (absolute, once the packages are resolved)
        int packageId = 0;              // the installation package that folder lies in
        juce::String folderRelative;    // the folder relative to its package ("pipe/PE/PEd_Princip16/01-dirMk2H-PEd_Princip16")
    };
    struct Stop
    {
        int id = 0, divisionId = 0;
        int switchId = 0;               // the switch that draws it (ControllingSwitchID): what combinations set
        juce::String name;
        std::vector<int> rankIds;       // the ranks of its StopRank rows (alternate ranks excluded), in file order
        // How each StopRank row maps the keys to the rank: the first key mapped,
        // how many keys, and the semitones from the key to the pipe (12 = a 4'
        // stop playing an 8' rank an octave up - an extension; 0 = the rank's own).
        struct RankUse { int rankId = 0, firstNote = 0, count = 0, increment = 0; };
        std::vector<RankUse> rankUses;
    };
    struct Division { int id = 0; juce::String name; };
    // A coupler of the definition (a KeyAction row): the keys of a keyboard play
    // a destination keyboard or a division directly, shifted by 'shift'
    // semitones, while the condition switch (the coupler's drawstop) is on;
    // melody = the top note only (Billerbeck "ii-ped cantus firmus").
    struct Coupler { int sourceKeyboardId = 0, destKeyboardId = 0, destDivisionId = 0, switchId = 0, shift = 0; bool melody = false, bass = false, unisonOff = false; juce::String name, switchName; };   // bass: the lowest key only (Doesburg "bass_1"); unisonOff: a keyboard's own division reached only under a condition switch (Ashton "1. unison off", 0.8.144)
    std::vector<Coupler> couplers;                  // in file order
    // The drawknobs on the definition's jamb page (0.8.142): the display page
    // holding the most stops (Ashton: "Simple Jamb", every division a band of
    // columns), each stop's pixel position - the plan turns them into grid cells.
    struct StopPosition { int x = 0, y = 0; };
    std::map<int, StopPosition> stopPositions;      // ODF stop id -> position on that page
    juce::String jambPageName;
    // A keyboard and the divisions its keys play: every division whose inputs
    // the key switches reach through the switch linkages, with or without a
    // condition switch (Caen: the Pedal keyboard plays "Pedal" always and
    // "PedalAnches" when the reeds ventil is drawn - both are its divisions,
    // for LibreOrgan Sampler a division IS a keyboard). divisionId is the primary one
    // (the ODF's hint when it is among them), divisionIds all of them, the
    // primary first.
    struct Keyboard
    {
        int id = 0, divisionId = 0, lowNote = 0, highNote = 0; juce::String name;
        std::vector<int> divisionIds;
        std::set<int> unconditionalDivisionIds;   // the divisions its keys reach without a condition switch (a division no keyboard reaches so is a floating one)
    };

    // A tremulant of the definition: its speed (Hauptwerk gives a range the
    // speed wanders in; the middle is taken) and the divisions whose pipes it
    // modulates (Tremulant > TremulantWaveform > TremulantWaveformPipe > pipe >
    // rank > the stop playing it > its division).
    struct Tremulant { int id = 0; juce::String name; float rateHz = 0.0f; std::set<int> divisionIds; };
    std::vector<Tremulant> tremulants;
    // The general crescendo of the definition: Hauptwerk keeps it as one
    // combination per stage ("Cresc.Comb.1" ... "Cresc.Comb.64"), each listing
    // the switches it sets; the stop switches are found through the switch
    // linkages (the stages set intermediate "CrescendoCombSwitch" switches).
    // A stage = the ODF stops drawn at it (couplers are left out).
    struct CrescendoStep { int stage = 0; std::vector<int> stopIds; std::vector<int> couplerSwitchIds; };   // couplerSwitchIds: the coupler switches the stage draws (0.8.138)
    std::vector<CrescendoStep> crescendo;        // by stage

    juce::String organName, builder, location, supplier;
    int mainPackageId = 0;
    std::vector<int> packageIds;                    // RequiredInstallationPackage, the main one included
    std::map<int, Division> divisions;
    std::vector<Keyboard> keyboards;                // in file order
    std::map<int, Stop> stops;
    std::map<int, Rank> ranks;
    int sampleCount = 0, encryptedSamples = 0;      // of the samples the pipes use
    bool compacted = false;
    juce::String error;                             // set when the file is not an ODF or holds no playable stop

    // The file looks like a Hauptwerk ODF: by its name, or by "<Hauptwerk" near the start.
    static bool looksLikeOdf(const juce::File& file);
    // Parses the file (seconds for a large one; progress 0..1, return false to cancel).
    static HauptwerkOdf parse(const juce::File& file, const std::function<bool(double)>& progress = nullptr);

    // The Hauptwerk root (the folder holding OrganDefinitions and
    // OrganInstallationPackages) for an ODF at this path, or an empty file.
    static juce::File hauptwerkRootFor(const juce::File& odf);
    // The OrganLibreOrgans folder for a root: outside the Hauptwerk folders (0.8.158).
    static juce::File configurationFolderFor(const juce::File& hauptwerkRoot);
    static juce::File packageFolder(const juce::File& hauptwerkRoot, int packageId);
    static juce::String spelledAsOnDisk(const juce::File& base, const juce::String& relative);   // the parts as the disk spells them (0.8.149)
    // The perspective a rank name tells ("Front A (direct): HW. Octave 2" -> "Direct",
    // "Rear: UW. Nasard 3" -> "Rear"); empty when none.
    static juce::String perspectiveOfRankName(const juce::String& name);

    //==========================================================================
    // What LibreOrgan Sampler makes of it: divisions in keyboard order (the pedal
    // first), each with its folder trees, the stops with their stop folders,
    // the perspective patterns, and the voicing per pipe.
    struct PlanStop
    {
        int stopId = 0;                              // the ODF stop
        juce::String name;
        std::vector<juce::File> folders;             // the stop folder(s): one per installation package the stop's ranks lie in
        std::vector<int> rankIds;
        // An extension or a transmission: the stop plays the ranks of another
        // stop (its owner, by ODF stop id) shifted by 'shift' semitones (an ODF
        // stop whose StopRank rows all borrow one other stop's ranks with the
        // same transposition). It has no folders of its own; LibreOrgan Sampler makes
        // a transmission of the owner into this division with that shift.
        int extensionOfStopId = 0;
        int shift = 0;
        // The keys the stop plays by ITS OWN StopRank rows (0.8.159): a rank folder
        // used by a bass stop and a treble stop is loaded whole, and the keys outside
        // each stop's rows are switched off in its voicing (-1: every key).
        int lowKey = -1, highKey = -1;
        // The sample files' numbers minus the keys they play on, from the ranks'
        // pipes (0.8.155): the Stop List row's "Files vs keys", the loader puts the
        // files on their keys (Doesburg: -12, before the organ sounded an octave high).
        int sampleOffset = 0;
        // The cell on our grid from the jamb page (0.8.142): the rank of the
        // drawknob's x among the division's columns and of its y among its rows;
        // -1 = none (no page, or a layout too wide or too tall to copy).
        int gridColumn = -1, gridRow = -1;
        // A stop with several ranks in ONE perspective (Caen: "Quintaton 16 (L
        // component)" and "(R component)", two folders side by side for the
        // same perspective) becomes a stop folder: this row has no folders of
        // its own, its children (one per rank group, the groups matched across
        // the perspectives by the rank name without its perspective head) carry
        // them. Ranks of different perspectives stay ONE stop with one folder
        // per perspective, as ever.
        std::vector<PlanStop> children;
    };
    struct PlanDivision
    {
        juce::String name;                           // the ODF's division name
        int divisionId = 0, lowNote = 36, highNote = 96;
        std::vector<int> divisionIds;                // every ODF division the keyboard plays (the primary first)
        float tremulantRateHz = 0.0f;                // the speed of the definition's tremulant on this division (0: none)
        std::vector<juce::File> trees;               // the folders holding the stop folders, the main one first
        std::vector<PlanStop> stops;
    };
    struct Plan
    {
        juce::String organName;
        std::vector<PlanDivision> divisions;         // [0] = pedal (when the organ has one), then the manuals
        std::vector<juce::String> perspectivePatterns;   // "Direct;01-dirMk2H-" ... (empty: a single perspective)
        std::vector<CrescendoStep> crescendo;        // the definition's crescendo stages (ODF stop ids drawn at each)
        std::vector<juce::File> allTrees;
        juce::File libraryRoot;                      // OrganInstallationPackages
        juce::File configurationFile;                // where the configuration goes: the main package folder
        int stopsWithoutSamples = 0, skippedTremmedRanks = 0, skippedNoiseRanks = 0, extensions = 0;
        std::vector<juce::String> oneShotFolders;    // stop folder names of the one-shot stops (no release samples): for the One-shot Patterns
        // The couplers of the definition LibreOrgan Sampler has a coupler for (0.8.138):
        // id = our coupler id ("target-source-type", MainComponent::rebuildCouplerCatalog),
        // label = the definition's name, switchId = its drawstop (the crescendo stages name it).
        struct PlanCoupler { juce::String id, label; int switchId = 0; };
        std::vector<PlanCoupler> couplers;
        int couplersUnmapped = 0;                    // couplers of the definition with no coupler of ours (the notes say which)
        juce::StringArray notes;                     // what was left out and why (for the log and the message)
        juce::String error;
    };
    Plan plan(const juce::File& hauptwerkRoot) const;

    // The voicing of a loaded stop: for every note the mean of the level
    // adjustments of the pipes the given rank folders hold (one per perspective).
    // Returns the notes found and their dB; the caller takes the mean as the
    // stop's volume and the rest as per-key voicing.
    std::map<int, float> levelsForFolders(const std::vector<juce::File>& rankFolders) const;
    // The keys the definition lets a stop play, from the StopRank rows of the
    // stop(s) playing these rank folders straight (a Cornet from c1 up:
    // first key 60, 37 keys; a stop built of several pieces is the union of
    // them). false when the definition says nothing (the whole compass).
    bool compassForFolders(const std::vector<juce::File>& rankFolders, int divisionLow, int divisionHigh, int& low, int& high) const;
};
