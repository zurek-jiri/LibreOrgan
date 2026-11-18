#pragma once

// Plug-in hosting (0.9.102): VST3 on every system, Audio Units on macOS too.
//
// Foreign code comes into the program here, and some of it is faulty. What keeps the program standing:
//   - The SCAN of a plug-in file runs in a process of its own (the program started with "--pluginscan <request>"):
//     a file that crashes its scan, or does not answer within the time given, is refused with the reason and not
//     asked again until the file changes. The user never waits for a scan: it runs behind the playing.
//   - Before a plug-in is LOADED into the program its name is written to a marker file and removed afterwards;
//     another marker says "plug-ins are loaded in this session" and is removed at a clean exit. A start that finds
//     the session marker is a SAFE START: every plug-in stays switched off (not even loaded) until the user
//     switches it on, and the plug-in that was being loaded when the program died is named.
//
// Header-only like the other dialogs' helpers; message thread unless said otherwise.

#include <JuceHeader.h>
#include "CrashHandler.h"
#if JUCE_WINDOWS
 #include <windows.h>
#endif

namespace PluginHost
{
    inline juce::File settingsFolder()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("LibreOrgan");
    }
    inline juce::File knownListFile()   { return settingsFolder().getChildFile("Plugins.xml"); }
    inline juce::File scanStampsFile()  { return settingsFolder().getChildFile("PluginScan.json"); }
    inline juce::File loadingMarker()   { return settingsFolder().getChildFile("PluginLoading.txt"); }
    inline juce::File sessionMarker()   { return settingsFolder().getChildFile("PluginsActive.txt"); }

    inline juce::AudioPluginFormatManager& formats()
    {
        static juce::AudioPluginFormatManager manager;
        static bool filled = false;
        if (!filled) { filled = true; juce::addDefaultFormatsToManager(manager); }
        return manager;
    }
    inline juce::AudioPluginFormat* formatNamed(const juce::String& name)
    {
        auto& manager = formats();
        for (int i = 0; i < manager.getNumFormats(); ++i)
            if (auto* format = manager.getFormat(i); format != nullptr && format->getName() == name) return format;
        return nullptr;
    }

    //==========================================================================
    // The scan worker: the program started with --pluginscan "<request file>". The request holds three lines: the
    // format's name, the plug-in file (or identifier), the result file. The result is the XML of every plug-in type
    // of the file; no result file = the scan died. Never returns to the caller's normal start.
    // A helper process (the scan of a plug-in, the trial of its window, the question to a sound card) that fails must
    // die WITHOUT A WORD: no dialog of Windows ("LibreOrgan has stopped working" - the user saw it twice while the
    // program itself went on: the helpers trying MeldaProduction's windows), no report, no abort message.
    inline void helperDiesQuietly()
    {
       #if JUCE_WINDOWS
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
        SetUnhandledExceptionFilter([](EXCEPTION_POINTERS*) -> LONG { TerminateProcess(GetCurrentProcess(), 3); return EXCEPTION_EXECUTE_HANDLER; });
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
       #endif
        std::set_terminate([] { std::_Exit(3); });
    }

    inline bool isScanWorker(const juce::String& commandLine) { return commandLine.contains("--pluginscan"); }
    [[noreturn]] inline void runScanWorker(const juce::String& commandLine)
    {
        helperDiesQuietly();   // a crashing plug-in must raise no system dialog
        const juce::File request(commandLine.fromFirstOccurrenceOf("--pluginscan", false, false).trim().unquoted().trim());
        juce::StringArray lines;
        lines.addLines(request.loadFileAsString());
        if (lines.size() >= 3)
            if (auto* format = formatNamed(lines[0].trim()))
            {
                juce::OwnedArray<juce::PluginDescription> found;
                format->findAllTypesForFile(found, lines[1].trim());
                juce::XmlElement root("SCAN");
                for (auto* description : found)
                    if (description != nullptr) root.addChildElement(description->createXml().release());
                root.writeTo(juce::File(lines[2].trim()));
            }
        std::fflush(nullptr);
        std::_Exit(0);   // no destructors: a plug-in's left-over threads and statics must not hold the exit
    }

    //==========================================================================
    // The TRIAL of a plug-in's window in a process of its own (0.9.107, the user: "musime umet vadne VST zvladnout,
    // aby program nespadl"). A plug-in makes its window inside its own code, and a faulty one fails there - or
    // hands out a view that is already dead, so that the program ends a moment later in a place nobody can guard
    // (two of MeldaProduction's did, at random). So the window is opened FIRST in a helper: this program started
    // with --plugineditor "<request file>" (the request: the result file on the first line, then the plug-in's
    // description as XML). The helper loads the plug-in, prepares it, shows its window in the screen's corner for a
    // second and a half with the message loop running and writes "ok<TAB>size"; no result = it crashed, thirty
    // seconds without an end = it hangs. Only a plug-in that passed (twice: the faults are random) gets its window
    // in the program itself.
    inline bool isEditorTrial(const juce::String& commandLine) { return commandLine.contains("--plugineditor"); }

    struct EditorTrial final : private juce::Timer
    {
        std::unique_ptr<juce::AudioPluginInstance> instance;
        std::unique_ptr<juce::DocumentWindow> window;
        juce::File result;

        [[noreturn]] void finish(const juce::String& verdict)
        {
            result.replaceWithText(verdict);
            std::fflush(nullptr);
            std::_Exit(0);   // no destructors: the plug-in's own threads and statics must not hold the exit
        }
        void start(const juce::String& commandLine)
        {
            helperDiesQuietly();   // a crashing plug-in must raise no system dialog
            const juce::File request(commandLine.fromFirstOccurrenceOf("--plugineditor", false, false).trim().unquoted().trim());
            const auto text = request.loadFileAsString();
            result = juce::File(text.upToFirstOccurrenceOf("\n", false, false).trim());
            juce::PluginDescription description;
            if (auto xml = juce::parseXML(text.fromFirstOccurrenceOf("\n", false, false))) description.loadFromXml(*xml);
            juce::String error;
            instance = formats().createPluginInstance(description, 48000.0, 4096, error);
            if (instance == nullptr) finish("no\tit could not be loaded (" + error + ")");
            instance->setRateAndBufferSizeDetails(48000.0, 4096);
            instance->prepareToPlay(48000.0, 4096);
            auto* editor = instance->createEditorIfNeeded();
            if (editor == nullptr) finish("ok\tno window of its own");
            window = std::make_unique<juce::DocumentWindow>(description.name, juce::Colours::darkgrey, juce::DocumentWindow::closeButton);
            window->setUsingNativeTitleBar(true);
            window->setContentNonOwned(editor, true);
            // Nearly off the screen (its top left corner in the screen's bottom right one): nothing to look at.
            if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
                window->setTopLeftPosition(display->userArea.getRight() - 3, display->userArea.getBottom() - 3);
            window->setVisible(true);
            startTimer(1500);
        }
        void timerCallback() override
        {
            auto* editor = instance != nullptr ? instance->getActiveEditor() : nullptr;
            const int w = editor != nullptr ? editor->getWidth() : 0, h = editor != nullptr ? editor->getHeight() : 0;
            if (w >= 40 && h >= 40) finish("ok\t" + juce::String(w) + " x " + juce::String(h));
            finish("no\tits window came up without a size (" + juce::String(w) + " x " + juce::String(h) + ")");
        }
    };
    // Called in the helper's initialise: the window is up when it returns, the message loop runs it, the timer ends it.
    inline void startEditorTrial(const juce::String& commandLine)
    {
        static auto* trial = new EditorTrial();   // never deleted (the helper ends by _Exit)
        trial->start(commandLine);
    }

    // The verdicts are kept (PluginWindows.txt beside the settings: "ok or no <TAB> key <TAB> why" a line) - a
    // plug-in's window is tried ONCE, not at every start; the key holds the plug-in file's time, so a new version
    // of the plug-in is tried anew.
    struct EditorVerdict { bool known = false, ok = false; juce::String why; };
    inline juce::String editorTrialKey(const juce::PluginDescription& d)
    {
        return d.createIdentifierString() + "|" + juce::String(juce::File(d.fileOrIdentifier).getLastModificationTime().toMilliseconds());
    }
    inline std::map<juce::String, EditorVerdict>& editorVerdicts()
    {
        static std::map<juce::String, EditorVerdict> verdicts;
        static bool loaded = false;
        if (!loaded)
        {
            loaded = true;
            juce::StringArray lines;
            lines.addLines(settingsFolder().getChildFile("PluginWindows.txt").loadFileAsString());
            for (const auto& line : lines)
            {
                const auto parts = juce::StringArray::fromTokens(line, "\t", "");
                if (parts.size() < 2) continue;
                auto& v = verdicts[parts[1]];
                v.known = true; v.ok = parts[0] == "ok"; v.why = parts.size() > 2 ? parts[2] : juce::String();
            }
        }
        return verdicts;
    }
    inline void saveEditorVerdicts()
    {
        juce::String text;
        for (const auto& [key, v] : editorVerdicts())
            if (v.known) text << (v.ok ? "ok" : "no") << "\t" << key << "\t" << v.why.replaceCharacters("\t\r\n", "   ") << "\n";
        settingsFolder().getChildFile("PluginWindows.txt").replaceWithText(text);
    }

    // The program's side: the helper started `trials` times in a row, looked at ten times a second; the first that
    // fails ends it. onDone comes on the message thread (from the timer: delete the runner later, not inside it).
    class EditorTrialRunner final : private juce::Timer
    {
    public:
        std::function<void(bool ok, const juce::String& why)> onDone;
        void run(const juce::PluginDescription& d, int trials) { description = d; left = trials; next(); }
        ~EditorTrialRunner() override { stopTimer(); if (process.isRunning()) process.kill(); request.deleteFile(); result.deleteFile(); }
    private:
        void next()
        {
            const auto stamp = juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
            const auto temp = juce::File::getSpecialLocation(juce::File::tempDirectory);
            request = temp.getChildFile("LibreOrgan-plugineditor-" + stamp + ".request.txt");
            result = temp.getChildFile("LibreOrgan-plugineditor-" + stamp + ".result.txt");
            juce::String xml;
            if (auto x = description.createXml()) xml = x->toString();
            request.replaceWithText(result.getFullPathName() + "\n" + xml);
            juce::StringArray args;
            args.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName());
            args.add("--plugineditor"); args.add(request.getFullPathName());
            startedMs = juce::Time::getMillisecondCounterHiRes();
            if (!process.start(args, 0)) { request.deleteFile(); if (onDone) onDone(false, "the trial could not be started"); return; }
            startTimer(100);
        }
        void timerCallback() override
        {
            const bool finished = !process.isRunning();
            if (!finished && juce::Time::getMillisecondCounterHiRes() - startedMs < 30000.0) return;   // (Waves' shell alone takes ten seconds to load in a fresh process)
            stopTimer();
            juce::String verdict;
            if (!finished) { process.kill(); verdict = "no\tits window did not answer for 30 s"; }
            else
            {
                verdict = result.loadFileAsString().trim();
                if (verdict.isEmpty()) verdict = "no\tits window crashed";
            }
            request.deleteFile(); result.deleteFile();
            const auto detail = verdict.fromFirstOccurrenceOf("\t", false, false).trim();
            if (!verdict.startsWith("ok")) { if (onDone) onDone(false, detail); return; }
            if (--left > 0) { next(); return; }
            if (onDone) onDone(true, detail);
        }
        juce::PluginDescription description;
        juce::ChildProcess process;
        juce::File request, result;
        double startedMs = 0.0;
        int left = 0;
    };

    //==========================================================================
    // What is known: the plug-in types found (JUCE's list, Plugins.xml) and, per plug-in file, when it was scanned
    // and with what result (PluginScan.json) - a file is scanned again only when it changed.
    struct ScanStamp { juce::int64 modified = 0; bool refused = false; juce::String why; int types = 0; };

    struct State
    {
        juce::KnownPluginList known;
        std::map<juce::String, ScanStamp> stamps;   // "<format>|<identifier>"
        bool loaded = false;
    };
    inline State& state() { static State s; return s; }

    inline void save()
    {
        auto& s = state();
        if (auto xml = s.known.createXml()) xml->writeTo(knownListFile());
        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        juce::Array<juce::var> list;
        for (const auto& [key, stamp] : s.stamps)
        {
            juce::DynamicObject::Ptr item = new juce::DynamicObject();
            item->setProperty("file", key);
            item->setProperty("modified", stamp.modified);
            item->setProperty("types", stamp.types);
            if (stamp.refused) item->setProperty("refused", stamp.why);
            list.add(juce::var(item.get()));
        }
        root->setProperty("scanned", list);
        scanStampsFile().replaceWithText(juce::JSON::toString(juce::var(root.get())));
    }
    inline void load()
    {
        auto& s = state();
        if (s.loaded) return;
        s.loaded = true;
        if (auto xml = juce::parseXML(knownListFile())) s.known.recreateFromXml(*xml);
        const auto parsed = juce::JSON::parse(scanStampsFile().loadFileAsString());
        if (const auto* list = parsed.getProperty("scanned", juce::var()).getArray())
            for (const auto& item : *list)
            {
                ScanStamp stamp;
                stamp.modified = (juce::int64)item.getProperty("modified", 0);
                stamp.types = (int)item.getProperty("types", 0);
                stamp.refused = item.hasProperty("refused");
                stamp.why = item.getProperty("refused", "").toString();
                s.stamps[item.getProperty("file", "").toString()] = stamp;
            }
    }
    // The effect plug-ins (instruments are of no use in an effects chain), by maker then name.
    inline juce::Array<juce::PluginDescription> effects()
    {
        load();
        juce::Array<juce::PluginDescription> list;
        for (const auto& type : state().known.getTypes())
            if (!type.isInstrument) list.add(type);
        std::sort(list.begin(), list.end(), [](const juce::PluginDescription& a, const juce::PluginDescription& b)
        {
            const int maker = a.manufacturerName.compareIgnoreCase(b.manufacturerName);
            return maker != 0 ? maker < 0 : a.name.compareIgnoreCase(b.name) < 0;
        });
        return list;
    }
    inline juce::StringArray refusedLines()
    {
        load();
        juce::StringArray lines;
        for (const auto& [key, stamp] : state().stamps)
            if (stamp.refused) lines.add(key.fromFirstOccurrenceOf("|", false, false) + "  -  " + stamp.why);
        return lines;
    }
    // The folders a format looks in by itself (shown in the Configuration Editor as what is searched anyway).
    inline juce::StringArray defaultFolders()
    {
        juce::StringArray folders;
        auto& manager = formats();
        for (int i = 0; i < manager.getNumFormats(); ++i)
            if (auto* format = manager.getFormat(i))
            {
                const auto path = format->getDefaultLocationsToSearch();
                for (int k = 0; k < path.getNumPaths(); ++k) folders.addIfNotAlreadyThere(path[k].getFullPathName());
            }
        return folders;
    }

    //==========================================================================
    // The scan: a thread that lists the plug-in files of every format (the formats' own folders and the user's),
    // starts the worker for each file not scanned yet (or changed, or asked for again) and waits at most
    // `secondsPerFile` for it. What it finds is handed to the message thread at the end of each file.
    class Scanner final : private juce::Thread
    {
    public:
        Scanner() : juce::Thread("Plug-in scan") {}
        ~Scanner() override { stopThread(15000); }
        bool running() const { return isThreadRunning(); }
        juce::String progress() const { const juce::ScopedLock lock(textLock); return text; }
        void cancel() { signalThreadShouldExit(); }
        // rescanAll: every file again, the refused ones too. onlyRefusedWithSeconds > 0: the refused files alone,
        // each given that long (the "try again, longer" of the list).
        void start(const juce::StringArray& userFolders, bool rescanAll, int secondsPerFileIn, bool onlyRefused, std::function<void(juce::String)> finishedIn)
        {
            if (isThreadRunning()) return;
            load();
            folders = userFolders; all = rescanAll; secondsPerFile = juce::jmax(2, secondsPerFileIn); refusedOnly = onlyRefused;
            finished = std::move(finishedIn);
            setText("Looking for plug-in files ...");
            startThread(juce::Thread::Priority::low);
        }
    private:
        void setText(const juce::String& t) { const juce::ScopedLock lock(textLock); text = t; }
        void run() override
        {
            struct Job { juce::String format, identifier; juce::int64 modified; };
            std::vector<Job> jobs;
            int skipped = 0;
            auto& manager = formats();
            for (int i = 0; i < manager.getNumFormats() && !threadShouldExit(); ++i)
            {
                auto* format = manager.getFormat(i);
                if (format == nullptr) continue;
                auto path = format->getDefaultLocationsToSearch();
                for (const auto& folder : folders) if (folder.trim().isNotEmpty()) path.addIfNotAlreadyThere(juce::File(folder.trim()));
                for (const auto& identifier : format->searchPathsForPlugins(path, true, false))
                {
                    const juce::File file(identifier);
                    const juce::int64 modified = juce::File::isAbsolutePath(identifier) && file.exists() ? file.getLastModificationTime().toMilliseconds() : 0;
                    const auto key = format->getName() + "|" + identifier;
                    bool wanted = true;
                    {
                        const juce::MessageManagerLock lock(this);
                        if (!lock.lockWasGained()) return;
                        const auto found = state().stamps.find(key);
                        const bool knownSame = found != state().stamps.end() && found->second.modified == modified;
                        if (refusedOnly) wanted = found != state().stamps.end() && found->second.refused;
                        else if (!all && knownSame) wanted = false;
                    }
                    if (wanted) jobs.push_back({ format->getName(), identifier, modified }); else ++skipped;
                }
            }
            const auto exe = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
            const auto work = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("LibreOrgan-pluginscan-" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));
            work.createDirectory();
            int found = 0, refused = 0, done = 0;
            for (const auto& job : jobs)
            {
                if (threadShouldExit()) break;
                ++done;
                const auto shortName = juce::File::isAbsolutePath(job.identifier) ? juce::File(job.identifier).getFileName() : job.identifier;
                setText("Scanning " + juce::String(done) + " of " + juce::String((int)jobs.size()) + ": " + shortName);
                const auto request = work.getChildFile("request.txt"), result = work.getChildFile("result.xml");
                result.deleteFile();
                request.replaceWithText(job.format + "\n" + job.identifier + "\n" + result.getFullPathName() + "\n");
                juce::ChildProcess worker;
                juce::StringArray args;
                args.add(exe.getFullPathName()); args.add("--pluginscan"); args.add(request.getFullPathName());
                ScanStamp stamp;
                stamp.modified = job.modified;
                juce::Array<juce::PluginDescription> types;
                const double startMs = juce::Time::getMillisecondCounterHiRes();
                if (!worker.start(args, 0)) { stamp.refused = true; stamp.why = "the scan could not be started"; }
                else
                {
                    bool timedOut = false;
                    while (worker.isRunning())
                    {
                        if (threadShouldExit()) { worker.kill(); break; }
                        if (juce::Time::getMillisecondCounterHiRes() - startMs > 1000.0 * secondsPerFile) { timedOut = true; worker.kill(); break; }
                        juce::Thread::sleep(40);
                    }
                    if (threadShouldExit()) break;
                    const auto xml = juce::parseXML(result);
                    if (timedOut) { stamp.refused = true; stamp.why = "did not answer within " + juce::String(secondsPerFile) + " s"; }
                    else if (xml == nullptr) { stamp.refused = true; stamp.why = "its scan crashed or gave nothing"; }
                    else
                        for (auto* child : xml->getChildIterator())
                        {
                            juce::PluginDescription description;
                            if (description.loadFromXml(*child)) types.add(description);
                        }
                }
                const double tookMs = juce::Time::getMillisecondCounterHiRes() - startMs;
                stamp.types = types.size();
                if (stamp.refused) ++refused; else found += types.size();
                const auto key = job.format + "|" + job.identifier;
                juce::MessageManager::callAsync([key, stamp, types, shortName, tookMs]
                {
                    auto& s = state();
                    s.stamps[key] = stamp;
                    for (const auto& type : types) s.known.addType(type);
                    CrashHandler::note("Plug-in scan: " + shortName + " - " + (stamp.refused ? "REFUSED: " + stamp.why
                        : juce::String(types.size()) + " type(s)") + " in " + juce::String(juce::roundToInt(tookMs)) + " ms");
                });
            }
            work.deleteRecursively();
            const juce::String summary = juce::String((int)jobs.size()) + " plug-in file(s) scanned" + (skipped > 0 ? " (" + juce::String(skipped) + " unchanged, not scanned again)" : juce::String())
                + ": " + juce::String(found) + " plug-in(s) found, " + juce::String(refused) + " refused" + (threadShouldExit() ? " - stopped" : "");
            setText(summary);
            auto callback = finished;
            juce::MessageManager::callAsync([summary, callback]
            {
                save();
                CrashHandler::note("Plug-in scan: " + summary);
                if (callback) callback(summary);
            });
        }
        juce::StringArray folders;
        bool all = false, refusedOnly = false;
        int secondsPerFile = 8;
        std::function<void(juce::String)> finished;
        juce::CriticalSection textLock;
        juce::String text;
    };
    inline Scanner& scanner() { static Scanner s; return s; }

    //==========================================================================
    // The safe start. sessionBegan() is called once at the start of the program, before any plug-in is loaded.
    struct Session
    {
        bool safeStart = false;          // every plug-in stays off until the user switches it on
        juce::String diedWhileLoading;   // the plug-in that was being loaded when the last session ended, if any
        bool markerWritten = false;
    };
    inline Session& session() { static Session s; return s; }
    inline void sessionBegan(bool shiftHeld)
    {
        auto& s = session();
        s.diedWhileLoading = loadingMarker().loadFileAsString().trim();
        const bool unclean = sessionMarker().existsAsFile();
        s.safeStart = unclean || shiftHeld || s.diedWhileLoading.isNotEmpty();
        loadingMarker().deleteFile();
        sessionMarker().deleteFile();
        if (s.safeStart)
            CrashHandler::note("Plug-ins: SAFE START - every plug-in stays switched off until it is switched on"
                + juce::String(shiftHeld ? " (Shift held at the start)" : unclean ? " (the last session with plug-ins did not end in order)" : "")
                + (s.diedWhileLoading.isNotEmpty() ? "; the plug-in being loaded when it ended: " + s.diedWhileLoading : juce::String()));
    }
    inline void sessionEndedInOrder() { sessionMarker().deleteFile(); loadingMarker().deleteFile(); }

    // A plug-in loaded into the program (message thread). The markers stand around the load.
    inline std::unique_ptr<juce::AudioPluginInstance> create(const juce::PluginDescription& description, double sampleRate, int blockSize, juce::String& error)
    {
        auto& s = session();
        if (!s.markerWritten) { s.markerWritten = true; sessionMarker().replaceWithText("plug-ins loaded in this session"); }
        loadingMarker().replaceWithText(description.name + " (" + description.manufacturerName + ") - " + description.fileOrIdentifier);
        const double startMs = juce::Time::getMillisecondCounterHiRes();
        auto instance = formats().createPluginInstance(description, sampleRate > 0.0 ? sampleRate : 48000.0, blockSize > 0 ? blockSize : 512, error);
        loadingMarker().deleteFile();
        CrashHandler::note("Plug-in " + description.name + (instance != nullptr ? " loaded in " : " NOT loaded (" + error + ") after ")
            + juce::String(juce::roundToInt(juce::Time::getMillisecondCounterHiRes() - startMs)) + " ms");
        return instance;
    }
}
