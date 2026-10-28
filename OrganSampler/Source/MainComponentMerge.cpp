// Merging a second configuration into the loaded one (right-click Load
// Configuration > Merge another configuration): the added organ's divisions
// land on the divisions of the same number, its folder trees join theirs, its
// stops become further rows of the Stop List under the existing ones, its
// perspectives that are new bring their audio settings, everything that would
// collide with what is loaded (shared folders, perspectives already set up,
// MIDI of divisions that have stops, couplers, combinations, crescendo,
// temperament, the global settings) is left as it is and reported.
#include "MainComponent.h"
#include "CrashHandler.h"
#include "MemoryInfo.h"
#include "SampleFiles.h"
#include "ArchiveExtract.h"

#if JUCE_WINDOWS
 #include <windows.h>
 #define PSAPI_VERSION 2   // GetProcessMemoryInfo from kernel32 (K32...), no psapi.lib
 #include <psapi.h>
#elif JUCE_MAC
 #include <mach/mach.h>
 #include <mach/mach_host.h>
 #include <sys/resource.h>
 #include <unistd.h>
#elif JUCE_LINUX
 #include <unistd.h>
#endif

#include <set>

namespace
{
    juce::String slashesOf(const juce::String& path) { return path.replaceCharacter('\\', '/'); }
    juce::String pathKey(const juce::String& path) { return slashesOf(path).trimCharactersAtEnd("/").toLowerCase(); }

    // The files of a sample folder tree: the bytes of every WAV below it (the
    // samples are held in RAM as 16-bit, so a 24-bit set takes about two thirds
    // of this; loops and copies of filled pipes add a little).
    juce::int64 wavBytesUnder(const juce::File& folder)
    {
        return SampleFiles::totalBytes(folder, { ".wav", ".flac" }, true);   // disk or archive (0.8.157)
    }
}

// The figures live in MemoryInfo.h (shared with the engine's loading stop).
juce::int64 MainComponent::availablePhysicalMemoryBytes() { return MemoryInfo::freeBytes(); }
juce::int64 MainComponent::processMemoryBytes() { return MemoryInfo::heldBytes(); }

void MainComponent::warnIfMemoryShortForTrees(std::vector<juce::String> trees, const juce::String& configurationName)
{
    // Measured before the load starts (the free memory is what the samples get);
    // the WAV bytes are summed on a thread of their own - a big organ has tens
    // of thousands of files - and the verdict comes back to the message thread.
    const juce::int64 installed = (juce::int64)juce::SystemStats::getMemorySizeInMegabytes() * 1024 * 1024;
    const bool asFloat = sampleStorageFloat, resample = resampleOnLoad;
    const double storageRate = storageSampleRate;
    juce::Component::SafePointer<MainComponent> safeThis(this);
    SamplerEngine* eng = &engine;   // outlives this thread (the engine waits for its own frees in its destructor)
    juce::Thread::launch([trees, configurationName, installed, asFloat, resample, storageRate, safeThis, eng]
    {
        // The free memory once the previous organ is freed (0.8.184: its stops are
        // freed on a thread of their own now, so the figure read before was short).
        eng->waitForRemovedStopsFreed();
        const juce::int64 available = availablePhysicalMemoryBytes();
        juce::int64 bytes = 0;
        std::vector<juce::String> done;
        // What the files turn into in memory: 16-bit storage keeps 16-bit files
        // as they are and 24-bit ones at two thirds (float storage: 32 bits a
        // sample); resampling to 48 kHz scales by the rate; the filled pipes'
        // copies add a share. Read from the headers of the first files met
        // (a 16-bit 44.1 kHz set held 6.9 GB for 5.3 GB of files, #19 - the
        // flat "two thirds" said 3.7).
        double bitsSum = 0.0, rateSum = 0.0; int headers = 0;
        for (const auto& tree : trees)
        {
            if (tree.isEmpty()) continue;
            bool below = false;   // a tree inside another counted tree: counted already
            for (const auto& other : done) if (tree == other || juce::File(tree).isAChildOf(juce::File(other))) below = true;
            if (below) continue;
            const juce::File folder(tree);
            if (!SampleFiles::isDirectory(folder)) continue;
            bytes += wavBytesUnder(folder);
            done.push_back(tree);
            if (headers < 48 && folder.isDirectory())   // the headers from the disk alone (an archive tree: the usual 24 bit / 48 kHz)
                for (const auto& entry : juce::RangedDirectoryIterator(folder, true, "*.wav;*.WAV", juce::File::findFiles))
                {
                    juce::FileInputStream in(entry.getFile());
                    char header[36]{};
                    if (!in.openedOk() || in.read(header, 36) < 36 || std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0) continue;
                    const int rate = (int)juce::ByteOrder::littleEndianInt(header + 24), bits = (int)juce::ByteOrder::littleEndianShort(header + 34);
                    if (rate <= 0 || bits <= 0) continue;   // a format chunk not first: left out
                    bitsSum += bits; rateSum += rate; ++headers;
                    if (headers >= 48) break;
                }
        }
        const double bits = headers > 0 ? bitsSum / headers : 24.0, rate = headers > 0 ? rateSum / headers : 48000.0;
        double factor = asFloat ? 32.0 / bits : 16.0 / bits;
        if (resample && rate > 1000.0) factor *= storageRate / rate;   // the storage rate (48, 36 or 32 kHz) against the files' rate
        factor *= 1.15;   // the copies of filled pipes, loops, the program itself
        const juce::int64 needed = (juce::int64)((double)bytes * factor);
        juce::MessageManager::callAsync([safeThis, bytes, needed, available, installed, configurationName]
        {
            if (safeThis == nullptr) return;
            const auto gb = [](juce::int64 b) { return juce::String((double)b / (1024.0 * 1024.0 * 1024.0), 1) + " GB"; };
            const bool tight = needed > available - (juce::int64)512 * 1024 * 1024;
            CrashHandler::note("Memory for \"" + configurationName + "\": " + gb(bytes) + " of WAV files, about " + gb(needed) + " in memory, "
                + gb(available) + " free of " + gb(installed) + " installed" + (tight ? " - TOO LITTLE: the system will page samples out" : ""));
            // No message box (0.8.146, the user's rule): the estimate cannot know
            // the packing, the half-rate releases or the storage rate's effect for
            // sure, and the loading stops itself when the memory runs short - that
            // message, naming the stops left out, is the one that counts.
        });
    });
}

bool MainComponent::readCpuClocks(double& processMs, double& systemBusyMs, double& systemTotalMs)
{
   #if JUCE_WINDOWS
    FILETIME creation{}, exit{}, kernel{}, user{};
    FILETIME idle{}, sysKernel{}, sysUser{};
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user) || !GetSystemTimes(&idle, &sysKernel, &sysUser)) return false;
    auto toMs = [](const FILETIME& t) { return (double)(((juce::uint64)t.dwHighDateTime << 32) | t.dwLowDateTime) / 10000.0; };
    processMs = toMs(kernel) + toMs(user);
    systemTotalMs = toMs(sysKernel) + toMs(sysUser);   // the kernel time includes the idle time
    systemBusyMs = systemTotalMs - toMs(idle);
    return true;
   #elif JUCE_LINUX
    const auto self = juce::File("/proc/self/stat").loadFileAsString();
    const auto afterName = self.fromLastOccurrenceOf(")", false, false);
    const auto fields = juce::StringArray::fromTokens(afterName, " ", "");
    if (fields.size() < 13) return false;
    const double ticks = (double)sysconf(_SC_CLK_TCK);
    processMs = (fields[11].getDoubleValue() + fields[12].getDoubleValue()) * 1000.0 / ticks;   // utime + stime
    const auto stat = juce::File("/proc/stat").loadFileAsString();
    const auto cpu = juce::StringArray::fromTokens(stat.upToFirstOccurrenceOf("\n", false, false), " ", "");
    if (cpu.size() < 5 || cpu[0] != "cpu") return false;
    double total = 0.0;
    for (int i = 1; i < cpu.size(); ++i) total += cpu[i].getDoubleValue();
    const double idle = cpu[4].getDoubleValue() + (cpu.size() > 5 ? cpu[5].getDoubleValue() : 0.0);
    systemTotalMs = total * 1000.0 / ticks;
    systemBusyMs = (total - idle) * 1000.0 / ticks;
    return true;
   #elif JUCE_MAC
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0) return false;
    processMs = (double)usage.ru_utime.tv_sec * 1000.0 + (double)usage.ru_utime.tv_usec / 1000.0
              + (double)usage.ru_stime.tv_sec * 1000.0 + (double)usage.ru_stime.tv_usec / 1000.0;
    host_cpu_load_info_data_t load{};
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&load, &count) != KERN_SUCCESS) return false;
    // Ticks of all CPUs together; the share is what matters, the tick length cancels out.
    const double ticks = (double)sysconf(_SC_CLK_TCK);
    const double user = (double)load.cpu_ticks[CPU_STATE_USER], system = (double)load.cpu_ticks[CPU_STATE_SYSTEM];
    const double nice = (double)load.cpu_ticks[CPU_STATE_NICE], idle = (double)load.cpu_ticks[CPU_STATE_IDLE];
    systemTotalMs = (user + system + nice + idle) * 1000.0 / ticks;
    systemBusyMs = (user + system + nice) * 1000.0 / ticks;
    return true;
   #else
    juce::ignoreUnused(processMs, systemBusyMs, systemTotalMs);
    return false;
   #endif
}

juce::String MainComponent::powerSettingDescription()
{
   #if JUCE_LINUX
    const auto governor = juce::File("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor").loadFileAsString().trim();
    if (governor.isEmpty()) return {};
    return "CPU governor: " + governor + (governor == "performance" ? " (good)."
        : " - 'performance' keeps the clock up for the audio thread (sudo cpupower frequency-set -g performance, or the system's power settings).");
   #elif JUCE_WINDOWS
    juce::ChildProcess process;
    if (!process.start("powercfg /getactivescheme")) return {};
    const auto output = process.readAllProcessOutput().trim();
    const auto name = output.fromFirstOccurrenceOf("(", false, false).upToLastOccurrenceOf(")", false, false);
    if (name.isEmpty()) return {};
    return "Windows power plan: " + name + (name.containsIgnoreCase("performance") || name.containsIgnoreCase("výkon") ? "." : " - a High performance plan keeps the clock up for the audio thread.");
   #else
    return {};
   #endif
}

void MainComponent::mergeConfigurationDialog()
{
    const auto start = configurationsDirectory();   // 0.8.190: the one folder of the configurations
    juce::Component::SafePointer<MainComponent> safeThis(this);
    configurationFileChooser = std::make_unique<juce::FileChooser>(
        "Merge another configuration into the loaded one (OrganSampler .json, a Hauptwerk organ definition, or RAR archives holding one)", start, "*.json;*.Organ_Hauptwerk_xml;*.xml;*.rar");
    configurationFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems,
        [safeThis](const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr) return;
            // RAR archives (0.8.159): what is inside is merged, archive after archive if need be.
            std::vector<juce::File> archives;
            for (const auto& file : chooser.getResults()) if (file.existsAsFile() && ArchiveExtractor::looksLikeArchive(file)) archives.push_back(file);
            if (!archives.empty()) { safeThis->loadConfigurationFromArchives(archives, true); return; }
            safeThis->mergeConfigurationFile(chooser.getResult());
        });
}

void MainComponent::mergeConfigurationFile(const juce::File& file)
{
    if (!SampleFiles::existsAsFile(file)) return;   // on the disk or inside a RAR archive (0.8.159)
    if (HauptwerkOdf::looksLikeOdf(file)) { importHauptwerkOdf(file, true); return; }
    juce::String text;
    if (file.existsAsFile()) text = file.loadFileAsString();
    else if (auto in = SampleFiles::openInput(file)) text = in->readEntireStreamAsString();
    auto configuration = juce::JSON::parse(text);
    if (!configuration.isObject() || !configuration.getProperty("manualKeyboards", juce::var()).isArray())
    {
        queueNotice(juce::MessageBoxIconType::WarningIcon,
            "Merge configuration", "The selected file is not a valid OrganSampler configuration.");
        return;
    }
    mergeConfiguration(configuration, file, file.getFileNameWithoutExtension(), {});
}

void MainComponent::mergeConfiguration(juce::var added, const juce::File& addedFile, const juce::String& addedName, std::function<void()> afterLoad)
{
    if (activeFolderLoader != nullptr)
    {
        queueNotice(juce::MessageBoxIconType::InfoIcon, "Merge configuration", "Please wait until the folders have finished loading.", "OK", this);
        return;
    }
    auto* addedRoot = added.getDynamicObject();
    if (addedRoot == nullptr) return;
    saveSettings();
    auto base = juce::JSON::parse(settingsFile.loadFileAsString());
    auto* baseRoot = base.getDynamicObject();
    if (baseRoot == nullptr) return;
    juce::StringArray done, ignored;   // the report

    //--------------------------------------------------------------------------
    // The added configuration's trees, found as Load Configuration finds them
    // (relative to its library root or to the file; absolute paths as they are).
    auto resolveTree = [&](const juce::String& relative, const juce::String& absolute) -> juce::File
    {
        juce::Array<juce::File> candidates;
        const auto rootName = added.getProperty("sampleLibraryRootName", "").toString();
        const auto rootRelative = slashesOf(added.getProperty("sampleLibraryRootRelative", "").toString());
        const auto dir = addedFile.getParentDirectory();
        if (relative.isNotEmpty())
        {
            if (rootRelative.isNotEmpty()) candidates.add(dir.getChildFile(rootRelative).getChildFile(relative));
            if (rootName.isNotEmpty()) candidates.add(dir.getChildFile(rootName).getChildFile(relative));
            candidates.add(dir.getChildFile(relative));
        }
        if (absolute.isNotEmpty() && juce::File::isAbsolutePath(absolute)) candidates.add(juce::File(absolute));
        for (const auto& candidate : candidates) if (SampleFiles::isDirectory(candidate)) return candidate;   // on the disk or inside a RAR archive (0.8.160)
        return {};
    };
    // The added pool (its stop building tables index it), resolved entry by entry.
    std::vector<juce::File> addedPool;
    if (auto* pool = added.getProperty("sampleTrees", juce::var()).getArray())
        for (const auto& entry : *pool)
        {
            const auto text = entry.toString();
            addedPool.push_back(juce::File::isAbsolutePath(text) && !(text.length() > 1 && text[1] == ':' && juce::File::getSeparatorChar() == '/')
                ? (SampleFiles::isDirectory(juce::File(text)) ? juce::File(text) : resolveTree(text.fromLastOccurrenceOf("/", false, false), text))
                : resolveTree(text, {}));
        }
    // The merged pool: the base's, then every new tree of the added one.
    std::vector<juce::String> mergedPool;
    if (auto* pool = base.getProperty("sampleTrees", juce::var()).getArray())
        for (const auto& entry : *pool) if (entry.toString().isNotEmpty()) mergedPool.push_back(entry.toString());
    auto poolIndexOfPath = [&](const juce::File& tree) -> int
    {
        for (size_t i = 0; i < mergedPool.size(); ++i) if (pathKey(mergedPool[i]) == pathKey(tree.getFullPathName())) return (int)i;
        return -1;
    };
    auto poolIndexOrAdd = [&](const juce::File& tree) -> int
    {
        const int present = poolIndexOfPath(tree);
        if (present >= 0) return present;
        mergedPool.push_back(tree.getFullPathName());
        return (int)mergedPool.size() - 1;
    };

    auto addedManuals = added.getProperty("manualKeyboards", juce::var());
    auto baseManuals = base.getProperty("manualKeyboards", juce::var());
    auto* addedArray = addedManuals.getArray();
    auto* baseArray = baseManuals.getArray();
    if (addedArray == nullptr || baseArray == nullptr) return;
    const int baseCount = (int)base.getProperty("manualCount", manualCount), addedCount = (int)added.getProperty("manualCount", 1);
    int mergedCount = juce::jmax(baseCount, addedCount);
    // The settings hold the divisions in use only: the added organ's further
    // divisions get fresh, empty entries (filled from the added ones below).
    while (baseArray->size() < juce::jmin(SamplerEngine::maxManuals, addedArray->size()))
        baseArray->add(juce::var(new juce::DynamicObject()));

    // What the stops of the added configuration need in memory, before anything
    // is touched: the WAV bytes of its new trees against the memory free now.
    juce::int64 addedBytes = 0;
    std::vector<juce::File> newTrees;
    for (int manual = 0; manual < juce::jmin(SamplerEngine::maxManuals, addedArray->size()); ++manual)
    {
        const auto item = addedArray->getReference(manual);
        juce::Array<juce::File> trees;
        const auto main = resolveTree(item.getProperty("folderRelativePath", "").toString(), item.getProperty("folderPath", "").toString());
        if (main != juce::File()) trees.add(main);
        if (auto* extras = item.getProperty("extraFolderRelativePaths", juce::var()).getArray())
            for (const auto& extra : *extras) { const auto f = resolveTree(extra.toString(), {}); if (f != juce::File()) trees.addIfNotAlreadyThere(f); }
        if (auto* extras = item.getProperty("extraFolderPaths", juce::var()).getArray())
            for (const auto& extra : *extras) { const auto f = resolveTree({}, extra.toString()); if (f != juce::File()) trees.addIfNotAlreadyThere(f); }
        for (const auto& tree : trees)
            if (poolIndexOfPath(tree) < 0 && std::find(newTrees.begin(), newTrees.end(), tree) == newTrees.end()) newTrees.push_back(tree);
    }
    for (const auto& tree : newTrees) addedBytes += wavBytesUnder(tree);
    const juce::int64 available = availablePhysicalMemoryBytes();
    const juce::int64 needed = (juce::int64)((double)addedBytes * 0.7);   // 16-bit storage of 24-bit files, plus the copies of filled pipes
    const auto gb = [](juce::int64 bytes) { return juce::String((double)bytes / (1024.0 * 1024.0 * 1024.0), 2) + " GB"; };
    CrashHandler::note("Merge configuration: " + addedName + " - " + juce::String((int)newTrees.size()) + " new tree(s), " + gb(addedBytes) + " of WAV files (about "
        + gb(needed) + " in memory), " + gb(available) + " free");
    if (needed > available - (juce::int64)512 * 1024 * 1024)
    {
        queueNotice(juce::MessageBoxIconType::WarningIcon, "Merge configuration",
            "Not enough memory for the added organ: its new sample folders hold " + gb(addedBytes) + " of WAV files (about " + gb(needed)
            + " in memory), and only " + gb(available) + " is free. Nothing was changed. Close other programs, or add fewer folders (a configuration with fewer divisions).",
            "OK", this);
        CrashHandler::note("Merge configuration refused: not enough memory");
        return;
    }

    //--------------------------------------------------------------------------
    // Division by division: trees, the Stop List rows, the perspectives' audio, MIDI.
    const bool addedMidi = (bool)added.getProperty("containsMidiMappings", false);
    int stopsAdded = 0, stopsShared = 0, stopsRenamed = 0, perspectivesAdded = 0, perspectivesKept = 0, stopsJoined = 0;
    std::vector<juce::String> learnedPatterns;   // 0.8.220: "Label;prefix" of a perspective the added stops brought
    juce::StringArray joinedPerspectiveNames;
    std::set<juce::String> keptStopKeys;   // "<manual>|<comparable name>" of the added stops that made it (for the voicing and the grid)
    std::map<juce::String, juce::String> sharedStopNames;   // "<manual>|<comparable added name>" -> the base row's name playing the same folder (0.8.161: the same organ in volumes)
    std::map<int, std::set<juce::String>> baseShown;   // per division: comparable names of the base's own grid rows (not the children of stop folders)
    std::map<juce::String, juce::String> renamed;   // "<manual>|<old name>" -> new name
    juce::StringArray addedPerspectiveNames;
    for (int manual = 0; manual < juce::jmin(SamplerEngine::maxManuals, addedArray->size(), baseArray->size()); ++manual)
    {
        auto addedItem = addedArray->getReference(manual);
        auto* baseItem = baseArray->getReference(manual).getDynamicObject();
        if (baseItem == nullptr) continue;
        // The added division's trees (resolved) and its stop table, derived by
        // the automatic rules when it has none.
        juce::Array<juce::File> addedTrees;
        {
            const auto main = resolveTree(addedItem.getProperty("folderRelativePath", "").toString(), addedItem.getProperty("folderPath", "").toString());
            if (main != juce::File()) addedTrees.add(main);
            if (auto* extras = addedItem.getProperty("extraFolderRelativePaths", juce::var()).getArray())
                for (const auto& extra : *extras) { const auto f = resolveTree(extra.toString(), {}); if (f != juce::File()) addedTrees.addIfNotAlreadyThere(f); }
            if (auto* extras = addedItem.getProperty("extraFolderPaths", juce::var()).getArray())
                for (const auto& extra : *extras) { const auto f = resolveTree({}, extra.toString()); if (f != juce::File()) addedTrees.addIfNotAlreadyThere(f); }
        }
        if (addedTrees.isEmpty()) continue;   // nothing to add to this division
        static const juce::String divisionNames[8]{ "P.", "I.", "II.", "III.", "IV.", "V.", "VI.", "VII." };
        const juce::String divisionName = divisionNames[juce::jlimit(0, 7, manual)];
        // Base trees and whether the base division has anything.
        juce::Array<juce::File> baseTrees;
        {
            const auto main = baseItem->getProperty("folderPath").toString();
            if (main.isNotEmpty()) baseTrees.add(juce::File(main));
            if (auto* extras = baseItem->getProperty("extraFolderPaths").getArray())
                for (const auto& extra : *extras) if (extra.toString().isNotEmpty()) baseTrees.addIfNotAlreadyThere(juce::File(extra.toString()));
        }
        const bool baseEmpty = baseTrees.isEmpty();
        // The base stop table, explicit (the automatic rules written down) so the
        // added rows can follow it and shared folders can be seen.
        ManualConfig::StopBuild baseBuild = stopBuildFromVar(baseItem->getProperty("stopBuild"));
        if (!baseBuild.active && !baseEmpty) { baseBuild = deriveStopBuildFromEngine(manual); baseBuild.active = true; }
        if (baseEmpty) { baseBuild = {}; baseBuild.active = true; }
        for (const auto& existing : baseBuild.entries)
            if (!existing.child) baseShown[manual].insert(stopBuildKey(existing.name));
        // The added stop table: its own (tree indexes remapped), or the automatic rules on its trees.
        ManualConfig::StopBuild addedBuild = stopBuildFromVar(addedItem.getProperty("stopBuild", juce::var()));
        std::vector<ManualConfig::StopBuild::Entry> addedEntries;   // with tree = merged pool index
        if (addedBuild.active)
        {
            for (auto entry : addedBuild.entries)
            {
                std::map<juce::String, ManualConfig::StopBuild::FolderRef> folders;
                for (const auto& [perspective, ref] : entry.folders)
                {
                    if (ref.tree < 0 || ref.tree >= (int)addedPool.size() || addedPool[(size_t)ref.tree] == juce::File()) continue;
                    folders[perspective] = { poolIndexOrAdd(addedPool[(size_t)ref.tree]), ref.relativePath };
                }
                if (folders.empty() && !entry.folders.empty()) continue;   // its trees are not on this computer
                entry.folders = folders;
                addedEntries.push_back(entry);
            }
        }
        else
        {
            for (const auto& tree : addedTrees)
            {
                const int treeIndex = poolIndexOrAdd(tree);
                for (const auto& assignment : engine.scanTreeAssignments(tree, false))
                {
                    const juce::String perspective = assignment.perspective.isEmpty() ? juce::String("default") : assignment.perspective;
                    ManualConfig::StopBuild::Entry* target = nullptr;
                    if (assignment.grouped)
                        for (auto& existing : addedEntries)
                            if (stopBuildKey(existing.name) == stopBuildKey(assignment.stopName)) { target = &existing; break; }
                    if (target == nullptr)
                    {
                        ManualConfig::StopBuild::Entry entry;
                        entry.name = assignment.stopName;
                        addedEntries.push_back(entry);
                        target = &addedEntries.back();
                    }
                    if (target->folders.count(perspective) == 0)
                        target->folders[perspective] = { treeIndex, assignment.folder == tree ? juce::String(".") : slashesOf(assignment.folder.getRelativePathFrom(tree)) };
                }
            }
        }
        // Folders the base already plays: a stop of the added configuration that
        // uses one is shared and not added (its sound is already there).
        std::map<juce::String, juce::String> baseFolders;   // "tree|path" -> the base row's name
        for (const auto& entry : baseBuild.entries)
            for (const auto& [perspective, ref] : entry.folders)
                baseFolders.emplace(juce::String(ref.tree) + "|" + pathKey(ref.relativePath), entry.name);
        // 0.8.220: the added stops as ANOTHER PERSPECTIVE of the base rows (Bergen op Zoom: the
        // "Wet - Distant" definition merged into the Surround one doubled every stop; its folders
        // "06-far__HW_Octava2" are the base rows' "03-ortf_HW_Octava2" with another prefix). The
        // name rule of matchFoldersAsPerspective, and for a definition the Hauptwerk stop names
        // must agree as well: a foreign organ with the same folder stems is still foreign.
        std::map<juce::String, size_t> perspectiveRow;   // lower-case added folder name -> base entry index
        juce::String perspectiveHead;
        juce::StringArray divisionJoinedLabels;   // the perspective(s) the joined folders brought to this division
        {
            std::vector<juce::String> names;
            std::map<juce::String, const ManualConfig::StopBuild::Entry*> entryOfName;
            for (const auto& entry : addedEntries)
            {
                bool shared = false;
                for (const auto& [perspective, ref] : entry.folders)
                    if (baseFolders.count(juce::String(ref.tree) + "|" + pathKey(ref.relativePath)) != 0) shared = true;
                if (shared) continue;
                for (const auto& [perspective, ref] : entry.folders)
                {
                    const auto last = slashesOf(ref.relativePath).fromLastOccurrenceOf("/", false, false);
                    if (last.isEmpty() || last == ".") continue;
                    names.push_back(last);
                    entryOfName[last.toLowerCase()] = &entry;
                }
            }
            if (!names.empty())
            {
                std::vector<juce::String> basePatterns;
                if (auto* list = base.getProperty("perspectivePatterns", juce::var()).getArray()) for (const auto& item : *list) basePatterns.push_back(item.toString());
                auto match = matchFoldersAsPerspective(names, baseBuild.entries, basePatterns);
                if (match.matched > 0)
                {
                    // The stop names (aliases of an import) must agree where both sides have them.
                    int withNames = 0, agree = 0;
                    for (const auto& [name, row] : match.rowOfFolder)
                    {
                        const auto added = entryOfName.find(name);
                        if (added == entryOfName.end() || row >= baseBuild.entries.size()) continue;
                        const auto& a = added->second->alias, & b = baseBuild.entries[row].alias;
                        if (a.isEmpty() || b.isEmpty()) continue;
                        ++withNames;
                        if (stopBuildKey(a) == stopBuildKey(b)) ++agree;
                    }
                    if (withNames >= 3 && agree * 5 < withNames * 4)
                    {
                        match.matched = 0;
                        match.why = "the folder names repeat the division's after the prefix " + match.head + ", but the stop names agree on " + juce::String(agree) + " of " + juce::String(withNames) + " only - another organ";
                    }
                }
                if (match.matched > 0) { perspectiveRow = match.rowOfFolder; perspectiveHead = match.head; }
                CrashHandler::note("Merge configuration: " + divisionName + " " + (match.matched > 0
                    ? "the added stops are another perspective of the loaded ones: the prefix " + match.head + " on " + juce::String(match.matched) + " of " + juce::String(match.candidates) + " folder(s)"
                    : "the added stops are not a perspective of the loaded ones (" + match.why + ")"));
            }
        }
        for (auto entry : addedEntries)
        {
            bool shared = false;
            for (const auto& [perspective, ref] : entry.folders)
                if (const auto found = baseFolders.find(juce::String(ref.tree) + "|" + pathKey(ref.relativePath)); found != baseFolders.end())
                { shared = true; sharedStopNames[juce::String(manual) + "|" + stopBuildKey(entry.name)] = found->second; }
            if (shared) { ++stopsShared; ignored.add(divisionName + " " + entry.name + ": its sample folder is already loaded (shared) - not added"); continue; }
            // Another perspective of a base row: its folders join that row, no new stop.
            if (!perspectiveRow.empty())
            {
                size_t row = baseBuild.entries.size();
                for (const auto& [perspective, ref] : entry.folders)
                {
                    const auto found = perspectiveRow.find(slashesOf(ref.relativePath).fromLastOccurrenceOf("/", false, false).toLowerCase());
                    if (found == perspectiveRow.end() || (row < baseBuild.entries.size() && found->second != row)) { row = baseBuild.entries.size(); break; }
                    row = found->second;
                }
                if (row < baseBuild.entries.size())
                {
                    auto& target = baseBuild.entries[row];
                    bool clash = false;
                    for (const auto& [perspective, ref] : entry.folders)
                    {
                        const juce::String label = perspective.isEmpty() || perspective == "default" ? perspectiveLabelForHead(perspectiveHead, {}) : perspective;
                        if (target.folders.count(label) != 0) clash = true;
                    }
                    if (!clash)
                    {
                        for (const auto& [perspective, ref] : entry.folders)
                        {
                            const juce::String label = perspective.isEmpty() || perspective == "default" ? perspectiveLabelForHead(perspectiveHead, {}) : perspective;
                            target.folders[label] = ref;
                            bool known = false;
                            for (const auto& p : baseBuild.perspectives) if (p.equalsIgnoreCase(label)) known = true;
                            if (!known) baseBuild.perspectives.push_back(label);
                            learnedPatterns.push_back(label + ";" + perspectiveHead);
                            joinedPerspectiveNames.addIfNotAlreadyThere(label);
                            divisionJoinedLabels.addIfNotAlreadyThere(label);
                        }
                        sharedStopNames[juce::String(manual) + "|" + stopBuildKey(entry.name)] = target.name;
                        ++stopsJoined;
                        continue;
                    }
                }
            }
            const juce::String originalName = entry.name;
            for (int suffix = 2; ; ++suffix)
            {
                bool taken = false;
                for (const auto& existing : baseBuild.entries)
                    if (stopBuildKey(existing.name) == stopBuildKey(entry.name)
                        || (entry.alias.isNotEmpty() && stopBuildKey(existing.alias) == stopBuildKey(entry.alias))) taken = true;   // 0.8.220: the caption (alias) too - the user sees that one
                if (!taken) break;
                entry.name = originalName + " (" + juce::String(suffix) + ")";
                if (entry.alias.isNotEmpty() && !entry.alias.endsWith(")")) entry.alias = entry.alias + " (" + juce::String(suffix) + ")";
                entry.customName = true;
            }
            if (entry.name != originalName) { ++stopsRenamed; renamed[juce::String(manual) + "|" + originalName] = entry.name; done.add(divisionName + " " + originalName + " added as \"" + entry.name + "\" (the name was taken)"); }
            keptStopKeys.insert(juce::String(manual) + "|" + stopBuildKey(originalName));
            for (const auto& [perspective, ref] : entry.folders)
            {
                bool known = false;
                for (const auto& p : baseBuild.perspectives) if (p.equalsIgnoreCase(perspective)) known = true;
                if (!known) baseBuild.perspectives.push_back(perspective);
            }
            baseBuild.entries.push_back(entry);
            ++stopsAdded;
        }
        // The trees: the base's, then the added ones (new ones only).
        juce::Array<juce::var> extras;
        juce::String mainTree = baseItem->getProperty("folderPath").toString();
        if (auto* existing = baseItem->getProperty("extraFolderPaths").getArray()) for (const auto& extra : *existing) extras.add(extra);
        for (const auto& tree : addedTrees)
        {
            bool present = pathKey(mainTree) == pathKey(tree.getFullPathName());
            for (const auto& extra : extras) if (pathKey(extra.toString()) == pathKey(tree.getFullPathName())) present = true;
            if (present) continue;
            if (mainTree.isEmpty()) mainTree = tree.getFullPathName(); else extras.add(tree.getFullPathName());
            poolIndexOrAdd(tree);
        }
        baseItem->setProperty("folderPath", mainTree);
        baseItem->setProperty("folderRelativePath", juce::String());
        baseItem->setProperty("extraFolderPaths", extras);
        baseItem->setProperty("extraFolderRelativePaths", juce::Array<juce::var>());
        baseItem->setProperty("stopBuild", stopBuildToVar(baseBuild));
        if (baseEmpty)
        {
            // An empty division takes the added division's compass, volume, wind, tremulant and swell settings.
            for (const auto& key : { "compassLow", "compassHigh", "divisionVolumeDb", "allowRandomTuningError", "allowPipeCoupling", "tremulantEnabled", "tremulantRateHz",
                "tremulantDepthCents", "tremulantRankDriftDegrees", "windDepthCents", "windFrequencyHz", "windOscillations", "windOscillationStrength", "windSmoothingMs",
                "windTargetMask", "bassOctaveLayout", "midiTranspose", "disableVelocity", "expressionMaxAttenuationDb", "expressionInertiaMs" })
                if (addedItem.hasProperty(key)) baseItem->setProperty(key, addedItem.getProperty(key, juce::var()));
            if (addedMidi)
                for (const auto& key : { "deviceIdentifier", "channel", "expressionDeviceIdentifier", "expressionChannel", "expressionController", "expressionMinimum", "expressionMaximum" })
                    if (addedItem.hasProperty(key)) baseItem->setProperty(key, addedItem.getProperty(key, juce::var()));
            done.add(divisionName + ": division was empty - the added configuration's keys, " + (addedMidi ? "MIDI, " : "") + "wind and tremulant settings taken");
        }
        else if (addedMidi) ignored.add(divisionName + ": MIDI settings of the added configuration (this division has its own)");
        // The perspectives' audio: a perspective the base has keeps its settings,
        // a new one brings the added configuration's route (its IR found beside
        // the added file when the path is relative).
        {
            juce::Array<juce::var> routes;
            juce::StringArray baseNames;
            if (auto* existing = baseItem->getProperty("perspectiveAudio").getArray())
                for (const auto& route : *existing) { routes.add(route); baseNames.add(route.getProperty("name", "").toString()); }
            if (auto* addedRoutes = addedItem.getProperty("perspectiveAudio", juce::var()).getArray())
                for (const auto& routeVar : *addedRoutes)
                {
                    const auto name = routeVar.getProperty("name", "").toString();
                    if (name.isEmpty()) continue;
                    bool known = false;
                    for (const auto& b : baseNames) if (b.equalsIgnoreCase(name)) known = true;
                    bool usedByAdded = false;
                    for (const auto& p : baseBuild.perspectives) if (p.equalsIgnoreCase(name)) usedByAdded = true;
                    if (known) { if (usedByAdded && name != "default") { ++perspectivesKept; ignored.add(divisionName + " perspective " + name + ": audio settings of the added configuration (the route exists)"); } continue; }
                    if (!usedByAdded && name != "default" && !MainComponent::isAuxiliaryPerspective(name)) continue;   // a route of a perspective nobody uses
                    auto route = routeVar.clone();
                    if (auto* routeObject = route.getDynamicObject())
                    {
                        // A portable route: no device of this computer; the IR relative to the added file.
                        const auto irRelative = slashesOf(routeObject->getProperty("reverbIrRelative").toString());
                        const auto irAbsolute = routeObject->getProperty("reverbIr").toString();
                        juce::File ir;
                        if (irRelative.isNotEmpty() && addedFile.getParentDirectory().getChildFile(irRelative).existsAsFile()) ir = addedFile.getParentDirectory().getChildFile(irRelative);
                        else if (irAbsolute.isNotEmpty() && juce::File(irAbsolute).existsAsFile()) ir = juce::File(irAbsolute);
                        routeObject->setProperty("reverbIr", ir != juce::File() ? ir.getFullPathName() : juce::String());
                    }
                    routes.add(route);
                    baseNames.add(name);
                    ++perspectivesAdded;
                    addedPerspectiveNames.addIfNotAlreadyThere(name);
                    done.add(divisionName + " perspective " + name + ": route and settings of the added configuration taken (device: this computer's default)");
                }
            // 0.8.220: the perspective the joined folders brought (a single-perspective set has its
            // route under "default" in the added configuration): a route of that name, the added
            // default route's settings when there are any - counted as a new route, which it is.
            for (const auto& label : divisionJoinedLabels)
            {
                bool known = false;
                for (const auto& b : baseNames) if (b.equalsIgnoreCase(label)) known = true;
                if (known) continue;
                juce::var route;
                if (auto* addedRoutes = addedItem.getProperty("perspectiveAudio", juce::var()).getArray())
                    for (const auto& routeVar : *addedRoutes)
                        if (routeVar.getProperty("name", "").toString() == "default") { route = routeVar.clone(); break; }
                if (route.getDynamicObject() == nullptr) route = juce::var(new juce::DynamicObject());
                if (auto* routeObject = route.getDynamicObject())
                {
                    routeObject->setProperty("name", label);
                    routeObject->setProperty("reverbIr", juce::String());
                }
                routes.add(route);
                baseNames.add(label);
                ++perspectivesAdded;
                addedPerspectiveNames.addIfNotAlreadyThere(label);
                done.add(divisionName + " perspective " + label + ": a new route for the perspective the added folders brought (device: this computer's default)");
            }
            baseItem->setProperty("perspectiveAudio", routes);
        }
        if (manual > baseCount && manual <= 7) mergedCount = juce::jmax(mergedCount, manual);
    }
    baseRoot->setProperty("manualCount", juce::jlimit(1, SamplerEngine::maxManuals - 1, mergedCount));
    {
        juce::Array<juce::var> pool;
        for (const auto& tree : mergedPool) pool.add(tree);
        baseRoot->setProperty("sampleTrees", pool);
    }

    //--------------------------------------------------------------------------
    // Patterns: the union, the base's first.
    for (const auto& key : { "perspectivePatterns", "ignoredPatterns", "oneShotPatterns" })
    {
        juce::Array<juce::var> merged;
        if (auto* list = base.getProperty(key, juce::var()).getArray()) for (const auto& item : *list) merged.add(item);
        if (auto* list = added.getProperty(key, juce::var()).getArray())
            for (const auto& item : *list)
            {
                bool present = false;
                for (const auto& existing : merged) if (existing.toString().equalsIgnoreCase(item.toString())) present = true;
                if (!present) merged.add(item);
            }
        if (juce::String(key) == "perspectivePatterns" && !learnedPatterns.empty())
        {
            // 0.8.220: the prefix of the perspective the added stops brought ("Distant;06-far__") joins
            // its label's line, so the loader knows it from now on (a tree of it loaded by hand too).
            std::vector<juce::String> lines;
            for (const auto& item : merged) lines.push_back(item.toString());
            for (const auto& learned : learnedPatterns)
                lines = patternsWithHead(lines, learned.upToFirstOccurrenceOf(";", false, false), learned.fromFirstOccurrenceOf(";", false, false));
            merged.clear();
            for (const auto& line : lines) merged.add(line);
        }
        baseRoot->setProperty(key, merged);
    }

    //--------------------------------------------------------------------------
    // The voicing of the added stops (renamed where they were), their grid cells
    // under the base's rows, their pool entries.
    {
        auto baseVoicing = base.getProperty("voicingSettings", juce::var());
        juce::DynamicObject::Ptr voicing = baseVoicing.getDynamicObject() != nullptr ? baseVoicing.getDynamicObject() : new juce::DynamicObject();
        juce::Array<juce::var> stops;
        if (auto* list = voicing->getProperty("stops").getArray()) for (const auto& item : *list) stops.add(item);
        if (auto* list = added.getProperty("voicingSettings", juce::var()).getProperty("stops", juce::var()).getArray())
            for (const auto& item : *list)
            {
                const int manual = (int)item.getProperty("primaryManual", 0);
                const auto name = item.getProperty("name", "").toString();
                if (keptStopKeys.count(juce::String(manual) + "|" + stopBuildKey(name)) == 0) continue;
                auto entry = item.clone();
                if (auto* object = entry.getDynamicObject())
                {
                    const auto newName = renamed.find(juce::String(manual) + "|" + name);
                    if (newName != renamed.end()) object->setProperty("name", newName->second);
                    // Transmissions into divisions the merged organ does not have are dropped.
                    if (auto* transmissions = object->getProperty("transmissions").getArray())
                    {
                        juce::Array<juce::var> kept;
                        for (const auto& t : *transmissions) if ((int)t.getProperty("manual", 0) <= mergedCount) kept.add(t);
                        object->setProperty("transmissions", kept);
                    }
                }
                stops.add(entry);
            }
        voicing->setProperty("stops", stops);
        // Grid: the added cells shifted below the base's last row of their division.
        juce::DynamicObject::Ptr grid = baseVoicing.getProperty("stopGrid", juce::var()).getDynamicObject() != nullptr
            ? baseVoicing.getProperty("stopGrid", juce::var()).getDynamicObject() : new juce::DynamicObject();
        if (auto* addedGrid = added.getProperty("voicingSettings", juce::var()).getProperty("stopGrid", juce::var()).getDynamicObject())
            for (const auto& division : addedGrid->getProperties())
            {
                const int manual = division.name.toString().getIntValue();
                juce::DynamicObject::Ptr cells = grid->getProperty(division.name).getDynamicObject() != nullptr ? grid->getProperty(division.name).getDynamicObject() : new juce::DynamicObject();
                // The last row the base really shows here: the cells of its own table rows
                // (0.8.154: every stored cell counted before - stale ones of stops long gone,
                // pooled or indented under a folder - and Beziers stood eleven empty rows
                // under Caen, out of sight). The added rows start right under it, their own
                // first row shifted to it.
                int lastRow = -1;
                const auto& shown = baseShown[manual];
                for (const auto& cell : cells->getProperties())
                {
                    const auto parts = juce::StringArray::fromTokens(cell.value.toString(), ",", "");
                    if (parts.size() == 2 && shown.count(stopBuildKey(cell.name.toString())) != 0) lastRow = juce::jmax(lastRow, parts[1].getIntValue());
                }
                int firstAdded = std::numeric_limits<int>::max();
                if (auto* addedCells = division.value.getDynamicObject())
                    for (const auto& cell : addedCells->getProperties())
                    {
                        const auto key = cell.name.toString();
                        if (key.startsWith("tr:") || keptStopKeys.count(juce::String(manual) + "|" + stopBuildKey(key)) == 0) continue;
                        const auto parts = juce::StringArray::fromTokens(cell.value.toString(), ",", "");
                        if (parts.size() == 2) firstAdded = juce::jmin(firstAdded, parts[1].getIntValue());
                    }
                if (firstAdded == std::numeric_limits<int>::max()) firstAdded = 0;
                // The same organ in volumes (0.8.161: a division where the added stops
                // share folders with the base's rows - Adlington Vol. 1 over Vol. 0):
                // the added organ's page is the layout of the whole division, the
                // shared rows take its cells too, nothing is shifted. Another organ
                // (Beziers into Caen) keeps its block right under the last row.
                bool sameOrgan = false;
                for (const auto& [key, baseName] : sharedStopNames) if (key.startsWith(juce::String(manual) + "|")) sameOrgan = true;
                const int shift = sameOrgan ? 0 : lastRow + 1 - firstAdded;
                if (auto* addedCells = division.value.getDynamicObject())
                    for (const auto& cell : addedCells->getProperties())
                    {
                        auto key = cell.name.toString();
                        if (key.startsWith("tr:")) continue;   // a transmission's cell: placed afresh by the grid
                        const auto sharedName = sharedStopNames.find(juce::String(manual) + "|" + stopBuildKey(key));
                        if (sameOrgan && sharedName != sharedStopNames.end()) key = sharedName->second;
                        else if (keptStopKeys.count(juce::String(manual) + "|" + stopBuildKey(key)) == 0) continue;
                        const auto newName = renamed.find(juce::String(manual) + "|" + key);
                        if (newName != renamed.end()) key = newName->second;
                        const auto parts = juce::StringArray::fromTokens(cell.value.toString(), ",", "");
                        if (parts.size() != 2) continue;
                        cells->setProperty(juce::Identifier(key), parts[0] + "," + juce::String(parts[1].getIntValue() + shift));
                    }
                grid->setProperty(division.name, juce::var(cells.get()));
            }
        voicing->setProperty("stopGrid", juce::var(grid.get()));
        baseRoot->setProperty("voicingSettings", juce::var(voicing.get()));
    }

    //--------------------------------------------------------------------------
    // What stays the base's, by design.
    for (const auto& key : { "couplers", "combinationMemory", "crescendo", "selectedTemperament", "baseTemperament", "customTemperaments", "temperamentRotations",
        "masterVolumeDb", "globalPitchCents", "midiButtonBindings", "registrationStops", "registrationCouplers", "registrationTremulants", "registrationMasterDb" })
        if (added.hasProperty(key)) { ignored.add(juce::String(key) + " of the added configuration"); break; }
    ignored.add("couplers, combinations, crescendo, temperament, the global sound settings and the MIDI buttons of the added configuration (the loaded ones stay)");
    baseRoot->removeProperty("portable");   // this computer's settings, as they are
    baseRoot->setProperty("containsMidiMappings", true);
    // Two organs rarely stand at one pitch (a1 = 440, 415, 465 Hz ...): every
    // stop of the merged organ is tuned to equal temperament at 440 Hz
    // (Temperaments > "Equal at 440 Hz"), so they play together in tune.
    // Not for the same organ in volumes (0.8.162: a division with shared stops -
    // Adlington Vol. 1 over Vol. 0 stands at the same a1 = 409 Hz; the retune to
    // 440 Hz moved every pipe by +128 cents and the linear kernel's images of the
    // bright filled pipes came out as "aliasing").
    if (sharedStopNames.empty() && !(bool)baseRoot->getProperty("equalAt440"))
    {
        baseRoot->setProperty("equalAt440", true);
        done.add("every stop tuned to a1 = 440 Hz (Temperaments > Equal at 440 Hz switched on: two organs rarely stand at one pitch)");
    }
    else if (!sharedStopNames.empty()) done.add("the same organ in volumes: its pitch stays as it is (no retune to 440 Hz)");

    const juce::String summary = "Merged \"" + addedName + "\" into " + (currentConfigurationName.isNotEmpty() ? "\"" + currentConfigurationName + "\"" : juce::String("the loaded organ"))
        + ": " + juce::String(stopsAdded) + " stop(s) added" + (stopsShared > 0 ? ", " + juce::String(stopsShared) + " shared (not added)" : juce::String())
        + (stopsRenamed > 0 ? ", " + juce::String(stopsRenamed) + " renamed" : juce::String()) + ", " + juce::String((int)newTrees.size()) + " new folder tree(s) ("
        + gb(addedBytes) + " of WAV files), " + juce::String(perspectivesAdded) + " new perspective route(s)"
        + (stopsJoined > 0 ? ", " + juce::String(stopsJoined) + " stop(s) gained the perspective " + joinedPerspectiveNames.joinIntoString(", ") + " (the added folders joined the loaded stops: another perspective of the same organ)" : juce::String())
        + (perspectivesKept > 0 ? ", " + juce::String(perspectivesKept) + " perspective(s) kept as set up" : juce::String()) + ", divisions now " + juce::String(mergedCount + 1) + ".";
    CrashHandler::note("Merge configuration: " + summary + " | " + done.joinIntoString(" | ") + " | ignored: " + ignored.joinIntoString(" | "));
    if (stopsAdded == 0 && stopsJoined == 0 && newTrees.empty())
    {
        queueNotice(juce::MessageBoxIconType::InfoIcon, "Merge configuration", summary + "\n\nNothing new to add"
            + (ignored.isEmpty() ? juce::String(".") : ":\n- " + ignored.joinIntoString("\n- ")), "OK", this);
        return;
    }
    // The merged organ is loaded as a configuration of this computer is; its
    // file stays the one that was open (the merge is unsaved until Save Configuration).
    const auto file = currentConfigurationFile.existsAsFile() ? currentConfigurationFile
        : settingsFile.getSiblingFile((currentConfigurationName.isNotEmpty() ? currentConfigurationName : juce::String("Merged organ")) + ".json");
    juce::Component::SafePointer<MainComponent> safeThis(this);
    const juce::String report = summary + (done.isEmpty() ? juce::String() : "\n\nDone:\n- " + done.joinIntoString("\n- "))
        + (ignored.isEmpty() ? juce::String() : "\n\nLeft as it was (ignored):\n- " + ignored.joinIntoString("\n- "))
        + "\n\nThe added stops stand under the existing rows of their divisions; drag them where they belong. Save Configuration keeps the merged organ.";
    runWhenLoaded([safeThis, report, afterLoad]
    {
        if (safeThis == nullptr) return;
        safeThis->configurationDirty = true;
        if (afterLoad) afterLoad();
        queueNotice(juce::MessageBoxIconType::InfoIcon, "Merge configuration", report, "OK", safeThis.getComponent());
    });
    applyPortableConfiguration(base, file, {});
    configurationDirty = true;
}
