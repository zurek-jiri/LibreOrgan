#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

//==============================================================================
// Offline engine exercise, started with:
//     OrganSampler.exe --selftest "<division sample folder>"
// Loads the folder as division 0, plays chords/tutti/repetitions through the
// real render path (all worker threads) and writes timing and sanity results
// to <AppData>/OrganSampler/selftest.txt. No audio device is opened.
//==============================================================================
namespace EngineSelfTest
{
    //--------------------------------------------------------------------------
    // ASIO driver probe: opens the first ASIO device whose name contains
    // 'deviceFilter' with several buffer sizes and measures how the driver
    // really calls back (frames per callback, period), including the
    // "close and reopen with another size" sequence used after a control panel change.
    struct CallbackProbe final : public juce::AudioIODeviceCallback
    {
        std::atomic<int> callbacks{ 0 }, minFrames{ 1 << 30 }, maxFrames{ 0 }, channels{ 0 };
        std::atomic<int64_t> firstTicks{ 0 }, lastTicks{ 0 }, totalFrames{ 0 };
        std::atomic<double> maxGapMs{ 0.0 };
        void audioDeviceAboutToStart(juce::AudioIODevice*) override {}
        void audioDeviceStopped() override {}
        void audioDeviceIOCallbackWithContext(const float* const*, int, float* const* outputs, int numOutputs,
            int numSamples, const juce::AudioIODeviceCallbackContext&) override
        {
            const int64_t now = juce::Time::getHighResolutionTicks();
            for (int c = 0; c < numOutputs; ++c)
                if (outputs[c] != nullptr) juce::FloatVectorOperations::clear(outputs[c], numSamples);
            if (callbacks.load() == 0) firstTicks.store(now);
            else
            {
                const double gapMs = 1000.0 * (double)(now - lastTicks.load()) / (double)juce::Time::getHighResolutionTicksPerSecond();
                if (gapMs > maxGapMs.load()) maxGapMs.store(gapMs);
            }
            lastTicks.store(now);
            callbacks.fetch_add(1);
            totalFrames.fetch_add(numSamples);
            channels.store(numOutputs);
            int expected = minFrames.load(); while (numSamples < expected && !minFrames.compare_exchange_weak(expected, numSamples)) {}
            expected = maxFrames.load(); while (numSamples > expected && !maxFrames.compare_exchange_weak(expected, numSamples)) {}
        }
        void reset() { callbacks = 0; minFrames = 1 << 30; maxFrames = 0; firstTicks = 0; lastTicks = 0; totalFrames = 0; maxGapMs = 0.0; }
    };

    inline juce::String runAsioProbe(const juce::String& deviceFilter)
    {
        juce::String report;
        report << "ASIO driver probe\n";
        juce::AudioDeviceManager manager;
        manager.initialiseWithDefaultDevices(0, 2);
        manager.setCurrentAudioDeviceType("ASIO", true);
        juce::AudioIODeviceType* asioType = nullptr;
        for (auto* type : manager.getAvailableDeviceTypes())
            if (type->getTypeName().equalsIgnoreCase("ASIO")) asioType = type;
        if (asioType == nullptr) return report + "No ASIO support.\n";
        asioType->scanForDevices();
        juce::String deviceName;
        for (const auto& name : asioType->getDeviceNames(false))
            if (deviceFilter.isEmpty() || name.containsIgnoreCase(deviceFilter)) { deviceName = name; break; }
        if (deviceName.isEmpty()) return report + "No ASIO device matching '" + deviceFilter + "'.\n";
        report << "Device: " << deviceName << "\n";

        // ORGAN_ASIO_PANEL_WASAPI=<name part>: additionally keep a Windows Audio
        // device open for the whole test, as the application does (its WDM
        // streams usually run on the same interface as the ASIO driver).
        std::unique_ptr<juce::AudioDeviceManager> wasapiManager;
        CallbackProbe wasapiProbe;
        if (const char* wasapiFilter = std::getenv("ORGAN_ASIO_PANEL_WASAPI"))
        {
            wasapiManager = std::make_unique<juce::AudioDeviceManager>();
            wasapiManager->initialiseWithDefaultDevices(0, 2);
            wasapiManager->setCurrentAudioDeviceType("Windows Audio", true);
            juce::String found;
            for (auto* type : wasapiManager->getAvailableDeviceTypes())
                if (type->getTypeName() == "Windows Audio")
                {
                    type->scanForDevices();
                    for (const auto& name : type->getDeviceNames(false))
                        if (name.containsIgnoreCase(juce::String(wasapiFilter))) { found = name; break; }
                }
            auto setup = wasapiManager->getAudioDeviceSetup();
            setup.outputDeviceName = found;
            setup.inputDeviceName.clear();
            setup.useDefaultOutputChannels = true;
            const auto wasapiError = wasapiManager->setAudioDeviceSetup(setup, true);
            wasapiManager->addAudioCallback(&wasapiProbe);
            auto* companion = wasapiManager->getCurrentAudioDevice();
            report << "Companion Windows Audio device: " << (companion != nullptr
                ? companion->getName() + " buffer " + juce::String(companion->getCurrentBufferSizeSamples())
                : "NONE (" + wasapiError + ")") << "\n";
        }

        CallbackProbe probe;
        // ORGAN_ASIO_PANEL_PUMP: run the message loop while waiting so JUCE's own
        // reset handling (kAsioResetRequest -> timer -> in-place reopen) happens
        // exactly as in the application.  ORGAN_ASIO_PANEL_ALLCH: open every
        // output channel like the application does.
        const bool pumpMessages = std::getenv("ORGAN_ASIO_PANEL_PUMP") != nullptr;
        const bool allChannels = std::getenv("ORGAN_ASIO_PANEL_ALLCH") != nullptr;
        auto wait = [&](int ms)
        {
            if (!pumpMessages) { juce::Thread::sleep(ms); return; }
            // Dispatch the Windows message queue (JUCE timers and async calls
            // arrive through it) so the driver's reset request is handled.
            const auto until = juce::Time::getMillisecondCounter() + (juce::uint32)ms;
            while (juce::Time::getMillisecondCounter() < until)
            {
               #if JUCE_WINDOWS
                MSG msg;
                while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
               #endif
                juce::Thread::sleep(5);
            }
        };
        auto measure = [&](const juce::String& label, int requestedBuffer, bool closeFirst)
        {
            auto setup = manager.getAudioDeviceSetup();
            setup.outputDeviceName = deviceName;
            setup.inputDeviceName.clear();
            setup.bufferSize = requestedBuffer;
            setup.useDefaultOutputChannels = !allChannels;
            if (allChannels) { setup.outputChannels.clear(); setup.outputChannels.setRange(0, 64, true); }
            if (closeFirst) manager.closeAudioDevice();
            const auto error = manager.setAudioDeviceSetup(setup, true);
            auto* device = manager.getCurrentAudioDevice();
            report << "\n" << label << ": requested buffer " << requestedBuffer
                   << (error.isNotEmpty() ? "  ERROR: " + error : juce::String());
            if (device == nullptr) { report << "  (no device)\n"; return; }
            const int actual = device->getCurrentBufferSizeSamples();
            const double rate = device->getCurrentSampleRate();
            juce::String sizes;
            for (const int s : device->getAvailableBufferSizes()) sizes << s << " ";
            report << "\n  device reports: buffer " << actual << ", preferred " << device->getDefaultBufferSize()
                   << ", rate " << juce::String(rate, 0) << ", available sizes: " << sizes
                   << "\n  latency out " << device->getOutputLatencyInSamples() << " frames";
            report << "\n  manager setup says buffer " << manager.getAudioDeviceSetup().bufferSize;
            probe.reset();
            manager.addAudioCallback(&probe);
            wait(2500);
            manager.removeAudioCallback(&probe);
            const int n = probe.callbacks.load();
            const double seconds = n > 1 ? (double)(probe.lastTicks.load() - probe.firstTicks.load())
                / (double)juce::Time::getHighResolutionTicksPerSecond() : 0.0;
            const double measuredPeriodMs = n > 1 ? 1000.0 * seconds / (n - 1) : 0.0;
            const double expectedPeriodMs = 1000.0 * actual / juce::jmax(1.0, rate);
            report << "\n  callbacks: " << n << " in " << juce::String(seconds, 2) << " s, frames per callback "
                   << probe.minFrames.load() << ".." << probe.maxFrames.load() << ", outputs " << probe.channels.load()
                   << "\n  measured period " << juce::String(measuredPeriodMs, 3) << " ms vs expected "
                   << juce::String(expectedPeriodMs, 3) << " ms (max gap " << juce::String(probe.maxGapMs.load(), 2) << " ms)"
                   << "  -> " << (n < 2 ? "NO CALLBACKS" : std::abs(measuredPeriodMs - expectedPeriodMs) < expectedPeriodMs * 0.1
                       ? "consistent" : "*** PERIOD DOES NOT MATCH BUFFER SIZE ***") << "\n";
        };

        if (const char* env = std::getenv("ORGAN_ASIO_PANEL_SECONDS"))
        {
            // Interactive variant: open the driver's control panel and watch what
            // the running device does while its buffer size is changed there.
            // The message thread sleeps, so JUCE's own reset handling cannot run
            // and the raw driver behaviour is visible.
            const int seconds = juce::jlimit(5, 180, juce::String(env).getIntValue());
            // While watching, creating %TEMP%\asio-probe-reopen.txt makes the probe do
            // the application's "reopen fresh" (close + new instance at driver default).
            const auto trigger = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("asio-probe-reopen.txt");
            trigger.deleteFile();
            measure("Open with driver default", 0, false);
            auto* device = manager.getCurrentAudioDevice();
            if (device == nullptr) return report + "no device\n";
            const int beforeBuffer = device->getCurrentBufferSizeSamples();
            probe.reset();
            manager.addAudioCallback(&probe);
            report << "\nOpening the driver control panel; watching for " << seconds << " s (change the buffer size there)"
                   << (pumpMessages ? ", message loop running (JUCE may reset the device itself)" : ", message loop blocked")
                   << ":\n";
            device->showControlPanel();
            int lastCallbacks = 0;
            int64_t lastTicks = juce::Time::getHighResolutionTicks();
            for (int i = 0; i < seconds; ++i)
            {
                for (int step = 0; step < 5; ++step)
                {
                    wait(200);
                    if (trigger.existsAsFile())
                    {
                        trigger.deleteFile();
                        manager.removeAudioCallback(&probe);
                        report << "  -> trigger at t=" << (i + 1) << "s: application-style reopen (close + fresh instance at driver default)";
                        measure("Reopen after panel change", 0, true);
                        device = manager.getCurrentAudioDevice();
                        if (device == nullptr) return report + "no device after reopen\n";
                        probe.reset();
                        manager.addAudioCallback(&probe);
                        lastCallbacks = 0;
                        lastTicks = juce::Time::getHighResolutionTicks();
                    }
                }
                const int callbacks = probe.callbacks.load();
                const int64_t now = juce::Time::getHighResolutionTicks();
                const double elapsed = (double)(now - lastTicks) / (double)juce::Time::getHighResolutionTicksPerSecond();
                const int delta = callbacks - lastCallbacks;
                const double perSecond = delta / juce::jmax(0.001, elapsed);
                const int believed = device->getCurrentBufferSizeSamples();
                const double expectedPerSecond = device->getCurrentSampleRate() / juce::jmax(1, believed);
                report << "  t=" << (i + 1) << "s  callbacks/s " << juce::String(perSecond, 1)
                       << "  frames/callback " << probe.minFrames.load() << ".." << probe.maxFrames.load()
                       << "  JUCE believes buffer " << believed << " (setup " << manager.getAudioDeviceSetup().bufferSize << ")"
                       << "  max gap " << juce::String(probe.maxGapMs.load(), 1) << " ms"
                       << (delta > 0 && std::abs(perSecond - expectedPerSecond) > expectedPerSecond * 0.1 ? "  *** RATE MISMATCH ***" : "")
                       << "\n";
                lastCallbacks = callbacks;
                lastTicks = now;
                probe.maxGapMs.store(0.0);
                probe.minFrames.store(1 << 30);
                probe.maxFrames.store(0);
            }
            manager.removeAudioCallback(&probe);
            report << "Buffer before panel: " << beforeBuffer << "\n";
            measure("Close + fresh instance at driver default (after panel)", 0, true);
            measure("Second fresh instance", 0, true);
            if (wasapiManager != nullptr)
            {
                report << "\nCompanion Windows Audio callbacks during the test: " << wasapiProbe.callbacks.load()
                       << " (frames " << wasapiProbe.minFrames.load() << ".." << wasapiProbe.maxFrames.load() << ")\n";
                wasapiManager->removeAudioCallback(&wasapiProbe);
                wasapiManager->closeAudioDevice();
                measure("Fresh instance after closing the Windows Audio device", 0, true);
            }
            manager.closeAudioDevice();
            report << "\nASIO PANEL PROBE COMPLETED\n";
            return report;
        }

        measure("Open with driver default", 0, false);
        const int preferred = manager.getCurrentAudioDevice() != nullptr ? manager.getCurrentAudioDevice()->getDefaultBufferSize() : 256;
        measure("Reopen (same instance) at 2x", preferred * 2, false);
        measure("Close + fresh instance at 4x", preferred * 4, true);
        measure("Close + fresh instance at driver default", 0, true);
        measure("Reopen (same instance) at half", juce::jmax(16, preferred / 2), false);
        measure("Close + fresh instance at driver default again", 0, true);
        manager.closeAudioDevice();
        report << "\nASIO PROBE COMPLETED\n";
        return report;
    }

    struct BlockStats
    {
        double totalMs = 0.0, maxMs = 0.0;
        int blocks = 0;
        float peak = 0.0f;
        bool finite = true;
        int maxPolyphony = 0;
    };

    inline juce::String run(const std::vector<juce::File>& folders, const std::vector<juce::String>& perspectivePatterns,
        const std::vector<juce::String>& ignoredPatterns)
    {
        juce::String report;
        report << "OrganSampler engine self test\n";
        for (const auto& folder : folders) report << "Folder: " << folder.getFullPathName() << "\n";
        report << "CPU: " << juce::SystemStats::getCpuModel() << " (" << juce::SystemStats::getNumCpus() << " logical)\n";
        if (folders.empty()) return report + "No folders given.\n";

        SamplerEngine engine;
        // Always 32-bit float, like the application. ORGAN_SELFTEST_INT16=1 for the 16-bit path.
        engine.setSampleStorageFloat(true);
        if (const char* env = std::getenv("ORGAN_SELFTEST_INT16"))
            engine.setSampleStorageFloat(juce::String(env).getIntValue() == 0);
        // ORGAN_COMPRESS_SAMPLES=1: the 16-bit samples packed in RAM (0.8.145) - the
        // same test, rendered through the unpacking path; the figures must not move.
        if (const char* env = std::getenv("ORGAN_COMPRESS_SAMPLES"); env != nullptr && juce::String(env).getIntValue() != 0)
        {
            engine.setSampleStorageFloat(false);
            engine.setCompressSamples(true);
            report << "Samples packed in RAM (ORGAN_COMPRESS_SAMPLES=1, 16-bit storage).\n";
        }
        engine.setPerspectivePatterns(perspectivePatterns);
        engine.setIgnoredPatterns(ignoredPatterns);
        engine.setMaxPolyphony(100000);
        if (const char* env = std::getenv("ORGAN_RENDER_THREADS"))
            engine.setRenderThreadCount(juce::String(env).getIntValue());
        if (const char* env = std::getenv("ORGAN_NO_SIMD"))
            engine.setSimdDisabled(juce::String(env).getIntValue() != 0);
        // ORGAN_SINC_FROM_HZ=<Hz>: the sinc playback kernel from this sounding pitch up (0 = every pipe; 0.8.164).
        if (const char* env = std::getenv("ORGAN_SINC_FROM_HZ"))
        {
            engine.setSincKernelFromHz(juce::String(env).getFloatValue());
            report << "Sinc kernel from " << juce::String(engine.getSincKernelFromHz(), 0) << " Hz (ORGAN_SINC_FROM_HZ)\n";
        }
        if (const char* env = std::getenv("ORGAN_NO_TONEFILTER"))
            if (juce::String(env).getIntValue() != 0) engine.setHarmonicFilterDb(0.0f, 0.0f, 0.0f);
        // ORGAN_SELFTEST_RELEASE_CUT=<percent>: release cutting (shorter releases, more polyphony).
        if (const char* env = std::getenv("ORGAN_SELFTEST_RELEASE_CUT"))
            engine.setReleaseCutPercent((float)juce::String(env).getDoubleValue());
        report << "Release cutting: " << juce::String(engine.getReleaseCutPercent(), 0) << " %\n";
        // ORGAN_SELFTEST_IR=<wav>: every route gets this impulse response at 50 % wet
        // (measures the cost of the per-route convolution reverb).
        const juce::File selfTestIr(std::getenv("ORGAN_SELFTEST_IR") != nullptr ? juce::String(std::getenv("ORGAN_SELFTEST_IR")) : juce::String());
        report << "Render threads: " << engine.getRenderWorkerCount() + 1 << " (incl. audio thread)"
               << "   SIMD kernel: " << engine.currentKernelName()
               << "   harmonic filter swell/wind/tremulant: " << juce::String(engine.getHarmonicFilterSwellDb(), 1) << "/"
               << juce::String(engine.getHarmonicFilterWindDb(), 1) << "/" << juce::String(engine.getHarmonicFilterTremulantDb(), 1) << " dB"
               << "   sample storage: " << (engine.getSampleStorageFloat() ? "32-bit float" : "16-bit") << "\n";
        report << SamplerEngine::runKernelSelfCheck() << "\n";

        // ORGAN_SELFTEST_DIVISIONS=N loads the same folder into N divisions and
        // plays every note on all of them, to measure very high polyphony.
        int divisions = juce::jlimit(1, SamplerEngine::maxManuals, (int)folders.size());
        if (const char* env = std::getenv("ORGAN_SELFTEST_DIVISIONS"))
            if (folders.size() == 1) divisions = juce::jlimit(1, SamplerEngine::maxManuals, juce::String(env).getIntValue());
        const double loadStart = juce::Time::getMillisecondCounterHiRes();
        // ORGAN_SELFTEST_COMPOSITE=1: every folder tree goes into division 0
        // (the second and later ones appended), like a composite organ.
        const bool composite = std::getenv("ORGAN_SELFTEST_COMPOSITE") != nullptr && folders.size() > 1;
        if (composite)
        {
            divisions = 1;
            for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual) engine.setManualCompass(manual, 36, 96);   // like the application
            for (size_t i = 0; i < folders.size(); ++i)
                engine.loadManualFolder(folders[i], 0, 48000.0, {}, i > 0);
            report << "Composite division 0 from " << (int)folders.size() << " folder trees\n";
        }
        else
        {
            for (int manual = 0; manual < SamplerEngine::maxManuals; ++manual) engine.setManualCompass(manual, 36, 96);   // like the application
            for (int manual = 0; manual < divisions; ++manual)
                engine.loadManualFolder(folders[(size_t)juce::jmin(manual, (int)folders.size() - 1)], manual, 48000.0);
        }
        engine.commitPendingDivisions();
        if (composite)
        {
            std::map<juce::String, int> perTree;
            for (auto* stop : engine.getStops()) ++perTree[stop->tree.getFileName()];
            for (const auto& entry : perTree) report << "  tree " << entry.first << ": " << entry.second << " stops\n";
            for (auto* stop : engine.getStops())
                if (stop->name.endsWith(")")) report << "  renamed duplicate: " << stop->name << " from " << stop->tree.getFileName() << "\n";
        }
        const double loadMs = juce::Time::getMillisecondCounterHiRes() - loadStart;
        report << "Divisions loaded with this folder: " << divisions << "\n";
        int rankCount = 0, sampleCount = 0;
        std::map<int, int> resampledFrom;   // source rate -> samples resampled to the base rate
        for (auto* stop : engine.getStops())
        {
            ++rankCount;
            rankCount += (int)stop->ranks.size();
            auto countSamples = [&](const Stop* rank)
            {
                auto count = [&](const std::vector<std::unique_ptr<Sample>>& list)
                {
                    sampleCount += (int)list.size();
                    for (const auto& sample : list)
                        if (sample != nullptr && sample->sourceSampleRate > 0.0 && std::abs(sample->sourceSampleRate - sample->sampleRate) > 0.5)
                            ++resampledFrom[juce::roundToInt(sample->sourceSampleRate)];
                };
                for (int note = 0; note < 128; ++note) count(rank->samplesByNote[(size_t)note]);
                for (const auto& layer : rank->velocityLayers)
                    for (int note = 0; note < 128; ++note) count(layer.samplesByNote[(size_t)note]);
                for (const auto& layer : rank->releaseLayers)
                    for (int note = 0; note < 128; ++note) count(layer.samplesByNote[(size_t)note]);
            };
            countSamples(stop);
            for (const auto& rank : stop->ranks) countSamples(rank.get());
        }
        report << "Loaded " << engine.getStops().size() << " stops / " << rankCount << " ranks / "
               << sampleCount << " samples in " << juce::String(loadMs / 1000.0, 1) << " s\n";
        for (const auto& entry : resampledFrom)
            report << "Resampled to 48 kHz on loading: " << entry.second << " samples from " << entry.first << " Hz\n";
        if (engine.getStops().isEmpty()) { report << "NO STOPS FOUND - aborting\n"; return report; }
        // Tuning analysis (smpl pitches): recorded a1 per rank and the corrections
        // to 440 Hz equal temperament at a few keys.
        report << "Tuning analysis:\n";
        for (auto* stop : engine.getStops())
        {
            auto describeRank = [&](const Stop* rank, const juce::String& label)
            {
                report << "  " << label << " <" << rank->folder.getFullPathName() << ">";
                {
                    // The first three measured pitches as read (semitones above the key).
                    int shown = 0;
                    for (int note = 0; note < 128 && shown < 3; ++note)
                        if (rank->hasNote(note) && rank->samplesByNote[(size_t)note][0] != nullptr
                            && std::isfinite(rank->samplesByNote[(size_t)note][0]->smplPitch))
                        {
                            report << (shown == 0 ? " raw" : "") << " k" << note << "="
                                   << juce::String(rank->samplesByNote[(size_t)note][0]->smplPitch - note, 3);
                            ++shown;
                        }
                }
                if (!rank->tuningInfoAvailable) { report << ": no usable smpl pitch data\n"; return; }
                if (std::abs(rank->detectedCelesteBeats) > 1.0e-3f)
                {
                    // What the render applies at key 60 (cents, on top of the recorded
                    // pitch): the 440 Hz correction, the undulation table as loaded
                    // (detected beats), and their sum; then the sum with undulation 0.
                    report << "; celeste " << juce::String(rank->detectedCelesteBeats, 2) << " beats/s"
                           << " [k60: correction " << juce::String(rank->tuning440Cents[60], 1)
                           << ", undulation " << juce::String(rank->undulationCents[60], 1)
                           << " (" << juce::String(rank->undulationBeats, 2) << " beats), total "
                           << juce::String(rank->tuningCentsFor(60, true), 1) << "; with 0 beats the total would be "
                           << juce::String(rank->tuning440Cents[60], 1) << "]";
                }
                report << ": recorded a1 = " << juce::String(rank->measuredDiapasonHz, 2) << " Hz; corrections (cents)";
                for (const int note : { 36, 48, 60, 72, 84, 96 })
                    if (rank->hasNote(note))
                        report << "  k" << note << "=" << juce::String(rank->tuning440Cents[(size_t)note], 1);
                // Footage offsets actually detected (pitch - key - correction), rounded.
                std::map<int, int> offsets;
                for (int note = 0; note < 128; ++note)
                    if (rank->hasNote(note) && rank->samplesByNote[(size_t)note][0] != nullptr
                        && std::isfinite(rank->samplesByNote[(size_t)note][0]->smplPitch))
                        ++offsets[juce::roundToInt(rank->samplesByNote[(size_t)note][0]->smplPitch - note
                            + rank->tuning440Cents[(size_t)note] / 100.0f)];
                report << "; offsets";
                for (const auto& entry : offsets) report << " " << entry.first << "st x" << entry.second;
                if (rank->footageBreaks.isNotEmpty()) report << "; breaks: " << rank->footageBreaks;
                else if (rank->footageKnown) report << "; footage " << SamplerEngine::footageName(rank->footageSemitones);
                if (rank->footageSounding.isNotEmpty())
                {
                    // The lowest sounding harmonic and the levels behind that decision.
                    report << "; sounding: " << rank->footageSounding << " [harmonic " << rank->soundingHarmonic
                           << "; lowest pipe, harmonics 1..8 dB below the strongest:";
                    for (int k = 0; k < 8; ++k) report << " " << juce::String(rank->lowestNoteHarmonicDb[(size_t)k], 1);
                    report << "; between the harmonics " << juce::String(rank->lowestNoteResidualDb, 1) << " dB]";
                }
                if (rank->measuredTemperamentValid)
                {
                    static const char* names[12]{ "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
                    report << "; temperament (cents vs ET, mean removed):";
                    for (int pc = 0; pc < 12; ++pc)
                        report << " " << names[pc] << juce::String(rank->measuredTemperamentCents[(size_t)pc], 1);
                }
                report << "\n";
            };
            describeRank(stop, stop->name + " [" + stop->perspective + "]");
            for (const auto& rank : stop->ranks) describeRank(rank.get(), stop->name + " [" + rank->perspective + "]");
        }

        for (int manual = 0; manual < divisions; ++manual)
        {
            for (const auto& perspective : engine.getPerspectivesForManual(manual))
            {
                engine.setPerspectiveOutputChannels(manual, perspective, { 0, 1 });
                engine.setPerspectiveSampleRate(manual, perspective, 48000.0);
                if (selfTestIr.existsAsFile())
                {
                    engine.setPerspectiveReverb(manual, perspective, selfTestIr, 0.5f);
                    const auto info = engine.getPerspectiveReverbInfo(manual, perspective);
                    if (manual == 0) report << "IR reverb on every route: " << selfTestIr.getFileName() << " (" << info.channels
                        << " ch, " << juce::String(info.seconds, 2) << " s" << (info.error.isNotEmpty() ? ", ERROR " + info.error : "") << ")\n";
                }
            }
            engine.setManualTremulant(manual, true, 4.4f, 8.0f, 260.0f);
            engine.setManualExpressionGain(manual, 0.7f);
        }
        // ORGAN_IR_LIGHT=1: the IR reverbs in Light processing (the automatic choice on a weak computer).
        if (const char* lightEnv = std::getenv("ORGAN_IR_LIGHT"); lightEnv != nullptr && juce::String(lightEnv).getIntValue() != 0)
            engine.setReverbProcessingLight(true);
        if (selfTestIr.existsAsFile())
        {
            const auto summary = engine.getReverbSummary();
            report << "IR processing: " << (summary.light ? "Light" : "Full") << " - " << summary.routes << " route(s), "
                   << juce::String(summary.totalCostMsPerSecond, 1) << " ms of one thread per second of audio (worst route "
                   << juce::String(summary.worstCostMsPerSecond, 1) << " ms; " << juce::String(summary.irSecondsPerSecond, 1)
                   << " IR-channel-seconds per second; at full quality about " << juce::String(summary.estimatedFullMsPerSecond, 1) << " ms)\n";
        }
        for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
        auto noteOnAll = [&](int note) { for (int m = 0; m < divisions; ++m) engine.noteOn(note, 127, m); };
        auto noteOffAll = [&](int note) { for (int m = 0; m < divisions; ++m) engine.noteOff(note, m); };

        int blockSize = 256;   // ORGAN_SELFTEST_BLOCK overrides (e.g. 64 for small ASIO buffers)
        if (const char* env = std::getenv("ORGAN_SELFTEST_BLOCK"))
            blockSize = juce::jlimit(16, 4096, juce::String(env).getIntValue());
        report << "Block size: " << blockSize << " frames\n";
        for (int manual = 0; manual < divisions; ++manual)
            for (const auto& perspective : engine.getPerspectivesForManual(manual))
            {
                engine.setPerspectiveBlockSize(manual, perspective, blockSize);   // reverb partitions follow the block
                // Every route "has a device" here (the test renders them all itself):
                // a route without one gets no voices since 0.8.126.
                engine.setPerspectiveDeviceKey(manual, perspective, "selftest|" + perspective + "|" + juce::String(manual));
            }
        juce::AudioBuffer<float> buffer(2, blockSize);
       #if JUCE_WINDOWS
        // Measure like a real audio callback thread would run.
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
       #endif
        float maxTickMs = 0.0f;
        std::vector<float>* capture = nullptr;   // when set, the rendered left channel is appended here
        auto renderBlocks = [&](int blocks, BlockStats& stats)
        {
            blocks = blocks * 256 / blockSize;   // keep the phase duration constant
            for (int b = 0; b < blocks; ++b)
            {
                buffer.clear();
                const int64_t t0 = juce::Time::getHighResolutionTicks();
                engine.renderBlockForManual(buffer, 0.1f, -1);   // every division's routes
                if (capture != nullptr) capture->insert(capture->end(), buffer.getReadPointer(0), buffer.getReadPointer(0) + blockSize);
                const double ms = 1000.0 * (double)(juce::Time::getHighResolutionTicks() - t0)
                    / (double)juce::Time::getHighResolutionTicksPerSecond();
                stats.totalMs += ms;
                stats.maxMs = juce::jmax(stats.maxMs, ms);
                ++stats.blocks;
                stats.maxPolyphony = juce::jmax(stats.maxPolyphony, engine.getCurrentPolyphony());
                maxTickMs = juce::jmax(maxTickMs, engine.takeMaxControlTickMs());
                for (int ch = 0; ch < 2; ++ch)
                {
                    const float* d = buffer.getReadPointer(ch);
                    for (int i = 0; i < blockSize; ++i)
                    {
                        if (!std::isfinite(d[i])) stats.finite = false;
                        stats.peak = juce::jmax(stats.peak, std::abs(d[i]));
                    }
                }
                // Real devices deliver a block every 5.33 ms; the engine's
                // pending note logic is wall-clock based, so keep real time.
                const double deadline = juce::Time::getMillisecondCounterHiRes() + 1000.0 * blockSize / 48000.0 - ms;
                while (juce::Time::getMillisecondCounterHiRes() < deadline) {}
            }
        };
        auto describe = [&](const juce::String& name, const BlockStats& s)
        {
            const double budgetMs = 1000.0 * blockSize / 48000.0;
            report << name << ": blocks " << s.blocks << "  avg " << juce::String(s.totalMs / juce::jmax(1, s.blocks), 3)
                   << " ms  max " << juce::String(s.maxMs, 3) << " ms  (budget " << juce::String(budgetMs, 2)
                   << " ms)  peak polyphony " << s.maxPolyphony << "  peak level " << juce::String(s.peak, 3)
                   << "  max control tick " << juce::String(maxTickMs, 3) << " ms"
                   << (s.finite ? "" : "  *** NON-FINITE OUTPUT ***") << "\n";
            maxTickMs = 0.0f;
        };

        { BlockStats s; renderBlocks(40, s); describe("Silence", s); }
        // ORGAN_SELFTEST_RENDER="<stop name part>|<key>|<shift>|<out.wav>" (dev, 0.8.162): a
        // transmission of that stop (division 0) with the shift into the last division,
        // the key played there alone for 2 s and released (1 s of tail), the output
        // written as a mono 48 kHz WAV; the copy of the filled pipe itself beside it
        // (<out>.copy.wav) with its source pipe noted. The test ends there.
        if (const char* env = std::getenv("ORGAN_SELFTEST_RENDER"))
        {
            const auto parts = juce::StringArray::fromTokens(juce::String(env), "|", "");
            if (parts.size() >= 4)
            {
                Stop* target = nullptr;
                for (auto* stop : engine.getStops()) if (stop->primaryManual == 0 && stop->name.containsIgnoreCase(parts[0])) { target = stop; break; }
                if (target == nullptr) { report << "RENDER: no stop named like \"" << parts[0] << "\"\n"; return report; }
                const int key = parts[1].getIntValue(), shift = parts[2].getIntValue();
                const juce::File out(parts[3]);
                // Optional: |<stop transposition (sampleOffset)>|<velocity> - the user's case of a
                // stop transposed +24 with a transmission on top, played softly (a velocity layer).
                const int sampleOffset = parts.size() > 4 ? parts[4].getIntValue() : 0;
                const int velocity = parts.size() > 5 ? juce::jlimit(1, 127, parts[5].getIntValue()) : 127;
                const int division = divisions > 1 ? divisions - 1 : 0;
                // |late: the transposition applied AFTER the transmission exists (the GUI's
                // order when the box is changed on a stop that has extensions).
                const bool late = parts.size() > 6 && parts[6] == "late";
                // |pitch=<cents>: the global pitch (a retune bus beyond 25 cents, 0.8.163).
                for (int i = 6; i < parts.size(); ++i)
                {
                    if (parts[i].startsWith("pitch="))
                    {
                        engine.setGlobalPitchCents(parts[i].fromFirstOccurrenceOf("=", false, false).getFloatValue());
                        report << "RENDER: global pitch " << parts[i].fromFirstOccurrenceOf("=", false, false) << " cents\n";
                    }
                    // |sinc=<Hz>: the sinc playback kernel from this sounding fundamental up (0 = all, 20000 = none; 0.8.164).
                    if (parts[i].startsWith("sinc="))
                    {
                        engine.setSincKernelFromHz(parts[i].fromFirstOccurrenceOf("=", false, false).getFloatValue());
                        report << "RENDER: sinc kernel from " << parts[i].fromFirstOccurrenceOf("=", false, false) << " Hz\n";
                    }
                    // |pure: no random detune, no tremulant, no pipe coupling - the kernel alone.
                    if (parts[i] == "pure")
                    {
                        engine.setStopRandomPitchError(target, 0.0f, false);
                        for (int m = 0; m < divisions; ++m) engine.setManualTremulant(m, false, 4.4f, 8.0f, 260.0f);
                        report << "RENDER: pure (no random detune, no tremulant, no coupling)\n";
                    }
                    // |temper=<cents>: every pitch class but A retuned by this (a temperament's worst case, stays in the voice).
                    if (parts[i].startsWith("temper="))
                    {
                        const float c = parts[i].fromFirstOccurrenceOf("=", false, false).getFloatValue();
                        std::array<float, 12> cents{}; for (int pc = 0; pc < 12; ++pc) cents[(size_t)pc] = pc == 9 ? 0.0f : c;
                        engine.setTemperamentOffsets(cents);
                        report << "RENDER: temperament offsets " << juce::String(c, 1) << " cents on every class but A\n";
                    }
                }
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == target);
                auto applyOffset = [&]
                {
                    if (sampleOffset == 0) return;
                    engine.setStopSampleOffset(target, sampleOffset);
                    for (int w = 0; w < 300000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20);
                    report << "RENDER: stop transposition " << juce::String(sampleOffset) << " applied (files " << juce::String(sampleOffset) << " above their keys)"
                           << (late ? " after the transmission" : "") << "\n";
                };
                if (!late) applyOffset();
                const int transmission = engine.addTransmission(target, division, shift);
                engine.setTransmissionEnabled(target, transmission, true);
                for (int w = 0; w < 300000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20);
                if (late) applyOffset();
                const int pipe = key + shift;
                report << "RENDER: " << target->name << " [" << target->perspective << "] transmission " << juce::String(shift) << " into division " << juce::String(division)
                       << ", key " << juce::String(key) << " -> pipe " << juce::String(pipe);
                if (pipe >= 0 && pipe < 128 && target->isFilled(pipe))
                    report << " filled from pipe " << juce::String(pipe + (int)target->fillOffset[(size_t)pipe]) << " (fill offset " << juce::String((int)target->fillOffset[(size_t)pipe]) << ")";
                else report << " (a real pipe)";
                report << "\n";
                auto writeWav = [](const juce::File& file, const std::vector<float>& audio, double rate)
                {
                    file.deleteFile();
                    juce::WavAudioFormat format;
                    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(new juce::FileOutputStream(file), rate, 1, 24, {}, 0));
                    if (writer == nullptr) return;
                    const float* channels[1]{ audio.data() };
                    writer->writeFromFloatArrays(channels, 1, (int)audio.size());
                };
                std::vector<float> audio;
                capture = &audio;
                engine.noteOn(key, velocity, division);
                BlockStats held; renderBlocks(375, held);   // 2 s
                engine.noteOff(key, division);
                BlockStats tail; renderBlocks(190, tail);   // 1 s
                capture = nullptr;
                writeWav(out, audio, 48000.0);
                report << "RENDER: " << juce::String((int)audio.size()) << " frames written to " << out.getFullPathName() << ", peak " << juce::String(held.peak, 4)
                       << ", voices " << juce::String(held.maxPolyphony) << "\n";
                if (pipe >= 0 && pipe < 128)
                    if (auto it = target->aliasFree.find(pipe); it != target->aliasFree.end() && !it->second.empty() && it->second[0] != nullptr)
                    {
                        const Sample& s = *it->second[0];
                        std::vector<float> v((size_t)s.numFrames);
                        std::vector<int16_t> scratch;
                        std::vector<float> scratch32;
                        if (s.storesFloat) { const float* d = s.read32(0, scratch32); for (int i = 0; i < (int)s.numFrames; ++i) v[(size_t)i] = d[i]; }
                        else { const int16_t* d = s.read16(0, scratch); for (int i = 0; i < (int)s.numFrames; ++i) v[(size_t)i] = (float)d[i] / 32768.0f; }
                        const auto copyFile = out.getSiblingFile(out.getFileNameWithoutExtension() + ".copy.wav");
                        writeWav(copyFile, v, s.sampleRate);
                        report << "RENDER: copy of pipe " << juce::String(pipe) << ": " << juce::String((int)s.numFrames) << " frames at " << juce::String(s.sampleRate, 0) << " Hz, "
                               << juce::String((int)it->second.size()) << " copies (attack + releases), loops " << juce::String((int)s.loops.size()) << " -> " << copyFile.getFullPathName() << "\n";
                    }
                    else report << "RENDER: no copy for pipe " << juce::String(pipe) << "\n";
                return report;
            }
        }
        {
            // Release overlap: one key of division 0 held for 320 ms and released;
            // the voices block by block afterwards. At the release the sustaining
            // voice fades out while the release voice fades in - two voices per
            // pipe for the crossfade - so the peak lies right after the release.
            // (Blocks paced in real time by renderBlocks: the notes start on the wall clock.)
            engine.noteOn(60, 127, 0);
            int held = 0;
            for (int b = 0; b < 60; ++b) { BlockStats t; renderBlocks(1, t); held = juce::jmax(held, engine.getCurrentPolyphony()); }
            engine.noteOff(60, 0);
            int peakAfter = 0, peakAtMs = -1, overlapMs = 0, zeroAtMs = -1;
            constexpr double stepMs = 1000.0 * 256 / 48000.0;   // renderBlocks(1) = 256 frames whatever the block size
            for (int b = 0; b < 200; ++b)
            {
                BlockStats t; renderBlocks(1, t);
                const int now = engine.getCurrentPolyphony();
                if (now > peakAfter) { peakAfter = now; peakAtMs = (int)std::lround(b * stepMs); }
                if (now > held) overlapMs += (int)std::lround(stepMs);
                if (now == 0 && zeroAtMs < 0) zeroAtMs = (int)std::lround(b * stepMs);
            }
            report << "Release overlap (key 60 of division 0, every stop on): " << held << " voices while held; after the release the peak was "
                   << peakAfter << " voices " << peakAtMs << " ms after it, more than while held for " << overlapMs
                   << " ms, silence " << (zeroAtMs >= 0 ? juce::String(zeroAtMs) + " ms" : juce::String("not within 1 s")) << " after it\n";
            { BlockStats t; for (int drain = 0; drain < 200 && engine.getCurrentPolyphony() > 0; ++drain) renderBlocks(1, t); }
        }
        if (selfTestIr.existsAsFile() && divisions >= 2)
        {
            // Shared IR: every route has the same output channels, IR and settings
            // here, so once they are told they render on one device (as this
            // test's renderBlockForManual does, one after another) they must become
            // one convolution - and a chord must come out at the same level as
            // with a convolution per route (convolution is linear: the reverb of
            // the summed dry signal is the sum of the reverbs).
            const int chord[]{ 48, 55, 60, 64, 67 };
            auto chordLevelDb = [&](const juce::String& label)
            {
                for (int note : chord) noteOnAll(note);
                BlockStats warm; renderBlocks(60, warm);
                std::vector<float> captured;
                capture = &captured;
                BlockStats held; renderBlocks(200, held);
                capture = nullptr;
                double sum = 0.0;
                for (const float v : captured) sum += (double)v * v;
                for (int note : chord) noteOffAll(note);
                BlockStats rel; renderBlocks(600, rel);   // releases and most of the reverb tail die
                describe(label, held);
                return 10.0 * std::log10(sum / (captured.empty() ? 1.0 : (double)captured.size()) + 1.0e-20);   // (no jmax<size_t>: GCC picks juce_dsp's SIMD overload)
            };
            const double ownDb = chordLevelDb("Shared IR check, a convolution per route");
            for (int manual = 0; manual < divisions; ++manual)
                for (const auto& perspective : engine.getPerspectivesForManual(manual))
                    engine.setPerspectiveDeviceKey(manual, perspective, "selftest|one device");
            const auto shared = engine.getReverbSummary();
            const double sharedDb = chordLevelDb("Shared IR check, every route on one device");
            for (int manual = 0; manual < divisions; ++manual)
                for (const auto& perspective : engine.getPerspectivesForManual(manual))
                    engine.setPerspectiveDeviceKey(manual, perspective, "selftest|" + perspective + "|" + juce::String(manual));   // a device of its own again (not none: that would silence the route)
            const auto own = engine.getReverbSummary();
            const double diff = sharedDb - ownDb;
            report << "Shared IR: " << (shared.routes + shared.sharedRoutes) << " route(s) on one device in " << shared.routes
                   << " convolution(s) (" << shared.sharedRoutes << " shared; apart again: " << own.routes << " convolution(s), "
                   << own.sharedRoutes << " shared); chord level " << juce::String(ownDb, 2) << " dB apart, " << juce::String(sharedDb, 2)
                   << " dB shared, difference " << juce::String(diff, 2) << " dB"
                   << ((shared.routes == 1 && shared.sharedRoutes == shared.routes + shared.sharedRoutes - 1 && own.sharedRoutes == 0
                        && std::abs(diff) < 1.5) ? "  -> OK" : "  *** WRONG ***") << "\n";
        }
        if (std::getenv("ORGAN_SELFTEST_BENCH") != nullptr)
        {
            // Polyphony benchmark only (ORGAN_SELFTEST_BENCH=1): every stop on,
            // every key 36..96 held on every division (ORGAN_SELFTEST_DIVISIONS=N
            // multiplies the voices), the tremulant on and the swell at 0.7 (set
            // above) so that the harmonic filter acts; then the same with the
            // harmonic filter off. The figure that matters: ms per 1000 voices.
            auto bench = [&](const juce::String& label)
            {
                for (int note = 36; note <= 96; ++note) noteOnAll(note);
                BlockStats warm; renderBlocks(60, warm);   // every voice started
                BlockStats s; renderBlocks(200, s);
                for (int note = 36; note <= 96; ++note) noteOffAll(note);
                BlockStats rel; renderBlocks(400, rel);    // the releases die
                const double avg = s.totalMs / juce::jmax(1, s.blocks);
                const double budgetMs = 1000.0 * blockSize / 48000.0;
                report << "BENCH " << label << ": voices " << s.maxPolyphony << "  avg " << juce::String(avg, 3)
                       << " ms/block  max " << juce::String(s.maxMs, 3) << " ms  (budget " << juce::String(budgetMs, 2) << " ms)  = "
                       << juce::String(avg * 1000.0 / juce::jmax(1, s.maxPolyphony), 4) << " ms per 1000 voices  -> about "
                       << (int)(budgetMs / juce::jmax(1.0e-6, avg) * s.maxPolyphony) << " voices would fill the budget\n";
            };
            bench("tutti, harmonic filter on, tremulant on, swell 0.7");
            engine.setHarmonicFilterDb(0.0f, 0.0f, 0.0f);
            bench("tutti, harmonic filter OFF");
            for (int manual = 0; manual < divisions; ++manual) engine.setManualTremulant(manual, false, 4.4f, 8.0f, 260.0f);
            bench("tutti, harmonic filter OFF, tremulant off");
            return report + "BENCH done.\n";
        }
        {
            // Transmission check: one stop played from its own division and from a
            // division it is transmitted to must sound once and stop on release.
            Stop* borrowed = nullptr;
            for (auto* stop : engine.getStops())
                if (stop->primaryManual == 0 && stop->hasNote(60)) { borrowed = stop; break; }
            if (borrowed != nullptr)
            {
                const int transmission = engine.addTransmission(borrowed, 1, 0);
                engine.setTransmissionEnabled(borrowed, transmission, true);
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == borrowed);
                auto check = [&](const juce::String& label, std::function<void()> press, std::function<void()> release)
                {
                    BlockStats s;
                    press();
                    renderBlocks(60, s);
                    const int sounding = engine.getCurrentPolyphony();
                    release();
                    renderBlocks(30, s);
                    const int justReleased = engine.getCurrentPolyphony();
                    renderBlocks(1400, s);   // ~7.5 s: every release tail should have died ...
                    int after = engine.getCurrentPolyphony();
                    double waitedSeconds = 7.5;
                    // ... unless the release samples are long (a 5 s release played an
                    // octave lower lasts 10 s): wait up to 25 s before calling it stuck.
                    while (after > 0 && waitedSeconds < 25.0)
                    {
                        renderBlocks(200, s);
                        waitedSeconds += 1.07;
                        after = engine.getCurrentPolyphony();
                    }
                    report << label << ": voices while held " << sounding << ", after release " << justReleased
                           << ", " << juce::String(waitedSeconds, 1) << " s later " << after
                           << (after == 0 ? "  -> OK" : "  *** STILL SOUNDING ***") << "\n";
                };
                check("Transmission (played from division 1 only)",
                    [&] { engine.noteOn(60, 127, 1); }, [&] { engine.noteOff(60, 1); });
                check("Transmission (played from division 0 and 1)",
                    [&] { engine.noteOn(60, 127, 0); engine.noteOn(60, 127, 1); },
                    [&] { engine.noteOff(60, 1); engine.noteOff(60, 0); });
                check("Transmission (division 1 released first, 0 held on)",
                    [&] { engine.noteOn(60, 127, 0); engine.noteOn(60, 127, 1); },
                    [&] { engine.noteOff(60, 1); BlockStats d; renderBlocks(20, d); engine.noteOff(60, 0); });
                check("Transmission (started from 1, then 0; 1 released first)",
                    [&] { engine.noteOn(60, 127, 1); engine.noteOn(60, 127, 0); },
                    [&] { engine.noteOff(60, 1); BlockStats d; renderBlocks(20, d); engine.noteOff(60, 0); });
                check("Transmission (started from 1, then 0; 0 released first)",
                    [&] { engine.noteOn(60, 127, 1); engine.noteOn(60, 127, 0); },
                    [&] { engine.noteOff(60, 0); BlockStats d; renderBlocks(20, d); engine.noteOff(60, 1); });
                // Transposing transmission (-12 = 16' extension): key 72 of division 1
                // sounds pipe 60; the same pipe held from the home division must
                // keep sounding until both keys are up.
                engine.setTransmissionShift(borrowed, transmission, -12);
                report << "Transposition: \"" << borrowed->name << "\" shifted -12 is labelled \""
                       << SamplerEngine::transposedStopName(borrowed, -12) << "\" (footage "
                       << SamplerEngine::footageLabel(SamplerEngine::stopFootageSemitones(borrowed)) << " -> "
                       << SamplerEngine::footageLabel(SamplerEngine::stopFootageSemitones(borrowed) - 12) << ")\n";
                check("Transposed transmission (-12, played from division 1 only)",
                    [&] { engine.noteOn(72, 127, 1); }, [&] { engine.noteOff(72, 1); });
                check("Transposed transmission (same pipe from division 0 key 60 and division 1 key 72; 1 released first)",
                    [&] { engine.noteOn(60, 127, 0); engine.noteOn(72, 127, 1); },
                    [&] { engine.noteOff(72, 1); BlockStats d; renderBlocks(20, d); engine.noteOff(60, 0); });
                check("Transposed transmission (same pipe; 0 released first)",
                    [&] { engine.noteOn(72, 127, 1); engine.noteOn(60, 127, 0); },
                    [&] { engine.noteOff(60, 0); BlockStats d; renderBlocks(20, d); engine.noteOff(72, 1); });
                check("Transposed transmission (shift changed to +7 while held)",
                    [&] { engine.noteOn(60, 127, 1); engine.setTransmissionShift(borrowed, transmission, 7); },
                    [&] { engine.noteOff(60, 1); });
                check("Transposed transmission (switched off while held, key released later)",
                    [&] { engine.noteOn(60, 127, 1); },
                    [&] { engine.setTransmissionEnabled(borrowed, transmission, false); BlockStats d; renderBlocks(20, d);
                          engine.noteOff(60, 1); engine.setTransmissionEnabled(borrowed, transmission, true); });
                // Filled pipes: the lowest key of the 16' extension needs a pipe an
                // octave below the rank's compass; it must sound (from a neighbour) and stop.
                {
                    int lowest = -1, highest = -1;
                    for (int note = 0; note < 128; ++note)
                        if (borrowed->hasNote(note)) { if (lowest < 0) lowest = note; highest = note; }
                    int filledBelow = 0, filledAbove = 0, filledInside = 0;
                    for (int note = 0; note < 128; ++note)
                        if (borrowed->isFilled(note))
                        {
                            if (note < lowest) ++filledBelow; else if (note > highest) ++filledAbove; else ++filledInside;
                        }
                    report << "Filled pipes of \"" << borrowed->name << "\" (compass " << lowest << "-" << highest
                           << ", group " << engine.getFillGroupSize() << "): " << filledBelow << " below, " << filledInside
                           << " inside, " << filledAbove << " above; pipe " << (lowest - 1) << " <- " << borrowed->sourceNoteFor(lowest - 1)
                           << ", pipe " << (lowest - 12) << " <- " << borrowed->sourceNoteFor(lowest - 12)
                           << ", pipe " << (highest + 1) << " <- " << borrowed->sourceNoteFor(highest + 1) << "\n";
                    // Anti-alias copies (quality 2): only for filled pipes transposed upwards.
                    {
                        int copies = 0; double megabytes = 0.0;
                        for (auto* stop : engine.getStops())
                        {
                            std::vector<Stop*> ranks{ stop };
                            for (auto& rank : stop->ranks) ranks.push_back(rank.get());
                            for (auto* rank : ranks)
                                for (const auto& entry : rank->aliasFree)
                                    for (const auto& sample : entry.second)
                                    {
                                        ++copies;
                                        megabytes += (double)sample->numFrames * sample->numChannels * (sample->storesFloat ? 4 : 2) / 1048576.0;
                                    }
                        }
                        report << "Anti-alias copies (resampling quality " << engine.getTransposeQuality() << "): " << copies
                               << " samples, " << juce::String(megabytes, 1) << " MB for " << engine.getStops().size() << " stops\n";
                    }
                    // Remainder rule: 17 pipes needed above the rank with a group of 12 ->
                    // 12 octave transpositions, then the 5 highest pipes shifted by 17.
                    {
                        std::unique_ptr<Stop> probe = std::make_unique<Stop>("probe", juce::File());
                        for (int note = 36; note <= 60; ++note)
                            probe->samplesByNote[(size_t)note].push_back(std::make_unique<Sample>(1, 8, 48000.0, true));
                        SamplerEngine::computeFilledPipes(probe.get(), 12, 36, 77);
                        report << "Remainder rule (rank 36-60, need up to 77, group 12): pipe 61 <- " << probe->sourceNoteFor(61)
                               << " (expect 49), pipe 72 <- " << probe->sourceNoteFor(72) << " (expect 60), pipe 73 <- "
                               << probe->sourceNoteFor(73) << " (expect 56), pipe 77 <- " << probe->sourceNoteFor(77)
                               << " (expect 60), pipe 78 filled: " << (probe->isFilled(78) ? "yes" : "no (expect no)")
                               << ((probe->sourceNoteFor(61) == 49 && probe->sourceNoteFor(72) == 60 && probe->sourceNoteFor(73) == 56
                                   && probe->sourceNoteFor(77) == 60 && !probe->isFilled(78)) ? "  -> OK" : "  *** WRONG ***") << "\n";
                        // The first group may already be the small one: a rank ending at 89 and a
                        // compass to 96 take only 7 pipes (83-89) shifted by 7, not 12 by an octave.
                        std::unique_ptr<Stop> probe2 = std::make_unique<Stop>("probe2", juce::File());
                        for (int note = 36; note <= 89; ++note)
                            probe2->samplesByNote[(size_t)note].push_back(std::make_unique<Sample>(1, 8, 48000.0, true));
                        SamplerEngine::computeFilledPipes(probe2.get(), 12, 36, 96);
                        report << "First group smaller than the setting (rank 36-89, need up to 96, group 12): pipe 90 <- "
                               << probe2->sourceNoteFor(90) << " (expect 83), pipe 96 <- " << probe2->sourceNoteFor(96)
                               << " (expect 89), pipe 97 filled: " << (probe2->isFilled(97) ? "yes" : "no (expect no)")
                               << ((probe2->sourceNoteFor(90) == 83 && probe2->sourceNoteFor(96) == 89 && !probe2->isFilled(97))
                                   ? "  -> OK" : "  *** WRONG ***") << "\n";
                    }
                    engine.setTransmissionShift(borrowed, transmission, -12);
                    check("Filled pipe (16' extension, lowest key -> pipe an octave below the compass)",
                        [&] { engine.noteOn(lowest, 127, 1); }, [&] { engine.noteOff(lowest, 1); });
                    engine.setTransmissionShift(borrowed, transmission, 12);
                    check("Filled pipe (4' extension, highest key -> pipe an octave above the compass)",
                        [&] { engine.noteOn(highest, 127, 1); }, [&] { engine.noteOff(highest, 1); });
                }
                // Route volume: the pipe held by the stop (0 dB) and a +6 dB transmission
                // sounds once; when the louder route lets go it keeps sounding.
                engine.setTransmissionShift(borrowed, transmission, 0);
                engine.setTransmissionDb(borrowed, transmission, 6.0f);
                check("Route volume (stop 0 dB + transmission +6 dB hold one pipe; transmission released first)",
                    [&] { engine.noteOn(60, 127, 0); engine.noteOn(60, 127, 1); },
                    [&] { engine.noteOff(60, 1); BlockStats d; renderBlocks(40, d); engine.noteOff(60, 0); });
                engine.setTransmissionDb(borrowed, transmission, 0.0f);
                // Sustained samples without loop points play as one big loop: the
                // pipe must still sound after the file's length while the key is held.
                {
                    Stop* loopless = nullptr; int looplessNote = -1; double seconds = 0.0;
                    for (auto* stop : engine.getStops())
                    {
                        if (stop->isOneShot) continue;
                        for (int note = 36; note <= 96 && loopless == nullptr; ++note)
                            if (stop->hasNote(note) && !stop->noteHasLoop(note))
                            {
                                const auto& sample = stop->samplesByNote[(size_t)note][0];
                                seconds = (double)sample->numFrames / sample->sampleRate;
                                if (seconds < 12.0) { loopless = stop; looplessNote = note; }
                            }
                        if (loopless != nullptr) break;
                    }
                    if (loopless == nullptr) report << "Whole-file loop: no sustained sample without loop points shorter than 12 s in this set\n";
                    else
                    {
                        for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == loopless);
                        BlockStats s;
                        engine.noteOn(looplessNote, 127, 0);
                        renderBlocks((int)((seconds + 2.0) * 48000.0 / 256.0), s);   // well past the file's end
                        const int sounding = engine.getCurrentPolyphony();
                        engine.noteOff(looplessNote, 0);
                        renderBlocks(1400, s);
                        report << "Whole-file loop (\"" << loopless->name << "\" note " << looplessNote << ", " << juce::String(seconds, 1)
                               << " s, no loop points): voices " << juce::String(seconds + 2.0, 0) << " s into the held key " << sounding
                               << (sounding >= 1 ? "  -> OK (still sounding)" : "  *** STOPPED ***") << ", after release "
                               << engine.getCurrentPolyphony() << (s.finite ? "" : "  *** NON-FINITE ***") << "\n";
                        for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == borrowed);
                    }
                }
                // Division compass: keys outside it are silent on every route.
                engine.setManualCompass(1, 36, 67);
                {
                    BlockStats s;
                    engine.noteOn(70, 127, 1);
                    renderBlocks(60, s);
                    const int sounding = engine.getCurrentPolyphony();
                    engine.noteOff(70, 1);
                    renderBlocks(30, s);
                    report << "Compass 36-67 on division 1: key 70 through the transmission -> voices " << sounding
                           << (sounding == 0 ? "  -> OK (silent)" : "  *** SOUNDS OUTSIDE THE COMPASS ***") << "\n";
                }
                engine.setManualCompass(1, 36, 96);
                engine.setTransmissionEnabled(borrowed, transmission, false);
                engine.removeTransmission(borrowed, transmission);
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            }
        }
        {
            // Celeste tuning: with the 440 Hz normalisation on, a celeste at undulation
            // 0 must sound exactly at equal-tempered pitch and with its detected
            // undulation as recorded (the pitch of the rendered key 60 is measured).
            Stop* celeste = nullptr;
            for (auto* stop : engine.getStops())
                if (std::abs(stop->detectedCelesteBeats) > 1.0e-3f && stop->hasNote(60) && stop->tuningInfoAvailable) { celeste = stop; break; }
            if (celeste != nullptr)
            {
                const int manual = celeste->primaryManual;
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == celeste);
                engine.setAllStopsTuneTo440(true);
                engine.setStopRandomPitchError(celeste, 0.0f, true);
                engine.setManualTremulant(manual, false, 4.4f, 8.0f, 260.0f);   // a steady pitch to measure
                const double recordedCents = (celeste->samplesByNote[60][0]->smplPitch - 60.0) * 100.0;
                auto measureCents = [&]
                {
                    std::vector<float> mono;
                    engine.noteOn(60, 127, manual);
                    const int blocks = 300 * 256 / blockSize;   // 1.6 s; the first 0.6 s (attack) are skipped
                    for (int b = 0; b < blocks; ++b)
                    {
                        buffer.clear();
                        engine.renderBlockForManual(buffer, 0.1f, -1);
                        if (b >= blocks * 3 / 8)
                            for (int i = 0; i < blockSize; ++i) mono.push_back(buffer.getSample(0, i) + buffer.getSample(1, i));
                        const double deadline = juce::Time::getMillisecondCounterHiRes() + 1000.0 * blockSize / 48000.0;
                        while (juce::Time::getMillisecondCounterHiRes() < deadline) {}
                    }
                    engine.noteOff(60, manual);
                    BlockStats s; renderBlocks(150, s);
                    // Autocorrelation: the lag with the highest normalised correlation
                    // between 80 and 600 samples (80..600 Hz at 48 kHz), refined parabolically.
                    const int n = (int)mono.size();
                    int bestLag = 0; double best = -1.0;
                    std::vector<double> corr(601, 0.0);
                    for (int lag = 80; lag <= 600; ++lag)
                    {
                        double sum = 0.0, e0 = 0.0, e1 = 0.0;
                        for (int i = 0; i + lag < n; ++i) { sum += mono[(size_t)i] * mono[(size_t)(i + lag)]; e0 += mono[(size_t)i] * mono[(size_t)i]; e1 += mono[(size_t)(i + lag)] * mono[(size_t)(i + lag)]; }
                        corr[(size_t)lag] = e0 > 0.0 && e1 > 0.0 ? sum / std::sqrt(e0 * e1) : 0.0;
                        if (corr[(size_t)lag] > best) { best = corr[(size_t)lag]; bestLag = lag; }
                    }
                    // The true period is the shortest lag whose peak comes close to the
                    // best one (a doubled lag correlates just as well: an octave error).
                    for (int lag = 81; lag < bestLag; ++lag)
                        if (corr[(size_t)lag] >= 0.9 * best && corr[(size_t)lag] >= corr[(size_t)lag - 1]
                            && corr[(size_t)lag] >= corr[(size_t)lag + 1]) { bestLag = lag; break; }
                    double lag = bestLag;
                    if (bestLag > 80 && bestLag < 600)
                    {
                        const double a = corr[(size_t)bestLag - 1], b = corr[(size_t)bestLag], c = corr[(size_t)bestLag + 1];
                        const double denominator = a - 2.0 * b + c;
                        if (std::abs(denominator) > 1.0e-12) lag += 0.5 * (a - c) / denominator;
                    }
                    const double hz = 48000.0 / lag;
                    return 1200.0 * std::log2(hz / 261.6256);
                };
                const double withUndulation = measureCents();
                const float detected = celeste->undulationBeats;
                // The undulation follows the fitted trend of the whole rank, not the
                // one pipe at key 60 (which may be an outlier): the table says what to expect.
                const double trendCents = celeste->undulationCents[60];
                engine.setStopUndulation(celeste, 0.0f);
                const double withZero = measureCents();
                engine.setStopUndulation(celeste, detected);
                const bool ok = std::abs(withZero) < 6.0 && std::abs((withUndulation - withZero) - trendCents) < 5.0;
                report << "Celeste tuning (" << celeste->name << ", key 60, recorded " << juce::String(recordedCents, 1)
                       << " c off ET, rank trend " << juce::String(trendCents, 1) << " c): with detected undulation "
                       << juce::String(detected, 2) << " beats -> " << juce::String(withUndulation, 1)
                       << " c; with undulation 0 -> " << juce::String(withZero, 1)
                       << " c  -> " << (ok ? "OK" : "*** WRONG ***") << "\n";
                engine.setManualTremulant(manual, true, 4.4f, 8.0f, 260.0f);
                engine.setStopRandomPitchError(celeste, 4.0f, true);
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            }
        }
        {
            // Swapping divisions: the stops of division 0 move to division 1 (and
            // back); they must sound from their new keyboard and stay silent on the old.
            // (With two or more real divisions the old keyboard is not silent: it
            // now carries the stops of division 1 - as many as division 1 had.)
            const int before = engine.getStops().size();
            int inZeroBefore = 0, inOneBefore = 0;
            for (auto* stop : engine.getStops()) { if (stop->primaryManual == 0) ++inZeroBefore; if (stop->primaryManual == 1) ++inOneBefore; }
            engine.swapDivisions(0, 1);
            int inOne = 0, inZeroSwapped = 0;
            for (auto* stop : engine.getStops()) { if (stop->primaryManual == 1) ++inOne; if (stop->primaryManual == 0) ++inZeroSwapped; }
            for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            // The routes of the division the stops moved to were never set up when
            // the test loaded one division only: a route without a device gets no
            // voices (0.8.126), so key 60 there counted 0 and the test said WRONG
            // (0.8.163, twice taken for an engine fault). Both divisions get the
            // test's device key, channels, rate and block size here, like the setup.
            for (int m = 0; m < 2; ++m)
                for (const auto& perspective : engine.getPerspectivesForManual(m))
                {
                    engine.setPerspectiveOutputChannels(m, perspective, { 0, 1 });
                    engine.setPerspectiveSampleRate(m, perspective, 48000.0);
                    engine.setPerspectiveBlockSize(m, perspective, blockSize);
                    engine.setPerspectiveDeviceKey(m, perspective, "selftest|" + perspective + "|" + juce::String(m));
                }
            engine.noteOn(60, 127, 1);
            BlockStats s; renderBlocks(40, s);
            const int voicesNew = engine.getCurrentPolyphony();
            engine.noteOff(60, 1);
            // Let the release samples die before the old keyboard is tried (long
            // releases would otherwise be counted as its voices).
            for (int waited = 0; waited < 40 && engine.getCurrentPolyphony() > 0; ++waited) renderBlocks(100, s);
            engine.noteOn(60, 127, 0);
            renderBlocks(40, s);
            const int voicesOld = engine.getCurrentPolyphony();
            engine.noteOff(60, 0);
            for (int waited = 0; waited < 40 && engine.getCurrentPolyphony() > 0; ++waited) renderBlocks(100, s);
            engine.swapDivisions(0, 1);
            int inZeroAfter = 0;
            for (auto* stop : engine.getStops()) if (stop->primaryManual == 0) ++inZeroAfter;
            report << "Swap divisions 0 <-> 1: " << inZeroBefore << " stops moved to division 1 (" << inOne << "), " << inOneBefore
                   << " of division 1 moved to division 0 (" << inZeroSwapped << "), key 60 on division 1 -> "
                   << voicesNew << " voices, on division 0 -> " << voicesOld << " (expected " << (inOneBefore > 0 ? "some" : "none")
                   << "); swapped back: " << inZeroAfter << " in division 0 of " << engine.getStops().size() << " (" << before << ")  -> "
                   << (inOne == inZeroBefore && inZeroSwapped == inOneBefore && voicesNew > 0 && (inOneBefore > 0 ? voicesOld > 0 : voicesOld == 0)
                       && inZeroAfter == inZeroBefore && engine.getStops().size() == before ? "OK" : "*** WRONG ***") << "\n";
        }
        {
            // Pipe on/off (voicing checkbox): a real pipe switched off stays silent
            // and keeps its samples; switched on again it plays its own sample, not
            // an extension.
            Stop* probe = engine.getStops()[0];
            int lowest = -1;
            for (int note = 0; note < 128 && lowest < 0; ++note) if (probe->hasNote(note)) lowest = note;
            if (lowest >= 0)
            {
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == probe);
                engine.setPipeEnabled(probe, lowest, false);
                const bool keptSamples = probe->hasNote(lowest);
                engine.noteOn(lowest, 127, 0);
                BlockStats s; renderBlocks(40, s);
                const int voicesOff = engine.getCurrentPolyphony();
                engine.noteOff(lowest, 0);
                renderBlocks(20, s);
                engine.setPipeEnabled(probe, lowest, true);
                const bool ownSample = probe->hasNote(lowest) && !probe->isFilled(lowest);
                engine.noteOn(lowest, 127, 0);
                renderBlocks(40, s);
                const int voicesOn = engine.getCurrentPolyphony();
                engine.noteOff(lowest, 0);
                renderBlocks(200, s);
                report << "Pipe off/on (" << probe->name << " key " << lowest << "): off -> voices " << voicesOff
                       << ", samples kept " << (keptSamples ? "yes" : "NO") << "; on -> own sample "
                       << (ownSample ? "yes" : "NO") << ", voices " << voicesOn << "  -> "
                       << (voicesOff == 0 && keptSamples && ownSample && voicesOn > 0 ? "OK" : "*** WRONG ***") << "\n";
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            }
        }
        {
            // Transposition method "pitch shift, length kept": the copies of a filled
            // pipe have the source's length (Signalsmith Stretch), and the pipe sounds.
            Stop* filledStop = nullptr;
            int filledPipe = -1;
            for (auto* stop : engine.getStops())
            {
                for (int pipe = 0; pipe < 128 && filledPipe < 0; ++pipe)
                    if (stop->isFilled(pipe) && stop->fillOffset[(size_t)pipe] != 0) { filledStop = stop; filledPipe = pipe; }
                if (filledPipe >= 0) break;
            }
            if (filledStop != nullptr)
            {
                const int source = filledStop->sourceNoteFor(filledPipe);
                engine.setTransposeMethod(1);
                const double t0 = juce::Time::getMillisecondCounterHiRes();
                engine.buildAntiAliasCopies(filledStop, false);   // synchronous: the copies of this stop (attacks and releases)
                const double buildMs = juce::Time::getMillisecondCounterHiRes() - t0;
                int copies = 0, releaseCopies = 0;
                bool sameLength = true;
                const Sample* sourceSample = source >= 0 && !filledStop->samplesByNote[(size_t)source].empty() ? filledStop->samplesByNote[(size_t)source][0].get() : nullptr;
                // Resampled: the attack copy is the source's length over the pitch ratio.
                const double pitchRatio = std::pow(2.0, (double)(filledPipe - source) / 12.0);
                juce::String lengthNote;
                if (auto it = filledStop->aliasFree.find(filledPipe); it != filledStop->aliasFree.end())
                {
                    copies = (int)it->second.size();
                    for (const auto& copy : it->second)
                        if (sourceSample != nullptr)
                        {
                            const int64_t expected = (int64_t)std::llround((double)sourceSample->numFrames / pitchRatio);
                            if (std::abs(copy->numFrames - expected) > 2) sameLength = false;
                            if (lengthNote.isEmpty()) lengthNote = juce::String(copy->numFrames) + " frames (expected " + juce::String(expected) + ")";
                        }
                }
                // Releases: the room decay is laid out again to the source's duration
                // (times the release length setting, 100 % by default) in both
                // directions - a pipe's room does not grow or shrink with its pitch.
                juce::String releaseNote;
                for (const auto& layer : filledStop->releaseLayers)
                    if (auto it = layer.aliasFree.find(filledPipe); it != layer.aliasFree.end())
                    {
                        releaseCopies += (int)it->second.size();
                        if (releaseNote.isEmpty() && !it->second.empty() && !layer.samplesByNote[(size_t)source].empty())
                        {
                            const Sample& sourceRelease = *layer.samplesByNote[(size_t)source][0];
                            const double ratio = (double)it->second[0]->numFrames / (double)juce::jmax<int64_t>(1, sourceRelease.numFrames);
                            const double expected = (double)engine.getTransposeReleaseLength();
                            releaseNote = "; release length " + juce::String(ratio * 100.0, 0) + " % of the source (expected " + juce::String(expected * 100.0, 0) + " %)";
                            if (std::abs(ratio - expected) > 0.02) sameLength = false;
                            // The decay profile: the level (dB re the first 100 ms) at 25/50/75/95 % of
                            // the length, source against copy - the copy's decay should follow the source's.
                            auto profile = [](const Sample& s)
                            {
                                auto rmsDb = [&s](double from, double to)
                                {
                                    const int64_t a = (int64_t)(from * (double)s.numFrames), b = juce::jmax(a + 1, (int64_t)(to * (double)s.numFrames));
                                    double power = 0.0;
                                    std::vector<int16_t> scratch;
                                    std::vector<float> scratch32;
                                    for (int ch = 0; ch < s.numChannels; ++ch)
                                    {
                                        const int16_t* x16 = s.storesFloat ? nullptr : s.read16(ch, scratch);
                                        const float* x32 = s.storesFloat ? s.read32(ch, scratch32) : nullptr;
                                        for (int64_t i = a; i < juce::jmin(b, s.numFrames); ++i)
                                        {
                                            const double v = s.storesFloat ? x32[i] : (double)x16[i] / 32768.0;
                                            power += v * v;
                                        }
                                    }
                                    return 10.0 * std::log10(juce::jmax(1.0e-12, power / (double)juce::jmax<int64_t>(1, (b - a) * s.numChannels)));
                                };
                                const double head = rmsDb(0.0, juce::jmin(1.0, 0.1 * s.sampleRate / (double)juce::jmax<int64_t>(1, s.numFrames)));
                                juce::String text;
                                for (const double at : { 0.25, 0.5, 0.75, 0.95 })
                                    text << (text.isEmpty() ? "" : "/") << juce::String(rmsDb(at - 0.02, at + 0.02) - head, 0);
                                return text;
                            };
                            releaseNote << "; decay at 25/50/75/95 % (dB re the first 100 ms): source " << profile(*layer.samplesByNote[(size_t)source][0])
                                        << ", copy " << profile(*it->second[0]);
                        }
                    }
                // Timbre and loudness of the copy against its source: the spectral centroid
                // (Hz) of the sustained part and the RMS. With the timbre kept (the default)
                // the centroid stays close to the source's although the pitch moved; the
                // loudness is brought to the source's.
                auto analyse = [](const Sample& sample, double& rms, double& centroidHz)
                {
                    rms = 0.0; centroidHz = 0.0;
                    constexpr int order = 13, n = 1 << order;
                    const int frames = (int)sample.numFrames;
                    if (frames < n) return;
                    const int start = juce::jlimit(0, frames - n, (int)(frames * 0.3));
                    std::vector<float> data((size_t)n * 2, 0.0f);
                    double power = 0.0;
                    std::vector<std::vector<int16_t>> scratch16((size_t)sample.numChannels);
                    std::vector<std::vector<float>> scratch32((size_t)sample.numChannels);
                    std::vector<const int16_t*> x16((size_t)sample.numChannels, nullptr);
                    std::vector<const float*> x32((size_t)sample.numChannels, nullptr);
                    for (int ch = 0; ch < sample.numChannels; ++ch)
                    {
                        if (sample.storesFloat) x32[(size_t)ch] = sample.read32(ch, scratch32[(size_t)ch]);
                        else x16[(size_t)ch] = sample.read16(ch, scratch16[(size_t)ch]);
                    }
                    for (int i = 0; i < n; ++i)
                    {
                        float v = 0.0f;
                        for (int ch = 0; ch < sample.numChannels; ++ch)
                            v += sample.storesFloat ? x32[(size_t)ch][start + i] : (float)x16[(size_t)ch][start + i] / 32768.0f;
                        v /= (float)juce::jmax(1, sample.numChannels);
                        power += (double)v * v;
                        const float window = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * (float)i / (float)(n - 1));
                        data[(size_t)i] = v * window;
                    }
                    rms = std::sqrt(power / n);
                    juce::dsp::FFT fft(order);
                    fft.performFrequencyOnlyForwardTransform(data.data());
                    double weighted = 0.0, total = 0.0;
                    for (int bin = 1; bin < n / 2; ++bin)
                    {
                        const double hz = bin * sample.sampleRate / n;
                        if (hz < 20.0 || hz > 16000.0) continue;
                        weighted += hz * data[(size_t)bin]; total += data[(size_t)bin];
                    }
                    centroidHz = total > 0.0 ? weighted / total : 0.0;
                };
                juce::String timbreNote;
                bool timbreKept = true, harmonicSource = false;
                if (auto it = filledStop->aliasFree.find(filledPipe); sourceSample != nullptr && it != filledStop->aliasFree.end() && !it->second.empty())
                {
                    double sourceRms, sourceCentroid, copyRms, copyCentroid;
                    analyse(*sourceSample, sourceRms, sourceCentroid);
                    analyse(*it->second[0], copyRms, copyCentroid);
                    if (sourceCentroid > 0.0 && copyCentroid > 0.0 && sourceRms > 0.0 && copyRms > 0.0)
                    {
                        // A pure tone (the synthetic sets) has no envelope to keep: its centroid is
                        // its pitch and moves with it; judged only when the source has harmonics.
                        const double f0 = 440.0 * std::pow(2.0, (source - 69) / 12.0);
                        const bool harmonic = sourceCentroid > 2.5 * f0;
                        harmonicSource = harmonic;
                        const double centroidRatio = copyCentroid / sourceCentroid;
                        const double levelDb = 20.0 * std::log10(copyRms / sourceRms);
                        // The loudness match works on the whole sample: that figure too.
                        auto wholeRms = [](const Sample& sample)
                        {
                            double power = 0.0;
                            const int frames = (int)sample.numFrames;
                            std::vector<int16_t> scratch;
                            std::vector<float> scratch32;
                            for (int ch = 0; ch < sample.numChannels; ++ch)
                            {
                                const int16_t* x16 = sample.storesFloat ? nullptr : sample.read16(ch, scratch);
                                const float* x32 = sample.storesFloat ? sample.read32(ch, scratch32) : nullptr;
                                for (int i = 0; i < frames; ++i)
                                {
                                    const double v = sample.storesFloat ? x32[i] : (double)x16[i] / 32768.0;
                                    power += v * v;
                                }
                            }
                            return std::sqrt(power / juce::jmax(1, frames * sample.numChannels));
                        };
                        const double wholeDb = 20.0 * std::log10(juce::jmax(1.0e-9, wholeRms(*it->second[0])) / juce::jmax(1.0e-9, wholeRms(*sourceSample)));
                        // The envelope is kept in place and moved along with the shift by the
                        // follow share: the centroid is expected at 2^(shift * follow / 12) of
                        // the source's (within a factor of 1.6 - the centroid also follows the
                        // harmonics, which did move).
                        // The stop's own setting (per stop since 0.8.17); upwards the
                        // envelope follows 30 points more than it.
                        const double setting = (double)engine.formantFollowForPipe(filledStop, filledPipe);
                        const int shift = filledPipe - source;
                        const double beyondOctave = juce::jmax(0.0, (std::abs(shift) - 12.0) / 12.0);
                        const double follow = juce::jlimit(0.0, 1.0, setting + (shift > 0 ? 0.3 : 0.0) + 0.15 * beyondOctave);   // the pitch shift method: 15 points per further octave
                        const double expectedRatio = std::pow(2.0, (double)(filledPipe - source) * follow / 12.0);
                        const bool centroidKept = centroidRatio > expectedRatio / 1.6 && centroidRatio < expectedRatio * 1.6;
                        timbreKept = (!harmonic || centroidKept) && std::abs(levelDb) < 3.0;
                        timbreNote = "; timbre: spectral centroid source " + juce::String(sourceCentroid, 0) + " Hz, copy " + juce::String(copyCentroid, 0)
                            + " Hz, ratio " + juce::String(centroidRatio, 2) + " (formants follow " + juce::String(follow * 100.0, 0) + " %: expected "
                            + juce::String(expectedRatio, 2) + (harmonic ? juce::String(", ") + (centroidKept ? "ok" : "NO") : juce::String(", pure tone, not judged"))
                            + "); loudness copy vs source " + juce::String(levelDb, 1) + " dB in the measured window, "
                            + juce::String(wholeDb, 1) + " dB over the whole sample";
                    }
                }
                // The formant move itself: the same copy at 0 % (envelope kept) and at 100 %
                // (envelope fully with the pitch) must differ in the direction of the shift,
                // otherwise the formant processing has no grip on this material.
                if (harmonicSource && sourceSample != nullptr)
                {
                    const float saved = filledStop->formantFollow;
                    auto centroidAt = [&](float counter)
                    {
                        engine.setStopFormantFollow(filledStop, counter);   // retires this stop's copies, builds again in the background
                        for (int w = 0; w < 120000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20);
                        double rms = 0.0, centroid = 0.0;
                        if (auto it = filledStop->aliasFree.find(filledPipe); it != filledStop->aliasFree.end() && !it->second.empty())
                            analyse(*it->second[0], rms, centroid);
                        return centroid;
                    };
                    const double at0 = centroidAt(0.0f), at100 = centroidAt(1.0f);
                    engine.setStopFormantFollow(filledStop, saved);
                    for (int w = 0; w < 120000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20);
                    const double ratio = at0 > 0.0 ? at100 / at0 : 0.0;
                    const int shift = filledPipe - source;
                    // The centroid follows the envelope only in part (the harmonics' own
                    // positions weigh in too): a clear move in the right direction counts,
                    // and the achieved share of the full envelope shift is reported.
                    const double movedSemitones = ratio > 0.0 ? 12.0 * std::log2(ratio) : 0.0;
                    const double share = shift != 0 ? movedSemitones / (double)shift : 0.0;
                    const bool works = at0 > 0.0 && at100 > 0.0 && share > 0.1;
                    timbreKept = timbreKept && works;
                    timbreNote << "; formants follow 0 % -> 100 %: centroid " << juce::String(at0, 0) << " -> " << juce::String(at100, 0)
                               << " Hz = " << juce::String(movedSemitones, 1) << " st (" << juce::String(share * 100.0, 0) << " % of the envelope shift, "
                               << (works ? "ok" : "NO") << ")";
                }
                // The loop seam of the copy: what the splice blends (the frames before the
                // loop end against the frames before the loop start) must match as well in
                // the copy as in the source - the copy's loop ends are moved to whole
                // periods of the shifted pitch.
                juce::String loopNote;
                bool loopsOk = true;
                if (sourceSample != nullptr && !sourceSample->loops.empty())
                {
                    auto seam = [](const Sample& sample, int which, int64_t& length)
                    {
                        const auto& loop = sample.loops[(size_t)which];
                        length = loop.end - loop.start;
                        const int64_t window = juce::jmin<int64_t>(loop.start, length - 1, (int64_t)4096);
                        if (window < 8) return 0.0;
                        double dot = 0.0, energyA = 0.0, energyB = 0.0;
                        std::vector<int16_t> scratch;
                        std::vector<float> scratch32;
                        for (int ch = 0; ch < sample.numChannels; ++ch)
                        {
                            const int16_t* x16 = sample.storesFloat ? nullptr : sample.read16(ch, scratch);
                            const float* x32 = sample.storesFloat ? sample.read32(ch, scratch32) : nullptr;
                            for (int64_t k = 0; k < window; ++k)
                            {
                                const int64_t ia = loop.start - window + k, ib = loop.end - window + k;
                                const double a = sample.storesFloat ? x32[ia] : (double)x16[ia] / 32768.0;
                                const double b = sample.storesFloat ? x32[ib] : (double)x16[ib] / 32768.0;
                                dot += a * b; energyA += a * a; energyB += b * b;
                            }
                        }
                        return energyA > 1.0e-12 && energyB > 1.0e-12 ? dot / std::sqrt(energyA * energyB) : 0.0;
                    };
                    int64_t sourceLength = 0, copyLength = 0;
                    const double sourceSeam = seam(*sourceSample, 0, sourceLength);
                    double copySeam = 0.0;
                    int copyLoops = 0;
                    if (auto it = filledStop->aliasFree.find(filledPipe); it != filledStop->aliasFree.end() && !it->second.empty())
                    {
                        copyLoops = (int)it->second[0]->loops.size();
                        if (copyLoops > 0) copySeam = seam(*it->second[0], 0, copyLength);
                    }
                    loopsOk = copyLoops == (int)sourceSample->loops.size() && (sourceSeam < 0.7 || copySeam > 0.5);
                    loopNote << "; loops: source " << (int)sourceSample->loops.size() << " (first " << sourceLength << " frames, seam match "
                             << juce::String(sourceSeam, 2) << "), copy " << copyLoops << " (first " << copyLength << " frames, seam match "
                             << juce::String(copySeam, 2) << ", " << (loopsOk ? "ok" : "NO") << ")";
                }
                timbreKept = timbreKept && loopsOk;
                timbreNote << loopNote;
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == filledStop);
                const int manual = filledStop->primaryManual;
                // The rendered loop seams: the key held across the first loop and one
                // more pass, the output captured, and the largest sample-to-sample jump
                // in any 10 ms window after the attack measured against the median of
                // those windows ("click factor"; a smooth loop stays near 1-2, a click
                // is far above). For the copy at 0 % and 50 % loop crossfade and for
                // the source pipe itself as the reference.
                juce::String seamNote;
                if (sourceSample != nullptr && !sourceSample->loops.empty())
                {
                    auto clickFactor = [&](int pipe, double seconds, double& atSecond)
                    {
                        std::vector<float> audio;
                        capture = &audio;
                        engine.noteOn(pipe, 127, manual);
                        BlockStats held; renderBlocks(juce::jlimit(100, 4000, (int)(seconds * 48000.0 / 256.0)), held);
                        engine.noteOff(pipe, manual);
                        capture = nullptr;
                        BlockStats tail; renderBlocks(150, tail);
                        const int window = 480, from = 24000;
                        std::vector<double> jumps;
                        std::vector<int> at;
                        for (int w = from; w + window <= (int)audio.size(); w += window)
                        {
                            double jump = 0.0;
                            for (int i = w + 1; i < w + window; ++i) jump = juce::jmax(jump, (double)std::abs(audio[(size_t)i] - audio[(size_t)i - 1]));
                            jumps.push_back(jump); at.push_back(w);
                        }
                        if (jumps.size() < 8) { atSecond = 0.0; return 0.0; }
                        std::vector<double> sorted = jumps;
                        std::nth_element(sorted.begin(), sorted.begin() + (long)(sorted.size() / 2), sorted.end());
                        const double median = juce::jmax(1.0e-9, sorted[sorted.size() / 2]);
                        size_t worst = 0;
                        for (size_t i = 1; i < jumps.size(); ++i) if (jumps[i] > jumps[worst]) worst = i;
                        atSecond = at[worst] / 48000.0;
                        return jumps[worst] / median;
                    };
                    const auto& loop0 = sourceSample->loops[0];
                    const double seconds = juce::jlimit(2.0, 20.0, ((double)loop0.end + 1.5 * (double)(loop0.end - loop0.start)) / 48000.0 + 0.5);
                    const float savedCrossfade = engine.getCopyLoopCrossfade();
                    double atSource = 0.0, at0 = 0.0, at50 = 0.0;
                    const double sourceFactor = clickFactor(source, seconds, atSource);
                    engine.setCopyLoopCrossfade(0.0f);
                    const double factor0 = clickFactor(filledPipe, seconds, at0);
                    engine.setCopyLoopCrossfade(0.5f);
                    const double factor50 = clickFactor(filledPipe, seconds, at50);
                    engine.setCopyLoopCrossfade(savedCrossfade);
                    const bool seamsOk = factor0 < juce::jmax(4.0, 2.0 * sourceFactor) || factor50 < juce::jmax(4.0, 2.0 * sourceFactor);
                    timbreKept = timbreKept && seamsOk;
                    seamNote << "; rendered " << juce::String(seconds, 1) << " s: click factor source pipe " << juce::String(sourceFactor, 1) << " (at " << juce::String(atSource, 2)
                             << " s), copy with 0 % crossfade " << juce::String(factor0, 1) << " (at " << juce::String(at0, 2) << " s), with 50 % " << juce::String(factor50, 1)
                             << " (at " << juce::String(at50, 2) << " s)" << (seamsOk ? "" : " NO");
                }
                timbreNote << seamNote;
                engine.noteOn(filledPipe, 127, manual);
                BlockStats on; renderBlocks(60, on);
                const int voices = engine.getCurrentPolyphony();
                engine.noteOff(filledPipe, manual);
                BlockStats off; renderBlocks(200, off);
                report << "Pitch shift (" << filledStop->name << ", filled pipe " << filledPipe << " from " << source << ", "
                       << (filledPipe - source) << " st): " << copies << " attack cop" << (copies == 1 ? "y" : "ies") << ", " << releaseCopies
                       << " release cop" << (releaseCopies == 1 ? "y" : "ies") << " built in " << juce::String(buildMs, 0) << " ms, attack " << lengthNote
                       << (sameLength ? "" : " NO") << releaseNote << timbreNote << "; key played -> voices " << voices << ", peak " << juce::String(on.peak, 4) << "  -> "
                       << (copies > 0 && sameLength && timbreKept && voices > 0 && on.peak > 0.0f ? "OK" : "*** WRONG ***") << "\n";
                // Every pitch-shift method on two filled pipes: the one above (the first
                // found) and one an octave below the compass, made by a transmission
                // shifted -12 (the user's 32' extension of a 16' stop). Judged: the
                // attack's length to the first loop and the release's length against the
                // method's rule (the start of a release is trimmed by the setting), and
                // the level step at the key release - the sustain's last 50 ms against the
                // release's first 50 ms - which must be what the real source pipe shows.
                // The transmission goes into another division (when there is one), so
                // that its key plays the shifted pipe alone - on the stop's own division
                // the key would sound its own pipe as well.
                const int downManual = divisions > 1 ? (manual + 1) % divisions : -1;
                const int transmissionIndex = downManual >= 0 ? engine.addTransmission(filledStop, downManual, -12) : -1;
                for (int w = 0; w < 120000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20);
                int pipeDown = -1;
                if (downManual >= 0)
                    for (int pipe = 0; pipe < 128 && pipeDown < 0; ++pipe)
                        if (filledStop->isFilled(pipe) && filledStop->fillOffset[(size_t)pipe] == 12) pipeDown = pipe;
                // The step is measured with the crossfade at the key release made short
                // (fade-out 10 ms, no fade-in; they scale with the pitch and would hide
                // the release's own level) in the window 15-65 ms after the key release.
                engine.setReleaseFadeOutMs(10.0f);
                engine.setReleaseFadeInMs(0.0f);
                // (key, division): a pipe of the stop's own division is its key there; the
                // pipe an octave below the compass is played by its key on the
                // transmission's division.
                auto levelStep = [&](int pipe)
                {
                    const bool viaTransmission = pipe == pipeDown && downManual >= 0;
                    const int key = viaTransmission ? pipe + 12 : pipe, division = viaTransmission ? downManual : manual;
                    std::vector<float> audio;
                    capture = &audio;
                    engine.noteOn(key, 127, division);
                    BlockStats held; renderBlocks(560, held);   // 3 s: well into the sustain
                    const size_t before = audio.size();
                    engine.noteOff(key, division);
                    BlockStats tail; renderBlocks(25, tail);    // the first 130 ms of the release
                    capture = nullptr;
                    BlockStats rest; renderBlocks(150, rest);
                    auto rms = [&](size_t a, size_t b)
                    {
                        double power = 0.0;
                        for (size_t i = a; i < b && i < audio.size(); ++i) power += (double)audio[i] * audio[i];
                        return std::sqrt(power / (double)std::max<size_t>(1, b - a));   // not jmax<size_t>: clang instantiates the SIMDRegister overload
                    };
                    const size_t window = 4800, skip = 720;   // 100 ms windows, the release's after the 10 ms fade-out
                    const double sustain = rms(before > window ? before - window : 0, before), release = rms(before + skip, before + skip + window);
                    return 20.0 * std::log10(juce::jmax(1.0e-9, release) / juce::jmax(1.0e-9, sustain));
                };
                // The same relation straight from the samples: the release's first 100 ms
                // (the source's after the trimmed part) against the sustain (the loops).
                auto staticStep = [&](const Sample* attack, const Sample* releaseSample, double skipSeconds)
                {
                    if (attack == nullptr || releaseSample == nullptr) return 0.0;
                    auto rmsOf = [](const Sample& s, int64_t from, int64_t to)
                    {
                        from = juce::jlimit<int64_t>(0, s.numFrames, from); to = juce::jlimit<int64_t>(from, s.numFrames, to);
                        double power = 0.0;
                        std::vector<int16_t> scratch;
                        std::vector<float> scratch32;
                        for (int ch = 0; ch < s.numChannels; ++ch)
                        {
                            const int16_t* x16 = s.storesFloat ? nullptr : s.read16(ch, scratch);
                            const float* x32 = s.storesFloat ? s.read32(ch, scratch32) : nullptr;
                            for (int64_t i = from; i < to; ++i) { const double v = s.storesFloat ? (double)x32[i] : (double)x16[i] / 32768.0; power += v * v; }
                        }
                        return std::sqrt(power / (double)juce::jmax<int64_t>(1, (to - from) * juce::jmax(1, s.numChannels)));
                    };
                    int64_t from = attack->numFrames * 7 / 10, to = attack->numFrames;
                    if (!attack->loops.empty()) { from = attack->loops.front().start; to = juce::jmax(from + 1, attack->maxLoopEnd > from ? attack->maxLoopEnd : attack->numFrames); }
                    const double sustain = rmsOf(*attack, from, to);
                    const int64_t skip = (int64_t)(skipSeconds * releaseSample->sampleRate);
                    const double start = rmsOf(*releaseSample, skip, skip + (int64_t)(0.1 * releaseSample->sampleRate));
                    return 20.0 * std::log10(juce::jmax(1.0e-9, start) / juce::jmax(1.0e-9, sustain));
                };
                const double sourceStepUp = levelStep(source);
                const double sourceStepDown = pipeDown >= 0 ? levelStep(pipeDown + 12) : 0.0;
                // ORGAN_SELFTEST_METHODS=1,2,3 (development): only these methods - the
                // Rubber Band build of a whole stop takes minutes.
                juce::String methodsWanted = "1,2,3";
                if (const char* env = std::getenv("ORGAN_SELFTEST_METHODS")) methodsWanted = env;
                for (const int method : { 1, 2, 3 }) if (sourceSample != nullptr && juce::StringArray::fromTokens(methodsWanted, ",", "").contains(juce::String(method)))
                {
                    engine.setTransposeMethod(method);
                    {
                        SamplerEngine::BuildPause pause(engine);   // the copies of this stop again, with the new method
                        std::vector<Stop*> ranks{ filledStop };
                        for (auto& rank : filledStop->ranks) ranks.push_back(rank.get());
                        const juce::ScopedLock sl(engine.getLock());
                        // Retired, not freed: the release voice of the previous method may still read them.
                        auto retire = [](Stop* rank, std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, std::map<int, int>& offsets)
                        {
                            for (auto& [pipe, copies] : aliasFree) for (auto& sample : copies) rank->retiredSamples.push_back(std::move(sample));
                            aliasFree.clear(); offsets.clear();
                        };
                        for (auto* rank : ranks)
                        {
                            retire(rank, rank->aliasFree, rank->aliasFreeOffset);
                            for (auto& layer : rank->releaseLayers) retire(rank, layer.aliasFree, layer.aliasFreeOffset);
                        }
                    }
                    const double t2 = juce::Time::getMillisecondCounterHiRes();
                    engine.buildAntiAliasCopies(filledStop, false);
                    const double build2Ms = juce::Time::getMillisecondCounterHiRes() - t2;
                    bool ok = true;
                    juce::String note;
                    // The rules: method 1 resamples (the attack to 1/ratio, the release to the
                    // release length setting); method 2 gives the attack's timing back downwards
                    // only and the release's by the release timing share; the vocoders keep
                    // every length, their releases shortened upwards by the share. Every
                    // release loses the trimmed start.
                    auto judge = [&](int pipe, int src, double sourceStep)
                    {
                        if (pipe < 0 || src < 0 || filledStop->samplesByNote[(size_t)src].empty()) return;
                        const Sample* srcSample = filledStop->samplesByNote[(size_t)src][0].get();
                        const double ratio = std::pow(2.0, (double)(pipe - src) / 12.0);
                        const double attackKeep = method >= 3 ? 1.0 : method == 2 && ratio < 1.0
                            ? std::pow(1.0 / ratio, 1.0 - (double)engine.getTransposeAttackTiming()) : 1.0 / ratio;
                        const double releaseKeep = method == 1 ? (double)engine.getTransposeReleaseLength()
                            : method == 2 || ratio > 1.0 ? std::pow(1.0 / ratio, 1.0 - (double)engine.getTransposeReleaseTiming()) : 1.0;
                        note << (note.isEmpty() ? "" : " | ") << "pipe " << pipe << " from " << src << " (" << (pipe - src > 0 ? "+" : "") << (pipe - src) << " st): ";
                        int64_t srcFirstLoop = -1;
                        for (const auto& loop : srcSample->loops) if (srcFirstLoop < 0 || loop.start < srcFirstLoop) srcFirstLoop = loop.start;
                        if (auto it = filledStop->aliasFree.find(pipe); it != filledStop->aliasFree.end() && !it->second.empty())
                        {
                            const Sample& copy = *it->second[0];
                            int64_t copyFirstLoop = -1;
                            for (const auto& loop : copy.loops) if (copyFirstLoop < 0 || loop.start < copyFirstLoop) copyFirstLoop = loop.start;
                            if (srcFirstLoop > 0 && copyFirstLoop > 0)
                            {
                                const double expected = (double)srcFirstLoop * attackKeep;
                                const double periodFrames = srcSample->sampleRate / juce::jmax(1.0, 440.0 * std::pow(2.0, ((double)pipe - 69.0) / 12.0));
                                const bool attackOk = std::abs((double)copyFirstLoop - expected) <= 0.02 * expected + 2.0 * periodFrames;   // the join lands within a window
                                ok = ok && attackOk && copy.loops.size() == srcSample->loops.size();
                                note << "attack " << juce::String(100.0 * (double)copyFirstLoop / (double)srcFirstLoop, 0) << " % of the source (expected "
                                     << juce::String(attackKeep * 100.0, 0) << " %" << (attackOk ? ")" : ", NO)") << ", loops " << copy.loops.size() << "/" << srcSample->loops.size();
                            }
                            else note << "attack " << copy.numFrames << " frames (no loops to judge by)";
                        }
                        else { note << "no attack copy"; ok = false; }
                        for (const auto& layer : filledStop->releaseLayers)
                            if (auto it = layer.aliasFree.find(pipe); it != layer.aliasFree.end() && !it->second.empty() && !layer.samplesByNote[(size_t)src].empty())
                            {
                                const Sample& srcRelease = *layer.samplesByNote[(size_t)src][0];
                                const double expectedFrames = (double)srcRelease.numFrames * releaseKeep;   // (the trim is a playback setting)
                                const bool releaseOk = std::abs((double)it->second[0]->numFrames - expectedFrames) <= 0.02 * (double)srcRelease.numFrames + 1.0;
                                ok = ok && releaseOk;
                                note << ", release " << juce::String(100.0 * (double)it->second[0]->numFrames / (double)juce::jmax<int64_t>(1, srcRelease.numFrames), 0)
                                     << " % of the source (expected " << juce::String(100.0 * expectedFrames / (double)juce::jmax<int64_t>(1, srcRelease.numFrames), 0) << " %"
                                     << (releaseOk ? ")" : ", NO)");
                                break;
                            }
                        // Reported, not judged: the rendered step differs from the samples'
                        // relation by the playback path (open question since 0.8.22).
                        const double step = levelStep(pipe);
                        const bool stepOk = std::abs(step - sourceStep) <= 4.0;
                        // ... and the same with the release played from its very start (the
                        // "played from" setting at playback must only skip the start).
                        const float savedTrim = engine.getTransposeReleaseTrim();
                        engine.setTransposeReleaseTrim(0.0f);
                        const double stepUntrimmed = levelStep(pipe);
                        engine.setTransposeReleaseTrim(savedTrim);
                        note << ", level step at the key release " << juce::String(step, 1) << " dB (source pipe " << juce::String(sourceStep, 1) << " dB" << (stepOk ? ")" : ", NO)")
                             << ", with the release played from 0 ms " << juce::String(stepUntrimmed, 1) << " dB";
                        // ... and the relation in the samples themselves (what the build matched).
                        const Sample* copyAttack = nullptr;
                        if (auto it = filledStop->aliasFree.find(pipe); it != filledStop->aliasFree.end() && !it->second.empty()) copyAttack = it->second[0].get();
                        for (const auto& layer : filledStop->releaseLayers)
                            if (auto it = layer.aliasFree.find(pipe); it != layer.aliasFree.end() && !it->second.empty() && !layer.samplesByNote[(size_t)src].empty())
                            {
                                // The copy's own relation, and the one the matching option would play it at.
                                const double sourceStatic = staticStep(srcSample, layer.samplesByNote[(size_t)src][0].get(), 0.0);
                                const double copyStatic = staticStep(copyAttack, it->second[0].get(), 0.0);
                                const double matched = copyStatic + 20.0 * std::log10(juce::jmax(1.0e-6, (double)it->second[0]->releaseLevelGain));
                                const bool staticOk = std::abs(matched - sourceStatic) <= 1.5;
                                ok = ok && staticOk;
                                note << ", in the samples release/sustain source " << juce::String(sourceStatic, 1) << " dB, copy " << juce::String(copyStatic, 1)
                                     << " dB (matched " << juce::String(matched, 1) << " dB" << (staticOk ? ")" : ", NO)");
                                break;
                            }
                    };
                    judge(filledPipe, source, sourceStepUp);
                    if (pipeDown >= 0) judge(pipeDown, pipeDown + 12, sourceStepDown);
                    report << (method == 4 ? "Rubber Band (" : method == 3 ? "Phase vocoder (" : method == 2 ? "Pitch shift, timing kept (" : "Pitch shift, both ways (")
                           << filledStop->name << "): built in " << juce::String(build2Ms, 0) << " ms; " << note << "  -> " << (ok ? "OK" : "*** WRONG ***") << "\n";
                }
                if (transmissionIndex >= 0) engine.removeTransmission(filledStop, transmissionIndex);
                engine.setReleaseFadeOutMs(70.0f);
                engine.setReleaseFadeInMs(70.0f);
                // The tests below (interruption, cache) run with the pitch shift method again:
                // the vocoders take minutes for a whole stop, which would time them out.
                // The rebuild this launches must finish first (the next test starts its own).
                engine.setTransposeMethod(1);
                for (int w = 0; w < 300000 && engine.pendingBackgroundWorkers() > 0; w += 50) juce::Thread::sleep(50);
                // Build interruption: with the copies of every stop building in the
                // background, switching a pipe off (the voicing's checkbox) must return
                // at once - the running build stops at its next pipe - and the builds
                // go on afterwards until every stop has its copies.
                {
                    {
                        // Everything again - retired under the build lock (a build or a
                        // voice may still read the copies; see the rule in the engine).
                        SamplerEngine::BuildPause pause(engine);
                        const juce::ScopedLock sl(engine.getLock());
                        for (auto* stop : engine.getStops())
                        {
                            std::vector<Stop*> ranks{ stop };
                            for (auto& rank : stop->ranks) ranks.push_back(rank.get());
                            for (auto* rank : ranks)
                            {
                                for (auto& [pipe, copies] : rank->aliasFree) for (auto& sample : copies) rank->retiredSamples.push_back(std::move(sample));
                                rank->aliasFree.clear(); rank->aliasFreeOffset.clear();
                            }
                        }
                    }
                    for (auto* stop : engine.getStops()) engine.buildAntiAliasCopies(stop, true);
                    juce::Thread::sleep(400);
                    int queuedBefore = 0;
                    for (auto* stop : engine.getStops()) if (stop->copiesQueued.load() || stop->copiesBuilding.load()) ++queuedBefore;
                    const double t1 = juce::Time::getMillisecondCounterHiRes();
                    engine.setPipeEnabled(filledStop, filledPipe, false);
                    const double toggleMs = juce::Time::getMillisecondCounterHiRes() - t1;
                    engine.setPipeEnabled(filledStop, filledPipe, true);
                    double waited = 0.0;
                    for (; waited < 120000.0; waited += 50.0)
                    {
                        bool any = false;
                        for (auto* stop : engine.getStops()) if (stop->copiesQueued.load() || stop->copiesBuilding.load()) any = true;
                        if (!any) break;
                        juce::Thread::sleep(50);
                    }
                    int missing = 0;
                    juce::String detail;
                    for (auto* stop : engine.getStops()) missing += engine.countMissingCopies(stop, &detail);
                    report << "Build interruption (" << queuedBefore << " stops building): pipe off/on took " << juce::String(toggleMs, 0)
                           << " ms; builds finished after " << juce::String(waited / 1000.0, 1) << " s, copies still missing " << missing
                           << (missing > 0 ? " (" + detail.trim() + ")" : juce::String()) << "  -> "
                           << (toggleMs < 5000.0 && missing == 0 ? "OK" : "*** WRONG ***") << "\n";
                }
                // Copy cache: the copies one build writes to disk, the next build reads
                // (same parameters, same sources) - identical samples, no computing.
                {
                    const auto cacheDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("OrganSampler_selftest_cache");
                    cacheDir.deleteRecursively();
                    auto waitForWorkers = [&] { for (int w = 0; w < 60000 && engine.pendingBackgroundWorkers() > 0; w += 20) juce::Thread::sleep(20); };
                    // Switched on with the copies already built: they are exported at once.
                    engine.copyCacheWrites = 0;
                    engine.setCopyCache(true, cacheDir);
                    waitForWorkers();
                    const int exported = engine.copyCacheWrites.load();
                    const int exportedFiles = cacheDir.getNumberOfChildFiles(juce::File::findFiles, "*.ospc");
                    const auto exportedBytes = engine.copyCacheBytes();
                    engine.clearCopyCache();   // now the builds below start from an empty cache
                    auto retireAll = [&]
                    {
                        SamplerEngine::BuildPause pause(engine);   // never touch a stop's copies without the build lock
                        std::vector<Stop*> ranks{ filledStop };
                        for (auto& rank : filledStop->ranks) ranks.push_back(rank.get());
                        for (auto* rank : ranks)
                        {
                            rank->aliasFree.clear(); rank->aliasFreeOffset.clear();
                            for (auto& layer : rank->velocityLayers) { layer.aliasFree.clear(); layer.aliasFreeOffset.clear(); }
                            for (auto& layer : rank->releaseLayers) { layer.aliasFree.clear(); layer.aliasFreeOffset.clear(); }
                        }
                    };
                    // Retiring the copies (under the pause) relaunches the background build at
                    // once: the timing is from the retirement to the end of that build.
                    engine.copyCacheHits = 0; engine.copyCacheWrites = 0;
                    const double c0 = juce::Time::getMillisecondCounterHiRes();
                    retireAll();
                    waitForWorkers();
                    const double builtMs = juce::Time::getMillisecondCounterHiRes() - c0;
                    const int writes = engine.copyCacheWrites.load();
                    std::vector<float> head; int64_t frames = 0;
                    if (auto it = filledStop->aliasFree.find(filledPipe); it != filledStop->aliasFree.end() && !it->second.empty())
                    {
                        const auto& s = *it->second[0];
                        frames = s.numFrames;
                        std::vector<int16_t> scratch;
                        std::vector<float> scratch32;
                        const int16_t* x16 = s.storesFloat ? nullptr : s.read16(0, scratch);
                        const float* x32 = s.storesFloat ? s.read32(0, scratch32) : nullptr;
                        for (int64_t i = 0; i < juce::jmin<int64_t>(4000, s.numFrames); ++i)
                            head.push_back(s.storesFloat ? x32[i] : (float)x16[i]);
                    }
                    engine.copyCacheHits = 0;
                    const double c1 = juce::Time::getMillisecondCounterHiRes();
                    retireAll();
                    waitForWorkers();
                    const double cachedMs = juce::Time::getMillisecondCounterHiRes() - c1;
                    const int hits = engine.copyCacheHits.load();
                    bool identical = false;
                    if (auto it = filledStop->aliasFree.find(filledPipe); it != filledStop->aliasFree.end() && !it->second.empty())
                    {
                        const auto& s = *it->second[0];
                        identical = s.numFrames == frames && !head.empty();
                        std::vector<int16_t> scratch;
                        std::vector<float> scratch32;
                        const int16_t* x16 = s.storesFloat ? nullptr : s.read16(0, scratch);
                        const float* x32 = s.storesFloat ? s.read32(0, scratch32) : nullptr;
                        for (size_t i = 0; identical && i < head.size(); ++i)
                            if ((s.storesFloat ? x32[(int64_t)i] : (float)x16[(int64_t)i]) != head[i]) identical = false;
                    }
                    report << "Copy cache: switched on with the copies built -> " << exported << " set file(s) exported for every stop (" << exportedFiles
                           << " on disk, " << juce::File::descriptionOfSizeInBytes(exportedBytes) << "); " << filledStop->name << " built again from nothing in "
                           << juce::String(builtMs, 0) << " ms writing " << writes << " set file(s), then again in " << juce::String(cachedMs, 0) << " ms with "
                           << hits << " pipes from the cache, copy identical " << (identical ? "yes" : "NO") << "  -> "
                           << (exported > 0 && writes > 0 && hits > 0 && identical ? "OK" : "*** WRONG ***") << "\n";
                    engine.setCopyCache(false, juce::File());
                    waitForWorkers();
                    cacheDir.deleteRecursively();
                }
                engine.setTransposeMethod(0);
                engine.buildAntiAliasCopies(filledStop, false);
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            }
        }
        {
            // Key velocity on a stop without velocity layers: never softer, only a
            // fade-in at the start that is longest at the lowest velocity.
            Stop* plain = nullptr;
            for (auto* stop : engine.getStops()) if (stop->velocityLayers.empty() && !stop->container) { plain = stop; break; }
            if (plain != nullptr)
            {
                int key = -1;
                for (int n = 0; n < 128 && key < 0; ++n) if (plain->hasNote(n)) key = n;
                if (key >= 0)
                {
                    for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == plain);
                    const int manual = plain->primaryManual;
                    auto peaks = [&](int velocity, float& early, float& whole)
                    {
                        engine.noteOn(key, velocity, manual);
                        BlockStats first; renderBlocks(2, first);        // the first ~10 ms
                        BlockStats rest; renderBlocks(60, rest);
                        engine.noteOff(key, manual);
                        BlockStats tail; renderBlocks(150, tail);
                        early = first.peak;
                        whole = juce::jmax(first.peak, rest.peak);
                    };
                    float softEarly = 0.0f, softWhole = 0.0f, loudEarly = 0.0f, loudWhole = 0.0f;
                    peaks(1, softEarly, softWhole);
                    peaks(127, loudEarly, loudWhole);
                    const bool sameLoudness = softWhole > 0.5f * loudWhole && softWhole < 2.0f * loudWhole;
                    const bool faded = engine.getVelocityFadeInMs() <= 0.0f || softEarly < 0.7f * loudEarly;
                    report << "Key velocity (" << plain->name << ", key " << key << ", fade-in " << juce::String(engine.getVelocityFadeInMs(), 0)
                           << " ms at velocity 1): peak over 320 ms at velocity 1 = " << juce::String(softWhole, 4) << ", at 127 = "
                           << juce::String(loudWhole, 4) << " (same loudness " << (sameLoudness ? "yes" : "NO") << "); first 10 ms at 1 = "
                           << juce::String(softEarly, 4) << ", at 127 = " << juce::String(loudEarly, 4) << " (fade-in " << (faded ? "yes" : "NO")
                           << ")  -> " << (sameLoudness && faded ? "OK" : "*** WRONG ***") << "\n";
                    for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
                }
            }
        }
        {
            // Velocity layers ("_NNN" files): a key played up to NNN uses the layer's
            // sample, louder keys the plain one; with the layer switched off the plain
            // sample plays for the soft key as well.
            Stop* layered = nullptr;
            for (auto* stop : engine.getStops()) if (!stop->velocityLayers.empty()) { layered = stop; break; }
            if (layered != nullptr)
            {
                const int maxVelocity = layered->velocityLayers.front().maxVelocity;
                int note = -1;
                for (int n = 0; n < 128 && note < 0; ++n)
                    if (!layered->velocityLayers.front().samplesByNote[(size_t)n].empty() && layered->hasNote(n)) note = n;
                if (note >= 0)
                {
                    for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, stop == layered);
                    const int manual = layered->primaryManual;
                    float releasePeak = 0.0f;
                    auto peakFor = [&](int velocity, int key = -1)
                    {
                        if (key < 0) key = note;
                        engine.noteOn(key, velocity, manual);
                        BlockStats on; renderBlocks(60, on);
                        engine.noteOff(key, manual);
                        BlockStats off; renderBlocks(30, off);      // the release sample's own level
                        releasePeak = off.peak;
                        BlockStats tail; renderBlocks(120, tail);
                        return on.peak;
                    };
                    const float soft = peakFor(maxVelocity);         // the layer
                    const float softRelease = releasePeak;
                    const float loud = peakFor(127);                 // the plain sample
                    const float loudRelease = releasePeak;
                    engine.setVelocityLayerEnabled(layered, maxVelocity, false);
                    const float softPlain = peakFor(maxVelocity);    // the next layer up (or the plain sample) at the soft velocity
                    engine.setVelocityLayerEnabled(layered, maxVelocity, true);
                    const float softAgain = peakFor(maxVelocity);
                    // Real sets may hold the same level in every layer (the fine-tuned case); the
                    // judgement is "every path sounds", the levels are reported for the eye.
                    const bool differs = std::abs(softPlain - soft) > 0.05f * juce::jmax(soft, softPlain);
                    report << "Velocity layers (" << layered->name << ", key " << note << ", layer _" << maxVelocity << " of "
                           << (int)layered->velocityLayers.size() << "): peak at velocity " << maxVelocity << " = " << juce::String(soft, 4)
                           << ", at 127 = " << juce::String(loud, 4) << ", at " << maxVelocity << " with the layer off = " << juce::String(softPlain, 4)
                           << (differs ? " (another sample)" : " (same level)") << ", on again = " << juce::String(softAgain, 4) << "  -> "
                           << (soft > 0.0f && loud > 0.0f && softPlain > 0.0f && softAgain > 0.0f ? "OK" : "*** WRONG ***") << "\n";
                    // Releases follow the attack's layer: with velocity-layered release files
                    // the soft key's release differs from the loud key's.
                    bool layeredReleases = false;
                    for (const auto& layer : layered->releaseLayers) layeredReleases = layeredReleases || layer.maxVelocity >= 0;
                    if (layeredReleases)
                        report << "  release of the soft key: peak " << juce::String(softRelease, 4) << ", of the loud key: " << juce::String(loudRelease, 4)
                               << "  -> " << (softRelease > 0.0f && loudRelease > 0.0f
                                   ? (std::abs(loudRelease - softRelease) > 0.05f * juce::jmax(softRelease, loudRelease) ? "OK (another release)" : "OK (same level)") : "*** WRONG ***") << "\n";
                    // A filled pipe (above the compass, transposed from a real one) follows the layers too.
                    int highest = -1;
                    for (int n = 127; n >= 0 && highest < 0; --n) if (layered->hasNote(n)) highest = n;
                    int filledKey = -1;
                    for (int n = highest + 1; n < 128 && filledKey < 0; ++n) if (layered->isFilled(n)) filledKey = n;
                    if (filledKey >= 0)
                    {
                        const float filledSoft = peakFor(maxVelocity, filledKey), filledLoud = peakFor(127, filledKey);
                        report << "  filled pipe " << filledKey << " (from " << layered->sourceNoteFor(filledKey) << "): peak at velocity " << maxVelocity
                               << " = " << juce::String(filledSoft, 4) << ", at 127 = " << juce::String(filledLoud, 4) << "  -> "
                               << (filledSoft > 0.0f && filledLoud > 0.0f ? "OK" : "*** WRONG ***") << "\n";
                    }
                    for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
                }
            }
        }
        {
            for (int note = 48; note < 58; ++note) noteOnAll(note);
            BlockStats s; renderBlocks(400, s); describe("10-note chord, all stops", s);
            for (int note = 48; note < 58; ++note) noteOffAll(note);
            BlockStats r; renderBlocks(200, r); describe("  release tails", r);
        }
        {
            const auto wav = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("OrganSampler").getChildFile("selftest.wav");
            const bool recording = engine.startRecording(wav);
            for (int note = 36; note < 97; ++note) noteOnAll(note);
            BlockStats s; renderBlocks(600, s); describe("61-note tutti (massive)", s);
            for (int note = 36; note < 97; ++note) noteOffAll(note);
            BlockStats r; renderBlocks(400, r); describe("  release tails", r);
            const double seconds = engine.getRecordedSeconds();
            engine.stopRecording();
            report << "Recording: " << (recording ? "started" : "FAILED") << ", " << juce::String(seconds, 2)
                   << " s flushed, file " << juce::String(wav.getSize() >> 10) << " KB -> "
                   << (recording && wav.getSize() > 1000000 ? "OK" : "*** CHECK ***") << "\n";
        }
        {
            // Release tails of a full tutti last several seconds; striking the
            // whole tutti three times stacks them, which is how 40k+ voices arise.
            // Run twice, without and with recording: recording must not cost
            // the audio thread anything noticeable.
            auto strikeThreeTimes = [&](const juce::String& label)
            {
                // Start from silence so both passes see the same voice counts.
                for (int drain = 0; drain < 400 && engine.getCurrentPolyphony() > 0; ++drain)
                {
                    BlockStats dummy;
                    renderBlocks(10, dummy);
                }
                BlockStats s;
                for (int strike = 0; strike < 3; ++strike)
                {
                    for (int note = 36; note < 97; ++note) noteOnAll(note);
                    renderBlocks(80, s);
                    for (int note = 36; note < 97; ++note) noteOffAll(note);
                    renderBlocks(25, s);
                }
                describe(label, s);
                BlockStats r; renderBlocks(300, r); describe("  stacked release tails", r);
            };
            strikeThreeTimes("61-note tutti struck 3x (tails stack)");
            const auto wav2 = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("OrganSampler").getChildFile("selftest-recording.wav");
            const bool recording2 = engine.startRecording(wav2);
            strikeThreeTimes("61-note tutti struck 3x WHILE RECORDING");
            const double seconds2 = engine.getRecordedSeconds();
            engine.stopRecording();
            report << "Recording during tutti: " << (recording2 ? "started" : "FAILED") << ", "
                   << juce::String(seconds2, 2) << " s flushed, file " << juce::String(wav2.getSize() >> 10) << " KB\n";
        }
        {
            BlockStats s;
            for (int i = 0; i < 40; ++i)
            {
                noteOnAll(60); renderBlocks(3, s);
                noteOffAll(60); renderBlocks(3, s);
            }
            describe("Rapid repetition", s);
        }
        {
            for (int note = 48; note < 60; ++note) noteOnAll(note);
            BlockStats s;
            for (int i = 0; i < 12; ++i)
            {
                for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, (i & 1) != 0);
                renderBlocks(20, s);
            }
            for (auto* stop : engine.getStops()) engine.setStopEnabled(stop, true);
            renderBlocks(40, s);
            describe("Stop toggling while held", s);
            engine.noteOffAll();
            BlockStats p; renderBlocks(40, p); describe("  after panic", p);
        }
        {
            for (int note = 36; note < 97; ++note) noteOnAll(note);
            BlockStats s; renderBlocks(100, s);
            for (int manual = 0; manual < divisions; ++manual) engine.unloadManual(manual);
            renderBlocks(20, s);
            describe("Unload while sounding", s);
            report << "Stops after unload: " << engine.getStops().size() << "  polyphony " << engine.getCurrentPolyphony() << "\n";
        }
        report << "SELF TEST COMPLETED WITHOUT CRASH\n";
        return report;
    }
}
