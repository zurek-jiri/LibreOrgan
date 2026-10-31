#pragma once

#include <JuceHeader.h>
#include "AggregatedWASAPIAudio.h"

// The audio device types every AudioDeviceManager of the program gets beyond
// JUCE's own: the Aggregated WASAPI device on Windows, and on Linux an ALSA
// list without the template PCMs.

#if JUCE_LINUX
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <set>
#include <map>
#include <thread>
#include <chrono>
#include <functional>
#include "CrashHandler.h"

// Linux: whether the ALSA devices that cannot be opened (held by the sound
// server or another program) are listed too. Off by default: chosen on a route
// they give no channels or 'none'. The settings editor switches it.
inline std::atomic<bool>& unusableAudioDevicesShown() { static std::atomic<bool> flag{ false }; return flag; }
// What the last ALSA scan left out, for the activity log.
inline juce::String& hiddenAlsaDevicesNote() { static juce::String note; return note; }

// An audio device that hands everything to the real one. The callback is
// interposed so that the program sees this object (not the inner one) as the
// device that starts and stops.
class ForwardingAudioIODevice : public juce::AudioIODevice, private juce::AudioIODeviceCallback
{
public:
    ForwardingAudioIODevice(std::unique_ptr<juce::AudioIODevice> device, const juce::String& typeName)
        : juce::AudioIODevice(device->getName(), typeName), inner(std::move(device)) {}
    ~ForwardingAudioIODevice() override { if (inner != nullptr) { inner->stop(); inner->close(); } }

    juce::StringArray getOutputChannelNames() override { return inner->getOutputChannelNames(); }
    juce::StringArray getInputChannelNames() override { return inner->getInputChannelNames(); }
    std::optional<juce::BigInteger> getDefaultOutputChannels() const override { return inner->getDefaultOutputChannels(); }
    std::optional<juce::BigInteger> getDefaultInputChannels() const override { return inner->getDefaultInputChannels(); }
    juce::Array<double> getAvailableSampleRates() override { return inner->getAvailableSampleRates(); }
    juce::Array<int> getAvailableBufferSizes() override { return inner->getAvailableBufferSizes(); }
    int getDefaultBufferSize() override { return inner->getDefaultBufferSize(); }
    juce::String open(const juce::BigInteger& inputChannels, const juce::BigInteger& outputChannels, double sampleRate, int bufferSizeSamples) override
    { return inner->open(inputChannels, outputChannels, sampleRate, bufferSizeSamples); }
    void close() override { inner->close(); }
    bool isOpen() override { return inner->isOpen(); }
    void start(juce::AudioIODeviceCallback* callback) override { target.store(callback); inner->start(this); }
    void stop() override { inner->stop(); target.store(nullptr); }
    bool isPlaying() override { return inner->isPlaying(); }
    juce::String getLastError() override { return inner->getLastError(); }
    int getCurrentBufferSizeSamples() override { return inner->getCurrentBufferSizeSamples(); }
    double getCurrentSampleRate() override { return inner->getCurrentSampleRate(); }
    int getCurrentBitDepth() override { return inner->getCurrentBitDepth(); }
    juce::BigInteger getActiveOutputChannels() const override { return inner->getActiveOutputChannels(); }
    juce::BigInteger getActiveInputChannels() const override { return inner->getActiveInputChannels(); }
    int getOutputLatencyInSamples() override { return inner->getOutputLatencyInSamples(); }
    int getInputLatencyInSamples() override { return inner->getInputLatencyInSamples(); }
    juce::AudioWorkgroup getWorkgroup() const override { return inner->getWorkgroup(); }
    bool hasControlPanel() const override { return inner->hasControlPanel(); }
    bool showControlPanel() override { return inner->showControlPanel(); }
    bool setAudioPreprocessingEnabled(bool enabled) override { return inner->setAudioPreprocessingEnabled(enabled); }
    int getXRunCount() const noexcept override { return inner->getXRunCount(); }
protected:
    std::unique_ptr<juce::AudioIODevice> inner;
private:
    void audioDeviceIOCallbackWithContext(const float* const* inputs, int numInputs, float* const* outputs, int numOutputs,
        int numSamples, const juce::AudioIODeviceCallbackContext& context) override
    {
        if (auto* t = target.load()) t->audioDeviceIOCallbackWithContext(inputs, numInputs, outputs, numOutputs, numSamples, context);
        else for (int i = 0; i < numOutputs; ++i) if (outputs[i] != nullptr && numSamples > 0) juce::FloatVectorOperations::clear(outputs[i], numSamples);
    }
    void audioDeviceAboutToStart(juce::AudioIODevice*) override { if (auto* t = target.load()) t->audioDeviceAboutToStart(this); }
    void audioDeviceStopped() override { if (auto* t = target.load()) t->audioDeviceStopped(); }
    void audioDeviceError(const juce::String& message) override { if (auto* t = target.load()) t->audioDeviceError(message); }
    std::atomic<juce::AudioIODeviceCallback*> target{ nullptr };
};

// The ALSA devices this program holds open (their probe says "busy", which
// must not hide them from the list). Message thread only.
inline std::multiset<juce::String>& heldAlsaDeviceNames() { static std::multiset<juce::String> names; return names; }
// The JACK devices this program holds open (PipeWire then holds those cards:
// their ALSA entries probe "busy" because of us, and stay listed).
inline std::multiset<juce::String>& heldJackDeviceNames() { static std::multiset<juce::String> names; return names; }
// ... and the ones closed lately (PipeWire keeps the card for a few seconds after
// the last client left): their ALSA entries stay listed for that long, so that a
// route can move from JACK to ALSA on the same card.
inline std::map<juce::String, double>& recentlyHeldJackNames() { static std::map<juce::String, double> names; return names; }
class TrackedAlsaDevice final : public ForwardingAudioIODevice
{
public:
    explicit TrackedAlsaDevice(std::unique_ptr<juce::AudioIODevice> device) : ForwardingAudioIODevice(std::move(device), "ALSA")
    { heldAlsaDeviceNames().insert(getName()); }
    // A card PipeWire still holds (a route just left it through JACK, or the
    // desktop used it a moment ago) is released a few seconds later: a failed
    // open of a card device is tried again for up to seven seconds.
    juce::String open(const juce::BigInteger& inputChannels, const juce::BigInteger& outputChannels, double sampleRate, int bufferSizeSamples) override
    {
        juce::String error = inner->open(inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        const bool card = !(getName().containsIgnoreCase("PipeWire") || getName().containsIgnoreCase("Pulseaudio") || getName().startsWithIgnoreCase("Default ALSA"));
        if (error.isEmpty() || !card) return error;
        const double start = juce::Time::getMillisecondCounterHiRes();
        CrashHandler::note("ALSA: " + getName() + " could not be opened (" + error + "): waiting for the card to be released");
        while (juce::Time::getMillisecondCounterHiRes() - start < 7000.0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            error = inner->open(inputChannels, outputChannels, sampleRate, bufferSizeSamples);
            if (error.isEmpty())
            {
                CrashHandler::note("ALSA: " + getName() + " opened after " + juce::String(juce::roundToInt(juce::Time::getMillisecondCounterHiRes() - start)) + " ms");
                return error;
            }
        }
        CrashHandler::note("ALSA: " + getName() + " still not available after 7 s: " + error);
        return error;
    }
    ~TrackedAlsaDevice() override
    {
        auto& names = heldAlsaDeviceNames();
        if (auto it = names.find(getName()); it != names.end()) names.erase(it);
    }
};

// ALSA advertises, for every card, the PCM templates of its configuration
// (surround40 ... surround71, front, rear, center_lfe, side, dmix, dsnoop)
// whether or not the card has those channels; most cannot be opened and the
// list drowns the real devices. This type wraps JUCE's ALSA type and lists
// only the real ones: the card's hardware device, the default and the
// digital/HDMI outputs. The saved device type name stays "ALSA".
class FilteredAlsaDeviceType final : public juce::AudioIODeviceType
{
public:
    FilteredAlsaDeviceType()
        : juce::AudioIODeviceType("ALSA"), inner(juce::AudioIODeviceType::createAudioIODeviceType_ALSA()) {}

    static bool isTemplate(const juce::String& name)
    {
        return name.containsIgnoreCase("Surround output") || name.containsIgnoreCase("Front output")
            || name.containsIgnoreCase("Rear output") || name.containsIgnoreCase("Center and Subwoofer")
            || name.containsIgnoreCase("Side output") || name.containsIgnoreCase("Direct sample mixing")
            || name.containsIgnoreCase("Direct sample snooping");
    }
    // The sound server's own entries: the PipeWire/PulseAudio plugins and the
    // "default" device, which lead to the server. Everything else is a card.
    static bool isSoundServerDevice(const juce::String& name)
    {
        return name.containsIgnoreCase("PipeWire") || name.containsIgnoreCase("Pulseaudio")
            || name.startsWithIgnoreCase("Default ALSA");
    }
    // ALSA's playback PCMs as JUCE names them (the description with "; " for
    // its line break), with the id to open: the same walk JUCE's ALSA type does.
    static std::map<juce::String, juce::String> outputIdsByName()
    {
        std::map<juce::String, juce::String> result;
        void** hints = nullptr;
        if (snd_device_name_hint(-1, "pcm", &hints) != 0) return result;
        for (void** h = hints; *h != nullptr; ++h)
        {
            auto get = [h](const char* key)
            {
                char* value = snd_device_name_get_hint(*h, key);
                const juce::String text = value != nullptr ? juce::String(juce::CharPointer_UTF8(value)) : juce::String();
                if (value != nullptr) free(value);
                return text;
            };
            const auto id = get("NAME"), description = get("DESC"), ioid = get("IOID");
            if (id.isEmpty() || id.startsWith("default:") || id.startsWith("sysdefault:") || id.startsWith("plughw:") || id == "null") continue;
            if (ioid == "Input" || id.startsWith("dsnoop")) continue;
            juce::String name = description.replace("\n", "; ");
            if (name.isEmpty()) name = id;
            result.emplace(name, id);   // the first id of a name wins, as in JUCE's list
        }
        snd_device_name_free_hint(hints);
        return result;
    }
    // Tries to open a playback PCM; empty when it opens, else why not.
    static juce::String probeOutput(const juce::String& id)
    {
        snd_pcm_t* pcm = nullptr;
        const int error = snd_pcm_open(&pcm, id.toRawUTF8(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (error < 0) return error == -EBUSY ? juce::String("held by another program") : juce::String(snd_strerror(error));
        snd_pcm_close(pcm);
        return {};
    }
    void scanForDevices() override
    {
        if (inner == nullptr) return;
        inner->scanForDevices();
        outputs.clear(); inputs.clear();
        const auto allOutputs = inner->getDeviceNames(false), allInputs = inner->getDeviceNames(true);
        // Every card device is tried: one that cannot be opened now (the sound
        // server or another program holds it) is left out, unless asked for. A
        // card's second PCM (S/PDIF, HDMI) often opens while its first is held,
        // with every channel of the card - it stays.
        const bool showAll = unusableAudioDevicesShown().load();
        const auto ids = showAll ? std::map<juce::String, juce::String>() : outputIdsByName();
        juce::StringArray hidden, hiddenNames;
        for (const auto& name : allOutputs)
        {
            if (isTemplate(name)) continue;
            if (!showAll && !isSoundServerDevice(name) && heldAlsaDeviceNames().count(name) == 0)
                if (auto it = ids.find(name); it != ids.end())
                    if (const auto why = probeOutput(it->second); why.isNotEmpty())
                    {
                        // Busy because PipeWire runs one of our own JACK routes on the
                        // card ("Komplete Audio 6" in "Komplete Audio 6, USB Audio; ..."):
                        // listed, the user may well want to move that route here.
                        bool ours = false;
                        for (const auto& jackName : heldJackDeviceNames())
                            if (jackName.isNotEmpty() && name.containsIgnoreCase(jackName)) ours = true;
                        for (const auto& [jackName, when] : recentlyHeldJackNames())
                            if (jackName.isNotEmpty() && name.containsIgnoreCase(jackName) && juce::Time::getMillisecondCounterHiRes() - when < 20000.0) ours = true;
                        if (!ours)
                        {
                            hidden.add(name + " (" + why + ")");
                            hiddenNames.add(name);
                            continue;
                        }
                    }
            outputs.add(name);
        }
        for (const auto& name : allInputs)
            if (!isTemplate(name) && !hiddenNames.contains(name)) inputs.add(name);
        hiddenAlsaDevicesNote() = hidden.isEmpty() ? juce::String()
            : "ALSA: " + juce::String(hidden.size()) + " device(s) that cannot be opened not listed (the settings editor can list them): "
              + hidden.joinIntoString("; ");
    }
    juce::StringArray getDeviceNames(bool wantInputNames = false) const override { return wantInputNames ? inputs : outputs; }
    int getDefaultDeviceIndex(bool forInput) const override
    {
        if (inner == nullptr) return -1;
        const auto& names = forInput ? inputs : outputs;
        const auto all = inner->getDeviceNames(forInput);
        const int inner_index = inner->getDefaultDeviceIndex(forInput);
        if (inner_index >= 0 && inner_index < all.size())
        {
            const int index = names.indexOf(all[inner_index]);
            if (index >= 0) return index;
        }
        return names.isEmpty() ? -1 : 0;
    }
    int getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const override
    {
        return device == nullptr ? -1 : (asInput ? inputs : outputs).indexOf(device->getName());
    }
    bool hasSeparateInputsAndOutputs() const override { return inner != nullptr && inner->hasSeparateInputsAndOutputs(); }
    juce::AudioIODevice* createDevice(const juce::String& outputDeviceName, const juce::String& inputDeviceName) override
    {
        auto* device = inner != nullptr ? inner->createDevice(outputDeviceName, inputDeviceName) : nullptr;
        return device != nullptr ? new TrackedAlsaDevice(std::unique_ptr<juce::AudioIODevice>(device)) : nullptr;
    }
private:
    std::unique_ptr<juce::AudioIODeviceType> inner;
    juce::StringArray outputs, inputs;
};

inline void installFilteredAlsaType(juce::AudioDeviceManager& manager)
{
    // JUCE's own ALSA type goes, the filtered one takes its place (same name,
    // so saved configurations still say "ALSA").
    for (auto* type : manager.getAvailableDeviceTypes())
        if (type != nullptr && type->getTypeName() == "ALSA" && dynamic_cast<FilteredAlsaDeviceType*>(type) == nullptr)
        {
            manager.removeAudioDeviceType(type);
            break;
        }
    auto filtered = std::make_unique<FilteredAlsaDeviceType>();
    filtered->scanForDevices();
    manager.addAudioDeviceType(std::move(filtered));
}
#endif

#if JUCE_LINUX
// JACK through PipeWire: JUCE loads "libjack.so.0" by name, which on most
// distributions is the jackd2 client library - useless unless a jackd runs.
// PipeWire ships its own libjack in a directory of its own (what pw-jack puts
// on the library path). Loading that one first, globally, makes JUCE's later
// dlopen of the same soname find it, so "JACK" in the device list connects to
// PipeWire and shows every channel of every card. Returns what was loaded.
#include <dlfcn.h>
inline void*& jackLibraryHandle() { static void* handle = nullptr; return handle; }
inline juce::String preloadPipeWireJack()
{
    static const char* const candidates[]{
        "/usr/lib/x86_64-linux-gnu/pipewire-0.3/jack/libjack.so.0",
        "/usr/lib64/pipewire-0.3/jack/libjack.so.0",
        "/usr/lib/pipewire-0.3/jack/libjack.so.0",
        "/usr/lib/aarch64-linux-gnu/pipewire-0.3/jack/libjack.so.0",
        "/usr/local/lib/pipewire-0.3/jack/libjack.so.0" };
    for (const char* path : candidates)
        if (juce::File(path).existsAsFile())
            if (void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL)) { jackLibraryHandle() = handle; return path; }
    jackLibraryHandle() = dlopen("libjack.so.0", RTLD_NOW | RTLD_GLOBAL);
    return {};
}

// PipeWire's buffer size (quantum) and sample rate are the server's, shared by
// every JACK client. pw-metadata sets them; 0 = unchanged. Empty = done.
inline juce::String pipeWireSetClock(int quantum, int rate)
{
    auto run = [](const juce::String& key, int value) -> juce::String
    {
        juce::StringArray args;
        args.add("pw-metadata"); args.add("-n"); args.add("settings"); args.add("0"); args.add(key); args.add(juce::String(value));
        juce::ChildProcess process;
        if (!process.start(args, juce::ChildProcess::wantStdOut | juce::ChildProcess::wantStdErr))
            return juce::String("pw-metadata could not be started (package pipewire-bin)");
        process.waitForProcessToFinish(5000);
        const auto output = process.readAllProcessOutput().trim();
        return process.getExitCode() == 0 ? juce::String() : (output.isNotEmpty() ? output : juce::String("pw-metadata failed"));
    };
    juce::String error;
    if (quantum > 0) error = run("clock.force-quantum", quantum);
    if (error.isEmpty() && rate > 0) error = run("clock.force-rate", rate);
    return error;
}

// The buffer size and rate the JACK server reports now, read through a client
// of its own that is never activated (an activated client opened while
// PipeWire changes its quantum got a buffer of 1 frame and a flood of empty
// callbacks that stalled the whole computer). False when no server answers.
inline bool jackServerClock(int& bufferSize, int& sampleRate)
{
    void* lib = jackLibraryHandle();
    if (lib == nullptr) return false;
    using ClientOpen = void* (*)(const char*, int, int*, ...);
    using GetSize = unsigned int (*)(void*);
    using ClientClose = int (*)(void*);
    auto open = (ClientOpen)dlsym(lib, "jack_client_open");
    auto getBuffer = (GetSize)dlsym(lib, "jack_get_buffer_size");
    auto getRate = (GetSize)dlsym(lib, "jack_get_sample_rate");
    auto close = (ClientClose)dlsym(lib, "jack_client_close");
    if (open == nullptr || getBuffer == nullptr || getRate == nullptr || close == nullptr) return false;
    int status = 0;
    void* client = open("LibreOrganClock", 0x01 /* JackNoStartServer */, &status);
    if (client == nullptr) return false;
    bufferSize = (int)getBuffer(client);
    sampleRate = (int)getRate(client);
    close(client);
    return true;
}

// Told after PipeWire's clock was changed by one route's device (the other
// JACK routes then reopen at the new size). Set by the main component.
inline std::function<void(juce::AudioIODevice*)>& pipeWireClockChangedHook()
{
    static std::function<void(juce::AudioIODevice*)> hook; return hook;
}

// A JACK device whose buffer size and sample rate can be chosen in the usual
// boxes: JUCE's own offers only the current values. A choice sets PipeWire's
// clock (pw-metadata), waits until the server reports it, and opens the
// client then - never during the change.
class PipeWireJackDevice final : public ForwardingAudioIODevice
{
public:
    explicit PipeWireJackDevice(std::unique_ptr<juce::AudioIODevice> device) : ForwardingAudioIODevice(std::move(device), "JACK")
    { heldJackDeviceNames().insert(getName()); }
    ~PipeWireJackDevice() override
    {
        auto& names = heldJackDeviceNames();
        if (auto it = names.find(getName()); it != names.end()) names.erase(it);
        recentlyHeldJackNames()[getName()] = juce::Time::getMillisecondCounterHiRes();
    }

    juce::Array<int> getAvailableBufferSizes() override
    {
        juce::Array<int> sizes{ 64, 128, 256, 512, 1024, 2048 };
        const int current = inner->getCurrentBufferSizeSamples();
        if (current > 0 && !sizes.contains(current)) { sizes.add(current); sizes.sort(); }
        return sizes;
    }
    juce::Array<double> getAvailableSampleRates() override
    {
        juce::Array<double> rates{ 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
        const double current = getCurrentSampleRate();
        if (current > 0.0 && !rates.contains(current)) { rates.add(current); rates.sort(); }
        return rates;
    }
    // A client that is not running yet reports no rate and no buffer size
    // (JUCE's device asks its own client); the device manager would then take
    // 44.1 kHz and the smallest size of the list when asked for "the driver's
    // own" values. The server's clock is what a JACK device runs at.
    double getCurrentSampleRate() override
    {
        const double rate = inner->getCurrentSampleRate();
        if (rate > 0.0) return rate;
        int serverBuffer = 0, serverRate = 0;
        return jackServerClock(serverBuffer, serverRate) && serverRate > 0 ? (double)serverRate : 0.0;
    }
    int getCurrentBufferSizeSamples() override
    {
        const int size = inner->getCurrentBufferSizeSamples();
        if (size > 0) return size;
        int serverBuffer = 0, serverRate = 0;
        return jackServerClock(serverBuffer, serverRate) && serverBuffer > 0 ? serverBuffer : 0;
    }
    int getDefaultBufferSize() override { return getCurrentBufferSizeSamples(); }
    juce::String open(const juce::BigInteger& inputChannels, const juce::BigInteger& outputChannels, double sampleRate, int bufferSizeSamples) override
    {
        const int wantBuffer = bufferSizeSamples > 0 ? bufferSizeSamples : 0;
        const int wantRate = sampleRate > 0.0 ? juce::roundToInt(sampleRate) : 0;
        int serverBuffer = 0, serverRate = 0;
        const bool known = jackServerClock(serverBuffer, serverRate);
        const bool changeBuffer = known && wantBuffer > 0 && wantBuffer != serverBuffer;
        const bool changeRate = known && wantRate > 0 && wantRate != serverRate;
        bool changed = false;
        if (changeBuffer || changeRate)
        {
            const auto error = pipeWireSetClock(changeBuffer ? wantBuffer : 0, changeRate ? wantRate : 0);
            if (error.isNotEmpty())
            {
                CrashHandler::note("JACK: PipeWire's clock could not be changed: " + error);
                return "PipeWire's buffer size / sample rate could not be changed: " + error;
            }
            CrashHandler::note("JACK: PipeWire clock " + juce::String(serverBuffer) + " frames / " + juce::String(serverRate) + " Hz -> "
                + juce::String(changeBuffer ? wantBuffer : serverBuffer) + " / " + juce::String(changeRate ? wantRate : serverRate)
                + " asked for " + getName());
            changed = true;
        }
        // Open, then wait for the client's buffer size: a fresh client learns it a
        // moment after activation (0 until then), later still while the server is
        // changing its quantum. The device is never handed over before the size
        // is sane; a client that got the old size (the change not through yet) is
        // thrown away and opened again. The last attempt takes any sane size.
        const double start = juce::Time::getMillisecondCounterHiRes();
        juce::String error;
        for (int attempt = 0; attempt < 5; ++attempt)
        {
            error = inner->open(inputChannels, outputChannels, sampleRate, bufferSizeSamples);
            if (error.isNotEmpty()) return error;
            int got = 0;
            for (int wait = 0; wait < 40 && (got = inner->getCurrentBufferSizeSamples()) < 16; ++wait)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            const int gotRate = juce::roundToInt(inner->getCurrentSampleRate());
            const bool sane = got >= 16;
            const bool asked = (!changeBuffer || got == wantBuffer) && (!changeRate || gotRate == wantRate);
            if (sane && (asked || attempt == 4))
            {
                if (changed)
                    CrashHandler::note("JACK: " + getName() + " open with " + juce::String(got) + " frames / " + juce::String(gotRate) + " Hz"
                        + (asked ? juce::String() : juce::String(" (not what was asked for)")) + " after "
                        + juce::String(juce::roundToInt(juce::Time::getMillisecondCounterHiRes() - start)) + " ms");
                error.clear();
                break;
            }
            inner->close();
            CrashHandler::note("JACK: " + getName() + " reported " + juce::String(got) + " frames / " + juce::String(gotRate) + " Hz"
                + (changed ? " (asked: " + juce::String(changeBuffer ? wantBuffer : got) + " / " + juce::String(changeRate ? wantRate : gotRate) + ")" : juce::String())
                + ", opened again");
            error = "JACK reported a buffer of " + juce::String(got) + " frames";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        if (changed && error.isEmpty() && pipeWireClockChangedHook())
        {
            juce::AudioIODevice* self = this;
            juce::MessageManager::callAsync([self] { if (auto& hook = pipeWireClockChangedHook()) hook(self); });
        }
        return error;
    }
};

// JUCE's JACK type lists every client that owns a port, MIDI bridges and MIDI
// devices included ("Midi-Bridge", "BLE MIDI 1" - routes "opened" on them run
// with a buffer of 1 frame). This wrapper keeps the clients that own AUDIO
// ports in the wanted direction, asked of the JACK server through the library
// loaded above. The type name stays "JACK" for the saved configurations.
class FilteredJackDeviceType final : public juce::AudioIODeviceType
{
public:
    FilteredJackDeviceType() : juce::AudioIODeviceType("JACK"), inner(juce::AudioIODeviceType::createAudioIODeviceType_JACK()) {}

    void scanForDevices() override
    {
        if (inner == nullptr) return;
        inner->scanForDevices();
        const auto audioOutputs = audioClients(true);    // clients with audio INPUT ports: where we can play
        const auto audioInputs = audioClients(false);
        outputs.clear(); inputs.clear();
        const bool all = unusableAudioDevicesShown().load();   // the "list unusable devices" switch: every client
        for (const auto& name : inner->getDeviceNames(false)) if (all || audioOutputs.contains(name)) outputs.add(name);
        for (const auto& name : inner->getDeviceNames(true)) if (all || audioInputs.contains(name)) inputs.add(name);
    }
    juce::StringArray getDeviceNames(bool wantInputNames = false) const override { return wantInputNames ? inputs : outputs; }
    int getDefaultDeviceIndex(bool forInput) const override { return (forInput ? inputs : outputs).isEmpty() ? -1 : 0; }
    int getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const override
    {
        return device == nullptr ? -1 : (asInput ? inputs : outputs).indexOf(device->getName());
    }
    bool hasSeparateInputsAndOutputs() const override { return true; }
    juce::AudioIODevice* createDevice(const juce::String& outputDeviceName, const juce::String& inputDeviceName) override
    {
        auto* device = inner != nullptr ? inner->createDevice(outputDeviceName, inputDeviceName) : nullptr;
        return device != nullptr ? new PipeWireJackDevice(std::unique_ptr<juce::AudioIODevice>(device)) : nullptr;
    }
private:
    // The clients owning audio ports of one direction, by the JACK API itself
    // (only the handful of functions needed, taken from the loaded library).
    static juce::StringArray audioClients(bool playbackPorts)
    {
        juce::StringArray clients;
        void* lib = jackLibraryHandle();
        if (lib == nullptr) return clients;
        using ClientOpen = void* (*)(const char*, int, int*, ...);
        using GetPorts = const char** (*)(void*, const char*, const char*, unsigned long);
        using Free = void (*)(void*);
        using ClientClose = int (*)(void*);
        auto open = (ClientOpen)dlsym(lib, "jack_client_open");
        auto getPorts = (GetPorts)dlsym(lib, "jack_get_ports");
        auto freePorts = (Free)dlsym(lib, "jack_free");
        auto close = (ClientClose)dlsym(lib, "jack_client_close");
        if (open == nullptr || getPorts == nullptr || freePorts == nullptr || close == nullptr) return clients;
        int status = 0;
        void* client = open("LibreOrganScan", 0x01 /* JackNoStartServer */, &status);
        if (client == nullptr) return clients;
        // Playback ports of the server are INPUT ports (we write into them): JackPortIsInput = 1, JackPortIsOutput = 2.
        if (const char** ports = getPorts(client, nullptr, "32 bit float mono audio", playbackPorts ? 1UL : 2UL))
        {
            for (const char** p = ports; *p != nullptr; ++p)
            {
                const juce::String name = juce::CharPointer_UTF8(*p);
                clients.addIfNotAlreadyThere(name.upToFirstOccurrenceOf(":", false, false));
            }
            freePorts(ports);
        }
        close(client);
        return clients;
    }
    std::unique_ptr<juce::AudioIODeviceType> inner;
    juce::StringArray outputs, inputs;
};

inline void installFilteredJackType(juce::AudioDeviceManager& manager)
{
    for (auto* type : manager.getAvailableDeviceTypes())
        if (type != nullptr && type->getTypeName() == "JACK" && dynamic_cast<FilteredJackDeviceType*>(type) == nullptr)
        {
            manager.removeAudioDeviceType(type);
            break;
        }
    auto filtered = std::make_unique<FilteredJackDeviceType>();
    filtered->scanForDevices();
    manager.addAudioDeviceType(std::move(filtered));
}
#endif

inline void installPlatformAudioTypes(juce::AudioDeviceManager& manager)
{
    addAggregatedWASAPIType(manager);
   #if JUCE_LINUX
    installFilteredAlsaType(manager);
    installFilteredJackType(manager);
   #endif
}
