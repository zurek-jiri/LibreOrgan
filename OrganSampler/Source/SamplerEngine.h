#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <bitset>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

//==============================================================================
// Massive-polyphony organ sample engine.
//
// Threading model (the reason this engine scales to the whole machine):
//
//  * The GUI/MIDI threads only ever push small commands (note on/off) into a
//    lock-free ring, or write atomics (gains, tuning, routing masks).
//  * Every audio device callback belongs to exactly one "route"
//    (division + perspective).  It renders only that route's voices.
//  * The first callback of a period also runs the "control tick" (MIDI, pending
//    attacks/releases, wind model) under a lock taken with tryEnter(), so no
//    audio thread ever blocks on another one or on the GUI.
//  * Rendering is spread over a persistent, real-time-priority worker pool that
//    all device callbacks share.  Each lane renders into its private scratch;
//    the callback sums the scratches.
//  * Voices live in one preallocated pool.  Control and render threads talk
//    through atomics and per-route single-producer/single-consumer rings, so
//    the real-time path never allocates and never touches a std::map.
//  * All modulators (wind, tremulant, swell, stop volume) are evaluated once
//    per route per block and linearly interpolated inside the block.  Nothing
//    is stepped per sample, so no zipper noise, and the LFO phase advances in
//    sample time (no wall-clock jitter "humps").
//==============================================================================

#include <limits>
#include <map>
#include <mutex>
#include <set>

struct LoopPoints {
    int64_t start = 0;
    int64_t end = 0;
};

// Contents of a WAV "smpl" chunk that matter here.
struct SmplInfo
{
    std::vector<LoopPoints> loops;
    float pitchSemitones = std::numeric_limits<float>::quiet_NaN();   // unity note + fraction, NaN if absent
};

struct Sample
{
    static constexpr int maxLoops = 64;

    double sampleRate = 44100.0;         // the rate the frames are stored at (the base rate after resampling)
    double sourceSampleRate = 0.0;       // the rate of the file (differs when the sample was resampled on loading)
    int numChannels = 0;
    int64_t numFrames = 0;
    int64_t stride = 0;                  // frames per channel, includes guard frames
    bool storesFloat = false;
    juce::HeapBlock<int16_t, true> pcm16;   // planar, used when !storesFloat (throws std::bad_alloc)
    juce::HeapBlock<float, true> pcm32;     // planar, used when storesFloat

    std::vector<LoopPoints> loops;       // clamped so that frame end+1 is always readable
    std::vector<int> loopOrderByEnd;     // indices into loops sorted by end
    std::vector<int64_t> crossfadeFrames; // per loop: length of the splice before loop.end
    // Per loop: the splice runs AFTER the jump (from loop.start, blending the
    // continuation past loop.end into the loop) instead of before the end. Used
    // for long splices: the head before the start would reach into the attack.
    std::vector<uint8_t> crossfadeAfterJump;
    int64_t maxLoopEnd = 0;
    // Recorded pitch from the WAV "smpl" chunk (MIDI unity note + fraction) in
    // MIDI semitones on the equal-tempered a1 = 440 Hz scale; NaN when absent.
    float smplPitch = std::numeric_limits<float>::quiet_NaN();
    // A pitch-shifted copy: the "formants follow" share it was built with, in
    // per cent (-1 = not a pitch-shifted copy). Lets a build and the cache tell
    // a copy made for another setting of its stop.
    int formantPct = -1;
    // A release copy: the gain that puts its start at the level relation the
    // source's release has to the source's sustain (measured when built; applied
    // at playback when "release level matched" is on).
    float releaseLevelGain = 1.0f;
    // A sample stored at half the route's rate whose first second carries
    // treble above a quarter of its own rate (0.8.149): played through the sinc
    // kernel for its first 1.3 s; a dull one (nothing above 6 kHz at -40 dB)
    // has nothing to mirror and keeps the cheap linear kernel.
    bool sincHead = false;
    juce::String fileName;               // the file the sample was read from (its name only; copies carry their source's)

    Sample(int channels, int64_t frames, double sr, bool asFloat);
    void finaliseLoops(std::vector<LoopPoints> lp);

    const int16_t* channel16(int c) const noexcept { return pcm16.get() + (size_t)c * (size_t)stride; }
    const float* channel32(int c) const noexcept { return pcm32.get() + (size_t)c * (size_t)stride; }
    int16_t* writable16(int c) noexcept { return pcm16.get() + (size_t)c * (size_t)stride; }
    float* writable32(int c) noexcept { return pcm32.get() + (size_t)c * (size_t)stride; }

    // Lossless packing of 16-bit samples in RAM (0.8.145, optional, off by
    // default): FLAC-like fixed predictors (order 0, 1 or 2) per block of
    // kPackBlock frames, the residuals stored with the bit width the block
    // needs. Loops, pitch and everything else stay as they are; the player
    // unpacks the window it reads before every span (SamplerEngine.cpp,
    // renderVoiceSpans) - several times the per-voice cost, hence the option.
    static constexpr int kPackBlock = 256;
    struct PackedChannel
    {
        std::vector<uint8_t> data;
        std::vector<uint32_t> blockOffsets;   // byte offset of every block in data (+ one past the end)
    };
    std::vector<PackedChannel> packed;
    bool compressed = false;                  // pcm16 freed, the frames live in packed[]
    // Packs pcm16 and frees it; returns the packed bytes of all channels. Checks
    // every frame against the original first - a mismatch keeps the raw data
    // (returns 0). Float samples and tiny ones are left alone (returns 0).
    size_t compress16();
    // Frames [from, to) of a channel as plain 16-bit; zeros outside 0..numFrames
    // (the guard frames the kernels may read past the end). Works on packed and
    // raw samples alike. Real-time safe: no allocation.
    void decode16(int c, int64_t from, int64_t to, int16_t* out) const noexcept;
    // A whole channel as plain 16-bit (stride frames, the guard zeroed): the pcm16
    // pointer of a raw sample, else decoded into the scratch given. For the readers
    // outside the render path (copy builds, cache files, analyses, the self-test).
    const int16_t* read16(int c, std::vector<int16_t>& scratch) const;
    // Float samples (0.8.183): packed as the integers the file held - the 24-bit
    // (or 16-bit) values a float holds exactly, found by trying the two scales and
    // checked frame by frame. Real floats (a 32-bit float WAV, a resampled sample,
    // a filled pipe's copy) are rounded to 24 bits first (packRounded); only a
    // sample beyond +-2 full scale stays raw (returns 0). packScale turns the
    // stored integers back into the floats: f = int x packScale.
    float packScale = 0.0f;
    bool packRounded = false;
    size_t compress32();
    void decode32(int c, int64_t from, int64_t to, float* out) const noexcept;
    const float* read32(int c, std::vector<float>& scratch) const;
    // Bytes the frames take in RAM now (packed or raw, every channel).
    size_t frameBytes() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sample)
};

// A stop name for display: a bare footage number at the end of the folder name
// gets its foot mark ("Fletna 8" -> "Fletna 8'"); names with the mark are unchanged.
juce::String footageDisplayName(const juce::String& name);
// "I", "II", "III", "IV" ... (the number of ranks in a stop folder's caption).
juce::String romanNumeral(int n);

struct Stop
{
    juce::String name;      // from the folder: the stop's identity in every saved state
    juce::String alias;     // shown instead of the name when set (voicing "Alias")
    bool showRankCount = true;   // a stop folder's button shows its rank count "[II]" (voicing, 0.8.146)
    // The stop's TRANSPOSITION (0.8.158; "Files vs keys" in 0.8.155-0.8.157): the
    // sample files' numbers minus the keys they land on, semitones. The files
    // placed by their numbers are "the original" - they sound at the footage the
    // smpl pitches say; -12 puts the file 024 on key 36, an octave down (a 16' of
    // files that sound 8' on their own numbers - Doesburg), +12 an octave up.
    // From a Hauptwerk definition (its pipe notes against the file names), else
    // found by the loader from the smpl pitches against the footage in the
    // folder's name, else the voicing's Transposition box / the Stop List column
    // "Transp.". Transmissions and extensions add their own shift on top.
    int sampleOffset = 0;
    bool sampleOffsetDetected = false;   // set by the loader when it found the offset itself (the Stop List row takes it)
    bool sampleOffsetSet = false;        // the offset is known (a definition, the user, or found already): the loader leaves it
    int keyShift() const { return sampleOffset; }   // file number - the key the file lands on
    // The alias, else the name with its foot mark; when neither carries a
    // footage, the footage measured from the smpl pitch of the lowest pipe is appended.
    juce::String displayName() const;
    // Stop folders (Stop List): a stop indented under another one sounds with
    // it - the master's button switches its children too; the children have no
    // grid cell of their own and appear under the master's opened voicing.
    juce::String groupMaster;   // name of the master stop of the same division (empty = none)
    // The master as a pointer (set under the control lock): a child also sounds
    // through every transmission of its master, so a stop folder is transmitted
    // as a whole. A master may be an empty "container" stop without samples.
    Stop* groupMasterPtr = nullptr;
    int groupChildOrdinal = 0;  // a stop folder's child: its number among the children (1, 2 ...), for the tremulant phase drift; 0 otherwise
    bool container = false;     // a stop folder's master without samples of its own (only its children sound)
    juce::File folder;
    juce::File tree;   // the folder tree this stop was loaded from
    // The storage the samples were decoded with (SamplerEngine::storageSignature
    // at the time: float/16-bit, rate, halved releases, packing; 0.8.149). A reload
    // adopts loaded samples only with the same signature - a reload with other
    // choices in the Load box kept the packed, halved samples of the load before
    // (#62: "16-bit, 32 kHz" runs measured the packed organ).
    juce::String storageSignature;

    // Attack/sustain samples per MIDI note (round robin between variants).
    std::array<std::vector<std::unique_ptr<Sample>>, 128> samplesByNote;
    std::array<uint8_t, 128> nextSampleIndex{};

    bool hasNote(int note) const noexcept
    {
        return note >= 0 && note < 128 && !samplesByNote[(size_t)note].empty();
    }
    // Includes the notes of all perspective ranks belonging to this stop.
    bool hasNoteInAnyRank(int note) const noexcept
    {
        if (hasNote(note)) return true;
        for (const auto& rank : ranks) if (rank->hasNote(note)) return true;
        return false;
    }
    std::vector<int> notesWithSamples() const
    {
        std::vector<int> result;
        for (int note = 0; note < 128; ++note) if (hasNote(note)) result.push_back(note);
        return result;
    }
    const Sample* getNextSample(int note) noexcept
    {
        return pickRoundRobin(samplesByNote, nextSampleIndex, note);
    }

    // Velocity layers: a file named "060_070.wav" (or "060-C4_070.wav") plays for
    // key velocities up to 70; above that the plain "060.wav" plays (or the next
    // layer up). Several layers may exist (_025, _070, _100 ...). A key that has
    // layers but no plain file plays its highest layer above it. The voicing can
    // switch a layer off (velocityLayerDisabled, kept on the logical stop): its
    // velocities then use the next layer up - or the plain samples.
    struct VelocityLayer
    {
        VelocityLayer() = default;
        VelocityLayer(const VelocityLayer&) = delete;
        VelocityLayer& operator=(const VelocityLayer&) = delete;
        VelocityLayer(VelocityLayer&&) noexcept = default;
        VelocityLayer& operator=(VelocityLayer&&) noexcept = default;

        int maxVelocity = 127;
        std::array<std::vector<std::unique_ptr<Sample>>, 128> samplesByNote;
        std::array<uint8_t, 128> nextSampleIndex{};
        // Anti-alias copies of this layer's samples for filled pipes (like the
        // plain samples' aliasFree below): every layer gets its own, so a filled
        // pipe plays the right layer at every velocity.
        std::map<int, std::vector<std::unique_ptr<Sample>>> aliasFree;
        std::map<int, int> aliasFreeOffset;
        const Sample* nextAliasFree(int pipe) noexcept
        {
            const auto it = aliasFree.find(pipe);
            if (it == aliasFree.end() || it->second.empty()) return nullptr;
            int next = nextSampleIndex[(size_t)juce::jlimit(0, 127, pipe)];
            if (next < 0 || next >= (int)it->second.size()) next = 0;
            nextSampleIndex[(size_t)juce::jlimit(0, 127, pipe)] = (uint8_t)((next + 1) % (int)it->second.size());
            return it->second[(size_t)next].get();
        }
    };
    std::vector<VelocityLayer> velocityLayers;   // ascending by maxVelocity
    // Layers switched off in the voicing, by maxVelocity (logical stop; ranks
    // follow it). Bit 'plainLayerBit' (0, no file layer can have it) stands for the
    // plain samples, the top layer "_128": switched off, the velocities above the
    // highest file layer play the highest enabled layer instead.
    static constexpr size_t plainLayerBit = 0;
    std::bitset<128> velocityLayerDisabled;
    bool plainLayerDisabled() const noexcept { return velocityLayerDisabled.test(plainLayerBit); }
    // The layer that plays 'note' at 'velocity': the lowest enabled layer covering
    // the velocity; above every layer the plain samples (nullptr) - unless they are
    // switched off, then the highest enabled layer that has the note. Nothing
    // enabled at all: the plain samples, so a key never falls silent.
    VelocityLayer* velocityLayerFor(int note, int velocity, const std::bitset<128>& disabled) noexcept
    {
        if (note < 0 || note >= 128) return nullptr;
        for (auto& layer : velocityLayers)
        {
            if (velocity > layer.maxVelocity || disabled.test((size_t)juce::jlimit(0, 127, layer.maxVelocity))) continue;
            if (!layer.samplesByNote[(size_t)note].empty()) return &layer;
        }
        if (!disabled.test(plainLayerBit)) return nullptr;   // the plain samples
        for (auto it = velocityLayers.rbegin(); it != velocityLayers.rend(); ++it)
            if (!disabled.test((size_t)juce::jlimit(0, 127, it->maxVelocity)) && !it->samplesByNote[(size_t)note].empty()) return &*it;
        return nullptr;
    }
    VelocityLayer& getOrCreateVelocityLayer(int maxVelocity)
    {
        for (auto& layer : velocityLayers)
            if (layer.maxVelocity == maxVelocity) return layer;
        velocityLayers.push_back(VelocityLayer{});
        velocityLayers.back().maxVelocity = maxVelocity;
        return velocityLayers.back();
    }
    void sortVelocityLayers()
    {
        std::sort(velocityLayers.begin(), velocityLayers.end(),
            [](const VelocityLayer& a, const VelocityLayer& b) { return a.maxVelocity < b.maxVelocity; });
    }
    // The maxVelocity of every layer this stop and its ranks have, ascending.
    std::vector<int> velocityLayerValues() const
    {
        std::vector<int> values;
        auto collect = [&values](const Stop* rank)
        {
            for (const auto& layer : rank->velocityLayers)
                if (std::find(values.begin(), values.end(), layer.maxVelocity) == values.end()) values.push_back(layer.maxVelocity);
        };
        collect(this);
        for (const auto& rank : ranks) collect(rank.get());
        std::sort(values.begin(), values.end());
        return values;
    }

    // Release layers: folder relNNNNN holds releases for keys held up to NNNNN ms.
    // Release files carry velocity layers like the attacks ("060_070.wav" in the
    // rel folder: the release of the _070 attack); maxVelocity -1 = the plain release.
    struct ReleaseLayer
    {
        ReleaseLayer() = default;
        ReleaseLayer(const ReleaseLayer&) = delete;
        ReleaseLayer& operator=(const ReleaseLayer&) = delete;
        ReleaseLayer(ReleaseLayer&&) noexcept = default;
        ReleaseLayer& operator=(ReleaseLayer&&) noexcept = default;

        int maxHoldMs = 99999;
        int maxVelocity = -1;
        std::array<std::vector<std::unique_ptr<Sample>>, 128> samplesByNote;
        std::array<uint8_t, 128> nextSampleIndex{};
        // Pitch-shifted copies for filled pipes (transposition method "pitch shift,
        // length kept"), keyed by the filled pipe like the attacks' aliasFree.
        std::map<int, std::vector<std::unique_ptr<Sample>>> aliasFree;
        std::map<int, int> aliasFreeOffset;
        const Sample* nextAliasFree(int pipe) noexcept
        {
            const auto it = aliasFree.find(pipe);
            if (it == aliasFree.end() || it->second.empty()) return nullptr;
            int next = nextSampleIndex[(size_t)juce::jlimit(0, 127, pipe)];
            if (next < 0 || next >= (int)it->second.size()) next = 0;
            nextSampleIndex[(size_t)juce::jlimit(0, 127, pipe)] = (uint8_t)((next + 1) % (int)it->second.size());
            return it->second[(size_t)next].get();
        }
    };
    std::vector<ReleaseLayer> releaseLayers;   // ascending by maxHoldMs, then maxVelocity (plain last)

    ReleaseLayer& getOrCreateReleaseLayer(int maxHoldMs, int maxVelocity = -1)
    {
        for (auto& layer : releaseLayers)
            if (layer.maxHoldMs == maxHoldMs && layer.maxVelocity == maxVelocity) return layer;
        releaseLayers.push_back(ReleaseLayer{});
        releaseLayers.back().maxHoldMs = maxHoldMs;
        releaseLayers.back().maxVelocity = maxVelocity;
        return releaseLayers.back();
    }
    void sortReleaseLayers()
    {
        // The plain release (-1) sorts after every velocity layer of the same hold time.
        auto velocityRank = [](const ReleaseLayer& l) { return l.maxVelocity < 0 ? 128 : l.maxVelocity; };
        std::sort(releaseLayers.begin(), releaseLayers.end(),
            [&](const ReleaseLayer& a, const ReleaseLayer& b)
            { return a.maxHoldMs != b.maxHoldMs ? a.maxHoldMs < b.maxHoldMs : velocityRank(a) < velocityRank(b); });
    }
    bool hasReleaseSamples() const noexcept { return !releaseLayers.empty(); }
    // The release for a pipe held 'holdMs' and struck with 'velocity': the first
    // hold time that covers holdMs (else the longest), and within it the release of
    // the attack's own velocity layer - the lowest enabled layer covering the
    // velocity, else the plain release; when that velocity has no release of its
    // own, the nearest one that exists (the one just above, else the one below).
    // copyPipe >= 0: the pitch-shifted release copy of that filled pipe is preferred
    // (usedCopy tells whether one was found) - the transposition method "pitch shift".
    const Sample* getNextReleaseSample(int note, double holdMs, int velocity = 127,
        const std::bitset<128>* disabledVelocityLayers = nullptr, int copyPipe = -1, bool* usedCopy = nullptr) noexcept
    {
        if (usedCopy != nullptr) *usedCopy = false;
        if (releaseLayers.empty() || note < 0 || note >= 128) return nullptr;
        auto velocityRank = [](const ReleaseLayer& l) { return l.maxVelocity < 0 ? 128 : l.maxVelocity; };
        auto layerEnabled = [&](const ReleaseLayer& l)
        {
            if (disabledVelocityLayers == nullptr) return true;
            if (l.maxVelocity < 0) return !disabledVelocityLayers->test(plainLayerBit);   // the plain release follows the plain layer's switch
            return !disabledVelocityLayers->test((size_t)juce::jlimit(0, 127, l.maxVelocity));
        };
        auto pickWithin = [&](int holdMsOfGroup) -> const Sample*
        {
            ReleaseLayer* above = nullptr; ReleaseLayer* below = nullptr;   // nearest layers with this note
            for (auto& layer : releaseLayers)
            {
                if (layer.maxHoldMs != holdMsOfGroup || layer.samplesByNote[(size_t)note].empty() || !layerEnabled(layer)) continue;
                if (velocityRank(layer) >= velocity) { if (above == nullptr || velocityRank(layer) < velocityRank(*above)) above = &layer; }
                else if (below == nullptr || velocityRank(layer) > velocityRank(*below)) below = &layer;
            }
            ReleaseLayer* chosen = above != nullptr ? above : below;
            if (chosen == nullptr)   // everything switched off: the plain release rather than silence
                for (auto& layer : releaseLayers)
                    if (layer.maxHoldMs == holdMsOfGroup && layer.maxVelocity < 0 && !layer.samplesByNote[(size_t)note].empty()) chosen = &layer;
            if (chosen == nullptr) return nullptr;
            if (copyPipe >= 0)
                if (const Sample* copy = chosen->nextAliasFree(copyPipe))
                {
                    if (usedCopy != nullptr) *usedCopy = true;
                    return copy;
                }
            return pickRoundRobin(chosen->samplesByNote, chosen->nextSampleIndex, note);
        };
        for (auto& layer : releaseLayers)
            if (holdMs <= (double)layer.maxHoldMs)
                if (const Sample* s = pickWithin(layer.maxHoldMs)) return s;
        return pickWithin(releaseLayers.back().maxHoldMs);
    }

    // --- Voicing (GUI writes, audio reads atomics) ---
    float db = 0.0f;
    std::atomic<float> linearGain{ 1.0f };
    bool enabled = true;
    float gainDb = 0.0f;          // attack boost, -6..6 dB
    float attackMs = 75.0f;       // the attack gain holds this long (the voicing's "Attack Time" = this + attackFadeoutMs, half each; 0..500 ms, default 150)
    float attackFadeoutMs = 75.0f; // ... and then fades back to the normal level over this long
    float randomPitchErrorCents = 4.0f;
    // Pitch shift of this stop's filled pipes: how far the spectral envelope
    // follows the shift (0..1) for shifts downwards; upwards 0.3 more. Per stop
    // (every stop wants its own); a transmission has its own for the pipes
    // only it asks for.
    float formantFollow = 0.3f;
    std::array<float, 128> randomPitchByNote{};
    std::array<float, 128> perNoteDb{};
    std::array<std::atomic<float>, 128> perNoteGain;
    // Per-key tone ("Treble per key"): a high shelf of this many dB (-9..9) above
    // the tone start frequency (SamplerEngine::setToneStartHz, or the pipe's own
    // fundamental when that is higher). Indexed by the pipe like perNoteDb.
    std::array<float, 128> perNoteToneDb{};
    std::array<std::atomic<float>, 128> perNoteTone;

    bool isOneShot = false;                // plays each sample once through, whenever the key is released (chimes, harp)
    std::atomic<bool> reloading{ false };  // its samples are being decoded anew (a transposition change): no voice starts on it meanwhile (0.8.160)
    int oneShotManual = -1;                // the voicing's own One-shot switch: -1 = by the One-shot Patterns, 0 off, 1 on (saved)
    int primaryManual = 0;                 // home division: P., I., II., III., IV., V.
    // Transmissions: additional stop buttons (in any division, the home division
    // included) that play this stop's pipes, optionally transposed. A shift of
    // -12 turns an 8' stop into a 16' extension: the key sounds the pipe an
    // octave below. Shifts follow the harmonic series (pure intervals), so a
    // quint shifted by +5 gives an exact 2'. Keys whose pipe lies beyond the
    // rank's compass use its outermost pipe, resampled.
    struct Transmission
    {
        static constexpr int maxShift = 24;
        int manual = 0;                    // receiving division
        int shift = 0;                     // semitones the pipes are shifted (-24..24)
        float db = 0.0f;                   // volume relative to the parent stop (-10..10 dB)
        bool enabled = false;              // the borrowed stop button is on
        std::array<float, 128> keyDb{};    // per-key voicing of this transmission (dB, indexed by the key played)
        std::array<float, 128> keyTone{};  // per-key tone (high shelf dB, -9..9) of this transmission, by key played
        juce::String alias;                // shown instead of the automatic caption when set
        bool voicingExpanded = false;      // its voicing row is open (not saved)
        bool volumeKeysExpanded = false, toneKeysExpanded = false;   // its per-key rows unfolded
        std::bitset<128> keyDisabled;      // keys switched off in this transmission's voicing (by key played; independent of the stop's pipes)
        juce::String groupMaster;          // a stop folder of the receiving division this transmission is indented under (switches with it)
        float formantFollow = 0.3f;        // formants follow the pitch shift (0..1, downwards; upwards 0.3 more) of the pipes only this transmission asks for
    };
    std::vector<Transmission> transmissions;
    int transmissionCount(int manual) const noexcept
    {
        int count = 0;
        for (const auto& t : transmissions) if (t.manual == manual) ++count;
        return count;
    }
    // Index in 'transmissions' of the ordinal-th transmission into 'manual' (-1 if none).
    int transmissionIndex(int manual, int ordinal) const noexcept
    {
        for (int i = 0; i < (int)transmissions.size(); ++i)
            if (transmissions[(size_t)i].manual == manual && ordinal-- == 0) return i;
        return -1;
    }
    // Position of transmission 'index' among those into the same division.
    int transmissionOrdinal(int index) const noexcept
    {
        if (index < 0 || index >= (int)transmissions.size()) return -1;
        int ordinal = 0;
        for (int i = 0; i < index; ++i)
            if (transmissions[(size_t)i].manual == transmissions[(size_t)index].manual) ++ordinal;
        return ordinal;
    }
    bool hasTransmissionInto(int manual) const noexcept { return transmissionCount(manual) > 0; }
    bool anyTransmissionEnabled() const noexcept
    {
        for (const auto& t : transmissions) if (t.enabled) return true;
        return false;
    }
    // Pipes the rank does not have (beyond its compass, gaps inside it) are
    // filled from neighbouring pipes, transposed: pipe n plays the sample of
    // n + fillOffset[n]. See SamplerEngine::computeFilledPipes.
    std::bitset<128> filled;
    std::array<int8_t, 128> fillOffset{};
    // Pipes whose recorded pitch lies beyond the retune limit (0.8.165): a faulty
    // sample (Adlington's 035 and 037 of UW_Flute4 are a third low) - never
    // played, filled from the neighbours like a missing pipe. rawTuningCents keeps
    // the unclamped deviation of every measured note, so a new limit can be
    // applied without the analysis.
    std::bitset<128> faultyPipes;
    std::array<float, 128> rawTuningCents{};
    // Pipes switched off in the voicing (checkbox under the key): never played,
    // never filled, an extension's anti-alias copies freed (a cornet must not
    // grow an extension). Kept on the logical stop; ranks follow it.
    std::bitset<128> pipeDisabled;
    bool isPipeDisabled(int pipe) const noexcept { return pipe >= 0 && pipe < 128 && pipeDisabled.test((size_t)pipe); }
    bool isFilled(int note) const noexcept { return note >= 0 && note < 128 && filled.test((size_t)note); }
    bool isFilledInAnyRank(int note) const noexcept
    {
        if (isFilled(note)) return true;
        for (const auto& rank : ranks) if (rank->isFilled(note)) return true;
        return false;
    }
    // The pipe whose samples play for 'note': the note itself, its fill source, or -1.
    int sourceNoteFor(int note) const noexcept
    {
        // A faulty pipe (0.8.166) has a sample of its own but plays its neighbour's.
        if (hasNote(note) && !(note >= 0 && note < 128 && faultyPipes.test((size_t)note))) return note;
        if (isFilled(note)) return note + fillOffset[(size_t)note];
        return -1;
    }
    // Does any attack sample of this note carry a loop (smpl chunk)?
    bool noteHasLoop(int note) const noexcept
    {
        if (note < 0 || note >= 128) return false;
        for (const auto& sample : samplesByNote[(size_t)note])
            if (sample != nullptr && !sample->loops.empty()) return true;
        return false;
    }
    bool noteHasLoopInAnyRank(int note) const noexcept
    {
        if (noteHasLoop(note)) return true;
        for (const auto& rank : ranks) if (rank->noteHasLoop(note)) return true;
        return false;
    }
    // Anti-alias copies for filled pipes that play a neighbour's sample faster
    // (transposed up): the source sample low-passed so that nothing above the
    // new Nyquist limit folds back. Built on demand (SamplerEngine::
    // buildAntiAliasCopies), keyed by the filled pipe; obsolete copies are
    // retired (kept until the stop unloads, voices may still read them).
    std::map<int, std::vector<std::unique_ptr<Sample>>> aliasFree;
    std::map<int, int> aliasFreeOffset;   // pipe -> fillOffset the copies were built for
    int copiesMethod = 0;                 // the transposition method the copies were built for (0 resampling, 1 pitch shift)
    // Progress of the copies being built for this (logical) stop: the UI shows a
    // strip on the stop button while copiesBuilding (done of planned pipes).
    std::atomic<bool> copiesQueued{ false };    // a background build is launched (waiting for its turn or running)
    std::atomic<bool> copiesBuilding{ false };  // ... and running now
    std::atomic<int> copiesPlanned{ 0 }, copiesDone{ 0 };
    // The last build did not run to its end (interrupted by a message-thread
    // operation, or it gave up its turn): resumeIncompleteCopyBuilds launches it again.
    std::atomic<bool> copiesIncomplete{ false };
    std::vector<std::unique_ptr<Sample>> retiredSamples;
    const Sample* getNextAliasFreeSample(int note) noexcept
    {
        if (note < 0 || note >= 128) return nullptr;
        const auto it = aliasFree.find(note);
        if (it == aliasFree.end() || it->second.empty()) return nullptr;
        auto& variants = it->second;
        int next = nextSampleIndex[(size_t)note];   // the slot of a filled pipe is otherwise unused
        if (next < 0 || next >= (int)variants.size()) next = 0;
        nextSampleIndex[(size_t)note] = (uint8_t)((next + 1) % (int)variants.size());
        return variants[(size_t)next].get();
    }
    // Notes that can be voiced: real pipes and filled ones (any rank).
    std::vector<int> voicedNotes() const
    {
        std::vector<int> result;
        for (int note = 0; note < 128; ++note)
            if (hasNoteInAnyRank(note) || isFilledInAnyRank(note)) result.push_back(note);
        return result;
    }
    bool voicingExpanded = false;
    // The per-key rows of this stop's voicing ("Volume per key", "Treble per
    // key"): folded by default, remembered per stop in the voicing state.
    bool volumeKeysExpanded = false, toneKeysExpanded = false;
    juce::String perspective{ "default" };
    bool rankEnabled = true;
    float windContribution = 1.0f;         // precomputed from the footage in the name
    // --- Tuning to a1 = 440 Hz equal temperament (per rank, from the smpl pitches) ---
    bool tuneTo440 = false;                        // logical stop: apply the corrections below
    bool tuningInfoAvailable = false;              // this rank had usable smpl pitch data
    float measuredDiapasonHz = 0.0f;               // a1 of this rank as recorded (0 = unknown)
    std::array<float, 128> tuning440Cents{};       // per note: cents that bring it to 440 Hz ET
    // Recorded temperament of the rank: mean deviation per pitch class (C..B) from
    // equal temperament in cents, mean removed; valid with >= 8 pitch classes.
    std::array<float, 12> measuredTemperamentCents{};
    bool measuredTemperamentValid = false;
    // Undulating (celeste) rank detected from the smpl pitches: its recorded
    // detune at the a1 key expressed as beats per second (0 = not a celeste).
    float detectedCelesteBeats = 0.0f;
    // Footage of the rank from the smpl pitches: semitones the pipes sound above
    // their FILE NUMBERS - the real footage of the samples on their own numbers
    // (0 = 8', 12 = 4', 19 = 2 2/3' fifth, 28 = 1 3/5' third, ...); what sounds on
    // the keys is this plus sampleOffset (0.8.158).
    bool footageKnown = false;
    int footageSemitones = 0;
    // The footage the lowest pipes really sound at: the smpl pitch of a mixture
    // names the key's fundamental, which its ranks (2 2/3', 2', ...) do not
    // produce, so the sustain is measured for the lowest harmonic that sounds.
    int lowestFootageSemitones = 0;   // offset of that harmonic above the key (the name shows it)
    int soundingHarmonic = 1;         // harmonic number in the lowest segment (1 = the fundamental sounds)
    bool soundingMeasured = false;    // measured once per rank (the samples do not change)
    juce::String footageSounding;     // "8' (2 2/3')", with breaks "C 2' (1 1/3') | c1 4'" (empty without pitch data)
    std::array<float, 16> lowestNoteHarmonicDb{};   // harmonics 1..16 of the lowest pipe, dB below the strongest (self-test)
    float lowestNoteResidualDb = 0.0f;              // spectrum between the harmonics vs. the harmonics, dB (noise, aliasing)
    juce::String footageBreaks;   // ranks with breaks: "C 2' | c1 4' | c3 8'" (empty without breaks)
    // --- Undulation (celeste), only together with the 440 Hz tuning ---
    float undulationBeats = 0.0f;                   // beats per second at the a1 key (sign = direction)
    bool undulationManual = false;                  // set by the user (saved); otherwise the detected value rules
    std::array<float, 128> undulationCents{};       // per note, derived from undulationBeats
    // --- Stop pool: samples released from RAM while the stop is off the grid ---
    bool samplesUnloaded = false;
    // Set by the loader when the division already holds this folder with its
    // samples: nothing is decoded, the commit moves the samples, layers, copies
    // and analysis of the old rank over (a reload keeps everything expensive).
    bool adoptSamples = false;
    float tuningCentsFor(int note, bool tuned) const noexcept
    {
        if (!tuned || note < 0 || note >= 128) return 0.0f;
        return tuning440Cents[(size_t)note] + undulationCents[(size_t)note];
    }
    // Additional perspective ranks belonging to this logical stop. The Stop
    // itself is the first rank.
    std::vector<std::unique_ptr<Stop>> ranks;

    // --- Control-thread bookkeeping (engine internal) ---
    std::array<double, 128> pipeStartMs;   // per logical stop: when the pipe started speaking
    std::bitset<128> speaking;             // per rank: attack/sustain currently sounding
    std::bitset<128> releasing;            // scratch of one key-off: pipes being released now
    uint64_t shiftHistory = 1ull << Transmission::maxShift;   // bit (shift + maxShift) of every shift a voice of this stop ever started with
    int tremulantOrdinal = 0;              // position of this stop within its division
    uint32_t folderHash = 0;               // stable per-pipe wind variation seed
    std::array<int16_t, 8> routeByManual{ -1, -1, -1, -1, -1, -1, -1, -1 }; // render route per division (SamplerEngine::maxManuals)

    float getGain() const noexcept { return linearGain.load(std::memory_order_relaxed); }
    void setDb(float value) noexcept
    {
        db = value;
        linearGain.store(juce::Decibels::decibelsToGain(value), std::memory_order_relaxed);
    }
    void setNoteDb(int note, float value) noexcept
    {
        if (note < 0 || note >= 128) return;
        perNoteDb[(size_t)note] = value;
        perNoteGain[(size_t)note].store(juce::Decibels::decibelsToGain(value), std::memory_order_relaxed);
    }
    float getNoteDb(int note) const noexcept
    {
        return note >= 0 && note < 128 ? perNoteDb[(size_t)note] : 0.0f;
    }
    float getNoteGain(int note) const noexcept
    {
        return note >= 0 && note < 128 ? perNoteGain[(size_t)note].load(std::memory_order_relaxed) : 1.0f;
    }
    void resetNoteDb() noexcept
    {
        for (int note = 0; note < 128; ++note) setNoteDb(note, 0.0f);
    }
    void setNoteToneDb(int note, float value) noexcept
    {
        if (note < 0 || note >= 128) return;
        value = juce::jlimit(-9.0f, 9.0f, value);
        perNoteToneDb[(size_t)note] = value;
        perNoteTone[(size_t)note].store(value, std::memory_order_relaxed);
    }
    float getNoteToneDb(int note) const noexcept
    {
        return note >= 0 && note < 128 ? perNoteToneDb[(size_t)note] : 0.0f;
    }
    void resetNoteTone() noexcept
    {
        for (int note = 0; note < 128; ++note) setNoteToneDb(note, 0.0f);
    }

    Stop(const juce::String& n, const juce::File& f) : name(n), folder(f)
    {
        for (auto& g : perNoteGain) g.store(1.0f, std::memory_order_relaxed);
        for (auto& t : perNoteTone) t.store(0.0f, std::memory_order_relaxed);
        pipeStartMs.fill(-1.0);
    }

    static const Sample* pickRoundRobin(std::array<std::vector<std::unique_ptr<Sample>>, 128>& map,
        std::array<uint8_t, 128>& indices, int note) noexcept
    {
        if (note < 0 || note >= 128) return nullptr;
        auto& variants = map[(size_t)note];
        if (variants.empty()) return nullptr;
        const int numVariants = (int)variants.size();
        int next = indices[(size_t)note];
        if (next < 0 || next >= numVariants) next = 0;
        const Sample* chosen = variants[(size_t)next].get();
        indices[(size_t)note] = (uint8_t)((next + 1) % numVariants);
        return chosen;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Stop)
};

//==============================================================================
class SamplerEngine
{
public:
    SamplerEngine();
    ~SamplerEngine();

    // The hard polyphony ceiling (the slider's top, the measurement's headroom).
    // It sizes the voice pool (maxVoices + 8192 Voice records) and every route's
    // rings at start-up, so it costs RAM whether it is used or not: about 1.4 KB
    // per voice over the pool and the routes (420 MB at 300 000). Nothing else
    // depends on it - no per-block work grows with the pool, only with the
    // voices actually sounding - so a faster computer may use all of it.
    static constexpr int maxVoices = 500000;   // 0.8.146: 300 000 before; the pool is built at start (about 160 MB)
    static constexpr int maxManuals = 8;   // pedal + up to seven manuals

    void setSampleRate(double sr);
    double getSampleRate() const noexcept { return defaultSampleRate.load(std::memory_order_relaxed); }

    // Samples are kept as 32-bit float by default (full fidelity of 24-bit
    // sets and their release tails). 16-bit halves memory bandwidth but adds
    // quantisation noise. Call before loading; affects subsequently loaded divisions.
    void setSampleStorageFloat(bool useFloat) noexcept { storeSamplesAsFloat = useFloat; }
    bool getSampleStorageFloat() const noexcept { return storeSamplesAsFloat; }
    // Lossless packing of the 16-bit samples in RAM (0.8.145, Sample::compress16):
    // every sample decoded or copied from now on is packed. Off by default - the
    // player unpacks every span it plays, several times the per-voice cost.
    void setCompressSamples(bool on) noexcept { compressSamples = on; }
    bool getCompressSamples() const noexcept { return compressSamples; }
    // The storage choices in force, as one string ("16-bit, 32 kHz, releases at half the rate, packed").
    juce::String storageSignature() const;
    // 0.8.151: the RAM the samples would take with every saving option of the load
    // box - 16-bit, 32 kHz on loading, releases at half the rate (where the treble
    // rule of the loader keeps them so), packed - estimated from the samples loaded
    // now: every k-th sample (about 160 of them, the first 3 s of each) is brought
    // into that form and the ratio of its bytes to what it takes now is applied to
    // all the sample bytes. The fixed part of the process (the program, the voice
    // pool, the UI) is the caller's: the measured RAM minus bytesNow.
    struct MinimumRamEstimate { int64_t bytesNow = 0, bytesMinimum = 0; double ratio = 1.0; int samplesTotal = 0, samplesTested = 0, releasesHalved = 0, releasesKeptFull = 0, releasesAlreadyHalved = 0; };
    static constexpr const char* minimumRamStorageText = "16-bit, 32 kHz on loading, releases at half the rate, packed";
    MinimumRamEstimate estimateMinimumRam() const;
    // Packing statistics since the last call: raw bytes in, packed bytes out,
    // samples packed, samples left raw because the check failed.
    struct PackStats { int64_t rawBytes = 0, packedBytes = 0; int samples = 0, failed = 0, floatsLeftRaw = 0, floatsRounded = 0; int64_t nanoseconds = 0; };
    PackStats takePackStats() noexcept;
    // Logs the statistics taken ("Packed samples: <what>: N samples, X -> Y (Z % saved)"), nothing when none.
    void notePackStats(const juce::String& what);
    // Packs a sample when the option is on (and counts it); the loaders, the copy
    // builds and the cache import call it on every finished sample.
    void packIfEnabled(Sample& sample) noexcept;
    // Samples recorded at another rate are resampled to the 48 kHz base rate
    // while they load (windowed sinc); off = they keep their rate and the
    // player reads them at the right speed (linear interpolation). Affects
    // subsequently loaded divisions.
    void setResampleOnLoad(bool on) noexcept { resampleOnLoad = on; }
    // The rate the samples are stored at when resampled on loading (0.8.143): 48 kHz
    // as ever, or 36 / 32 kHz for a quarter / a third less RAM (the content above
    // 17 / 15 kHz goes - organ pipes carry next to nothing there); the kernel
    // interpolates from the coarser grid at the same cost.
    void setStorageSampleRate(double rate) noexcept { storageSampleRate = juce::jlimit(22050.0, 96000.0, rate); }
    double getStorageSampleRate() const noexcept { return storageSampleRate; }
    // Release samples at half the storage rate (0.8.143): the releases are a large
    // part of a set and a decaying tail - stored at half the rate they take a
    // quarter of the RAM they would (the room noise of the tail stays as it is,
    // nothing is requantised).
    void setReleaseHalfRate(bool on) noexcept { releaseHalfRate = on; }
    bool getReleaseHalfRate() const noexcept { return releaseHalfRate; }
    bool getResampleOnLoad() const noexcept { return resampleOnLoad; }
    // Text for the loading window about the phases after the samples are decoded
    // (tuning and footage analysis, pipe fills): called on the loading thread.
    void setLoadStatusCallback(std::function<void(const juce::String&)> callback) { loadStatusCallback = std::move(callback); }

    // Loading runs on any (background) thread and prepares a division without
    // touching the live engine.  commitPendingDivisions() must then be called
    // on the message thread; it swaps the prepared stops in.
    void loadTopFolder(const juce::File& topFolder, double targetSR = 48000.0);
    // append = true keeps the division's existing stops and adds this folder
    // tree to them (composite divisions from several folder trees).
    // Stop building: which sample folder becomes which perspective of which
    // stop. Without a map the folder names decide (perspective prefixes,
    // sub-folders). A map - the table of the Stop building dialog - replaces that
    // decision: the folders it names are loaded as it says (folders of one stop
    // name form one stop, also across folder trees), folders it does not name
    // are left out.
    struct FolderAssignment
    {
        juce::File folder;
        juce::String stopName;
        juce::String perspective;   // "default" or a perspective name
        bool grouped = true;        // same-named folders form one stop (its perspectives)
        bool container = false;     // an empty stop without samples (a stop folder's master); folder = the tree it is created with
        int manual = -1;            // the division this stop belongs to (-1 = the manual given to loadManualFolder)
        // The sample files' numbers minus the keys they play on (0.8.155): Doesburg
        // names the file of key 36 "024" - the offset is -12, the key = number - offset.
        int sampleOffset = 0;
        bool sampleOffsetSet = false;   // the row carries the offset (a definition or the user); unset: the loader finds it from the smpl pitches (0.8.158)
        juce::String component;         // "L" / "R": one half of a stereo stop ("quintaton16L" beside "quintaton16R"), both ranks of one stop (0.8.160)
    };
    // 'replacing': divisions whose stops the commit replaces with the loaded
    // ones (their old stops stay until then and lend their samples and copies
    // to new stops of the same folders); without it, 'append' decides.
    void loadManualFolder(const juce::File& folder, int manualIndex, double targetSR = 48000.0,
        std::function<void(double)> progress = {}, bool append = false,
        const std::vector<FolderAssignment>* map = nullptr, const std::set<int>* replacing = nullptr);
    // Every folder of the tree that holds numbered samples (release and ignored folders aside).
    std::vector<juce::File> listSampleFolders(const juce::File& tree) const;
    // What the automatic rules make of a tree: one assignment per sample folder,
    // in the order the folders are loaded. learnPerspectives stores perspective
    // prefixes the tree reveals (as loading does); false only looks.
    std::vector<FolderAssignment> scanTreeAssignments(const juce::File& tree, bool learnPerspectives);
    static bool folderHasNumberedSamples(const juce::File& folder);
    void commitPendingDivisions();
    bool hasPendingDivisions() const;
    void unloadManual(int manualIndex);
    // Removes only the stops that came from one folder tree of the division.
    void unloadManualTree(int manualIndex, const juce::File& tree);
    // Removes one stop (its ranks, transmissions and samples) and nothing else:
    // no other stop is touched, refilled or rebuilt. The pointer is invalid afterwards.
    void removeStop(Stop* stop);
    // Moves one stop (with its samples and copies) to another division: its
    // routes and fills follow the new compass; missing copies only are built.
    void moveStopToDivision(Stop* stop, int manual);
    // An empty stop (a stop folder's master) added to a division in place.
    Stop* addContainerStop(int manual, const juce::String& name);
    // The fill map of one stop again (its transmissions or its folder changed)
    // and the copies it still misses; everything built before stays.
    void refillStop(Stop* stop);
    // Non-empty after a load ran out of memory (the division is then incomplete).
    juce::String takeLastLoadError();
    bool lastLoadWasMemoryLimited() const noexcept { return lastLoadMemoryLimited.load(std::memory_order_relaxed); }

    // Records the stereo mix of every route's output (even output channels to
    // the left, odd to the right) into a 24-bit WAV. Message thread only.
    bool startRecording(const juce::File& file);
    // The file starts with a 500 ms fade-in. To end it with a 500 ms fade-out,
    // call beginRecordingFadeOut() and stopRecording() once
    // isRecordingFadeOutComplete() reports true (stopRecording() alone ends at once).
    void beginRecordingFadeOut();
    bool isRecordingFadeOutComplete() const noexcept;
    void stopRecording();
    bool isRecording() const noexcept { return recordingActive.load(std::memory_order_relaxed); }
    double getRecordedSeconds() const noexcept;

    void noteOn(int midiNote, int velocity, int manualIndex = -1);
    void noteOff(int midiNote, int manualIndex = -1);
    void noteOffAll();

    void renderBlock(juce::AudioBuffer<float>& buffer, float masterGain);
    void renderBlockForManual(juce::AudioBuffer<float>& buffer, float masterGain, int manualIndex);
    // laneLimit: maximum render threads for this call including the calling
    // thread (0 = use the whole pool, 1 = render inline on the caller only).
    // Windows Audio (WASAPI) callbacks work best with 1, ASIO with the pool.
    void renderBlockForPerspective(juce::AudioBuffer<float>& buffer, float masterGain,
        int manualIndex, const juce::String& perspective, int laneLimit = 0);

    void setManualExpressionGain(int manualIndex, float gain);
    // Compass of a division (lowest and highest MIDI key): keys outside it are
    // silent on every stop and route of that division (an extension ends with
    // the compass). Keys without a division (-1) are not limited.
    void setManualCompass(int manualIndex, int lowestKey, int highestKey);
    // Deepest attenuation the swell of this division can apply (for the tone shelf).
    void setManualExpressionMaxAttenuationDb(int manualIndex, float db);
    // Harmonic filter: every level reduction by wind, tremulant or swell also
    // applies a high shelf that follows the level with a 15 degree phase lag.
    // 'db' is the shelf depth (above ~3 kHz) at full attenuation; 0 = off, which
    // also costs no render time. Costs render time per modulated voice otherwise.
    // Separate depths for the three modulators (dB of high-frequency loss at full
    // attenuation by that modulator); their shares add up, capped at -12 dB.
    void setHarmonicFilterDb(float swellDb, float windDb, float tremulantDb) noexcept;
    // Per-key tone (voicing "Treble per key"): a second-order high shelf per
    // voice. The transition band starts at this frequency (default 1800 Hz) and
    // ends about an octave above it; a pipe whose recorded fundamental (smpl
    // chunk) lies above the start frequency uses its fundamental instead. A pipe
    // held through several routes takes the highest tone setting.
    void setToneStartHz(float hz) noexcept { toneStartHz.store(juce::jlimit(120.0f, 5000.0f, hz), std::memory_order_relaxed); }
    float getToneStartHz() const noexcept { return toneStartHz.load(std::memory_order_relaxed); }
    // The full (8-tap windowed sinc) playback kernel for every voice whose
    // sounding fundamental is at or above this (0.8.164): the linear kernel
    // leaves an image of a retuned high tone below its fundamental (a foreign
    // tone, 48000 x (ratio - 1) Hz under it, -26 dB); the sinc puts it 50 dB
    // down. 0 = every voice, 20000 or more = never (the linear kernel as before).
    void setSincKernelFromHz(float hz) noexcept { sincKernelFromHz.store(juce::jlimit(0.0f, 20000.0f, hz), std::memory_order_relaxed); }
    float getSincKernelFromHz() const noexcept { return sincKernelFromHz.load(std::memory_order_relaxed); }
    // The retune limit per pipe (0.8.165): a pipe whose recorded pitch lies further
    // than this from its key (cents) is a faulty sample - switched off and filled
    // from its neighbours; within it the pipe is retuned (a1 = 440 Hz, temperament).
    void setRetuneLimitCents(float cents);
    float getRetuneLimitCents() const noexcept { return retuneLimitCents.load(std::memory_order_relaxed); }
    void setStopNoteTone(Stop* stop, int note, float db);
    void setTransmissionKeyTone(Stop* stop, int index, int key, float db);
    // Start frequency of the tone shelf for a pipe's sample: the global start, or
    // the sample's own fundamental (smpl chunk) when that is higher.
    static float toneStartHzFor(const Stop* rank, int note, float globalStartHz) noexcept;
    // Coefficients (b0 b1 b2 a1 a2, normalised) of the tone shelf and an offline
    // application of it (Stop-Merging renders the voicing into the files).
    static void toneShelfCoefficients(double sampleRate, float db, float startHz, float coefficients[5]) noexcept;
    static void applyToneShelf(juce::AudioBuffer<float>& buffer, double sampleRate, float db, float startHz);
    // Wind model strength 0..1 (Configuration Editor, default 1): the steady sag
    // scales with it and every key stroke shakes the wind with a random 40..100 %
    // of it. 0 switches the wind model off.
    void setWindMaxEffect(float amount) noexcept { windMaxEffect.store(juce::jlimit(0.0f, 1.0f, amount), std::memory_order_relaxed); }
    float getWindMaxEffect() const noexcept { return windMaxEffect.load(std::memory_order_relaxed); }
    float getHarmonicFilterSwellDb() const noexcept { return harmonicSwellDb.load(std::memory_order_relaxed); }
    float getHarmonicFilterWindDb() const noexcept { return harmonicWindDb.load(std::memory_order_relaxed); }
    float getHarmonicFilterTremulantDb() const noexcept { return harmonicTremulantDb.load(std::memory_order_relaxed); }
    void setTemperamentOffsets(const std::array<float, 12>& centsByPitchClass);
    void setManualOutputChannels(int manualIndex, std::vector<int> channels);
    void setManualDivisionGain(int manualIndex, float gain);
    void setManualSampleRate(int manualIndex, double sampleRate);
    void setPerspectiveOutputChannels(int manualIndex, const juce::String& perspective,
        std::vector<int> channels);
    void setPerspectiveSampleRate(int manualIndex, const juce::String& perspective, double sampleRate);
    void setPerspectiveVolumeDb(int manualIndex, const juce::String& perspective, float db);
    // Channel separation of a route, 0..1 (1 = default): below 1 every channel of
    // the route also carries (1 - separation) of the other channels; 0 = all
    // channels carry the same sum (mono pipes spread over channels leak into each other).
    void setPerspectiveChannelSeparation(int manualIndex, const juce::String& perspective, float separation);
    // Every division has an always-present "auxiliary" route: the downmix of all
    // its perspectives (each pipe is rendered once more into it), with its own
    // device, channels, volume and IR (rear channels with another IR, for example).
    static const char* auxiliaryPerspectiveName() noexcept { return "auxiliary"; }
    // IR reverb of one route (the audio stream sent to this perspective's device):
    // a WAV impulse response (mono, stereo or true stereo = 4 channels LL LR RL RR)
    // convolved with each output pair of the route, mixed dry/wet (0..1). An empty
    // file removes the reverb. Costs the device callback thread a fixed amount per
    // route regardless of polyphony. Message thread only.
    void setPerspectiveReverb(int manualIndex, const juce::String& perspective,
        const juce::File& impulseResponse, float wet);
    void setPerspectiveReverbWet(int manualIndex, const juce::String& perspective, float wet);
    // The device buffer size of a route: the reverb's convolution is partitioned by it.
    void setPerspectiveBlockSize(int manualIndex, const juce::String& perspective, int blockSize);
    // The device (type|name) whose callback renders the route, empty when none:
    // routes of one device with the same output channels and IR settings share
    // one convolution (shared IR). Message thread.
    void setPerspectiveDeviceKey(int manualIndex, const juce::String& perspective, const juce::String& key);
    struct ReverbInfo
    {
        int channels = 0; double seconds = 0.0; double sampleRate = 0.0; juce::String error;
        // As built: Light processing, its rate divider (1 = full rate), the
        // seconds of the IR really convolved (trimmed in Light), and the cost
        // measured at the build - ms of one thread per second of audio.
        bool light = false; int rateDivider = 1; double usedSeconds = 0.0; double costMsPerSecond = 0.0;
        juce::String sharedWith;   // shared IR: the route ("manual N perspective P") whose convolution this route rides on; empty when its own
    };
    ReverbInfo getPerspectiveReverbInfo(int manualIndex, const juce::String& perspective) const;
    // IR processing for weak computers: Light runs every route's convolution at a
    // quarter of the sample rate (half when the high cut lies above 2.5 kHz),
    // trims the IR where its energy has fallen 60 dB and reduces a true stereo
    // IR to its LL/RR channels - a fraction of the memory traffic and work of
    // the full convolution, inaudible under the organ with the wet path's 2 kHz
    // high cut. Rebuilds every route's reverb; message thread.
    void setReverbProcessingLight(bool light);
    bool isReverbProcessingLight() const noexcept;
    // All routes' reverbs as built: their number, the summed and the worst cost
    // (ms of one thread per second of audio, measured at the build), what the
    // same reverbs would cost at full quality, the mode, and the mono-channel
    // IR seconds convolved per second of audio (the size of the task).
    struct ReverbSummary
    {
        int routes = 0; bool light = false;
        int sharedRoutes = 0;   // routes whose IR is convolved by another route of the same device, channels and IR settings (shared IR)
        double totalCostMsPerSecond = 0.0, worstCostMsPerSecond = 0.0, estimatedFullMsPerSecond = 0.0, irSecondsPerSecond = 0.0;
    };
    ReverbSummary getReverbSummary() const;
    // The runtime guard: some route's convolution took more than 60 % of its
    // period twelve times in a row (cleared when read).
    bool takeReverbOverloadFlag() noexcept;
    // Shaping of the wet (reverberated) signal: distance loses highs (high cut),
    // rumble is removed (low cut), pre-delay separates direct sound from the
    // room, wet level trims the reverb, decay shortens the IR (fade at its end).
    // The cuts grow with the dry/wet ratio: at 0 % wet nothing is cut, at 100 %
    // the set frequencies apply. Decay shortens the IR to a share of its length;
    // fade-out (0 = none, the IR is used untouched) lets the shortened IR die away
    // over this share of its end so that a cut never ends abruptly.
    struct ReverbTone
    {
        float highCutHz = 2000.0f, lowCutHz = 195.0f, preDelayMs = 0.0f, wetDb = -4.0f, decayPercent = 100.0f,
            fadePercent = 0.0f;
    };
    void setPerspectiveReverbTone(int manualIndex, const juce::String& perspective, const ReverbTone& tone);

    // Advanced global audio (Configuration Editor).
    // Level change per cent of wind sag (default 0.2 dB) and per cent of tremulant pitch (default 0.187 dB).
    void setWindLevelDbPerCent(float db) noexcept { windLevelDbPerCentValue.store(juce::jlimit(0.0f, 1.0f, db), std::memory_order_relaxed); }
    void setTremulantLevelDbPerCent(float db) noexcept { tremulantLevelDbPerCentValue.store(juce::jlimit(0.0f, 1.0f, db), std::memory_order_relaxed); }
    // Phase of the tremulant's harmonic-filter oscillation relative to its volume
    // oscillation, 0..360 degrees (default 90: the tone dips a quarter cycle after the level).
    void setTremulantShelfPhaseDegrees(float degrees) noexcept
    {
        tremulantShelfPhaseTurns.store(juce::jlimit(0.0f, 360.0f, degrees) / 360.0f, std::memory_order_relaxed);
    }
    float getTremulantShelfPhaseDegrees() const noexcept { return tremulantShelfPhaseTurns.load(std::memory_order_relaxed) * 360.0f; }
    float getWindLevelDbPerCent() const noexcept { return windLevelDbPerCentValue.load(std::memory_order_relaxed); }
    float getTremulantLevelDbPerCent() const noexcept { return tremulantLevelDbPerCentValue.load(std::memory_order_relaxed); }
    // Shares of the full wind sag: one 8' stop alone, one sounding pipe, two pipes
    // (more stops / pipes approach the full sag). Defaults 0.2, 0.5, 0.5.
    void setWindShares(float oneStop, float onePipe, float twoPipes) noexcept;
    // Release samples are cut to this percentage of their length and faded out
    // over the whole shortened length, so they leave the polyphony exactly there
    // (100 = untouched). Applies to releases started afterwards.
    void setReleaseCutPercent(float percent) noexcept { releaseCutPercent.store(juce::jlimit(5.0f, 100.0f, percent), std::memory_order_relaxed); }
    float getReleaseCutPercent() const noexcept { return releaseCutPercent.load(std::memory_order_relaxed); }
    void prepareRenderWorkers(int numChannels, int blockSize);
    // The control thread: the note bookkeeping (MIDI commands, attacks and
    // releases, reclaiming finished voices) runs on a thread of its own instead
    // of inside the audio callback, where it took up to a third of the period.
    // Started by the application once; the self-test never starts it (its
    // renders expect the bookkeeping done synchronously on the audio thread).
    void startControlThread();
    void stopControlThread();
    bool isControlThreadRunning() const noexcept;

    int getCurrentPolyphony() const noexcept { return currentPolyphony.load(std::memory_order_relaxed); }
    // The most voices sounding at once since the last take - kept by the control
    // thread at every change, so no peak between two readings is missed (a key
    // struck while the releases of the last stroke still sound is the peak, and
    // it lasts a few blocks). One counter for the display, one for the polyphony
    // measurement; a take restarts the counter at the current value.
    int takeDisplayPeakPolyphony() noexcept { return peakPolyphonyDisplay.exchange(currentPolyphony.load(std::memory_order_relaxed), std::memory_order_relaxed); }
    int takeMeasuredPeakPolyphony() noexcept { return peakPolyphonyMeasured.exchange(currentPolyphony.load(std::memory_order_relaxed), std::memory_order_relaxed); }
    void setMaxPolyphony(int voices) noexcept { maxPolyphony.store(juce::jlimit(64, maxVoices, voices)); }
    int getMaxPolyphony() const noexcept { return maxPolyphony.load(); }
    // Fraction of the audio period spent rendering (0..1+), smoothed peak.
    float getRenderLoad() const noexcept { return renderLoad.load(std::memory_order_relaxed); }
    // The load the DSP field shows: the whole driver callback of the busiest device
    // (render, note bookkeeping, copies - what really decides about dropouts) when
    // the application reports it, never less than the render alone.
    float getDisplayLoad() const noexcept { return juce::jmax(getRenderLoad(), deviceCallbackLoad.load(std::memory_order_relaxed)); }
    void reportDeviceCallbackShare(float share) noexcept
    {
        float known = deviceCallbackLoad.load(std::memory_order_relaxed);
        const float next = juce::jmax(share, known * 0.98f);   // peak hold with a slow decay (per driver callback)
        deviceCallbackLoad.store(next, std::memory_order_relaxed);
    }
    // The load of every device thread that rendered recently (the busiest first is
    // not guaranteed; getRenderLoad() is their maximum). Returns the count filled.
    int getRenderLoads(float* loads, int* firstRouteIndices, int maxCount) const noexcept
    {
        const int n = juce::jlimit(0, juce::jmin(maxCount, 8), renderThreadLoadCount.load(std::memory_order_relaxed));
        for (int i = 0; i < n; ++i)
        {
            loads[i] = renderThreadLoads[(size_t)i].load(std::memory_order_relaxed);
            firstRouteIndices[i] = renderThreadRoutes[(size_t)i].load(std::memory_order_relaxed);
        }
        return n;
    }
    // "manual N perspective X" of a route index (as in getRenderLoads), for logs.
    juce::String routeLabel(int routeIndex) const;
    // Every route with voices right now, "label: count", for the measurement log.
    juce::String routeVoiceSummary() const;
    int getRenderWorkerCount() const noexcept;
    // Number of render worker threads in addition to the audio callback thread.
    // 0 = the callback thread renders alone; negative = automatic (one lane per
    // physical core). Message thread only; briefly suspends rendering.
    void setRenderThreadCount(int workers, bool force = false);   // force: rebuild the pool even at the same count (a new affinity, 0.8.140)
    void setRenderWorkerAffinityMask(juce::uint64 mask);         // Windows: the logical processors the render workers run on (0 = any; the performance cores of a hybrid CPU)
    // Longest control tick (MIDI/attack/release bookkeeping) since the last call, in ms.
    float takeMaxControlTickMs() noexcept { return maxControlTickMs.exchange(0.0f); }
    // Control work since the last call: ticks run, their total time in ms, voices started.
    void takeControlStats(int& ticks, double& totalMs, int& voicesStarted, float* phaseMaxMs = nullptr) noexcept
    {
        ticks = controlTickCount.exchange(0);
        totalMs = 1000.0 * (double)controlTickTicksTotal.exchange(0) / (double)juce::Time::getHighResolutionTicksPerSecond();
        voicesStarted = controlVoicesStarted.exchange(0);
        for (size_t i = 0; i < controlPhaseMs.size(); ++i)
        {
            const float ms = controlPhaseMs[i].exchange(0.0f);
            if (phaseMaxMs != nullptr) phaseMaxMs[i] = ms;
        }
    }
    // The worst render of one device block since the last call, as a share of
    // that block's duration (1.0 = the whole budget): what decides about dropouts.
    float takeMaxBlockLoad(int* routeIndex = nullptr) noexcept
    {
        if (routeIndex != nullptr) *routeIndex = maxBlockRoute.load(std::memory_order_relaxed);
        return maxBlockMs.exchange(0.0f);
    }
    double getLastBlockMs() const noexcept;   // the duration of the device block rendered last (the budget)
    // Diagnostics: force the scalar interpolation kernel even on SIMD CPUs.
    void setSimdDisabled(bool disabled) noexcept { simdDisabled.store(disabled); }
    bool isUsingSimd() const noexcept;
    static const char* simdKernelName() noexcept;     // the kernel this CPU gets: "AVX2", "SSE2", "NEON" or "scalar"
    const char* currentKernelName() const noexcept;   // ... or "scalar" while SIMD is switched off
    // The kernel switch of the Configuration Editor (0.8.124): the kernels this
    // build and CPU can run, by their setting names ("scalar", "sse2", "avx2",
    // "neon"; "auto" = what the CPU gets by default), and the override applied
    // to every block rendered from now on (-1 = automatic). kernelKindForName
    // gives -1 for "auto" and -2 for a name this computer cannot run.
    static juce::StringArray availableKernelNames();
    static int kernelKindForName(const juce::String& name) noexcept;
    static const char* kernelDisplayName(int kind) noexcept;   // "AVX2", "SSE2", "NEON", "scalar"
    static const char* automaticKernelName() noexcept;         // what "auto" renders with (ORGAN_KERNEL counted)
    static void setKernelOverride(int kind) noexcept;          // -1 (or any unavailable kind) = automatic
    static int getKernelOverride() noexcept;
    // Renders a synthetic signal through the scalar and every SIMD kernel of
    // this CPU, reports the largest difference, and benchmarks the kernel;
    // used by --selftest and --kerneltest.
    static juce::String runKernelSelfCheck();
    // What one thread renders: ns per voice and frame of the kernel alone, with
    // the sample in the cache and streaming from RAM (a buffer of ramBytes).
    struct KernelBenchmark
    {
        juce::String kernel;                          // the SIMD kernel measured (empty: scalar only)
        int blockFrames = 0;
        double nsCacheScalar = 0.0, nsCacheSimd = 0.0;
        double nsRamScalar = 0.0, nsRamSimd = 0.0;
        juce::int64 ramBytes = 0;                     // the buffer the RAM figures were taken on (0: not measured)
        double nsBest() const noexcept { return kernel.isEmpty() ? nsRamScalar : nsRamSimd; }
        juce::String describe() const;
    };
    static KernelBenchmark benchmarkKernel(int blockFrames, juce::int64 ramBytes);
    // The render pool since the last query: the route jobs, the lanes' busy time
    // against lanes x job wall time (utilisation), the device thread's own share.
    struct RenderPoolStats
    {
        int jobs = 0, lanes = 1; double laneUtilisation = 0.0, posterShare = 0.0, jobSeconds = 0.0;
        // The voice profile (while setVoiceProfiling is on): ns per voice and block
        // in the voice header (bookkeeping), the span logic, the kernels, the tone post-processing.
        juce::int64 profiledVoices = 0; double voiceHeaderNs = 0.0, voiceSpansNs = 0.0, voiceKernelNs = 0.0, voicePostNs = 0.0;
        double voiceUnpackNs = 0.0;   // unpacking the windows of packed samples (0.8.145; 0 without the option)
        // The device thread's profile (0.8.137, while setVoiceProfiling is on): ns per
        // route callback in its phases - the voices taken in, the pool (this thread's own
        // rendering, the wait for the other lanes with the hand-over), the mixing
        // around the pool (block context, shelf low-pass), the finished voices handed
        // back, the channel separation, the IR hand-over (of it: waiting for the
        // convolution task), the safety clip, the recording, the load statistics.
        juce::int64 routeCallbacks = 0;
        double routeTakeNs = 0.0, routePoolOwnNs = 0.0, routePoolWaitNs = 0.0, routeMixNs = 0.0, routeCompactNs = 0.0, routeSeparationNs = 0.0,
               routeReverbNs = 0.0, routeReverbWaitNs = 0.0, routeClipNs = 0.0, routeRecordNs = 0.0, routeStatsNs = 0.0;
        double routeTotalNs() const noexcept { return routeTakeNs + routePoolOwnNs + routePoolWaitNs + routeMixNs + routeCompactNs + routeSeparationNs + routeReverbNs + routeClipNs + routeRecordNs + routeStatsNs; }
        // The longest route callback since the last query and its parts (0.8.138): the
        // sound breaks on such a peak, never on the average - the averages of the
        // two-core laptops sat at a third of the period while the longest driver
        // callback took 87 % of it.
        double routeMaxTotalNs = 0.0, routeMaxOwnNs = 0.0, routeMaxWaitNs = 0.0;
        // The latest a worker joined a job after it opened (0.8.143): the wake-up
        // of a sleeping worker, which the poster renders alone meanwhile.
        double laneJoinDelayMaxNs = 0.0;
    };
    RenderPoolStats takeRenderPoolStats() noexcept;
    // Times the parts of every voice render (a few clock reads per voice): the
    // polyphony measurement switches it on for its run, off afterwards.
    void setVoiceProfiling(bool enabled) noexcept;

    void setWindModel(int sourceManual, float depthCents, float frequencyHz,
        float oscillations, float oscillationStrength, float baseSmoothingMs,
        uint8_t targetMask);
    void setManualTremulant(int manualIndex, bool enabled, float rateHz, float depthCents,
        float rankDriftDegrees);
    std::vector<juce::String> getPerspectivesForManual(int manualIndex) const;
    void setStopEnabled(Stop* stop, bool enabled);
    void setRankEnabled(Stop* logicalStop, Stop* rank, bool enabled);
    // Transmissions (see Stop::Transmission). 'index' addresses stop->transmissions.
    // All message thread only; pipes that no key holds any more get their normal
    // release, pipes still held through another route keep sounding.
    void setTransmissionEnabled(Stop* stop, int index, bool enabled);
    // Transposition of a transmission in semitones (-24..24). Applied at once:
    // its sounding pipes are released and the held keys restart with the new shift.
    void setTransmissionShift(Stop* stop, int index, int semitones);
    // Volume of a transmission relative to the parent stop (-10..10 dB). A pipe
    // held through several routes sounds at the loudest one; when that route
    // lets go, the volume glides to the next loudest over the division's wind
    // sag lag without interrupting the sound.
    void setTransmissionDb(Stop* stop, int index, float db);
    // Per-key voicing of a transmission (dB for the key played), on top of its dB.
    void setTransmissionKeyDb(Stop* stop, int index, int key, float db);
    // Missing pipes (beyond a rank's compass, gaps inside it) are filled from
    // neighbouring pipes taken as a group of this many adjacent pipes (1..12),
    // transposed by the group's width; the group repeats for further pipes.
    // 12 keeps every pitch class on its own pipe an octave away.
    void setFillGroupSize(int pipes);
    int getFillGroupSize() const noexcept { return fillGroupSize.load(std::memory_order_relaxed); }
    // Fill map of one rank for the pipes [needLow, needHigh] (beyond the compass
    // in groups of groupSize; a last, smaller group takes the outermost pipes with
    // the smallest possible transposition; gaps inside from both sides).
    static void computeFilledPipes(Stop* rank, int groupSize, int needLow = 0, int needHigh = 127,
        const std::bitset<128>* disabledPipes = nullptr, bool keepExisting = true);   // keepExisting: pipes filled before keep their source
    // Switches a pipe of a stop off or on (voicing checkbox). Off: the pipe is
    // never played or filled again; the anti-alias copies of an extension are
    // freed, a real pipe keeps its samples in RAM. On: it plays again at once
    // (fill map recomputed). Message thread only.
    void setPipeEnabled(Stop* stop, int pipe, bool enabled);
    // Velocity layer "_NNN" (files for key velocities up to NNN) on or off in the
    // stop's voicing: off, those velocities play the next layer up or the plain
    // samples. maxVelocity 128 = the plain samples themselves ("_128"): off, the
    // velocities above the highest file layer play the highest enabled layer.
    void setVelocityLayerEnabled(Stop* stop, int maxVelocity, bool enabled);
    // A key of one transmission on or off (its own voicing row): the stop's own
    // pipes and its other transmissions are not affected. Message thread only.
    void setTransmissionKeyEnabled(Stop* stop, int index, int key, bool enabled);
    // Resampling of filled pipes (a neighbour's sample played at another pitch):
    // 0 = linear interpolation (as every other voice), 1 = cubic (Hermite)
    // interpolation for these voices, 2 = cubic plus an anti-alias low-pass copy
    // of the source for pipes transposed upwards (built per filled pipe only,
    // when needed). Real pipes are never touched. Message thread only.
    void setTransposeQuality(int quality);
    int getTransposeQuality() const noexcept { return transposeQuality.load(std::memory_order_relaxed); }
    // How a filled pipe gets its pitch: 0 = resampling (pitch and length change
    // together, the anti-alias copies above), 1 = pitch shift with the length kept
    // (Signalsmith Stretch, copies of every filled pipe's attacks and releases built
    // in the background, played at unity). Changing it rebuilds the copies.
    void setTransposeMethod(int method);
    int getTransposeMethod() const noexcept { return transposeMethod.load(std::memory_order_relaxed); }
    // Pitch shift method: a release copy's duration as a share of its source's
    // (0.25..2, default 1). The resampling alone would make a release shifted
    // down two octaves four times as long and one shifted up a quarter; the
    // room decay is laid out again to this length instead (the pipe's own stop
    // transient at the start stays as resampled). Changing it rebuilds the release copies.
    void setTransposeReleaseLength(float share);
    // Method 2 ("timing kept"): how much of the length change the resampling made
    // is given back to the attack (before the first loop) and to the release,
    // 0..1: 0 = as resampled (two octaves down: four times as long), 1 = the
    // source's duration, 0.5 = half way on a logarithmic scale (twice as long).
    // Done by correlated overlap-add (WSOLA with windows of three periods of the
    // copy's pitch); the loops are never touched. Changing them rebuilds the copies.
    void setTransposeAttackTiming(float share);
    float getTransposeAttackTiming() const noexcept { return transposeAttackTiming.load(std::memory_order_relaxed); }
    void setTransposeReleaseTiming(float share);
    float getTransposeReleaseTiming() const noexcept { return transposeReleaseTiming.load(std::memory_order_relaxed); }
    // Every pitch-shift method: a release copy is played from this many milliseconds
    // in (0..300, default 50) - the resampling and the vocoders leave their worst at
    // a release's start. Applied at playback: nothing is rebuilt.
    void setTransposeReleaseTrim(float milliseconds);
    float getTransposeReleaseTrim() const noexcept { return transposeReleaseTrimMs.load(std::memory_order_relaxed); }
    // Optional: a release copy played at the level relation the source's release has
    // to the source's sustain (Sample::releaseLevelGain, measured when the copy was
    // built). Applied at playback: nothing is rebuilt.
    void setTransposeReleaseLevelMatch(bool enabled);
    bool getTransposeReleaseLevelMatch() const noexcept { return transposeReleaseLevelMatch.load(std::memory_order_relaxed); }
    float getTransposeReleaseLength() const noexcept { return transposeReleaseLength.load(std::memory_order_relaxed); }
    // Pitch shift method: how far the spectral envelope (the formants) follows the
    // pitch, 0..1, per stop (Stop::formantFollow) and per transmission for the
    // pipes only it asks for. The harmonics of a shifted copy move to the new
    // pitch; the envelope is kept in place (0) or moved along by this share of
    // the shift downwards (0.5: a pipe shifted -24 gets its envelope 12
    // semitones lower; 1: plain transposition); upwards the share is 0.3 more.
    // A pipe is one copy: when the stop's own compass holds it, the stop's value;
    // otherwise the first transmission (in the stop's list) whose shifted
    // compass holds it - two transmissions asking for the same pipe share that copy.
    // A change retires the copies of the pipes concerned and builds them again.
    static constexpr float defaultFormantFollow = 0.3f;
    void setStopFormantFollow(Stop* stop, float share);
    void setTransmissionFormantFollow(Stop* stop, int index, float share);
    float formantFollowForPipe(const Stop* stop, int pipe) const;   // whose setting a pipe of the stop (or its ranks) is built with
    // Pitch shift method: a copy is brought to the loudness (RMS) of its source.
    void setTransposeKeepLoudness(bool keep);
    bool getTransposeKeepLoudness() const noexcept { return transposeKeepLoudness.load(std::memory_order_relaxed); }
    // Loop splice of the filled pipes' copies: 0 = the ordinary 50 ms, otherwise
    // this share of each loop's length. Applied to the existing copies at once.
    void setCopyLoopCrossfade(float fraction);
    float getCopyLoopCrossfade() const noexcept { return copyLoopCrossfade.load(std::memory_order_relaxed); }
    static void applyCopyLoopCrossfade(Sample& sample, double fraction);
    // How many copies (over the stop's ranks and folder trees) are missing for the current method.
    int countMissingCopies(Stop* stop, juce::String* detail = nullptr);   // detail: per-set counts (diagnostics)
    // Cache of the generated copies on disk (a Configuration Editor option): one
    // file per rank and folder tree in 'directory'. A copy whose build parameters
    // and source sample match is read from the file instead of being computed;
    // every build writes its set. The build parameters are part of the file name
    // (sets built with other settings stay beside it) and a copy's "formants
    // follow" value is in its record: nothing is ever emptied by a change of a
    // setting, only "Empty the cache" (clearCopyCache) deletes the files.
    void setCopyCache(bool enabled, const juce::File& directory);
    bool isCopyCacheEnabled() const noexcept { return copyCacheEnabled.load(std::memory_order_relaxed); }
    juce::File getCopyCacheDirectory() const { return copyCacheDir; }
    void clearCopyCache();            // deletes the cache files (the builds are paused meanwhile)
    int removeCopyCacheFilesOfUnloadedRanks();   // deletes the files of ranks not loaded now (other configurations, gone folders); how many
    void exportCopiesToCache();       // writes the copies already built (a worker); done by setCopyCache when switched on
    int pendingBackgroundWorkers() const noexcept { return backgroundBuilds.load(std::memory_order_acquire); }   // builds and exports launched and not finished
    // After loading: every stop that misses copies and has no build queued gets a
    // background build (the loader does not build; the stops exist only after the commit).
    void launchCopyBuilds();
    // While "Compiling the Stop List" runs, no copy build is launched: the voicing
    // state alone would launch dozens, and every later step that needs the build
    // lock (a pipe switched off, a transmission ...) would wait for the running
    // pipe. The stops just remember that their copies are missing;
    // launchCopyBuilds (the last step) lifts the hold and launches them all.
    void holdCopyBuilds();
    // The stops an unload removes are freed on a thread of their own (0.8.184): 23 GB
    // in 150 000 packed vectors held the message thread 11 s per organ, the window
    // dead - "the load waits for the filled pipes". A load waits for that free
    // before it counts memory (the loader thread, with a status line).
    void waitForRemovedStopsFreed();
    bool removedStopsBeingFreed() const noexcept { return removedStopsFreeing.load(std::memory_order_acquire) > 0; }
    // Load Configuration unloads every division in turn: between the unloads no build
    // of the old organ is launched again by a BuildPause's end - the next unload
    // would interrupt it (and a partial cache file be written for a stop about to go).
    void setRelaunchHeld(bool held) noexcept { relaunchHeld.store(held, std::memory_order_release); }
    // The loading dialog's Cancel (0.8.139): the loading stops at the next stop (or
    // 32 files), the instrument is built from the stops loaded so far, the rest
    // is named in the message like a memory stop.
    void cancelLoading() noexcept { loadCancelRequested.store(true, std::memory_order_relaxed); }
    bool copyBuildsHeld() const noexcept { return buildsHeld.load(std::memory_order_acquire); }
    // A safety net for the UI's timer: builds that ended before their last pipe and
    // were not launched again (whatever the reason) are launched now. Returns how many.
    int resumeIncompleteCopyBuilds();
    // Takes the build lock without stalling the caller for a whole stop's build:
    // the running build stops at its next pipe (a second at most), the operation
    // runs, then the pending builds are launched again. For the message thread
    // (and the self-test: nothing touches a stop's copies without it).
    struct BuildPause
    {
        explicit BuildPause(SamplerEngine& e);
        ~BuildPause();
        SamplerEngine& engine;
    };
    int64_t copyCacheBytes() const;   // the size of the cache files on disk
    std::atomic<int> copyCacheHits{ 0 }, copyCacheWrites{ 0 };   // pipes read from the cache, set files written (statistics)
    // Builds (or refreshes) the anti-alias copies of a stop's filled pipes; no-op
    // below quality 2. Filtering runs without the control lock; voices play the
    // plain source until a copy exists. 'background' runs it on a worker thread
    // (message-thread callers), the loader thread calls it directly.
    void buildAntiAliasCopies(Stop* stop, bool background = false);
    // Adds a switched-off transmission into 'manual'; returns its index.
    int addTransmission(Stop* stop, int manual, int shift = 0);
    // Removes a transmission (switching it off first). Later indices move down by one.
    void removeTransmission(Stop* stop, int index);
    // Replaces the whole list (loading a state): everything is switched off
    // first, then the entries flagged enabled are switched on.
    void replaceTransmissions(Stop* stop, std::vector<Stop::Transmission> list);
    // Footage of a stop in semitones above its keys (0 = 8'): from the smpl
    // analysis when available, else from the footage written in its name.
    static int stopFootageSemitones(const Stop* stop);
    // The footage the files sound at on their own numbers - before the stop's
    // transposition (the base the Transposition box counts from: "0 -> 8' (as
    // the original)"; the measured footage holds the transposition already).
    static int untransposedFootageSemitones(const Stop* stop);
    // The name without its footage ("Prestant 8" -> "Prestant"): the Hauptwerk
    // import's aliases, so the footage shown follows the transposition (0.8.158).
    static juce::String nameWithoutFootage(const juce::String& name);
    // The name with its footage token replaced by the label of 'semitones'
    // (appended when the name has none); a division prefix is skipped.
    static juce::String nameWithFootage(const juce::String& name, int semitones);
    // The pure (just) ratio of a semitone class 0..11 (0.8.159): the footage name
    // and the pure retune of a transposition outside the harmonic footages.
    static int justIntervalNumerator(int semitoneClass);
    static int justIntervalDenominator(int semitoneClass);
    // The voicing's One-shot switch: -1 = by the patterns, 0 off, 1 on.
    void setStopOneShot(Stop* stop, int manual);
    // The footage the stop's caption shows (written in its name or alias, else the
    // measured one that displayName() appends, else the smpl footage): the base
    // for the footage of its transmissions, so both agree.
    static int displayedFootageSemitones(const Stop* stop);
    // Footage written in a stop name ("Quinte 2 2/3'" -> 19); false if none.
    static bool footageFromName(const juce::String& name, int& semitones);
    // Short footage caption ("2 2/3'") without the interval in brackets.
    static juce::String footageLabel(int semitones);
    // Name of a stop's transmission shifted by 'shift' semitones: the footage in
    // the name is replaced ("Principal 8'" -> "Principal 16'" for -12).
    static juce::String transposedStopName(const Stop* stop, int shift);
    // The mark a transmission's caption ends with: " (Ext.)" when it plays in the
    // stop's own division (an extension, usually at another footage), " (Trsm.)"
    // when it plays in another division (at any footage).
    static juce::String transmissionSuffix(const Stop* stop, const Stop::Transmission& t);
    // The automatic caption of a transmission: the transposed stop name plus the mark.
    static juce::String transmissionCaption(const Stop* stop, const Stop::Transmission& t);
    // Deviation (cents) of the pure harmonic at this footage offset from equal
    // temperament: +1.955 for fifths (2 2/3'), -13.686 for thirds (1 3/5'), 0 for octaves.
    static double pureDeviationCents(int semitones);
    void setGlobalPitchCents(float cents);
    void setManualRandomTuningAllowed(int manualIndex, bool allowed);
    void setManualPipeCouplingAllowed(int manualIndex, bool allowed);
    void setStopRandomPitchError(Stop* stop, float cents, bool regenerate = true);
    // Tuning to a1 = 440 Hz equal temperament from the recorded smpl pitches
    // (footages, fifths, thirds and breaks are kept; only diapason and the
    // recorded temperament are removed). Message thread only.
    void setStopTuneTo440(Stop* stop, bool enabled);
    // Global "equal temperament at a1 = 440 Hz": every stop (also stops loaded
    // later) is normalised from its smpl pitches. Message thread only.
    void setAllStopsTuneTo440(bool enabled);
    bool getTuneAllTo440() const noexcept { return tuneAllTo440; }
    // Temperaments as absolute cents per pitch class (C..B): the selected target
    // and the samples' base table. A rank whose recorded temperament was measured
    // retunes from that measurement; a rank normalised to 440 Hz retunes from
    // equal temperament; every other rank retunes from the base table.
    void setTemperaments(const std::array<float, 12>& selectedCents, const std::array<float, 12>& baseCents);
    // Celeste: beats per second against an in-tune rank at the a1 key (sign =
    // sharp/flat). Beats grow with pitch so that the top of the compass beats
    // twice as fast as the bottom; the detune in cents is clamped to +-30.
    // Only effective together with the 440 Hz tuning. Message thread only.
    void setStopUndulation(Stop* stop, float beatsAtA1);
    // How much faster the top of a celeste's compass beats than its bottom
    // (default 4). Recomputes every undulating stop. Message thread only.
    void setUndulationBeatRatio(float ratio);
    float getUndulationBeatRatio() const noexcept { return undulationBeatRatio.load(std::memory_order_relaxed); }
    // Exchanges the stops of two divisions (every stop with its ranks, voicing
    // and transmissions; transmissions into either division move with it), as
    // if their folders had been swapped. The divisions keep their own settings
    // (tremulant, swell, wind, couplers, MIDI, audio). Message thread only.
    void swapDivisions(int a, int b);
    // Stop pool: release / reload the samples of a stop (and its perspective
    // ranks). Message thread only; reloading decodes synchronously.
    void unloadStopSamples(Stop* stop);
    void reloadStopSamples(Stop* stop, bool evenIfLoaded = false);
    // "Files vs keys" of a loaded stop (0.8.155): the offset set on the stop and
    // its ranks, their samples decoded again on their keys, the fills rebuilt.
    void setStopSampleOffset(Stop* stop, int offset);
    // Stops (per division, by name) whose samples are not loaded at all while
    // they sit in the stop pool. Consulted by the loaders.
    void setPooledStopNames(std::map<int, std::set<juce::String>> names);

    // Only call from the message thread. Division loads are committed on the
    // message thread, so the list is stable for GUI code.
    juce::OwnedArray<Stop>& getStops() { return stops; }
    const juce::OwnedArray<Stop>& getStops() const { return stops; }

    void setReleaseFadeOutMs(float ms) { releaseFadeOutMs.store(juce::jlimit(1.0f, 2000.0f, ms)); }
    void setReleaseFadeInMs(float ms) { releaseFadeInMs.store(juce::jlimit(0.0f, 2000.0f, ms)); }
    // Key velocity on a rank without velocity layers: the start fade-in at the
    // lowest velocity (default 40 ms), linearly down to none at velocity 127.
    void setVelocityFadeInMs(float ms) { velocityFadeInMs.store(juce::jlimit(0.0f, 500.0f, ms)); }
    float getVelocityFadeInMs() const noexcept { return velocityFadeInMs.load(std::memory_order_relaxed); }
    // ... and the chiff blunted by an extra high-shelf cut at the start: this many dB
    // at the lowest velocity (default 9), opening to nothing over velocityShelfMs (default 80).
    void setVelocityShelfDb(float db) { velocityShelfDb.store(juce::jlimit(0.0f, 24.0f, db)); }
    float getVelocityShelfDb() const noexcept { return velocityShelfDb.load(std::memory_order_relaxed); }
    void setVelocityShelfMs(float ms) { velocityShelfMs.store(juce::jlimit(5.0f, 1000.0f, ms)); }
    float getVelocityShelfMs() const noexcept { return velocityShelfMs.load(std::memory_order_relaxed); }
    void setReleaseScalingDb(float db);
    void setAttackMinimumMs(float ms) { attackMinimumMs.store(juce::jlimit(0.0f, 500.0f, ms)); }
    void setAttackMinimumDelayMs(float ms) { attackMinimumDelayMs.store(juce::jlimit(0.0f, 500.0f, ms)); }

    // Compatibility for GUI code that used to guard stop edits.  The audio
    // thread only tries this lock, so holding it from the GUI never blocks audio.
    juce::CriticalSection& getLock() const { return controlLock; }

    static int extractMidiFromFilename(const juce::String& filename);
    // The velocity layer of a sample file: "_NNN" (two or three digits, 1..127) at
    // the end of the name, after the key and an optional note name ("060-C4_070.wav"
    // -> 70). -1 when the name has none (the plain sample, for every velocity above).
    static int extractVelocityLayerFromFilename(const juce::String& filename);
    // "8'", "2 2/3' (fifth)", "1 3/5' (third)" ... for a footage in semitones above the key.
    static juce::String footageName(int semitones);
    static int extractReleaseHoldMs(const juce::String& folderName);

    // Default folder-name pattern lists: used when the program starts without
    // saved lists, when a saved list is empty and after a Global Reset.
    // One-shot stops: percussion and bells play once, however long the key is held ("Celesta" is the keyboard instrument, "Celeste" a rank of strings - not matched).
    static std::vector<juce::String> defaultOneShotPatterns() { return { "Chiff", "Chimes", "Tubular", "Celesta", "Harp", "Glockenspiel" }; }
    // Folders a tree scan never loads: "trem" anywhere, names ending in "_tr", and "output" (the summed/harmonic renderings some sets keep under a rank, "outputsum"/"outputharm").
    static std::vector<juce::String> defaultIgnoredPatterns() { return { "trem", "*_tr", "output" }; }
    // Folder-name patterns: no star = anywhere in the name, "*x" = the name ends
    // in x, "x*" = it starts with x; case-insensitive, underscores count.
    static bool folderNameMatchesPattern(const juce::String& folderName, const juce::String& pattern);
    static std::vector<juce::String> defaultPerspectivePatterns() { return { "Direct", "Diffuse", "Rear" }; }

    void setOneShotPatterns(const std::vector<juce::String>& patterns);
    void updateOneShotFlags();
    const std::vector<juce::String>& getOneShotPatterns() const { return oneShotPatterns; }
    void setIgnoredPatterns(const std::vector<juce::String>& patterns);
    const std::vector<juce::String>& getIgnoredPatterns() const { return ignoredPatterns; }
    bool isIgnoredFolderName(const juce::String& folderName) const;
    void setPerspectivePatterns(const std::vector<juce::String>& patterns) { perspectivePatterns = patterns; }
    // An alias for a stop from the bare name of its sample folder (no parent
    // folders, no perspective prefix, the footage with its foot mark): what the
    // Stop List's "Aliases from folders" fills in where no alias is set.
    juce::String folderStopAlias(const juce::File& folder) const;
    const std::vector<juce::String>& getPerspectivePatterns() const { return perspectivePatterns; }

    struct Impl;

private:
    std::unique_ptr<Impl> impl;
    mutable juce::CriticalSection controlLock;
    juce::OwnedArray<Stop> stops;
    juce::OwnedArray<Stop> removedStops;          // unloaded stops awaiting their free on a thread (0.8.184)
    juce::CriticalSection removedStopsLock;
    std::atomic<int> removedStopsFreeing{ 0 };   // free threads running
    std::atomic<bool> relaunchHeld{ false };     // setRelaunchHeld: no relaunch of pending builds between the unloads of a load
    void freeRemovedStopsLater();

    std::atomic<int> currentPolyphony{ 0 };
    std::atomic<int> peakPolyphonyDisplay{ 0 }, peakPolyphonyMeasured{ 0 };
    void storePolyphony(int voices) noexcept
    {
        currentPolyphony.store(voices, std::memory_order_relaxed);
        for (auto* peak : { &peakPolyphonyDisplay, &peakPolyphonyMeasured })
        {
            int known = peak->load(std::memory_order_relaxed);
            while (voices > known && !peak->compare_exchange_weak(known, voices, std::memory_order_relaxed)) {}
        }
    }
    std::atomic<int> maxPolyphony{ 30000 };
    std::atomic<float> renderLoad{ 0.0f };
    std::atomic<float> deviceCallbackLoad{ 0.0f };
    std::array<std::atomic<float>, 8> renderThreadLoads{};
    std::array<std::atomic<int>, 8> renderThreadRoutes{};
    std::atomic<int> renderThreadLoadCount{ 0 };
    std::atomic<float> maxControlTickMs{ 0.0f };
    std::array<std::atomic<float>, 4> controlPhaseMs{};   // longest reclaim / MIDI / pending / stale-route phase
    std::atomic<int> controlTickCount{ 0 }, controlVoicesStarted{ 0 };
    std::atomic<int64_t> controlTickTicksTotal{ 0 };
    std::atomic<float> maxBlockMs{ 0.0f };
    std::atomic<int> maxBlockRoute{ -1 };
    std::atomic<bool> simdDisabled{ false };
    std::atomic<float> harmonicSwellDb{ 8.0f }, harmonicWindDb{ 2.0f }, harmonicTremulantDb{ 4.0f };
    // 1 - 10^(-db/20): the shelf's high-frequency loss per modulator
    std::atomic<float> harmonicSwellMix{ 0.60189f }, harmonicWindMix{ 0.20567f }, harmonicTremulantMix{ 0.36904f };
    std::atomic<float> windLevelDbPerCentValue{ 0.2f };
    std::atomic<float> tremulantLevelDbPerCentValue{ 0.187f };
    std::atomic<float> tremulantShelfPhaseTurns{ 0.25f };
    std::atomic<float> windOneStopShare{ 0.2f }, windOnePipeShare{ 0.5f }, windTwoPipesShare{ 0.5f };
    std::atomic<float> releaseCutPercent{ 100.0f };
    std::atomic<float> toneStartHz{ 1800.0f };   // per-key tone shelf: start of the transition band
    std::atomic<float> sincKernelFromHz{ 3000.0f };   // the sinc playback kernel from this sounding fundamental up (0.8.164)
    std::atomic<float> retuneLimitCents{ 200.0f };    // a pipe further off its key than this is faulty: off, filled from the neighbours (0.8.165)
    std::atomic<float> windMaxEffect{ 1.0f };    // wind model strength 0..1
    std::atomic<int> fillGroupSize{ 12 };
    std::atomic<int> transposeQuality{ 2 };
    std::atomic<int> transposeMethod{ 2 };   // pitch shift + time stretch by default (the GUI sets it from the settings)
    std::atomic<float> transposeReleaseLength{ 1.0f };  // pitch shift: a release copy's duration as a share of its source's (0.25..2)
    std::atomic<float> transposeAttackTiming{ 0.5f };   // timing kept: share of the attack's length change given back (0..1)
    std::atomic<float> transposeReleaseTiming{ 0.55f }; // time stretch / vocoder: the same for the release (vocoder: upwards only)
    std::atomic<float> transposeReleaseTrimMs{ 140.0f }; // every pitch-shift method: a release copy played from here (playback)
    std::atomic<bool> transposeReleaseLevelMatch{ false };   // release copies at the source's level step (playback)
    std::atomic<bool> transposeKeepLoudness{ true };    // pitch shift: copy at the source's RMS
    std::atomic<float> copyLoopCrossfade{ 0.4f };       // filled pipes: loop splice as a share of the loop length (0 = 50 ms)
    std::atomic<int> backgroundBuilds{ 0 };   // copy builds launched on worker threads and not finished yet
    std::atomic<bool> buildsAborted{ false }; // set by the destructor: running copy builds stop at the next pipe
    std::atomic<bool> buildsInterrupted{ false };   // a message-thread operation needs the build lock: builds stop at the next pipe
    std::atomic<bool> buildsHeld{ false };          // holdCopyBuilds: nothing is launched until launchCopyBuilds
    std::atomic<bool> copyCacheEnabled{ false };
    juce::File copyCacheDir;
    juce::String copyParametersKey() const;   // the build parameters a cached copy must have been made with
    void retirePitchShiftCopiesAndRebuild();  // every rank's shifted copies again (a build parameter changed)
    void retireReleaseCopiesAndRebuild();     // every rank's release copies again (a release parameter changed)
    void pruneStaleCopyCacheFiles();          // files of an older cache format and leftover temporaries go
    // Applies a change of a stop's or transmission's "formants follow" and retires
    // the copies of the pipes (of the stop and its stop folder's children) whose
    // setting changed, then launches their builds.
    void changeFormantFollow(Stop* stop, const std::function<void()>& apply);
    // Launches the background build of every stop that still misses copies.
    void relaunchPendingBuilds();
    juce::CriticalSection antiAliasBuildLock;   // one build at a time; unloading waits for it
    std::array<std::atomic<int>, maxManuals> compassLow{}, compassHigh{};   // set in the constructor (0..127)
    std::atomic<bool> recordingActive{ false };
    std::atomic<bool> loadOutOfMemory{ false };
    std::atomic<bool> loadCancelRequested{ false }, loadCancelled{ false };   // cancelLoading(): asked, and acted on in this load
    std::atomic<bool> loadStoppedForMemory{ false };   // the memory reserve was reached (not a failed allocation): the load stopped on purpose
    juce::StringArray loadMemorySkipped;               // the stops left out since the stop, over every tree of one load (cleared by takeLastLoadError)
    std::set<juce::String> loadMemorySkippedKeys;      // group keys (comparable names) of the stops skipped in this tree: a stop with one perspective loaded and another skipped is dropped whole (0.8.136)
    std::atomic<bool> lastLoadMemoryLimited{ false };  // the last load (every tree of it) stopped at the memory limit: the polyphony report says so instead of "the system pages" (#25, #26)
    std::set<Stop*> loadMemoryCutStops;                // stops (or perspective ranks) the memory stop cut in the middle of their files: dropped at the end of the tree, not half a stop on the grid (0.8.135)
    juce::String loadMemoryStopReason;                 // why (free memory, the compressor, the program's footprint): for the message
    std::atomic<float> undulationBeatRatio{ 12.0f };
    juce::String lastLoadError;
    std::atomic<double> defaultSampleRate{ 48000.0 };
    std::atomic<float> releaseFadeOutMs{ 70.0f };
    std::atomic<float> releaseFadeInMs{ 20.0f };
    std::atomic<float> velocityFadeInMs{ 40.0f };
    std::atomic<float> velocityShelfDb{ 9.0f };
    std::atomic<float> velocityShelfMs{ 80.0f };
    std::atomic<float> releaseScalingGain{ 1.0f };
    std::atomic<float> attackMinimumMs{ 80.0f };
    std::atomic<float> attackMinimumDelayMs{ 2.0f };
    bool storeSamplesAsFloat = true;
    bool resampleOnLoad = true;
    double storageSampleRate = 48000.0;
    bool releaseHalfRate = false;
    bool compressSamples = false;   // 0.8.145: pack the 16-bit samples in RAM (Sample::compress16)
    std::atomic<int64_t> packRawBytes{ 0 }, packPackedBytes{ 0 };
    std::atomic<int> packSamples{ 0 }, packFailed{ 0 }, packFloatsLeftRaw{ 0 }, packFloatsRounded{ 0 };   // floats: beyond +-2 full scale (left raw) / rounded to 24 bits (0.8.183)
    std::atomic<int64_t> packNanoseconds{ 0 };   // thread time spent packing since the last takePackStats (0.8.153: what the option costs at the load)
    std::atomic<int> halfRateReleases{ 0 }, halfRateKeptFull{ 0 }, halfRateSincHeads{ 0 };   // 0.8.146: releases stored at half the rate / kept full for their treble
    std::function<void(const juce::String&)> loadStatusCallback;
    bool tuneAllTo440 = false;

    std::vector<juce::String> oneShotPatterns = defaultOneShotPatterns();
    std::vector<juce::String> ignoredPatterns = defaultIgnoredPatterns();
    std::vector<juce::String> perspectivePatterns = defaultPerspectivePatterns();

    void loadSamplesIntoStop(Stop* stop, const juce::File& folder, bool isRelease, int releaseMaxHoldMs);
    // Attack samples of rank->folder plus its rel* release folders.
    void loadRankSamples(Stop* rank);
    static SmplInfo parseSmpl(const juce::File& file, std::shared_ptr<juce::MemoryBlock> data = nullptr);   // data: the file's bytes out of an archive (0.8.157)
    // Fills tuning440Cents / measuredDiapasonHz of every rank of one organ
    // (all ranks loaded from the same folder share one diapason).
    static void computeTuning440(const std::vector<Stop*>& ranksOfOrgan);
    static void computeUndulationTable(Stop* rank, float beatsAtA1, float ratio);
    // Fill maps of a stop and its ranks for the pipes its routes need (the compass
    // of its division and of every transmission, shifted). Call with controlLock held.
    void refillStopPipes(Stop* stop, bool keepExisting = true);   // keepExisting: see computeFilledPipes (false: the group size changed)
    void refillAllStops();
    std::mutex pooledMutex;
    std::map<int, std::set<juce::String>> pooledStopNames;
    bool isPooledStop(int manual, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerEngine)
};
