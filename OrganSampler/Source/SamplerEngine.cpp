#include "SamplerEngine.h"
#include "CrashHandler.h"   // activity log notes (copy builds)
#include "MemoryInfo.h"     // the free memory and the program's footprint (the loading stops when short)
#include "StopNames.h"      // the capital after a division abbreviation in the names the program makes
// Signalsmith Stretch (MIT): pitch shift with the length kept, for the
// transposed copies of filled pipes (ThirdParty/signalsmith).
#if JUCE_MSVC
 #pragma warning(push)
 #pragma warning(disable: 4244 4267 4305 4456 4458 4100 4127 4189 4324)
#endif
#include "signalsmith-stretch.h"
#if JUCE_MSVC
 #pragma warning(pop)
#endif
#include "SampleFiles.h"
#include <chrono>
#include <cmath>
#include <complex>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <type_traits>
#if JUCE_LINUX
 #include <sys/resource.h>
 #include <sys/syscall.h>
 #include <unistd.h>
#elif JUCE_MAC
 #include <pthread.h>
 #include <sched.h>
#endif
#if JUCE_WINDOWS
#include <filesystem>
#include <system_error>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
#include <cerrno>
#endif
#if JUCE_MAC
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <mach/mach_time.h>
#endif

namespace
{
    // Directory listings without a stat per file. JUCE's iterator asks for the
    // size and times of every entry, which an NTFS mount under Linux (FUSE)
    // answers at a millisecond each: 30 000 samples made the Stop List wait 15 s.
    // The entry type comes with the listing (d_type, the find data on Windows).
#if JUCE_WINDOWS
    std::filesystem::path fsPath(const juce::File& file) { return std::filesystem::path(std::wstring(file.getFullPathName().toWideCharPointer())); }
    juce::File fileOf(const std::filesystem::path& path) { return juce::File(juce::String(path.wstring().c_str())); }
    void collectSubdirectories(const std::filesystem::path& dir, std::vector<juce::File>& out)
    {
        std::error_code error;
        for (std::filesystem::directory_iterator it(dir, std::filesystem::directory_options::skip_permission_denied, error), end;
             !error && it != end; it.increment(error))
        {
            std::error_code typeError;
            if (!it->is_directory(typeError) || typeError) continue;
            out.push_back(fileOf(it->path()));
            collectSubdirectories(it->path(), out);
        }
    }
    std::vector<juce::File> subdirectoriesRecursive(const juce::File& top)
    {
        // A tree in RAR archives (0.8.157): the paths below it are served by SampleFiles.
        if (!top.isDirectory() || !SampleFiles::archivesIn(top).empty()) return SampleFiles::subdirectoriesRecursive(top);
        std::vector<juce::File> result;
        collectSubdirectories(fsPath(top), result);
        return result;
    }
    // The names of the plain files of a folder (no stat).
    std::vector<juce::String> fileNamesOf(const juce::File& folder)
    {
        if (!folder.isDirectory()) return SampleFiles::fileNames(folder);   // an archive folder (0.8.157)
        std::vector<juce::String> names;
        std::error_code error;
        for (std::filesystem::directory_iterator it(fsPath(folder), std::filesystem::directory_options::skip_permission_denied, error), end;
             !error && it != end; it.increment(error))
        {
            std::error_code typeError;
            if (it->is_directory(typeError)) continue;
            names.push_back(juce::String(it->path().filename().wstring().c_str()));
        }
        return names;
    }
#else
    // POSIX (Linux, macOS back to 10.11 - std::filesystem exists there only from 10.15).
    bool entryIsDirectory(const std::string& fullPath, const dirent* entry)
    {
        if (entry->d_type == DT_DIR) return true;
        if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK) return false;
        struct stat st;
        return stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }
    void collectSubdirectories(const std::string& dir, std::vector<juce::File>& out)
    {
        DIR* d = opendir(dir.c_str());
        if (d == nullptr) return;
        while (dirent* entry = readdir(d))
        {
            const std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            const std::string full = dir + "/" + name;
            if (!entryIsDirectory(full, entry)) continue;
            out.push_back(juce::File(juce::String(juce::CharPointer_UTF8(full.c_str()))));
            collectSubdirectories(full, out);
        }
        closedir(d);
    }
    std::vector<juce::File> subdirectoriesRecursive(const juce::File& top)
    {
        // A tree in RAR archives (0.8.157): the paths below it are served by SampleFiles.
        if (!top.isDirectory() || !SampleFiles::archivesIn(top).empty()) return SampleFiles::subdirectoriesRecursive(top);
        std::vector<juce::File> result;
        collectSubdirectories(top.getFullPathName().toStdString(), result);
        return result;
    }
    std::vector<juce::String> fileNamesOf(const juce::File& folder)
    {
        if (!folder.isDirectory()) return SampleFiles::fileNames(folder);   // an archive folder (0.8.157)
        std::vector<juce::String> names;
        const std::string dir = folder.getFullPathName().toStdString();
        DIR* d = opendir(dir.c_str());
        if (d == nullptr) return names;
        while (dirent* entry = readdir(d))
        {
            const std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            if (entryIsDirectory(dir + "/" + name, entry)) continue;
            names.push_back(juce::String(juce::CharPointer_UTF8(name.c_str())));
        }
        closedir(d);
        return names;
    }
#endif
}

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <timeapi.h>   // timeBeginPeriod (the control thread's 1 ms waits)
 #include <intrin.h>
 #pragma comment(lib, "winmm.lib")
#endif

// The SIMD kernels per processor family: AVX2 (eight frames a step) and SSE2
// (four) on x86-64 - MSVC, GCC and clang alike, the AVX2 functions carrying
// their own target attribute so the rest of the binary stays SSE2 - and NEON
// (four) on arm64 (Apple silicon, Linux arm64). The kernel is chosen at run
// time from what the CPU reports. ENGINE_CPU_PAUSE is the spin-wait hint of
// the architecture (a thread yield is a system call - too slow for a spin).
#if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64__) || defined(__i386__)
 #define ENGINE_X86 1
 #include <immintrin.h>
 #if defined(_MSC_VER)
  #define ENGINE_AVX2_FN
 #else
  #define ENGINE_AVX2_FN __attribute__((target("avx2")))
  #include <x86intrin.h>   // __rdtsc (the profiling clock)
 #endif
 // The spin-wait hint: _mm_pause on Windows (the workers hold their cores
 // there, with MMCSS behind them); a thread yield everywhere else, as before
 // 0.8.119 - on a two-core laptop a spinning worker took the core the sound
 // server and the note bookkeeping needed (#9, #10).
 #if JUCE_WINDOWS
  #define ENGINE_CPU_PAUSE() _mm_pause()
 #else
  #define ENGINE_CPU_PAUSE() std::this_thread::yield()
 #endif
#elif defined(__aarch64__) || defined(_M_ARM64)
 #define ENGINE_ARM64 1
 #include <arm_neon.h>
 #define ENGINE_CPU_PAUSE() std::this_thread::yield()
#else
 #define ENGINE_CPU_PAUSE() std::this_thread::yield()
#endif
// The wait of a real-time thread for another thread (the poster for its lanes,
// a worker for its task): on Windows the pause instruction (the lanes hold
// their cores, MMCSS behind them); everywhere else a short SLEEP, never a
// yield - a SCHED_FIFO thread's sched_yield hands the core only to threads of
// the same policy, so on a two-core Linux laptop the yielding audio thread and
// worker kept both cores and the window thread (SCHED_OTHER) starved; as the
// window thread drives the polyphony measurement's strokes, the keys stayed
// down, the load stayed, and the program hung for good (#15, 0.8.126).
#if JUCE_WINDOWS
 #define ENGINE_RT_WAIT() _mm_pause()
#else
 #define ENGINE_RT_WAIT() std::this_thread::sleep_for(std::chrono::microseconds(30))
#endif

//==============================================================================
// Small helpers
//==============================================================================
namespace
{
    // Defaults of the level coupling (now Configuration Editor settings on the engine):
    // tremulant 0.187 dB per cent of pitch change, wind sag 0.2 dB per cent of sag.
    constexpr int kMaxRoutes = 128;
    constexpr int kSourceKeys = 7 * 128;           // manuals -1..5
    constexpr int kWindVariations = 16;
    constexpr int kTremBuckets = 128;
    constexpr double kFixedOne = 4294967296.0;    // 32.32 fixed point
    constexpr int kMaxBlockFrames = 1024;         // longest render block (RenderPool::kFrames)
    // Retune buses (0.8.163): the COMMON part of a voice's retune - the global pitch
    // and an organ's tune-to-440 diapason shift - is taken out of the voices and
    // applied once to their mixed output with a windowed-sinc resampler. The
    // voices of a bus render into its FIFO at the route rate x ratio, so their own
    // increment stays near 1 and the linear kernel leaves no images; only a shift
    // of kBusMinCents or more gets a bus, smaller ones (temperaments, the random
    // detune) play exactly as before. A voice whose shift fits no bus (all slots
    // taken by other values) plays as before too.
    constexpr int kMaxBuses = 3;
    constexpr int kBusTaps = 16, kBusPhases = 128;
    constexpr int kBusFifo = 4 * kMaxBlockFrames + 2 * kBusTaps;
    constexpr float kBusMinCents = 25.0f, kBusJoinCents = 6.0f;

    // Voice profiling (the polyphony measurement switches it on): where a lane's
    // time goes per voice and block - the voice header (commands, pitch, gains,
    // tone targets: the bookkeeping), the span logic (loops, splices, envelopes),
    // the interpolation kernels, and the tone/shelf post-processing. Thread-local
    // sums, folded into the pool's statistics after every chunk. Off: one load.
    std::atomic<bool> gVoiceProfiling{ false };
    std::atomic<float> gRetuneLimitCents{ 200.0f };   // the retune limit (0.8.165) for the static tuning analysis; set with SamplerEngine::retuneLimitCents
    struct VoiceProfile { int64_t header = 0, spans = 0, kernel = 0, post = 0, unpack = 0; int64_t voices = 0; };
    thread_local VoiceProfile tlVoiceProfile;

    // Packed samples (Sample::compress16, 0.8.145): the window a lane decodes
    // before a span - the main region and, for a splice, the head region a loop
    // length away - per thread, kept while the next spans of the voice read
    // inside it. kPackedWindow frames a region, 24 of them the SIMD guard.
    constexpr int kPackedWindow = 4096;
    constexpr int kPackedGuard = 24;
    constexpr int kPackedSpanFrames = kPackedWindow - 40;   // what a span may advance within one window
   #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4324)   // padded for the aligned buffer on purpose
   #endif
    struct PackedWindow
    {
        const Sample* sample = nullptr;
        int64_t from = 0, to = 0;      // sample frames of the main region
        int channels = 0;
        bool headValid = false;
        int64_t headFrames = 0;        // the head region = main region + headFrames
        alignas(32) int16_t buf[2][2][kPackedWindow];   // [channel][main/head][frame]
        alignas(32) float bufF[2][2][kPackedWindow];    // the same regions for a packed float sample (0.8.183)
        template <typename T> T* region(int ch, int which) noexcept
        {
            if constexpr (std::is_same<T, float>::value) return bufF[ch][which];
            else return buf[ch][which];
        }
    };
    // The frames [from, to) of a packed (or raw) sample as the type the kernels read.
    template <typename T> inline void decodeWindow(const Sample& s, int ch, int64_t from, int64_t to, T* out) noexcept
    {
        if constexpr (std::is_same<T, float>::value) s.decode32(ch, from, to, out);
        else s.decode16(ch, from, to, out);
    }
   #if defined(_MSC_VER)
    #pragma warning(pop)
   #endif
    thread_local PackedWindow tlPackedWindow;
    // The device thread's profile (0.8.137): inside pool->render, this thread's own
    // rendering and the rest (waiting for the other lanes, the hand-over, merging
    // their scratch); inside the IR hand-over, the wait for the convolution task.
    thread_local int64_t tlPoolOwnTicks = 0, tlPoolWaitTicks = 0, tlReverbWaitTicks = 0;
    // The profiling clock is the CPU's cycle counter (rdtsc, cntvct_el0), a
    // register read of a few ns: six reads per voice. The system clock of JUCE
    // (clock_gettime) was a ~1 us call on an old Linux laptop with an HPET clock
    // source and inflated the bookkeeping and span figures of the Core 2 Duo
    // threefold (0.8.123, #11). Calibrated once against the system clock when
    // profiling starts (profileTicksPerSecond); a delta that is negative or
    // over a second (a thread migrated between cores with unsynchronised
    // counters, a sleep) is dropped.
   #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    inline int64_t profileTicks() noexcept { return (int64_t)__rdtsc(); }
   #elif (defined(__aarch64__) || defined(__arm64__)) && (defined(__GNUC__) || defined(__clang__))
    inline int64_t profileTicks() noexcept { uint64_t v; __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v)); return (int64_t)v; }
   #else
    inline int64_t profileTicks() noexcept { return juce::Time::getHighResolutionTicks(); }
   #endif
    std::atomic<int64_t> gProfileTicksPerSecond{ 0 };
    inline int64_t profileTicksPerSecond() noexcept
    {
        const int64_t known = gProfileTicksPerSecond.load(std::memory_order_relaxed);
        return known > 0 ? known : juce::Time::getHighResolutionTicksPerSecond();
    }
    // Measures the counter against the system clock over 100 ms; called from
    // the message thread when profiling is switched on (once).
    void calibrateProfileClock() noexcept
    {
        if (gProfileTicksPerSecond.load() > 0) return;
        const int64_t sysPerSecond = juce::Time::getHighResolutionTicksPerSecond();
        const int64_t sys0 = juce::Time::getHighResolutionTicks();
        const int64_t prof0 = profileTicks();
        int64_t sys1 = sys0, prof1 = prof0;
        while (sys1 - sys0 < sysPerSecond / 10) { juce::Thread::sleep(5); sys1 = juce::Time::getHighResolutionTicks(); prof1 = profileTicks(); }
        const double seconds = (double)(sys1 - sys0) / (double)sysPerSecond;
        const int64_t perSecond = seconds > 0.0 && prof1 > prof0 ? (int64_t)((double)(prof1 - prof0) / seconds) : sysPerSecond;
        gProfileTicksPerSecond.store(perSecond);
        CrashHandler::note("Profiling clock: " + juce::String((double)perSecond * 1e-6, 1) + " MHz");
    }
    inline int64_t profileDelta(int64_t from, int64_t to) noexcept
    {
        const int64_t d = to - from;
        return d >= 0 && d <= profileTicksPerSecond() ? d : 0;
    }

    // One or two physical cores: no spare core for the note bookkeeping, the
    // sound server and the window while every lane renders - smaller chunks
    // (a lane preempted in the middle of a chunk holds the whole job up), the
    // waits yield the core instead of spinning on it.
    const bool gFewCores = juce::SystemStats::getNumPhysicalCpus() <= 2;

    // The free physical memory now (Windows: available; Linux: MemAvailable;
    // macOS: free + inactive pages), and the reserve the loading keeps for the
    // system: below it the system starts compressing and paging, and every
    // stroke stalls on reading samples back (#16, #17) - the loading stops
    // there instead (loadDivisionTree).
    // The memory figures (MemoryInfo.h, shared with the window): the loading
    // stops when the memory is short - the free memory under the reserve, the
    // program's footprint within the system's share, or paging under way. The
    // free figure alone was not enough: a 7 GB organ loaded whole on an 8 GB
    // Mac (the system keeps inactive pages around while the compressor already
    // holds the samples) and on a 7.7 GB Linux laptop (MemAvailable counts the
    // reclaimable cache; the program held 6.9 GB, 2 GB were "free", and it
    // swapped) - issues #17, #19.
    juce::int64 freePhysicalMemoryBytes() { return MemoryInfo::freeBytes(); }
    // The worker threads of the loading and the copy builds run below the window
    // thread (0.8.139): with all cores busy building filled pipes the message
    // box's OK did not react, and the loading dialog's Cancel must.
    void lowerCurrentThreadPriority() noexcept
    {
       #if JUCE_WINDOWS
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
       #elif JUCE_LINUX
        setpriority(PRIO_PROCESS, (id_t)syscall(SYS_gettid), 10);   // the nice value of THIS thread on Linux
       #elif JUCE_MAC
        sched_param sp{}; int policy = 0;
        if (pthread_getschedparam(pthread_self(), &policy, &sp) == 0)
        {
            sp.sched_priority = juce::jmax(sched_get_priority_min(policy), sp.sched_priority - 8);
            pthread_setschedparam(pthread_self(), policy, &sp);
        }
       #endif
    }
    // The swapped/compressed figure at the start of a load: the paging rule
    // counts the growth since then, not the figure itself - the macOS
    // compressor holds other programs' pages at idle (0.43 GB on the 8 GB Mac,
    // #21, and the rule stopped a load of 2.4 GB). -1: no load running.
    std::atomic<juce::int64> gSwapBaseline{ -1 };
    std::atomic<uint64_t> gRenderWorkerAffinity{ 0 };   // the render workers' processors (Windows, a hybrid CPU's performance cores; 0 = any)
    bool memoryShortNow(juce::String* reason = nullptr) { return MemoryInfo::shortNow(reason, juce::jmax((juce::int64)0, gSwapBaseline.load())); }

    // The render workers and the control thread run beside the audio callbacks
    // at audio priority on every system (Windows had it from the start; on
    // Linux and macOS they ran at the priority of the window - on a two-core
    // laptop a worker preempted by the sound server or the window held every
    // block up, #9/#10). Returns what was granted, for the log.
    juce::String raiseAudioThreadPriority(const char* what)
    {
       #if JUCE_WINDOWS
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        if (HMODULE avrt = LoadLibraryW(L"avrt.dll"))
        {
            typedef HANDLE (WINAPI *AvSetFn)(LPCWSTR, LPDWORD);
            if (auto fn = (AvSetFn)GetProcAddress(avrt, "AvSetMmThreadCharacteristicsW"))
            {
                DWORD taskIndex = 0;
                fn(L"Pro Audio", &taskIndex);
            }
        }
        return juce::String(what) + ": highest priority, MMCSS Pro Audio";
       #elif JUCE_MAC
        // A time-constraint (real-time) thread like CoreAudio's own: a 10 ms
        // period with up to 8 ms of computation, preemptible.
        mach_timebase_info_data_t timebase{};
        mach_timebase_info(&timebase);
        auto toAbsolute = [&timebase](double ms) { return (uint32_t)(ms * 1.0e6 * (double)timebase.denom / (double)juce::jmax(1u, timebase.numer)); };
        thread_time_constraint_policy_data_t policy{};
        policy.period = toAbsolute(10.0);
        policy.computation = toAbsolute(8.0);
        policy.constraint = toAbsolute(10.0);
        policy.preemptible = 1;
        const auto result = thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_TIME_CONSTRAINT_POLICY,
            (thread_policy_t)&policy, THREAD_TIME_CONSTRAINT_POLICY_COUNT);
        return juce::String(what) + (result == KERN_SUCCESS ? ": real-time (time constraint) thread" : ": real-time priority REFUSED (" + juce::String((int)result) + ")");
       #else
        // SCHED_FIFO when the limits allow it (PipeWire's own threads run that
        // way; most desktop setups grant it to the audio group or through
        // rtkit), SCHED_RR as the second try, else the ordinary scheduler.
        sched_param param{};
        const int maxFifo = sched_get_priority_max(SCHED_FIFO);
        param.sched_priority = juce::jlimit(1, juce::jmax(1, maxFifo), maxFifo * 6 / 10);
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) == 0) return juce::String(what) + ": SCHED_FIFO " + juce::String(param.sched_priority);
        if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0) return juce::String(what) + ": SCHED_RR " + juce::String(param.sched_priority);
        return juce::String(what) + ": real-time priority REFUSED (" + juce::String(errno) + ") - the ordinary scheduler; allow it in /etc/security/limits.d (rtprio) or run under PipeWire's rtkit";
       #endif
    }
    // Dynamic tone: every level reduction by wind, tremulant or swell also dulls
    // the tone with a high shelf whose transition band starts at 1.8 kHz and is
    // about 1.8 kHz wide: out = (1 - k) x + k lowpass(x) with a fourth-order
    // lowpass (two Butterworth sections) at 3 kHz, which is -1 dB at 1.8 kHz,
    // -10 dB at 3.6 kHz and -30 dB at 7 kHz. kShelfDepth is the shelf mix for
    // -3 dB: 1 - 10^(-3/20).
    constexpr float kShelfDepth = 0.29205f;
    constexpr float kShelfStartHz = 1800.0f;
    constexpr float kShelfCornerHz = 3000.0f;
    // -ln(g) for g in (0, 1]: Pade form 2u(1 + u^2/3), u = (1 - g)/(1 + g); within
    // a few per cent down to g = 0.2 (-14 dB), far cheaper than std::log.
    inline float fastNegLn(float g) noexcept
    {
        const float u = (1.0f - g) / (1.0f + g);
        return 2.0f * u * (1.0f + u * u * (1.0f / 3.0f));
    }

    inline int makeSourceKey(int midiNote, int manualIndex) noexcept
    {
        return juce::jlimit(0, kSourceKeys - 1, (manualIndex + 1) * 128 + midiNote);
    }
    inline int keyNote(int key) noexcept { return key % 128; }
    inline int keyManual(int key) noexcept { return key / 128 - 1; }

    juce::String comparableFolderName(juce::String text)
    {
        return text.removeCharacters("-_ \t\r\n").toLowerCase();
    }

    juce::String displayFolderName(juce::String text)
    {
        text = text.replaceCharacter('_', ' ');
        juce::String result;
        for (int i = 0; i < text.length(); ++i)
        {
            if (i > 0 && juce::CharacterFunctions::isLetter(text[i - 1])
                && juce::CharacterFunctions::isDigit(text[i])
                && !juce::CharacterFunctions::isWhitespace(result.getLastCharacter()))
                result += ' ';
            result += text[i];
        }
        return result;
    }

    // A folder named after its sibling or its parent with "tr" or "trem"
    // appended ("02-suchy_pos_bourdontr" beside "02-suchy_pos_bourdon",
    // "HW_Gedackt8TREM" beside "HW_Gedackt8", "Ped_claironTr" beside
    // "Ped_clairon"): the tremmed recording of that stop, never loaded by a
    // tree scan - it stood in the pool as a second copy of every stop (0.8.135,
    // Bellevue, Billerbeck). 'keys' = the lower-case full paths of every folder
    // of the tree. A folder a row names explicitly is loaded whatever its name.
    std::set<juce::String> folderKeysOf(const std::vector<juce::File>& folders)
    {
        std::set<juce::String> keys;
        for (const auto& f : folders) keys.insert(f.getFullPathName().toLowerCase());
        return keys;
    }
    bool isTremmedVariantFolder(const juce::File& folder, const std::set<juce::String>& keys)
    {
        const auto name = folder.getFileName().toLowerCase();
        juce::String base;
        if (name.endsWith("trem")) base = name.dropLastCharacters(4);
        else if (name.endsWith("trrp")) base = name.dropLastCharacters(4);   // 0.8.198: Altenbruch "RP_BlockFl2TRRP" / "Dulcian16trRP" beside "RP_BlockFl2"
        else if (name.endsWith("tr")) base = name.dropLastCharacters(2);
        else return false;
        base = base.trimCharactersAtEnd("-_ ");
        if (base.isEmpty()) return false;
        const auto parent = folder.getParentDirectory();
        if (parent.getFileName().toLowerCase() == base) return true;
        return keys.count(parent.getChildFile(base).getFullPathName().toLowerCase()) > 0;
    }

    // The perspective prefix of a folder name taken off ("dir-Principal 8" ->
    // "Principal 8"). One pattern line may list several prefixes separated by
    // ';': ranks with any of them belong to one perspective (one audio output)
    // named after the first prefix of the line.
    juce::String stripPerspectivePrefix(const std::vector<juce::String>& perspectives, juce::String name, juce::String* matchedPerspective)
    {
        for (const auto& rawLine : perspectives)
        {
            auto alternatives = juce::StringArray::fromTokens(rawLine, ";", "");
            alternatives.trim();
            alternatives.removeEmptyStrings();
            if (alternatives.isEmpty()) continue;
            const juce::String perspectiveName = alternatives[0];
            for (const auto& pattern : alternatives)
            {
                const auto comparablePattern = comparableFolderName(pattern);
                if (comparablePattern.isEmpty() || !comparableFolderName(name).startsWith(comparablePattern)) continue;
                int significant = 0, consumed = 0;
                while (consumed < name.length() && significant < comparablePattern.length())
                {
                    const auto c = name[consumed];
                    if (c != '-' && c != '_' && !juce::CharacterFunctions::isWhitespace(c)) ++significant;
                    ++consumed;
                }
                while (consumed < name.length()
                    && (name[consumed] == '-' || name[consumed] == '_' || juce::CharacterFunctions::isWhitespace(name[consumed])))
                    ++consumed;
                if (matchedPerspective != nullptr) *matchedPerspective = perspectiveName;
                return name.substring(consumed).trim();
            }
        }
        return name;
    }

    float windStopContribution(juce::String name)
    {
        name = name.trimEnd();
        while (name.isNotEmpty() && (name.endsWithChar('\'') || name.endsWithChar('"')))
            name = name.dropLastCharacters(1).trimEnd();
        int firstDigit = name.length();
        while (firstDigit > 0 && juce::CharacterFunctions::isDigit(name[firstDigit - 1]))
            --firstDigit;
        if (firstDigit == name.length()) return 1.0f;
        const int trailingNumber = name.substring(firstDigit).getIntValue();
        if (trailingNumber <= 3) return 1.0f / 3.0f;
        if (trailingNumber <= 5) return 2.0f / 3.0f;
        return 1.0f;
    }

    inline double nowMs() noexcept { return juce::Time::getMillisecondCounterHiRes(); }

    //--------------------------------------------------------------------------
    // Fast approximations. Modulation depths are small, so a few 1e-6 relative
    // errors are far below anything audible ("no great precision required").
    inline float fastPow2(float x) noexcept
    {
        x = juce::jlimit(-100.0f, 100.0f, x);
        const float fi = std::floor(x);
        const float f = x - fi;
        const float p = 1.0f + f * (0.6931471806f + f * (0.2402265070f + f * (0.0555041087f
            + f * (0.0096181291f + f * (0.0013333558f + f * (0.0001540353f + f * 0.0000152527f))))));
        union { uint32_t u; float f; } scale;
        scale.u = (uint32_t)((int)fi + 127) << 23;
        return p * scale.f;
    }
    inline float fastDbToGain(float db) noexcept { return fastPow2(db * 0.16609640474f); }
    inline float fastCentsToRatio(float cents) noexcept { return fastPow2(cents * (1.0f / 1200.0f)); }

    struct SinTable
    {
        std::array<float, 1026> v{};
        SinTable()
        {
            for (int i = 0; i < 1026; ++i)
                v[(size_t)i] = (float)std::sin(juce::MathConstants<double>::twoPi * i / 1024.0);
        }
    };
    const SinTable& sinTable() { static SinTable t; return t; }
    // sine of an angle expressed in turns (1 turn = 2 pi)
    inline float fastSinTurns(float turns) noexcept
    {
        const auto& t = sinTable().v;
        const float x = (turns - std::floor(turns)) * 1024.0f;
        const int i = juce::jlimit(0, 1024, (int)x);
        const float f = x - (float)i;
        return t[(size_t)i] + (t[(size_t)i + 1] - t[(size_t)i]) * f;
    }
    inline float fastSinRad(double radians) noexcept
    {
        return fastSinTurns((float)(radians * (1.0 / juce::MathConstants<double>::twoPi)));
    }

    // Publish a struct from the control thread to render threads without locks.
    template <typename T>
    struct SeqLocked
    {
        void write(const T& value) noexcept
        {
            seq.fetch_add(1, std::memory_order_acq_rel);
            std::memcpy((void*)&data, &value, sizeof(T));
            seq.fetch_add(1, std::memory_order_release);
        }
        // Copies into a scratch first so that a torn read never reaches 'out'.
        bool read(T& out) const noexcept
        {
            T scratch;
            for (int attempt = 0; attempt < 16; ++attempt)
            {
                const uint32_t s1 = seq.load(std::memory_order_acquire);
                if (s1 & 1u) { ENGINE_CPU_PAUSE(); continue; }
                std::memcpy(&scratch, (const void*)&data, sizeof(T));
                std::atomic_thread_fence(std::memory_order_acquire);
                if (seq.load(std::memory_order_relaxed) == s1)
                {
                    std::memcpy(&out, &scratch, sizeof(T));
                    return true;
                }
            }
            return false;
        }
        T data{};
        std::atomic<uint32_t> seq{ 0 };
    };
}

//==============================================================================
// Sample
//==============================================================================
Sample::Sample(int channels, int64_t frames, double sr, bool asFloat)
    : sampleRate(sr > 0.0 ? sr : 44100.0), numChannels(juce::jmax(1, channels)),
      numFrames(juce::jmax<int64_t>(0, frames)), storesFloat(asFloat)
{
    stride = numFrames + 24;  // guard frames: the SIMD kernels read up to 16 frames past idx
    const size_t total = (size_t)stride * (size_t)numChannels;
    if (storesFloat) pcm32.calloc(total);
    else pcm16.calloc(total);
}

void Sample::finaliseLoops(std::vector<LoopPoints> lp)
{
    loops.clear();
    loopOrderByEnd.clear();
    crossfadeFrames.clear();
    crossfadeAfterJump.clear();
    maxLoopEnd = 0;
    if (numFrames < 8) return;
    for (auto loop : lp)
    {
        // start >= 1 keeps the pre-loop splice head (start - crossfade) at or above frame 0.
        loop.start = juce::jlimit<int64_t>(1, numFrames - 4, loop.start);
        loop.end = juce::jlimit<int64_t>(0, numFrames - 3, loop.end);
        if (loop.end <= loop.start + 1) continue;
        loops.push_back(loop);
        if ((int)loops.size() >= maxLoops) break;
    }
    loopOrderByEnd.resize(loops.size());
    std::iota(loopOrderByEnd.begin(), loopOrderByEnd.end(), 0);
    std::stable_sort(loopOrderByEnd.begin(), loopOrderByEnd.end(), [this](int a, int b)
    {
        return loops[(size_t)a].end < loops[(size_t)b].end;
    });
    crossfadeFrames.resize(loops.size());
    crossfadeAfterJump.assign(loops.size(), 0);
    for (size_t i = 0; i < loops.size(); ++i)
    {
        const int64_t length = loops[i].end - loops[i].start;
        int64_t xf = (int64_t)(sampleRate * 0.050);
        xf = juce::jmin(xf, loops[i].start, length - 1);
        crossfadeFrames[i] = juce::jmax<int64_t>(1, xf);
        maxLoopEnd = juce::jmax(maxLoopEnd, loops[i].end);
    }
}

//==============================================================================
// Lossless packing of 16-bit samples (0.8.145). A block of kPackBlock frames:
// one header byte (bits 0-1 the predictor order 0/1/2, bits 2-6 the residual
// width w), `order` warm-up frames as plain int16, then one residual of w bits
// per remaining frame, LSB first, stored as residual + 2^(w-1) (w = 0: every
// residual is zero - digital silence, the guard tail). The order with the
// fewest bits wins; w may reach 18 (an order-2 residual of a full-scale step).
// A table of block offsets gives random access: the player decodes only the
// window a span reads.
//==============================================================================
namespace
{
    constexpr int kPackHeaderOrderMask = 3;

    inline int bitLength(uint32_t m) noexcept
    {
        int bits = 0;
        while (m != 0) { ++bits; m >>= 1; }
        return bits;
    }

    // Residual of frame i for the predictor order (the warm-up frames excluded by the caller).
    inline int32_t packResidual(const int16_t* x, int i, int order) noexcept
    {
        if (order == 0) return x[i];
        if (order == 1) return (int32_t)x[i] - (int32_t)x[i - 1];
        return (int32_t)x[i] - 2 * (int32_t)x[i - 1] + (int32_t)x[i - 2];
    }

    // Residuals of w bits need -2^(w-1) <= r < 2^(w-1): the width of a residual.
    inline int residualWidth(int32_t r) noexcept
    {
        const uint32_t m = r < 0 ? (uint32_t)(-(int64_t)r - 1) : (uint32_t)r;
        return bitLength(m) + 1;
    }

    struct BitWriter
    {
        std::vector<uint8_t>& out;
        uint64_t acc = 0;
        int bits = 0;
        explicit BitWriter(std::vector<uint8_t>& o) noexcept : out(o) {}
        void put(uint32_t value, int width) noexcept
        {
            acc |= (uint64_t)value << bits;
            bits += width;
            while (bits >= 8) { out.push_back((uint8_t)(acc & 0xff)); acc >>= 8; bits -= 8; }
        }
        void flush() noexcept
        {
            if (bits > 0) { out.push_back((uint8_t)(acc & 0xff)); acc = 0; bits = 0; }
        }
    };

    // Decodes one block of n frames at `in` into out. Returns false on a corrupt header.
    inline bool unpackBlock(const uint8_t* in, int n, int16_t* out) noexcept
    {
        const int order = in[0] & kPackHeaderOrderMask;
        const int width = (in[0] >> 2) & 31;
        if (order > 2 || width > 24 || n <= 0) return false;
        const uint8_t* p = in + 1;
        int i = 0;
        for (; i < order && i < n; ++i)
        {
            out[i] = (int16_t)(uint16_t)(p[0] | (p[1] << 8));
            p += 2;
        }
        if (width == 0)
        {
            // Every residual zero: the prediction continues (silence for order 0,
            // a constant for order 1, a straight line for order 2).
            for (; i < n; ++i)
                out[i] = order == 0 ? (int16_t)0 : order == 1 ? out[i - 1] : (int16_t)(2 * (int32_t)out[i - 1] - (int32_t)out[i - 2]);
            return true;
        }
        uint64_t acc = 0;
        int bits = 0;
        const uint32_t mask = (1u << width) - 1u;
        const int32_t bias = 1 << (width - 1);
        if (order == 0)
        {
            for (; i < n; ++i)
            {
                while (bits < width) { acc |= (uint64_t)*p++ << bits; bits += 8; }
                out[i] = (int16_t)((int32_t)(acc & mask) - bias);
                acc >>= width; bits -= width;
            }
        }
        else if (order == 1)
        {
            int32_t x1 = out[0];
            for (; i < n; ++i)
            {
                while (bits < width) { acc |= (uint64_t)*p++ << bits; bits += 8; }
                const int32_t x = (int32_t)(acc & mask) - bias + x1;
                acc >>= width; bits -= width;
                out[i] = (int16_t)x;
                x1 = x;
            }
        }
        else
        {
            int32_t x1 = out[1], x2 = out[0];
            for (; i < n; ++i)
            {
                while (bits < width) { acc |= (uint64_t)*p++ << bits; bits += 8; }
                const int32_t x = (int32_t)(acc & mask) - bias + 2 * x1 - x2;
                acc >>= width; bits -= width;
                out[i] = (int16_t)x;
                x2 = x1; x1 = x;
            }
        }
        return true;
    }

    // The same format for 32-bit integers (0.8.183: a float sample packed as the
    // 24-bit or 16-bit values its file held): the warm-up frames are four bytes,
    // the residual width up to 27 bits. The 16-bit functions above are untouched.
    inline bool unpackBlock32(const uint8_t* in, int n, int32_t* out) noexcept
    {
        const int order = in[0] & kPackHeaderOrderMask;
        const int width = (in[0] >> 2) & 31;
        if (order > 2 || width > 27 || n <= 0) return false;
        const uint8_t* p = in + 1;
        int i = 0;
        for (; i < order && i < n; ++i)
        {
            out[i] = (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
            p += 4;
        }
        if (width == 0)
        {
            for (; i < n; ++i)
                out[i] = order == 0 ? 0 : order == 1 ? out[i - 1] : 2 * out[i - 1] - out[i - 2];
            return true;
        }
        uint64_t acc = 0;
        int bits = 0;
        const uint32_t mask = (1u << width) - 1u;
        const int32_t bias = 1 << (width - 1);
        int32_t x1 = order >= 1 ? out[order - 1] : 0, x2 = order >= 2 ? out[order - 2] : 0;
        for (; i < n; ++i)
        {
            while (bits < width) { acc |= (uint64_t)*p++ << bits; bits += 8; }
            const int32_t r = (int32_t)(acc & mask) - bias;
            acc >>= width; bits -= width;
            const int32_t x = order == 0 ? r : order == 1 ? r + x1 : r + 2 * x1 - x2;
            out[i] = x;
            x2 = x1; x1 = x;
        }
        return true;
    }

    void packChannel32(const int32_t* x, int64_t numFrames, Sample::PackedChannel& packed)
    {
        const int64_t blocks = (numFrames + Sample::kPackBlock - 1) / Sample::kPackBlock;
        // Order 0 at 25 bits is the worst a 24-bit value takes: 4 bytes a frame + header and warm-up per block + guard.
        // Packed into the thread's scratch first (0.8.184, see packChannel), then one allocation of the exact size.
        thread_local std::vector<uint8_t> scratch;
        const size_t worst = (size_t)numFrames * 4 + (size_t)blocks * 16 + 64;
        if (scratch.size() < worst) scratch.resize(worst);
        packed.blockOffsets.clear();
        packed.blockOffsets.reserve((size_t)blocks + 1);
        uint8_t* out = scratch.data();
        size_t pos = 0;
        auto magnitude = [](int32_t r) noexcept { return r < 0 ? (uint32_t)(-(int64_t)r - 1) : (uint32_t)r; };
        for (int64_t b = 0; b < blocks; ++b)
        {
            const int32_t* block = x + b * Sample::kPackBlock;
            const int n = (int)juce::jmin<int64_t>(Sample::kPackBlock, numFrames - b * Sample::kPackBlock);
            uint32_t or0 = 0, or1 = 0, or2 = 0;
            bool nz0 = false, nz1 = false, nz2 = false;
            int32_t x1 = 0, x2 = 0;
            for (int i = 0; i < n; ++i)
            {
                const int32_t v = block[i];
                or0 |= magnitude(v); nz0 |= v != 0;
                if (i >= 1) { const int32_t r1 = v - x1; or1 |= magnitude(r1); nz1 |= r1 != 0; }
                if (i >= 2) { const int32_t r2 = v - 2 * x1 + x2; or2 |= magnitude(r2); nz2 |= r2 != 0; }
                x2 = x1; x1 = v;
            }
            const int widths[3] = { nz0 ? bitLength(or0) + 1 : 0, nz1 ? bitLength(or1) + 1 : 0, nz2 ? bitLength(or2) + 1 : 0 };
            int bestOrder = 0, bestWidth = 32;
            int64_t bestBits = std::numeric_limits<int64_t>::max();
            for (int order = 0; order <= 2; ++order)
            {
                if (n <= order || widths[order] > 27) continue;
                const int64_t bits = 8 + 32 * order + (int64_t)(n - order) * widths[order];
                if (bits < bestBits) { bestBits = bits; bestOrder = order; bestWidth = widths[order]; }
            }
            packed.blockOffsets.push_back((uint32_t)pos);
            out[pos++] = (uint8_t)(bestOrder | (bestWidth << 2));
            for (int i = 0; i < bestOrder; ++i)
            {
                const uint32_t u = (uint32_t)block[i];
                out[pos++] = (uint8_t)(u & 0xff); out[pos++] = (uint8_t)((u >> 8) & 0xff);
                out[pos++] = (uint8_t)((u >> 16) & 0xff); out[pos++] = (uint8_t)(u >> 24);
            }
            if (bestWidth > 0)
            {
                const int32_t bias = 1 << (bestWidth - 1);
                uint64_t acc = 0;
                int bits = 0;
                for (int i = bestOrder; i < n; ++i)
                {
                    const int32_t r = bestOrder == 0 ? block[i] : bestOrder == 1 ? block[i] - block[i - 1] : block[i] - 2 * block[i - 1] + block[i - 2];
                    acc |= (uint64_t)(uint32_t)(r + bias) << bits;
                    bits += bestWidth;
                    while (bits >= 8) { out[pos++] = (uint8_t)(acc & 0xff); acc >>= 8; bits -= 8; }
                }
                if (bits > 0) out[pos++] = (uint8_t)(acc & 0xff);
            }
        }
        packed.blockOffsets.push_back((uint32_t)pos);
        jassert(pos + 8 <= worst);
        std::memset(out + pos, 0, 8);   // the bit reader may fetch a few bytes past a block: zeros
        packed.data.assign(out, out + pos + 8);
    }

    void packChannel(const int16_t* x, int64_t numFrames, Sample::PackedChannel& packed)
    {
        // 0.8.153: one pass over a block measures the three predictors at once (the
        // widths come from the OR of the residual magnitudes) and the bits go into a
        // buffer sized for the worst case through a pointer - the three passes and
        // the byte-by-byte push_back of 0.8.145 cost 15 ns per frame (Doesburg 63 s
        // of thread time). The format is unchanged.
        const int64_t blocks = (numFrames + Sample::kPackBlock - 1) / Sample::kPackBlock;
        // The chosen width never exceeds 16 (a wider order-1/2 width always loses to
        // order 0 at 16 bits): 2 bytes a frame + header and warm-up per block + guard.
        // Packed into the thread's scratch (kept across the files a loader thread packs;
        // the threads end with the stop), then ONE allocation of the exact size (0.8.184:
        // a fresh worst-case vector and a shrink_to_fit copy per sample were two
        // multi-MB allocations whose page faults grew dearer as the process grew -
        // the pack went from 6 to 45 ns per frame over a 23 GB load).
        thread_local std::vector<uint8_t> scratch;
        const size_t worst = (size_t)numFrames * 2 + (size_t)blocks * 8 + 64;
        if (scratch.size() < worst) scratch.resize(worst);
        packed.blockOffsets.clear();
        packed.blockOffsets.reserve((size_t)blocks + 1);
        uint8_t* out = scratch.data();
        size_t pos = 0;
        for (int64_t b = 0; b < blocks; ++b)
        {
            const int16_t* block = x + b * Sample::kPackBlock;
            const int n = (int)juce::jmin<int64_t>(Sample::kPackBlock, numFrames - b * Sample::kPackBlock);
            uint32_t or0 = 0, or1 = 0, or2 = 0;
            bool nz0 = false, nz1 = false, nz2 = false;
            auto magnitude = [](int32_t r) noexcept { return r < 0 ? (uint32_t)(-(int64_t)r - 1) : (uint32_t)r; };
            int32_t x1 = 0, x2 = 0;
            for (int i = 0; i < n; ++i)
            {
                const int32_t v = block[i];
                or0 |= magnitude(v); nz0 |= v != 0;
                if (i >= 1) { const int32_t r1 = v - x1; or1 |= magnitude(r1); nz1 |= r1 != 0; }
                if (i >= 2) { const int32_t r2 = v - 2 * x1 + x2; or2 |= magnitude(r2); nz2 |= r2 != 0; }
                x2 = x1; x1 = v;
            }
            // The best order: the fewest bits for the block (a tie keeps the lower order).
            const int widths[3] = { nz0 ? bitLength(or0) + 1 : 0, nz1 ? bitLength(or1) + 1 : 0, nz2 ? bitLength(or2) + 1 : 0 };
            int bestOrder = 0, bestWidth = 32;
            int64_t bestBits = std::numeric_limits<int64_t>::max();
            for (int order = 0; order <= 2; ++order)
            {
                if (n <= order) break;
                const int64_t bits = 8 + 16 * order + (int64_t)(n - order) * widths[order];
                if (bits < bestBits) { bestBits = bits; bestOrder = order; bestWidth = widths[order]; }
            }
            packed.blockOffsets.push_back((uint32_t)pos);
            out[pos++] = (uint8_t)(bestOrder | (bestWidth << 2));
            for (int i = 0; i < bestOrder; ++i)
            {
                const uint16_t u = (uint16_t)block[i];
                out[pos++] = (uint8_t)(u & 0xff);
                out[pos++] = (uint8_t)(u >> 8);
            }
            if (bestWidth > 0)
            {
                const int32_t bias = 1 << (bestWidth - 1);
                uint64_t acc = 0;
                int bits = 0;
                for (int i = bestOrder; i < n; ++i)
                {
                    acc |= (uint64_t)(uint32_t)(packResidual(block, i, bestOrder) + bias) << bits;
                    bits += bestWidth;
                    while (bits >= 8) { out[pos++] = (uint8_t)(acc & 0xff); acc >>= 8; bits -= 8; }
                }
                if (bits > 0) out[pos++] = (uint8_t)(acc & 0xff);
            }
        }
        packed.blockOffsets.push_back((uint32_t)pos);
        jassert(pos + 8 <= worst);
        std::memset(out + pos, 0, 8);   // the bit reader may fetch a few bytes past a block: zeros
        packed.data.assign(out, out + pos + 8);
    }
}

size_t Sample::compress16()
{
    if (compressed || storesFloat || numFrames < kPackBlock || numChannels < 1) return 0;
    if (numFrames > (int64_t)0x7fffffff / 4) return 0;   // the offsets are 32-bit (2 bytes a frame at most)
    std::vector<PackedChannel> result((size_t)numChannels);
    thread_local std::vector<int16_t> check;   // reused across the files a loader thread packs (0.8.184)
    if (check.size() < (size_t)numFrames) check.resize((size_t)numFrames);
    for (int c = 0; c < numChannels; ++c)
    {
        const int16_t* x = channel16(c);
        packChannel(x, numFrames, result[(size_t)c]);
        // Every frame back as it was, else the raw data stays.
        const auto& pc = result[(size_t)c];
        const int64_t blocks = (numFrames + kPackBlock - 1) / kPackBlock;
        for (int64_t b = 0; b < blocks; ++b)
        {
            const int n = (int)juce::jmin<int64_t>(kPackBlock, numFrames - b * kPackBlock);
            if (!unpackBlock(pc.data.data() + pc.blockOffsets[(size_t)b], n, check.data() + b * kPackBlock)) return 0;
        }
        if (std::memcmp(check.data(), x, (size_t)numFrames * sizeof(int16_t)) != 0) return 0;
    }
    size_t bytes = 0;
    for (const auto& pc : result) bytes += pc.data.size() + pc.blockOffsets.size() * sizeof(uint32_t);
    packed = std::move(result);
    compressed = true;
    pcm16.free();
    return bytes;
}

void Sample::decode16(int c, int64_t from, int64_t to, int16_t* out) const noexcept
{
    if (to <= from) return;
    if (!compressed)
    {
        // Raw: the frames as stored (the guard up to stride is zero), zeros beyond.
        const int16_t* x = storesFloat ? nullptr : channel16(c);
        for (int64_t i = from; i < to; ++i)
            out[i - from] = x != nullptr && i >= 0 && i < stride ? x[i] : (int16_t)0;
        return;
    }
    if (c < 0 || c >= (int)packed.size()) { std::memset(out, 0, (size_t)(to - from) * sizeof(int16_t)); return; }
    const auto& pc = packed[(size_t)c];
    const int64_t blocks = (int64_t)pc.blockOffsets.size() - 1;
    int16_t temp[kPackBlock];
    for (int64_t i = from; i < to;)
    {
        if (i < 0) { const int64_t n = juce::jmin<int64_t>(-i, to - i); std::memset(out + (i - from), 0, (size_t)n * sizeof(int16_t)); i += n; continue; }
        if (i >= numFrames) { std::memset(out + (i - from), 0, (size_t)(to - i) * sizeof(int16_t)); break; }
        const int64_t b = i / kPackBlock;
        const int64_t blockStart = b * kPackBlock;
        const int n = (int)juce::jmin<int64_t>(kPackBlock, numFrames - blockStart);
        const int64_t take = juce::jmin<int64_t>(to, blockStart + n) - i;
        if (b >= blocks) { std::memset(out + (i - from), 0, (size_t)take * sizeof(int16_t)); i += take; continue; }
        const uint8_t* in = pc.data.data() + pc.blockOffsets[(size_t)b];
        if (i == blockStart && take == n)
        {
            if (!unpackBlock(in, n, out + (i - from))) std::memset(out + (i - from), 0, (size_t)n * sizeof(int16_t));
        }
        else
        {
            if (!unpackBlock(in, n, temp)) std::memset(temp, 0, sizeof(temp));
            std::memcpy(out + (i - from), temp + (i - blockStart), (size_t)take * sizeof(int16_t));
        }
        i += take;
    }
}

const int16_t* Sample::read16(int c, std::vector<int16_t>& scratch) const
{
    if (!compressed) return channel16(c);
    scratch.assign((size_t)juce::jmax<int64_t>(stride, numFrames), 0);
    decode16(c, 0, numFrames, scratch.data());
    return scratch.data();
}

size_t Sample::compress32()
{
    if (compressed || !storesFloat || numFrames < kPackBlock || numChannels < 1) return 0;
    if (numFrames > (int64_t)0x7fffffff / 8) return 0;   // the offsets are 32-bit (4 bytes a frame at most)
    // The integers the file held: 16-bit (v / 32768) or 24-bit (v / 2^23) - the
    // first scale every frame of every channel fits exactly, bit for bit. A file
    // of real floats (a 32-bit float WAV: Stralsund) has no such integers: its
    // frames are ROUNDED to 24 bits (the nearest v / 2^23, half a step at most -
    // below what any converter resolves), said by packRounded and the log.
    float scale = 0.0f;
    bool rounded = false;
    for (const float candidate : { 1.0f / 32768.0f, 1.0f / 8388608.0f })
    {
        const float inverse = candidate == 1.0f / 32768.0f ? 32768.0f : 8388608.0f;
        bool exact = true;
        for (int c = 0; c < numChannels && exact; ++c)
        {
            const float* x = channel32(c);
            for (int64_t i = 0; i < numFrames; ++i)
            {
                const float scaled = x[i] * inverse;
                if (!(std::abs(scaled) < 16777216.0f)) { exact = false; break; }   // beyond 24 bits, or not a number
                const int32_t v = (int32_t)scaled;
                if ((float)v != scaled || (float)v * candidate != x[i]) { exact = false; break; }
            }
        }
        if (exact) { scale = candidate; break; }
    }
    if (scale == 0.0f)
    {
        // Real floats: rounded to 24 bits - unless any frame lies beyond +-2 (a float
        // file may exceed full scale; such a sample stays as it is).
        for (int c = 0; c < numChannels; ++c)
        {
            const float* x = channel32(c);
            for (int64_t i = 0; i < numFrames; ++i)
                if (!(std::abs(x[i] * 8388608.0f) < 16777215.0f)) return 0;
        }
        scale = 1.0f / 8388608.0f;
        rounded = true;
    }
    const float inverse = scale == 1.0f / 32768.0f ? 32768.0f : 8388608.0f;
    std::vector<PackedChannel> result((size_t)numChannels);
    thread_local std::vector<int32_t> ints, check;   // reused across the files a loader thread packs (0.8.184)
    if (ints.size() < (size_t)numFrames) { ints.resize((size_t)numFrames); check.resize((size_t)numFrames); }
    for (int c = 0; c < numChannels; ++c)
    {
        const float* x = channel32(c);
        for (int64_t i = 0; i < numFrames; ++i) ints[(size_t)i] = rounded ? (int32_t)std::lrint(x[i] * inverse) : (int32_t)(x[i] * inverse);
        packChannel32(ints.data(), numFrames, result[(size_t)c]);
        // Every frame back as it was, else the raw data stays.
        const auto& pc = result[(size_t)c];
        const int64_t blocks = (numFrames + kPackBlock - 1) / kPackBlock;
        for (int64_t b = 0; b < blocks; ++b)
        {
            const int n = (int)juce::jmin<int64_t>(kPackBlock, numFrames - b * kPackBlock);
            if (!unpackBlock32(pc.data.data() + pc.blockOffsets[(size_t)b], n, check.data() + b * kPackBlock)) return 0;
        }
        for (int64_t i = 0; i < numFrames; ++i)
            if (rounded ? check[(size_t)i] != ints[(size_t)i] : (float)check[(size_t)i] * scale != x[i]) return 0;
    }
    size_t bytes = 0;
    for (const auto& pc : result) bytes += pc.data.size() + pc.blockOffsets.size() * sizeof(uint32_t);
    packed = std::move(result);
    packScale = scale;
    packRounded = rounded;
    compressed = true;
    pcm32.free();
    return bytes;
}

void Sample::decode32(int c, int64_t from, int64_t to, float* out) const noexcept
{
    if (to <= from) return;
    if (!compressed)
    {
        const float* x = storesFloat ? channel32(c) : nullptr;
        for (int64_t i = from; i < to; ++i)
            out[i - from] = x != nullptr && i >= 0 && i < stride ? x[i] : 0.0f;
        return;
    }
    if (c < 0 || c >= (int)packed.size() || !storesFloat) { std::fill(out, out + (to - from), 0.0f); return; }
    const auto& pc = packed[(size_t)c];
    const int64_t blocks = (int64_t)pc.blockOffsets.size() - 1;
    int32_t temp[kPackBlock];
    for (int64_t i = from; i < to;)
    {
        if (i < 0) { const int64_t n = juce::jmin<int64_t>(-i, to - i); std::fill(out + (i - from), out + (i - from) + n, 0.0f); i += n; continue; }
        if (i >= numFrames) { std::fill(out + (i - from), out + (to - from), 0.0f); break; }
        const int64_t b = i / kPackBlock;
        const int64_t blockStart = b * kPackBlock;
        const int n = (int)juce::jmin<int64_t>(kPackBlock, numFrames - blockStart);
        const int64_t take = juce::jmin<int64_t>(to, blockStart + n) - i;
        if (b >= blocks) { std::fill(out + (i - from), out + (i - from) + take, 0.0f); i += take; continue; }
        if (!unpackBlock32(pc.data.data() + pc.blockOffsets[(size_t)b], n, temp)) std::memset(temp, 0, sizeof(temp));
        const int32_t* src = temp + (i - blockStart);
        float* dst = out + (i - from);
        for (int64_t k = 0; k < take; ++k) dst[k] = (float)src[k] * packScale;
        i += take;
    }
}

const float* Sample::read32(int c, std::vector<float>& scratch) const
{
    if (!compressed) return channel32(c);
    scratch.assign((size_t)juce::jmax<int64_t>(stride, numFrames), 0.0f);
    decode32(c, 0, numFrames, scratch.data());
    return scratch.data();
}

size_t Sample::frameBytes() const noexcept
{
    if (!compressed) return (size_t)stride * (size_t)numChannels * (storesFloat ? sizeof(float) : sizeof(int16_t));
    size_t bytes = 0;
    for (const auto& pc : packed) bytes += pc.data.size() + pc.blockOffsets.size() * sizeof(uint32_t);
    return bytes;
}

//==============================================================================
// Internal real-time structures
//==============================================================================
namespace
{
    enum VoiceState : uint8_t { VoiceFree = 0, VoicePending = 1, VoiceActive = 2, VoiceFinished = 3 };
    enum VoiceCommand : uint8_t { CmdRelease = 1, CmdSteal = 2, CmdKill = 4, CmdFreeze = 8, CmdResume = 16 };

   #if JUCE_MSVC
    #pragma warning(push)
    #pragma warning(disable: 4324)   // padded to the cache line on purpose
   #endif
    struct alignas(64) Voice
    {
        // ---- written by control before publishing, then read-only
        const Sample* sample = nullptr;
        Stop* stop = nullptr;
        Stop* rank = nullptr;
        uint64_t creationOrder = 0;
        int32_t routeIndex = -1;
        int16_t sourceKey = -1;
        int8_t manual = -1;
        uint8_t note = 0;                 // the pipe (index into the rank), key + shift
        uint8_t keyNote = 0;              // the key that plays it (temperament pitch class)
        uint8_t sampleNote = 0;           // the pipe whose sample is used (differs beyond the compass)
        int8_t shift = 0;                 // transposition of the route that started it
        uint8_t bus = 0;                  // retune bus of its route (1..kMaxBuses) or 0: rendered straight into the route (0.8.163)
        bool fullKernel = false;          // a high pipe (sounding fundamental >= sincKernelFromHz): the 8-tap sinc kernel for its whole length (0.8.164)
        bool cubic = false;               // filled pipe resampled by semitones: cubic interpolation
        bool preShifted = false;          // filled pipe playing a pitch-shifted copy at unity (its shiftCents lack the semitones)
        uint8_t windPitchVar = 0, windGainVar = 0, tremBucket = 0;
        bool detachedRelease = false, loopEnabled = true, couplingUsed = false, startFrozen = false;
        float velocityGain = 1.0f;        // always 1: organ keys do not play softer when struck gently (see 'velocity')
        uint8_t velocity = 127;           // the key velocity: picks the velocity layer (and its release), scales the start fade-in
        float coupledPitchCents = 0.0f;
        uint32_t couplingTotalSamples = 1;
        uint32_t fadeInTotal = 0;
        uint32_t attackHold = 0, attackFade = 0;
        float attackBoost = 1.0f;
        float initialFrozenExpr = 1.0f;
        float initialWindGain = 1.0f;     // release samples: wind level of the pipe at release
        float windKeyScale = 1.0f;        // how strongly this pipe follows the wind sag (small pipes more)
        std::atomic<float> tuningCents{ 0.0f };   // 440 Hz correction + undulation; may change while sounding
        // Transposing transmission: pure-interval correction plus the resampling
        // of a pipe beyond the rank's compass (0 for untransposed voices).
        std::atomic<float> shiftCents{ 0.0f };
        // Volume of the loudest route holding this pipe (a transmission's dB
        // relative to the parent, 1 for the stop itself); the render glides to it.
        std::atomic<float> routeGainTarget{ 1.0f };
        // Per-key tone: high shelf dB of the route with the highest setting holding
        // this pipe (glides 0.5 dB per block in the render); the shelf's start frequency.
        std::atomic<float> toneTarget{ 0.0f };
        float toneStartHz = 1800.0f;
        // A filled pipe played from a lower pipe (transposed upwards) is a little
        // quieter and duller than the real thing: -0.1 dB of level and -0.1 dB of
        // treble per semitone of transposition, set when the voice starts.
        float fillGain = 1.0f;
        float toneOffset = 0.0f;
        // Key velocity on a rank without velocity layers: a gentle stroke starts with
        // an extra negative high shelf (the chiff blunted) that opens over a short time.
        float attackToneDb = 0.0f;              // current extra shelf (<= 0), climbs to 0
        float attackToneStepPerFrame = 0.0f;    // dB per frame it climbs by
        // Temperament reference: 0 = base table, 1 = equal (normalised to 440 Hz),
        // 2 = the rank's own measured temperament.
        std::atomic<uint8_t> tempMode{ 0 };

        // ---- control <-> render atomics
        std::atomic<uint8_t> state{ VoiceFree };
        std::atomic<uint8_t> command{ 0 };
        std::atomic<float> randomPitchCents{ 0.0f };
        std::atomic<float> releaseFadeMsRequested{ 70.0f };
        std::atomic<float> frozenExprRequested{ 1.0f };
        std::atomic<float> curRandomPitch{ 0.0f }, curWindPitch{ 0.0f }, curTremPitch{ 0.0f }, curWindGain{ 1.0f };

        // ---- render private
        uint64_t pos = 0;
        int64_t lastInc = 0;
        float lastGain = 0.0f;
        bool started = false, finished = false, fadingOut = false, killing = false;
        bool attackActive = false, exprFrozen = false, inCrossfade = false;
        bool windFrozen = false;          // wind held constant from the moment of release
        float frozenWindPitch = 0.0f, frozenWindGain = 1.0f, lastWindGain = 1.0f;
        uint32_t fadeInRemaining = 0, fadeOutRemaining = 0, fadeOutTotal = 0;
        uint32_t attackElapsed = 0, couplingElapsed = 0;
        float frozenExpr = 1.0f;
        int nextLoopOrder = 0;
        int targetLoop = -1;
        uint64_t loopDecided = 0, loopTake = 0;
        uint32_t rng = 1;
        // The long splice after a loop jump (crossfadeAfterJump): until tailBlendEndF
        // the continuation past the loop end (pos + tailLoopLenF) fades into the loop.
        bool tailBlend = false;
        uint64_t tailBlendEndF = 0, tailBlendLenF = 0, tailLoopLenF = 0;
        float shelfState[2][4]{};         // harmonic filter: two biquad sections per channel
        float lastShelf = 0.0f;           // shelf mix at the end of the previous block
        float shelfSmoothed = 0.0f;       // shelf mix lagging the level (15 degrees)
        float routeGain = 1.0f, routeGainFrom = 1.0f, routeGainLastTarget = 1.0f;   // linear glide between route volumes
        uint32_t routeGainRemaining = 0, routeGainTotal = 0;
        float toneDb = 0.0f;                        // current tone shelf dB (glides to toneTarget)
        float toneCoeffDb = 1.0e9f, toneCoeffRate = 0.0f;   // what toneCoeff was computed for
        float toneCoeff[5]{};                       // b0 b1 b2 a1 a2
        float toneState[2][2]{};                    // biquad state per channel (transposed direct form II)

        // ---- control private
        int32_t controlIndex = -1;
        int32_t keyPrev = -1, keyNext = -1;
        bool ctlReleasing = false, ctlStolen = false;
    };
   #if JUCE_MSVC
    #pragma warning(pop)
   #endif

    struct BlockContext
    {
        int numSamples = 0;
        double routeRate = 48000.0;
        int outChannels[64]{};
        int outCount = 0;
        float windPitchCents[kWindVariations]{};
        float windGain[kWindVariations]{};
        bool tremActive = false;
        float tremPitchCents[kTremBuckets]{};
        float tremGain[kTremBuckets]{};
        float tremPitchCentsRelease[kTremBuckets]{};
        float tremGainRelease[kTremBuckets]{};
        float exprGain = 1.0f;
        float divisionGain = 1.0f;
        float releaseScale = 1.0f;
        float outputGain = 1.0f;
        float routeGainLagMs = 170.0f;    // glide between route volumes: the division's wind sag lag
        float pitchClassCents[12]{};      // global pitch + (selected - base) temperament
        float selectedCents[12]{};        // global pitch + selected temperament (absolute)
        bool randomAllowed = true, couplingAllowed = true;
        int kernel = 0;             // KernelKind: the interpolation kernel of this block (scalar, SSE2, NEON, AVX2)
        // Dynamic tone shelf: fractions of each modulator's deepest attenuation
        // (0 = none, 1 = full depth); the voice sums them and clamps to 1.
        bool shelfEnabled = true;
        float shelfSwell = 0.60189f, shelfWind = 0.20567f, shelfTrem = 0.36904f;   // mixes at full attenuation (1 - 10^(-db/20))
        float shelfSmoothing = 1.0f;      // per-block smoothing of the shelf mix: 15 degrees lag at the tremulant rate
        float shelfLp[5]{};               // Butterworth lowpass section (b0 b1 b2 a1 a2) at kShelfCornerHz for routeRate
        float exprMaxDb = 13.0f;          // swell: deepest attenuation of this division
        float windMaxDb = 0.0f;           // wind: level change at the current sag limit (0 = no wind)
        float tremShelf[kTremBuckets]{};  // tremulant dip fraction per phase bucket
        float tremShelfRelease[kTremBuckets]{};
    };

    struct LaneOutputs
    {
        float* out[64]{};
        // The harmonic filter's input per channel: every voice adds the share of
        // its signal the dynamic tone shelf is to filter (k * x) here and the rest
        // ((1 - k) * x) to 'out'; the route low-passes the sum once per block and
        // adds it to the output. The filter is linear, so this equals filtering
        // every voice on its own (as before) - at a fraction of the cost.
        float* wet[64]{};
        int numOuts = 0;
        float* sink = nullptr;
    };

    struct WindSourceState;

    // IR reverb of one route: a non-uniform partitioned convolution (JUCE dsp)
    // per output pair. True stereo IRs (4 channels) use two convolutions per
    // pair: the left input through LL/LR, the right input through RL/RR.
    struct RouteReverb
    {
        static constexpr int kMaxBlock = 4096;
        // The head (first kHeadSize samples of the IR) runs with partitions of the
        // device block size; the tail with partitions of kHeadSize, so long tails
        // cost little. The engines must be prepared for the real block size: JUCE
        // partitions by maximumBlockSize, and a partition larger than the blocks
        // actually processed multiplies the FFT work per block.
        static constexpr int kHeadSize = 2048;
        int preparedBlock = 256;
        static constexpr double kMaxPreDelaySeconds = 0.5;
        struct Pair
        {
            juce::dsp::Convolution left{ juce::dsp::Convolution::NonUniform{ kHeadSize } };
            juce::dsp::Convolution right{ juce::dsp::Convolution::NonUniform{ kHeadSize } };   // true stereo only
            // Silence gate: after the input has been silent for longer than the IR,
            // the convolution is skipped (and reset before the next sound).
            int64_t silentSamples = 0;
            bool sleeping = false;
            // Wet-path shaping state: one-pole low pass (high cut), one-pole high
            // pass (low cut) per channel, and the pre-delay ring per channel.
            float lp[2]{ 0.0f, 0.0f }, hp[2]{ 0.0f, 0.0f };
            std::vector<float> delay[2];
            int delayMask = 0, delayWrite = 0;
            // Light processing: the convolution runs at the rate divided by
            // rateDivider. The input is low-passed (anti-alias) and every
            // rateDivider-th sample taken; the wet result is zero-stuffed back and
            // low-passed again. Four biquads (8th-order Butterworth) per channel
            // and direction, the phases carried across blocks so that any block
            // length works.
            juce::dsp::IIR::Filter<float> downFilter[2][4], upFilter[2][4];
            int downPhase = 0, upPhase = 0;
            juce::AudioBuffer<float> down{ 2, kMaxBlock / 2 + 8 };   // the decimated input / wet (rateDivider >= 2)
        };
        std::vector<std::unique_ptr<Pair>> pairs;
        juce::AudioBuffer<float> scratch{ 4, kMaxBlock };
        bool trueStereo = false;
        int channels = 0;
        int irSamples = 0;                // at the prepared rate
        double seconds = 0.0, irSampleRate = 0.0, preparedRate = 0.0;
        // Light processing (weak computers): rate divider 2 or 4 (1 = full), the
        // IR trimmed where its energy has fallen 60 dB (usedSeconds), a true
        // stereo IR reduced to its LL/RR channels. costMsPerSecond: measured at
        // the build - the ms of one thread this reverb takes per second of audio.
        bool light = false;
        bool crossTermsDropped = false;   // a true stereo IR reduced to LL/RR (Light)
        int rateDivider = 1;
        int convolutionBlock = 256;       // the block the engines were prepared for (preparedBlock / rateDivider, rounded up)
        double usedSeconds = 0.0;
        double costMsPerSecond = 0.0;
        // Runtime guard: tasks that took more than 60 % of their period in a row.
        int slowTasks = 0;
        std::atomic<float> wet{ 0.3f };
        // Tone of the wet signal (ReverbTone), read by the render thread.
        std::atomic<float> highCutHz{ 2000.0f }, lowCutHz{ 195.0f }, preDelayMs{ 0.0f }, wetGain{ 0.63f };
    };

    struct Route
    {
        int manual = 0;
        juce::String perspective;
        std::atomic<uint64_t> channelMask{ 3 };
        std::atomic<bool> maskExplicit{ false };
        std::atomic<double> sampleRate{ 48000.0 };
        std::atomic<bool> rateExplicit{ false };
        std::atomic<int> blockSize{ 256 };          // the device's buffer size (reverb partitioning)
        std::atomic<float> volumeGain{ 1.0f };
        // Channel separation 0..1: 1 = every pipe only on its own channel pair, 0 =
        // every channel carries the sum of all (mono to everywhere); in between the
        // other channels leak (1 - separation) of each pipe.
        std::atomic<float> separation{ 1.0f };
        float windPhase = 0.0f;
        // The harmonic filter's input of the block per output channel (the wet
        // shares of all voices, see LaneOutputs) and the low-pass state per channel.
        std::vector<float> wetStore = std::vector<float>((size_t)64 * kMaxBlockFrames);
        float shelfState[64][4]{};
        // Reverb: the specification lives on the control side, the built object is
        // swapped in under renderLock.
        juce::File reverbIrFile;
        float reverbWet = 0.3f;
        SamplerEngine::ReverbTone reverbTone;
        juce::String reverbError;
        std::unique_ptr<RouteReverb> reverb;
        // The device (type|name) whose callback renders this route: the routes of
        // one device render one after another in it (MainComponent sets it when
        // the device starts). Shared IR: routes of one device with the same output
        // channels and the same IR settings (file, wet, tone, decay) convolve once -
        // the first of them owns the engines and the hand-over buffers, the others
        // (reverbOwner set) add their dry signal to its input and carry no engines.
        juce::String deviceKey;
        // Whether a device renders this route at all (deviceKey set): a route set
        // to "none" gets no voices - before 0.8.126 every pipe was published to
        // its perspective's route whatever the device, and on a route without
        // one the voices sat unrendered until cleanStaleRoutes flushed them a
        // second later, counted as polyphony all the while (the "8000 voices" of
        // a Core 2 Duo Mac with the default routes on none and the auxiliary ones
        // on the device: half of them never sounded, #14).
        std::atomic<bool> hasDevice{ false };
        Route* reverbOwner = nullptr;
        int reverbGroupSize = 1;            // owner: the routes of its group, itself included
        int reverbGroupArrived = 0;         // owner: routes that handed their dry over in the current block
        uint32_t reverbGroupBlockId = 0;    // owner: the current block (one up per block)
        uint32_t reverbGroupSeenBlock = 0;  // every route: the owner's block it last handed over to
        juce::SpinLock reverbGroupLock;     // owner: the hand-over (one thread in practice; a lock all the same)

        // Retune buses (see kMaxBuses): made by the control thread when a voice
        // needs one (the vectors allocated then, before 'ready'), rendered by the
        // route's thread. Never freed: a bus keeps its cents for the route's life.
        struct Bus
        {
            std::atomic<bool> ready{ false };
            float cents = 0.0f;
            double ratio = 1.0;                     // input frames per output frame
            std::vector<uint32_t> list;             // its voices of the current block (render thread)
            std::vector<float> fifo[64];            // the bus's rendered frames per route channel
            std::vector<float> wet;                 // the harmonic filter's input, 64 x kMaxBlockFrames
            float shelfState[64][4]{};
            std::vector<float> table;               // kBusPhases x kBusTaps windowed sinc
            double readPos = 0.0;                   // the resampler's position in the FIFO
            int fill = 0;                           // frames rendered into the FIFO
            int idleBlocks = 0;
        };
        Bus buses[kMaxBuses];
        std::vector<uint32_t> direct;               // the voices on no bus (render thread scratch, reserved with the first bus)
        std::atomic<bool> anyBus{ false };

        juce::SpinLock renderLock;
        std::vector<uint32_t> active;               // owned by the render thread
        std::vector<uint32_t> addRing;              // control -> render
        std::atomic<uint32_t> addHead{ 0 }, addTail{ 0 };
        std::vector<uint32_t> finishedScratch;
        std::atomic<int> activeCount{ 0 };
        std::atomic<double> lastRenderMs{ -1.0 };
        std::atomic<float> lastLoad{ 0.0f };
        std::atomic<uintptr_t> lastRenderThread{ 0 };
        // Output discontinuity detector (0.8.226, SamplerEngine::outputEdgeJump): the
        // last sample of the previous block per channel, stage 0 = after the pool
        // render, stage 1 = at the end of renderRoute (after separation, reverb, clip);
        // the jumps counted per stage since the last query. Render thread only.
        SamplerEngine::EdgeState edge[2][64];
        std::atomic<int> edgeJumps[2]{};
        uint32_t edgeDebugBlocks = 0;               // dev hook ORGAN_TEST_GLITCH: blocks rendered, every n-th one silenced after the render

        // render private modulator state
        double clockMs = -1.0;
        double tremPhase = 0.0;
        float tremMix = 0.0f;
        float tremVary = 1.0f;      // wandering strength 0.6..1.0 (see prepareBlock)
        float exprSmoothed = 1.0f;
        bool exprInit = false;

        std::unique_ptr<WindSourceState[]> windCache;   // [6] last successfully read wind snapshots

        // recording: this route's write position in the shared mix ring
        uint32_t recEpoch = 0;
        int64_t recPos = 0;
        std::atomic<int64_t> recPosPublished{ -1 };
        // This route's own stereo recording ring. Routes on different devices
        // render on different threads; a ring shared between them lost samples.
        std::vector<float> recRing[2];

        // The IR convolution runs on a worker, one block behind: the callback
        // hands the block's dry output pairs to irIn, posts the task, and mixes the
        // wet result of the PREVIOUS block (irOut) into this one. Allocated by
        // rebuildRouteReverb (message thread) for the reverb's pairs.
        juce::AudioBuffer<float> irIn, irOut;   // pairs x 2 channels, RouteReverb::kMaxBlock frames (a whole device block)
        int irInFrames = 0, irOutFrames = 0;    // frames handed in / frames of the finished wet block
        bool irOutValid = false;                // irOut holds the previous block's wet signal
        int irTaskIndex = -1;                   // the pool task slot (= route index) while a task may be pending
        struct IrTask { void* impl = nullptr; Route* route = nullptr; } irTask;
    };

    // Stereo mix recording. Every route writes its output into its own ring
    // (Route::recRing) at its own position; the writer thread sums the rings and
    // flushes up to the slowest live route.
    struct Recorder
    {
        static constexpr int ringBits = 19;                 // ~11 s at 48 kHz, per route (4 MB)
        static constexpr int64_t ringSize = (int64_t)1 << ringBits;
        static constexpr int64_t ringMask = ringSize - 1;
        std::atomic<int64_t> flushed{ 0 };
        std::atomic<int64_t> flushTarget{ 0 };              // end of the flush currently in progress
        // The file starts with a fade-in and ends with a fade-out of fadeFrames
        // (500 ms). fadeOutStart is the frame where the fade-out begins (-1 = none);
        // nothing beyond fadeOutStart + fadeFrames is written.
        int64_t fadeFrames = 24000;
        std::atomic<int64_t> fadeOutStart{ -1 };
        std::unique_ptr<juce::AudioFormatWriter> writer;
        juce::AudioBuffer<float> flushBuffer;
        double sampleRate = 48000.0;
        uint32_t epoch = 1;
        juce::File file;
        double wallStartMs = 0.0;   // 0.8.208: the file keeps wall time - a device restart (a division reloaded, a route changed) is silence in it, not a cut
    };

    struct WindTransient
    {
        double startMs = -1.0, durationMs = 0.0;
        float amplitude = 0.0f, oscillations = 0.0f;
        bool release = false;
    };

    struct WindSourceState
    {
        float depthCents = 10.0f, frequencyHz = 7.5f, oscillations = 7.0f;
        float oscillationStrength = 0.70f;
        float baseSmoothingMs = 170.0f;
        uint8_t targetMask = 0;
        std::array<bool, 128> held{};
        float heldWeight = 0.0f;
        int heldCount = 0;
        std::array<int, 4> influentialNotes{ -1, -1, -1, -1 };
        int influentialCount = 0;
        double baseTransitionStartMs = 0.0;
        double baseTransitionDurationMs = 170.0;
        float baseStartSag = 0.0f, targetSag = 0.0f;
        std::array<WindTransient, 64> transients{};
    };

    // Maximum sag a source may produce: 1.25 x depth, extended when the
    // oscillation strength exceeds 100 % (1.0) so stronger impulses are not cut.
    inline float windSagLimit(const WindSourceState& state) noexcept
    {
        return juce::jmax(0.0f, state.depthCents * (1.25f + juce::jmax(0.0f, state.oscillationStrength - 1.0f)));
    }

    float windSagAt(const WindSourceState& state, double timeMs, float phase, bool affectsAnotherDivision) noexcept
    {
        const double baseSmoothingMs = juce::jmax(1.0, state.baseTransitionDurationMs);
        const double bp = juce::jlimit(0.0, 1.0, (timeMs - state.baseTransitionStartMs) / baseSmoothingMs);
        const float smooth = (float)(bp * bp * (3.0 - 2.0 * bp));
        const float divisionScale = affectsAnotherDivision ? 0.5f : 1.0f;
        // The steady sag never goes negative; the oscillation is added on top so
        // that a release impulse (pressure briefly rising, pitch slightly sharp)
        // is heard immediately instead of being clipped by the zero floor until
        // the steady sag has settled.
        const float base = juce::jlimit(0.0f, windSagLimit(state),
            (state.baseStartSag + (state.targetSag - state.baseStartSag) * smooth) * divisionScale);
        const float sinPhase = fastSinRad(phase);
        float pressSum = 0.0f, releaseSum = 0.0f;
        for (const auto& event : state.transients)
        {
            if (event.startMs < 0.0 || event.durationMs <= 0.0) continue;
            const double p = (timeMs - event.startMs) / event.durationMs;
            if (p < 0.0 || p >= 1.0) continue;
            const float envelope = (float)((1.0 - p) * (1.0 - p));
            const double angle = juce::MathConstants<double>::twoPi * event.oscillations * p;
            // Release impulses at 85 % of the attack impulse; other divisions fed
            // from this wind receive 80 % of the oscillation.
            const float eventScale = affectsAnotherDivision
                ? (event.release ? 0.68f : 0.80f)
                : (event.release ? 0.85f : 1.00f);
            const float value = event.amplitude * eventScale * envelope * (fastSinRad(angle + phase) - sinPhase);
            if (event.release) releaseSum -= value;
            else pressSum += value;
        }
        // Pressing keys can only ever lower pitch and level (sag >= 0): the
        // oscillation's negative half is floored at normal wind, so nothing gets
        // louder or sharper because a key was pressed. Only a release, where
        // the pressure genuinely overshoots, may briefly exceed normal by a little.
        // Release oscillations are limited to half the depth, however many stops
        // are drawn; otherwise a full registration made them louder than the sag itself.
        const float releaseLimit = 0.5f * state.depthCents * divisionScale;
        const float sag = juce::jmax(0.0f, base + pressSum) + juce::jlimit(-releaseLimit, releaseLimit, releaseSum);
        return juce::jlimit(-0.35f * state.depthCents * divisionScale, windSagLimit(state), sag);
    }

    //==========================================================================
    // Voice rendering kernels (32.32 fixed-point position, linear interpolation)
    //==========================================================================
    template <typename T> struct SampleScale;
    template <> struct SampleScale<int16_t> { static constexpr float value = 1.0f / 32768.0f; };
    template <> struct SampleScale<float> { static constexpr float value = 1.0f; };

    // The fractional part is reduced to 24 bits so it converts through a fast
    // signed int->float instruction; the 2^-24 rounding is far below audibility.
    constexpr float kFracScale = 1.0f / 16777216.0f;
    inline float fracOf(uint64_t pos) noexcept { return (float)(int32_t)((uint32_t)pos >> 8) * kFracScale; }

    template <typename T>
    inline void prefetchSample(const T* s, uint32_t idx) noexcept
    {
        // Eight cache lines: the first 128 (float) or 256 (int16) source frames of
        // this block, enough for the hardware prefetcher to pick up the stream.
        const char* p = (const char*)(s + idx);
       #if ENGINE_X86
        for (int line = 0; line < 8; ++line) _mm_prefetch(p + line * 64, _MM_HINT_T0);
       #elif defined(__GNUC__) || defined(__clang__)
        for (int line = 0; line < 8; ++line) __builtin_prefetch(p + line * 64, 0, 3);
       #else
        juce::ignoreUnused(p);
       #endif
    }

    template <typename T>
    inline void kernelMono(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const float a = (float)s[idx];
            out[i] += (a + ((float)s[idx + 1] - a) * frac) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereo(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const float a0 = (float)s0[idx], a1 = (float)s1[idx];
            out0[i] += (a0 + ((float)s0[idx + 1] - a0) * frac) * g;
            out1[i] += (a1 + ((float)s1[idx + 1] - a1) * frac) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    //--------------------------------------------------------------------------
    // Cubic (4-point Hermite / Catmull-Rom) kernels for filled pipes, whose
    // sample plays at a different pitch: the images of linear interpolation are
    // ~20 dB lower. Scalar only; few voices use them. The guard frames make
    // s[idx + 2] safe; idx - 1 is clamped at the first frame.
    // The same kernels with the dynamic tone shelf folded in: the voice's signal
    // y goes (1 - k) into the output and k into the wet buffer (the route
    // low-passes the wet sum once per block), k ramping across the span. One
    // multiply-add more per frame instead of a private buffer and a second pass
    // (that pass cost as much as the kernel itself).
    template <typename T>
    inline void kernelMonoShelf(const T* s, float* out, float* wet, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const float a = (float)s[idx];
            const float y = (a + ((float)s[idx + 1] - a) * frac) * g;
            const float w = y * k;
            out[i] += y - w;
            wet[i] += w;
            g += gStep; k += kStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereoShelf(const T* s0, const T* s1, float* out0, float* out1, float* wet0, float* wet1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const float a0 = (float)s0[idx], a1 = (float)s1[idx];
            const float y0 = (a0 + ((float)s0[idx + 1] - a0) * frac) * g;
            const float y1 = (a1 + ((float)s1[idx + 1] - a1) * frac) * g;
            const float w0 = y0 * k, w1 = y1 * k;
            out0[i] += y0 - w0; wet0[i] += w0;
            out1[i] += y1 - w1; wet1[i] += w1;
            g += gStep; k += kStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    // An 8-tap windowed-sinc interpolator (0.8.147) for samples stored at a lower
    // rate than the route's (releases at half the rate, 36/32 kHz storage): the
    // linear kernel, a two-point interpolator, lets the images of what the file
    // holds through at only 7-11 dB below it (a 24 kHz file played at 48 kHz
    // mirrors 6-12 kHz into 12-18 kHz - the "digital ringing" at the start of a
    // reed's release). Eight taps of a Kaiser-windowed sinc put them 40-50 dB
    // down. Used for the first 1.3 s of such a sample (where the treble lives),
    // linear after. 128 phases, taps at idx - 3 .. idx + 4.
    struct PlaySincTable
    {
        static constexpr int phases = 128, taps = 8;
        float h[phases][taps];
        PlaySincTable()
        {
            constexpr double beta = 6.0, cutoff = 0.45;   // cycles per input frame: a little under the file's Nyquist
            auto besselI0 = [](double x) { double sum = 1.0, term = 1.0; for (int k = 1; k < 60; ++k) { const double f = x / (2.0 * k); term *= f * f; sum += term; if (term < 1.0e-12 * sum) break; } return sum; };
            const double i0Beta = besselI0(beta);
            for (int q = 0; q < phases; ++q)
            {
                const double frac = (double)q / (double)phases;
                double sum = 0.0;
                for (int k = 0; k < taps; ++k)
                {
                    const double t = (double)(k - 3) - frac;
                    const double x = t / 4.0;
                    const double window = std::abs(x) < 1.0 ? besselI0(beta * std::sqrt(1.0 - x * x)) / i0Beta : 0.0;
                    const double arg = 2.0 * cutoff * t;
                    const double sinc = std::abs(arg) < 1.0e-12 ? 1.0 : std::sin(juce::MathConstants<double>::pi * arg) / (juce::MathConstants<double>::pi * arg);
                    h[q][k] = (float)(2.0 * cutoff * sinc * window);
                    sum += h[q][k];
                }
                for (int k = 0; k < taps; ++k) h[q][k] = (float)(h[q][k] / sum);
            }
        }
    };
    const PlaySincTable& playSincTable() { static const PlaySincTable table; return table; }

    // The coefficients of a phase, interpolated between the two nearest rows
    // (0.8.164): the nearest row alone (128 phases) stepped the position by up
    // to 1/256 frame, a sawtooth error that put a sideband 48000 x (ratio - 1) Hz
    // under a 9 kHz tone at -41 dB - the very tone the sinc is there to remove.
    template <typename T>
    inline float sincAt(const float (*table)[PlaySincTable::taps], const T* s, uint32_t idx, float frac) noexcept
    {
        const float fp = frac * (float)(PlaySincTable::phases - 1);
        const int q = (int)fp;
        const float w = fp - (float)q;
        const float* h = table[q];
        const float* h2 = table[q + 1 < PlaySincTable::phases ? q + 1 : q];
        const T* p = s + idx - 3;
        float acc = 0.0f;
        for (int k = 0; k < PlaySincTable::taps; ++k) acc += (float)p[k] * (h[k] + (h2[k] - h[k]) * w);
        return acc;
    }

    template <typename T>
    inline void kernelMonoSinc(const T* s, float* out, int n, uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            out[i] += sincAt(table, s, (uint32_t)(pos >> 32), fracOf(pos)) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereoSinc(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            out0[i] += sincAt(table, s0, idx, frac) * g;
            out1[i] += sincAt(table, s1, idx, frac) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline float hermiteAt(const T* s, uint32_t idx, float frac) noexcept
    {
        const float y0 = (float)s[idx > 0 ? idx - 1 : 0];
        const float y1 = (float)s[idx];
        const float y2 = (float)s[idx + 1];
        const float y3 = (float)s[idx + 2];
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * frac + c2) * frac + c1) * frac + y1;
    }

    template <typename T>
    inline void kernelMonoCubic(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            out[i] += hermiteAt(s, (uint32_t)(pos >> 32), fracOf(pos)) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereoCubic(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            out0[i] += hermiteAt(s0, idx, frac) * g;
            out1[i] += hermiteAt(s1, idx, frac) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    // The splice kernels blend the frame at pos with the frame at pos + headOffset
    // (a loop length back: the head before the loop start; a loop length ahead:
    // the continuation past the loop end, for the long blend after the jump).
    template <typename T>
    inline void kernelMonoXfCubic(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const float main = hermiteAt(s, (uint32_t)(pos >> 32), fracOf(pos));
            const uint64_t hp = pos + (uint64_t)headOffset;
            const float head = hermiteAt(s, (uint32_t)(hp >> 32), fracOf(hp));
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            out[i] += (main + (head - main) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereoXfCubic(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep,
        int64_t headOffset, float t, float dt) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const uint64_t hp = pos + (uint64_t)headOffset;
            const uint32_t hidx = (uint32_t)(hp >> 32);
            const float hfrac = fracOf(hp);
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            const float m0 = hermiteAt(s0, idx, frac), m1 = hermiteAt(s1, idx, frac);
            const float h0 = hermiteAt(s0, hidx, hfrac), h1 = hermiteAt(s1, hidx, hfrac);
            out0[i] += (m0 + (h0 - m0) * mix) * g;
            out1[i] += (m1 + (h1 - m1) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelMonoXf(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const float a = (float)s[idx];
            const float main = a + ((float)s[idx + 1] - a) * frac;
            const uint64_t hp = pos + (uint64_t)headOffset;
            const uint32_t hidx = (uint32_t)(hp >> 32);
            const float hfrac = fracOf(hp);
            const float ha = (float)s[hidx];
            const float head = ha + ((float)s[hidx + 1] - ha) * hfrac;
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            out[i] += (main + (head - main) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    template <typename T>
    inline void kernelStereoXf(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep,
        int64_t headOffset, float t, float dt) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const uint64_t hp = pos + (uint64_t)headOffset;
            const uint32_t hidx = (uint32_t)(hp >> 32);
            const float hfrac = fracOf(hp);
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            const float a0 = (float)s0[idx], a1 = (float)s1[idx];
            const float m0 = a0 + ((float)s0[idx + 1] - a0) * frac;
            const float m1 = a1 + ((float)s1[idx + 1] - a1) * frac;
            const float h0a = (float)s0[hidx], h1a = (float)s1[hidx];
            const float h0 = h0a + ((float)s0[hidx + 1] - h0a) * hfrac;
            const float h1 = h1a + ((float)s1[hidx + 1] - h1a) * hfrac;
            out0[i] += (m0 + (h0 - m0) * mix) * g;
            out1[i] += (m1 + (h1 - m1) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }

    //--------------------------------------------------------------------------
    // The kernel the CPU gets: AVX2 (eight frames a step), SSE2 or NEON (four
    // frames a step), scalar when nothing else is there or SIMD is switched off.
    enum KernelKind : int { KernelScalar = 0, KernelSse2 = 1, KernelNeon = 2, KernelAvx2 = 3 };
    inline bool cpuHasAvx2() noexcept
    {
       #if ENGINE_X86
        static const bool has = juce::SystemStats::hasAVX2();
        return has;
       #else
        return false;
       #endif
    }
    inline KernelKind defaultKernel() noexcept
    {
       #if ENGINE_X86
        // x86 without AVX2 (Core 2 Duo and the like): SSE2 (0.8.126). What had
        // made those laptops slower in 0.8.119-0.8.122 was the thread priority
        // and the spin, not the kernel (0.8.123); measured on the same routes
        // with the editor's switch, SSE2 gave 2500 voices against 1700 scalar on
        // a Core 2 Duo Mac (#14, 0.8.125) and benchmarks 5.1 against 7.7 ns on
        // the Linux one (#13). The switch stays for comparing.
        return cpuHasAvx2() ? KernelAvx2 : KernelSse2;
       #elif ENGINE_ARM64
        return KernelNeon;
       #else
        return KernelScalar;
       #endif
    }
    // Whether this build and CPU can run a kernel.
    inline bool kernelAvailable(int kind) noexcept
    {
        switch (kind)
        {
            case KernelScalar: return true;
           #if ENGINE_HAS_AVX2_KERNELS
            case KernelAvx2: return cpuHasAvx2();
           #endif
           #if ENGINE_X86
            case KernelSse2: return true;   // the four-frame kernels exist on every x86-64 build
           #endif
           #if ENGINE_ARM64
            case KernelNeon: return true;
           #endif
            default: return false;
        }
    }
    // A kernel by its setting name: -1 for "auto", -2 for a name this computer
    // cannot run (or an unknown one).
    inline int kernelKindByName(const juce::String& nameIn) noexcept
    {
        const juce::String name = nameIn.trim().toLowerCase();
        if (name.isEmpty() || name == "auto") return -1;
        const int kind = name == "scalar" ? (int)KernelScalar : name == "sse2" ? (int)KernelSse2
                       : name == "avx2" ? (int)KernelAvx2 : name == "neon" ? (int)KernelNeon : -2;
        return kind >= 0 && kernelAvailable(kind) ? kind : -2;
    }
    // The kernel chosen in the Configuration Editor ("Interpolation kernel",
    // 0.8.124), -1 = automatic; read by every block. ORGAN_KERNEL=scalar|sse2|
    // avx2|neon|auto sets it at the start (for measuring one against another
    // on a real organ: the polyphony measurement with each, same computer, same
    // buffer); the editor's choice then replaces it. A kernel the CPU or the
    // build does not have falls back to the default.
    std::atomic<int> gKernelOverride{ -1 };
    inline KernelKind envKernel() noexcept
    {
        static const int chosen = []
        {
            const char* env = std::getenv("ORGAN_KERNEL");
            return env == nullptr ? -1 : kernelKindByName(juce::String(env));
        }();
        return chosen >= 0 ? (KernelKind)chosen : defaultKernel();
    }
    inline KernelKind bestKernel() noexcept
    {
        const int forced = gKernelOverride.load(std::memory_order_relaxed);
        return forced >= 0 ? (KernelKind)forced : envKernel();
    }
    inline const char* kernelName(KernelKind kind) noexcept
    {
        switch (kind) { case KernelAvx2: return "AVX2"; case KernelSse2: return "SSE2"; case KernelNeon: return "NEON"; default: return "scalar"; }
    }

    //--------------------------------------------------------------------------
    // AVX2 kernels: eight output frames per iteration. Only called when the CPU
    // reports AVX2; the functions carry the target attribute (GCC, clang) or
    // compile as plain intrinsics (MSVC), so the rest of the binary stays SSE2.
   #if ENGINE_X86
    #define ENGINE_HAS_AVX2_KERNELS 1

    struct Avx2State
    {
        __m256i offA, offB, stepA, stepB;
        __m256 gain, gainStep8, fracScale;
        int64_t incGroup;   // inc advance per group of eight frames
        int groups;
    };

    ENGINE_AVX2_FN inline void avx2Prepare(Avx2State& st, int n, int64_t inc, int64_t incStep, float g, float gStep) noexcept
    {
        st.groups = n / 8;
        // 64-bit lane order: A = frames {0,1,4,5}, B = frames {2,3,6,7}; the
        // shuffles in the loop then produce frames 0..7 in natural order.
        st.offA = _mm256_set_epi64x(5 * inc, 4 * inc, inc, 0);
        st.offB = _mm256_set_epi64x(7 * inc, 6 * inc, 3 * inc, 2 * inc);
        const int64_t s8 = incStep * 8;
        st.stepA = _mm256_set_epi64x(5 * s8, 4 * s8, s8, 0);
        st.stepB = _mm256_set_epi64x(7 * s8, 6 * s8, 3 * s8, 2 * s8);
        st.gain = _mm256_setr_ps(g, g + gStep, g + 2 * gStep, g + 3 * gStep,
            g + 4 * gStep, g + 5 * gStep, g + 6 * gStep, g + 7 * gStep);
        st.gainStep8 = _mm256_set1_ps(8.0f * gStep);
        st.fracScale = _mm256_set1_ps(kFracScale);
        st.incGroup = 8 * inc + 28 * incStep;
    }

    ENGINE_AVX2_FN inline void avx2Positions(const Avx2State& st, uint64_t pos, __m256i& idx, __m256& frac) noexcept
    {
        const __m256i base = _mm256_set1_epi64x((long long)pos);
        const __m256 pA = _mm256_castsi256_ps(_mm256_add_epi64(base, st.offA));
        const __m256 pB = _mm256_castsi256_ps(_mm256_add_epi64(base, st.offB));
        const __m256i lo = _mm256_castps_si256(_mm256_shuffle_ps(pA, pB, _MM_SHUFFLE(2, 0, 2, 0)));
        idx = _mm256_castps_si256(_mm256_shuffle_ps(pA, pB, _MM_SHUFFLE(3, 1, 3, 1)));
        frac = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_srli_epi32(lo, 8)), st.fracScale);
    }

    ENGINE_AVX2_FN inline __m256 avx2Interpolate(const int16_t* s, __m256i idx, __m256 frac) noexcept
    {
        // Eight consecutive output frames read source frames idx0 .. idx7+1: for
        // pitch ratios up to 2:1 at most 16 int16 = one 256-bit load, widened to
        // two eight-lane int32 vectors and picked per lane - a gather (microcoded,
        // 12-20 cycles on AMD) only for larger ratios. The 24 guard frames of
        // every sample make the 16-frame window safe at the end of the file.
        const int i0 = _mm_cvtsi128_si32(_mm256_castsi256_si128(idx));
        const int i7 = _mm256_extract_epi32(idx, 7);
        if ((unsigned)(i7 - i0) <= 14u)
        {
            const __m256i raw = _mm256_loadu_si256((const __m256i*)(s + i0));
            const __m256i lo = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(raw));        // frames i0 .. i0+7
            const __m256i hi = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(raw, 1));   // frames i0+8 .. i0+15
            const __m256i d = _mm256_sub_epi32(idx, _mm256_set1_epi32(i0));
            const __m256i d1 = _mm256_add_epi32(d, _mm256_set1_epi32(1));
            const __m256i seven = _mm256_set1_epi32(7);
            const __m256i ai = _mm256_blendv_epi8(_mm256_permutevar8x32_epi32(lo, d), _mm256_permutevar8x32_epi32(hi, d), _mm256_cmpgt_epi32(d, seven));
            const __m256i bi = _mm256_blendv_epi8(_mm256_permutevar8x32_epi32(lo, d1), _mm256_permutevar8x32_epi32(hi, d1), _mm256_cmpgt_epi32(d1, seven));
            const __m256 a = _mm256_cvtepi32_ps(ai), b = _mm256_cvtepi32_ps(bi);
            return _mm256_add_ps(a, _mm256_mul_ps(_mm256_sub_ps(b, a), frac));
        }
        const __m256i pair = _mm256_i32gather_epi32((const int*)s, idx, 2);   // s[idx] and s[idx+1] in one 32-bit lane
        const __m256 a = _mm256_cvtepi32_ps(_mm256_srai_epi32(_mm256_slli_epi32(pair, 16), 16));
        const __m256 b = _mm256_cvtepi32_ps(_mm256_srai_epi32(pair, 16));
        return _mm256_add_ps(a, _mm256_mul_ps(_mm256_sub_ps(b, a), frac));
    }

    ENGINE_AVX2_FN inline __m256 avx2Interpolate(const float* s, __m256i idx, __m256 frac) noexcept
    {
        // Eight consecutive output frames read source frames idx0 .. idx7+1. For
        // pitch ratios up to 2:1 that is at most 16 floats, so two contiguous
        // loads plus lane permutes replace the (slow) gathers. The sample's
        // guard frames make the 16-float window safe at the end of the file.
        const int i0 = _mm_cvtsi128_si32(_mm256_castsi256_si128(idx));
        const int i7 = _mm256_extract_epi32(idx, 7);
        if ((unsigned)(i7 - i0) <= 14u)
        {
            const __m256 lo = _mm256_loadu_ps(s + i0);
            const __m256 hi = _mm256_loadu_ps(s + i0 + 8);
            const __m256i d = _mm256_sub_epi32(idx, _mm256_set1_epi32(i0));
            const __m256i d1 = _mm256_add_epi32(d, _mm256_set1_epi32(1));
            const __m256i seven = _mm256_set1_epi32(7);
            const __m256 a = _mm256_blendv_ps(_mm256_permutevar8x32_ps(lo, d), _mm256_permutevar8x32_ps(hi, d),
                _mm256_castsi256_ps(_mm256_cmpgt_epi32(d, seven)));
            const __m256 b = _mm256_blendv_ps(_mm256_permutevar8x32_ps(lo, d1), _mm256_permutevar8x32_ps(hi, d1),
                _mm256_castsi256_ps(_mm256_cmpgt_epi32(d1, seven)));
            return _mm256_add_ps(a, _mm256_mul_ps(_mm256_sub_ps(b, a), frac));
        }
        const __m256 a = _mm256_i32gather_ps(s, idx, 4);
        const __m256 b = _mm256_i32gather_ps(s + 1, idx, 4);
        return _mm256_add_ps(a, _mm256_mul_ps(_mm256_sub_ps(b, a), frac));
    }

    template <typename T>
    ENGINE_AVX2_FN inline void kernelMonoAvx2(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep) noexcept
    {
        Avx2State st;
        avx2Prepare(st, n, inc, incStep, g, gStep);
        int i = 0;
        for (int grp = 0; grp < st.groups; ++grp, i += 8)
        {
            __m256i idx; __m256 frac;
            avx2Positions(st, pos, idx, frac);
            const __m256 v = avx2Interpolate(s, idx, frac);
            _mm256_storeu_ps(out + i, _mm256_add_ps(_mm256_loadu_ps(out + i), _mm256_mul_ps(v, st.gain)));
            st.gain = _mm256_add_ps(st.gain, st.gainStep8);
            st.offA = _mm256_add_epi64(st.offA, st.stepA);
            st.offB = _mm256_add_epi64(st.offB, st.stepB);
            pos += (uint64_t)st.incGroup;
            st.incGroup += 64 * incStep;
            inc += 8 * incStep;
        }
        _mm256_zeroupper();
        if (i < n) kernelMono<T>(s, out + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep);
    }

    template <typename T>
    ENGINE_AVX2_FN inline void kernelStereoAvx2(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        Avx2State st;
        avx2Prepare(st, n, inc, incStep, g, gStep);
        int i = 0;
        for (int grp = 0; grp < st.groups; ++grp, i += 8)
        {
            __m256i idx; __m256 frac;
            avx2Positions(st, pos, idx, frac);
            const __m256 v0 = avx2Interpolate(s0, idx, frac);
            const __m256 v1 = avx2Interpolate(s1, idx, frac);
            _mm256_storeu_ps(out0 + i, _mm256_add_ps(_mm256_loadu_ps(out0 + i), _mm256_mul_ps(v0, st.gain)));
            _mm256_storeu_ps(out1 + i, _mm256_add_ps(_mm256_loadu_ps(out1 + i), _mm256_mul_ps(v1, st.gain)));
            st.gain = _mm256_add_ps(st.gain, st.gainStep8);
            st.offA = _mm256_add_epi64(st.offA, st.stepA);
            st.offB = _mm256_add_epi64(st.offB, st.stepB);
            pos += (uint64_t)st.incGroup;
            st.incGroup += 64 * incStep;
            inc += 8 * incStep;
        }
        _mm256_zeroupper();
        if (i < n) kernelStereo<T>(s0, s1, out0 + i, out1 + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep);
    }
    // The shelf variants: y = v * gain, (1 - k) y into out, k y into wet.
    template <typename T>
    ENGINE_AVX2_FN inline void kernelMonoAvx2Shelf(const T* s, float* out, float* wet, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        Avx2State st;
        avx2Prepare(st, n, inc, incStep, g, gStep);
        __m256 kv = _mm256_setr_ps(k, k + kStep, k + 2 * kStep, k + 3 * kStep, k + 4 * kStep, k + 5 * kStep, k + 6 * kStep, k + 7 * kStep);
        const __m256 kStep8 = _mm256_set1_ps(8.0f * kStep);
        int i = 0;
        for (int grp = 0; grp < st.groups; ++grp, i += 8)
        {
            __m256i idx; __m256 frac;
            avx2Positions(st, pos, idx, frac);
            const __m256 y = _mm256_mul_ps(avx2Interpolate(s, idx, frac), st.gain);
            const __m256 w = _mm256_mul_ps(y, kv);
            _mm256_storeu_ps(out + i, _mm256_add_ps(_mm256_loadu_ps(out + i), _mm256_sub_ps(y, w)));
            _mm256_storeu_ps(wet + i, _mm256_add_ps(_mm256_loadu_ps(wet + i), w));
            st.gain = _mm256_add_ps(st.gain, st.gainStep8);
            kv = _mm256_add_ps(kv, kStep8);
            st.offA = _mm256_add_epi64(st.offA, st.stepA);
            st.offB = _mm256_add_epi64(st.offB, st.stepB);
            pos += (uint64_t)st.incGroup;
            st.incGroup += 64 * incStep;
            inc += 8 * incStep;
        }
        _mm256_zeroupper();
        if (i < n) kernelMonoShelf<T>(s, out + i, wet + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep, k + kStep * (float)i, kStep);
    }

    template <typename T>
    ENGINE_AVX2_FN inline void kernelStereoAvx2Shelf(const T* s0, const T* s1, float* out0, float* out1, float* wet0, float* wet1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        Avx2State st;
        avx2Prepare(st, n, inc, incStep, g, gStep);
        __m256 kv = _mm256_setr_ps(k, k + kStep, k + 2 * kStep, k + 3 * kStep, k + 4 * kStep, k + 5 * kStep, k + 6 * kStep, k + 7 * kStep);
        const __m256 kStep8 = _mm256_set1_ps(8.0f * kStep);
        int i = 0;
        for (int grp = 0; grp < st.groups; ++grp, i += 8)
        {
            __m256i idx; __m256 frac;
            avx2Positions(st, pos, idx, frac);
            const __m256 y0 = _mm256_mul_ps(avx2Interpolate(s0, idx, frac), st.gain);
            const __m256 y1 = _mm256_mul_ps(avx2Interpolate(s1, idx, frac), st.gain);
            const __m256 w0 = _mm256_mul_ps(y0, kv), w1 = _mm256_mul_ps(y1, kv);
            _mm256_storeu_ps(out0 + i, _mm256_add_ps(_mm256_loadu_ps(out0 + i), _mm256_sub_ps(y0, w0)));
            _mm256_storeu_ps(wet0 + i, _mm256_add_ps(_mm256_loadu_ps(wet0 + i), w0));
            _mm256_storeu_ps(out1 + i, _mm256_add_ps(_mm256_loadu_ps(out1 + i), _mm256_sub_ps(y1, w1)));
            _mm256_storeu_ps(wet1 + i, _mm256_add_ps(_mm256_loadu_ps(wet1 + i), w1));
            st.gain = _mm256_add_ps(st.gain, st.gainStep8);
            kv = _mm256_add_ps(kv, kStep8);
            st.offA = _mm256_add_epi64(st.offA, st.stepA);
            st.offB = _mm256_add_epi64(st.offB, st.stepB);
            pos += (uint64_t)st.incGroup;
            st.incGroup += 64 * incStep;
            inc += 8 * incStep;
        }
        _mm256_zeroupper();
        if (i < n) kernelStereoShelf<T>(s0, s1, out0 + i, out1 + i, wet0 + i, wet1 + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep, k + kStep * (float)i, kStep);
    }
   #endif   // ENGINE_X86

    //--------------------------------------------------------------------------
    // Four-frame kernels: SSE2 (every x86-64 CPU) and NEON (arm64: Apple
    // silicon, Linux arm64). The positions are 64-bit scalars; the two source
    // frames of a lane come by scalar loads (one 32-bit load for an int16
    // pair), the arithmetic runs in four lanes. The same code for both: the
    // handful of operations it needs are wrapped once per instruction set.
   #if ENGINE_X86 || ENGINE_ARM64
    #define ENGINE_HAS_LANE4_KERNELS 1
    struct Lane4
    {
       #if ENGINE_X86
        using F = __m128; using I = __m128i;
        static F loadF(const float* p) noexcept { return _mm_loadu_ps(p); }
        static void storeF(float* p, F v) noexcept { _mm_storeu_ps(p, v); }
        static F setF(float a, float b, float c, float d) noexcept { return _mm_setr_ps(a, b, c, d); }
        static F splat(float a) noexcept { return _mm_set1_ps(a); }
        static I setI(int32_t a, int32_t b, int32_t c, int32_t d) noexcept { return _mm_setr_epi32(a, b, c, d); }
        static F lowHalfToF(I pairs) noexcept { return _mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(pairs, 16), 16)); }
        static F highHalfToF(I pairs) noexcept { return _mm_cvtepi32_ps(_mm_srai_epi32(pairs, 16)); }
        static F add(F a, F b) noexcept { return _mm_add_ps(a, b); }
        static F sub(F a, F b) noexcept { return _mm_sub_ps(a, b); }
        static F mul(F a, F b) noexcept { return _mm_mul_ps(a, b); }
       #else
        using F = float32x4_t; using I = int32x4_t;
        static F loadF(const float* p) noexcept { return vld1q_f32(p); }
        static void storeF(float* p, F v) noexcept { vst1q_f32(p, v); }
        static F setF(float a, float b, float c, float d) noexcept { const float t[4]{ a, b, c, d }; return vld1q_f32(t); }
        static F splat(float a) noexcept { return vdupq_n_f32(a); }
        static I setI(int32_t a, int32_t b, int32_t c, int32_t d) noexcept { const int32_t t[4]{ a, b, c, d }; return vld1q_s32(t); }
        static F lowHalfToF(I pairs) noexcept { return vcvtq_f32_s32(vshrq_n_s32(vshlq_n_s32(pairs, 16), 16)); }
        static F highHalfToF(I pairs) noexcept { return vcvtq_f32_s32(vshrq_n_s32(pairs, 16)); }
        static F add(F a, F b) noexcept { return vaddq_f32(a, b); }
        static F sub(F a, F b) noexcept { return vsubq_f32(a, b); }
        static F mul(F a, F b) noexcept { return vmulq_f32(a, b); }
       #endif
    };

    inline int32_t int16PairAt(const int16_t* s, uint32_t idx) noexcept { int32_t v; std::memcpy(&v, s + idx, 4); return v; }   // s[idx] low, s[idx+1] high (little-endian)

    // The two source frames of four lanes, interpolated.
    inline Lane4::F lane4Interpolate(const int16_t* s, const uint32_t* idx, Lane4::F frac) noexcept
    {
        const Lane4::I pairs = Lane4::setI(int16PairAt(s, idx[0]), int16PairAt(s, idx[1]), int16PairAt(s, idx[2]), int16PairAt(s, idx[3]));
        const Lane4::F a = Lane4::lowHalfToF(pairs), b = Lane4::highHalfToF(pairs);
        return Lane4::add(a, Lane4::mul(Lane4::sub(b, a), frac));
    }
    inline Lane4::F lane4Interpolate(const float* s, const uint32_t* idx, Lane4::F frac) noexcept
    {
        const Lane4::F a = Lane4::setF(s[idx[0]], s[idx[1]], s[idx[2]], s[idx[3]]);
        const Lane4::F b = Lane4::setF(s[idx[0] + 1], s[idx[1] + 1], s[idx[2] + 1], s[idx[3] + 1]);
        return Lane4::add(a, Lane4::mul(Lane4::sub(b, a), frac));
    }

    // The positions of four consecutive frames, stepped as the scalar kernel
    // steps them (pos += inc; inc += incStep per frame); returns the position
    // after the four.
    inline uint64_t lane4Positions(uint64_t pos, int64_t inc, int64_t incStep, uint32_t* idx, Lane4::F& frac) noexcept
    {
        const uint64_t p0 = pos;
        const uint64_t p1 = p0 + (uint64_t)inc;
        const uint64_t p2 = p1 + (uint64_t)(inc + incStep);
        const uint64_t p3 = p2 + (uint64_t)(inc + 2 * incStep);
        idx[0] = (uint32_t)(p0 >> 32); idx[1] = (uint32_t)(p1 >> 32); idx[2] = (uint32_t)(p2 >> 32); idx[3] = (uint32_t)(p3 >> 32);
        frac = Lane4::setF(fracOf(p0), fracOf(p1), fracOf(p2), fracOf(p3));
        return p3 + (uint64_t)(inc + 3 * incStep);
    }

    template <typename T>
    inline void kernelMonoLane4(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep) noexcept
    {
        Lane4::F gain = Lane4::setF(g, g + gStep, g + 2 * gStep, g + 3 * gStep);
        const Lane4::F gainStep4 = Lane4::splat(4.0f * gStep);
        int i = 0;
        for (; i + 4 <= n; i += 4)
        {
            uint32_t idx[4]; Lane4::F frac;
            pos = lane4Positions(pos, inc, incStep, idx, frac);
            inc += 4 * incStep;
            const Lane4::F v = lane4Interpolate(s, idx, frac);
            Lane4::storeF(out + i, Lane4::add(Lane4::loadF(out + i), Lane4::mul(v, gain)));
            gain = Lane4::add(gain, gainStep4);
        }
        if (i < n) kernelMono<T>(s, out + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep);
    }

    template <typename T>
    inline void kernelStereoLane4(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        Lane4::F gain = Lane4::setF(g, g + gStep, g + 2 * gStep, g + 3 * gStep);
        const Lane4::F gainStep4 = Lane4::splat(4.0f * gStep);
        int i = 0;
        for (; i + 4 <= n; i += 4)
        {
            uint32_t idx[4]; Lane4::F frac;
            pos = lane4Positions(pos, inc, incStep, idx, frac);
            inc += 4 * incStep;
            const Lane4::F v0 = lane4Interpolate(s0, idx, frac);
            const Lane4::F v1 = lane4Interpolate(s1, idx, frac);
            Lane4::storeF(out0 + i, Lane4::add(Lane4::loadF(out0 + i), Lane4::mul(v0, gain)));
            Lane4::storeF(out1 + i, Lane4::add(Lane4::loadF(out1 + i), Lane4::mul(v1, gain)));
            gain = Lane4::add(gain, gainStep4);
        }
        if (i < n) kernelStereo<T>(s0, s1, out0 + i, out1 + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep);
    }
    // The shelf variants: y = v * gain, (1 - k) y into out, k y into wet.
    template <typename T>
    inline void kernelMonoLane4Shelf(const T* s, float* out, float* wet, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        Lane4::F gain = Lane4::setF(g, g + gStep, g + 2 * gStep, g + 3 * gStep);
        Lane4::F kv = Lane4::setF(k, k + kStep, k + 2 * kStep, k + 3 * kStep);
        const Lane4::F gainStep4 = Lane4::splat(4.0f * gStep), kStep4 = Lane4::splat(4.0f * kStep);
        int i = 0;
        for (; i + 4 <= n; i += 4)
        {
            uint32_t idx[4]; Lane4::F frac;
            pos = lane4Positions(pos, inc, incStep, idx, frac);
            inc += 4 * incStep;
            const Lane4::F y = Lane4::mul(lane4Interpolate(s, idx, frac), gain);
            const Lane4::F w = Lane4::mul(y, kv);
            Lane4::storeF(out + i, Lane4::add(Lane4::loadF(out + i), Lane4::sub(y, w)));
            Lane4::storeF(wet + i, Lane4::add(Lane4::loadF(wet + i), w));
            gain = Lane4::add(gain, gainStep4);
            kv = Lane4::add(kv, kStep4);
        }
        if (i < n) kernelMonoShelf<T>(s, out + i, wet + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep, k + kStep * (float)i, kStep);
    }

    template <typename T>
    inline void kernelStereoLane4Shelf(const T* s0, const T* s1, float* out0, float* out1, float* wet0, float* wet1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, float k, float kStep) noexcept
    {
        Lane4::F gain = Lane4::setF(g, g + gStep, g + 2 * gStep, g + 3 * gStep);
        Lane4::F kv = Lane4::setF(k, k + kStep, k + 2 * kStep, k + 3 * kStep);
        const Lane4::F gainStep4 = Lane4::splat(4.0f * gStep), kStep4 = Lane4::splat(4.0f * kStep);
        int i = 0;
        for (; i + 4 <= n; i += 4)
        {
            uint32_t idx[4]; Lane4::F frac;
            pos = lane4Positions(pos, inc, incStep, idx, frac);
            inc += 4 * incStep;
            const Lane4::F y0 = Lane4::mul(lane4Interpolate(s0, idx, frac), gain);
            const Lane4::F y1 = Lane4::mul(lane4Interpolate(s1, idx, frac), gain);
            const Lane4::F w0 = Lane4::mul(y0, kv), w1 = Lane4::mul(y1, kv);
            Lane4::storeF(out0 + i, Lane4::add(Lane4::loadF(out0 + i), Lane4::sub(y0, w0)));
            Lane4::storeF(wet0 + i, Lane4::add(Lane4::loadF(wet0 + i), w0));
            Lane4::storeF(out1 + i, Lane4::add(Lane4::loadF(out1 + i), Lane4::sub(y1, w1)));
            Lane4::storeF(wet1 + i, Lane4::add(Lane4::loadF(wet1 + i), w1));
            gain = Lane4::add(gain, gainStep4);
            kv = Lane4::add(kv, kStep4);
        }
        if (i < n) kernelStereoShelf<T>(s0, s1, out0 + i, out1 + i, wet0 + i, wet1 + i, n - i, pos, inc, incStep, g + gStep * (float)i, gStep, k + kStep * (float)i, kStep);
    }
   #endif   // ENGINE_X86 || ENGINE_ARM64

    template <typename T>
    inline void dispatchMonoShelf(const T* s, float* out, float* wet, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, float k, float kStep, KernelKind kernel) noexcept
    {
        if (n < 16 || kernel == KernelScalar) { kernelMonoShelf<T>(s, out, wet, n, pos, inc, incStep, g, gStep, k, kStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelMonoAvx2Shelf<T>(s, out, wet, n, pos, inc, incStep, g, gStep, k, kStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelMonoLane4Shelf<T>(s, out, wet, n, pos, inc, incStep, g, gStep, k, kStep);
       #else
        kernelMonoShelf<T>(s, out, wet, n, pos, inc, incStep, g, gStep, k, kStep);
       #endif
    }

    template <typename T>
    inline void dispatchStereoShelf(const T* s0, const T* s1, float* out0, float* out1, float* wet0, float* wet1, int n, uint64_t& pos,
        int64_t& inc, int64_t incStep, float g, float gStep, float k, float kStep, KernelKind kernel) noexcept
    {
        if (n < 16 || kernel == KernelScalar) { kernelStereoShelf<T>(s0, s1, out0, out1, wet0, wet1, n, pos, inc, incStep, g, gStep, k, kStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelStereoAvx2Shelf<T>(s0, s1, out0, out1, wet0, wet1, n, pos, inc, incStep, g, gStep, k, kStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelStereoLane4Shelf<T>(s0, s1, out0, out1, wet0, wet1, n, pos, inc, incStep, g, gStep, k, kStep);
       #else
        kernelStereoShelf<T>(s0, s1, out0, out1, wet0, wet1, n, pos, inc, incStep, g, gStep, k, kStep);
       #endif
    }

    template <typename T>
    inline void dispatchMono(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, KernelKind kernel) noexcept
    {
        if (n < 16 || kernel == KernelScalar) { kernelMono<T>(s, out, n, pos, inc, incStep, g, gStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelMonoAvx2<T>(s, out, n, pos, inc, incStep, g, gStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelMonoLane4<T>(s, out, n, pos, inc, incStep, g, gStep);
       #else
        kernelMono<T>(s, out, n, pos, inc, incStep, g, gStep);
       #endif
    }

    template <typename T>
    inline void dispatchStereo(const T* s0, const T* s1, float* out0, float* out1, int n, uint64_t& pos,
        int64_t& inc, int64_t incStep, float g, float gStep, KernelKind kernel) noexcept
    {
        if (n < 16 || kernel == KernelScalar) { kernelStereo<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelStereoAvx2<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelStereoLane4<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep);
       #else
        kernelStereo<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep);
       #endif
    }

    //--------------------------------------------------------------------------
    // The 8-tap sinc in SIMD (0.8.164): an output frame is eight taps (idx - 3 ..
    // idx + 4) times the eight coefficients of its phase, summed. Scalar it was
    // 13 ns a frame (0.8.148: Doesburg 45 400 -> 15 900 voices with every 32 kHz
    // attack through it); vectorised it serves every high pipe (fullKernel).
   #if ENGINE_HAS_AVX2_KERNELS
    ENGINE_AVX2_FN inline __m256 avx2SincTaps(const int16_t* p) noexcept { return _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm_loadu_si128((const __m128i*)p))); }
    ENGINE_AVX2_FN inline __m256 avx2SincTaps(const float* p) noexcept { return _mm256_loadu_ps(p); }
    ENGINE_AVX2_FN inline float avx2Sum8(__m256 v) noexcept
    {
        __m128 s = _mm_add_ps(_mm256_castps256_ps128(v), _mm256_extractf128_ps(v, 1));
        s = _mm_add_ps(s, _mm_movehl_ps(s, s));
        s = _mm_add_ss(s, _mm_shuffle_ps(s, s, 1));
        return _mm_cvtss_f32(s);
    }
    // The eight coefficients of a position: the two nearest phase rows blended (see sincAt).
    ENGINE_AVX2_FN inline __m256 avx2SincCoeffs(const float (*table)[PlaySincTable::taps], uint64_t p) noexcept
    {
        const float fp = fracOf(p) * (float)(PlaySincTable::phases - 1);
        const int q = (int)fp;
        const __m256 h = _mm256_loadu_ps(table[q]);
        const __m256 h2 = _mm256_loadu_ps(table[q + 1 < PlaySincTable::phases ? q + 1 : q]);
        return _mm256_add_ps(h, _mm256_mul_ps(_mm256_sub_ps(h2, h), _mm256_set1_ps(fp - (float)q)));
    }
    template <typename T>
    ENGINE_AVX2_FN inline void kernelMonoSincAvx2(const T* s, float* out, int n, uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            out[i] += avx2Sum8(_mm256_mul_ps(avx2SincTaps(s + idx - 3), avx2SincCoeffs(table, pos))) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
        _mm256_zeroupper();
    }
    template <typename T>
    ENGINE_AVX2_FN inline void kernelStereoSincAvx2(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const __m256 h = avx2SincCoeffs(table, pos);
            out0[i] += avx2Sum8(_mm256_mul_ps(avx2SincTaps(s0 + idx - 3), h)) * g;
            out1[i] += avx2Sum8(_mm256_mul_ps(avx2SincTaps(s1 + idx - 3), h)) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
        _mm256_zeroupper();
    }
   #endif
   #if ENGINE_HAS_LANE4_KERNELS
    inline Lane4::F lane4SincTaps(const int16_t* p) noexcept
    {
       #if ENGINE_X86
        return _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), _mm_loadl_epi64((const __m128i*)p)), 16));
       #else
        return vcvtq_f32_s32(vmovl_s16(vld1_s16(p)));
       #endif
    }
    inline Lane4::F lane4SincTaps(const float* p) noexcept { return Lane4::loadF(p); }
    inline float lane4Sum(Lane4::F v) noexcept
    {
       #if ENGINE_X86
        __m128 s = _mm_add_ps(v, _mm_movehl_ps(v, v));
        s = _mm_add_ss(s, _mm_shuffle_ps(s, s, 1));
        return _mm_cvtss_f32(s);
       #else
        return vaddvq_f32(v);
       #endif
    }
    // The eight coefficients of a position as two lanes of four (the two nearest phase rows blended, see sincAt).
    inline void lane4SincCoeffs(const float (*table)[PlaySincTable::taps], uint64_t p, Lane4::F& lo, Lane4::F& hi) noexcept
    {
        const float fp = fracOf(p) * (float)(PlaySincTable::phases - 1);
        const int q = (int)fp;
        const float* h = table[q];
        const float* h2 = table[q + 1 < PlaySincTable::phases ? q + 1 : q];
        const Lane4::F w = Lane4::splat(fp - (float)q);
        const Lane4::F a = Lane4::loadF(h), b = Lane4::loadF(h + 4);
        lo = Lane4::add(a, Lane4::mul(Lane4::sub(Lane4::loadF(h2), a), w));
        hi = Lane4::add(b, Lane4::mul(Lane4::sub(Lane4::loadF(h2 + 4), b), w));
    }
    template <typename T>
    inline float lane4SincAt(Lane4::F lo, Lane4::F hi, const T* p) noexcept
    {
        return lane4Sum(Lane4::add(Lane4::mul(lane4SincTaps(p), lo), Lane4::mul(lane4SincTaps(p + 4), hi)));
    }
    template <typename T>
    inline void kernelMonoSincLane4(const T* s, float* out, int n, uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            Lane4::F lo, hi;
            lane4SincCoeffs(table, pos, lo, hi);
            out[i] += lane4SincAt(lo, hi, s + idx - 3) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
    template <typename T>
    inline void kernelStereoSincLane4(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            Lane4::F lo, hi;
            lane4SincCoeffs(table, pos, lo, hi);
            out0[i] += lane4SincAt(lo, hi, s0 + idx - 3) * g;
            out1[i] += lane4SincAt(lo, hi, s1 + idx - 3) * g;
            g += gStep;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
   #endif
    // The loop splices of a high pipe through the sinc as well (0.8.164): a copy
    // of a filled pipe spends up to 40 % of its loop in the crossfade (the setting),
    // and a linear splice there left the image standing for that share.
    template <typename T>
    inline void kernelMonoXfSinc(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const float main = sincAt(table, s, (uint32_t)(pos >> 32), fracOf(pos));
            const uint64_t hp = pos + (uint64_t)headOffset;
            const float head = sincAt(table, s, (uint32_t)(hp >> 32), fracOf(hp));
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            out[i] += (main + (head - main) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
    template <typename T>
    inline void kernelStereoXfSinc(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint32_t idx = (uint32_t)(pos >> 32);
            const float frac = fracOf(pos);
            const uint64_t hp = pos + (uint64_t)headOffset;
            const uint32_t hidx = (uint32_t)(hp >> 32);
            const float hfrac = fracOf(hp);
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            const float m0 = sincAt(table, s0, idx, frac), m1 = sincAt(table, s1, idx, frac);
            const float h0 = sincAt(table, s0, hidx, hfrac), h1 = sincAt(table, s1, hidx, hfrac);
            out0[i] += (m0 + (h0 - m0) * mix) * g;
            out1[i] += (m1 + (h1 - m1) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
   #if ENGINE_HAS_AVX2_KERNELS
    template <typename T>
    ENGINE_AVX2_FN inline float avx2SincAt(const float (*table)[PlaySincTable::taps], const T* s, uint64_t p) noexcept
    {
        return avx2Sum8(_mm256_mul_ps(avx2SincTaps(s + (uint32_t)(p >> 32) - 3), avx2SincCoeffs(table, p)));
    }
    template <typename T>
    ENGINE_AVX2_FN inline void kernelMonoXfSincAvx2(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const float main = avx2SincAt(table, s, pos), head = avx2SincAt(table, s, pos + (uint64_t)headOffset);
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            out[i] += (main + (head - main) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
        _mm256_zeroupper();
    }
    template <typename T>
    ENGINE_AVX2_FN inline void kernelStereoXfSincAvx2(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint64_t hp = pos + (uint64_t)headOffset;
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            const float m0 = avx2SincAt(table, s0, pos), m1 = avx2SincAt(table, s1, pos);
            const float h0 = avx2SincAt(table, s0, hp), h1 = avx2SincAt(table, s1, hp);
            out0[i] += (m0 + (h0 - m0) * mix) * g;
            out1[i] += (m1 + (h1 - m1) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
        _mm256_zeroupper();
    }
   #endif
   #if ENGINE_HAS_LANE4_KERNELS
    template <typename T>
    inline float lane4SincAtPos(const float (*table)[PlaySincTable::taps], const T* s, uint64_t p) noexcept
    {
        Lane4::F lo, hi;
        lane4SincCoeffs(table, p, lo, hi);
        return lane4SincAt(lo, hi, s + (uint32_t)(p >> 32) - 3);
    }
    template <typename T>
    inline void kernelMonoXfSincLane4(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const float main = lane4SincAtPos(table, s, pos), head = lane4SincAtPos(table, s, pos + (uint64_t)headOffset);
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            out[i] += (main + (head - main) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
    template <typename T>
    inline void kernelStereoXfSincLane4(const T* s0, const T* s1, float* out0, float* out1, int n,
        uint64_t& pos, int64_t& inc, int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt) noexcept
    {
        const auto& table = playSincTable().h;
        for (int i = 0; i < n; ++i)
        {
            const uint64_t hp = pos + (uint64_t)headOffset;
            const float tt = juce::jlimit(0.0f, 1.0f, t);
            const float mix = tt * tt * (3.0f - 2.0f * tt);
            const float m0 = lane4SincAtPos(table, s0, pos), m1 = lane4SincAtPos(table, s1, pos);
            const float h0 = lane4SincAtPos(table, s0, hp), h1 = lane4SincAtPos(table, s1, hp);
            out0[i] += (m0 + (h0 - m0) * mix) * g;
            out1[i] += (m1 + (h1 - m1) * mix) * g;
            g += gStep; t += dt;
            pos += (uint64_t)inc;
            inc += incStep;
        }
    }
   #endif
    template <typename T>
    inline void dispatchMonoXfSinc(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt, KernelKind kernel) noexcept
    {
        if (kernel == KernelScalar) { kernelMonoXfSinc<T>(s, out, n, pos, inc, incStep, g, gStep, headOffset, t, dt); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelMonoXfSincAvx2<T>(s, out, n, pos, inc, incStep, g, gStep, headOffset, t, dt); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelMonoXfSincLane4<T>(s, out, n, pos, inc, incStep, g, gStep, headOffset, t, dt);
       #else
        kernelMonoXfSinc<T>(s, out, n, pos, inc, incStep, g, gStep, headOffset, t, dt);
       #endif
    }
    template <typename T>
    inline void dispatchStereoXfSinc(const T* s0, const T* s1, float* out0, float* out1, int n, uint64_t& pos,
        int64_t& inc, int64_t incStep, float g, float gStep, int64_t headOffset, float t, float dt, KernelKind kernel) noexcept
    {
        if (kernel == KernelScalar) { kernelStereoXfSinc<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep, headOffset, t, dt); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelStereoXfSincAvx2<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep, headOffset, t, dt); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelStereoXfSincLane4<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep, headOffset, t, dt);
       #else
        kernelStereoXfSinc<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep, headOffset, t, dt);
       #endif
    }
    template <typename T>
    inline void dispatchMonoSinc(const T* s, float* out, int n, uint64_t& pos, int64_t& inc,
        int64_t incStep, float g, float gStep, KernelKind kernel) noexcept
    {
        if (kernel == KernelScalar) { kernelMonoSinc<T>(s, out, n, pos, inc, incStep, g, gStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelMonoSincAvx2<T>(s, out, n, pos, inc, incStep, g, gStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelMonoSincLane4<T>(s, out, n, pos, inc, incStep, g, gStep);
       #else
        kernelMonoSinc<T>(s, out, n, pos, inc, incStep, g, gStep);
       #endif
    }
    template <typename T>
    inline void dispatchStereoSinc(const T* s0, const T* s1, float* out0, float* out1, int n, uint64_t& pos,
        int64_t& inc, int64_t incStep, float g, float gStep, KernelKind kernel) noexcept
    {
        if (kernel == KernelScalar) { kernelStereoSinc<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep); return; }
       #if ENGINE_HAS_AVX2_KERNELS
        if (kernel == KernelAvx2) { kernelStereoSincAvx2<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep); return; }
       #endif
       #if ENGINE_HAS_LANE4_KERNELS
        kernelStereoSincLane4<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep);
       #else
        kernelStereoSinc<T>(s0, s1, out0, out1, n, pos, inc, incStep, g, gStep);
       #endif
    }

    inline void startFade(Voice& v, float ms, double rate, bool kill) noexcept
    {
        const uint32_t frames = (uint32_t)juce::jmax(1.0, ms * 0.001 * rate);
        if (v.fadingOut)
        {
            // A faster fade replaces a slower one, preserving the current level.
            if (frames < v.fadeOutRemaining)
            {
                const float level = v.fadeOutTotal > 0 ? (float)v.fadeOutRemaining / (float)v.fadeOutTotal : 1.0f;
                v.fadeOutRemaining = frames;
                v.fadeOutTotal = (uint32_t)juce::jmax(1.0f, (float)frames / juce::jmax(0.001f, level));
            }
        }
        else
        {
            v.fadingOut = true;
            v.fadeOutRemaining = frames;
            v.fadeOutTotal = frames;
        }
        v.killing = v.killing || kill;
    }

    inline void splitShelf(const float* in, float* out, float* wet, int n, float k0, float k1) noexcept;   // defined below

    // shelf0/shelf1 >= 0: the dynamic tone shelf's mix at the start and the end
    // of the block - the kernels then put (1 - k) of the voice into the outputs
    // and k into the lanes' wet buffers themselves (see LaneOutputs); -1 = none.
    template <typename T>
    void renderVoiceSpans(Voice& v, const Sample& s, const BlockContext& c, const LaneOutputs& lanes,
        int64_t incStartF, int64_t incEndF, float gainStart, float gainEnd, float shelf0 = -1.0f, float shelf1 = -1.0f)
    {
        const int n = c.numSamples;
        const int sampleChans = s.numChannels;
        // Output routing: notes are spread over the routed channel pairs.
        float* out0 = lanes.sink;
        float* out1 = lanes.sink;
        float* wet0 = lanes.sink;
        float* wet1 = lanes.sink;
        int firstSlot = 0;
        if (c.outCount > 0)
        {
            firstSlot = ((int)v.note * sampleChans) % c.outCount;
            const int ch0 = c.outChannels[firstSlot];
            if (ch0 >= 0 && ch0 < lanes.numOuts) { out0 = lanes.out[ch0]; if (lanes.wet[ch0] != nullptr) wet0 = lanes.wet[ch0]; }
            if (sampleChans > 1)
            {
                const int ch1 = c.outChannels[(firstSlot + 1) % c.outCount];
                if (ch1 >= 0 && ch1 < lanes.numOuts) { out1 = lanes.out[ch1]; if (lanes.wet[ch1] != nullptr) wet1 = lanes.wet[ch1]; }
            }
        }
        const bool shelfActive = shelf0 >= 0.0f && sampleChans <= 2;
        alignas(32) static thread_local float spanTmp0[kMaxBlockFrames];   // a span of a splice or cubic kernel under the shelf
        alignas(32) static thread_local float spanTmp1[kMaxBlockFrames];
        const T* src0 = s.storesFloat ? (const T*)s.channel32(0) : (const T*)s.channel16(0);
        const T* src1 = sampleChans > 1 ? (s.storesFloat ? (const T*)s.channel32(1) : (const T*)s.channel16(1)) : src0;
        constexpr float scale = SampleScale<T>::value;
        // A packed sample (Sample::compress16, 0.8.145): every span is rendered
        // from a window decoded into the thread's scratch, the positions made
        // relative to the window's first frame (posBase); the splice head goes
        // into the second half of the scratch, kPackedWindow frames on.
        constexpr bool packable = std::is_same<T, int16_t>::value || std::is_same<T, float>::value;   // float since 0.8.183 (Sample::compress32)
        const bool packedSample = packable && s.compressed;
        uint64_t posBase = 0;

        const int64_t incStepF = n > 0 ? (incEndF - incStartF) / n : 0;
        const uint64_t endPosF = (uint64_t)(s.numFrames - 3) << 32;
        // A sample stored below the route's rate (releases at half the rate, 36/32 kHz
        // storage): its first 1.3 s through the windowed-sinc kernel (0.8.147), the
        // rest - and every loop splice and cubic (filled pipe) span - as before.
        // Only a HALVED sample (releases at half the rate): the 36/32 kHz storage is
        // 0.75/0.67 of the rate and the user took the linear kernel's result there;
        // 0.8.147 ran every 32 kHz attack through the scalar sinc (13 ns a frame
        // against 1.1) and Doesburg fell from 45 000 to 16 000 voices (#61).
        const bool lowRate = s.sincHead && s.sampleRate < c.routeRate * 0.6;
        const uint64_t sincEndF = lowRate ? (uint64_t)(1.3 * s.sampleRate) << 32 : 0;
        const bool hasLoops = v.loopEnabled && !s.loops.empty();
        // A sustained sample without loop points plays as one big loop: it fades
        // out over its last 50 ms (no click at the end), starts again from the
        // beginning with a short fade-in and keeps going while the key is held.
        const bool wholeFileLoop = v.loopEnabled && s.loops.empty();
        const uint64_t fadeLenF = wholeFileLoop ? ((uint64_t)juce::jmax(1.0, 0.05 * s.sampleRate) << 32) : 0;
        const uint64_t fadeStartF = endPosF > fadeLenF ? endPosF - fadeLenF : 0;

        const bool prof = gVoiceProfiling.load(std::memory_order_relaxed);
        int produced = 0;
        int guard = 0;
        while (produced < n)
        {
            if (++guard > 4096) { v.finished = true; return; }   // absolute safety net
            const int remaining = n - produced;
            int span = remaining;

            // --- envelope breakpoints
            if (v.fadeInRemaining > 0) span = juce::jmin(span, (int)v.fadeInRemaining);
            if (v.fadingOut) span = juce::jmin(span, (int)juce::jmax(1u, v.fadeOutRemaining));
            if (v.attackActive)
            {
                if (v.attackElapsed < v.attackHold) span = juce::jmin(span, (int)(v.attackHold - v.attackElapsed));
                else if (v.attackElapsed < v.attackHold + v.attackFade)
                    span = juce::jmin(span, (int)(v.attackHold + v.attackFade - v.attackElapsed));
                else v.attackActive = false;
            }
            if (span < 1) span = 1;

            // --- position events
            uint64_t eventPos = endPosF;
            int eventKind = 0;   // 0 sample end, 1 crossfade start, 2 loop end (in the splice), 4 long splice after a jump, 5 loop end without a splice
            uint64_t loopLenF = 0;
            float xfT = 0.0f, xfDt = 0.0f;
            if (v.tailBlend && v.pos >= v.tailBlendEndF) v.tailBlend = false;
            if (v.tailBlend)
            {
                // Inside the long splice after a jump: nothing else can happen before it ends
                // (it is shorter than the loop).
                eventPos = v.tailBlendEndF;
                eventKind = 4;
                const double lenFrames = (double)(v.tailBlendLenF >> 32);
                xfT = (float)((double)(int64_t)(v.pos - (v.tailBlendEndF - v.tailBlendLenF)) / kFixedOne / juce::jmax(1.0, lenFrames));
                xfDt = (float)((double)(incStartF + incStepF * produced) / kFixedOne / juce::jmax(1.0, lenFrames));
            }
            else if (hasLoops)
            {
                if (v.targetLoop < 0)
                {
                    const int numLoops = (int)s.loopOrderByEnd.size();
                    v.nextLoopOrder = juce::jlimit(0, numLoops, v.nextLoopOrder);
                    while (v.nextLoopOrder < numLoops
                        && ((uint64_t)s.loops[(size_t)s.loopOrderByEnd[(size_t)v.nextLoopOrder]].end << 32) <= v.pos)
                        ++v.nextLoopOrder;
                    for (int order = v.nextLoopOrder; order < numLoops; ++order)
                    {
                        const int li = s.loopOrderByEnd[(size_t)order];
                        const uint64_t bit = 1ull << (li & 63);
                        if ((v.loopDecided & bit) == 0)
                        {
                            v.rng = v.rng * 1664525u + 1013904223u;
                            const bool take = s.loops[(size_t)li].end == s.maxLoopEnd || ((v.rng >> 31) & 1u) != 0;
                            v.loopDecided |= bit;
                            if (take) v.loopTake |= bit;
                        }
                        if ((v.loopTake & bit) != 0) { v.targetLoop = li; break; }
                    }
                    v.inCrossfade = false;
                }
                if (v.targetLoop >= 0)
                {
                    const auto& L = s.loops[(size_t)v.targetLoop];
                    const int64_t xf = s.crossfadeFrames[(size_t)v.targetLoop];
                    const uint64_t endF = (uint64_t)L.end << 32;
                    const uint64_t xfStartF = (uint64_t)(L.end - xf) << 32;
                    loopLenF = (uint64_t)(L.end - L.start) << 32;
                    const bool afterJump = (size_t)v.targetLoop < s.crossfadeAfterJump.size() && s.crossfadeAfterJump[(size_t)v.targetLoop] != 0;
                    if (afterJump) { eventPos = endF; eventKind = 5; v.inCrossfade = false; }   // plain up to the end, the splice follows the jump
                    else if (v.pos < xfStartF) { eventPos = xfStartF; eventKind = 1; v.inCrossfade = false; }
                    else
                    {
                        // pos >= xfStartF here, so the head read (pos - loopLen >= start - xf >= 0)
                        // can never underflow.
                        v.inCrossfade = true;
                        eventPos = endF;
                        eventKind = 2;
                        const double xfFrames = (double)xf;
                        xfT = (float)((double)(int64_t)(v.pos - xfStartF) / kFixedOne / xfFrames);
                        xfDt = (float)((double)(incStartF + incStepF * produced) / kFixedOne / xfFrames);
                    }
                }
            }

            if (wholeFileLoop && v.pos < fadeStartF) { eventPos = fadeStartF; eventKind = 3; }   // the end fade begins here
            const int64_t inc0 = incStartF + incStepF * produced;
            const int64_t inc1 = incStartF + incStepF * (produced + span);
            const int64_t maxInc = juce::jmax<int64_t>(1, juce::jmax(inc0, inc1));
            const uint64_t dist = eventPos > v.pos ? (uint64_t)((eventPos - v.pos) / (uint64_t)maxInc) : 0;
            if (dist == 0)
            {
                if (eventKind == 1 || eventKind == 3)
                {
                    // Less than one sample before the splice (or the end fade) begins:
                    // play one plain sample, the next iteration enters it.
                    span = 1;
                    eventKind = 0;
                }
                else if (eventKind == 4)
                {
                    if (v.pos >= eventPos) { v.tailBlend = false; continue; }   // the long splice is over
                }
                else if (eventKind == 2 || eventKind == 5)
                {
                    if (v.pos >= eventPos)
                    {
                        const auto& L = s.loops[(size_t)v.targetLoop];
                        v.pos = v.pos - eventPos + ((uint64_t)L.start << 32);
                        if (eventKind == 5)
                        {
                            // The long splice: from here, the continuation past the loop end
                            // (one loop length ahead) fades into the loop over crossfadeFrames.
                            v.tailBlend = true;
                            v.tailLoopLenF = loopLenF;
                            v.tailBlendLenF = (uint64_t)s.crossfadeFrames[(size_t)v.targetLoop] << 32;
                            v.tailBlendEndF = ((uint64_t)L.start << 32) + v.tailBlendLenF;
                        }
                        v.loopDecided = v.loopTake = 0;
                        v.targetLoop = -1;
                        v.inCrossfade = false;
                        v.nextLoopOrder = 0;
                        continue;
                    }
                }
                else if (v.pos >= eventPos)
                {
                    if (hasLoops)
                    {
                        v.pos = (uint64_t)s.loops[(size_t)s.loopOrderByEnd.back()].start << 32;
                        v.loopDecided = v.loopTake = 0;
                        v.targetLoop = -1;
                        v.inCrossfade = false;
                        v.nextLoopOrder = 0;
                        continue;
                    }
                    if (wholeFileLoop)
                    {
                        // Faded out completely: start the file again with a 5 ms fade-in.
                        v.pos = 0;
                        v.fadeInTotal = v.fadeInRemaining = (uint32_t)juce::jmax(1.0, 0.005 * c.routeRate);
                        continue;
                    }
                    v.finished = true;
                    return;
                }
                span = 1;
            }
            else span = (int)std::min<uint64_t>((uint64_t)span, dist);
            if (packedSample)
            {
                // The span must fit the decode window (fast playback of a transposed pipe).
                const uint64_t spanFrames = ((uint64_t)span * (uint64_t)maxInc) >> 32;
                if (spanFrames + 2 > (uint64_t)kPackedSpanFrames)
                    span = (int)juce::jmax<uint64_t>(1, ((uint64_t)kPackedSpanFrames << 32) / (uint64_t)maxInc);
            }

            // --- envelope values at span start/end
            float e0 = 1.0f, e1 = 1.0f;
            if (v.fadeInRemaining > 0 && v.fadeInTotal > 0)
            {
                const float inv = 1.0f / (float)v.fadeInTotal;
                e0 *= 1.0f - (float)v.fadeInRemaining * inv;
                e1 *= 1.0f - (float)(v.fadeInRemaining - (uint32_t)span) * inv;
            }
            if (v.fadingOut && v.fadeOutTotal > 0)
            {
                const float inv = 1.0f / (float)v.fadeOutTotal;
                e0 *= (float)v.fadeOutRemaining * inv;
                e1 *= (float)(v.fadeOutRemaining - (uint32_t)span) * inv;
            }
            if (wholeFileLoop && fadeLenF > 0 && v.pos >= fadeStartF)
            {
                // The last 50 ms of the file: linear fade to silence at its end.
                auto factorAt = [&](uint64_t p)
                {
                    if (p >= endPosF) return 0.0f;
                    if (p <= fadeStartF) return 1.0f;
                    return (float)((double)(endPosF - p) / (double)fadeLenF);
                };
                e0 *= factorAt(v.pos);
                e1 *= factorAt(v.pos + (uint64_t)span * (uint64_t)juce::jmax<int64_t>(1, inc0));
            }
            if (v.attackActive)
            {
                if (v.attackElapsed < v.attackHold) { e0 *= v.attackBoost; e1 *= v.attackBoost; }
                else if (v.attackFade > 0)
                {
                    const float inv = 1.0f / (float)v.attackFade;
                    const float t0 = (float)(v.attackElapsed - v.attackHold) * inv;
                    const float t1 = (float)(v.attackElapsed + (uint32_t)span - v.attackHold) * inv;
                    e0 *= v.attackBoost + (1.0f - v.attackBoost) * t0;
                    e1 *= v.attackBoost + (1.0f - v.attackBoost) * t1;
                }
            }
            const float blockFrac0 = (float)produced / (float)n;
            const float blockFrac1 = (float)(produced + span) / (float)n;
            const float g0 = (gainStart + (gainEnd - gainStart) * blockFrac0) * e0 * scale;
            const float g1 = (gainStart + (gainEnd - gainStart) * blockFrac1) * e1 * scale;
            const float gStep = (g1 - g0) / (float)span;

            int64_t inc = inc0;
            const KernelKind kernel = (KernelKind)c.kernel;
            const int64_t kernelStart = prof ? profileTicks() : 0;
            // The splice kernels: before the loop end they blend towards the head a
            // loop length back (mix 0 -> 1); after a jump they blend from the
            // continuation a loop length ahead into the loop (mix 1 -> 0).
            const bool blend = eventKind == 2 || eventKind == 4;
            int64_t headOffset = eventKind == 4 ? (int64_t)v.tailLoopLenF : -(int64_t)loopLenF;
            const float bt = eventKind == 4 ? 1.0f - xfT : xfT;
            const float bdt = eventKind == 4 ? -xfDt : xfDt;
            if constexpr (packable)
            {
                if (packedSample && sampleChans <= 2)
                {
                    // The window this span reads: from four frames before the position
                    // (the cubic kernel's idx - 1) to the span's end plus the SIMD guard;
                    // the rest of the block ahead is decoded too when it fits, so the
                    // next spans of this block find it there.
                    const int64_t posFrame = (int64_t)(v.pos >> 32);
                    const int64_t spanFrames = (int64_t)(((uint64_t)span * (uint64_t)maxInc) >> 32) + 2;
                    const int64_t blockFrames = (int64_t)(((uint64_t)remaining * (uint64_t)maxInc) >> 32) + 2;
                    const int64_t from = posFrame - 4;
                    const int64_t toSpan = posFrame + spanFrames + kPackedGuard;
                    const int64_t toBlock = posFrame + blockFrames + kPackedGuard;
                    const int64_t to = toBlock - from <= kPackedWindow ? toBlock : juce::jmin(toSpan, from + kPackedWindow);
                    const int64_t headFrames = blend ? headOffset / ((int64_t)1 << 32) : 0;   // whole frames (a loop length)
                    auto& win = tlPackedWindow;
                    const bool haveMain = win.sample == &s && win.from <= from && win.to >= to && win.channels >= sampleChans;
                    const int64_t unpackStart = prof ? profileTicks() : 0;
                    if (!haveMain)
                    {
                        for (int ch = 0; ch < sampleChans; ++ch) decodeWindow<T>(s, ch, from, to, win.region<T>(ch, 0));
                        win.sample = &s; win.from = from; win.to = to; win.channels = sampleChans;
                        win.headValid = false;
                    }
                    if (blend && (!win.headValid || win.headFrames != headFrames))
                    {
                        // The head region: the window a loop length back (or ahead), frame
                        // k of it pairing with frame k of the main region.
                        for (int ch = 0; ch < sampleChans; ++ch) decodeWindow<T>(s, ch, win.from + headFrames, win.to + headFrames, win.region<T>(ch, 1));
                        win.headValid = true; win.headFrames = headFrames;
                    }
                    if (prof) tlVoiceProfile.unpack += profileDelta(unpackStart, profileTicks());
                    posBase = (uint64_t)win.from << 32;
                    src0 = win.region<T>(0, 0);
                    src1 = sampleChans > 1 ? win.region<T>(1, 0) : src0;
                    if (blend) headOffset = (int64_t)kPackedWindow << 32;   // the head region lies kPackedWindow frames on
                }
            }
            uint64_t pos = v.pos - posBase;
            // The dynamic tone shelf over this span: its mix at the span's start
            // and end (the block's ramp cut to the span). The plain kernels fold
            // it in; a splice or cubic kernel renders the span into a private
            // buffer first and splits it afterwards (rare: filled pipes, loop
            // splices - a few per cent of the spans).
            const float k0 = shelfActive ? shelf0 + (shelf1 - shelf0) * blockFrac0 : 0.0f;
            const float k1 = shelfActive ? shelf0 + (shelf1 - shelf0) * blockFrac1 : 0.0f;
            const float kStep = shelfActive ? (k1 - k0) / (float)span : 0.0f;
            // The sinc: the head of a low-rate sample (0.8.147), or the whole of a high
            // pipe's (0.8.164, fullKernel); the taps reach idx - 3 .. idx + 4 (the guard
            // frames of every sample cover the end).
            const bool sinc = ((lowRate && v.pos < sincEndF) || v.fullKernel) && !blend && !v.cubic && sampleChans <= 2 && (v.pos >> 32) >= 4;
            // A high pipe's loop splice through the sinc too (both reads need 3 frames before them).
            const bool sincXf = blend && v.fullKernel && !v.cubic && sampleChans <= 2 && (v.pos >> 32) >= 4
                && ((v.pos + (uint64_t)headOffset) >> 32) >= 4;
            const bool viaTmp = shelfActive && (blend || v.cubic || sinc);
            float* o0 = out0 + produced;
            float* o1 = out1 + produced;
            if (viaTmp)
            {
                juce::FloatVectorOperations::clear(spanTmp0, span); o0 = spanTmp0;
                if (sampleChans == 2) { juce::FloatVectorOperations::clear(spanTmp1, span); o1 = spanTmp1; }
            }
            if (sinc)
            {
                if (sampleChans == 1) dispatchMonoSinc<T>(src0, o0, span, pos, inc, incStepF, g0, gStep, kernel);
                else dispatchStereoSinc<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep, kernel);
                v.pos = pos + posBase;
            }
            else if (v.cubic && sampleChans <= 2)
            {
                // A filled pipe resampled by semitones: cubic interpolation.
                if (sampleChans == 1)
                {
                    if (blend) kernelMonoXfCubic<T>(src0, o0, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt);
                    else kernelMonoCubic<T>(src0, o0, span, pos, inc, incStepF, g0, gStep);
                }
                else
                {
                    if (blend) kernelStereoXfCubic<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt);
                    else kernelStereoCubic<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep);
                }
                v.pos = pos + posBase;
            }
            else if (sampleChans == 1)
            {
                if (blend && sincXf) dispatchMonoXfSinc<T>(src0, o0, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt, kernel);
                else if (blend) kernelMonoXf<T>(src0, o0, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt);
                else if (shelfActive) dispatchMonoShelf<T>(src0, o0, wet0 + produced, span, pos, inc, incStepF, g0, gStep, k0, kStep, kernel);
                else dispatchMono<T>(src0, o0, span, pos, inc, incStepF, g0, gStep, kernel);
                v.pos = pos + posBase;
            }
            else if (sampleChans == 2)
            {
                if (blend && sincXf) dispatchStereoXfSinc<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt, kernel);
                else if (blend) kernelStereoXf<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep, headOffset, bt, bdt);
                else if (shelfActive) dispatchStereoShelf<T>(src0, src1, o0, o1, wet0 + produced, wet1 + produced, span, pos, inc, incStepF, g0, gStep, k0, kStep, kernel);
                else dispatchStereo<T>(src0, src1, o0, o1, span, pos, inc, incStepF, g0, gStep, kernel);
                v.pos = pos + posBase;
            }
            else
            {
                uint64_t finalPos = v.pos;
                for (int ch = 0; ch < sampleChans; ++ch)
                {
                    uint64_t p = v.pos;
                    int64_t ic = inc0;
                    const T* src = s.storesFloat ? (const T*)s.channel32(ch) : (const T*)s.channel16(ch);
                    int64_t head = headOffset;
                    if constexpr (packable)
                    {
                        if (packedSample)
                        {
                            // More than two channels (rare): the window of this channel decoded
                            // on the spot, the shared window cache left invalid.
                            const int64_t posFrame = (int64_t)(v.pos >> 32);
                            const int64_t from = posFrame - 4;
                            const int64_t to = juce::jmin(from + (int64_t)kPackedWindow, posFrame + (int64_t)(((uint64_t)span * (uint64_t)maxInc) >> 32) + 2 + kPackedGuard);
                            auto& win = tlPackedWindow;
                            win.sample = nullptr; win.headValid = false;
                            decodeWindow<T>(s, ch, from, to, win.region<T>(0, 0));
                            if (blend)
                            {
                                const int64_t headFrames = headOffset / ((int64_t)1 << 32);
                                decodeWindow<T>(s, ch, from + headFrames, to + headFrames, win.region<T>(0, 1));
                                head = (int64_t)kPackedWindow << 32;
                            }
                            src = win.region<T>(0, 0);
                            p = v.pos - ((uint64_t)from << 32);
                        }
                    }
                    float* out = lanes.sink;
                    if (c.outCount > 0)
                    {
                        const int och = c.outChannels[(firstSlot + ch) % c.outCount];
                        out = (och >= 0 && och < lanes.numOuts) ? lanes.out[och] : lanes.sink;
                    }
                    if (blend) kernelMonoXf<T>(src, out + produced, span, p, ic, incStepF, g0, gStep, head, bt, bdt);
                    else dispatchMono<T>(src, out + produced, span, p, ic, incStepF, g0, gStep, kernel);
                    finalPos = packedSample ? p + ((uint64_t)((int64_t)(v.pos >> 32) - 4) << 32) : p;
                }
                v.pos = finalPos;
            }
            if (viaTmp)
            {
                // The span rendered into the private buffer: split it now (k ramping over the span).
                splitShelf(spanTmp0, out0 + produced, wet0 + produced, span, k0, k1);
                if (sampleChans == 2) splitShelf(spanTmp1, out1 + produced, wet1 + produced, span, k0, k1);
            }

            if (prof) tlVoiceProfile.kernel += profileDelta(kernelStart, profileTicks());

            // --- advance envelopes
            produced += span;
            if (v.fadeInRemaining > 0) v.fadeInRemaining -= juce::jmin(v.fadeInRemaining, (uint32_t)span);
            if (v.attackActive) v.attackElapsed += (uint32_t)span;
            if (v.fadingOut)
            {
                v.fadeOutRemaining -= juce::jmin(v.fadeOutRemaining, (uint32_t)span);
                if (v.fadeOutRemaining == 0) { v.finished = true; return; }
            }
        }
    }

    // Second-order Butterworth lowpass (Q = 1/sqrt 2), normalised b0 b1 b2 a1 a2.
    inline void butterworthLowpass(double sampleRate, double cutoffHz, float k[5]) noexcept
    {
        const double rate = juce::jmax(8000.0, sampleRate);
        const double w0 = juce::MathConstants<double>::twoPi * juce::jlimit(20.0, rate * 0.45, cutoffHz) / rate;
        const double cw = std::cos(w0), sw = std::sin(w0);
        const double alpha = sw / std::sqrt(2.0);   // sin / (2 Q), Q = 1/sqrt 2
        const double a0 = 1.0 + alpha;
        k[0] = (float)((1.0 - cw) * 0.5 / a0); k[1] = (float)((1.0 - cw) / a0); k[2] = k[0];
        k[3] = (float)(-2.0 * cw / a0); k[4] = (float)((1.0 - alpha) / a0);
    }

    // Two cascaded sections of the same biquad on one sample (transposed direct form II).
    inline float lowpass4(float x, const float* k, float* s) noexcept
    {
        const float y1 = k[0] * x + s[0];
        s[0] = k[1] * x - k[3] * y1 + s[1];
        s[1] = k[2] * x - k[4] * y1;
        const float y2 = k[0] * y1 + s[2];
        s[2] = k[1] * y1 - k[3] * y2 + s[3];
        s[3] = k[2] * y1 - k[4] * y2;
        return y2;
    }

    // Dynamic tone shelf of a voice: out += (1 - k) * x, wet += k * x, k ramping
    // k0 -> k1 over the block. The route adds lowpass4(wet) to out afterwards, so
    // the voice sounds as x - k * (x - lowpass4(x)): unity below the transition
    // band, (1 - k) above it (-3 dB for k = kShelfDepth).
    inline void splitShelf(const float* in, float* out, float* wet, int n, float k0, float k1) noexcept
    {
        float k = k0;
        const float kStep = n > 0 ? (k1 - k0) / (float)n : 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float x = in[i];
            const float w = k * x;
            out[i] += x - w;
            wet[i] += w;
            k += kStep;
        }
    }

    // The route's side of the dynamic tone shelf: out += lowpass4(wet) per
    // channel, with the channel's own filter state carried across the blocks.
    inline void applyShelfLowpass(const float* wet, float* out, int n, const float* lp, float* state) noexcept
    {
        for (int i = 0; i < n; ++i) out[i] += lowpass4(wet[i], lp, state);
    }

    // Per-key tone shelf (voicing "Treble per key"): one biquad in place,
    // transposed direct form II. k = b0 b1 b2 a1 a2 (normalised), s = two states.
    inline void applyBiquadInPlace(float* x, int n, const float* k, float* s) noexcept
    {
        float s1 = s[0], s2 = s[1];
        for (int i = 0; i < n; ++i)
        {
            const float in = x[i];
            const float y = k[0] * in + s1;
            s1 = k[1] * in - k[3] * y + s2;
            s2 = k[2] * in - k[4] * y;
            x[i] = y;
        }
        s[0] = s1;
        s[1] = s2;
    }

    // Magnitude (dB) of the fourth-order lowpass at a frequency: for the self-check.
    inline double lowpass4GainDb(const float* k, double sampleRate, double hz) noexcept
    {
        const std::complex<double> z = std::polar(1.0, -juce::MathConstants<double>::twoPi * hz / sampleRate);
        const std::complex<double> h = ((double)k[0] + (double)k[1] * z + (double)k[2] * z * z)
            / (1.0 + (double)k[3] * z + (double)k[4] * z * z);
        return 40.0 * std::log10(juce::jmax(1.0e-9, std::abs(h)));   // two sections
    }

    // Pull the first cache lines of a voice's sample data into L1 while the
    // previous voice is still rendering; the per-voice initial miss (random
    // access into gigabytes of samples) is the dominant memory cost.
    inline void prefetchLine(const void* p) noexcept
    {
       #if ENGINE_X86
        _mm_prefetch((const char*)p, _MM_HINT_T0);
       #elif defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(p, 0, 3);
       #else
        juce::ignoreUnused(p);
       #endif
    }

    // Pull in the sample data plus the stop gain lines the voice header reads.
    inline void prefetchVoice(const Voice& v) noexcept
    {
        const Sample* s = v.sample;
        if (s == nullptr) return;
        if (v.stop != nullptr)
        {
            prefetchLine(&v.stop->linearGain);
            prefetchLine(&v.stop->perNoteGain[v.note]);
        }
        if (s->compressed) return;   // packed: the window is decoded into the lane's scratch, nothing to pull in
        const uint32_t idx = (uint32_t)(v.pos >> 32);
        const int channels = juce::jmin(2, s->numChannels);
        for (int c = 0; c < channels; ++c)
        {
            if (s->storesFloat) prefetchSample(s->channel32(c), idx);
            else prefetchSample(s->channel16(c), idx);
        }
    }

    // The Voice record itself (four cache lines) two voices ahead, so the
    // following prefetchVoice() finds sample/stop pointers already in cache.
    inline void prefetchVoiceRecord(const Voice& v) noexcept
    {
        const char* p = (const char*)&v;
        prefetchLine(p);
        prefetchLine(p + 64);
        prefetchLine(p + 128);
        prefetchLine(p + 192);
    }

    // Returns false when the voice has finished.
    bool renderVoice(Voice& v, const BlockContext& c, const LaneOutputs& lanes) noexcept
    {
        const Sample* s = v.sample;
        if (s == nullptr || s->numFrames < 8 || v.stop == nullptr) { v.finished = true; return false; }
        const int n = c.numSamples;
        if (n <= 0) return true;
        const bool prof = gVoiceProfiling.load(std::memory_order_relaxed);
        const int64_t profStart = prof ? profileTicks() : 0;

        // --- commands from the control thread
        const uint8_t cmd = v.command.exchange(0, std::memory_order_acq_rel);
        if (cmd != 0)
        {
            if (cmd & CmdFreeze) { v.exprFrozen = true; v.frozenExpr = v.frozenExprRequested.load(std::memory_order_relaxed); }
            if (cmd & CmdResume) v.exprFrozen = false;
            if (cmd & CmdRelease)
            {
                startFade(v, v.releaseFadeMsRequested.load(std::memory_order_relaxed), c.routeRate, false);
                // The pipe stops taking wind: hold its wind pitch/level so the
                // fading sustain and the release sample stay matched.
                if (!v.windFrozen)
                {
                    v.windFrozen = true;
                    v.frozenWindPitch = v.curWindPitch.load(std::memory_order_relaxed);
                    v.frozenWindGain = v.lastWindGain;
                }
            }
            if (cmd & CmdSteal) startFade(v, 5.0f, c.routeRate, false);
            if (cmd & CmdKill) startFade(v, 2.0f, c.routeRate, true);
        }

        // --- end-of-block targets (interpolated linearly across the block)
        float randomCents = 0.0f;
        if (c.randomAllowed)
        {
            randomCents = v.randomPitchCents.load(std::memory_order_relaxed);
            if (v.couplingUsed && c.couplingAllowed && v.couplingTotalSamples > 0)
            {
                const float progress = juce::jlimit(0.0f, 1.0f,
                    (float)(v.couplingElapsed + (uint32_t)n) / (float)v.couplingTotalSamples);
                const float curved = 1.0f - fastSinTurns(progress * 0.25f + 0.25f);   // 1 - cos(pi/2 * p)
                randomCents += (v.coupledPitchCents - randomCents) * curved;
            }
        }
        if (v.couplingUsed) v.couplingElapsed = juce::jmin(v.couplingTotalSamples, v.couplingElapsed + (uint32_t)n);

        float windPitch = 0.0f, windGain = 1.0f, tremPitch = 0.0f, tremGain = 1.0f;
        if (v.detachedRelease) windGain = v.initialWindGain;
        else if (v.windFrozen) { windPitch = v.frozenWindPitch; windGain = v.frozenWindGain; }
        else
        {
            windPitch = c.windPitchCents[v.windPitchVar & (kWindVariations - 1)] * v.windKeyScale;
            windGain = c.windGain[v.windGainVar & (kWindVariations - 1)];
        }
        v.lastWindGain = windGain;
        if (c.tremActive)
        {
            const int b = v.tremBucket & (kTremBuckets - 1);
            if (v.detachedRelease) { tremPitch = c.tremPitchCentsRelease[b]; tremGain = c.tremGainRelease[b]; }
            else { tremPitch = c.tremPitchCents[b]; tremGain = c.tremGain[b]; }
        }
        // The temperament belongs to the key that is played (a transposed pipe
        // must sound as a pure harmonic of its key), the recorded deviation to the
        // pipe itself. Untransposed voices: keyClass == pitchClass, as before.
        const int pitchClass = v.note % 12;
        const int keyClass = v.keyNote % 12;
        const uint8_t mode = v.tempMode.load(std::memory_order_relaxed);
        float temperamentCents;
        if (mode == 1) temperamentCents = c.selectedCents[keyClass];
        else if (mode == 2 && v.rank != nullptr)
            temperamentCents = c.selectedCents[keyClass] - (v.rank->measuredTemperamentCents[(size_t)pitchClass]
                - v.rank->measuredTemperamentCents[9]);   // measured table referenced to A as well
        else
        {
            temperamentCents = c.pitchClassCents[pitchClass];
            if (keyClass != pitchClass) temperamentCents += c.selectedCents[keyClass] - c.selectedCents[pitchClass];
        }
        const float totalCents = temperamentCents + randomCents + windPitch + tremPitch
            + v.tuningCents.load(std::memory_order_relaxed) + v.shiftCents.load(std::memory_order_relaxed);
        const double incEnd = (s->sampleRate / juce::jmax(1.0, c.routeRate)) * (double)fastCentsToRatio(totalCents);
        // Fixed-point increment; ratios up to 32:1 (the old 4.0e9 ceiling capped
        // every upward retune and pulled 48 kHz samples ~120 cents flat).
        const int64_t incEndF = (int64_t)juce::jlimit(1.0, 32.0 * kFixedOne, incEnd * kFixedOne);

        const float expr = v.exprFrozen ? v.frozenExpr : c.exprGain;
        // Route volume: the loudest route holding the pipe. A change (that route
        // let go, another took over) glides linearly over the wind sag lag.
        {
            const float target = v.routeGainTarget.load(std::memory_order_relaxed);
            if (!v.started) { v.routeGain = v.routeGainLastTarget = target; v.routeGainRemaining = 0; }
            else if (target != v.routeGainLastTarget)
            {
                v.routeGainLastTarget = target;
                v.routeGainFrom = v.routeGain;
                v.routeGainTotal = v.routeGainRemaining =
                    (uint32_t)juce::jmax(1.0, c.routeGainLagMs * 0.001 * c.routeRate);
            }
            if (v.routeGainRemaining > 0)
            {
                const uint32_t step = juce::jmin(v.routeGainRemaining, (uint32_t)n);
                v.routeGainRemaining -= step;
                const float progress = 1.0f - (float)v.routeGainRemaining / (float)v.routeGainTotal;
                v.routeGain = v.routeGainFrom + (target - v.routeGainFrom) * progress;
            }
        }
        const float gainEnd = v.velocityGain * v.stop->getGain() * v.stop->getNoteGain((int)v.note)
            * expr * c.divisionGain * (v.detachedRelease ? c.releaseScale : 1.0f)
            * windGain * tremGain * c.outputGain * v.routeGain * v.fillGain;

        // --- per-key tone ("Treble per key"): a high shelf per voice. The setting
        // of the route with the highest tone holding the pipe (plus the treble
        // loss of an upward extension) is the target; the voice glides to it by
        // at most 0.5 dB per block.
        {
            const float target = v.toneTarget.load(std::memory_order_relaxed) + v.toneOffset + v.attackToneDb;
            if (!v.started) v.toneDb = target;
            else
            {
                // The attack shelf of a gentle key opens at its own pace (per frame), the rest glides at 0.5 dB per block.
                const float maxStep = juce::jmax(0.5f, v.attackToneStepPerFrame * (float)n);
                v.toneDb += juce::jlimit(-maxStep, maxStep, target - v.toneDb);
            }
            if (v.attackToneDb < 0.0f) v.attackToneDb = juce::jmin(0.0f, v.attackToneDb + v.attackToneStepPerFrame * (float)n);
        }
        const bool toneActive = std::abs(v.toneDb) > 0.005f;
        if (toneActive && (v.toneDb != v.toneCoeffDb || (float)c.routeRate != v.toneCoeffRate))
        {
            SamplerEngine::toneShelfCoefficients(c.routeRate, v.toneDb, v.toneStartHz, v.toneCoeff);
            v.toneCoeffDb = v.toneDb;
            v.toneCoeffRate = (float)c.routeRate;
        }
        if (!toneActive) v.toneState[0][0] = v.toneState[0][1] = v.toneState[1][0] = v.toneState[1][1] = 0.0f;

        // --- dynamic tone: the high shelf follows the level modulation. Each
        // modulator contributes the fraction of its own deepest attenuation it is
        // currently applying (swell closed = 1, tremulant at the bottom of its
        // dip = 1, wind at its sag limit = 1); the sum, clamped to 1, scales the
        // -3 dB shelf.
        float shelfTarget = 0.0f;
        if (c.shelfEnabled)
        {
            // Each modulator has its own depth; the three shares add up (capped at -12 dB).
            float mix = 0.0f;
            if (c.exprMaxDb > 0.01f && expr < 0.9999f)
                mix += c.shelfSwell * juce::jlimit(0.0f, 1.0f, 8.6859f * fastNegLn(juce::jmax(0.001f, expr)) / c.exprMaxDb);
            if (c.windMaxDb > 0.01f && windGain < 0.9999f)
                mix += c.shelfWind * juce::jlimit(0.0f, 1.0f, 8.6859f * fastNegLn(juce::jmax(0.001f, windGain)) / c.windMaxDb);
            if (c.tremActive)
            {
                const int b = v.tremBucket & (kTremBuckets - 1);
                mix += c.shelfTrem * (v.detachedRelease ? c.tremShelfRelease[b] : c.tremShelf[b]);
            }
            shelfTarget = juce::jmin(0.7488f, mix);   // 1 - 10^(-12/20)
        }
        // The tone lags the level by about 15 degrees (one-pole smoothing per block).
        if (v.started) v.shelfSmoothed += (shelfTarget - v.shelfSmoothed) * c.shelfSmoothing;
        else v.shelfSmoothed = shelfTarget;
        const float shelfEnd = v.shelfSmoothed;

        if (!v.started)
        {
            v.started = true;
            v.lastInc = incEndF;
            v.lastGain = gainEnd;
            v.lastShelf = shelfEnd;
            if (v.startFrozen) { v.exprFrozen = true; v.frozenExpr = v.initialFrozenExpr; }
        }
        const int64_t incStartF = v.lastInc;
        const float gainStart = v.lastGain;
        const float shelfStart = v.lastShelf;
        v.lastInc = incEndF;
        v.lastGain = gainEnd;
        v.lastShelf = shelfEnd;
        v.curRandomPitch.store(randomCents, std::memory_order_relaxed);
        v.curWindPitch.store(windPitch, std::memory_order_relaxed);
        v.curTremPitch.store(tremPitch, std::memory_order_relaxed);
        v.curWindGain.store(windGain, std::memory_order_relaxed);

        // A voice with an audible shelf (dynamic tone or per-key tone) renders
        // into private buffers first; the filtered result is then added to the
        // lane outputs. Without one the kernels accumulate straight into the
        // lanes as before (no extra cost).
        const int64_t profHeaderEnd = prof ? profileTicks() : 0;
        const int64_t profKernelBefore = prof ? tlVoiceProfile.kernel : 0;
        const int64_t profUnpackBefore = prof ? tlVoiceProfile.unpack : 0;
        if (prof) tlVoiceProfile.header += profileDelta(profStart, profHeaderEnd);
        const int sampleChans = s->numChannels;
        const bool dynamicShelf = shelfStart > 0.002f || shelfEnd > 0.002f;
        // The dynamic shelf alone is folded into the kernels (renderVoiceSpans);
        // only the per-key tone biquad still needs the voice in a private buffer.
        int ch0 = -1, ch1 = -1;
        if (toneActive && sampleChans <= 2 && c.outCount > 0)
        {
            const int firstSlot = ((int)v.note * sampleChans) % c.outCount;
            ch0 = c.outChannels[firstSlot];
            if (ch0 < 0 || ch0 >= lanes.numOuts) ch0 = -1;
            if (sampleChans > 1)
            {
                ch1 = c.outChannels[(firstSlot + 1) % c.outCount];
                if (ch1 < 0 || ch1 >= lanes.numOuts || ch1 == ch0) ch1 = -1;
            }
        }
        if (ch0 < 0 && ch1 < 0)
        {
            const float shelf0 = dynamicShelf && sampleChans <= 2 ? shelfStart : -1.0f;
            const float shelf1 = dynamicShelf && sampleChans <= 2 ? shelfEnd : -1.0f;
            if (s->storesFloat) renderVoiceSpans<float>(v, *s, c, lanes, incStartF, incEndF, gainStart, gainEnd, shelf0, shelf1);
            else renderVoiceSpans<int16_t>(v, *s, c, lanes, incStartF, incEndF, gainStart, gainEnd, shelf0, shelf1);
            if (prof)
            {
                tlVoiceProfile.spans += juce::jmax<int64_t>(0, profileDelta(profHeaderEnd, profileTicks()) - (tlVoiceProfile.kernel - profKernelBefore) - (tlVoiceProfile.unpack - profUnpackBefore));
                ++tlVoiceProfile.voices;
            }
            return !v.finished;
        }
        alignas(32) static thread_local float tmp0[kMaxBlockFrames];   // the attribute first: GCC rejects it between decl-specifiers
        alignas(32) static thread_local float tmp1[kMaxBlockFrames];
        LaneOutputs filtered = lanes;
        if (ch0 >= 0) { juce::FloatVectorOperations::clear(tmp0, n); filtered.out[ch0] = tmp0; }
        if (ch1 >= 0) { juce::FloatVectorOperations::clear(tmp1, n); filtered.out[ch1] = tmp1; }
        if (s->storesFloat) renderVoiceSpans<float>(v, *s, c, filtered, incStartF, incEndF, gainStart, gainEnd);
        else renderVoiceSpans<int16_t>(v, *s, c, filtered, incStartF, incEndF, gainStart, gainEnd);
        const int64_t profSpansEnd = prof ? profileTicks() : 0;
        if (prof) tlVoiceProfile.spans += juce::jmax<int64_t>(0, profileDelta(profHeaderEnd, profSpansEnd) - (tlVoiceProfile.kernel - profKernelBefore) - (tlVoiceProfile.unpack - profUnpackBefore));
        if (toneActive)
        {
            if (ch0 >= 0) applyBiquadInPlace(tmp0, n, v.toneCoeff, v.toneState[0]);
            if (ch1 >= 0) applyBiquadInPlace(tmp1, n, v.toneCoeff, v.toneState[1]);
        }
        if (!dynamicShelf)
        {
            // Only the per-key tone: add the filtered voice to the lanes.
            if (ch0 >= 0) juce::FloatVectorOperations::add(lanes.out[ch0], tmp0, n);
            if (ch1 >= 0) juce::FloatVectorOperations::add(lanes.out[ch1], tmp1, n);
            if (prof) { tlVoiceProfile.post += profileDelta(profSpansEnd, profileTicks()); ++tlVoiceProfile.voices; }
            return !v.finished;
        }
        // The dynamic shelf: the voice only splits its signal; the lane's wet
        // buffers are low-passed once per block by the route (see LaneOutputs).
        if (ch0 >= 0) splitShelf(tmp0, lanes.out[ch0], lanes.wet[ch0] != nullptr ? lanes.wet[ch0] : lanes.sink, n, shelfStart, shelfEnd);
        if (ch1 >= 0) splitShelf(tmp1, lanes.out[ch1], lanes.wet[ch1] != nullptr ? lanes.wet[ch1] : lanes.sink, n, shelfStart, shelfEnd);
        if (prof) { tlVoiceProfile.post += profileDelta(profSpansEnd, profileTicks()); ++tlVoiceProfile.voices; }
        return !v.finished;
    }

    //==========================================================================
    // Persistent render worker pool shared by all device callbacks.
    //==========================================================================
    class RenderPool
    {
    public:
        static constexpr int kScratchChannels = 64;
        static constexpr int kFrames = kMaxBlockFrames;
        static constexpr int kMaxJobs = 64;
        static constexpr int kMaxLanesPerJob = 128;

        explicit RenderPool(int workerCount)
        {
            numWorkers = juce::jlimit(0, 62, workerCount);
            scratchStore.resize((size_t)numWorkers + 2);
            for (auto& s : scratchStore) s.assign((size_t)2 * kScratchChannels * kFrames, 0.0f);   // the outputs, then the wet shares
            for (int i = 0; i < (int)scratchStore.size(); ++i) scratchFree.push_back(i);
            for (int i = 0; i < numWorkers; ++i)
                threads.emplace_back([this, i] { workerLoop(i); });
        }

        ~RenderPool()
        {
            shutdown.store(true);
            {
                std::lock_guard<std::mutex> lk(sleepMutex);
                sleepCv.notify_all();
            }
            for (auto& t : threads) if (t.joinable()) t.join();
        }

        int getWorkerCount() const noexcept { return numWorkers; }

        // Render voices [0..num) of the given list. The calling thread is a lane
        // and writes directly into outs; other lanes render into scratches which
        // are summed into outs afterwards.
        // otherAudioThreads: device callback threads besides the calling one that
        // rendered recently (Windows Audio routes, one thread each). They need
        // CPUs of their own: the workers then neither spin across the gaps nor
        // take every core, or those threads miss their periods (gaps of 30-40 ms
        // on every route were seen with one ASIO route next to twelve WASAPI routes).
        // wets: the route's harmonic-filter input buffers per channel (cleared by
        // the caller); every lane's wet shares are summed into them like the outputs.
        void render(const uint32_t* voiceIndices, int num, Voice* pool, const BlockContext& ctx,
            float* const* outs, float* const* wets, int numOuts, float* sink, int laneLimit = 0, int otherAudioThreads = 0)
        {
            if (num <= 0) return;
            LaneOutputs mine;
            mine.numOuts = juce::jmin(numOuts, kScratchChannels);
            for (int c = 0; c < mine.numOuts; ++c) { mine.out[c] = outs[c]; mine.wet[c] = wets[c]; }
            mine.sink = sink;

            const bool prof = gVoiceProfiling.load(std::memory_order_relaxed);
            const int64_t profEntry = prof ? profileTicks() : 0;
            Job* job = numWorkers > 0 && num >= 24 && laneLimit != 1 ? acquireJob() : nullptr;
            if (job == nullptr)
            {
                renderRange(voiceIndices, 0, num, pool, ctx, mine);
                if (prof) tlPoolOwnTicks += profileDelta(profEntry, profileTicks());
                return;
            }

            job->voices = voiceIndices;
            job->numVoices = num;
            job->pool = pool;
            job->ctx = &ctx;
            job->maxWorkers = laneLimit > 1 ? laneLimit - 1
                : (otherAudioThreads > 0 ? juce::jmax(1, numWorkers - otherAudioThreads) : 0);   // 0 = no limit
            job->lanesHint = numWorkers + 1;
            job->completedCount.store(0, std::memory_order_relaxed);
            job->laneBusyTicks.store(0, std::memory_order_relaxed);
            // The lane counters are NEVER reset (0.8.226): they count on across the
            // uses of a job and only their equality matters. A reset to 0 erased a
            // join that landed between the stores (a worker that had passed the
            // open/nextVoice checks of the previous use and was preempted): its
            // leave then stood against no join, the poster saw equality one lane
            // early, that lane's scratch was never merged (a block with voices
            // missing, a click at both block edges) and leaked. joinBase is the
            // count at this opening: the lane limit and the statistics count from it.
            job->joinBase = job->lanesJoined.load(std::memory_order_relaxed);
            job->nextVoice.store(0, std::memory_order_relaxed);
            const int64_t jobStart = juce::Time::getHighResolutionTicks();
            job->openedTicks = jobStart;
            job->open.store(true, std::memory_order_release);
            // Keep the workers spinning across the gap to the next audio period.
            // The period is learned from the job cadence, so small ASIO buffers
            // never pay a thread wake-up; when sound stops, workers go to sleep.
            {
                const int64_t now = juce::Time::getHighResolutionTicks();
                const int64_t last = lastJobTicks.exchange(now, std::memory_order_relaxed);
                if (last > 0)
                {
                    const int64_t interval = now - last;
                    const int64_t maxInterval = juce::Time::getHighResolutionTicksPerSecond() / 50;   // 20 ms
                    if (interval > 0 && interval < maxInterval)
                    {
                        const int64_t previous = periodTicks.load(std::memory_order_relaxed);
                        periodTicks.store(previous > 0 ? (previous * 7 + interval) / 8 : interval, std::memory_order_relaxed);
                    }
                }
                // The DEVICE period (0.8.144): a job after a gap of more than 0.3 ms starts a
                // new period; the interval between such starts is what the wake-ahead
                // counts with. The job cadence above is the gap between the routes of one
                // callback when a device carries several - 0.8.143 took it for the period
                // and woke the workers every few hundred microseconds, which cost a third
                // of the polyphony on the two-core laptops and four fifths on a notebook
                // with eight routes (#48-#50).
                if (last <= 0 || now - last > juce::Time::getHighResolutionTicksPerSecond() * 3 / 10000)
                {
                    const int64_t start = periodStartTicks.load(std::memory_order_relaxed);
                    if (start > 0 && now - start < juce::Time::getHighResolutionTicksPerSecond() / 20)
                    {
                        const int64_t previous = devicePeriodTicks.load(std::memory_order_relaxed);
                        devicePeriodTicks.store(previous > 0 ? (previous * 7 + (now - start)) / 8 : now - start, std::memory_order_relaxed);
                    }
                    periodStartTicks.store(now, std::memory_order_relaxed);
                }
                // Spin across the whole gap only for short periods (small ASIO
                // buffers). For long periods (WASAPI, large buffers) the workers
                // sleep between blocks so the Windows audio engine and the GUI
                // always find idle CPUs; the wake-up costs ~0.1 ms of a 10 ms period.
                // With other device threads around, only the brief spin after work.
                const int64_t period = periodTicks.load(std::memory_order_relaxed);
                // Few cores (one or two): only the brief spin after work - a worker
                // spinning across the gap would hold a core the bookkeeping, the
                // sound server and the window need (the two-core laptops, #9/#10).
                const int64_t window = (otherAudioThreads > 0 || gFewCores) ? hotWindowTicks
                    : juce::jlimit(hotWindowTicks, hotWindowTicks * 5 / 2, period + period / 2);
                hotUntilTicks.store(now + window, std::memory_order_relaxed);
            }
            if (sleepers.load(std::memory_order_acquire) > 0)
            {
                std::lock_guard<std::mutex> lk(sleepMutex);
                sleepCv.notify_all();
            }

            // This thread is a lane as well.
            int idx, count;
            const int64_t posterStart = juce::Time::getHighResolutionTicks();
            const int64_t profPoster0 = prof ? profileTicks() : 0;
            while ((count = takeChunkAsPoster(*job, idx)) > 0)
                renderRange(voiceIndices, idx, idx + count, pool, ctx, mine);
            const int64_t profOwn = prof ? profileDelta(profPoster0, profileTicks()) : 0;
            const int64_t posterBusy = juce::Time::getHighResolutionTicks() - posterStart;
            foldVoiceProfile();
            // Wait for the other lanes. On few cores the wait yields the core: a
            // lane that was preempted (the sound server, the window) gets it back
            // at once instead of this thread spinning on it.
            while (job->lanesLeft.load(std::memory_order_acquire) != job->lanesJoined.load(std::memory_order_acquire))
                ENGINE_RT_WAIT();
            job->open.store(false, std::memory_order_release);
            {
                // Utilisation of this job: the lanes' rendering time against the
                // lanes x the job's wall time (the rest: waiting for the slowest
                // lane, the chunk hand-over, lanes arriving late).
                const int64_t wall = juce::jmax<int64_t>(1, juce::Time::getHighResolutionTicks() - jobStart);
                const int lanesUsed = juce::jmax(1, (int)(job->lanesJoined.load(std::memory_order_relaxed) - job->joinBase) + 1);
                statJobs.fetch_add(1, std::memory_order_relaxed);
                statWallTicks.fetch_add(wall, std::memory_order_relaxed);
                statBusyTicks.fetch_add(job->laneBusyTicks.load(std::memory_order_relaxed) + posterBusy, std::memory_order_relaxed);
                statSlotTicks.fetch_add(wall * lanesUsed, std::memory_order_relaxed);
                statPosterTicks.fetch_add(posterBusy, std::memory_order_relaxed);
            }

            const int completed = job->completedCount.load(std::memory_order_acquire);
            for (int k = 0; k < completed; ++k)
            {
                const int scratchIndex = job->completed[(size_t)k];
                const float* base = scratchStore[(size_t)scratchIndex].data();
                const float* wetBase = base + (size_t)kScratchChannels * kFrames;
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch >= 0 && ch < mine.numOuts)
                    {
                        juce::FloatVectorOperations::add(outs[ch], base + (size_t)ch * kFrames, ctx.numSamples);
                        juce::FloatVectorOperations::add(wets[ch], wetBase + (size_t)ch * kFrames, ctx.numSamples);
                    }
                }
                releaseScratch(scratchIndex);
            }
            job->inUse.store(false, std::memory_order_release);
            if (prof)
            {
                tlPoolOwnTicks += profOwn;
                tlPoolWaitTicks += juce::jmax<int64_t>(0, profileDelta(profEntry, profileTicks()) - profOwn);
            }
        }

        //----------------------------------------------------------------------
        // Tasks: one-off work for a worker beside the voice jobs - the IR
        // convolution of a route, posted by the route's callback and finished
        // before the next block (joined there). Voice jobs come first in
        // tryWork(): they are the callback's critical path, a task has a period.
        static constexpr int kMaxTasks = 64;
        using TaskFn = void (*)(void*);
        bool postTask(int index, TaskFn fn, void* ctx) noexcept
        {
            if (index < 0 || index >= kMaxTasks) return false;
            Task& task = tasks[(size_t)index];
            const int state = task.state.load(std::memory_order_acquire);
            if (state == 1 || state == 2) return false;   // still pending: the caller waits first
            task.fn = fn;
            task.ctx = ctx;
            task.state.store(1, std::memory_order_release);
            if (numWorkers == 0) { runTask(task); return true; }   // no workers: done at once, on the caller
            if (sleepers.load(std::memory_order_acquire) > 0)
            {
                std::lock_guard<std::mutex> lk(sleepMutex);
                sleepCv.notify_all();
            }
            return true;
        }
        // Waits for the task (a few microseconds normally: it had a whole period).
        void waitTask(int index) noexcept
        {
            if (index < 0 || index >= kMaxTasks) return;
            Task& task = tasks[(size_t)index];
            for (;;)
            {
                const int state = task.state.load(std::memory_order_acquire);
                if (state != 1 && state != 2) return;
                if (state == 1 && numWorkers == 0) { runTask(task); return; }
                // Not picked up yet: do it here rather than wait for a busy worker.
                if (state == 1)
                {
                    int expected = 1;
                    if (task.state.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) { task.fn(task.ctx); task.state.store(3, std::memory_order_release); return; }
                }
                ENGINE_RT_WAIT();
            }
        }

    private:
        struct Task
        {
            std::atomic<int> state{ 0 };   // 0 idle, 1 posted, 2 running, 3 done
            TaskFn fn = nullptr;
            void* ctx = nullptr;
        };
        std::array<Task, kMaxTasks> tasks;
        static void runTask(Task& task) noexcept
        {
            int expected = 1;
            if (!task.state.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) return;
            task.fn(task.ctx);
            task.state.store(3, std::memory_order_release);
        }
        bool tryTasks() noexcept
        {
            bool didWork = false;
            for (auto& task : tasks)
                if (task.state.load(std::memory_order_acquire) == 1)
                {
                    int expected = 1;
                    if (task.state.compare_exchange_strong(expected, 2, std::memory_order_acq_rel))
                    {
                        task.fn(task.ctx);
                        task.state.store(3, std::memory_order_release);
                        didWork = true;
                    }
                }
            return didWork;
        }
        bool anyPostedTask() const noexcept
        {
            for (const auto& task : tasks) if (task.state.load(std::memory_order_acquire) == 1) return true;
            return false;
        }

        struct Job
        {
            std::atomic<bool> inUse{ false };
            std::atomic<bool> open{ false };
            std::atomic<int> nextVoice{ 0 }, completedCount{ 0 };
            // Lanes joined / left: monotonic across the uses of the job (0.8.226, see
            // render()); unsigned so the wrap-around keeps the differences right.
            std::atomic<uint32_t> lanesJoined{ 0 }, lanesLeft{ 0 };
            uint32_t joinBase = 0;                     // lanesJoined when this use opened
            std::atomic<int64_t> laneBusyTicks{ 0 };   // the lanes' rendering time of this job, summed (utilisation statistics)
            int64_t openedTicks = 0;                   // when the job opened (the lanes' join delay, 0.8.143)
            const uint32_t* voices = nullptr;
            int numVoices = 0, lanesHint = 1, maxWorkers = 0;
            Voice* pool = nullptr;
            const BlockContext* ctx = nullptr;
            std::array<int, kMaxLanesPerJob> completed{};
        };

        // The poster at the tail of a job (0.8.143): with less left than two voices
        // per lane still working, it takes nothing more - those lanes finish it in
        // the time the poster would need, and its callback ends sooner (the longest
        // callbacks of #46/#47 were the poster's own rendering, 2.3-2.5 ms).
        static int takeChunkAsPoster(Job& job, int& idxOut) noexcept
        {
            const int working = (int)(job.lanesJoined.load(std::memory_order_relaxed) - job.lanesLeft.load(std::memory_order_relaxed));
            if (working > 0 && job.numVoices - job.nextVoice.load(std::memory_order_relaxed) <= working * 2) return 0;
            return takeChunk(job, idxOut);
        }
        // Guided self-scheduling: big chunks while much work remains, small
        // chunks at the end so no lane is left waiting for one slow straggler.
        static int takeChunk(Job& job, int& idxOut) noexcept
        {
            for (;;)
            {
                int current = job.nextVoice.load(std::memory_order_relaxed);
                const int remaining = job.numVoices - current;
                if (remaining <= 0) return 0;
                const int chunk = juce::jlimit(2, gFewCores ? 12 : 48, remaining / juce::jmax(1, job.lanesHint * 3));   // small chunks on few cores: a preempted lane holds up less
                if (job.nextVoice.compare_exchange_weak(current, current + chunk, std::memory_order_acq_rel))
                {
                    idxOut = current;
                    return juce::jmin(chunk, remaining);
                }
            }
        }

        static void renderRange(const uint32_t* voices, int begin, int end, Voice* pool,
            const BlockContext& ctx, const LaneOutputs& lane) noexcept
        {
            if (begin < end) prefetchVoice(pool[voices[begin]]);
            if (begin + 1 < end) prefetchVoiceRecord(pool[voices[begin + 1]]);
            for (int i = begin; i < end; ++i)
            {
                if (i + 2 < end) prefetchVoiceRecord(pool[voices[i + 2]]);
                if (i + 1 < end) prefetchVoice(pool[voices[i + 1]]);
                Voice& v = pool[voices[i]];
                if (!renderVoice(v, ctx, lane)) v.finished = true;
            }
        }

        Job* acquireJob() noexcept
        {
            for (int attempt = 0; attempt < kMaxJobs; ++attempt)
            {
                Job& job = jobs[(size_t)((jobCursor.fetch_add(1, std::memory_order_relaxed) + attempt) % kMaxJobs)];
                bool expected = false;
                if (job.inUse.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return &job;
            }
            return nullptr;
        }

        int acquireScratch() noexcept
        {
            const juce::SpinLock::ScopedLockType lk(scratchLock);
            if (scratchFree.empty()) return -1;
            const int idx = scratchFree.back();
            scratchFree.pop_back();
            return idx;
        }
        void releaseScratch(int idx) noexcept
        {
            const juce::SpinLock::ScopedLockType lk(scratchLock);
            scratchFree.push_back(idx);
        }

        bool tryWork() noexcept
        {
            bool didWork = false;
            for (auto& job : jobs)
            {
                if (!job.open.load(std::memory_order_acquire)) continue;
                if (job.nextVoice.load(std::memory_order_relaxed) >= job.numVoices) continue;
                if (job.maxWorkers > 0 && (int)(job.lanesJoined.load(std::memory_order_relaxed) - job.joinBase) >= job.maxWorkers) continue;
                if ((int)(job.lanesJoined.fetch_add(1, std::memory_order_acq_rel) - job.joinBase) >= job.maxWorkers && job.maxWorkers > 0)
                {
                    job.lanesLeft.fetch_add(1, std::memory_order_acq_rel);   // over the limit, back off
                    continue;
                }
                // The scratch BEFORE the voices (0.8.226): a lane without a scratch
                // takes no chunk at all - the poster and the other lanes render those
                // voices. Until now it took the chunk first and, with no scratch left,
                // rendered it into nothing: the voices advanced but were not heard for
                // that block (a click at both block edges). It is counted, never silent.
                const int scratchIndex = acquireScratch();
                if (scratchIndex < 0)
                {
                    statNoScratch.fetch_add(1, std::memory_order_relaxed);
                    job.lanesLeft.fetch_add(1, std::memory_order_acq_rel);
                    continue;
                }
                int idx = 0, count = takeChunk(job, idx);
                if (count <= 0) { releaseScratch(scratchIndex); job.lanesLeft.fetch_add(1, std::memory_order_acq_rel); continue; }
                {
                    // How late this lane came to the job (the wake-up of a sleeping worker).
                    const int64_t delay = juce::Time::getHighResolutionTicks() - job.openedTicks;
                    int64_t known = statJoinDelayMax.load(std::memory_order_relaxed);
                    while (delay > known && !statJoinDelayMax.compare_exchange_weak(known, delay, std::memory_order_relaxed)) {}
                }
                float* base = scratchStore[(size_t)scratchIndex].data();
                float* wetBase = base + (size_t)kScratchChannels * kFrames;
                const BlockContext& ctx = *job.ctx;
                LaneOutputs lane;
                alignas(16) float sink[kFrames];
                lane.sink = sink;
                lane.numOuts = kScratchChannels;
                for (int c = 0; c < kScratchChannels; ++c) { lane.out[c] = base + (size_t)c * kFrames; lane.wet[c] = wetBase + (size_t)c * kFrames; }
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch >= 0 && ch < kScratchChannels)
                    {
                        juce::FloatVectorOperations::clear(lane.out[ch], ctx.numSamples);
                        juce::FloatVectorOperations::clear(lane.wet[ch], ctx.numSamples);
                    }
                }
                const int64_t laneStart = juce::Time::getHighResolutionTicks();
                do renderRange(job.voices, idx, idx + count, job.pool, ctx, lane);
                while ((count = takeChunk(job, idx)) > 0);
                job.laneBusyTicks.fetch_add(juce::Time::getHighResolutionTicks() - laneStart, std::memory_order_relaxed);
                foldVoiceProfile();
                const int slot = job.completedCount.fetch_add(1, std::memory_order_acq_rel);
                if (slot < kMaxLanesPerJob) job.completed[(size_t)slot] = scratchIndex;
                else releaseScratch(scratchIndex);
                job.lanesLeft.fetch_add(1, std::memory_order_acq_rel);
                didWork = true;
            }
            if (tryTasks()) didWork = true;   // after the voices: the routes' IR convolutions
            return didWork;
        }

        bool anyOpenJob() const noexcept
        {
            for (const auto& job : jobs)
                if (job.open.load(std::memory_order_acquire)
                    && job.nextVoice.load(std::memory_order_relaxed) < job.numVoices) return true;
            return anyPostedTask();
        }

        void workerLoop(int index)
        {
            juce::Thread::setCurrentThreadName("Organ Render " + juce::String(index));
            const auto granted = raiseAudioThreadPriority("Render worker");
            if (index == 0) CrashHandler::note(granted);   // once: every worker gets the same
           #if JUCE_WINDOWS
            if (const uint64_t mask = gRenderWorkerAffinity.load(std::memory_order_relaxed); mask != 0)
            {
                SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)mask);
                if (index == 0) CrashHandler::note("Render workers pinned to the performance cores (mask 0x" + juce::String::toHexString((juce::int64)mask) + ")");
            }
           #endif
            juce::ScopedNoDenormals noDenormals;
            int64_t lastWorkTicks = 0;
            while (!shutdown.load(std::memory_order_relaxed))
            {
                if (tryWork()) { lastWorkTicks = juce::Time::getHighResolutionTicks(); continue; }
                // Spin only briefly after work: long enough to catch the next
                // route of the same callback, short enough to hand the CPU back
                // to the OS/GUI between audio periods. Sleeping between periods is
                // what keeps Windows from throttling our real-time threads and
                // preempting a lane in the middle of a chunk.
                const int64_t now = juce::Time::getHighResolutionTicks();
               #if JUCE_WINDOWS
                const bool spinAllowed = true;
               #else
                const bool spinAllowed = !gFewCores;   // one or two cores: a spinning SCHED_FIFO worker starves the window thread (#15)
               #endif
                if (spinAllowed && (now < hotUntilTicks.load(std::memory_order_relaxed) || now - lastWorkTicks < hotWindowTicks))
                {
                    for (int i = 0; i < 16; ++i) ENGINE_CPU_PAUSE();
                    continue;
                }
                std::unique_lock<std::mutex> lk(sleepMutex);
                sleepers.fetch_add(1, std::memory_order_acq_rel);
                // Posters notify when a job opens; the timeout is a safety net - and,
                // since 0.8.143, the wake-ahead: the period is known from the jobs'
                // cadence, so a sleeping worker wakes 1.5 ms before the next period is
                // due and spins into it. A worker woken by the job itself came a wake-up
                // late, and the poster rendered alone meanwhile (the 2.3-2.5 ms peaks
                // of #46/#47 on 10 ms periods).
                const int64_t ticksPerSecond = juce::Time::getHighResolutionTicksPerSecond();
                int64_t waitTicks = ticksPerSecond / 50;   // 20 ms
                // The wake-ahead needs a spare core: with one worker on a two-core
                // computer it would only wake into the poster's core.
                if (numWorkers >= 2)
                {
                    const int64_t period = devicePeriodTicks.load(std::memory_order_relaxed), start = periodStartTicks.load(std::memory_order_relaxed);
                    if (period > ticksPerSecond / 1000 && start > 0)   // a period of a millisecond or more, known
                    {
                        const int64_t lead = ticksPerSecond * 3 / 2000;   // 1.5 ms before the period is due
                        int64_t next = start + period - lead;
                        const int64_t sleepFrom = juce::Time::getHighResolutionTicks();
                        while (next <= sleepFrom) next += period;
                        waitTicks = juce::jlimit(ticksPerSecond / 5000, waitTicks, next - sleepFrom);   // at least 0.2 ms
                    }
                }
                bool wokeAhead = false;
                if (!anyOpenJob() && !shutdown.load(std::memory_order_relaxed))
                    wokeAhead = sleepCv.wait_for(lk, std::chrono::microseconds(juce::jmax<int64_t>(100, waitTicks * 1000000 / juce::jmax<int64_t>(1, ticksPerSecond)))) == std::cv_status::timeout;
                sleepers.fetch_sub(1, std::memory_order_acq_rel);
                if (wokeAhead && spinAllowed) lastWorkTicks = juce::Time::getHighResolutionTicks();   // spin through the hot window into the period
            }
        }

        // Utilisation statistics since the last query (the polyphony measurement
        // reads them once a second): jobs, their wall time, the lanes' busy time,
        // lanes x wall ("slots"), the poster's own share.
        std::atomic<int64_t> statJobs{ 0 }, statWallTicks{ 0 }, statBusyTicks{ 0 }, statSlotTicks{ 0 }, statPosterTicks{ 0 }, statJoinDelayMax{ 0 };
        std::atomic<int64_t> statNoScratch{ 0 };   // lanes that found no scratch buffer and took no voices (0.8.226)
        // The voice profile (gVoiceProfiling): the lanes' thread-local sums, folded here after every chunk.
        std::atomic<int64_t> statProfHeader{ 0 }, statProfSpans{ 0 }, statProfKernel{ 0 }, statProfPost{ 0 }, statProfUnpack{ 0 }, statProfVoices{ 0 };
        void foldVoiceProfile() noexcept
        {
            auto& p = tlVoiceProfile;
            if (p.voices == 0) return;
            statProfHeader.fetch_add(p.header, std::memory_order_relaxed);
            statProfSpans.fetch_add(p.spans, std::memory_order_relaxed);
            statProfKernel.fetch_add(p.kernel, std::memory_order_relaxed);
            statProfPost.fetch_add(p.post, std::memory_order_relaxed);
            statProfUnpack.fetch_add(p.unpack, std::memory_order_relaxed);
            statProfVoices.fetch_add(p.voices, std::memory_order_relaxed);
            p = VoiceProfile{};
        }
    public:
        int takeNoScratch() noexcept { return (int)statNoScratch.exchange(0, std::memory_order_relaxed); }
        void takeStats(int& jobs, double& laneUtilisation, double& posterShare, double& jobSeconds,
            juce::int64& profVoices, double& headerNs, double& spansNs, double& kernelNs, double& postNs, double& unpackNs, double& joinDelayMaxNs) noexcept   // juce::int64: long long on Linux too
        {
            joinDelayMaxNs = (double)statJoinDelayMax.exchange(0, std::memory_order_relaxed) * 1.0e9 / (double)juce::Time::getHighResolutionTicksPerSecond();
            jobs = (int)statJobs.exchange(0, std::memory_order_relaxed);
            const int64_t wall = statWallTicks.exchange(0, std::memory_order_relaxed);
            const int64_t busy = statBusyTicks.exchange(0, std::memory_order_relaxed);
            const int64_t slots = statSlotTicks.exchange(0, std::memory_order_relaxed);
            const int64_t poster = statPosterTicks.exchange(0, std::memory_order_relaxed);
            laneUtilisation = slots > 0 ? (double)busy / (double)slots : 0.0;
            posterShare = wall > 0 ? (double)poster / (double)wall : 0.0;
            const double tickNs = 1.0e9 / (double)juce::Time::getHighResolutionTicksPerSecond();
            jobSeconds = (double)wall * tickNs * 1.0e-9;
            profVoices = statProfVoices.exchange(0, std::memory_order_relaxed);
            const double profTickNs = 1.0e9 / (double)profileTicksPerSecond();   // the profiling clock has its own rate (rdtsc)
            const double per = profVoices > 0 ? profTickNs / (double)profVoices : 0.0;
            headerNs = (double)statProfHeader.exchange(0, std::memory_order_relaxed) * per;
            spansNs = (double)statProfSpans.exchange(0, std::memory_order_relaxed) * per;
            kernelNs = (double)statProfKernel.exchange(0, std::memory_order_relaxed) * per;
            postNs = (double)statProfPost.exchange(0, std::memory_order_relaxed) * per;
            unpackNs = (double)statProfUnpack.exchange(0, std::memory_order_relaxed) * per;
        }
    private:

        int numWorkers = 0;
        std::vector<std::thread> threads;
        std::atomic<bool> shutdown{ false };
        std::array<Job, kMaxJobs> jobs;
        std::atomic<int> jobCursor{ 0 };
        std::vector<std::vector<float>> scratchStore;
        std::vector<int> scratchFree;
        juce::SpinLock scratchLock;
        std::mutex sleepMutex;
        std::condition_variable sleepCv;
        std::atomic<int> sleepers{ 0 };
        std::atomic<int64_t> hotUntilTicks{ 0 };
        std::atomic<int64_t> periodStartTicks{ 0 }, devicePeriodTicks{ 0 };   // the device callbacks' cadence (the wake-ahead, 0.8.144)
        std::atomic<int64_t> lastJobTicks{ 0 };
        std::atomic<int64_t> periodTicks{ 0 };
        const int64_t hotWindowTicks = juce::Time::getHighResolutionTicksPerSecond() / 1000;   // 1 ms
    };
}

//==============================================================================
// Engine implementation state
//==============================================================================
struct SamplerEngine::Impl
{
    static constexpr int poolSize = SamplerEngine::maxVoices + 8192;

    static int automaticWorkerCount() noexcept
    {
        // Default preset "all physical cores": the callback thread is a lane too,
        // so workers = physical - 1. Hyper-threads add nothing to this memory-bound
        // kernel and only cause crackle. The Configuration Editor offers other counts.
        const int physical = juce::jmax(1, juce::SystemStats::getNumPhysicalCpus());
        const int logical = juce::jmax(1, juce::SystemStats::getNumCpus());
        return juce::jlimit(0, 62, juce::jmin(physical, logical - 1) - 1);
    }

    explicit Impl(SamplerEngine& e)
        : owner(e), pool(std::make_unique<RenderPool>(automaticWorkerCount()))
    {
        voices.reset(new Voice[(size_t)poolSize]);
        CrashHandler::note("Voice pool: " + juce::String(poolSize) + " voices, " + juce::String(sizeof(Voice)) + " bytes each, "
            + juce::String((double)poolSize * (double)sizeof(Voice) / (1024.0 * 1024.0), 0) + " MB (plus 12 bytes per voice and route)");
        stealable.assign((size_t)poolSize, 0);
        freeList.reserve((size_t)poolSize);
        for (int i = poolSize; --i >= 0;) freeList.push_back((uint32_t)i);
        controlActive.reserve((size_t)poolSize);
        finishedQueue.reserve((size_t)poolSize);
        finishedSwap.reserve((size_t)poolSize);
        keyHeads.fill(-1);
        releasePitches.reserve(4096);
        releaseTargets.reserve(4096);
        for (auto& g : expressionGain) g.store(1.0f);
        for (auto& g : expressionMaxDb) g.store(13.0f);
        for (auto& g : divisionGain) g.store(1.0f);
        for (auto& r : manualSampleRate) r.store(48000.0);
        for (auto& m : manualDefaultMask) m.store(3);
        for (auto& a : randomAllowed) a.store(true);
        for (auto& a : couplingAllowed) a.store(true);
        for (auto& c : pitchClassCents) c.store(0.0f);
        for (int m = 0; m < maxManuals; ++m)
        {
            windSources[(size_t)m].targetMask = (uint8_t)(1u << m);
            windSnapshots[(size_t)m].write(windSources[(size_t)m]);
            windLagMs[(size_t)m].store(windSources[(size_t)m].baseSmoothingMs);
            trem[(size_t)m].enabled.store(false);
            trem[(size_t)m].rateHz.store(4.4f);
            trem[(size_t)m].depthCents.store(6.0f);
            trem[(size_t)m].rankDriftDegrees.store(260.0f);
        }
        noteOnTimeMs.fill(-1.0);
        noteSoundStartMs.fill(-1.0);
        noteMinimumStartMs.fill(-1.0);
        lastAttackDelayNoteOnMs.fill(-1.0);
        noteFinishedMs.fill(-1.0);
        heldVelocity.fill(-1);
    }

    SamplerEngine& owner;
    std::unique_ptr<RenderPool> pool;
    std::unique_ptr<Voice[]> voices;
    std::vector<uint32_t> freeList;
    std::vector<uint32_t> controlActive;
    std::array<int32_t, kSourceKeys> keyHeads{};
    juce::SpinLock finishedLock;
    std::vector<uint32_t> finishedQueue, finishedSwap;
    std::array<std::unique_ptr<Route>, kMaxRoutes> routes;
    std::atomic<int> routeCount{ 0 };
    std::atomic<bool> suspended{ false };
    // IR processing: Light (weak computers, see RouteReverb) and the overload flag of the runtime guard.
    std::atomic<bool> reverbLight{ false };
    std::atomic<bool> reverbOverload{ false };

    struct MidiCommand { uint8_t note = 0, velocity = 0; int8_t manual = -1; bool on = false; };
    static constexpr uint32_t midiRingSize = 8192;
    std::array<MidiCommand, midiRingSize> midiRing{};
    std::atomic<uint32_t> midiHead{ 0 }, midiTail{ 0 };
    juce::SpinLock midiProducerLock;

    // The control thread (see SamplerEngine::startControlThread).
    std::thread controlThread;
    std::atomic<bool> controlThreadRunning{ false }, controlThreadStop{ false };
    juce::WaitableEvent controlWake;   // a MIDI command arrived, voices finished: look now, not at the next millisecond

    void controlLoop()
    {
        juce::Thread::setCurrentThreadName("Organ Control");
        CrashHandler::note(raiseAudioThreadPriority("Control thread"));
       #if JUCE_WINDOWS
        timeBeginPeriod(1);   // 1 ms waits really take 1 ms
       #endif
        juce::ScopedNoDenormals noDenormals;
        while (!controlThreadStop.load(std::memory_order_acquire))
        {
            bool more = false;
            if (!suspended.load(std::memory_order_acquire))
            {
                const int64_t t0 = juce::Time::getHighResolutionTicks();
                owner.controlLock.enter();
                if (!suspended.load(std::memory_order_acquire))
                {
                    // Two milliseconds per hold of the lock: the GUI thread, which
                    // takes the same lock for its changes, never waits for long;
                    // whatever is left continues after a yield.
                    tick(2.0);
                    const int64_t spent = juce::Time::getHighResolutionTicks() - t0;
                    owner.controlTickTicksTotal.fetch_add(spent, std::memory_order_relaxed);
                    owner.controlTickCount.fetch_add(1, std::memory_order_relaxed);
                    const float ms = (float)(1000.0 * (double)spent / (double)juce::Time::getHighResolutionTicksPerSecond());
                    float previous = owner.maxControlTickMs.load(std::memory_order_relaxed);
                    while (ms > previous && !owner.maxControlTickMs.compare_exchange_weak(previous, ms)) {}
                }
                owner.controlLock.exit();
                more = midiHead.load(std::memory_order_relaxed) != midiTail.load(std::memory_order_acquire);
            }
            if (more) std::this_thread::yield();
            else controlWake.wait(1);
        }
       #if JUCE_WINDOWS
        timeEndPeriod(1);
       #endif
    }

    // note bookkeeping by source key
    std::array<double, kSourceKeys> noteOnTimeMs{}, noteSoundStartMs{}, noteMinimumStartMs{},
        lastAttackDelayNoteOnMs{}, noteFinishedMs{};
    std::array<int8_t, kSourceKeys> heldVelocity{};
    struct PendingOn { double dueMs = 0.0; int velocity = 0; bool valid = false; };
    struct PendingOff { double dueMs = 0.0; std::array<float, maxManuals> gains{}; bool valid = false; };
    std::array<PendingOn, kSourceKeys> pendingOn{};
    std::array<PendingOff, kSourceKeys> pendingOff{};

    std::array<std::atomic<float>, maxManuals> expressionGain, divisionGain;
    std::array<std::atomic<float>, maxManuals> expressionMaxDb;   // deepest swell attenuation per division
    std::array<std::atomic<double>, maxManuals> manualSampleRate;
    std::array<std::atomic<uint64_t>, maxManuals> manualDefaultMask;
    std::array<std::atomic<bool>, maxManuals> randomAllowed, couplingAllowed;
    std::array<std::atomic<float>, 12> pitchClassCents;
    float globalPitchCents = 0.0f;
    std::array<float, 12> temperamentSelected{};                 // selected table, absolute cents
    std::array<std::atomic<float>, 12> selectedPitchCents{};     // global pitch + selected table
    std::array<float, 12> temperamentOffsets{};

    std::array<WindSourceState, maxManuals> windSources;
    std::array<SeqLocked<WindSourceState>, maxManuals> windSnapshots;
    struct TremParams
    {
        std::atomic<bool> enabled{ false };
        std::atomic<float> rateHz{ 4.4f }, depthCents{ 6.0f }, rankDriftDegrees{ 260.0f };
    };
    std::array<TremParams, maxManuals> trem;
    std::array<std::atomic<float>, maxManuals> windLagMs{};   // wind sag lag per division (route volume glide)

    uint64_t nextCreationOrder = 1;
    int stealCursor = 0;
    bool stealExhausted = false;   // this tick found nothing to steal; cleared by the next reclaim
    // One byte per pool slot: may this voice be stolen (not stolen yet, and a
    // release tail or a sustaining pipe)? The steal scan reads this array (a
    // hundred KB, cache resident) instead of the voices themselves (every one a
    // cache miss into a pool of many MB). Control thread only, like the fields.
    std::vector<uint8_t> stealable;
    void refreshStealable(const Voice& v) noexcept
    {
        const size_t idx = (size_t)(&v - voices.get());
        if (idx < stealable.size()) stealable[idx] = (!v.ctlStolen && (v.detachedRelease || !v.ctlReleasing)) ? 1 : 0;
    }
    juce::Random tuningRandom, delayRandom, windRandom;
    double lastStaleCheckMs = 0.0;
    std::atomic<double> lastBlockMs{ 5.333 };   // shortest recently rendered block, for control budgeting
    std::atomic<int64_t> lastTickStartTicks{ 0 }, lastTickEndTicks{ 0 };   // one control tick per device period, see controlTickIfPossible
    struct ReleasePitch
    {
        Stop* stop; Stop* rank; float cents; float windGain;
        uint8_t note, keyNote, sampleNote; int8_t shift; float shiftCents;   // the release follows a transposed attack
        float routeGain;
        float toneDb, toneStartHz;   // the release keeps the attack's tone shelf
        uint8_t velocity;            // the key velocity: the release of the attack's velocity layer
        bool preShifted;             // the attack played a pitch-shifted copy (its shiftCents lack the semitones)
    };
    std::vector<ReleasePitch> releasePitches;
    struct ReleaseTarget { Stop* stop; int pipe; int keyNote; };
    std::vector<ReleaseTarget> releaseTargets;

    struct PendingDivision
    {
        int manual = 0;
        double targetSR = 48000.0;
        bool append = false;   // keep the division's existing stops, add these
        bool replace = true;   // the commit retires the division's old stops (once per division; the new ones adopt their samples)
        std::shared_ptr<const std::set<int>> replacingAll;   // every division the load replaces (retired even without a batch)
        bool mapped = false;   // built from a folder map: same-named stops of other trees merge
        juce::File tree;       // folder tree these stops come from
        juce::OwnedArray<Stop> stops;
    };
    std::mutex pendingMutex;
    std::vector<std::unique_ptr<PendingDivision>> pendingDivisions;

    //--------------------------------------------------------------------------
    // Recording
    std::unique_ptr<Recorder> recorder;
    class RecorderThread final : public juce::Thread
    {
    public:
        explicit RecorderThread(Impl& i) : juce::Thread("Organ Recorder"), impl(i) {}
        void run() override
        {
            while (!threadShouldExit())
            {
                impl.flushRecording(false);
                wait(50);
            }
        }
    private:
        Impl& impl;
    };
    std::unique_ptr<RecorderThread> recorderThread;

    // Writes every frame up to the slowest live route (or everything when final).
    void flushRecording(bool final)
    {
        Recorder* rec = recorder.get();
        if (rec == nullptr || rec->writer == nullptr) return;
        int64_t target = -1;
        const double now = nowMs();
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            const Route& r = *routes[(size_t)i];
            const int64_t published = r.recPosPublished.load(std::memory_order_acquire);
            if (published < 0) continue;
            if (!final && now - r.lastRenderMs.load(std::memory_order_relaxed) > 1000.0) continue;
            target = final ? juce::jmax(target, published) : (target < 0 ? published : juce::jmin(target, published));
        }
        int64_t flushed = rec->flushed.load(std::memory_order_acquire);
        // 0.8.208: the file follows the wall clock. While the audio device restarts (a division reloaded, a route
        // or a device changed) no route renders, the positions stand still and the sound after the pause landed
        // right after the sound before it - the recording came out shorter than the time it covered (the video's
        // organ ran 7 s ahead of its picture by the end). Now the writer flushes silence up to the wall time (a
        // 300 ms margin behind it, the routes publish a block late) and the routes that come back are "left
        // behind" (recPos < flushed) and continue from there: the pause is silence in the file, as it was in the room.
        if (!final && rec->wallStartMs > 0.0)
        {
            const int64_t expected = (int64_t)((now - rec->wallStartMs) / 1000.0 * rec->sampleRate) - (int64_t)(0.3 * rec->sampleRate);
            if (expected > target) target = expected;
        }
        const int64_t fadeFrames = juce::jmax<int64_t>(1, rec->fadeFrames);
        const int64_t fadeOutStart = rec->fadeOutStart.load(std::memory_order_acquire);
        if (fadeOutStart >= 0) target = juce::jmin(target, fadeOutStart + fadeFrames);   // the file ends there
        if (target <= flushed) return;
        rec->flushTarget.store(target, std::memory_order_release);
        while (flushed < target)
        {
            const int chunk = (int)juce::jmin<int64_t>(target - flushed, rec->flushBuffer.getNumSamples());
            rec->flushBuffer.clear(0, chunk);
            for (int i = 0; i < count; ++i)
            {
                Route& r = *routes[(size_t)i];
                if (r.recRing[0].size() != (size_t)Recorder::ringSize) continue;
                for (int c = 0; c < 2; ++c)
                {
                    float* ring = r.recRing[c].data();
                    float* out = rec->flushBuffer.getWritePointer(c);
                    for (int k = 0; k < chunk; ++k)
                    {
                        const size_t index = (size_t)((flushed + k) & Recorder::ringMask);
                        out[k] += ring[index];
                        ring[index] = 0.0f;
                    }
                }
            }
            // 500 ms fade-in at the start of the file, 500 ms fade-out at its end.
            if (flushed < fadeFrames || fadeOutStart >= 0)
                for (int k = 0; k < chunk; ++k)
                {
                    const int64_t frame = flushed + k;
                    float gain = frame < fadeFrames ? (float)frame / (float)fadeFrames : 1.0f;
                    if (fadeOutStart >= 0 && frame >= fadeOutStart)
                        gain *= juce::jmax(0.0f, 1.0f - (float)(frame - fadeOutStart) / (float)fadeFrames);
                    for (int c = 0; c < 2; ++c) rec->flushBuffer.getWritePointer(c)[k] *= gain;
                }
            rec->writer->writeFromAudioSampleBuffer(rec->flushBuffer, 0, chunk);
            flushed += chunk;
            rec->flushed.store(flushed, std::memory_order_release);
        }
    }

    // Allocates the recording rings of one route (message thread / control lock).
    bool allocateRecordingRing(Route& route)
    {
        try
        {
            for (auto& ring : route.recRing) ring.assign((size_t)Recorder::ringSize, 0.0f);
            return true;
        }
        catch (const std::bad_alloc&)
        {
            for (auto& ring : route.recRing) { ring.clear(); ring.shrink_to_fit(); }
            return false;
        }
    }

    //--------------------------------------------------------------------------
    // Routes
    Route* findRoute(int manual, const juce::String& perspective) const noexcept
    {
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            Route* r = routes[(size_t)i].get();
            if (r->manual == manual && r->perspective.equalsIgnoreCase(perspective)) return r;
        }
        return nullptr;
    }
    int routeIndexOf(const Route* r) const noexcept
    {
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i) if (routes[(size_t)i].get() == r) return i;
        return -1;
    }
    // A high pipe gets the 8-tap sinc playback kernel for its whole length
    // (0.8.164): its sounding fundamental (the pipe, its rank's footage and the
    // transposition - what fadeScale counts with) against the setting.
    bool wantsFullKernel(const Stop* rank, int pipe) const noexcept
    {
        const float fromHz = owner.sincKernelFromHz.load(std::memory_order_relaxed);
        if (fromHz >= 20000.0f) return false;
        if (fromHz <= 0.0f) return true;
        const int sounding = pipe + (rank != nullptr && rank->footageKnown ? rank->footageSemitones + rank->sampleOffset : 0);
        const double hz = 440.0 * std::pow(2.0, ((double)sounding - 69.0) / 12.0);
        return hz >= (double)fromHz;
    }
    // The common retune of a voice of this rank: the global pitch and, with the
    // stop tuned to 440 Hz, the rank's recorded diapason brought to 440 (the
    // per-pipe rest of tuning440Cents stays in the voice).
    float commonRetuneCents(const Stop* stop, const Stop* rank) const noexcept
    {
        float cents = globalPitchCents;
        if (stop != nullptr && rank != nullptr && stop->tuneTo440 && rank->measuredDiapasonHz > 1.0f)
            cents += (float)(1200.0 * std::log2(440.0 / (double)rank->measuredDiapasonHz));
        return cents;
    }
    static void buildBusTable(Route::Bus& bus)
    {
        // Windowed sinc, 16 taps, 128 phases: the cutoff a little under the lower
        // of the two Nyquists (reading faster than 1 folds the input's top).
        const double cutoff = 0.5 * juce::jmin(1.0, 1.0 / juce::jmax(1.0e-6, bus.ratio)) * 0.92;   // cycles per input frame
        bus.table.assign((size_t)kBusPhases * kBusTaps, 0.0f);
        for (int p = 0; p < kBusPhases; ++p)
        {
            const double frac = (double)p / (double)kBusPhases;
            double sum = 0.0;
            float* h = bus.table.data() + (size_t)p * kBusTaps;
            for (int t = 0; t < kBusTaps; ++t)
            {
                const double d = (double)(t - (kBusTaps / 2 - 1)) - frac;   // distance of tap t from the output position
                const double x = 2.0 * juce::MathConstants<double>::pi * cutoff * d;
                const double sinc = std::abs(x) < 1.0e-9 ? 1.0 : std::sin(x) / x;
                const double w = (double)t + 1.0 - frac;   // Blackman over the 16 + 1 taps around the centre
                const double window = 0.42 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * w / (kBusTaps + 1))
                    + 0.08 * std::cos(4.0 * juce::MathConstants<double>::pi * w / (kBusTaps + 1));
                h[t] = (float)(sinc * juce::jmax(0.0, window));
                sum += h[t];
            }
            for (int t = 0; t < kBusTaps; ++t) h[t] = (float)(h[t] / juce::jmax(1.0e-9, sum));   // unity at DC
        }
    }
    // The bus a voice with this common retune goes to on the route (0 = none,
    // the voice renders straight into the route as ever). Control thread (voice
    // starts) only: a new bus is filled in and allocated before it is 'ready'.
    int busFor(Route& route, float cents)
    {
        if (std::abs(cents) < kBusMinCents) return 0;
        for (int i = 0; i < kMaxBuses; ++i)
        {
            auto& b = route.buses[i];
            if (b.ready.load(std::memory_order_acquire) && std::abs(b.cents - cents) <= kBusJoinCents) return i + 1;
        }
        for (int i = 0; i < kMaxBuses; ++i)
        {
            auto& b = route.buses[i];
            if (b.ready.load(std::memory_order_acquire)) continue;
            b.cents = cents;
            b.ratio = std::pow(2.0, (double)cents / 1200.0);
            b.list.reserve((size_t)poolSize);
            for (auto& f : b.fifo) f.assign((size_t)kBusFifo, 0.0f);
            b.wet.assign((size_t)64 * kMaxBlockFrames, 0.0f);
            for (auto& s : b.shelfState) std::fill(s, s + 4, 0.0f);
            buildBusTable(b);
            b.readPos = (double)(kBusTaps / 2 + 1);   // history of zeros before the first frame
            b.fill = kBusTaps / 2 + 1;
            b.idleBlocks = 0;
            if (route.direct.capacity() < (size_t)poolSize) route.direct.reserve((size_t)poolSize);
            b.ready.store(true, std::memory_order_release);
            route.anyBus.store(true, std::memory_order_release);
            CrashHandler::note("Retune bus " + juce::String(i + 1) + " of route manual " + juce::String(route.manual) + " perspective " + route.perspective
                + ": " + juce::String(cents, 1) + " cents (ratio " + juce::String(b.ratio, 5) + ") - its voices render at that rate and the bus is resampled once");
            return i + 1;
        }
        return 0;
    }
    // One retune bus of the route for this block: its voices render into the
    // FIFO at routeRate x ratio (so their increments stay near 1), then the FIFO
    // is read at 'ratio' through the windowed sinc into the route's output.
    void renderBus(Route::Bus& bus, const BlockContext& ctx, int n, float* const* outs, int usableOuts, float* sink, int laneLimit, int otherAudioThreads)
    {
        const int half = kBusTaps / 2;
        if (bus.list.empty())
        {
            // Nothing sounds: the FIFO drains for two blocks (its tail), then rests.
            if (++bus.idleBlocks > 2)
            {
                if (bus.fill != half + 1)
                {
                    for (int ci = 0; ci < ctx.outCount; ++ci)
                    {
                        const int ch = ctx.outChannels[ci];
                        if (ch >= 0 && ch < usableOuts) juce::FloatVectorOperations::clear(bus.fifo[ch].data(), half + 1);
                    }
                    bus.readPos = (double)(half + 1);
                    bus.fill = half + 1;
                }
                return;
            }
        }
        else bus.idleBlocks = 0;
        const double r = bus.ratio;
        // Frames already read move out of the FIFO, the taps' history stays.
        if (bus.readPos > 2.0 * kMaxBlockFrames)
        {
            const int drop = (int)bus.readPos - half - 1;
            if (drop > 0 && drop < bus.fill)
            {
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch < 0 || ch >= usableOuts) continue;
                    float* f = bus.fifo[ch].data();
                    std::memmove(f, f + drop, (size_t)(bus.fill - drop) * sizeof(float));
                }
                bus.readPos -= drop;
                bus.fill -= drop;
            }
        }
        const int needed = juce::jmin(kBusFifo, (int)std::ceil(bus.readPos + (double)n * r) + half + 2);
        if (bus.fill < needed)
        {
            BlockContext busCtx = ctx;
            busCtx.routeRate = ctx.routeRate * r;
            butterworthLowpass(busCtx.routeRate, kShelfCornerHz, busCtx.shelfLp);
            float* bouts[64]{};
            float* bwets[64]{};
            while (bus.fill < needed)
            {
                const int k = juce::jmin(RenderPool::kFrames, needed - bus.fill);
                busCtx.numSamples = k;
                for (int c = 0; c < usableOuts; ++c)
                {
                    bouts[c] = bus.fifo[c].data() + bus.fill;
                    bwets[c] = bus.wet.data() + (size_t)c * kMaxBlockFrames;
                }
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch < 0 || ch >= usableOuts) continue;
                    juce::FloatVectorOperations::clear(bouts[ch], k);
                    juce::FloatVectorOperations::clear(bwets[ch], k);
                }
                if (!bus.list.empty())
                {
                    pool->render(bus.list.data(), (int)bus.list.size(), voices.get(), busCtx, bouts, bwets, usableOuts, sink, laneLimit, otherAudioThreads);
                    if (busCtx.shelfEnabled)
                        for (int ci = 0; ci < ctx.outCount; ++ci)
                        {
                            const int ch = ctx.outChannels[ci];
                            if (ch >= 0 && ch < usableOuts) applyShelfLowpass(bwets[ch], bouts[ch], k, busCtx.shelfLp, bus.shelfState[ch]);
                        }
                }
                bus.fill += k;
            }
        }
        for (int ci = 0; ci < ctx.outCount; ++ci)
        {
            const int ch = ctx.outChannels[ci];
            if (ch < 0 || ch >= usableOuts) continue;
            const float* f = bus.fifo[ch].data();
            float* o = outs[ch];
            double pos = bus.readPos;
            for (int i = 0; i < n; ++i, pos += r)
            {
                const int idx = (int)pos;
                if (idx + half >= bus.fill || idx < half - 1) break;
                const int phase = juce::jmin(kBusPhases - 1, (int)((pos - (double)idx) * kBusPhases));
                const float* h = bus.table.data() + (size_t)phase * kBusTaps;
                const float* x = f + idx - (half - 1);
                float acc = 0.0f;
                for (int t = 0; t < kBusTaps; ++t) acc += x[t] * h[t];
                o[i] += acc;
            }
        }
        bus.readPos += (double)n * r;
    }
    // Message thread or control lock holder only; may allocate.
    Route* ensureRoute(int manual, const juce::String& perspectiveIn)
    {
        const juce::String perspective = perspectiveIn.isEmpty() ? juce::String("default") : perspectiveIn;
        if (auto* existing = findRoute(manual, perspective)) return existing;
        const int count = routeCount.load(std::memory_order_acquire);
        if (count >= kMaxRoutes) return nullptr;
        auto route = std::make_unique<Route>();
        route->manual = juce::jlimit(0, maxManuals - 1, manual);
        route->perspective = perspective;
        route->channelMask.store(manualDefaultMask[(size_t)route->manual].load());
        route->sampleRate.store(manualSampleRate[(size_t)route->manual].load());
        const auto hash = (uint32_t)perspective.toLowerCase().hashCode();
        route->windPhase = perspective.equalsIgnoreCase("default") ? 0.0f : ((int)(hash % 9u) - 4) * 0.12f;
        route->active.reserve((size_t)poolSize);
        route->addRing.resize((size_t)poolSize);
        route->finishedScratch.reserve((size_t)poolSize);
        route->windCache.reset(new WindSourceState[maxManuals]);
        if (recorder != nullptr) allocateRecordingRing(*route);   // joins a recording in progress
        routes[(size_t)count] = std::move(route);
        routeCount.store(count + 1, std::memory_order_release);
        return routes[(size_t)count].get();
    }
    void ensureRoutesForStops()
    {
        for (auto* stop : owner.stops)
        {
            auto rankRoutes = [this](Stop* rank)
            {
                for (int m = 0; m < maxManuals; ++m)
                {
                    Route* r = ensureRoute(m, rank->perspective);
                    rank->routeByManual[(size_t)m] = (int16_t)(r != nullptr ? routeIndexOf(r) : -1);
                }
            };
            rankRoutes(stop);
            for (auto& rank : stop->ranks) rankRoutes(rank.get());
        }
        // The auxiliary (downmix) route of every division.
        for (int m = 0; m < maxManuals; ++m)
        {
            Route* r = ensureRoute(m, SamplerEngine::auxiliaryPerspectiveName());
            auxRouteIndex[(size_t)m] = r != nullptr ? routeIndexOf(r) : -1;
        }
    }
    std::array<int, maxManuals> auxRouteIndex{ -1, -1, -1, -1, -1, -1, -1, -1 };

    // The auxiliary route of a division takes a copy of every voice started for
    // one of its perspectives, but only while a device actually renders it (an
    // unused auxiliary route costs nothing).
    Route* auxRouteFor(int manual, int mainRouteIndex, double now) noexcept
    {
        const int index = auxRouteIndex[(size_t)juce::jlimit(0, maxManuals - 1, manual)];
        if (index < 0 || index == mainRouteIndex || index >= routeCount.load(std::memory_order_acquire)) return nullptr;
        Route& route = *routes[(size_t)index];
        const double last = route.lastRenderMs.load(std::memory_order_relaxed);
        if (last < 0.0 || now - last > 1000.0) return nullptr;
        return &route;
    }

    // Publishes a copy of a just-prepared voice into the auxiliary route; the
    // sample-count fields follow the auxiliary device's rate.
    void duplicateVoiceToAux(const Voice& v, int manual, double mainRate, double now, float busCents = 0.0f)
    {
        const int mainIndex = v.routeIndex;
        Route* aux = auxRouteFor(manual, mainIndex, now);
        if (aux == nullptr) return;
        makeRoomForVoice();
        uint32_t idx = 0;
        Voice* copy = allocVoice(idx);
        if (copy == nullptr) return;
        const int auxBus = busFor(*aux, busCents);
        const double auxRate = juce::jmax(1.0, aux->sampleRate.load(std::memory_order_relaxed)) * (auxBus > 0 ? aux->buses[auxBus - 1].ratio : 1.0);
        const double scale = auxRate / juce::jmax(1.0, mainRate);
        Voice& w = *copy;
        w.sample = v.sample; w.stop = v.stop; w.rank = v.rank;
        w.routeIndex = routeIndexOf(aux);
        w.bus = (uint8_t)auxBus;
        w.sourceKey = v.sourceKey; w.manual = v.manual; w.note = v.note; w.keyNote = v.keyNote;
        w.sampleNote = v.sampleNote; w.shift = v.shift; w.cubic = v.cubic; w.fullKernel = v.fullKernel;
        w.windPitchVar = v.windPitchVar; w.windGainVar = v.windGainVar; w.tremBucket = v.tremBucket;
        w.detachedRelease = v.detachedRelease; w.loopEnabled = v.loopEnabled; w.couplingUsed = v.couplingUsed;
        w.startFrozen = v.startFrozen;
        w.velocityGain = v.velocityGain; w.velocity = v.velocity; w.coupledPitchCents = v.coupledPitchCents;
        w.couplingTotalSamples = (uint32_t)juce::jmax(1.0, v.couplingTotalSamples * scale);
        w.fadeInTotal = (uint32_t)(v.fadeInTotal * scale); w.fadeInRemaining = w.fadeInTotal;
        w.attackHold = (uint32_t)(v.attackHold * scale); w.attackFade = (uint32_t)(v.attackFade * scale);
        w.attackBoost = v.attackBoost; w.attackActive = v.attackActive;
        w.initialFrozenExpr = v.initialFrozenExpr; w.initialWindGain = v.initialWindGain; w.windKeyScale = v.windKeyScale;
        w.tuningCents.store(v.tuningCents.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.shiftCents.store(v.shiftCents.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.routeGainTarget.store(v.routeGainTarget.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.toneTarget.store(v.toneTarget.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.toneStartHz = v.toneStartHz;
        w.fillGain = v.fillGain; w.toneOffset = v.toneOffset;
        w.attackToneDb = v.attackToneDb; w.attackToneStepPerFrame = v.attackToneStepPerFrame / (float)juce::jmax(0.01, scale);
        w.tempMode.store(v.tempMode.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.randomPitchCents.store(v.randomPitchCents.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.releaseFadeMsRequested.store(v.releaseFadeMsRequested.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.frozenExprRequested.store(v.frozenExprRequested.load(std::memory_order_relaxed), std::memory_order_relaxed);
        w.ctlReleasing = v.ctlReleasing;
        if (v.fadingOut)   // release cutting already started on the original
        {
            w.fadingOut = true;
            w.fadeOutTotal = (uint32_t)juce::jmax(1.0, v.fadeOutTotal * scale);
            w.fadeOutRemaining = (uint32_t)juce::jmax(1.0, v.fadeOutRemaining * scale);
            w.killing = v.killing;
        }
        publishVoice(idx, *aux);
    }

    //--------------------------------------------------------------------------
    // Route reverb (message thread builds, render thread uses)
    void rebuildRouteReverb(Route& route)
    {
        std::unique_ptr<RouteReverb> fresh;
        route.reverbError.clear();
        if (route.reverbOwner == nullptr && SampleFiles::existsAsFile(route.reverbIrFile))   // a follower of a shared IR builds nothing: its owner convolves for it
        {
            juce::AudioFormatManager formats;
            formats.registerBasicFormats();
            // On the disk or inside a chosen RAR archive (0.8.207: the Rural Organ demo's IRs live in its archive).
            std::unique_ptr<juce::AudioFormatReader> reader;
            if (route.reverbIrFile.existsAsFile()) reader.reset(formats.createReaderFor(route.reverbIrFile));
            else if (auto stream = SampleFiles::openInput(route.reverbIrFile)) reader.reset(formats.createReaderFor(std::move(stream)));
            if (reader == nullptr || reader->numChannels < 1 || reader->lengthInSamples < 8)
                route.reverbError = "The impulse response could not be read.";
            else
            {
                // Up to 12 s of IR (longer tails cost render time for no audible gain);
                // "decay" shortens it further. The used IR is then faded to zero over
                // the last "fade-out" share of its length (100 % = from its very
                // start), like release cutting, so the tail never stops abruptly.
                const int64_t maxFrames = (int64_t)(12.0 * reader->sampleRate);
                int frames = (int)juce::jmin<int64_t>(reader->lengthInSamples, maxFrames);
                const float decay = juce::jlimit(5.0f, 100.0f, route.reverbTone.decayPercent) / 100.0f;
                frames = juce::jmax(64, (int)std::lround(frames * decay));
                const double fullSeconds = frames / juce::jmax(1.0, reader->sampleRate);
                int channels = (int)reader->numChannels;
                juce::AudioBuffer<float> ir(channels, frames);
                reader->read(&ir, 0, frames, 0, true, true);
                // Fade-out 0 % leaves the IR exactly as recorded (its own tail is the
                // best one); it is meant for IRs shortened with "decay".
                const float fadeShare = juce::jlimit(0.0f, 100.0f, route.reverbTone.fadePercent) / 100.0f;
                const int fadeFrames = juce::jlimit(0, frames, (int)std::lround(frames * fadeShare));
                if (fadeFrames > 0)
                {
                    // A gentle S-curve (squared raised cosine): level at the start, the
                    // main descent in the middle (-12 dB halfway, -40 dB at 80 %), then a
                    // long quiet tail landing on zero with zero slope, so nothing clicks
                    // and the room dies into silence like a church.
                    const int start = frames - fadeFrames;
                    for (int c = 0; c < channels; ++c)
                    {
                        float* d = ir.getWritePointer(c);
                        for (int i = 0; i < fadeFrames; ++i)
                        {
                            const float p = (float)i / (float)fadeFrames;
                            const float window = 0.5f * (1.0f + std::cos(juce::MathConstants<float>::pi * p));
                            d[start + i] *= window * window;
                        }
                    }
                }
                // Light processing: the IR ends where its energy has fallen 60 dB (a
                // church IR of 7 s is there after 3.5 s; what follows is inaudible
                // under the organ) with a 100 ms fade, and a true stereo IR keeps its
                // direct channels only (LL, RR). Half the memory and work, or less.
                const bool lightMode = reverbLight.load(std::memory_order_relaxed);
                bool crossTermsDropped = false;
                if (lightMode)
                {
                    double total = 0.0;
                    for (int c = 0; c < channels; ++c)
                    {
                        const float* d = ir.getReadPointer(c);
                        for (int i = 0; i < frames; ++i) total += (double)d[i] * d[i];
                    }
                    if (total > 1.0e-12)
                    {
                        double remaining = 0.0;
                        int cut = frames;
                        for (int i = frames - 1; i >= 0; --i)
                        {
                            for (int c = 0; c < channels; ++c) { const float v = ir.getReadPointer(c)[i]; remaining += (double)v * v; }
                            if (remaining >= total * 1.0e-6) { cut = i + 1; break; }
                        }
                        const int fade = (int)(0.1 * reader->sampleRate);
                        cut = juce::jmin(frames, cut + fade);
                        if (cut < frames)
                        {
                            const int start = juce::jmax(0, cut - fade);
                            for (int c = 0; c < channels; ++c)
                            {
                                float* d = ir.getWritePointer(c);
                                for (int i = start; i < cut; ++i)
                                {
                                    const float p = (float)(i - start) / (float)juce::jmax(1, cut - start);
                                    d[i] *= 0.5f * (1.0f + std::cos(juce::MathConstants<float>::pi * p));
                                }
                            }
                            frames = cut;
                            ir.setSize(channels, frames, true, false, true);
                        }
                    }
                    if (channels >= 4)
                    {
                        juce::AudioBuffer<float> direct(2, frames);
                        direct.copyFrom(0, 0, ir, 0, 0, frames);
                        direct.copyFrom(1, 0, ir, 3, 0, frames);
                        ir = std::move(direct);
                        channels = 2;
                        crossTermsDropped = true;
                    }
                }
                // One normalisation for the whole IR: unit energy per output channel.
                double energy = 0.0;
                for (int c = 0; c < channels; ++c)
                {
                    const float* d = ir.getReadPointer(c);
                    for (int i = 0; i < frames; ++i) energy += (double)d[i] * d[i];
                }
                const bool trueStereo = channels >= 4;
                energy /= trueStereo ? 2.0 : (channels >= 2 ? 2.0 : 1.0);
                if (energy > 1.0e-12) ir.applyGain((float)(1.0 / std::sqrt(energy)));

                fresh = std::make_unique<RouteReverb>();
                fresh->trueStereo = trueStereo;
                fresh->channels = channels;
                fresh->seconds = fullSeconds;
                fresh->usedSeconds = frames / juce::jmax(1.0, reader->sampleRate);
                fresh->irSampleRate = reader->sampleRate;
                fresh->preparedRate = juce::jmax(8000.0, route.sampleRate.load(std::memory_order_relaxed));
                fresh->preparedBlock = juce::jlimit(32, RouteReverb::kMaxBlock, route.blockSize.load(std::memory_order_relaxed));
                fresh->wet.store(juce::jlimit(0.0f, 1.0f, route.reverbWet), std::memory_order_relaxed);
                fresh->highCutHz.store(route.reverbTone.highCutHz, std::memory_order_relaxed);
                fresh->lowCutHz.store(route.reverbTone.lowCutHz, std::memory_order_relaxed);
                fresh->preDelayMs.store(route.reverbTone.preDelayMs, std::memory_order_relaxed);
                fresh->wetGain.store(juce::Decibels::decibelsToGain(juce::jlimit(-40.0f, 12.0f, route.reverbTone.wetDb)), std::memory_order_relaxed);
                const int delaySize = juce::nextPowerOfTwo((int)(RouteReverb::kMaxPreDelaySeconds * fresh->preparedRate) + RouteReverb::kMaxBlock);
                const int pairCount = juce::jmax(1, juce::countNumberOfBits((juce::uint64)route.channelMask.load(std::memory_order_relaxed)) / 2);
                // Light: the convolution runs at a quarter of the rate (half when the
                // high cut lies above 2.5 kHz, so that the wet band stays intact);
                // the engines take blocks of the decimated length.
                fresh->light = lightMode;
                fresh->crossTermsDropped = crossTermsDropped;
                fresh->rateDivider = lightMode ? (route.reverbTone.highCutHz <= 2500.0f ? 4 : 2) : 1;
                fresh->convolutionBlock = (fresh->preparedBlock + fresh->rateDivider - 1) / fresh->rateDivider + (fresh->rateDivider > 1 ? 1 : 0);
                const double convolutionRate = fresh->preparedRate / fresh->rateDivider;
                const juce::dsp::ProcessSpec spec{ convolutionRate, (juce::uint32)fresh->convolutionBlock, 2 };
                auto stereoCopy = [&](int c0, int c1)
                {
                    juce::AudioBuffer<float> copy(2, frames);
                    copy.copyFrom(0, 0, ir, juce::jmin(c0, channels - 1), 0, frames);
                    copy.copyFrom(1, 0, ir, juce::jmin(c1, channels - 1), 0, frames);
                    return copy;
                };
                fresh->irSamples = (int)std::ceil(fresh->seconds * fresh->preparedRate);
                // The tail engines do their large FFT every kHeadSize samples. Feeding
                // each engine a different number of zero blocks first spreads those
                // bursts over different callbacks instead of piling them into one.
                const int blocksPerPeriod = juce::jmax(1, RouteReverb::kHeadSize / fresh->convolutionBlock);
                const int routeOrdinal = juce::jmax(0, routeIndexOf(&route));
                juce::AudioBuffer<float> zeros(2, fresh->convolutionBlock);
                zeros.clear();
                auto stagger = [&](juce::dsp::Convolution& convolution, int blocks)
                {
                    for (int b = 0; b < blocks; ++b)
                    {
                        zeros.clear();
                        juce::dsp::AudioBlock<float> block(zeros);
                        juce::dsp::ProcessContextReplacing<float> context(block);
                        convolution.process(context);
                    }
                };
                for (int p = 0; p < pairCount; ++p)
                {
                    auto pair = std::make_unique<RouteReverb::Pair>();
                    for (auto& ring : pair->delay) ring.assign((size_t)delaySize, 0.0f);
                    pair->delayMask = delaySize - 1;
                    pair->left.loadImpulseResponse(stereoCopy(0, 1), reader->sampleRate,
                        juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no,   // no -80 dB cut of the quiet tail
                        juce::dsp::Convolution::Normalise::no);
                    pair->left.prepare(spec);   // makes the IR active synchronously
                    stagger(pair->left, ((routeOrdinal + p) * 5) % blocksPerPeriod);
                    if (trueStereo)
                    {
                        pair->right.loadImpulseResponse(stereoCopy(2, 3), reader->sampleRate,
                            juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no,   // no -80 dB cut of the quiet tail
                            juce::dsp::Convolution::Normalise::no);
                        pair->right.prepare(spec);
                        stagger(pair->right, ((routeOrdinal + p) * 5 + blocksPerPeriod / 2) % blocksPerPeriod);
                    }
                    if (fresh->rateDivider > 1)
                    {
                        // Anti-alias before the decimation and image rejection after the
                        // interpolation: an 8th-order Butterworth at 0.4 of the reduced
                        // rate (4.8 kHz at a quarter of 48 kHz; the wet path's high cut
                        // of 2 kHz lies well below), four biquads with the Butterworth Qs.
                        const float cutoff = (float)(0.4 * convolutionRate);
                        static const float qs[4]{ 0.50979558f, 0.60134489f, 0.89997622f, 2.56291545f };
                        for (int ch = 0; ch < 2; ++ch)
                            for (int k = 0; k < 4; ++k)
                            {
                                pair->downFilter[ch][k].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(fresh->preparedRate, cutoff, qs[k]);
                                pair->upFilter[ch][k].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(fresh->preparedRate, cutoff, qs[k]);
                                pair->downFilter[ch][k].reset();
                                pair->upFilter[ch][k].reset();
                            }
                        pair->down.setSize(2, RouteReverb::kMaxBlock / fresh->rateDivider + 8);
                        pair->down.clear();
                    }
                    fresh->pairs.push_back(std::move(pair));
                }
                // The cost of this reverb on this computer: two tail periods of noise
                // through every engine, timed - the ms of one thread per second of
                // audio. The automatic choice of Light processing adds the routes up.
                {
                    const int blocks = juce::jmax(1, 2 * RouteReverb::kHeadSize / fresh->convolutionBlock);
                    juce::AudioBuffer<float> noise(2, fresh->convolutionBlock), work(2, fresh->convolutionBlock);
                    juce::Random random(1234);
                    for (int c = 0; c < 2; ++c)
                        for (int i = 0; i < fresh->convolutionBlock; ++i) noise.setSample(c, i, random.nextFloat() * 0.2f - 0.1f);
                    const double start = juce::Time::getMillisecondCounterHiRes();
                    for (auto& pair : fresh->pairs)
                        for (int b = 0; b < blocks; ++b)
                        {
                            work.makeCopyOf(noise, true);
                            juce::dsp::AudioBlock<float> block(work);
                            juce::dsp::ProcessContextReplacing<float> context(block);
                            pair->left.process(context);
                            if (trueStereo)
                            {
                                work.makeCopyOf(noise, true);
                                juce::dsp::AudioBlock<float> block2(work);
                                juce::dsp::ProcessContextReplacing<float> context2(block2);
                                pair->right.process(context2);
                            }
                        }
                    const double elapsed = juce::Time::getMillisecondCounterHiRes() - start;
                    const double audioSeconds = (double)blocks * fresh->convolutionBlock * fresh->rateDivider / juce::jmax(1.0, fresh->preparedRate);
                    fresh->costMsPerSecond = audioSeconds > 0.0 ? elapsed / audioSeconds : 0.0;
                    // Clean engines again, staggered as before.
                    int ordinal = 0;
                    for (auto& pair : fresh->pairs)
                    {
                        pair->left.reset();
                        stagger(pair->left, ((routeOrdinal + ordinal) * 5) % blocksPerPeriod);
                        if (trueStereo)
                        {
                            pair->right.reset();
                            stagger(pair->right, ((routeOrdinal + ordinal) * 5 + blocksPerPeriod / 2) % blocksPerPeriod);
                        }
                        ++ordinal;
                    }
                }
            }
        }
        // The buffers of the asynchronous convolution (the task reads irIn, writes
        // irOut): sized for the pairs before the swap, never on the audio thread.
        const int pairsWanted = fresh != nullptr ? (int)fresh->pairs.size() : 0;
        // Sized for the largest device block the engine accepts (RouteReverb::kMaxBlock),
        // not for the render chunk: the callback hands over the WHOLE device block.
        // With kMaxBlockFrames (1024) a buffer of 2048 overflowed the heap here
        // (crash in the convolution, corrupted heap elsewhere - macOS, RME, 2048).
        juce::AudioBuffer<float> inBuffer(juce::jmax(2, 2 * pairsWanted), RouteReverb::kMaxBlock), outBuffer(juce::jmax(2, 2 * pairsWanted), RouteReverb::kMaxBlock);
        inBuffer.clear(); outBuffer.clear();
        {
            const juce::SpinLock::ScopedLockType lk(route.renderLock);
            const juce::SpinLock::ScopedLockType gl(route.reverbGroupLock);   // the followers of a shared IR touch irIn in their own renders
            // A convolution of the previous block may still run on a worker: it
            // uses the reverb about to go, so it finishes first.
            if (route.irTaskIndex >= 0) pool->waitTask(route.irTaskIndex);
            route.irTaskIndex = -1;
            route.irOutValid = false;
            route.reverbGroupArrived = 0;
            route.irInFrames = route.irOutFrames = 0;
            std::swap(route.irIn, inBuffer);
            std::swap(route.irOut, outBuffer);
            std::swap(route.reverb, fresh);
        }
        // 'fresh' now holds the previous reverb and is destroyed here, outside the lock.
    }

    // Shared IR: the roles of the routes after any change of a route's device,
    // channels or IR settings (message thread). Routes of one device with the
    // same output channels and the same IR settings form a group: the first
    // (by route index) owns the engines, the others follow it. Owners without
    // engines (new, or followers until now) are built, followers drop theirs;
    // 'changed' is rebuilt as well when 'rebuildChanged' says so (its IR or its
    // decay changed). Sharing is exact: convolution is linear, the group's wet
    // is the sum of its routes' wets, and all of them go to the same channels.
    void regroupReverbs(Route* changed, bool rebuildChanged)
    {
        std::vector<Route*> rebuild;
        std::map<juce::String, std::vector<Route*>> groups;
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            Route& r = *routes[(size_t)i];
            if (r.reverbIrFile == juce::File() || r.deviceKey.isEmpty()) continue;
            const auto& t = r.reverbTone;
            const juce::String key = r.deviceKey + "|" + juce::String((juce::int64)r.channelMask.load(std::memory_order_relaxed)) + "|"
                + r.reverbIrFile.getFullPathName() + "|" + juce::String(r.reverbWet, 4) + "|" + juce::String(t.highCutHz, 1) + "|"
                + juce::String(t.lowCutHz, 1) + "|" + juce::String(t.preDelayMs, 2) + "|" + juce::String(t.wetDb, 2) + "|"
                + juce::String(t.decayPercent, 1) + "|" + juce::String(t.fadePercent, 1);
            groups[key].push_back(&r);
        }
        std::map<Route*, std::pair<Route*, int>> desired;   // route -> its owner (nullptr = itself) and the group's size
        for (auto& group : groups)
            for (size_t m = 0; m < group.second.size(); ++m)
                desired[group.second[m]] = { m == 0 ? nullptr : group.second[0], (int)group.second.size() };
        int shared = 0;
        for (int i = 0; i < count; ++i)
        {
            Route& r = *routes[(size_t)i];
            const auto found = desired.find(&r);
            Route* groupOwner = found != desired.end() ? found->second.first : nullptr;
            const int size = found != desired.end() ? found->second.second : 1;
            const bool wasFollower = r.reverbOwner != nullptr;
            if (groupOwner != r.reverbOwner || r.reverbGroupSize != size)
            {
                const juce::SpinLock::ScopedLockType lk(r.renderLock);
                const juce::SpinLock::ScopedLockType gl(r.reverbGroupLock);
                if (r.irTaskIndex >= 0) pool->waitTask(r.irTaskIndex);
                r.irTaskIndex = -1;
                r.reverbOwner = groupOwner;
                r.reverbGroupSize = size;
                r.reverbGroupArrived = 0;
                r.reverbGroupSeenBlock = 0;
                r.irOutValid = false;
            }
            if (groupOwner != nullptr)
            {
                ++shared;
                if (r.reverb != nullptr) rebuild.push_back(&r);   // installs none: the follower's engines go
            }
            else if (r.reverbIrFile != juce::File() && (r.reverb == nullptr || wasFollower || (rebuildChanged && &r == changed)))
                rebuild.push_back(&r);
        }
        for (auto* r : rebuild) rebuildRouteReverb(*r);
        if (shared != lastSharedReverbs)
        {
            lastSharedReverbs = shared;
            int convolutions = 0;
            for (int i = 0; i < count; ++i) if (routes[(size_t)i]->reverb != nullptr && !routes[(size_t)i]->reverb->pairs.empty()) ++convolutions;
            CrashHandler::note("IR reverbs: " + juce::String(convolutions + shared) + " route(s) with an IR in " + juce::String(convolutions)
                + " convolution(s) - " + juce::String(shared) + " route(s) share the convolution of another route of the same device, channels and IR settings");
        }
    }
    int lastSharedReverbs = -1;

    // Worker side of the IR reverb: the dry block handed over in irIn is
    // convolved and shaped into irOut (wet only). One task per route at a time.
    static void convolveRouteTask(void* ctx) noexcept
    {
        auto* task = static_cast<Route::IrTask*>(ctx);
        static_cast<Impl*>(task->impl)->convolveRoute(*task->route);
    }

    void convolveRoute(Route& route) noexcept
    {
        RouteReverb* rv = route.reverb.get();
        if (rv == nullptr || rv->pairs.empty()) { route.irOutValid = false; return; }
        juce::ScopedNoDenormals noDenormals;
        const int64_t taskStart = juce::Time::getHighResolutionTicks();
        const int totalSamples = route.irInFrames;
        if (totalSamples <= 0) { route.irOutValid = false; return; }
        const float wet = rv->wet.load(std::memory_order_relaxed);
        // Wet-path shaping coefficients for this block. The cuts follow the dry/wet
        // ratio: nothing is cut at 0 % wet, the set frequencies apply at 100 %
        // (the high cut moves on a logarithmic scale from 20 kHz down to the set
        // value, the low cut rises linearly from 0 Hz).
        const float rate = (float)juce::jmax(8000.0, rv->preparedRate);
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float setHighCut = juce::jlimit(200.0f, 20000.0f, rv->highCutHz.load(std::memory_order_relaxed));
        const float highCut = std::exp(std::log(20000.0f) + (std::log(setHighCut) - std::log(20000.0f)) * wet);
        const float lpCoef = 1.0f - std::exp(-twoPi * juce::jmin(highCut, rate * 0.45f) / rate);
        const float lowCut = rv->lowCutHz.load(std::memory_order_relaxed) * wet;
        const float hpCoef = lowCut > 5.0f ? 1.0f - std::exp(-twoPi * juce::jlimit(5.0f, 2000.0f, lowCut) / rate) : 0.0f;
        // The wet path runs one block behind the dry: that block is part of the
        // pre-delay, so the ring adds only what is left of the set pre-delay.
        const int preDelay = juce::jmax(0, juce::jlimit(0, (int)(RouteReverb::kMaxPreDelaySeconds * rate),
            (int)std::lround(rv->preDelayMs.load(std::memory_order_relaxed) * 0.001f * rate)) - totalSamples);
        auto shapeWet = [&](RouteReverb::Pair& pair, int channel, float* wetData, int n)
        {
            float lp = pair.lp[channel], hpState = pair.hp[channel];
            float* ring = pair.delay[channel].data();
            const int mask = pair.delayMask;
            int write = pair.delayWrite;
            for (int i = 0; i < n; ++i)
            {
                float x = wetData[i];
                if (preDelay > 0 && mask > 0)
                {
                    ring[(size_t)(write & mask)] = x;
                    x = ring[(size_t)((write - preDelay) & mask)];
                    ++write;
                }
                if (hpCoef > 0.0f) { hpState += hpCoef * (x - hpState); x -= hpState; }   // low cut
                lp += lpCoef * (x - lp);                                                      // high cut (distance)
                wetData[i] = lp;
            }
            pair.lp[channel] = lp;
            pair.hp[channel] = hpState;
            if (channel == 1) pair.delayWrite = write;   // both channels advance together
        };
        const int pairCount = juce::jmin((int)rv->pairs.size(), route.irIn.getNumChannels() / 2, route.irOut.getNumChannels() / 2);
        for (int p = 0; p < pairCount; ++p)
        {
            auto& pair = *rv->pairs[(size_t)p];
            const float* L = route.irIn.getReadPointer(2 * p);
            const float* R = route.irIn.getReadPointer(2 * p + 1);
            float* wetL = route.irOut.getWritePointer(2 * p);
            float* wetR = route.irOut.getWritePointer(2 * p + 1);
            // Silence gate: nothing to convolve once the input has been silent for
            // longer than the IR (the tail has died out). Wake up with clean engines.
            {
                const auto rangeL = juce::FloatVectorOperations::findMinAndMax(L, totalSamples);
                const auto rangeR = juce::FloatVectorOperations::findMinAndMax(R, totalSamples);
                const float peak = juce::jmax(std::abs(rangeL.getStart()), std::abs(rangeL.getEnd()),
                    std::abs(rangeR.getStart()), std::abs(rangeR.getEnd()));
                if (peak < 1.0e-7f)
                {
                    pair.silentSamples += totalSamples;
                    if (pair.silentSamples > rv->irSamples + 2 * RouteReverb::kHeadSize)
                    {
                        pair.sleeping = true;
                        juce::FloatVectorOperations::clear(wetL, totalSamples);
                        juce::FloatVectorOperations::clear(wetR, totalSamples);
                        continue;
                    }
                }
                else
                {
                    pair.silentSamples = 0;
                    if (pair.sleeping)
                    {
                        pair.left.reset();
                        if (rv->trueStereo) pair.right.reset();
                        pair.sleeping = false;
                    }
                }
            }
            const int chunk = juce::jlimit(1, RouteReverb::kMaxBlock, rv->preparedBlock);   // never more than prepared for
            for (int offset = 0; offset < totalSamples; offset += chunk)
            {
                const int n = juce::jmin(chunk, totalSamples - offset);
                float* s0 = rv->scratch.getWritePointer(0);
                float* s1 = rv->scratch.getWritePointer(1);
                float* s2 = rv->scratch.getWritePointer(2);
                float* s3 = rv->scratch.getWritePointer(3);
                if (rv->rateDivider > 1)
                {
                    // Light: the input low-passed and every D-th sample taken, the
                    // convolution at the reduced rate, the wet result zero-stuffed and
                    // low-passed back to the full rate. The phases run on across
                    // blocks, so the decimated count of a block equals its
                    // interpolated count whatever the block length.
                    const int D = rv->rateDivider;
                    float* dl = pair.down.getWritePointer(0);
                    float* dr = pair.down.getWritePointer(1);
                    const int capacity = pair.down.getNumSamples();
                    int k = 0, phase = pair.downPhase;
                    for (int i = 0; i < n; ++i)
                    {
                        float xl = L[offset + i], xr = R[offset + i];
                        for (int f = 0; f < 4; ++f) { xl = pair.downFilter[0][f].processSample(xl); xr = pair.downFilter[1][f].processSample(xr); }
                        if (phase == 0 && k < capacity) { dl[k] = xl; dr[k] = xr; ++k; }
                        if (++phase >= D) phase = 0;
                    }
                    pair.downPhase = phase;
                    if (k > 0)
                    {
                        float* a[2]{ dl, dr };
                        juce::dsp::AudioBlock<float> blockA(a, 2, (size_t)k);
                        juce::dsp::ProcessContextReplacing<float> ctxA(blockA);
                        pair.left.process(ctxA);   // a true stereo IR is reduced to LL/RR in Light: one engine
                    }
                    int j = 0;
                    phase = pair.upPhase;
                    const float gain = (float)D;
                    for (int i = 0; i < n; ++i)
                    {
                        float yl = 0.0f, yr = 0.0f;
                        if (phase == 0 && j < k) { yl = dl[j] * gain; yr = dr[j] * gain; ++j; }
                        if (++phase >= D) phase = 0;
                        for (int f = 0; f < 4; ++f) { yl = pair.upFilter[0][f].processSample(yl); yr = pair.upFilter[1][f].processSample(yr); }
                        s0[i] = yl;
                        s1[i] = yr;
                    }
                    pair.upPhase = phase;
                }
                else if (rv->trueStereo)
                {
                    juce::FloatVectorOperations::copy(s0, L + offset, n);
                    juce::FloatVectorOperations::copy(s1, L + offset, n);
                    juce::FloatVectorOperations::copy(s2, R + offset, n);
                    juce::FloatVectorOperations::copy(s3, R + offset, n);
                    float* a[2]{ s0, s1 };
                    float* b[2]{ s2, s3 };
                    juce::dsp::AudioBlock<float> blockA(a, 2, (size_t)n), blockB(b, 2, (size_t)n);
                    juce::dsp::ProcessContextReplacing<float> ctxA(blockA), ctxB(blockB);
                    pair.left.process(ctxA);
                    pair.right.process(ctxB);
                    juce::FloatVectorOperations::add(s0, s2, n);   // wet L = LL*L + RL*R
                    juce::FloatVectorOperations::add(s1, s3, n);   // wet R = LR*L + RR*R
                }
                else
                {
                    juce::FloatVectorOperations::copy(s0, L + offset, n);
                    juce::FloatVectorOperations::copy(s1, R + offset, n);
                    float* a[2]{ s0, s1 };
                    juce::dsp::AudioBlock<float> blockA(a, 2, (size_t)n);
                    juce::dsp::ProcessContextReplacing<float> ctxA(blockA);
                    pair.left.process(ctxA);
                }
                shapeWet(pair, 0, s0, n);
                shapeWet(pair, 1, s1, n);
                juce::FloatVectorOperations::copy(wetL + offset, s0, n);
                juce::FloatVectorOperations::copy(wetR + offset, s1, n);
            }
        }
        // Runtime guard: a convolution that takes more than 60 % of its period
        // twelve times in a row is an overload - the automatic Light processing
        // answers it (MainComponent reads the flag).
        {
            const double elapsedMs = juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks() - taskStart) * 1000.0;
            const double periodMs = 1000.0 * totalSamples / juce::jmax(1.0, rv->preparedRate);
            if (elapsedMs > 0.6 * periodMs)
            {
                if (++rv->slowTasks >= 12) { reverbOverload.store(true, std::memory_order_relaxed); rv->slowTasks = 0; }
            }
            else rv->slowTasks = 0;
        }
        route.irOutFrames = totalSamples;
        route.irOutValid = true;
    }

    // Render thread: the IR reverb of the route's output pairs, one block behind.
    // The wet signal of the previous block (convolved by a worker meanwhile) is
    // mixed into this block's dry output; this block's dry output is handed to
    // the worker for the next one. The convolutions of all routes thus run in
    // parallel on the pool instead of one after another on the callback thread
    // (ten routes with a 4-channel IR at a 256 buffer cost 3 ms of a 5.3 ms
    // period that way, with tail-FFT bursts far beyond it).
    void applyRouteReverb(Route& route, juce::AudioBuffer<float>& buffer, const BlockContext& ctx, int numOuts, int totalSamples) noexcept
    {
        // Shared IR: the engines and the hand-over buffers live in the group's
        // owner; a follower only adds its dry signal there. The routes of one
        // device render one after another in its callback: the first of the group
        // to render in a block takes the previous block's wet of the whole group
        // into its own output (every route of the group goes to the same channels,
        // so the sum is the same) and starts the new input; the last posts the task.
        Route& host = route.reverbOwner != nullptr ? *route.reverbOwner : route;
        // Taken before the owner's engines are looked at: a follower's render holds
        // its own renderLock only, and the owner's reverb is swapped under this lock too.
        const juce::SpinLock::ScopedLockType groupLock(host.reverbGroupLock);
        RouteReverb* rv = host.reverb.get();
        if (rv == nullptr || rv->pairs.empty()) return;
        const float wet = rv->wet.load(std::memory_order_relaxed);
        if (wet <= 0.0005f) return;
        if (host.irIn.getNumChannels() < 2 || host.irOut.getNumChannels() < 2) return;
        // A block the hand-over buffers cannot hold (never, the callback drops
        // such blocks; a second guard all the same): this block stays dry.
        if (totalSamples > host.irIn.getNumSamples() || totalSamples > host.irOut.getNumSamples()) return;
        const float dry = 1.0f - wet;
        const float wetGain = wet * rv->wetGain.load(std::memory_order_relaxed);
        const bool sharedGroup = host.reverbGroupSize > 1;
        bool first = true;
        if (sharedGroup)
        {
            // A route arriving for the second time in one block means a new block
            // whose predecessor the group never completed (a route of it skipped
            // its render): what did arrive is convolved now, the new block starts.
            if (host.reverbGroupArrived > 0 && route.reverbGroupSeenBlock == host.reverbGroupBlockId)
            {
                postRouteConvolution(host);
                host.reverbGroupArrived = 0;
            }
            first = host.reverbGroupArrived == 0;
            if (first) ++host.reverbGroupBlockId;
            route.reverbGroupSeenBlock = host.reverbGroupBlockId;
        }
        // The previous block's convolution must be over (it had a whole period).
        if (first && host.irTaskIndex >= 0) waitIrTask(host);
        const int pairCount = juce::jmin((int)rv->pairs.size(), host.irIn.getNumChannels() / 2, host.irOut.getNumChannels() / 2);
        const bool mixPrevious = first && host.irOutValid && host.irOutFrames == totalSamples;
        int pairIndex = 0;
        for (int ci = 0; ci + 1 < ctx.outCount && pairIndex < pairCount; ci += 2, ++pairIndex)
        {
            const int chL = ctx.outChannels[ci], chR = ctx.outChannels[ci + 1];
            if (chL < 0 || chL >= numOuts || chR < 0 || chR >= numOuts) continue;
            float* L = buffer.getWritePointer(chL);
            float* R = buffer.getWritePointer(chR);
            // This block's dry output goes to the worker (the group's sum)...
            if (first)
            {
                juce::FloatVectorOperations::copy(host.irIn.getWritePointer(2 * pairIndex), L, totalSamples);
                juce::FloatVectorOperations::copy(host.irIn.getWritePointer(2 * pairIndex + 1), R, totalSamples);
            }
            else
            {
                juce::FloatVectorOperations::add(host.irIn.getWritePointer(2 * pairIndex), L, totalSamples);
                juce::FloatVectorOperations::add(host.irIn.getWritePointer(2 * pairIndex + 1), R, totalSamples);
            }
            // ... and the previous block's wet signal (of the whole group) comes out with its first route.
            if (mixPrevious)
            {
                const float* wetL = host.irOut.getReadPointer(2 * pairIndex);
                const float* wetR = host.irOut.getReadPointer(2 * pairIndex + 1);
                for (int i = 0; i < totalSamples; ++i)
                {
                    L[i] = L[i] * dry + wetL[i] * wetGain;
                    R[i] = R[i] * dry + wetR[i] * wetGain;
                }
            }
            else
            {
                juce::FloatVectorOperations::multiply(L, dry, totalSamples);
                juce::FloatVectorOperations::multiply(R, dry, totalSamples);
            }
        }
        host.irInFrames = totalSamples;
        if (first) host.irOutValid = false;
        if (sharedGroup && ++host.reverbGroupArrived < host.reverbGroupSize) return;   // the others of the group still to come; the last one posts
        host.reverbGroupArrived = 0;
        postRouteConvolution(host);
    }

    // The convolution of the dry block in host.irIn goes to a worker (the
    // previous one is over: waited above). Render thread, host.reverbGroupLock held.
    // The wait for the group's convolution task (timed for the device thread's profile).
    void waitIrTask(Route& host) noexcept
    {
        const bool prof = gVoiceProfiling.load(std::memory_order_relaxed);
        const int64_t w0 = prof ? profileTicks() : 0;
        pool->waitTask(host.irTaskIndex);
        if (prof) tlReverbWaitTicks += profileDelta(w0, profileTicks());
    }
    void postRouteConvolution(Route& host) noexcept
    {
        if (host.irTaskIndex >= 0) waitIrTask(host);
        host.irTask.impl = this;
        host.irTask.route = &host;
        const int index = routeIndexOf(&host);
        host.irTaskIndex = index;
        if (index < 0 || !pool->postTask(index, &Impl::convolveRouteTask, &host.irTask))
        {
            // No slot (cannot happen: one per route) - convolve here, as before.
            host.irTaskIndex = -1;
            convolveRoute(host);
        }
    }

    //--------------------------------------------------------------------------
    // Voice pool (control side)
    Voice* allocVoice(uint32_t& indexOut) noexcept
    {
        if (freeList.empty()) return nullptr;
        indexOut = freeList.back();
        freeList.pop_back();
        Voice& v = voices[indexOut];
        // reset render-private and control-private state
        v.pos = 0; v.lastInc = 0; v.lastGain = 0.0f;
        v.started = v.finished = v.fadingOut = v.killing = false;
        v.attackActive = v.exprFrozen = v.inCrossfade = false;
        v.tailBlend = false; v.tailBlendEndF = v.tailBlendLenF = v.tailLoopLenF = 0;
        v.fadeInRemaining = v.fadeOutRemaining = v.fadeOutTotal = 0;
        v.attackElapsed = v.couplingElapsed = 0;
        v.frozenExpr = 1.0f;
        v.nextLoopOrder = 0; v.targetLoop = -1;
        v.loopDecided = v.loopTake = 0;
        v.rng = (uint32_t)tuningRandom.nextInt() | 1u;
        v.command.store(0, std::memory_order_relaxed);
        v.curRandomPitch.store(0.0f, std::memory_order_relaxed);
        v.curWindPitch.store(0.0f, std::memory_order_relaxed);
        v.curTremPitch.store(0.0f, std::memory_order_relaxed);
        v.controlIndex = -1; v.keyPrev = v.keyNext = -1;
        v.ctlReleasing = v.ctlStolen = false;
        v.detachedRelease = false; v.loopEnabled = true; v.couplingUsed = false; v.startFrozen = false;
        v.windFrozen = false; v.frozenWindPitch = 0.0f; v.frozenWindGain = v.lastWindGain = v.initialWindGain = 1.0f;
        v.curWindGain.store(1.0f, std::memory_order_relaxed);
        std::fill(&v.shelfState[0][0], &v.shelfState[0][0] + 8, 0.0f); v.lastShelf = v.shelfSmoothed = 0.0f;
        v.keyNote = v.sampleNote = 0; v.shift = 0; v.cubic = false; v.bus = 0; v.fullKernel = false;
        v.shiftCents.store(0.0f, std::memory_order_relaxed);
        v.routeGainTarget.store(1.0f, std::memory_order_relaxed);
        v.routeGain = v.routeGainFrom = v.routeGainLastTarget = 1.0f;
        v.routeGainRemaining = v.routeGainTotal = 0;
        v.toneTarget.store(0.0f, std::memory_order_relaxed);
        v.toneStartHz = owner.toneStartHz.load(std::memory_order_relaxed);
        v.fillGain = 1.0f; v.toneOffset = 0.0f;
        v.attackToneDb = 0.0f; v.attackToneStepPerFrame = 0.0f;
        v.toneDb = 0.0f; v.toneCoeffDb = 1.0e9f; v.toneCoeffRate = 0.0f;
        v.toneState[0][0] = v.toneState[0][1] = v.toneState[1][0] = v.toneState[1][1] = 0.0f;
        return &v;
    }

    void publishVoice(uint32_t idx, Route& route) noexcept
    {
        Voice& v = voices[idx];
        v.creationOrder = nextCreationOrder++;
        v.controlIndex = (int32_t)controlActive.size();
        controlActive.push_back(idx);
        refreshStealable(v);
        owner.controlVoicesStarted.fetch_add(1, std::memory_order_relaxed);
        const int key = v.sourceKey;
        if (key >= 0 && key < kSourceKeys)
        {
            v.keyNext = keyHeads[(size_t)key];
            v.keyPrev = -1;
            if (v.keyNext >= 0) voices[(size_t)v.keyNext].keyPrev = (int32_t)idx;
            keyHeads[(size_t)key] = (int32_t)idx;
        }
        const uint32_t tail = route.addTail.load(std::memory_order_relaxed);
        const uint32_t head = route.addHead.load(std::memory_order_acquire);
        const uint32_t cap = (uint32_t)route.addRing.size();
        if (tail - head >= cap || !route.hasDevice.load(std::memory_order_acquire))
        {
            // Over the ring (cannot happen: it holds the whole pool), or a route
            // without a device ("none"): nothing would ever render the voice -
            // finished at once, reclaimed by the next tick, never counted as
            // polyphony. (The auxiliary copy, when the auxiliary route has a
            // device, was published before this - see duplicateVoiceToAux.)
            v.state.store(VoiceFinished, std::memory_order_release);
            const juce::SpinLock::ScopedLockType lk(finishedLock);
            finishedQueue.push_back(idx);
            return;
        }
        v.state.store(VoicePending, std::memory_order_release);
        route.addRing[tail % cap] = idx;
        route.addTail.store(tail + 1, std::memory_order_release);
    }

    void unlinkKey(Voice& v, uint32_t idx) noexcept
    {
        const int key = v.sourceKey;
        if (key >= 0 && key < kSourceKeys && keyHeads[(size_t)key] == (int32_t)idx)
            keyHeads[(size_t)key] = v.keyNext;
        if (v.keyPrev >= 0) voices[(size_t)v.keyPrev].keyNext = v.keyNext;
        if (v.keyNext >= 0) voices[(size_t)v.keyNext].keyPrev = v.keyPrev;
        v.keyPrev = v.keyNext = -1;
    }

    void reclaimFinished() noexcept
    {
        {
            const juce::SpinLock::ScopedLockType lk(finishedLock);
            finishedSwap.swap(finishedQueue);
        }
        for (const uint32_t idx : finishedSwap)
        {
            if (idx >= (uint32_t)poolSize) continue;
            Voice& v = voices[idx];
            if (v.state.load(std::memory_order_acquire) != VoiceFinished) continue;
            unlinkKey(v, idx);
            const int32_t ci = v.controlIndex;
            if (ci >= 0 && ci < (int32_t)controlActive.size() && controlActive[(size_t)ci] == idx)
            {
                const uint32_t moved = controlActive.back();
                controlActive[(size_t)ci] = moved;
                voices[moved].controlIndex = ci;
                controlActive.pop_back();
            }
            v.controlIndex = -1;
            v.sample = nullptr;
            v.state.store(VoiceFree, std::memory_order_release);
            freeList.push_back(idx);
        }
        finishedSwap.clear();
    }

    void sendCommand(Voice& v, uint8_t bits) noexcept
    {
        v.command.fetch_or(bits, std::memory_order_acq_rel);
    }

    void makeRoomForVoice() noexcept
    {
        const int limit = owner.maxPolyphony.load(std::memory_order_relaxed);
        if ((int)controlActive.size() < limit || stealExhausted) return;
        const int count = (int)controlActive.size();
        // A bounded scan: at the limit, with a bashed tutti stealing thousands of
        // voices per strike, nearly every voice near the cursor is stolen already
        // (it leaves the list only after its fade), and a full scan per new voice
        // made one key cost milliseconds (every step a cache miss into the pool).
        // The stolen voices free their slots within a few blocks anyway: a start
        // that finds nothing in 256 steps goes ahead, and so do the others of this
        // tick (nothing becomes stealable before the next reclaim).
        for (int scanned = 0; scanned < juce::jmin(count, 1024); ++scanned)
        {
            if (stealCursor >= count) stealCursor = 0;
            const uint32_t idx = controlActive[(size_t)stealCursor++];
            // Release tails and sustaining pipes may be stolen; a pipe already
            // fading out will free its slot by itself.
            if (stealable[idx] != 0)
            {
                Voice& v = voices[idx];
                v.ctlStolen = true;
                stealable[idx] = 0;
                sendCommand(v, CmdSteal);
                return;
            }
        }
        stealExhausted = true;
    }

    // Kill every voice; all bookkeeping cleared. Control lock must be held.
    void killAllVoices() noexcept
    {
        for (const uint32_t idx : controlActive) sendCommand(voices[idx], CmdKill);
        for (auto* stop : owner.stops)
        {
            stop->speaking.reset();
            stop->pipeStartMs.fill(-1.0);
            for (auto& rank : stop->ranks) rank->speaking.reset();
        }
        noteOnTimeMs.fill(-1.0);
        noteSoundStartMs.fill(-1.0);
        noteMinimumStartMs.fill(-1.0);
        lastAttackDelayNoteOnMs.fill(-1.0);
        noteFinishedMs.fill(-1.0);
        heldVelocity.fill(-1);
        for (auto& p : pendingOn) p.valid = false;
        for (auto& p : pendingOff) p.valid = false;
        for (int m = 0; m < maxManuals; ++m)
        {
            auto& w = windSources[(size_t)m];
            w.held.fill(false);
            w.heldWeight = 0.0f;
            w.heldCount = 0;
            w.influentialCount = 0;
            w.influentialNotes.fill(-1);
            windSnapshots[(size_t)m].write(w);
        }
    }

    // Hard reset while the engine is suspended and no render is in flight.
    void resetAllVoicesUnsafe() noexcept
    {
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            Route& r = *routes[(size_t)i];
            r.active.clear();
            r.addHead.store(r.addTail.load(std::memory_order_acquire), std::memory_order_release);
            r.activeCount.store(0, std::memory_order_relaxed);
            r.finishedScratch.clear();
        }
        {
            const juce::SpinLock::ScopedLockType lk(finishedLock);
            finishedQueue.clear();
        }
        for (const uint32_t idx : controlActive)
        {
            Voice& v = voices[idx];
            v.sample = nullptr; v.stop = nullptr; v.rank = nullptr;
            v.controlIndex = -1; v.keyPrev = v.keyNext = -1;
            v.state.store(VoiceFree, std::memory_order_release);
        }
        controlActive.clear();
        keyHeads.fill(-1);
        freeList.clear();
        for (int i = poolSize; --i >= 0;) freeList.push_back((uint32_t)i);
        owner.storePolyphony(0);
    }

    // Blocks render callbacks and waits until none is in flight. Must be paired
    // with resume(). Caller must not hold controlLock.
    void suspend()
    {
        suspended.store(true, std::memory_order_seq_cst);
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            const juce::SpinLock::ScopedLockType lk(routes[(size_t)i]->renderLock);
            if (routes[(size_t)i]->irTaskIndex >= 0) pool->waitTask(routes[(size_t)i]->irTaskIndex);   // a convolution still on a worker
        }
    }
    void resume() noexcept { suspended.store(false, std::memory_order_seq_cst); }

    //--------------------------------------------------------------------------
    // MIDI queue
    void pushMidi(const MidiCommand& command) noexcept
    {
        const juce::SpinLock::ScopedLockType lk(midiProducerLock);
        const uint32_t tail = midiTail.load(std::memory_order_relaxed);
        const uint32_t head = midiHead.load(std::memory_order_acquire);
        if (tail - head >= midiRingSize) return;   // overflow: drop
        midiRing[tail % midiRingSize] = command;
        midiTail.store(tail + 1, std::memory_order_release);
        if (controlThreadRunning.load(std::memory_order_relaxed)) controlWake.signal();
    }
    void clearMidi() noexcept
    {
        const juce::SpinLock::ScopedLockType lk(midiProducerLock);
        midiHead.store(midiTail.load(std::memory_order_acquire), std::memory_order_release);
    }
    // Control work is time-boxed: a 61-key tutti release creates thousands of
    // release voices, which must not eat a small ASIO period in one go. Whatever
    // does not fit is simply continued on the next tick (a few ms later).
    static bool pastDeadline(int64_t deadlineTicks) noexcept
    {
        return juce::Time::getHighResolutionTicks() > deadlineTicks;
    }

    void processMidi(int64_t deadlineTicks)
    {
        uint32_t head = midiHead.load(std::memory_order_relaxed);
        const uint32_t tail = midiTail.load(std::memory_order_acquire);
        int processed = 0;
        while (head != tail && processed++ < 4096)
        {
            const MidiCommand command = midiRing[head % midiRingSize];
            ++head;
            midiHead.store(head, std::memory_order_release);
            if (command.on) noteOnImmediate(command.note, command.velocity, command.manual);
            else noteOffImmediate(command.note, command.manual);
            if (pastDeadline(deadlineTicks)) break;   // a note-off of a tutti key releases dozens of voices: checked after every command
        }
    }

    //--------------------------------------------------------------------------
    // Wind model (control side)
    void publishWind(int source) noexcept
    {
        if (source >= 0 && source < maxManuals) windSnapshots[(size_t)source].write(windSources[(size_t)source]);
    }

    void updateWindKey(int sourceManual, int note, bool down)
    {
        if (sourceManual < 0 || sourceManual >= maxManuals || note < 36 || note > 69) return;
        const auto windKeyWeight = [](int midiNote)
        {
            if (midiNote <= 48) return 1.0f;
            return juce::jlimit(0.0f, 1.0f, (69.0f - (float)midiNote) / 21.0f);
        };
        const float weight = windKeyWeight(note);
        if (weight <= 0.0f) return;
        auto& state = windSources[(size_t)sourceManual];
        if (state.held[(size_t)note] == down) return;
        const double now = nowMs();
        const double baseSmoothingMs = juce::jmax(1.0, state.baseTransitionDurationMs);
        const double oldProgress = juce::jlimit(0.0, 1.0, (now - state.baseTransitionStartMs) / baseSmoothingMs);
        const float oldSmooth = (float)(oldProgress * oldProgress * (3.0 - 2.0 * oldProgress));
        const float currentBase = state.baseStartSag + (state.targetSag - state.baseStartSag) * oldSmooth;
        state.held[(size_t)note] = down;
        state.heldCount = juce::jmax(0, state.heldCount + (down ? 1 : -1));
        if (down && state.influentialCount < 4)
        {
            state.influentialNotes[(size_t)state.influentialCount++] = note;
            state.heldWeight += weight;
        }
        else if (!down)
        {
            int removed = -1;
            for (int i = 0; i < state.influentialCount; ++i)
                if (state.influentialNotes[(size_t)i] == note) { removed = i; break; }
            if (removed >= 0)
            {
                state.heldWeight -= weight;
                for (int i = removed; i + 1 < state.influentialCount; ++i)
                    state.influentialNotes[(size_t)i] = state.influentialNotes[(size_t)(i + 1)];
                --state.influentialCount;
                for (int candidate = 36; candidate <= 69 && state.influentialCount < 4; ++candidate)
                {
                    if (!state.held[(size_t)candidate]) continue;
                    bool alreadyUsed = false;
                    for (int i = 0; i < state.influentialCount; ++i)
                        alreadyUsed = alreadyUsed || state.influentialNotes[(size_t)i] == candidate;
                    if (alreadyUsed) continue;
                    const float candidateWeight = windKeyWeight(candidate);
                    if (candidateWeight <= 0.0f) continue;
                    state.influentialNotes[(size_t)state.influentialCount++] = candidate;
                    state.heldWeight += candidateWeight;
                }
            }
        }
        state.heldWeight = juce::jmax(0.0f, state.heldWeight);

        const int effectiveCount = state.influentialCount;
        const float averageWeight = effectiveCount > 0
            ? juce::jlimit(0.0f, 1.0f, state.heldWeight / (float)effectiveCount) : 0.0f;
        int engagedStops = 0;
        float effectiveStopLoad = 0.0f;
        for (auto* stop : owner.stops)
            if (stop->primaryManual == sourceManual && stop->enabled)
            {
                ++engagedStops;
                effectiveStopLoad += stop->windContribution;
            }
        // Stops: one 8' stop alone (load 1.0) produces the "one stop share" of the
        // full sag (Configuration Editor, default 20 %); more stops approach the
        // full sag exponentially (about 86 % of the way at 11 stops). Smaller
        // footages count less than a whole stop.
        const float oneStopShare = juce::jlimit(0.0f, 1.0f, owner.windOneStopShare.load(std::memory_order_relaxed));
        const float sustainedStopLoad = effectiveStopLoad > 1.0f
            ? oneStopShare + (1.0f - oneStopShare) * (1.0f - std::exp(-(effectiveStopLoad - 1.0f) / 5.0f))
            : oneStopShare * effectiveStopLoad;
        // Pipes: one sounding pipe gives the "one pipe share", two pipes the "two
        // pipes share" (defaults 50 %), from three on the rest closes geometrically.
        const float onePipeShare = juce::jlimit(0.0f, 1.0f, owner.windOnePipeShare.load(std::memory_order_relaxed));
        const float twoPipesShare = juce::jlimit(0.0f, 1.0f, owner.windTwoPipesShare.load(std::memory_order_relaxed));
        const float sustainedFraction = effectiveCount <= 0 ? 0.0f
            : effectiveCount == 1 ? onePipeShare
            : effectiveCount == 2 ? twoPipesShare
            : 1.0f - (1.0f - twoPipesShare) * std::pow(0.7f, (float)(effectiveCount - 2));
        // Wind model strength (Configuration Editor "maximum effect"): the steady
        // sag scales with it, 0 switches the model off.
        const float maxEffect = owner.windMaxEffect.load(std::memory_order_relaxed);
        const float newTarget = state.depthCents * averageWeight * sustainedFraction * sustainedStopLoad * maxEffect;
        state.baseStartSag = currentBase;
        state.targetSag = newTarget;
        state.baseTransitionStartMs = now;
        const float registrationAmount = juce::jlimit(0.0f, 1.0f, (effectiveStopLoad - 1.0f) / 9.0f);
        state.baseTransitionDurationMs = down ? state.baseSmoothingMs
            : state.baseSmoothingMs * (1.0 + (130.0 / 170.0) * registrationAmount);

        if (state.frequencyHz > 0.0f && state.oscillations > 0.0f && state.depthCents > 0.0f && maxEffect > 0.0f)
        {
            WindTransient* slot = nullptr;
            for (auto& event : state.transients)
                if (event.startMs < 0.0 || now >= event.startMs + event.durationMs) { slot = &event; break; }
            if (slot == nullptr)
                slot = &*std::min_element(state.transients.begin(), state.transients.end(),
                    [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
            const float oscillationRegistration = juce::jlimit(0.0f, 1.0f, ((float)engagedStops - 3.0f) / 7.0f);
            const float registrationDamping = 1.0f - 0.15f * oscillationRegistration;
            const float averageStopContribution = engagedStops > 0 ? effectiveStopLoad / (float)engagedStops : 0.0f;
            const float smallRegistrationContribution = juce::jmin(1.0f, effectiveStopLoad);
            const float oscillationStopContribution = smallRegistrationContribution
                + (averageStopContribution - smallRegistrationContribution) * oscillationRegistration;
            const float fullImpulse = state.depthCents * juce::jmax(0.15f, averageWeight)
                * state.oscillationStrength * registrationDamping * oscillationStopContribution;
            // No two key strokes shake the wind alike: every impulse gets a random
            // strength between 40 and 100 % of the computed one (times the model's
            // maximum effect).
            const float strokeStrength = (0.40f + 0.60f * windRandom.nextFloat()) * maxEffect;
            slot->startMs = now;
            slot->durationMs = 1000.0 * state.oscillations / state.frequencyHz;
            slot->oscillations = state.oscillations;
            slot->amplitude = fullImpulse * strokeStrength;
            slot->release = !down;
        }
        publishWind(sourceManual);
    }

    //--------------------------------------------------------------------------
    // Note logic (control side, lock held)
    // Routes of a stop: its own button (shift 0 from the home division) and its
    // enabled transmissions (division, shift). A key K of a route sounds the
    // stop's pipe K + shift. Keys without a division (-1) play every stop unshifted.
    // The transmissions a stop sounds through: its own and, for a stop folder's
    // child, those of its master (the folder is transmitted as a whole).
    template <typename Fn> static void forEachRoute(const Stop* stop, Fn&& fn)
    {
        for (const auto& t : stop->transmissions) fn(t);
        if (stop->groupMasterPtr != nullptr)
            for (const auto& t : stop->groupMasterPtr->transmissions) fn(t);
    }

    bool stopAssignedToManual(const Stop* stop, int manualIndex) const noexcept
    {
        if (manualIndex < 0 || (stop->primaryManual == manualIndex && stop->enabled)) return true;
        bool assigned = false;
        forEachRoute(stop, [&](const Stop::Transmission& t) { if (t.enabled && t.manual == manualIndex) assigned = true; });
        return assigned;
    }

    static constexpr int kMaxShift = Stop::Transmission::maxShift;
    static constexpr int kMaxRouteShifts = 2 * kMaxShift + 1;

    // Distinct shifts with which 'stop' sounds from a key of 'manualIndex'.
    int collectRouteShifts(Stop* stop, int manualIndex, std::array<int, kMaxRouteShifts>& shifts) noexcept
    {
        int count = 0;
        auto add = [&](int shift)
        {
            for (int i = 0; i < count; ++i) if (shifts[(size_t)i] == shift) return;
            if (count < kMaxRouteShifts) shifts[(size_t)count++] = shift;
            stop->shiftHistory |= 1ull << (shift + kMaxShift);
        };
        if (manualIndex < 0) { add(0); return count; }
        if (stop->primaryManual == manualIndex && stop->enabled) add(0);
        forEachRoute(stop, [&](const Stop::Transmission& t)
        {
            if (t.enabled && t.manual == manualIndex) add(juce::jlimit(-kMaxShift, kMaxShift, t.shift));
        });
        return count;
    }

    // May this key sound this pipe of the stop through one of its routes with this
    // shift? The stop's own route obeys the pipes switched off in its voicing; a
    // transmission obeys only its own switched-off keys, so a stop can be cut to
    // half its compass while its transmission still plays the whole of it.
    bool routeAllowsKey(Stop* stop, int manualIndex, int shift, int key, int pipe) noexcept
    {
        if (manualIndex < 0) return !stop->pipeDisabled.test((size_t)pipe);
        if (shift == 0 && stop->primaryManual == manualIndex && stop->enabled && !stop->pipeDisabled.test((size_t)pipe)) return true;
        bool allowed = false;
        forEachRoute(stop, [&](const Stop::Transmission& t)
        {
            if (!allowed && t.enabled && t.manual == manualIndex && juce::jlimit(-kMaxShift, kMaxShift, t.shift) == shift
                && !t.keyDisabled.test((size_t)juce::jlimit(0, 127, key)))
                allowed = true;
        });
        return allowed;
    }

    // A key is held and sounds: outside the division's compass a key is silent
    // (an extension ends with the compass). Keys without a division are not limited.
    bool keyHeldInCompass(int note, int manualIndex) const noexcept
    {
        // 0.8.156: a held key counts wherever it lies - the keys a coupler holds
        // beyond the compass sound now, so their pipes are held like any other.
        return note >= 0 && note < 128 && heldVelocity[(size_t)makeSourceKey(note, manualIndex)] >= 0;
    }
    bool keyInCompass(int note, int manualIndex) const noexcept
    {
        if (manualIndex < 0) return true;
        return note >= owner.compassLow[(size_t)manualIndex].load(std::memory_order_relaxed)
            && note <= owner.compassHigh[(size_t)manualIndex].load(std::memory_order_relaxed);
    }

    // Does any key of any active route of 'stop' still hold this pipe?
    bool pipeStillHeld(const Stop* stop, int pipe) const noexcept
    {
        if (heldVelocity[(size_t)makeSourceKey(pipe, -1)] >= 0) return true;
        if (stop->enabled && keyHeldInCompass(pipe, stop->primaryManual)) return true;
        bool held = false;
        forEachRoute(stop, [&](const Stop::Transmission& t) { if (t.enabled && keyHeldInCompass(pipe - t.shift, t.manual)) held = true; });
        return held;
    }

    // Cents a voice adds beyond its pipe's own tuning: the resampling of a filled
    // pipe (its sample belongs to a neighbouring pipe) and, for a transposed
    // route, the pure-interval correction of the new footage (relative to the
    // recorded one unless the rank is normalised to 440 Hz, where the recorded
    // mutations are already tempered).
    static float transpositionCents(const Stop* stop, const Stop* rank, int shift, int pipe, int sampleNote) noexcept
    {
        double cents = 100.0 * (pipe - sampleNote);
        if (shift != 0)
        {
            const int footage = SamplerEngine::stopFootageSemitones(rank->footageKnown ? rank : stop);
            cents += SamplerEngine::pureDeviationCents(footage + shift);
            if (!stop->tuneTo440) cents -= SamplerEngine::pureDeviationCents(footage);
        }
        return (float)cents;
    }

    // Volume of the loudest route currently holding this pipe of the stop: the
    // stop itself counts 1, a transmission its dB relative to the parent.
    float pipeRouteGain(const Stop* stop, int pipe) const noexcept
    {
        float best = -1.0f;
        if (heldVelocity[(size_t)makeSourceKey(pipe, -1)] >= 0) best = 1.0f;
        if (stop->enabled && keyHeldInCompass(pipe, stop->primaryManual)) best = juce::jmax(best, 1.0f);
        forEachRoute(stop, [&](const Stop::Transmission& t)
        {
            const int key = pipe - t.shift;
            if (t.enabled && keyHeldInCompass(key, t.manual))
                best = juce::jmax(best, juce::Decibels::decibelsToGain(t.db + t.keyDb[(size_t)key]));
        });
        return best < 0.0f ? 1.0f : best;
    }

    // A filled pipe transposed upwards: -0.1 dB of level and -0.1 dB of treble
    // per semitone (a pipe played faster sounds brighter and louder than a real
    // pipe of that pitch would). Downward transpositions and real pipes: nothing.
    static void setFillCompensation(Voice& v, int pipe, int sampleNote) noexcept
    {
        const int up = pipe - sampleNote;
        v.fillGain = up > 0 ? juce::Decibels::decibelsToGain(-0.1f * (float)up) : 1.0f;
        v.toneOffset = up > 0 ? -0.1f * (float)up : 0.0f;
    }

    // Tone of the pipe: the highest "Treble per key" setting among the routes
    // holding it (the stop's own per-pipe tone, a transmission's per-key tone).
    float pipeRouteTone(const Stop* stop, int pipe) const noexcept
    {
        bool any = false;
        float best = -100.0f;
        if (heldVelocity[(size_t)makeSourceKey(pipe, -1)] >= 0
            || (stop->enabled && keyHeldInCompass(pipe, stop->primaryManual)))
        {
            best = stop->getNoteToneDb(pipe);
            any = true;
        }
        forEachRoute(stop, [&](const Stop::Transmission& t)
        {
            const int key = pipe - t.shift;
            if (t.enabled && keyHeldInCompass(key, t.manual))
            {
                best = juce::jmax(best, t.keyTone[(size_t)key]);
                any = true;
            }
        });
        return any ? best : stop->getNoteToneDb(pipe);
    }

    // Sets the route volume target (and the tone) of every attack voice of this
    // pipe (they hang on the keys that started them: pipe - shift on any division).
    void setPipeRouteGain(Stop* stop, int pipe, float gain) noexcept
    {
        const float tone = pipeRouteTone(stop, pipe);
        for (int shiftBit = 0; shiftBit < kMaxRouteShifts; ++shiftBit)
        {
            if ((stop->shiftHistory & (1ull << shiftBit)) == 0) continue;
            const int key = pipe - (shiftBit - kMaxShift);
            if (key < 0 || key >= 128) continue;
            for (int m = -1; m < maxManuals; ++m)
                for (int32_t i = keyHeads[(size_t)makeSourceKey(key, m)]; i >= 0; i = voices[(size_t)i].keyNext)
                {
                    Voice& v = voices[(size_t)i];
                    if (v.detachedRelease || v.stop != stop || (int)v.note != pipe) continue;
                    v.routeGainTarget.store(gain, std::memory_order_relaxed);
                    v.toneTarget.store(tone, std::memory_order_relaxed);
                }
        }
    }

    // A route of 'stop' (its own button or a transmission) was switched off:
    // every pipe of it that no key holds any more is released properly (release
    // samples, no hard kill); pipes still held through another route keep sounding.
    void releaseUnheldPipes(Stop* stop)
    {
        std::array<float, maxManuals> gains{};
        for (int m = 0; m < maxManuals; ++m) gains[(size_t)m] = expressionGain[(size_t)m].load();
        for (int note = 0; note < 128; ++note)
        {
            bool speaking = stop->speaking.test((size_t)note);
            for (const auto& rank : stop->ranks) speaking = speaking || rank->speaking.test((size_t)note);
            if (!speaking) continue;
            if (pipeStillHeld(stop, note))
            {
                // Another route keeps it: its volume may have changed.
                setPipeRouteGain(stop, note, pipeRouteGain(stop, note));
                continue;
            }
            finishNoteOff(makeSourceKey(note, -1), gains, stop, false);
        }
    }

    void noteOnImmediate(int midiNote, int velocity, int manualIndex)
    {
        if (midiNote < 0 || midiNote >= 128) return;
        manualIndex = juce::jlimit(-1, maxManuals - 1, manualIndex);
        const int key = makeSourceKey(midiNote, manualIndex);
        heldVelocity[(size_t)key] = (int8_t)juce::jlimit(1, 127, velocity);
        updateWindKey(manualIndex, midiNote, true);
        const double now = nowMs();
        const float delayMs = owner.attackMinimumDelayMs.load(std::memory_order_relaxed);

        bool continuesDelaySeries = false;
        if (lastAttackDelayNoteOnMs[(size_t)key] >= 0.0)
        {
            const double intervalMs = now - lastAttackDelayNoteOnMs[(size_t)key];
            continuesDelaySeries = intervalMs >= 0.0 && intervalMs <= (double)delayMs;
        }
        lastAttackDelayNoteOnMs[(size_t)key] = now;

        double postReleaseDelayMs = -1.0;
        if (noteFinishedMs[(size_t)key] >= 0.0)
        {
            const double sinceFinish = now - noteFinishedMs[(size_t)key];
            const double fullDelayMs = (double)delayMs;
            const double halfDelayMs = fullDelayMs * 0.5;
            if (sinceFinish >= 0.0 && sinceFinish <= halfDelayMs) postReleaseDelayMs = 0.0;
            else if (sinceFinish > halfDelayMs && sinceFinish <= fullDelayMs) postReleaseDelayMs = halfDelayMs;
        }

        auto resumeVoicesOfKey = [this, key]
        {
            for (int32_t i = keyHeads[(size_t)key]; i >= 0; i = voices[(size_t)i].keyNext)
                if (!voices[(size_t)i].detachedRelease) sendCommand(voices[(size_t)i], CmdResume);
        };

        if (noteSoundStartMs[(size_t)key] >= 0.0 || pendingOff[(size_t)key].valid)
        {
            pendingOff[(size_t)key].valid = false;
            resumeVoicesOfKey();
            noteOnTimeMs[(size_t)key] = now;
            noteMinimumStartMs[(size_t)key] = now;
            return;
        }
        if (noteOnTimeMs[(size_t)key] >= 0.0) return;
        noteOnTimeMs[(size_t)key] = now;

        double randomDelay = 0.0;
        if (postReleaseDelayMs > 0.0)
        {
            const double minimumDelay = juce::jmax(0.0, postReleaseDelayMs - 2.0);
            const double maximumDelay = postReleaseDelayMs + 2.0;
            randomDelay = minimumDelay + delayRandom.nextDouble() * (maximumDelay - minimumDelay);
        }
        else if (postReleaseDelayMs < 0.0 && !continuesDelaySeries)
        {
            const double minimumDelay = juce::jmax(0.0, (double)delayMs - 2.0);
            const double maximumDelay = (double)delayMs + 2.0;
            randomDelay = minimumDelay + delayRandom.nextDouble() * (maximumDelay - minimumDelay);
        }
        pendingOn[(size_t)key] = { now + randomDelay, velocity, true };
    }

    void noteOffImmediate(int midiNote, int manualIndex)
    {
        if (midiNote < 0 || midiNote >= 128) return;
        manualIndex = juce::jlimit(-1, maxManuals - 1, manualIndex);
        const int key = makeSourceKey(midiNote, manualIndex);
        heldVelocity[(size_t)key] = -1;
        updateWindKey(manualIndex, midiNote, false);
        std::array<float, maxManuals> gains{};
        for (int m = 0; m < maxManuals; ++m) gains[(size_t)m] = expressionGain[(size_t)m].load(std::memory_order_relaxed);
        for (int32_t i = keyHeads[(size_t)key]; i >= 0; i = voices[(size_t)i].keyNext)
        {
            Voice& v = voices[(size_t)i];
            if (v.detachedRelease) continue;
            const int m = v.manual;
            v.frozenExprRequested.store(m >= 0 && m < maxManuals ? gains[(size_t)m] : 1.0f, std::memory_order_relaxed);
            sendCommand(v, CmdFreeze);
        }
        if (pendingOff[(size_t)key].valid) return;

        const double now = nowMs();
        double dueMs = now;
        const float minimumMs = owner.attackMinimumMs.load(std::memory_order_relaxed);
        if (noteOnTimeMs[(size_t)key] >= 0.0)
        {
            if (noteMinimumStartMs[(size_t)key] >= 0.0) dueMs = noteMinimumStartMs[(size_t)key] + (double)minimumMs;
            else if (pendingOn[(size_t)key].valid) dueMs = pendingOn[(size_t)key].dueMs + (double)minimumMs;
            noteOnTimeMs[(size_t)key] = -1.0;
        }
        if (dueMs > now)
        {
            pendingOff[(size_t)key] = { dueMs, gains, true };
            return;
        }
        finishNoteOff(key, gains, nullptr, true);
    }

    void startNote(int midiNote, int velocity, int manualIndex, int sourceKey)
    {
        if (midiNote < 0 || midiNote >= 128) return;
        // No compass here (0.8.156): the keyboard's compass is the owner's business
        // (a physical key outside it is not routed); a note a coupler puts beyond
        // the compass plays the pipe the folder has there (six-octave folders).
        int engagedStopCount = 0;
        for (auto* candidate : owner.stops)
            if (stopAssignedToManual(candidate, manualIndex)) ++engagedStopCount;

        bool consonantPairHeld = false;
        {
            std::array<uint8_t, 128> heldNotes{};
            int heldCount = 0;
            for (int m = -1; m < maxManuals && !consonantPairHeld; ++m)
            {
                if (manualIndex >= 0 && m != manualIndex) continue;
                for (int n = 0; n < 128; ++n)
                {
                    if (heldVelocity[(size_t)makeSourceKey(n, m)] < 0) continue;
                    for (int i = 0; i < heldCount; ++i)
                    {
                        const int interval = std::abs((int)heldNotes[(size_t)i] - n);
                        if (interval == 7 || interval == 12) { consonantPairHeld = true; break; }
                    }
                    if (consonantPairHeld) break;
                    if (heldCount < 128) heldNotes[(size_t)heldCount++] = (uint8_t)n;
                }
            }
        }
        const bool couplingSituation = engagedStopCount >= 2 || consonantPairHeld;
        const float releaseFadeOut = owner.releaseFadeOutMs.load(std::memory_order_relaxed);

        std::array<int, kMaxRouteShifts> shifts{};
        for (auto* stop : owner.stops)
        {
            if (stop->reloading.load(std::memory_order_acquire)) continue;   // its samples are being replaced (0.8.160)
            const int shiftCount = collectRouteShifts(stop, manualIndex, shifts);
            if (shiftCount == 0) continue;
            const int expressionManual = juce::jlimit(0, maxManuals - 1, stop->primaryManual);
            const double couplingMs = 500.0 + tuningRandom.nextDouble() * 2000.0;
            const bool useCouplingForThisPipe = couplingSituation && tuningRandom.nextInt(6) != 0;
            const float rankDrift = trem[(size_t)expressionManual].rankDriftDegrees.load(std::memory_order_relaxed);

            // One key may sound several pipes of the stop (its own button and a
            // transposing transmission, or two transmissions with different shifts).
            for (int si = 0; si < shiftCount; ++si)
            {
                const int shift = shifts[(size_t)si];
                const int pipe = midiNote + shift;
                if (pipe < 0 || pipe >= 128) continue;
                if (!routeAllowsKey(stop, manualIndex, shift, midiNote, pipe)) continue;   // switched off in the voicing of every route with this shift
                const float coupledPitchCents = stop->randomPitchByNote[(size_t)pipe] * tuningRandom.nextFloat() / 3.0f;
                // The pipe may already sound through another route: then only its
                // volume is revisited (the loudest route wins).
                bool alreadySpeaking = stop->speaking.test((size_t)pipe);
                for (const auto& rank : stop->ranks) alreadySpeaking = alreadySpeaking || rank->speaking.test((size_t)pipe);
                const float routeGain = pipeRouteGain(stop, pipe);
                if (alreadySpeaking) setPipeRouteGain(stop, pipe, routeGain);

                auto startRank = [&](Stop* rank, int rankOrdinal)
                {
                    if (!rank->rankEnabled) return;
                    if (rank->speaking.test((size_t)pipe)) return;
                    // A pipe the rank does not have plays the sample of a
                    // neighbouring pipe, transposed (see computeFilledPipes).
                    const int sampleNote = rank->sourceNoteFor(pipe);
                    if (sampleNote < 0) return;
                    // A filled pipe transposed upwards plays the anti-alias copy of
                    // its source when one has been built (quality 2).
                    const int quality = owner.transposeQuality.load(std::memory_order_relaxed);
                    // Filled pipes: resampling (a copy only for upward fills, anti-aliased,
                    // played faster or slower) or pitch shift (a copy at the pipe's pitch for
                    // every fill, played at unity - "preShifted"); a copy not built yet falls
                    // back to the plain sample resampled.
                    const bool pitchShiftMethod = owner.transposeMethod.load(std::memory_order_relaxed) >= 1;
                    const bool wantsCopy = pipe != sampleNote && (pitchShiftMethod || (quality >= 2 && pipe > sampleNote));
                    const Sample* sampleToPlay = nullptr;
                    bool preShifted = false;
                    // Velocity layers ("_070" files): the lowest enabled layer covering this
                    // velocity decides - its copy for a filled pipe, else its sample; no
                    // layer -> the plain samples (and their copies).
                    if (auto* layer = rank->velocityLayerFor(sampleNote, velocity, stop->velocityLayerDisabled))
                    {
                        if (wantsCopy) { sampleToPlay = layer->nextAliasFree(pipe); preShifted = pitchShiftMethod && sampleToPlay != nullptr; }
                        if (sampleToPlay == nullptr) sampleToPlay = Stop::pickRoundRobin(layer->samplesByNote, layer->nextSampleIndex, sampleNote);
                    }
                    // The option (0.8.192): an unticked "_128" never plays - no ticked layer has
                    // the note (the plain files would be the fallback) -> nothing sounds.
                    else if (stop->plainLayerDisabled() && !rank->velocityLayers.empty() && owner.velocityPlainStrict.load(std::memory_order_relaxed)) return;
                    if (sampleToPlay == nullptr && wantsCopy) { sampleToPlay = rank->getNextAliasFreeSample(pipe); preShifted = pitchShiftMethod && sampleToPlay != nullptr; }
                    if (sampleToPlay == nullptr) sampleToPlay = rank->getNextSample(sampleNote);
                    if (sampleToPlay == nullptr) return;
                    const int routeIndex = rank->routeByManual[(size_t)expressionManual];
                    if (routeIndex < 0 || routeIndex >= routeCount.load(std::memory_order_acquire)) return;
                    Route& route = *routes[(size_t)routeIndex];
                    makeRoomForVoice();
                    uint32_t idx = 0;
                    Voice* voice = allocVoice(idx);
                    if (voice == nullptr) return;
                    const float busCents = commonRetuneCents(stop, rank);
                    const int bus = busFor(route, busCents);
                    // On a bus the voice renders at the route rate x the bus ratio: its frame counts follow.
                    const double rate = juce::jmax(1.0, route.sampleRate.load(std::memory_order_relaxed)) * (bus > 0 ? route.buses[bus - 1].ratio : 1.0);
                    Voice& v = *voice;
                    v.sample = sampleToPlay;
                    v.stop = stop;
                    v.rank = rank;
                    v.routeIndex = routeIndex;
                    v.bus = (uint8_t)bus;
                    v.sourceKey = (int16_t)sourceKey;
                    v.manual = (int8_t)expressionManual;
                    v.note = (uint8_t)pipe;
                    v.keyNote = (uint8_t)midiNote;
                    v.sampleNote = (uint8_t)sampleNote;
                    v.shift = (int8_t)shift;
                    // A pre-shifted copy is at the pipe's pitch already: only the pure-interval part remains.
                    v.preShifted = preShifted;
                    v.shiftCents.store(transpositionCents(stop, rank, shift, pipe, sampleNote) - (preShifted ? 100.0f * (float)(pipe - sampleNote) : 0.0f),
                        std::memory_order_relaxed);
                    v.cubic = quality >= 1 && sampleNote != pipe && !preShifted;
                    v.fullKernel = wantsFullKernel(rank, pipe);
                    v.routeGainTarget.store(routeGain, std::memory_order_relaxed);
                    v.toneTarget.store(pipeRouteTone(stop, pipe), std::memory_order_relaxed);
                    v.toneStartHz = SamplerEngine::toneStartHzFor(rank, sampleNote, owner.toneStartHz.load(std::memory_order_relaxed));
                    // The level/treble compensation of an upward fill belongs to plain
                    // resampling only: a pitch-shifted copy has its loudness and timbre
                    // settled when it is built.
                    if (preShifted) { v.fillGain = 1.0f; v.toneOffset = 0.0f; }
                    else setFillCompensation(v, pipe, sampleNote);
                    v.detachedRelease = false;
                    v.loopEnabled = !stop->isOneShot;
                    v.velocityGain = 1.0f;   // never softer for a gentle key: that is a piano, not an organ
                    v.velocity = (uint8_t)juce::jlimit(1, 127, velocity);
                    v.randomPitchCents.store(stop->randomPitchByNote[(size_t)pipe], std::memory_order_relaxed);
                    v.tuningCents.store(rank->tuningCentsFor(sampleNote, stop->tuneTo440), std::memory_order_relaxed);
                    v.tempMode.store(stop->tuneTo440 ? 1 : (rank->measuredTemperamentValid ? 2 : 0), std::memory_order_relaxed);
                    v.couplingUsed = useCouplingForThisPipe;
                    v.coupledPitchCents = coupledPitchCents;
                    v.couplingTotalSamples = (uint32_t)juce::jmax(1.0, couplingMs * 0.001 * rate);
                    v.fadeInTotal = 0;
                    v.fadeInRemaining = 0;
                    // Organ keys: the velocity never changes the volume. A rank without
                    // velocity layers softens a gentle stroke with a short fade-in instead:
                    // velocityFadeInMs at the lowest velocity, shrinking linearly to nothing
                    // at the full-strike velocity (velocityFullFrom, default 80 - 0.8.196: a
                    // normal stroke is a full one; ignoring MIDI velocity sends 127, so no fade).
                    const int fullFrom = owner.velocityFullFrom.load(std::memory_order_relaxed);
                    if (rank->velocityLayers.empty() && velocity < fullFrom)
                    {
                        const float gentle = (float)(fullFrom - velocity) / (float)juce::jmax(1, fullFrom - 1);   // 1 at the lowest velocity, 0 from fullFrom up
                        const float ms = owner.velocityFadeInMs.load(std::memory_order_relaxed) * gentle;
                        v.fadeInTotal = v.fadeInRemaining = (uint32_t)juce::jmax(0.0, ms * 0.001 * rate);
                        // ... and the chiff blunted: an extra high-shelf cut that opens over velocityShelfMs.
                        const float shelfDb = owner.velocityShelfDb.load(std::memory_order_relaxed) * gentle;
                        const float shelfMs = juce::jmax(1.0f, owner.velocityShelfMs.load(std::memory_order_relaxed));
                        v.attackToneDb = -shelfDb;
                        v.attackToneStepPerFrame = shelfDb / (float)juce::jmax(1.0, shelfMs * 0.001 * rate);
                    }
                    v.attackBoost = juce::Decibels::decibelsToGain(stop->gainDb);
                    v.attackHold = (uint32_t)juce::jmax(0.0, stop->attackMs * 0.001 * rate);
                    v.attackFade = (uint32_t)juce::jmax(0.0, stop->attackFadeoutMs * 0.001 * rate);
                    v.attackActive = (v.attackHold > 0 || v.attackFade > 0) && std::abs(v.attackBoost - 1.0f) > 1.0e-6f;
                    v.releaseFadeMsRequested.store(releaseFadeOut, std::memory_order_relaxed);
                    uint32_t windHash = (uint32_t)pipe * 0x9e3779b9u;
                    windHash ^= rank->folderHash;
                    windHash ^= (uint32_t)(stop->primaryManual + 1) * 0x85ebca6bu;
                    windHash ^= windHash >> 16; windHash *= 0x7feb352du; windHash ^= windHash >> 15;
                    v.windPitchVar = (uint8_t)(windHash & 15u);
                    windHash *= 0x846ca68bu; windHash ^= windHash >> 16;
                    v.windGainVar = (uint8_t)(windHash & 15u);
                    // Every pipe follows the wind sag differently: a stable random
                    // factor per pipe (a sagging chord goes out of tune, it does not
                    // move as one) and more for small, high pipes than for large ones.
                    {
                        windHash *= 0x27d4eb2fu; windHash ^= windHash >> 15;
                        const float randomFactor = 0.55f + 0.9f * (float)(windHash & 1023u) / 1023.0f;   // 0.55 .. 1.45
                        const float pitchFactor = 0.65f + 0.7f * juce::jlimit(0.0f, 1.0f, (float)(pipe - 24) / 72.0f);   // 0.65 .. 1.35
                        v.windKeyScale = randomFactor * pitchFactor;
                    }
                    const int degrees = juce::roundToInt((stop->tremulantOrdinal % 2) * 7.0f + (rankOrdinal + stop->groupChildOrdinal) * rankDrift);   // stops differ by at most 7 degrees; a stop folder's children drift apart like ranks (0.8.128)
                    v.tremBucket = (uint8_t)((((degrees % 360) + 360) % 360) * kTremBuckets / 360);
                    v.startFrozen = false;
                    duplicateVoiceToAux(v, expressionManual, rate, nowMs(), busCents);   // the division's downmix route
                    publishVoice(idx, route);
                    rank->speaking.set((size_t)pipe);
                };
                startRank(stop, 0);
                for (int rank = 0; rank < (int)stop->ranks.size(); ++rank)
                    startRank(stop->ranks[(size_t)rank].get(), rank + 1);
                if (stop->pipeStartMs[(size_t)pipe] < 0.0) stop->pipeStartMs[(size_t)pipe] = nowMs();
            }
        }
        owner.storePolyphony((int)controlActive.size());
    }

    // The release fade-out of an attack voice and the fade-in of a release sample
    // (the global sliders) are set for the sounding c1 (MIDI 60 of an 8' rank);
    // lower pipes get 1.25 times as much per octave below it (0.8.227, the user's
    // figure; 1.5 before: with the 110 ms default the 8' C got 248 ms and a 32' C
    // 557 ms, now 172 and 269), higher pipes the value as set. The sounding pitch
    // is the pipe shifted by the rank's footage - a filled pipe counts as the pipe
    // it stands for.
    static float fadeScale(const Stop* rank, int pipe)
    {
        const int sounding = pipe + (rank != nullptr && rank->footageKnown ? rank->footageSemitones + rank->sampleOffset : 0);   // the pipe's real footage plus the transposition
        const double octavesBelow = juce::jmax(0.0, (60.0 - (double)sounding) / 12.0);
        return (float)std::pow(1.25, octavesBelow);
    }

    void finishNoteOff(int sourceKey, const std::array<float, maxManuals>& releaseGains,
        Stop* onlyStop, bool updateNoteBookkeeping)
    {
        const int midiNote = keyNote(sourceKey);
        const int manualIndex = keyManual(sourceKey);
        const double now = nowMs();
        double soundingTimeMs = 1.0e9;
        if (noteSoundStartMs[(size_t)sourceKey] >= 0.0)
            soundingTimeMs = juce::jmax(0.0, now - noteSoundStartMs[(size_t)sourceKey]);
        if (updateNoteBookkeeping)
        {
            noteSoundStartMs[(size_t)sourceKey] = -1.0;
            noteMinimumStartMs[(size_t)sourceKey] = -1.0;
            noteFinishedMs[(size_t)sourceKey] = now;
        }
        // The pipes this key held: one per route of the stop from this division
        // (its own button, transmissions with their shifts). A key without a
        // division, and the internal call with a pipe number, address the pipe
        // itself. Only pipes no other key holds any more are released.
        const float releaseFadeOut = owner.releaseFadeOutMs.load(std::memory_order_relaxed);
        std::array<int, kMaxRouteShifts> shifts{};
        releaseTargets.clear();
        uint64_t shiftHistory = 0;   // every shift any of these stops ever used
        for (auto* stop : owner.stops)
        {
            if (onlyStop != nullptr && stop != onlyStop) continue;
            // A one-shot stop (chimes, harp) sounds each sample once through to its
            // end whenever the key goes up: nothing of it is released or faded.
            int shiftCount;
            if (manualIndex < 0) { shifts[0] = 0; shiftCount = 1; }
            else shiftCount = collectRouteShifts(stop, manualIndex, shifts);
            if (stop->isOneShot && updateNoteBookkeeping)
            {
                // The pipe is free for the next strike (0.8.183: 0.8.158 left the speaking
                // bit set, so a chime struck twice was silent the second time until the
                // stop was switched off); its voices sound on untouched.
                for (int si = 0; si < shiftCount; ++si)
                {
                    const int pipe = midiNote + shifts[(size_t)si];
                    if (pipe < 0 || pipe >= 128 || pipeStillHeld(stop, pipe)) continue;
                    stop->speaking.reset((size_t)pipe);
                    for (auto& rank : stop->ranks) rank->speaking.reset((size_t)pipe);
                    stop->pipeStartMs[(size_t)pipe] = -1.0;
                }
                continue;
            }
            stop->releasing.reset();
            for (int si = 0; si < shiftCount; ++si)
            {
                const int pipe = midiNote + shifts[(size_t)si];
                if (pipe < 0 || pipe >= 128) continue;
                if (pipeStillHeld(stop, pipe))
                {
                    // Another key keeps the pipe; the loudest remaining route sets its volume.
                    setPipeRouteGain(stop, pipe, pipeRouteGain(stop, pipe));
                    continue;
                }
                if (releaseTargets.size() < releaseTargets.capacity())
                {
                    releaseTargets.push_back({ stop, pipe, midiNote });
                    stop->releasing.set((size_t)pipe);
                    shiftHistory |= stop->shiftHistory;
                }
            }
        }
        if (releaseTargets.empty()) return;

        // Pass 1: release the sounding attack voices of those pipes and remember
        // their current pitch so the release sample can match it. A pipe is one
        // set of voices however many keys hold it (its own division, a
        // transmission, a coupler): the voices hang on the key that started them,
        // so every key that can address one of the pipes is searched: pipe - shift
        // on any division, for every shift ever used. Without transpositions this
        // is the key itself on every division, as it always was.
        releasePitches.clear();
        std::bitset<128> pipes;
        for (const auto& target : releaseTargets) pipes.set((size_t)target.pipe);
        std::bitset<128> keysSearched;
        for (int pipe = 0; pipe < 128; ++pipe)
        {
            if (!pipes.test((size_t)pipe)) continue;
            for (int shiftBit = 0; shiftBit < kMaxRouteShifts; ++shiftBit)
            {
                if ((shiftHistory & (1ull << shiftBit)) == 0) continue;
                const int key = pipe - (shiftBit - kMaxShift);
                if (key < 0 || key >= 128 || keysSearched.test((size_t)key)) continue;
                keysSearched.set((size_t)key);
                for (int m = -1; m < maxManuals; ++m)
                    for (int32_t i = keyHeads[(size_t)makeSourceKey(key, m)]; i >= 0; i = voices[(size_t)i].keyNext)
                    {
                        Voice& v = voices[(size_t)i];
                        if (v.detachedRelease || v.ctlReleasing || v.stop == nullptr) continue;
                        if (!v.stop->releasing.test((size_t)v.note)) continue;
                        if (releasePitches.size() < releasePitches.capacity())
                            releasePitches.push_back({ v.stop, v.rank,
                                v.curRandomPitch.load(std::memory_order_relaxed) + v.curWindPitch.load(std::memory_order_relaxed)
                                    - v.curTremPitch.load(std::memory_order_relaxed),
                                v.curWindGain.load(std::memory_order_relaxed),
                                v.note, v.keyNote, v.sampleNote, v.shift, v.shiftCents.load(std::memory_order_relaxed),
                                v.routeGainTarget.load(std::memory_order_relaxed),
                                v.toneTarget.load(std::memory_order_relaxed), v.toneStartHz, v.velocity, v.preShifted });
                        const int vm = v.manual;
                        v.frozenExprRequested.store(vm >= 0 && vm < maxManuals ? releaseGains[(size_t)vm] : 1.0f, std::memory_order_relaxed);
                        v.releaseFadeMsRequested.store(releaseFadeOut * fadeScale(v.rank != nullptr ? v.rank : v.stop, v.note), std::memory_order_relaxed);
                        v.ctlReleasing = true;
                        refreshStealable(v);
                        sendCommand(v, CmdFreeze | CmdRelease);
                    }
            }
        }

        // Pass 2: per pipe, the duration-selected release samples.
        for (const auto& target : releaseTargets)
            startReleaseSamples(target.stop, target.pipe, target.keyNote, sourceKey, releaseGains, now, soundingTimeMs);
        for (const auto& target : releaseTargets) target.stop->releasing.reset();
        owner.storePolyphony((int)controlActive.size());
    }

    // Release samples of one pipe of a stop (its attack voices were released in
    // finishNoteOff and their pitches noted in releasePitches).
    void startReleaseSamples(Stop* stop, int pipe, int keyNote, int sourceKey,
        const std::array<float, maxManuals>& releaseGains, double now, double keySoundingTimeMs)
    {
        const float releaseFadeOut = owner.releaseFadeOutMs.load(std::memory_order_relaxed);
        const float releaseFadeIn = owner.releaseFadeInMs.load(std::memory_order_relaxed);
        bool wasSpeaking = stop->speaking.test((size_t)pipe);
        for (const auto& rank : stop->ranks) wasSpeaking = wasSpeaking || rank->speaking.test((size_t)pipe);
        if (!wasSpeaking) return;

        stop->speaking.reset((size_t)pipe);
        for (auto& rank : stop->ranks) rank->speaking.reset((size_t)pipe);
        double pipeSoundingTimeMs = keySoundingTimeMs;
        if (stop->pipeStartMs[(size_t)pipe] >= 0.0)
        {
            pipeSoundingTimeMs = juce::jmax(0.0, now - stop->pipeStartMs[(size_t)pipe]);
            stop->pipeStartMs[(size_t)pipe] = -1.0;
        }
        const int expressionManual = juce::jlimit(0, maxManuals - 1, stop->primaryManual);
        const float rankDrift = trem[(size_t)expressionManual].rankDriftDegrees.load(std::memory_order_relaxed);

        auto releaseRank = [&](Stop* rank, int rankOrdinal)
        {
            if (!rank->rankEnabled) return;
            // The release follows its attack: same key, same shift, same
            // resampling of a pipe beyond the compass.
            float releasePitchCents = 0.0f, releaseWindGain = 1.0f, shiftCents = 0.0f, routeGain = 1.0f;
            int releaseKeyNote = keyNote, sampleNote = rank->sourceNoteFor(pipe), shift = pipe - keyNote;
            if (sampleNote < 0) return;
            float toneDb = stop->getNoteToneDb(pipe);
            float toneStart = SamplerEngine::toneStartHzFor(rank, sampleNote, owner.toneStartHz.load(std::memory_order_relaxed));
            int attackVelocity = 127;
            bool attackFound = false, attackPreShifted = false;
            for (const auto& sounding : releasePitches)
                if (sounding.stop == stop && sounding.rank == rank && (int)sounding.note == pipe)
                {
                    attackVelocity = sounding.velocity;
                    attackPreShifted = sounding.preShifted;
                    releasePitchCents = sounding.cents;
                    releaseWindGain = sounding.windGain;
                    releaseKeyNote = sounding.keyNote;
                    sampleNote = sounding.sampleNote;
                    shift = sounding.shift;
                    shiftCents = sounding.shiftCents;
                    routeGain = sounding.routeGain;
                    toneDb = sounding.toneDb;
                    toneStart = sounding.toneStartHz;
                    attackFound = true;
                    break;
                }
            if (!attackFound) shiftCents = transpositionCents(stop, rank, shift, pipe, sampleNote);
            // The full transposition in cents (a pre-shifted attack had the semitones taken out).
            const float fullShiftCents = shiftCents + (attackPreShifted ? 100.0f * (float)(pipe - sampleNote) : 0.0f);
            // Pitch shift keeps the release length: the release copy of the filled pipe
            // when it exists (built with the attacks' copies), else the release resampled.
            const bool pitchShiftMethod = owner.transposeMethod.load(std::memory_order_relaxed) >= 1;
            bool releasePreShifted = false;
            const Sample* releaseSample = rank->getNextReleaseSample(sampleNote, pipeSoundingTimeMs, attackVelocity, &stop->velocityLayerDisabled,
                pitchShiftMethod && pipe != sampleNote ? pipe : -1, &releasePreShifted);
            if (releaseSample == nullptr) return;
            shiftCents = fullShiftCents - (releasePreShifted ? 100.0f * (float)(pipe - sampleNote) : 0.0f);
            const int routeIndex = rank->routeByManual[(size_t)expressionManual];
            if (routeIndex < 0 || routeIndex >= routeCount.load(std::memory_order_acquire)) return;
            Route& route = *routes[(size_t)routeIndex];
            // "Release played from" (0.8.227): every release starts this far into its file,
            // with NO limit (the user's rule) - a release shorter than the setting is not
            // played at all (a dry set's 60 ms releases with 150 ms set: the key ends with
            // the sustain fade alone).
            const double playedFromFrames = (double)owner.releasePlayedFromMs.load(std::memory_order_relaxed) * 0.001 * releaseSample->sampleRate;
            if (playedFromFrames >= (double)releaseSample->numFrames) return;
            makeRoomForVoice();
            uint32_t idx = 0;
            Voice* voice = allocVoice(idx);
            if (voice == nullptr) return;
            const float busCents = commonRetuneCents(stop, rank);
            const int bus = busFor(route, busCents);
            const double rate = juce::jmax(1.0, route.sampleRate.load(std::memory_order_relaxed)) * (bus > 0 ? route.buses[bus - 1].ratio : 1.0);
            Voice& v = *voice;
            v.sample = releaseSample;
            v.stop = stop;
            v.rank = rank;
            v.routeIndex = routeIndex;
            v.bus = (uint8_t)bus;
            {
                // The release starts at "Release played from" (above); a filled pipe's
                // release copy at the trimmed milliseconds of its own setting at least (up
                // to half of the copy, as it always did; applied here, so changing either
                // rebuilds nothing), and when chosen its level is matched to the source's
                // step at the key release (the gain measured when the copy was built).
                double skip = playedFromFrames;
                if (releasePreShifted)
                    skip = juce::jmax(skip, juce::jlimit(0.0, (double)releaseSample->numFrames / 2.0,
                        (double)owner.transposeReleaseTrimMs.load(std::memory_order_relaxed) * 0.001 * releaseSample->sampleRate));
                if (skip > 0.0) v.pos = (uint64_t)skip << 32;
                if (releasePreShifted && owner.transposeReleaseLevelMatch.load(std::memory_order_relaxed)) routeGain *= releaseSample->releaseLevelGain;
            }
            v.sourceKey = (int16_t)sourceKey;
            v.manual = (int8_t)expressionManual;
            v.note = (uint8_t)pipe;
            v.keyNote = (uint8_t)juce::jlimit(0, 127, releaseKeyNote);
            v.sampleNote = (uint8_t)sampleNote;
            v.shift = (int8_t)juce::jlimit(-kMaxShift, kMaxShift, shift);
            v.shiftCents.store(shiftCents, std::memory_order_relaxed);
            v.routeGainTarget.store(routeGain, std::memory_order_relaxed);   // the release keeps the attack's volume
            v.toneTarget.store(toneDb, std::memory_order_relaxed);           // ... and its tone
            v.toneStartHz = toneStart;
            if (releasePreShifted) { v.fillGain = 1.0f; v.toneOffset = 0.0f; }   // a copy: no resampling compensation
            else setFillCompensation(v, pipe, sampleNote);
            v.cubic = owner.transposeQuality.load(std::memory_order_relaxed) >= 1 && sampleNote != pipe && !releasePreShifted;
            v.fullKernel = wantsFullKernel(rank, pipe);
            v.preShifted = releasePreShifted;
            v.detachedRelease = true;
            v.loopEnabled = false;
            v.velocityGain = 1.0f;
            v.randomPitchCents.store(releasePitchCents, std::memory_order_relaxed);
            v.tuningCents.store(rank->tuningCentsFor(sampleNote, stop->tuneTo440), std::memory_order_relaxed);   // releases follow their attack
            v.tempMode.store(stop->tuneTo440 ? 1 : (rank->measuredTemperamentValid ? 2 : 0), std::memory_order_relaxed);
            v.couplingUsed = false;
            v.coupledPitchCents = 0.0f;
            v.couplingTotalSamples = 1;
            v.fadeInTotal = (uint32_t)juce::jmax(0.0, releaseFadeIn * fadeScale(rank, pipe) * 0.001 * rate);
            v.fadeInRemaining = v.fadeInTotal;
            v.attackBoost = juce::Decibels::decibelsToGain(stop->gainDb);
            v.attackHold = (uint32_t)juce::jmax(0.0, stop->attackMs * 0.001 * rate);
            v.attackFade = (uint32_t)juce::jmax(0.0, stop->attackFadeoutMs * 0.001 * rate);
            v.attackActive = (v.attackHold > 0 || v.attackFade > 0) && std::abs(v.attackBoost - 1.0f) > 1.0e-6f;
            v.releaseFadeMsRequested.store(releaseFadeOut, std::memory_order_relaxed);
            v.windPitchVar = v.windGainVar = 0;
            const int degrees = juce::roundToInt((stop->tremulantOrdinal % 2) * 7.0f + (rankOrdinal + stop->groupChildOrdinal) * rankDrift);   // stops differ by at most 7 degrees; a stop folder's children drift apart like ranks (0.8.128)
            v.tremBucket = (uint8_t)((((degrees % 360) + 360) % 360) * kTremBuckets / 360);
            v.startFrozen = true;
            v.initialFrozenExpr = releaseGains[(size_t)expressionManual];
            v.initialWindGain = releaseWindGain;
            v.ctlReleasing = true;   // a release sample is never released again
            // Release cutting: the sample is faded out over the chosen share of
            // its length and the voice ends (frees its polyphony) exactly there.
            const float cutPercent = owner.releaseCutPercent.load(std::memory_order_relaxed);
            if (cutPercent < 99.9f && releaseSample->sampleRate > 0.0)
            {
                const double lengthMs = 1000.0 * (double)releaseSample->numFrames / releaseSample->sampleRate;
                startFade(v, (float)(lengthMs * cutPercent / 100.0), rate, false);
            }
            duplicateVoiceToAux(v, expressionManual, rate, now, busCents);
            publishVoice(idx, route);
        };
        releaseRank(stop, 0);
        for (int rank = 0; rank < (int)stop->ranks.size(); ++rank)
            releaseRank(stop->ranks[(size_t)rank].get(), rank + 1);
    }

    void processPending(double now, int64_t deadlineTicks)
    {
        const float minimumMs = owner.attackMinimumMs.load(std::memory_order_relaxed);
        for (int key = 0; key < kSourceKeys; ++key)
        {
            auto& on = pendingOn[(size_t)key];
            if (!on.valid || on.dueMs > now) continue;
            if (pastDeadline(deadlineTicks)) return;
            on.valid = false;
            startNote(keyNote(key), on.velocity, keyManual(key), key);
            noteSoundStartMs[(size_t)key] = now;
            noteMinimumStartMs[(size_t)key] = now;
            auto& off = pendingOff[(size_t)key];
            if (off.valid) off.dueMs = juce::jmax(off.dueMs, now + (double)minimumMs);
        }
        for (int key = 0; key < kSourceKeys; ++key)
        {
            auto& off = pendingOff[(size_t)key];
            if (!off.valid || off.dueMs > now) continue;
            if (pastDeadline(deadlineTicks)) return;
            off.valid = false;
            finishNoteOff(key, off.gains, nullptr, true);
        }
    }

    // Every voice of a route finished at once, without a render (control lock
    // held): the route's device stopped or changed (0.8.146) - what sat in its
    // ring and active list would otherwise play on when the next device starts,
    // thousands of release tails and kill fades at once after a measurement
    // and a switch from JACK to ALSA. Returns false when a render holds the route.
    bool flushRouteVoices(Route& r)
    {
        const juce::SpinLock::ScopedTryLockType lk(r.renderLock);
        if (!lk.isLocked()) return false;
        finishRouteVoicesLocked(r);
        return true;
    }

    // Routes whose device stopped delivering callbacks must not accumulate voices.
    void cleanStaleRoutes(double now)
    {
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            Route& r = *routes[(size_t)i];
            const double last = r.lastRenderMs.load(std::memory_order_relaxed);
            const bool hasVoices = r.activeCount.load(std::memory_order_relaxed) > 0
                || r.addHead.load(std::memory_order_acquire) != r.addTail.load(std::memory_order_acquire);
            if (!hasVoices) continue;
            if (last >= 0.0 && now - last < 1000.0) continue;
            if (last < 0.0)
            {
                // Never rendered: give a freshly created device a moment to start.
                if (r.clockMs < 0.0) { r.clockMs = -now; continue; }        // remember first sighting
                if (now + r.clockMs < 1500.0) continue;                      // clockMs holds -firstSeen
            }
            const juce::SpinLock::ScopedTryLockType lk(r.renderLock);
            if (!lk.isLocked()) continue;
            finishRouteVoicesLocked(r);
        }
    }

    // The ring and the active list of a route emptied into the finished queue
    // (renderLock held by the caller).
    void finishRouteVoicesLocked(Route& r)
    {
        {
            uint32_t head = r.addHead.load(std::memory_order_relaxed);
            const uint32_t tail = r.addTail.load(std::memory_order_acquire);
            const uint32_t cap = (uint32_t)r.addRing.size();
            const juce::SpinLock::ScopedLockType fl(finishedLock);
            while (head != tail)
            {
                const uint32_t idx = r.addRing[head % cap];
                voices[idx].state.store(VoiceFinished, std::memory_order_release);
                finishedQueue.push_back(idx);
                ++head;
            }
            r.addHead.store(head, std::memory_order_release);
            for (const uint32_t idx : r.active)
            {
                voices[idx].state.store(VoiceFinished, std::memory_order_release);
                finishedQueue.push_back(idx);
            }
            r.active.clear();
            r.activeCount.store(0, std::memory_order_relaxed);
            r.clockMs = -1.0;
        }
    }

    void tick(double budgetOverrideMs = -1.0)
    {
        const double now = nowMs();
        // On the audio thread (no control thread): the callback carries more than
        // the render (a dozen route callbacks with their copies and statistics),
        // and the deadline is checked per command, so the budget keeps a wide
        // margin: 65 % of the period minus the render, at most 35 % of the period
        // (whole callbacks reached 130-150 % of the period with the earlier
        // 80 % / 50 % rule at a render share of 25 %). On the control thread the
        // caller sets the budget: the time one hold of the lock may take.
        const double periodMs = lastBlockMs.load(std::memory_order_relaxed);
        const double renderShare = juce::jlimit(0.0, 1.0, (double)owner.renderLoad.load(std::memory_order_relaxed));
        const double budgetMs = budgetOverrideMs > 0.0 ? budgetOverrideMs
            : juce::jlimit(0.3, 0.35 * periodMs, periodMs * (0.65 - renderShare));
        const int64_t deadline = juce::Time::getHighResolutionTicks()
            + (int64_t)(budgetMs * 0.001 * (double)juce::Time::getHighResolutionTicksPerSecond());
        auto phaseMax = [](std::atomic<float>& slot, int64_t from)
        {
            const float ms = (float)(1000.0 * (double)(juce::Time::getHighResolutionTicks() - from)
                / (double)juce::Time::getHighResolutionTicksPerSecond());
            float previous = slot.load(std::memory_order_relaxed);
            while (ms > previous && !slot.compare_exchange_weak(previous, ms, std::memory_order_relaxed)) {}
            return juce::Time::getHighResolutionTicks();
        };
        int64_t phase = juce::Time::getHighResolutionTicks();
        reclaimFinished();
        stealExhausted = false;
        phase = phaseMax(owner.controlPhaseMs[0], phase);
        processMidi(deadline);
        phase = phaseMax(owner.controlPhaseMs[1], phase);
        processPending(now, deadline);
        phase = phaseMax(owner.controlPhaseMs[2], phase);
        if (now - lastStaleCheckMs > 250.0)
        {
            lastStaleCheckMs = now;
            cleanStaleRoutes(now);
            phaseMax(owner.controlPhaseMs[3], phase);
        }
        owner.storePolyphony((int)controlActive.size());
    }

    void controlTickIfPossible()
    {
        // One tick per device period. The routes of one device (every division
        // and perspective on the ASIO card: a dozen or more) render in sequence
        // on its thread, and a tick in every one of them multiplied the budget by
        // the route count: a bashed tutti brings tens of thousands of attacks and
        // releases at once, and 13 ticks of 1.5 ms stretched a 5.3 ms period to
        // 20-30 ms - audible dropouts at a render load of 40 %.
        // "Once per period": the next tick starts no sooner than three quarters
        // of a period after the last one began, and a quarter period after it
        // ended - the routes of one callback follow each other without a pause,
        // so a tick that used its budget never gets a second one in the same
        // callback (the routes after it see no gap).
        if (controlThreadRunning.load(std::memory_order_relaxed)) return;   // the control thread does it
        const int64_t t0 = juce::Time::getHighResolutionTicks();
        const int64_t periodTicks = (int64_t)(lastBlockMs.load(std::memory_order_relaxed) * 0.001
            * (double)juce::Time::getHighResolutionTicksPerSecond());
        if (t0 - lastTickStartTicks.load(std::memory_order_relaxed) < periodTicks * 3 / 4) return;
        if (t0 - lastTickEndTicks.load(std::memory_order_relaxed) < periodTicks / 4) return;
        if (!owner.controlLock.tryEnter()) return;
        if (!suspended.load(std::memory_order_acquire))
        {
            lastTickStartTicks.store(t0, std::memory_order_relaxed);
            tick();
            const int64_t spent = juce::Time::getHighResolutionTicks() - t0;
            lastTickEndTicks.store(t0 + spent, std::memory_order_relaxed);
            owner.controlTickTicksTotal.fetch_add(spent, std::memory_order_relaxed);
            owner.controlTickCount.fetch_add(1, std::memory_order_relaxed);
            const float ms = (float)(1000.0 * (double)spent / (double)juce::Time::getHighResolutionTicksPerSecond());
            float previous = owner.maxControlTickMs.load(std::memory_order_relaxed);
            while (ms > previous && !owner.maxControlTickMs.compare_exchange_weak(previous, ms)) {}
        }
        owner.controlLock.exit();
    }

    //--------------------------------------------------------------------------
    // Rendering (render thread of one route)
    void buildBlockContext(Route& route, BlockContext& ctx, int numSamples, int numOuts, float outputGain, double now)
    {
        ctx.numSamples = numSamples;
        const double rate = juce::jmax(1000.0, route.sampleRate.load(std::memory_order_relaxed));
        ctx.routeRate = rate;
        const double blockMs = 1000.0 * numSamples / rate;
        ctx.outputGain = outputGain;

        // channel list
        ctx.outCount = 0;
        const uint64_t mask = route.channelMask.load(std::memory_order_relaxed);
        const int limit = juce::jmin(numOuts, 64);
        for (int ch = 0; ch < limit && ctx.outCount < 64; ++ch)
            if ((mask >> ch) & 1ull) ctx.outChannels[ctx.outCount++] = ch;

        // sample-time clock, gently slaved to the wall clock so that all
        // perspectives/devices agree, without any per-block jitter.
        bool clockJumped = false;
        if (route.clockMs < 0.0) { route.clockMs = now; clockJumped = true; }
        else
        {
            const double err = now - route.clockMs;
            if (std::abs(err) > 40.0)
            {
                // After a pause (device switch, mute, underrun) resynchronise at
                // once. Slewing seconds away at 2 % per block left this route far
                // behind the wall clock that stamps wind events, so sags moved
                // late and in jumps.
                route.clockMs = now;
                clockJumped = true;
            }
            else
            {
                const double limitMs = 0.02 * blockMs;
                route.clockMs += juce::jlimit(-limitMs, limitMs, err);
            }
        }
        const double tEnd = route.clockMs + blockMs;

        const int manual = juce::jlimit(0, maxManuals - 1, route.manual);
        const float exprTarget = expressionGain[(size_t)manual].load(std::memory_order_relaxed);
        if (!route.exprInit) { route.exprSmoothed = exprTarget; route.exprInit = true; }
        else
        {
            const float alpha = 1.0f - std::exp((float)(-blockMs / 15.0));
            route.exprSmoothed += (exprTarget - route.exprSmoothed) * alpha;
            if (std::abs(exprTarget - route.exprSmoothed) < 1.0e-5f) route.exprSmoothed = exprTarget;
        }
        ctx.exprGain = route.exprSmoothed;
        ctx.divisionGain = divisionGain[(size_t)manual].load(std::memory_order_relaxed);
        ctx.routeGainLagMs = windLagMs[(size_t)manual].load(std::memory_order_relaxed);
        ctx.releaseScale = owner.releaseScalingGain.load(std::memory_order_relaxed);
        for (int pc = 0; pc < 12; ++pc)
        {
            ctx.pitchClassCents[pc] = pitchClassCents[(size_t)pc].load(std::memory_order_relaxed);
            ctx.selectedCents[pc] = selectedPitchCents[(size_t)pc].load(std::memory_order_relaxed);
        }
        ctx.randomAllowed = randomAllowed[(size_t)manual].load(std::memory_order_relaxed);
        ctx.couplingAllowed = couplingAllowed[(size_t)manual].load(std::memory_order_relaxed);
        ctx.kernel = owner.simdDisabled.load(std::memory_order_relaxed) ? (int)KernelScalar : (int)bestKernel();

        // wind
        const float windLevelDbPerCent = owner.windLevelDbPerCentValue.load(std::memory_order_relaxed);
        const float windAttenuationDbPerCent = owner.tremulantLevelDbPerCentValue.load(std::memory_order_relaxed);
        float sag = 0.0f, combinedSagLimit = 0.0f, negativeLimit = 0.0f;
        bool hasExternalWind = false;
        for (int source = 0; source < maxManuals; ++source)
        {
            // A snapshot read that collides with a key event keeps the previous
            // copy, so the sag never jumps because of a failed read.
            WindSourceState& wind = route.windCache[(size_t)source];
            windSnapshots[(size_t)source].read(wind);
            if ((wind.targetMask & (1u << manual)) == 0) continue;
            const bool external = source != manual;
            hasExternalWind = hasExternalWind || external;
            sag += windSagAt(wind, tEnd, route.windPhase, external);
            combinedSagLimit = juce::jmax(combinedSagLimit, windSagLimit(wind) * (external ? 0.5f : 1.0f));
            negativeLimit = juce::jmax(negativeLimit, 0.35f * wind.depthCents * (external ? 0.5f : 1.0f));
        }
        if (hasExternalWind) combinedSagLimit *= 1.30f;
        sag = juce::jlimit(-negativeLimit, combinedSagLimit, sag);
        for (int variation = 0; variation < kWindVariations; ++variation)
        {
            const float proportion = (float)variation / (float)(kWindVariations - 1);
            const float pitchScale = 0.80f + 0.40f * proportion;
            const float gainScale = 0.88f + 0.24f * proportion;
            ctx.windPitchCents[variation] = -sag * pitchScale;
            // Wind level change per cent of sag (Configuration Editor, default 0.2 dB).
            ctx.windGain[variation] = std::abs(sag) > 1.0e-6f
                ? fastDbToGain(-windLevelDbPerCent * sag * gainScale) : 1.0f;
        }

        // tremulant
        auto& tp = trem[(size_t)manual];
        const float targetMix = tp.enabled.load(std::memory_order_relaxed) ? 1.0f : 0.0f;
        // Tremulant fades in over 700 ms and fades out over 1.4 s.
        const float move = (float)(blockMs / (targetMix > route.tremMix ? 700.0 : 1400.0));
        route.tremMix += juce::jlimit(-move, move, targetMix - route.tremMix);
        const float rateHz = tp.rateHz.load(std::memory_order_relaxed);
        // The tremulant's strength wanders: every cycle draws a new target between
        // 60 and 100 % of the set depth (hashed from the cycle number, so all
        // perspectives of the division agree) and the strength glides to it over
        // about half a second.
        {
            const int64_t cycle = (int64_t)std::floor(tEnd * 0.001 * juce::jmax(0.1f, rateHz));
            uint32_t h = ((uint32_t)cycle * 2654435761u) ^ ((uint32_t)(manual + 1) * 0x9e3779b9u);
            h ^= h >> 15; h *= 0x85ebca6bu; h ^= h >> 13;
            const float target = 0.60f + 0.40f * (float)(h & 1023u) / 1023.0f;
            const float step = (float)(blockMs / 500.0) * 0.40f;   // the whole 60..100 % span in 0.5 s
            route.tremVary += juce::jlimit(-step, step, target - route.tremVary);
        }
        const float depth = tp.depthCents.load(std::memory_order_relaxed) * route.tremMix * route.tremVary;
        // phase advances in sample time; a slow correction keeps perspectives in step
        route.tremPhase += blockMs * 0.001 * rateHz;
        route.tremPhase -= std::floor(route.tremPhase);
        {
            const double nominal = tEnd * 0.001 * rateHz;
            double diff = (nominal - std::floor(nominal)) - route.tremPhase;
            if (diff > 0.5) diff -= 1.0; else if (diff < -0.5) diff += 1.0;
            route.tremPhase += clockJumped ? diff : juce::jlimit(-0.002, 0.002, diff);
            route.tremPhase -= std::floor(route.tremPhase);
        }
        ctx.tremActive = std::abs(depth) > 1.0e-5f;
        if (ctx.tremActive)
        {
            constexpr float pitchPhaseOffsetTurns = 20.0f / 360.0f;
            // The harmonic filter oscillates a settable phase after the volume (Advanced global audio).
            const float shelfPhaseTurns = owner.tremulantShelfPhaseTurns.load(std::memory_order_relaxed);
            for (int b = 0; b < kTremBuckets; ++b)
            {
                const float turns = (float)route.tremPhase + (float)b / (float)kTremBuckets;
                const float pitch = depth * fastSinTurns(turns + pitchPhaseOffsetTurns);
                const float sine = fastSinTurns(turns);
                const float gainDb = depth * windAttenuationDbPerCent * sine;
                ctx.tremPitchCents[b] = pitch;
                ctx.tremGain[b] = fastDbToGain(gainDb);
                ctx.tremPitchCentsRelease[b] = pitch * 0.6f;
                ctx.tremGainRelease[b] = fastDbToGain(gainDb * 0.6f);
                // Tone shelf: the dip's fraction of its full depth (fade-in/out of the
                // tremulant is already inside 'depth', so scale by the mix as well).
                const float shelfSine = fastSinTurns(turns - shelfPhaseTurns);
                const float dip = juce::jmax(0.0f, -shelfSine) * juce::jlimit(0.0f, 1.0f, route.tremMix * route.tremVary);
                ctx.tremShelf[b] = dip;
                ctx.tremShelfRelease[b] = dip * 0.6f;
            }
        }

        // harmonic filter (tone shelf following the level)
        ctx.shelfSwell = owner.harmonicSwellMix.load(std::memory_order_relaxed);
        ctx.shelfWind = owner.harmonicWindMix.load(std::memory_order_relaxed);
        ctx.shelfTrem = owner.harmonicTremulantMix.load(std::memory_order_relaxed);
        ctx.shelfEnabled = ctx.shelfSwell > 1.0e-4f || ctx.shelfWind > 1.0e-4f || ctx.shelfTrem > 1.0e-4f;
        butterworthLowpass(ctx.routeRate, kShelfCornerHz, ctx.shelfLp);
        {
            // 15 degrees of lag at the tremulant rate: tau = tan(15 deg) / (2 pi f).
            const float lagSeconds = 0.26795f / (2.0f * juce::MathConstants<float>::pi * juce::jmax(0.5f, rateHz));
            ctx.shelfSmoothing = juce::jlimit(0.01f, 1.0f, 1.0f - std::exp((float)(-blockMs * 0.001) / lagSeconds));
        }
        ctx.exprMaxDb = expressionMaxDb[(size_t)manual].load(std::memory_order_relaxed);
        ctx.windMaxDb = windLevelDbPerCent * combinedSagLimit * 1.12f;   // largest gainScale of the variations
        route.clockMs = tEnd;
    }

    // The output discontinuity detector of a route at one stage (0.8.226): every
    // output channel's first sample against the last sample of the previous block
    // (SamplerEngine::outputEdgeJump); the jumps go to route.edgeJumps[stage], read
    // once a second by takeOutputDiscontinuities. The program hears its own clicks:
    // the user's crackle of 29 Oct 2026 (one route's voices missing for one block,
    // the timing fine) showed in nothing the program measured.
    void countEdgeJumps(Route& route, int stage, const juce::AudioBuffer<float>& buffer, const BlockContext& ctx, int usableOuts, int totalSamples, int added) noexcept
    {
        static const bool debug = std::getenv("ORGAN_EDGE_DEBUG") != nullptr;   // dev: one log line per jump (allocates - never in a release run)
        if (!owner.outputDiscontinuityDetector.load(std::memory_order_relaxed)) return;   // switched off (0.8.227): the state goes stale, the gap rule restarts it
        int jumps = 0;
        for (int ci = 0; ci < ctx.outCount; ++ci)
        {
            const int ch = ctx.outChannels[ci];
            if (ch < 0 || ch >= usableOuts) continue;
            const float before = route.edge[stage][ch].last;
            if (!SamplerEngine::outputEdgeJump(buffer.getReadPointer(ch), totalSamples, route.edge[stage][ch])) continue;
            ++jumps;
            if (debug)
            {
                int finished = 0, releases = 0, fading = 0, fresh = 0;
                for (const uint32_t idx : route.active)
                {
                    const Voice& v = voices[idx];
                    if (v.finished) ++finished;
                    if (v.detachedRelease) ++releases;
                    if (v.fadingOut || v.killing) ++fading;
                    if (v.pos < (uint64_t)totalSamples << 32) ++fresh;
                }
                const float* d = buffer.getReadPointer(ch);
                CrashHandler::note("Edge jump: manual " + juce::String(route.manual) + " " + route.perspective + " stage " + juce::String(stage)
                    + " ch " + juce::String(ch) + " last " + juce::String(before, 5) + " first " + juce::String(d[0], 5) + " then " + juce::String(d[1], 5)
                    + " " + juce::String(d[2], 5) + " active " + juce::String((int)route.active.size()) + " (releases " + juce::String(releases)
                    + ", fading " + juce::String(fading) + ", in their first block " + juce::String(fresh) + ") added " + juce::String(added)
                    + " finished " + juce::String(finished) + " polyphony " + juce::String(owner.getCurrentPolyphony())
                    + " limit " + juce::String(owner.maxPolyphony.load()));
            }
        }
        if (jumps > 0) route.edgeJumps[stage].fetch_add(jumps, std::memory_order_relaxed);
    }

    void renderRoute(Route& route, juce::AudioBuffer<float>& buffer, float masterGain, int laneLimit = 0)
    {
        const juce::SpinLock::ScopedLockType lk(route.renderLock);
        if (suspended.load(std::memory_order_acquire)) return;
        juce::ScopedNoDenormals noDenormals;
        const int64_t startTicks = juce::Time::getHighResolutionTicks();
        // The device thread's profile (0.8.137, while the measurement profiles): a
        // clock read at every phase boundary of this callback, ten per route.
        const bool prof = gVoiceProfiling.load(std::memory_order_relaxed);
        int64_t profLast = prof ? profileTicks() : 0;
        const int64_t profStart = profLast;
        auto lap = [&](std::atomic<int64_t>& into)
        {
            if (!prof) return;
            const int64_t t = profileTicks();
            into.fetch_add(profileDelta(profLast, t), std::memory_order_relaxed);
            profLast = t;
        };
        if (prof) { tlPoolOwnTicks = 0; tlPoolWaitTicks = 0; tlReverbWaitTicks = 0; }
        const double now = nowMs();
        const int totalSamples = buffer.getNumSamples();
        const int numOuts = buffer.getNumChannels();
        if (totalSamples <= 0 || numOuts <= 0) { route.lastRenderMs.store(now, std::memory_order_relaxed); return; }

        // Take newly published voices.
        int added = 0;   // voices taken in this block (the discontinuity detector's debug line)
        {
            uint32_t head = route.addHead.load(std::memory_order_relaxed);
            const uint32_t tail = route.addTail.load(std::memory_order_acquire);
            const uint32_t cap = (uint32_t)route.addRing.size();
            while (head != tail)
            {
                const uint32_t idx = route.addRing[head % cap];
                ++head;
                if (idx < (uint32_t)poolSize && route.active.size() < route.active.capacity())
                {
                    voices[idx].state.store(VoiceActive, std::memory_order_release);
                    route.active.push_back(idx);
                    ++added;
                }
            }
            route.addHead.store(head, std::memory_order_release);
        }
        lap(statRouteTake);
        {
            // A gap in the renders (a device stopped and started, the first block): the
            // discontinuity detector starts afresh, the first block after it is no jump.
            const double previous = route.lastRenderMs.load(std::memory_order_relaxed);
            if (previous < 0.0 || now - previous > 250.0)
                for (auto& stage : route.edge) for (auto& s : stage) s = SamplerEngine::EdgeState{};
        }
        route.lastRenderMs.store(now, std::memory_order_relaxed);
        lastBlockMs.store(1000.0 * totalSamples / juce::jmax(1000.0, route.sampleRate.load(std::memory_order_relaxed)),
            std::memory_order_relaxed);
        const float outputGain = masterGain * route.volumeGain.load(std::memory_order_relaxed);

        static thread_local BlockContext ctx;
        alignas(16) static thread_local float sink[RenderPool::kFrames];
        float* outs[64]{};
        float* wets[64]{};
        const int usableOuts = juce::jmin(numOuts, 64);
        for (int c = 0; c < usableOuts; ++c) wets[c] = route.wetStore.data() + (size_t)c * kMaxBlockFrames;
        // Other device threads rendering now (counted with the load below): the
        // worker pool leaves them their CPUs.
        const int otherAudioThreads = juce::jmax(0, activeAudioThreads.load(std::memory_order_relaxed) - 1);
        // Retune buses (0.8.163): the voices split into the route's own list and
        // one list per bus; a route without a bus takes the path it always took.
        const bool busing = route.anyBus.load(std::memory_order_acquire);
        if (busing)
        {
            route.direct.clear();
            for (auto& bus : route.buses) bus.list.clear();
            for (const uint32_t idx : route.active)
            {
                const uint8_t b = voices[idx].bus;
                if (b >= 1 && b <= kMaxBuses && route.buses[b - 1].ready.load(std::memory_order_relaxed)) route.buses[b - 1].list.push_back(idx);
                else route.direct.push_back(idx);
            }
        }
        const std::vector<uint32_t>& directList = busing ? route.direct : route.active;

        for (int offset = 0; offset < totalSamples; offset += RenderPool::kFrames)
        {
            const int n = juce::jmin(RenderPool::kFrames, totalSamples - offset);
            buildBlockContext(route, ctx, n, usableOuts, outputGain, now + 1000.0 * offset / ctx.routeRate);
            for (int c = 0; c < usableOuts; ++c) outs[c] = buffer.getWritePointer(c) + offset;
            if (busing)
                for (auto& bus : route.buses)
                    if (bus.ready.load(std::memory_order_relaxed)) renderBus(bus, ctx, n, outs, usableOuts, sink, laneLimit, otherAudioThreads);
            if (!directList.empty())
            {
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch >= 0 && ch < usableOuts) juce::FloatVectorOperations::clear(wets[ch], n);
                }
                pool->render(directList.data(), (int)directList.size(), voices.get(), ctx, outs, wets, usableOuts, sink,
                    laneLimit, otherAudioThreads);
                // The harmonic filter of the whole route: the voices' wet shares
                // low-passed once per channel and added to the output (see LaneOutputs).
                if (ctx.shelfEnabled)
                    for (int ci = 0; ci < ctx.outCount; ++ci)
                    {
                        const int ch = ctx.outChannels[ci];
                        if (ch >= 0 && ch < usableOuts) applyShelfLowpass(wets[ch], outs[ch], n, ctx.shelfLp, route.shelfState[ch]);
                    }
            }
        }

        lap(statRouteLoop);
        {
            // Dev hook ORGAN_TEST_GLITCH=<n> (0.8.226): every n-th block of every route is
            // silenced AFTER the render - the voices advance, their sound is missing for
            // one block, the user's crackle of 29 Oct 2026 made to order. The detector
            // below must count every one of them (one per channel per block).
            static const int glitchEvery = std::getenv("ORGAN_TEST_GLITCH") != nullptr ? juce::jmax(0, juce::String(std::getenv("ORGAN_TEST_GLITCH")).getIntValue()) : 0;
            if (glitchEvery > 0 && (++route.edgeDebugBlocks % (uint32_t)glitchEvery) == 0)
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch >= 0 && ch < usableOuts) juce::FloatVectorOperations::clear(buffer.getWritePointer(ch), totalSamples);
                }
        }
        countEdgeJumps(route, 0, buffer, ctx, usableOuts, totalSamples, added);   // stage 0: straight after the pool render (0.8.226)
        if (prof) { statRoutePoolOwn.fetch_add(tlPoolOwnTicks, std::memory_order_relaxed); statRoutePoolWait.fetch_add(tlPoolWaitTicks, std::memory_order_relaxed); }
        // Compact: finished voices go back to the control thread.
        route.finishedScratch.clear();
        size_t w = 0;
        for (size_t i = 0; i < route.active.size(); ++i)
        {
            const uint32_t idx = route.active[i];
            Voice& v = voices[idx];
            if (v.finished)
            {
                v.state.store(VoiceFinished, std::memory_order_release);
                route.finishedScratch.push_back(idx);
            }
            else route.active[w++] = idx;
        }
        route.active.resize(w);
        route.activeCount.store((int)w, std::memory_order_relaxed);
        if (!route.finishedScratch.empty())
        {
            {
                const juce::SpinLock::ScopedLockType fl(finishedLock);
                for (const uint32_t idx : route.finishedScratch) finishedQueue.push_back(idx);
            }
            if (controlThreadRunning.load(std::memory_order_relaxed)) controlWake.signal();   // reclaim them soon
        }

        lap(statRouteCompact);
        // Channel separation: with less than 100 % each route channel receives
        // (1 - separation) of every other channel's signal (mono pipes spread over
        // several channels leak into the others; at 0 % all channels carry the sum).
        {
            const float separation = juce::jlimit(0.0f, 1.0f, route.separation.load(std::memory_order_relaxed));
            if (separation < 0.999f && ctx.outCount > 1)
            {
                alignas(16) static thread_local float sum[8192];
                const int chunk = 8192;
                for (int offset = 0; offset < totalSamples; offset += chunk)
                {
                    const int n = juce::jmin(chunk, totalSamples - offset);
                    std::fill(sum, sum + n, 0.0f);
                    for (int ci = 0; ci < ctx.outCount; ++ci)
                    {
                        const int ch = ctx.outChannels[ci];
                        if (ch < 0 || ch >= numOuts) continue;
                        const float* data = buffer.getReadPointer(ch) + offset;
                        for (int i = 0; i < n; ++i) sum[i] += data[i];
                    }
                    const float leak = 1.0f - separation;
                    for (int ci = 0; ci < ctx.outCount; ++ci)
                    {
                        const int ch = ctx.outChannels[ci];
                        if (ch < 0 || ch >= numOuts) continue;
                        float* data = buffer.getWritePointer(ch) + offset;
                        for (int i = 0; i < n; ++i) data[i] = separation * data[i] + leak * sum[i];
                    }
                }
            }
        }

        lap(statRouteSeparation);
        // IR reverb of this route (its output pairs), before the safety clip and
        // the recording so both see the final sound of the perspective.
        applyRouteReverb(route, buffer, ctx, numOuts, totalSamples);
        lap(statRouteReverb);
        if (prof) statRouteReverbWait.fetch_add(tlReverbWaitTicks, std::memory_order_relaxed);

        // Safety clip: identity below 0.98, smooth saturation above so a huge
        // registration can never hand hard-clipped garbage to the driver.
        for (int ci = 0; ci < ctx.outCount; ++ci)
        {
            const int ch = ctx.outChannels[ci];
            if (ch < 0 || ch >= numOuts) continue;
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < totalSamples; ++i)
            {
                const float x = data[i];
                const float a = std::abs(x);
                if (a > 0.98f)
                {
                    const float over = a - 0.98f;
                    const float y = 0.98f + over / (1.0f + over * 50.0f) * 1.0f;
                    data[i] = x < 0.0f ? -juce::jmin(1.0f, y) : juce::jmin(1.0f, y);
                }
                else if (!std::isfinite(x)) data[i] = 0.0f;
            }
        }

        lap(statRouteClip);
        countEdgeJumps(route, 1, buffer, ctx, usableOuts, totalSamples, added);   // stage 1: the route's final output (0.8.226)
        // Recording: add this route's final output into the shared stereo ring.
        if (owner.recordingActive.load(std::memory_order_acquire) && recorder != nullptr
            && route.recRing[0].size() == (size_t)Recorder::ringSize)
        {
            Recorder& rec = *recorder;
            const int64_t flushed = rec.flushed.load(std::memory_order_acquire);
            if (route.recEpoch != rec.epoch || route.recPos < flushed)
            {
                // First block of this recording, or the route was silent long
                // enough to be left behind by the writer: continue behind the
                // flush in progress instead of adding into frames already written.
                route.recEpoch = rec.epoch;
                route.recPos = juce::jmax(flushed, rec.flushTarget.load(std::memory_order_acquire));
            }
            if (route.recPos + totalSamples - flushed < Recorder::ringSize)
            {
                for (int ci = 0; ci < ctx.outCount; ++ci)
                {
                    const int ch = ctx.outChannels[ci];
                    if (ch < 0 || ch >= numOuts) continue;
                    float* ring = route.recRing[ch & 1].data();
                    const float* data = buffer.getReadPointer(ch);
                    for (int i = 0; i < totalSamples; ++i)
                        ring[(size_t)((route.recPos + i) & Recorder::ringMask)] += data[i];
                }
            }
            route.recPos += totalSamples;
            route.recPosPublished.store(route.recPos, std::memory_order_release);
        }

        lap(statRouteRecord);
        const double elapsedSeconds = (double)(juce::Time::getHighResolutionTicks() - startTicks)
            / (double)juce::Time::getHighResolutionTicksPerSecond();
        const double blockSeconds = totalSamples / juce::jmax(1000.0, ctx.routeRate);
        const float load = (float)(elapsedSeconds / juce::jmax(1.0e-6, blockSeconds));
        route.lastLoad.store(juce::jmax(load, route.lastLoad.load(std::memory_order_relaxed) * 0.9f),
            std::memory_order_relaxed);
        {
            // The worst block since the last query, as a share of its own budget (the
            // routes have different block lengths: ASIO 5.3 ms, Windows Audio 10 ms).
            float previous = owner.maxBlockMs.load(std::memory_order_relaxed);
            while (load > previous && !owner.maxBlockMs.compare_exchange_weak(previous, load, std::memory_order_relaxed)) {}
            if (load > previous) owner.maxBlockRoute.store(routeIndexOf(&route), std::memory_order_relaxed);
        }
        route.lastRenderThread.store((uintptr_t)juce::Thread::getCurrentThreadId(), std::memory_order_relaxed);
        // DSP load = the busiest audio thread. Routes rendered by the same thread
        // (one shared ASIO device) are sequential and add up; routes on separate
        // devices (Windows Audio, one thread each) run in parallel and must not.
        uintptr_t threads[kMaxRoutes];
        float sums[kMaxRoutes];
        int firstRoute[kMaxRoutes];
        int numThreads = 0;
        float busiest = 0.0f;
        const int count = routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            const Route& r = *routes[(size_t)i];
            if (now - r.lastRenderMs.load(std::memory_order_relaxed) >= 250.0) continue;
            const uintptr_t thread = r.lastRenderThread.load(std::memory_order_relaxed);
            int slot = -1;
            for (int t = 0; t < numThreads; ++t) if (threads[t] == thread) { slot = t; break; }
            if (slot < 0) { slot = numThreads++; threads[slot] = thread; sums[slot] = 0.0f; firstRoute[slot] = i; }
            sums[slot] += r.lastLoad.load(std::memory_order_relaxed);
            busiest = juce::jmax(busiest, sums[slot]);
        }
        owner.renderLoad.store(busiest, std::memory_order_relaxed);
        for (int t = 0; t < juce::jmin(numThreads, 8); ++t)
        {
            owner.renderThreadLoads[(size_t)t].store(sums[t], std::memory_order_relaxed);
            owner.renderThreadRoutes[(size_t)t].store(firstRoute[t], std::memory_order_relaxed);
        }
        owner.renderThreadLoadCount.store(juce::jmin(numThreads, 8), std::memory_order_relaxed);
        activeAudioThreads.store(juce::jmax(1, numThreads), std::memory_order_relaxed);
        lap(statRouteStats);
        if (prof)
        {
            statRouteCalls.fetch_add(1, std::memory_order_relaxed);
            // The longest callback and its parts: the peak is what breaks the sound.
            auto atomicMax = [](std::atomic<int64_t>& into, int64_t value)
            {
                int64_t known = into.load(std::memory_order_relaxed);
                while (value > known && !into.compare_exchange_weak(known, value, std::memory_order_relaxed)) {}
            };
            atomicMax(statRouteMaxTotal, profileDelta(profStart, profLast));
            atomicMax(statRouteMaxOwn, tlPoolOwnTicks);
            atomicMax(statRouteMaxWait, tlPoolWaitTicks);
        }
    }
    std::atomic<int> activeAudioThreads{ 1 };   // device callback threads that rendered within 250 ms
    // The device thread's profile: profiling-clock ticks summed over the route
    // callbacks since the last takeRenderPoolStats (see RenderPoolStats).
    std::atomic<int64_t> statRouteCalls{ 0 }, statRouteTake{ 0 }, statRouteLoop{ 0 }, statRoutePoolOwn{ 0 }, statRoutePoolWait{ 0 }, statRouteCompact{ 0 },
        statRouteSeparation{ 0 }, statRouteReverb{ 0 }, statRouteReverbWait{ 0 }, statRouteClip{ 0 }, statRouteRecord{ 0 }, statRouteStats{ 0 },
        statRouteMaxTotal{ 0 }, statRouteMaxOwn{ 0 }, statRouteMaxWait{ 0 };
};

//==============================================================================
// SamplerEngine
//==============================================================================
SamplerEngine::SamplerEngine()
{
    {
        juce::String note;
        note << "Render kernel: " << kernelName(bestKernel()) << " (default " << kernelName(defaultKernel());
        if (const char* env = std::getenv("ORGAN_KERNEL")) note << ", ORGAN_KERNEL=" << env;
        note << ")";
        CrashHandler::note(note);
    }
    for (auto& c : compassLow) c.store(0);
    for (auto& c : compassHigh) c.store(127);
    impl = std::make_unique<Impl>(*this);
}

void SamplerEngine::setManualCompass(int manualIndex, int lowestKey, int highestKey)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    lowestKey = juce::jlimit(0, 127, lowestKey);
    highestKey = juce::jlimit(lowestKey, 127, highestKey);
    {
        const juce::ScopedLock sl(controlLock);
        compassLow[(size_t)manualIndex].store(lowestKey, std::memory_order_relaxed);
        compassHigh[(size_t)manualIndex].store(highestKey, std::memory_order_relaxed);
        refillAllStops();   // the pipes the routes need changed
    }
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

SamplerEngine::~SamplerEngine()
{
    stopRecording();
    waitForRemovedStopsFreed();   // a free thread of an unload still running touches the lists (0.8.184)
    // Background copy builds (launched threads) must be over before the stops and
    // the engine go: tell them to stop at the next pipe (a full build of a stop's
    // pitch-shifted copies takes up to a minute), wait for the ones still
    // starting, then take the build lock.
    buildsAborted.store(true, std::memory_order_release);
    // Every worker (builds, cache exports) must be gone before the lock and the
    // stops go: they all stop at their next pipe, so this is short.
    for (int waited = 0; backgroundBuilds.load(std::memory_order_acquire) > 0 && waited < 300000; waited += 10)
        juce::Thread::sleep(10);
    if (backgroundBuilds.load(std::memory_order_acquire) > 0)
        CrashHandler::note("Engine closing with " + juce::String(backgroundBuilds.load()) + " worker(s) still running");
    antiAliasBuildLock.enter();   // a running build finishes first
    stopControlThread();
    impl->suspend();
    impl.reset();
}

void SamplerEngine::startControlThread()
{
    if (impl->controlThreadRunning.load(std::memory_order_acquire)) return;
    impl->controlThreadStop.store(false, std::memory_order_release);
    impl->controlThreadRunning.store(true, std::memory_order_release);
    impl->controlThread = std::thread([this] { impl->controlLoop(); });
    CrashHandler::note("Control thread started: the note bookkeeping runs beside the audio callbacks");
}

void SamplerEngine::stopControlThread()
{
    if (!impl->controlThreadRunning.load(std::memory_order_acquire)) return;
    impl->controlThreadStop.store(true, std::memory_order_release);
    impl->controlWake.signal();
    if (impl->controlThread.joinable()) impl->controlThread.join();
    impl->controlThreadRunning.store(false, std::memory_order_release);
}

bool SamplerEngine::isControlThreadRunning() const noexcept { return impl->controlThreadRunning.load(std::memory_order_acquire); }

juce::String SamplerEngine::takeLastLoadError()
{
    std::lock_guard<std::mutex> lk(impl->pendingMutex);
    auto result = lastLoadError;
    lastLoadError.clear();
    loadMemorySkipped.clear();
    gSwapBaseline.store(-1);   // the next load takes its own baseline
    return result;
}

bool SamplerEngine::startRecording(const juce::File& file)
{
    stopRecording();
    auto rec = std::make_unique<Recorder>();
    double rate = getSampleRate();
    const int count = impl->routeCount.load(std::memory_order_acquire);
    const double now = nowMs();
    for (int i = 0; i < count; ++i)
    {
        const Route& r = *impl->routes[(size_t)i];
        if (now - r.lastRenderMs.load(std::memory_order_relaxed) < 1000.0)
        {
            rate = r.sampleRate.load(std::memory_order_relaxed);
            break;
        }
    }
    rec->sampleRate = rate > 1000.0 ? rate : 48000.0;
    rec->fadeFrames = juce::jmax<int64_t>(1, (int64_t)std::llround(0.5 * rec->sampleRate));
    rec->file = file;
    file.deleteFile();
    std::unique_ptr<juce::FileOutputStream> fileStream(file.createOutputStream());
    if (fileStream == nullptr || !fileStream->openedOk()) return false;
    std::unique_ptr<juce::OutputStream> stream(std::move(fileStream));
    juce::WavAudioFormat wav;
    rec->writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions{}
        .withSampleRate(rec->sampleRate).withNumChannels(2).withBitsPerSample(24));
    if (rec->writer == nullptr) return false;   // on success the writer took the stream (stream is null now)
    try
    {
        rec->flushBuffer.setSize(2, 8192);
    }
    catch (const std::bad_alloc&) { return false; }
    rec->epoch = (uint32_t)(juce::Time::getMillisecondCounter() | 1u);
    rec->wallStartMs = nowMs();
    for (int i = 0; i < count; ++i)
    {
        auto& r = *impl->routes[(size_t)i];
        r.recPosPublished.store(-1, std::memory_order_relaxed);
        impl->allocateRecordingRing(r);   // a route without a ring is simply not recorded
    }
    impl->recorder = std::move(rec);
    impl->recorderThread = std::make_unique<Impl::RecorderThread>(*impl);
    recordingActive.store(true, std::memory_order_release);
    // Below normal: the writer must never compete with audio threads. The
    // per-route rings hold about 11 s, so it may lag behind and catch up later.
    impl->recorderThread->startThread(juce::Thread::Priority::low);
    return true;
}

void SamplerEngine::beginRecordingFadeOut()
{
    if (impl == nullptr || impl->recorder == nullptr || !recordingActive.load(std::memory_order_acquire)) return;
    Recorder& rec = *impl->recorder;
    if (rec.fadeOutStart.load(std::memory_order_acquire) >= 0) return;
    // "Now" is the furthest position any live route has rendered to.
    int64_t position = rec.flushed.load(std::memory_order_acquire);
    const double now = nowMs();
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        const Route& r = *impl->routes[(size_t)i];
        if (now - r.lastRenderMs.load(std::memory_order_relaxed) > 1000.0) continue;
        position = juce::jmax(position, r.recPosPublished.load(std::memory_order_acquire));
    }
    rec.fadeOutStart.store(position, std::memory_order_release);
}

bool SamplerEngine::isRecordingFadeOutComplete() const noexcept
{
    if (impl == nullptr || impl->recorder == nullptr) return true;
    const Recorder& rec = *impl->recorder;
    const int64_t start = rec.fadeOutStart.load(std::memory_order_acquire);
    return start >= 0 && rec.flushed.load(std::memory_order_acquire) >= start + rec.fadeFrames;
}

void SamplerEngine::stopRecording()
{
    if (impl == nullptr || !recordingActive.load(std::memory_order_acquire)) return;
    recordingActive.store(false, std::memory_order_release);
    // Wait for renders in flight so nobody touches the ring afterwards.
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        const juce::SpinLock::ScopedLockType lk(impl->routes[(size_t)i]->renderLock);
    }
    if (impl->recorderThread != nullptr)
    {
        impl->recorderThread->stopThread(2000);
        impl->recorderThread.reset();
    }
    impl->flushRecording(true);
    if (impl->recorder != nullptr)
    {
        const double wallSeconds = impl->recorder->wallStartMs > 0.0 ? (nowMs() - impl->recorder->wallStartMs) / 1000.0 : 0.0;
        const double fileSeconds = (double)impl->recorder->flushed.load(std::memory_order_acquire) / juce::jmax(1.0, impl->recorder->sampleRate);
        CrashHandler::note("Recording stopped: " + juce::String(fileSeconds, 1) + " s in the file over " + juce::String(wallSeconds, 1) + " s of time");
        impl->recorder->writer.reset();   // finalises the WAV header
        impl->recorder.reset();
    }
    for (int i = 0; i < count; ++i)
    {
        auto& r = *impl->routes[(size_t)i];
        r.recPosPublished.store(-1, std::memory_order_relaxed);
        for (auto& ring : r.recRing) { ring.clear(); ring.shrink_to_fit(); }
    }
}

double SamplerEngine::getRecordedSeconds() const noexcept
{
    if (impl == nullptr || impl->recorder == nullptr) return 0.0;
    return (double)impl->recorder->flushed.load(std::memory_order_relaxed) / juce::jmax(1.0, impl->recorder->sampleRate);
}

void SamplerEngine::setSampleRate(double sr)
{
    if (sr > 0.0) defaultSampleRate.store(sr);
}

int SamplerEngine::getRenderWorkerCount() const noexcept { return impl->pool->getWorkerCount(); }

double SamplerEngine::getLastBlockMs() const noexcept { return impl->lastBlockMs.load(std::memory_order_relaxed); }

juce::String SamplerEngine::routeVoiceSummary() const
{
    juce::String text;
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        const Route& r = *impl->routes[(size_t)i];
        const int voices = r.activeCount.load(std::memory_order_relaxed);
        if (voices > 0) text << (text.isEmpty() ? "" : ", ") << "manual " << r.manual << " " << r.perspective << ": " << voices;
    }
    return text;
}

juce::String SamplerEngine::routeLabel(int routeIndex) const
{
    if (routeIndex < 0 || routeIndex >= impl->routeCount.load(std::memory_order_acquire)) return "?";
    const Route& r = *impl->routes[(size_t)routeIndex];
    return "manual " + juce::String(r.manual) + " " + r.perspective;
}

bool SamplerEngine::isUsingSimd() const noexcept { return bestKernel() != KernelScalar && !simdDisabled.load(); }
const char* SamplerEngine::simdKernelName() noexcept { return kernelName(bestKernel()); }
const char* SamplerEngine::currentKernelName() const noexcept { return kernelName(simdDisabled.load() ? KernelScalar : bestKernel()); }
juce::StringArray SamplerEngine::availableKernelNames()
{
    juce::StringArray names;
    for (const char* name : { "scalar", "sse2", "avx2", "neon" })
        if (kernelKindByName(name) >= 0) names.add(name);
    return names;
}
int SamplerEngine::kernelKindForName(const juce::String& name) noexcept { return kernelKindByName(name); }
const char* SamplerEngine::kernelDisplayName(int kind) noexcept { return kernelName((KernelKind)kind); }
const char* SamplerEngine::automaticKernelName() noexcept { return kernelName(envKernel()); }
void SamplerEngine::setKernelOverride(int kind) noexcept
{
    const int applied = kind >= 0 && kernelAvailable(kind) ? kind : -1;
    if (gKernelOverride.exchange(applied) != applied)
        CrashHandler::note(juce::String("Render kernel: ") + (applied < 0 ? juce::String("automatic (") + kernelName(envKernel()) + ")" : juce::String(kernelName((KernelKind)applied))));
}
int SamplerEngine::getKernelOverride() noexcept { return gKernelOverride.load(); }

SamplerEngine::RenderPoolStats SamplerEngine::takeRenderPoolStats() noexcept
{
    RenderPoolStats stats;
    if (impl != nullptr && impl->pool != nullptr)
        impl->pool->takeStats(stats.jobs, stats.laneUtilisation, stats.posterShare, stats.jobSeconds,
            stats.profiledVoices, stats.voiceHeaderNs, stats.voiceSpansNs, stats.voiceKernelNs, stats.voicePostNs, stats.voiceUnpackNs, stats.laneJoinDelayMaxNs);
    stats.lanes = getRenderWorkerCount() + 1;
    if (impl != nullptr)
    {
        auto& i = *impl;
        const int64_t calls = i.statRouteCalls.exchange(0, std::memory_order_relaxed);
        const double per = calls > 0 ? 1.0e9 / (double)profileTicksPerSecond() / (double)calls : 0.0;
        auto take = [&](std::atomic<int64_t>& a) { return (double)a.exchange(0, std::memory_order_relaxed) * per; };
        stats.routeCallbacks = calls;
        stats.routeTakeNs = take(i.statRouteTake);
        const double loop = take(i.statRouteLoop);
        stats.routePoolOwnNs = take(i.statRoutePoolOwn);
        stats.routePoolWaitNs = take(i.statRoutePoolWait);
        stats.routeMixNs = juce::jmax(0.0, loop - stats.routePoolOwnNs - stats.routePoolWaitNs);
        stats.routeCompactNs = take(i.statRouteCompact);
        stats.routeSeparationNs = take(i.statRouteSeparation);
        stats.routeReverbNs = take(i.statRouteReverb);
        stats.routeReverbWaitNs = take(i.statRouteReverbWait);
        stats.routeClipNs = take(i.statRouteClip);
        stats.routeRecordNs = take(i.statRouteRecord);
        stats.routeStatsNs = take(i.statRouteStats);
        const double tickNs = 1.0e9 / (double)profileTicksPerSecond();
        stats.routeMaxTotalNs = (double)i.statRouteMaxTotal.exchange(0, std::memory_order_relaxed) * tickNs;
        stats.routeMaxOwnNs = (double)i.statRouteMaxOwn.exchange(0, std::memory_order_relaxed) * tickNs;
        stats.routeMaxWaitNs = (double)i.statRouteMaxWait.exchange(0, std::memory_order_relaxed) * tickNs;
    }
    return stats;
}

void SamplerEngine::setVoiceProfiling(bool enabled) noexcept
{
    if (enabled) calibrateProfileClock();   // 100 ms on the message thread, the first time only
    gVoiceProfiling.store(enabled, std::memory_order_relaxed);
}

namespace
{
    // The SIMD kernels this build has for this CPU, besides the scalar one.
    std::vector<KernelKind> simdKernelsAvailable()
    {
        std::vector<KernelKind> kinds;
       #if ENGINE_ARM64
        kinds.push_back(KernelNeon);
       #elif ENGINE_HAS_LANE4_KERNELS
        kinds.push_back(KernelSse2);
       #endif
       #if ENGINE_HAS_AVX2_KERNELS
        if (cpuHasAvx2()) kinds.push_back(KernelAvx2);
       #endif
        return kinds;
    }

    // A buffer of int16 or float frames with the engine's guard frames, filled
    // with a deterministic signal (sines of a few frequencies plus a little noise).
    template <typename T>
    std::vector<T> testSignal(size_t frames, uint32_t seed, float scale)
    {
        std::vector<T> s(frames + 32);
        uint32_t state = seed | 1u;
        for (size_t i = 0; i < frames; ++i)
        {
            state = state * 1664525u + 1013904223u;
            const double noise = ((double)(state >> 8) / 16777216.0 - 0.5) * 0.05;
            const double v = 0.6 * std::sin((double)i * 0.037 + seed) + 0.3 * std::cos((double)i * 0.011) * std::sin((double)i * 0.29) + noise;
            if constexpr (std::is_same<T, int16_t>::value) s[i] = (int16_t)std::lrint(juce::jlimit(-32767.0, 32767.0, v * 32000.0 * scale));
            else s[i] = (float)(v * scale);
        }
        return s;
    }
}

juce::String SamplerEngine::KernelBenchmark::describe() const
{
    if (blockFrames <= 0) return "Render kernel: not measured.";
    juce::String text;
    text << "Render kernel " << (kernel.isEmpty() ? juce::String("scalar") : kernel) << ": " << juce::String(kernel.isEmpty() ? nsCacheScalar : nsCacheSimd, 2)
         << " ns per voice and frame with the sample in the cache";
    if (kernel.isNotEmpty()) text << " (scalar " << juce::String(nsCacheScalar, 2) << ")";
    if (ramBytes > 0)
    {
        text << ", " << juce::String(kernel.isEmpty() ? nsRamScalar : nsRamSimd, 2) << " ns streaming from RAM";
        if (kernel.isNotEmpty()) text << " (scalar " << juce::String(nsRamScalar, 2) << ")";
    }
    text << "; stereo 16-bit, blocks of " << blockFrames << " frames, one thread.";
    return text;
}

SamplerEngine::KernelBenchmark SamplerEngine::benchmarkKernel(int blockFrames, juce::int64 ramBytes)
{
    // What one thread renders: ns per voice and frame of the interpolation
    // kernel alone, (1) with the sample in the cache - the pure compute speed
    // of the kernel on this CPU - and (2) streaming from RAM, every voice from
    // a fresh place of a large buffer - what an organ of many GB really costs
    // (the memory system, not the arithmetic, is then the larger part). The
    // ceiling of the polyphony follows: threads x (85 % of the block) / (frames
    // x ns); the rest of a callback (bookkeeping, hand-over, waiting for the
    // slowest lane) is the measured difference to it.
    KernelBenchmark b;
    b.blockFrames = juce::jlimit(64, kMaxBlockFrames, blockFrames);
    const int n = b.blockFrames;
    const KernelKind kind = bestKernel();
    if (kind != KernelScalar) b.kernel = kernelName(kind);
    std::vector<float> o0((size_t)n), o1((size_t)n);
    auto nsPerVoiceFrame = [&](KernelKind k, const int16_t* c0, const int16_t* c1, size_t framesAvail, int voices, double seconds)
    {
        uint32_t rnd = 12345u;
        auto next = [&rnd] { rnd = rnd * 1664525u + 1013904223u; return (double)(rnd >> 8) / 16777216.0; };
        const double maxStart = juce::jmax(1.0, (double)framesAvail - (double)n * 1.1 - 40.0);
        const double tickSeconds = 1.0 / (double)juce::Time::getHighResolutionTicksPerSecond();
        const int64_t start = juce::Time::getHighResolutionTicks();
        const int64_t until = start + (int64_t)(seconds / tickSeconds);
        int64_t rendered = 0;
        int64_t last = start;
        do
        {
            for (int v = 0; v < voices; ++v)
            {
                uint64_t pos = (uint64_t)(next() * maxStart * kFixedOne);
                int64_t inc = (int64_t)((0.94 + 0.12 * next()) * kFixedOne);
                dispatchStereo(c0, c1, o0.data(), o1.data(), n, pos, inc, 0, 0.5f, 0.0f, k);
            }
            rendered += voices;
            last = juce::Time::getHighResolutionTicks();
        } while (last < until);
        return (double)(last - start) * tickSeconds * 1.0e9 / ((double)rendered * (double)n);
    };
    {
        const auto s0 = testSignal<int16_t>(16384, 3, 1.0f), s1 = testSignal<int16_t>(16384, 5, 0.9f);
        b.nsCacheScalar = nsPerVoiceFrame(KernelScalar, s0.data(), s1.data(), 16384, 64, 0.15);
        if (kind != KernelScalar) b.nsCacheSimd = nsPerVoiceFrame(kind, s0.data(), s1.data(), 16384, 64, 0.15);
    }
    const size_t ramFrames = (size_t)juce::jlimit((juce::int64)(8 << 20), (juce::int64)(1 << 30), ramBytes) / 4;   // two int16 channels
    try
    {
        std::vector<int16_t> r0(ramFrames + 32), r1(ramFrames + 32);
        // Filled fast with a 32-bit generator per pair of frames: real data, not
        // the zero pages a fresh allocation would hand out (those are never read).
        uint32_t rnd = 77u;
        for (size_t i = 0; i + 1 < ramFrames; i += 2)
        {
            rnd = rnd * 1664525u + 1013904223u;
            r0[i] = (int16_t)(rnd & 0x7fff); r0[i + 1] = (int16_t)((rnd >> 16) & 0x7fff);
            r1[i] = (int16_t)((rnd >> 3) & 0x7fff); r1[i + 1] = (int16_t)((rnd >> 19) & 0x7fff);
        }
        b.ramBytes = (juce::int64)ramFrames * 4;
        b.nsRamScalar = nsPerVoiceFrame(KernelScalar, r0.data(), r1.data(), ramFrames, 2048, 0.2);
        if (kind != KernelScalar) b.nsRamSimd = nsPerVoiceFrame(kind, r0.data(), r1.data(), ramFrames, 2048, 0.2);
    }
    catch (const std::bad_alloc&) { b.ramBytes = 0; }
    return b;
}

juce::String SamplerEngine::runKernelSelfCheck()
{
    // Every SIMD kernel of this CPU against the scalar one: several pitch ratios
    // (below 1, 1, between 1 and 2, above 2 - the AVX2 kernels change their
    // load strategy at 2:1), a rising and a falling increment, a gain ramp;
    // int16 and float, mono and stereo. The end position must be identical
    // (integer arithmetic), the samples equal within a rounding difference.
    juce::String result;
    const auto kinds = simdKernelsAvailable();
    result << "Kernels: scalar";
    for (const auto kind : kinds) result << ", " << kernelName(kind);
    result << " - this CPU renders with " << kernelName(bestKernel()) << ".\n";
    if (kinds.empty()) return result;
    constexpr int frames = 16384, n = 960;
    const auto s0 = testSignal<int16_t>(frames, 11, 1.0f), s1 = testSignal<int16_t>(frames, 29, 0.8f);
    const auto f0 = testSignal<float>(frames, 11, 1.0f), f1 = testSignal<float>(frames, 29, 0.8f);
    struct Case { double ratio, step; };
    const Case cases[]{ { 0.5, 0.0 }, { 0.9997, 0.0 }, { 1.0, 0.0 }, { 1.0873, 2.1e-6 }, { 1.9, -1.0e-6 }, { 2.6, 0.0 }, { 3.7, 4.0e-6 } };
    for (const auto kind : kinds)
    {
        float worst = 0.0f;
        bool positionsEqual = true;
        juce::String worstCase;
        auto run = [&](auto* c0, auto* c1, float scale, const char* name)
        {
            for (const auto& cs : cases)
            {
                const int64_t inc = (int64_t)(cs.ratio * kFixedOne), incStep = (int64_t)(cs.step * kFixedOne);
                const float g = 0.7f * scale, gStep = -0.0002f * scale;
                std::vector<float> a0((size_t)n, 0.25f), a1((size_t)n, -0.5f), b0(a0), b1(a1), m0(a0), m1(a0);
                uint64_t posA = (uint64_t)(17.31 * kFixedOne), posB = posA, posC = posA, posD = posA;
                int64_t incA = inc, incB = inc, incC = inc, incD = inc;
                kernelStereo(c0, c1, a0.data(), a1.data(), n, posA, incA, incStep, g, gStep);
                dispatchStereo(c0, c1, b0.data(), b1.data(), n, posB, incB, incStep, g, gStep, kind);
                kernelMono(c0, m0.data(), n, posC, incC, incStep, g, gStep);
                dispatchMono(c0, m1.data(), n, posD, incD, incStep, g, gStep, kind);
                // The shelf kernels: SIMD against scalar, and scalar against the
                // identity out + wet = the plain kernel's output (k ramping 0.1 -> 0.6).
                std::vector<float> sa0((size_t)n, 0.25f), sa1((size_t)n, -0.5f), wa0((size_t)n, 0.0f), wa1((size_t)n, 0.0f), sb0(sa0), sb1(sa1), wb0(wa0), wb1(wa1);
                uint64_t posE = (uint64_t)(17.31 * kFixedOne), posF = posE;
                int64_t incE = inc, incF = inc;
                const float k0 = 0.1f, kStep = 0.5f / (float)n;
                kernelStereoShelf(c0, c1, sa0.data(), sa1.data(), wa0.data(), wa1.data(), n, posE, incE, incStep, g, gStep, k0, kStep);
                dispatchStereoShelf(c0, c1, sb0.data(), sb1.data(), wb0.data(), wb1.data(), n, posF, incF, incStep, g, gStep, k0, kStep, kind);
                if (posE != posF || incE != incF) positionsEqual = false;
                // The 8-tap sinc kernels (0.8.164): SIMD against scalar, mono and stereo.
                std::vector<float> xa0((size_t)n, 0.25f), xa1((size_t)n, -0.5f), xb0(xa0), xb1(xa1), xm0(xa0), xm1(xa0);
                uint64_t posG = (uint64_t)(17.31 * kFixedOne), posH = posG, posI = posG, posJ = posG;
                int64_t incG = inc, incH = inc, incI = inc, incJ = inc;
                kernelStereoSinc(c0, c1, xa0.data(), xa1.data(), n, posG, incG, incStep, g, gStep);
                dispatchStereoSinc(c0, c1, xb0.data(), xb1.data(), n, posH, incH, incStep, g, gStep, kind);
                kernelMonoSinc(c0, xm0.data(), n, posI, incI, incStep, g, gStep);
                dispatchMonoSinc(c0, xm1.data(), n, posJ, incJ, incStep, g, gStep, kind);
                if (posG != posH || posI != posJ || incG != incH || incI != incJ) positionsEqual = false;
                // ... and their loop splices (a head read 4000 frames back, the mix ramping 0.2 -> 0.9).
                std::vector<float> ya0((size_t)n, 0.25f), ya1((size_t)n, -0.5f), yb0(ya0), yb1(ya1), ym0(ya0), ym1(ya0);
                uint64_t posK = (uint64_t)(4517.31 * kFixedOne), posL = posK, posM = posK, posN = posK;
                int64_t incK = inc, incL = inc, incM = inc, incN = inc;
                const int64_t headOff = -(int64_t)((uint64_t)4000 << 32);
                kernelStereoXfSinc(c0, c1, ya0.data(), ya1.data(), n, posK, incK, incStep, g, gStep, headOff, 0.2f, 0.7f / (float)n);
                dispatchStereoXfSinc(c0, c1, yb0.data(), yb1.data(), n, posL, incL, incStep, g, gStep, headOff, 0.2f, 0.7f / (float)n, kind);
                kernelMonoXfSinc(c0, ym0.data(), n, posM, incM, incStep, g, gStep, headOff, 0.2f, 0.7f / (float)n);
                dispatchMonoXfSinc(c0, ym1.data(), n, posN, incN, incStep, g, gStep, headOff, 0.2f, 0.7f / (float)n, kind);
                if (posK != posL || posM != posN || incK != incL || incM != incN) positionsEqual = false;
                float maxDiff = 0.0f, maxLevel = 1.0e-9f;
                for (int i = 0; i < n; ++i)
                {
                    maxDiff = juce::jmax(maxDiff, std::abs(a0[(size_t)i] - b0[(size_t)i]), std::abs(a1[(size_t)i] - b1[(size_t)i]), std::abs(m0[(size_t)i] - m1[(size_t)i]));
                    maxDiff = juce::jmax(maxDiff, std::abs(xa0[(size_t)i] - xb0[(size_t)i]), std::abs(xa1[(size_t)i] - xb1[(size_t)i]), std::abs(xm0[(size_t)i] - xm1[(size_t)i]));
                    maxDiff = juce::jmax(maxDiff, std::abs(ya0[(size_t)i] - yb0[(size_t)i]), std::abs(ya1[(size_t)i] - yb1[(size_t)i]), std::abs(ym0[(size_t)i] - ym1[(size_t)i]));
                    maxDiff = juce::jmax(maxDiff, std::abs(sa0[(size_t)i] - sb0[(size_t)i]), std::abs(sa1[(size_t)i] - sb1[(size_t)i]));
                    maxDiff = juce::jmax(maxDiff, std::abs(wa0[(size_t)i] - wb0[(size_t)i]), std::abs(wa1[(size_t)i] - wb1[(size_t)i]));
                    maxDiff = juce::jmax(maxDiff, std::abs((sa0[(size_t)i] + wa0[(size_t)i]) - a0[(size_t)i]), std::abs((sa1[(size_t)i] + wa1[(size_t)i]) - a1[(size_t)i]));
                    maxLevel = juce::jmax(maxLevel, std::abs(a0[(size_t)i]), std::abs(a1[(size_t)i]));
                }
                const float relative = maxDiff / maxLevel;
                if (relative > worst) { worst = relative; worstCase = juce::String(name) + " ratio " + juce::String(cs.ratio, 4); }
                if (posA != posB || posC != posD || incA != incB || incC != incD) positionsEqual = false;
            }
        };
        run(s0.data(), s1.data(), 1.0f / 32768.0f, "16-bit");
        run(f0.data(), f1.data(), 1.0f, "float");
        result << "Kernel check " << kernelName(kind) << " vs scalar: largest difference " << juce::String(worst * 100.0f, 5) << " % of the peak ("
               << worstCase << "), end positions " << (positionsEqual ? "identical" : "DIFFERENT") << " -> "
               << (worst < 1.0e-4f && positionsEqual ? "OK" : "*** MISMATCH ***") << "\n";
    }
    const auto bench = benchmarkKernel(n, 256 << 20);
    result << bench.describe() << "\n";
    {
        // The sinc kernel's price against the linear one (0.8.164): the same mono
        // 16-bit signal from the cache, ratio 1.0116 (a 20-cent retune), the
        // best kernel of this CPU; what "Full kernel from" costs per voice.
        const KernelKind best = bestKernel();
        constexpr int reps = 3000;
        const int64_t inc = (int64_t)(1.0116 * kFixedOne);
        std::vector<float> o((size_t)n, 0.0f);
        auto timeIt = [&](auto&& fn)
        {
            const int64_t t0 = juce::Time::getHighResolutionTicks();
            for (int r = 0; r < reps; ++r) { uint64_t pos = (uint64_t)(17.31 * kFixedOne); int64_t i2 = inc; fn(pos, i2); }
            const double seconds = (double)(juce::Time::getHighResolutionTicks() - t0) / (double)juce::Time::getHighResolutionTicksPerSecond();
            return 1.0e9 * seconds / ((double)reps * (double)n);
        };
        const double linearNs = timeIt([&](uint64_t& pos, int64_t& i2) { dispatchMono(s0.data(), o.data(), n, pos, i2, (int64_t)0, 0.7f, 0.0f, best); });
        const double sincNs = timeIt([&](uint64_t& pos, int64_t& i2) { dispatchMonoSinc(s0.data(), o.data(), n, pos, i2, (int64_t)0, 0.7f, 0.0f, best); });
        result << "Sinc kernel (" << kernelName(best) << "): " << juce::String(sincNs, 2) << " ns per voice and frame against "
               << juce::String(linearNs, 2) << " for the linear kernel (" << juce::String(sincNs / juce::jmax(1.0e-9, linearNs), 1)
               << " x; the price of \"Full kernel from\" per pipe above it).\n";
    }
    {
        // Packed samples (0.8.145): a stereo sample with two loops - a silent
        // stretch, a full-scale step and a quiet tail among the blocks - packed
        // and unpacked: every window back as it was (negative starts and reads past
        // the end included), and a voice rendered through the packed path equal to
        // the raw one over plain spans, both splices and the cubic kernel.
        constexpr int64_t pf = 120000;
        auto raw = std::make_unique<Sample>(2, pf, 48000.0, false);
        auto packedS = std::make_unique<Sample>(2, pf, 48000.0, false);
        {
            const auto t0 = testSignal<int16_t>((size_t)pf, 7, 1.0f), t1 = testSignal<int16_t>((size_t)pf, 13, 0.7f);
            for (int64_t i = 0; i < pf; ++i)
            {
                int16_t a = t0[(size_t)i], b = t1[(size_t)i];
                if (i >= 20000 && i < 21000) { a = 0; b = 0; }
                if (i == 30000) { a = 32767; b = -32768; }
                if (i >= 90000) { a = (int16_t)(a / 64); b = (int16_t)(b / 64); }
                raw->writable16(0)[i] = a; raw->writable16(1)[i] = b;
                packedS->writable16(0)[i] = a; packedS->writable16(1)[i] = b;
            }
            const std::vector<LoopPoints> loops{ { 40000, 80000 }, { 50000, 85000 } };
            for (Sample* s : { raw.get(), packedS.get() })
            {
                s->finaliseLoops(loops);
                applyCopyLoopCrossfade(*s, 0.3);   // the splice after the jump on both loops ...
                s->crossfadeAfterJump[0] = 0; s->crossfadeFrames[0] = 2400;   // ... the first one spliced before its end instead
            }
        }
        const double packStart = juce::Time::getMillisecondCounterHiRes();
        const size_t packedBytes = packedS->compress16();
        const double packMs = juce::Time::getMillisecondCounterHiRes() - packStart;
        bool ok = packedBytes > 0 && packedS->compressed;
        int64_t windowsChecked = 0;
        {
            std::vector<int16_t> a(5000), b(5000);
            uint32_t st = 99;
            for (int t = 0; t < 400 && ok; ++t)
            {
                st = st * 1664525u + 1013904223u; const int64_t from = (int64_t)(st % 130000u) - 5000;
                st = st * 1664525u + 1013904223u; const int64_t len = 1 + (int64_t)(st % 4999u);
                const int ch = t & 1;
                raw->decode16(ch, from, from + len, a.data());
                packedS->decode16(ch, from, from + len, b.data());
                if (std::memcmp(a.data(), b.data(), (size_t)len * sizeof(int16_t)) != 0) ok = false;
                ++windowsChecked;
            }
        }
        // Unpacking speed: the whole sample in windows of 1048 frames (a block of 1024 plus the guard).
        double unpackNsPerFrame = 0.0;
        {
            std::vector<int16_t> w(1048);
            const double t0 = juce::Time::getMillisecondCounterHiRes();
            int64_t decodedFrames = 0;
            for (int rep = 0; rep < 5; ++rep)
                for (int64_t from = -4; from < pf; from += 1024) { packedS->decode16(0, from, from + 1048, w.data()); decodedFrames += 1048; }
            unpackNsPerFrame = (juce::Time::getMillisecondCounterHiRes() - t0) * 1.0e6 / (double)juce::jmax<int64_t>(1, decodedFrames);
        }
        float worstRender = 0.0f;
        bool positionsSame = true;
        if (ok)
        {
            auto renderWith = [&](const Sample& s, bool cubic, double ratio, std::vector<float>& out0, std::vector<float>& out1, uint64_t& endPos)
            {
                auto v = std::make_unique<Voice>();
                v->sample = &s; v->note = 0; v->cubic = cubic; v->loopEnabled = true; v->rng = 12345;
                v->pos = (uint64_t)(100.0 * kFixedOne);
                BlockContext c; c.numSamples = 480; c.outCount = 2; c.outChannels[0] = 0; c.outChannels[1] = 1; c.kernel = (int)bestKernel();
                std::vector<float> sink((size_t)kMaxBlockFrames, 0.0f);
                LaneOutputs lanes; lanes.numOuts = 2; lanes.sink = sink.data();
                const int64_t inc = (int64_t)(ratio * kFixedOne);
                std::vector<float> o0(480), o1(480);
                for (int block = 0; block < 500 && !v->finished; ++block)
                {
                    std::fill(o0.begin(), o0.end(), 0.0f); std::fill(o1.begin(), o1.end(), 0.0f);
                    lanes.out[0] = o0.data(); lanes.out[1] = o1.data();
                    renderVoiceSpans<int16_t>(*v, s, c, lanes, inc, inc, 0.5f, 0.5f);
                    out0.insert(out0.end(), o0.begin(), o0.end());
                    out1.insert(out1.end(), o1.begin(), o1.end());
                }
                endPos = v->pos;
            };
            for (const double rateOfSample : { 48000.0, 24000.0 })   // 24 kHz: the sinc kernel of the first 1.3 s (0.8.147)
            for (const bool cubic : { false, true })
                for (const double ratio : { 0.75, 1.0, 1.4983, 2.6 })
                {
                    raw->sampleRate = packedS->sampleRate = rateOfSample;
                    raw->sincHead = packedS->sincHead = rateOfSample < 30000.0;
                    std::vector<float> r0, r1, p0, p1;
                    uint64_t endRaw = 0, endPacked = 0;
                    renderWith(*raw, cubic, ratio, r0, r1, endRaw);
                    renderWith(*packedS, cubic, ratio, p0, p1, endPacked);
                    if (endRaw != endPacked || r0.size() != p0.size()) positionsSame = false;
                    float peak = 1.0e-9f, diff = 0.0f;
                    for (size_t i = 0; i < juce::jmin(r0.size(), p0.size()); ++i)
                    {
                        peak = juce::jmax(peak, std::abs(r0[i]), std::abs(r1[i]));
                        diff = juce::jmax(diff, std::abs(r0[i] - p0[i]), std::abs(r1[i] - p1[i]));
                    }
                    worstRender = juce::jmax(worstRender, diff / peak);
                }
            if (!(worstRender < 1.0e-5f) || !positionsSame) ok = false;
        }
        result << "Packed samples: " << juce::String((double)packedBytes / (double)(pf * 4) * 100.0, 1) << " % of the raw size, packed in "
               << juce::String(packMs, 1) << " ms, " << windowsChecked << " windows unpacked as stored, unpacking " << juce::String(unpackNsPerFrame, 2)
               << " ns per frame; a voice rendered from the packed sample against the raw one (plain, splices, cubic, sinc at 24 kHz, ratios 0.75-2.6): largest difference "
               << juce::String(worstRender * 100.0f, 5) << " % of the peak, end positions " << (positionsSame ? "identical" : "DIFFERENT") << " -> "
               << (ok ? "OK" : "*** MISMATCH ***") << "\n";
    }
    {
        // Packed float samples (0.8.183, Sample::compress32): the same stereo signal
        // as 24-bit integers in floats (a 16-bit value plus a few low bits), packed
        // and unpacked bit for bit, and a voice rendered through the packed float
        // path equal to the raw one; a sample with floats that are no such integers
        // (a resampled one) must stay raw.
        constexpr int64_t pf = 120000;
        auto raw = std::make_unique<Sample>(2, pf, 48000.0, true);
        auto packedS = std::make_unique<Sample>(2, pf, 48000.0, true);
        {
            const auto t0 = testSignal<int16_t>((size_t)pf, 7, 1.0f), t1 = testSignal<int16_t>((size_t)pf, 13, 0.7f);
            for (int64_t i = 0; i < pf; ++i)
            {
                int32_t a = t0[(size_t)i], b = t1[(size_t)i];
                if (i >= 20000 && i < 21000) { a = 0; b = 0; }
                if (i == 30000) { a = 32767; b = -32768; }
                if (i >= 90000) { a /= 64; b /= 64; }
                const float fa = (float)(a * 256 + (int)(i % 5) - 2) / 8388608.0f, fb = (float)(b * 256 - (int)(i % 3)) / 8388608.0f;   // 24-bit values
                raw->writable32(0)[i] = fa; raw->writable32(1)[i] = fb;
                packedS->writable32(0)[i] = fa; packedS->writable32(1)[i] = fb;
            }
            const std::vector<LoopPoints> loops{ { 40000, 80000 }, { 50000, 85000 } };
            for (Sample* s : { raw.get(), packedS.get() })
            {
                s->finaliseLoops(loops);
                applyCopyLoopCrossfade(*s, 0.3);
                s->crossfadeAfterJump[0] = 0; s->crossfadeFrames[0] = 2400;
            }
        }
        const size_t packedBytes = packedS->compress32();
        bool ok = packedBytes > 0 && packedS->compressed && packedS->packScale == 1.0f / 8388608.0f;
        int64_t windowsChecked = 0;
        {
            std::vector<float> a(5000), b(5000);
            uint32_t st = 99;
            for (int t = 0; t < 400 && ok; ++t)
            {
                st = st * 1664525u + 1013904223u; const int64_t from = (int64_t)(st % 130000u) - 5000;
                st = st * 1664525u + 1013904223u; const int64_t len = 1 + (int64_t)(st % 4999u);
                const int ch = t & 1;
                raw->decode32(ch, from, from + len, a.data());
                packedS->decode32(ch, from, from + len, b.data());
                if (std::memcmp(a.data(), b.data(), (size_t)len * sizeof(float)) != 0) ok = false;
                ++windowsChecked;
            }
        }
        float worstRender = 0.0f;
        bool positionsSame = true;
        if (ok)
        {
            auto renderWith = [&](const Sample& s, bool cubic, double ratio, std::vector<float>& out0, std::vector<float>& out1, uint64_t& endPos)
            {
                auto v = std::make_unique<Voice>();
                v->sample = &s; v->note = 0; v->cubic = cubic; v->loopEnabled = true; v->rng = 12345;
                v->pos = (uint64_t)(100.0 * kFixedOne);
                BlockContext c; c.numSamples = 480; c.outCount = 2; c.outChannels[0] = 0; c.outChannels[1] = 1; c.kernel = (int)bestKernel();
                std::vector<float> sink((size_t)kMaxBlockFrames, 0.0f);
                LaneOutputs lanes; lanes.numOuts = 2; lanes.sink = sink.data();
                const int64_t inc = (int64_t)(ratio * kFixedOne);
                std::vector<float> o0(480), o1(480);
                for (int block = 0; block < 500 && !v->finished; ++block)
                {
                    std::fill(o0.begin(), o0.end(), 0.0f); std::fill(o1.begin(), o1.end(), 0.0f);
                    lanes.out[0] = o0.data(); lanes.out[1] = o1.data();
                    renderVoiceSpans<float>(*v, s, c, lanes, inc, inc, 0.5f, 0.5f);
                    out0.insert(out0.end(), o0.begin(), o0.end());
                    out1.insert(out1.end(), o1.begin(), o1.end());
                }
                endPos = v->pos;
            };
            for (const bool cubic : { false, true })
                for (const double ratio : { 0.75, 1.0, 1.4983, 2.6 })
                {
                    std::vector<float> r0, r1, p0, p1;
                    uint64_t endRaw = 0, endPacked = 0;
                    renderWith(*raw, cubic, ratio, r0, r1, endRaw);
                    renderWith(*packedS, cubic, ratio, p0, p1, endPacked);
                    if (endRaw != endPacked || r0.size() != p0.size()) positionsSame = false;
                    float peak = 1.0e-9f, diff = 0.0f;
                    for (size_t i = 0; i < juce::jmin(r0.size(), p0.size()); ++i)
                    {
                        peak = juce::jmax(peak, std::abs(r0[i]), std::abs(r1[i]));
                        diff = juce::jmax(diff, std::abs(r0[i] - p0[i]), std::abs(r1[i] - p1[i]));
                    }
                    worstRender = juce::jmax(worstRender, diff / peak);
                }
            if (!(worstRender < 1.0e-5f) || !positionsSame) ok = false;
        }
        // Real floats (a 32-bit float WAV, a resampled sample): rounded to 24 bits -
        // every frame back within half a 24-bit step, marked as rounded; a sample
        // beyond +-2 full scale stays raw.
        bool roundedOk = false, rawStays = false;
        float roundError = 0.0f;
        {
            auto other = std::make_unique<Sample>(1, 4096, 48000.0, true);
            std::vector<float> original(4096);
            for (int64_t i = 0; i < 4096; ++i) { original[(size_t)i] = std::sin((float)i * 0.01f) * 0.5f; other->writable32(0)[i] = original[(size_t)i]; }
            roundedOk = other->compress32() > 0 && other->compressed && other->packRounded;
            if (roundedOk)
            {
                std::vector<float> back(4096);
                other->decode32(0, 0, 4096, back.data());
                for (int64_t i = 0; i < 4096; ++i) roundError = juce::jmax(roundError, std::abs(back[(size_t)i] - original[(size_t)i]));
                if (!(roundError <= 0.5f / 8388608.0f + 1.0e-9f)) roundedOk = false;
            }
            auto loud = std::make_unique<Sample>(1, 4096, 48000.0, true);
            for (int64_t i = 0; i < 4096; ++i) loud->writable32(0)[i] = std::sin((float)i * 0.01f) * 3.0f;
            rawStays = loud->compress32() == 0 && !loud->compressed;
        }
        if (!roundedOk || !rawStays) ok = false;
        result << "Packed float samples: " << juce::String((double)packedBytes / (double)(pf * 8) * 100.0, 1) << " % of the raw size, "
               << windowsChecked << " windows unpacked as stored; a voice rendered from the packed sample against the raw one (plain, splices, cubic, ratios 0.75-2.6): largest difference "
               << juce::String(worstRender * 100.0f, 5) << " % of the peak, end positions " << (positionsSame ? "identical" : "DIFFERENT")
               << "; real floats rounded to 24 bits " << (roundedOk ? "within half a step (" + juce::String(roundError * 8388608.0f, 3) + ")" : "WRONG")
               << ", a sample beyond full scale " << (rawStays ? "stays raw" : "WAS PACKED") << " -> " << (ok ? "OK" : "*** MISMATCH ***") << "\n";
    }
    {
        // Dynamic tone shelf: the lowpass behind it must start at 1.8 kHz (about
        // -1 dB) and be down about 10 dB an octave higher (transition ~1.8 kHz wide).
        float lp[5];
        butterworthLowpass(48000.0, kShelfCornerHz, lp);
        const double atStart = lowpass4GainDb(lp, 48000.0, kShelfStartHz);
        const double atDouble = lowpass4GainDb(lp, 48000.0, 2.0 * kShelfStartHz);
        const double atFour = lowpass4GainDb(lp, 48000.0, 4.0 * kShelfStartHz);
        const bool ok = std::abs(atStart + 1.06) < 0.3 && std::abs(atDouble + 9.74) < 0.5 && atFour < -25.0;
        result << "Harmonic filter lowpass (start " << juce::String(kShelfStartHz, 0) << " Hz): "
               << juce::String(atStart, 2) << " dB at the start, " << juce::String(atDouble, 2) << " dB an octave up, "
               << juce::String(atFour, 1) << " dB two octaves up -> " << (ok ? "OK" : "*** WRONG ***") << "\n";
    }
    {
        // Per-key tone shelf: unity at DC, the set gain at Nyquist, for +9 and -9 dB.
        for (const float db : { 9.0f, -9.0f })
        {
            float k[5];
            toneShelfCoefficients(48000.0, db, 700.0f, k);
            const double dc = (k[0] + k[1] + k[2]) / (1.0 + k[3] + k[4]);
            const double nyquist = (k[0] - k[1] + k[2]) / (1.0 - k[3] + k[4]);
            const double wanted = std::pow(10.0, db / 20.0);
            const bool ok = std::abs(dc - 1.0) < 1.0e-3 && std::abs(nyquist / wanted - 1.0) < 1.0e-3;
            result << "Per-key tone shelf (" << juce::String(db, 0) << " dB, start 700 Hz): DC gain "
                   << juce::String(dc, 4) << ", top gain " << juce::String(20.0 * std::log10(nyquist), 2)
                   << " dB -> " << (ok ? "OK" : "*** WRONG ***") << "\n";
        }
    }
    return result.trimEnd();
}

void SamplerEngine::setRenderWorkerAffinityMask(juce::uint64 mask) { gRenderWorkerAffinity.store((uint64_t)mask, std::memory_order_relaxed); }

void SamplerEngine::setRenderThreadCount(int workers, bool force)
{
    const int wanted = workers < 0 ? Impl::automaticWorkerCount() : juce::jlimit(0, 62, workers);
    if (!force && impl->pool != nullptr && impl->pool->getWorkerCount() == wanted) return;
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->pool.reset();
        impl->pool = std::make_unique<RenderPool>(wanted);
    }
    impl->resume();
}

void SamplerEngine::prepareRenderWorkers(int, int) {}

bool SamplerEngine::outputEdgeJump(const float* block, int n, EdgeState& state) noexcept
{
    if (block == nullptr || n <= 1) return false;
    float inner = 0.0f, peak = std::abs(block[0]);
    for (int i = 1; i < n; ++i)
    {
        inner += std::abs(block[i] - block[i - 1]);
        peak = juce::jmax(peak, std::abs(block[i]));
    }
    inner /= (float)(n - 1);
    bool jump = false;
    if (state.valid)
    {
        // A jump: the step into this block is more than twelve times the mean step of
        // this block and of the previous one, above -74 dBFS, and the previous block
        // was sounding (a chord starting on a silent route is a start; a voice starting
        // on a sounding route begins near zero; a loud block has a large mean step).
        const float step = std::abs(block[0] - state.last);
        jump = step > 2.0e-4f && state.peak > 1.0e-3f && step > 12.0f * (juce::jmax(inner, state.inner) + 1.0e-6f);
    }
    state.last = block[n - 1];
    state.inner = inner;
    state.peak = peak;
    state.valid = true;
    return jump;
}

int SamplerEngine::takeOutputDiscontinuities(juce::String& detail, int& noScratchLanes)
{
    detail.clear();
    noScratchLanes = impl->pool != nullptr ? impl->pool->takeNoScratch() : 0;
    int total = 0;
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        Route& r = *impl->routes[(size_t)i];
        const int afterRender = r.edgeJumps[0].exchange(0, std::memory_order_relaxed);
        const int atEnd = r.edgeJumps[1].exchange(0, std::memory_order_relaxed);
        if (afterRender == 0 && atEnd == 0) continue;
        total += juce::jmax(afterRender, atEnd);
        detail << (detail.isEmpty() ? "" : "; ") << "manual " << r.manual << " " << r.perspective << ": "
               << afterRender << " after the render, " << atEnd << " at the route's end";
    }
    return total;
}

//------------------------------------------------------------------------------
void SamplerEngine::noteOn(int midiNote, int velocity, int manualIndex)
{
    if (midiNote < 0 || midiNote >= 128) return;
    impl->pushMidi({ (uint8_t)midiNote, (uint8_t)juce::jlimit(0, 127, velocity),
        (int8_t)juce::jlimit(-1, maxManuals - 1, manualIndex), true });
}

void SamplerEngine::noteOff(int midiNote, int manualIndex)
{
    if (midiNote < 0 || midiNote >= 128) return;
    impl->pushMidi({ (uint8_t)midiNote, 0, (int8_t)juce::jlimit(-1, maxManuals - 1, manualIndex), false });
}

void SamplerEngine::noteOffAll()
{
    impl->clearMidi();
    const juce::ScopedLock sl(controlLock);
    impl->reclaimFinished();
    impl->killAllVoices();
}

juce::String SamplerEngine::debugState() const
{
    // What a silent organ is doing (ORGAN_TEST_NOTE, #90): read without locks, a snapshot.
    return juce::String("suspended ") + (impl->suspended.load(std::memory_order_acquire) ? "YES" : "no")
        + ", control thread " + (impl->controlThreadRunning.load(std::memory_order_acquire) ? "running" : "STOPPED")
        + ", MIDI commands waiting " + juce::String((int)(impl->midiTail.load(std::memory_order_acquire) - impl->midiHead.load(std::memory_order_acquire)))
        + ", active voices " + juce::String((int)impl->controlActive.size()) + ", free voices " + juce::String((int)impl->freeList.size())
        + ", routes " + juce::String(impl->routeCount.load(std::memory_order_acquire)) + ", stops " + juce::String(stops.size())
        + ", limit " + juce::String(maxPolyphony.load());
}

//------------------------------------------------------------------------------
void SamplerEngine::renderBlockForPerspective(juce::AudioBuffer<float>& buffer, float masterGain,
    int manualIndex, const juce::String& perspective, int laneLimit)
{
    if (impl->suspended.load(std::memory_order_acquire)) return;
    impl->controlTickIfPossible();
    Route* route = impl->findRoute(juce::jlimit(0, maxManuals - 1, manualIndex),
        perspective.isEmpty() ? juce::String("default") : perspective);
    if (route == nullptr) return;
    impl->renderRoute(*route, buffer, masterGain, laneLimit);
}

void SamplerEngine::renderBlockForManual(juce::AudioBuffer<float>& buffer, float masterGain, int manualIndex)
{
    if (impl->suspended.load(std::memory_order_acquire)) return;
    impl->controlTickIfPossible();
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        Route& r = *impl->routes[(size_t)i];
        if (manualIndex < 0 || r.manual == manualIndex) impl->renderRoute(r, buffer, masterGain);
    }
}

void SamplerEngine::renderBlock(juce::AudioBuffer<float>& buffer, float masterGain)
{
    renderBlockForManual(buffer, masterGain, -1);
}

//------------------------------------------------------------------------------
void SamplerEngine::setManualExpressionGain(int manualIndex, float gain)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    impl->expressionGain[(size_t)manualIndex].store(juce::jlimit(0.0f, 1.0f, gain), std::memory_order_relaxed);
}

void SamplerEngine::setHarmonicFilterDb(float swellDb, float windDb, float tremulantDb) noexcept
{
    auto mixOf = [](float db) { db = juce::jlimit(0.0f, 24.0f, db); return db < 0.01f ? 0.0f : 1.0f - std::pow(10.0f, -db / 20.0f); };
    harmonicSwellDb.store(juce::jlimit(0.0f, 24.0f, swellDb), std::memory_order_relaxed);
    harmonicWindDb.store(juce::jlimit(0.0f, 24.0f, windDb), std::memory_order_relaxed);
    harmonicTremulantDb.store(juce::jlimit(0.0f, 24.0f, tremulantDb), std::memory_order_relaxed);
    harmonicSwellMix.store(mixOf(swellDb), std::memory_order_relaxed);
    harmonicWindMix.store(mixOf(windDb), std::memory_order_relaxed);
    harmonicTremulantMix.store(mixOf(tremulantDb), std::memory_order_relaxed);
}

void SamplerEngine::setManualExpressionMaxAttenuationDb(int manualIndex, float db)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    impl->expressionMaxDb[(size_t)manualIndex].store(juce::jlimit(0.0f, 120.0f, db), std::memory_order_relaxed);
}

void SamplerEngine::setManualDivisionGain(int manualIndex, float gain)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    impl->divisionGain[(size_t)manualIndex].store(juce::jmax(0.0f, gain), std::memory_order_relaxed);
}

void SamplerEngine::setReleaseScalingDb(float db)
{
    releaseScalingGain.store(db < -20.5f ? 0.0f : juce::Decibels::decibelsToGain(juce::jlimit(-20.0f, 10.0f, db)));
}

void SamplerEngine::setGlobalPitchCents(float cents)
{
    const juce::ScopedLock sl(controlLock);
    impl->globalPitchCents = juce::jlimit(-200.0f, 200.0f, cents);
    for (int pc = 0; pc < 12; ++pc)
    {
        impl->pitchClassCents[(size_t)pc].store(impl->globalPitchCents + impl->temperamentOffsets[(size_t)pc]);
        impl->selectedPitchCents[(size_t)pc].store(impl->globalPitchCents + impl->temperamentSelected[(size_t)pc]);
    }
}

void SamplerEngine::setTemperamentOffsets(const std::array<float, 12>& centsByPitchClass)
{
    const juce::ScopedLock sl(controlLock);
    impl->temperamentOffsets = centsByPitchClass;
    for (int pc = 0; pc < 12; ++pc)
        impl->pitchClassCents[(size_t)pc].store(impl->globalPitchCents + impl->temperamentOffsets[(size_t)pc]);
}

void SamplerEngine::setTemperaments(const std::array<float, 12>& selectedCents, const std::array<float, 12>& baseCents)
{
    const juce::ScopedLock sl(controlLock);
    // Tables are referenced to A (a1 stays at the global pitch), like the
    // measured temperaments in the render path.
    const float selectedA = selectedCents[9], baseA = baseCents[9];
    for (int pc = 0; pc < 12; ++pc)
    {
        impl->temperamentSelected[(size_t)pc] = selectedCents[(size_t)pc] - selectedA;
        impl->temperamentOffsets[(size_t)pc] = (selectedCents[(size_t)pc] - selectedA) - (baseCents[(size_t)pc] - baseA);
        impl->pitchClassCents[(size_t)pc].store(impl->globalPitchCents + impl->temperamentOffsets[(size_t)pc]);
        impl->selectedPitchCents[(size_t)pc].store(impl->globalPitchCents + impl->temperamentSelected[(size_t)pc]);
    }
}

void SamplerEngine::setAllStopsTuneTo440(bool enabled)
{
    tuneAllTo440 = enabled;
    for (auto* stop : stops) setStopTuneTo440(stop, enabled);
}

void SamplerEngine::setManualRandomTuningAllowed(int manualIndex, bool allowed)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    impl->randomAllowed[(size_t)manualIndex].store(allowed);
}

void SamplerEngine::setManualPipeCouplingAllowed(int manualIndex, bool allowed)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    impl->couplingAllowed[(size_t)manualIndex].store(allowed);
}

namespace
{
    void regenerateRandomTuning(Stop* stop, juce::Random& random)
    {
        if (stop == nullptr) return;
        const float amount = juce::jlimit(0.0f, 40.0f, stop->randomPitchErrorCents);
        for (int note = 0; note < 128; ++note)
        {
            if (!stop->hasNoteInAnyRank(note) || amount <= 0.0f) { stop->randomPitchByNote[(size_t)note] = 0.0f; continue; }
            const float magnitude = amount * std::pow(random.nextFloat(), 0.8f);
            stop->randomPitchByNote[(size_t)note] = random.nextBool() ? magnitude : -magnitude;
        }
    }
}

void SamplerEngine::setStopRandomPitchError(Stop* stop, float cents, bool regenerate)
{
    if (stop == nullptr) return;
    const juce::ScopedLock sl(controlLock);
    stop->randomPitchErrorCents = juce::jlimit(0.0f, 40.0f, cents);
    if (regenerate) regenerateRandomTuning(stop, impl->tuningRandom);
    for (const uint32_t idx : impl->controlActive)
    {
        Voice& v = impl->voices[idx];
        if (v.stop == stop && !v.detachedRelease)
            v.randomPitchCents.store(stop->randomPitchByNote[(size_t)v.note], std::memory_order_relaxed);
    }
}

//------------------------------------------------------------------------------
namespace
{
    uint64_t channelsToMask(const std::vector<int>& channels)
    {
        uint64_t mask = 0;
        for (const int channel : channels)
            if (channel >= 0 && channel < 64) mask |= 1ull << channel;
        // No channel ticked is none: the route is silent (it used to fall back to
        // channels 1+2, so a division with every channel unticked went on playing).
        return mask;
    }
}

void SamplerEngine::setManualOutputChannels(int manualIndex, std::vector<int> channels)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    const uint64_t mask = channelsToMask(channels);
    const juce::ScopedLock sl(controlLock);
    impl->manualDefaultMask[(size_t)manualIndex].store(mask);
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        Route& r = *impl->routes[(size_t)i];
        if (r.manual == manualIndex && !r.maskExplicit.load()) r.channelMask.store(mask);
    }
}

void SamplerEngine::setPerspectiveOutputChannels(int manualIndex, const juce::String& perspective,
    std::vector<int> channels)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    const uint64_t mask = channelsToMask(channels);
    Route* changed = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        if (Route* r = impl->ensureRoute(manualIndex, perspective))
        {
            const bool pairsChanged = juce::countNumberOfBits((juce::uint64)r->channelMask.load()) / 2 != juce::countNumberOfBits((juce::uint64)mask) / 2;
            r->channelMask.store(mask);
            r->maskExplicit.store(true);
            if (pairsChanged && r->reverb != nullptr) changed = r;
            if (changed == nullptr) changed = r;   // the channels decide the sharing of the IR
        }
    }
    if (changed != nullptr) impl->regroupReverbs(changed, changed->reverb != nullptr || changed->reverbOwner != nullptr);   // one convolution per output pair
}

void SamplerEngine::setWindShares(float oneStop, float onePipe, float twoPipes) noexcept
{
    windOneStopShare.store(juce::jlimit(0.02f, 1.0f, oneStop), std::memory_order_relaxed);
    windOnePipeShare.store(juce::jlimit(0.02f, 1.0f, onePipe), std::memory_order_relaxed);
    windTwoPipesShare.store(juce::jlimit(0.02f, 1.0f, twoPipes), std::memory_order_relaxed);
}

void SamplerEngine::setPerspectiveReverbTone(int manualIndex, const juce::String& perspective, const ReverbTone& tone)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    Route* rebuild = nullptr;
    Route* changed = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        Route* r = impl->ensureRoute(manualIndex, perspective);
        if (r == nullptr) return;
        const bool decayChanged = std::abs(r->reverbTone.decayPercent - tone.decayPercent) > 0.01f
            || std::abs(r->reverbTone.fadePercent - tone.fadePercent) > 0.01f
            || (impl->reverbLight.load(std::memory_order_relaxed) && (r->reverbTone.highCutHz <= 2500.0f) != (tone.highCutHz <= 2500.0f));   // Light's rate divider follows the high cut
        r->reverbTone = tone;
        if (r->reverb != nullptr)
        {
            r->reverb->highCutHz.store(tone.highCutHz, std::memory_order_relaxed);
            r->reverb->lowCutHz.store(tone.lowCutHz, std::memory_order_relaxed);
            r->reverb->preDelayMs.store(tone.preDelayMs, std::memory_order_relaxed);
            r->reverb->wetGain.store(juce::Decibels::decibelsToGain(juce::jlimit(-40.0f, 12.0f, tone.wetDb)), std::memory_order_relaxed);
            if (decayChanged) rebuild = r;   // the IR itself is shortened
        }
        changed = r;
    }
    // The tone is part of what a shared IR must have in common: regroup (the
    // decay change rebuilds the IR itself).
    impl->regroupReverbs(changed, rebuild != nullptr);
}

void SamplerEngine::setPerspectiveBlockSize(int manualIndex, const juce::String& perspective, int blockSize)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || blockSize <= 0) return;
    Route* changed = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        if (Route* r = impl->ensureRoute(manualIndex, perspective))
        {
            const int previous = r->blockSize.exchange(blockSize);
            if (r->reverb != nullptr && (previous != blockSize || r->reverb->preparedBlock != juce::jlimit(32, RouteReverb::kMaxBlock, blockSize)))
                changed = r;
        }
    }
    if (changed != nullptr) impl->rebuildRouteReverb(*changed);
}

void SamplerEngine::setPerspectiveReverb(int manualIndex, const juce::String& perspective,
    const juce::File& impulseResponse, float wet)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    Route* route = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        route = impl->ensureRoute(manualIndex, perspective);
        if (route == nullptr) return;
        route->reverbIrFile = impulseResponse;
        route->reverbWet = juce::jlimit(0.0f, 1.0f, wet);
    }
    impl->regroupReverbs(route, true);
}

void SamplerEngine::setPerspectiveDeviceKey(int manualIndex, const juce::String& perspective, const juce::String& key)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    Route* route = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        route = impl->ensureRoute(manualIndex, perspective);
        if (route == nullptr || route->deviceKey == key) return;
        // The device of this route stops or changes (0.8.146): nothing of what it
        // was playing goes on to the next one - the ring and the active list are
        // finished now, not after the next device's first callbacks (a switch from
        // JACK to ALSA right after a measurement played every release tail and
        // kill fade of the run at once).
        if (route->deviceKey.isNotEmpty())
        {
            bool flushed = false;
            for (int attempt = 0; attempt < 50 && !flushed; ++attempt)
            {
                flushed = impl->flushRouteVoices(*route);
                if (!flushed) juce::Thread::sleep(1);   // a last callback of the old device still renders it
            }
            if (flushed) impl->reclaimFinished();
            CrashHandler::note("Route manual " + juce::String(manualIndex) + " " + perspective + ": device " + (key.isEmpty() ? "stopped" : "changed")
                + (flushed ? ", its voices dropped" : ", its voices NOT dropped (a render held the route)"));
        }
        route->deviceKey = key;
        route->hasDevice.store(key.isNotEmpty(), std::memory_order_release);
    }
    impl->regroupReverbs(route, false);
}

void SamplerEngine::setPerspectiveReverbWet(int manualIndex, const juce::String& perspective, float wet)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    Route* route = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        route = impl->ensureRoute(manualIndex, perspective);
        if (route == nullptr) return;
        route->reverbWet = juce::jlimit(0.0f, 1.0f, wet);
        if (route->reverb != nullptr) route->reverb->wet.store(route->reverbWet, std::memory_order_relaxed);
    }
    impl->regroupReverbs(route, false);   // the wet is part of what a shared IR must have in common
}

SamplerEngine::ReverbInfo SamplerEngine::getPerspectiveReverbInfo(int manualIndex, const juce::String& perspective) const
{
    ReverbInfo info;
    if (manualIndex < 0 || manualIndex >= maxManuals) return info;
    const juce::ScopedLock sl(controlLock);
    if (Route* r = impl->findRoute(manualIndex, perspective.isEmpty() ? juce::String("default") : perspective))
    {
        if (r->reverbOwner != nullptr)
        {
            // Shared IR: the owner convolves for this route - its engines, its error.
            info.sharedWith = "manual " + juce::String(r->reverbOwner->manual) + " perspective " + r->reverbOwner->perspective;
            r = r->reverbOwner;
        }
        info.error = r->reverbError;
        if (r->reverb != nullptr)
        {
            info.channels = r->reverb->channels;
            info.seconds = r->reverb->seconds;
            info.sampleRate = r->reverb->irSampleRate;
            info.light = r->reverb->light;
            info.rateDivider = r->reverb->rateDivider;
            info.usedSeconds = r->reverb->usedSeconds;
            info.costMsPerSecond = r->reverb->costMsPerSecond;
        }
    }
    return info;
}

SamplerEngine::ReverbSummary SamplerEngine::getReverbSummary() const
{
    ReverbSummary summary;
    const juce::ScopedLock sl(controlLock);
    summary.light = impl->reverbLight.load(std::memory_order_relaxed);
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        const Route& r = *impl->routes[(size_t)i];
        if (r.reverbOwner != nullptr) { ++summary.sharedRoutes; continue; }   // convolved by its owner
        if (r.reverb == nullptr || r.reverb->pairs.empty()) continue;
        ++summary.routes;
        const double cost = r.reverb->costMsPerSecond;
        summary.totalCostMsPerSecond += cost;
        summary.worstCostMsPerSecond = juce::jmax(summary.worstCostMsPerSecond, cost);
        // What the same reverb would cost at full quality: the rate divider, the
        // trimmed share of the IR and the dropped cross terms multiply back.
        double factor = r.reverb->rateDivider;
        if (r.reverb->usedSeconds > 0.0 && r.reverb->seconds > r.reverb->usedSeconds) factor *= r.reverb->seconds / r.reverb->usedSeconds;
        if (r.reverb->crossTermsDropped) factor *= 2.0;
        summary.estimatedFullMsPerSecond += cost * factor;
        summary.irSecondsPerSecond += r.reverb->usedSeconds * (double)r.reverb->pairs.size() * (r.reverb->trueStereo ? 4.0 : 2.0);
    }
    return summary;
}

void SamplerEngine::setReverbProcessingLight(bool light)
{
    std::vector<Route*> rebuild;
    {
        const juce::ScopedLock sl(controlLock);
        if (impl->reverbLight.exchange(light) == light) return;
        const int count = impl->routeCount.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            Route& r = *impl->routes[(size_t)i];
            if (r.reverbIrFile != juce::File() || r.reverb != nullptr) rebuild.push_back(&r);
        }
    }
    for (auto* r : rebuild) impl->rebuildRouteReverb(*r);
    impl->regroupReverbs(nullptr, false);
}

bool SamplerEngine::isReverbProcessingLight() const noexcept
{
    return impl->reverbLight.load(std::memory_order_relaxed);
}

bool SamplerEngine::takeReverbOverloadFlag() noexcept
{
    return impl->reverbOverload.exchange(false, std::memory_order_relaxed);
}

void SamplerEngine::setManualSampleRate(int manualIndex, double newSampleRate)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || newSampleRate <= 0.0) return;
    const juce::ScopedLock sl(controlLock);
    impl->manualSampleRate[(size_t)manualIndex].store(newSampleRate);
    const int count = impl->routeCount.load(std::memory_order_acquire);
    for (int i = 0; i < count; ++i)
    {
        Route& r = *impl->routes[(size_t)i];
        if (r.manual == manualIndex && !r.rateExplicit.load()) r.sampleRate.store(newSampleRate);
    }
}

void SamplerEngine::setPerspectiveSampleRate(int manualIndex, const juce::String& perspective, double newSampleRate)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || newSampleRate <= 0.0) return;
    Route* changed = nullptr;
    {
        const juce::ScopedLock sl(controlLock);
        if (Route* r = impl->ensureRoute(manualIndex, perspective))
        {
            const bool rateChanged = std::abs(r->sampleRate.load() - newSampleRate) > 0.5;
            r->sampleRate.store(newSampleRate);
            r->rateExplicit.store(true);
            // The reverb's convolution was prepared for the old rate: rebuild it.
            if (r->reverb != nullptr && (rateChanged || std::abs(r->reverb->preparedRate - newSampleRate) > 0.5)) changed = r;
        }
    }
    if (changed != nullptr) impl->rebuildRouteReverb(*changed);
}

juce::uint64 SamplerEngine::perspectiveChannelMask(int manualIndex, const juce::String& perspective) const noexcept
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return 0;
    const Route* r = impl->findRoute(manualIndex, perspective.isEmpty() ? juce::String("default") : perspective);
    return r != nullptr ? (juce::uint64)r->channelMask.load(std::memory_order_relaxed) : 0;
}

void SamplerEngine::setPerspectiveVolumeDb(int manualIndex, const juce::String& perspective, float db)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    const juce::ScopedLock sl(controlLock);
    if (Route* r = impl->ensureRoute(manualIndex, perspective))
        r->volumeGain.store(db < -30.5f ? 0.0f : juce::Decibels::decibelsToGain(juce::jlimit(-30.0f, 6.0f, db)));
}

void SamplerEngine::setPerspectiveChannelSeparation(int manualIndex, const juce::String& perspective, float separation)
{
    if (manualIndex < 0 || manualIndex >= maxManuals || perspective.isEmpty()) return;
    const juce::ScopedLock sl(controlLock);
    if (Route* r = impl->ensureRoute(manualIndex, perspective))
        r->separation.store(juce::jlimit(0.0f, 1.0f, separation), std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
void SamplerEngine::setWindModel(int sourceManual, float depthCents, float frequencyHz,
    float oscillations, float oscillationStrength, float baseSmoothingMs, uint8_t targetMask)
{
    if (sourceManual < 0 || sourceManual >= maxManuals) return;
    const juce::ScopedLock sl(controlLock);
    auto& state = impl->windSources[(size_t)sourceManual];
    state.depthCents = juce::jlimit(0.0f, 50.0f, depthCents);
    state.frequencyHz = juce::jlimit(2.0f, 10.0f, frequencyHz);
    state.oscillations = juce::jlimit(0.0f, 20.0f, oscillations);
    // 50 % on the GUI slider arrives as 1.0; 100 % is a genuine 2.0 (no clamp at 50 %).
    state.oscillationStrength = juce::jlimit(0.0f, 2.0f, oscillationStrength);
    state.baseSmoothingMs = juce::jlimit(100.0f, 400.0f, baseSmoothingMs);
    state.targetMask = (uint8_t)(targetMask | (1u << sourceManual));
    impl->windLagMs[(size_t)sourceManual].store(state.baseSmoothingMs, std::memory_order_relaxed);
    impl->publishWind(sourceManual);
}

void SamplerEngine::setTransmissionDb(Stop* stop, int index, float db)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    const juce::ScopedLock sl(controlLock);
    stop->transmissions[(size_t)index].db = juce::jlimit(-10.0f, 10.0f, db);
    // Sounding pipes follow at once (glide in the render).
    for (int pipe = 0; pipe < 128; ++pipe)
    {
        bool speaking = stop->speaking.test((size_t)pipe);
        for (const auto& rank : stop->ranks) speaking = speaking || rank->speaking.test((size_t)pipe);
        if (speaking) impl->setPipeRouteGain(stop, pipe, impl->pipeRouteGain(stop, pipe));
    }
}

void SamplerEngine::setStopNoteTone(Stop* stop, int note, float db)
{
    if (stop == nullptr || note < 0 || note >= 128) return;
    const juce::ScopedLock sl(controlLock);
    stop->setNoteToneDb(note, db);
    bool speaking = stop->speaking.test((size_t)note);
    for (const auto& rank : stop->ranks) speaking = speaking || rank->speaking.test((size_t)note);
    if (speaking) impl->setPipeRouteGain(stop, note, impl->pipeRouteGain(stop, note));   // sets the tone as well
}

void SamplerEngine::setTransmissionKeyTone(Stop* stop, int index, int key, float db)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size() || key < 0 || key >= 128) return;
    const juce::ScopedLock sl(controlLock);
    auto& transmission = stop->transmissions[(size_t)index];
    transmission.keyTone[(size_t)key] = juce::jlimit(-9.0f, 9.0f, db);
    const int pipe = key + transmission.shift;
    if (pipe < 0 || pipe >= 128) return;
    bool speaking = stop->speaking.test((size_t)pipe);
    for (const auto& rank : stop->ranks) speaking = speaking || rank->speaking.test((size_t)pipe);
    if (speaking) impl->setPipeRouteGain(stop, pipe, impl->pipeRouteGain(stop, pipe));
}

float SamplerEngine::toneStartHzFor(const Stop* rank, int note, float globalStartHz) noexcept
{
    if (rank == nullptr || note < 0 || note >= 128) return globalStartHz;
    for (const auto& sample : rank->samplesByNote[(size_t)note])
    {
        if (sample == nullptr || std::isnan(sample->smplPitch)) continue;
        // The recorded fundamental (MIDI semitones, a1 = 69 = 440 Hz).
        const float f0 = 440.0f * std::pow(2.0f, (sample->smplPitch - 69.0f) / 12.0f);
        return f0 > globalStartHz ? f0 : globalStartHz;
    }
    return globalStartHz;
}

void SamplerEngine::toneShelfCoefficients(double sampleRate, float db, float startHz, float k[5]) noexcept
{
    // RBJ high shelf, slope 1, centred half an octave above the start frequency:
    // the transition runs from about the start to twice it. 'db' is the gain of
    // the band above; below the start nothing changes.
    const double rate = juce::jmax(8000.0, sampleRate);
    const double fc = juce::jlimit(20.0, rate * 0.45, (double)startHz * 1.41421356);
    const double A = std::pow(10.0, (double)db / 40.0);
    const double w0 = juce::MathConstants<double>::twoPi * fc / rate;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = sw * 0.5 * std::sqrt(2.0);   // S = 1
    const double twoSqrtAalpha = 2.0 * std::sqrt(A) * alpha;
    const double b0 = A * ((A + 1.0) + (A - 1.0) * cw + twoSqrtAalpha);
    const double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw);
    const double b2 = A * ((A + 1.0) + (A - 1.0) * cw - twoSqrtAalpha);
    const double a0 = (A + 1.0) - (A - 1.0) * cw + twoSqrtAalpha;
    const double a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cw);
    const double a2 = (A + 1.0) - (A - 1.0) * cw - twoSqrtAalpha;
    k[0] = (float)(b0 / a0); k[1] = (float)(b1 / a0); k[2] = (float)(b2 / a0);
    k[3] = (float)(a1 / a0); k[4] = (float)(a2 / a0);
}

void SamplerEngine::applyToneShelf(juce::AudioBuffer<float>& buffer, double sampleRate, float db, float startHz)
{
    if (std::abs(db) < 0.005f) return;
    float k[5];
    toneShelfCoefficients(sampleRate, db, startHz, k);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float state[2]{ 0.0f, 0.0f };
        applyBiquadInPlace(buffer.getWritePointer(channel), buffer.getNumSamples(), k, state);
    }
}

void SamplerEngine::setTransmissionKeyDb(Stop* stop, int index, int key, float db)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size() || key < 0 || key >= 128) return;
    const juce::ScopedLock sl(controlLock);
    auto& transmission = stop->transmissions[(size_t)index];
    transmission.keyDb[(size_t)key] = juce::jlimit(-24.0f, 24.0f, db);
    const int pipe = key + transmission.shift;
    if (pipe < 0 || pipe >= 128) return;
    bool speaking = stop->speaking.test((size_t)pipe);
    for (const auto& rank : stop->ranks) speaking = speaking || rank->speaking.test((size_t)pipe);
    if (speaking) impl->setPipeRouteGain(stop, pipe, impl->pipeRouteGain(stop, pipe));
}

void SamplerEngine::setFillGroupSize(int pipes)
{
    pipes = juce::jlimit(1, 12, pipes);
    fillGroupSize.store(pipes, std::memory_order_relaxed);
    {
        const juce::ScopedLock sl(controlLock);
        refillAllStops();
    }
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

void SamplerEngine::setRetuneLimitCents(float cents)
{
    cents = juce::jlimit(10.0f, 1200.0f, cents);
    if (std::abs(retuneLimitCents.exchange(cents, std::memory_order_acq_rel) - cents) < 0.01f) return;
    gRetuneLimitCents.store(cents, std::memory_order_relaxed);
    {
        const juce::ScopedLock sl(controlLock);
        // Every rank's faulty pipes and clamped corrections from the raw deviations, then the fills afresh.
        auto apply = [cents](Stop* rank)
        {
            rank->faultyPipes.reset();
            for (int note = 0; note < 128; ++note)
            {
                if (!rank->hasNote(note) || !rank->tuningInfoAvailable) continue;
                const float raw = rank->rawTuningCents[(size_t)note];
                if (raw == 0.0f && rank->tuning440Cents[(size_t)note] != 0.0f) continue;   // a note without pitch data keeps the diapason shift
                if (std::abs(raw) > cents) rank->faultyPipes.set((size_t)note);
                rank->tuning440Cents[(size_t)note] = juce::jlimit(-cents, cents, raw);
            }
        };
        for (auto* stop : stops) { apply(stop); for (auto& rank : stop->ranks) apply(rank.get()); }
        refillAllStops();
    }
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

void SamplerEngine::refillAllStops()
{
    for (auto* stop : stops) refillStopPipes(stop, false);   // the group size changed: every fill afresh
}

void SamplerEngine::setTransposeQuality(int quality)
{
    if (transposeQuality.exchange(juce::jlimit(0, 2, quality)) == juce::jlimit(0, 2, quality)) return;
    quality = juce::jlimit(0, 2, quality);
    // Copies are only needed at quality 2; existing ones stay (voices may read
    // them) and are simply not chosen at lower quality.
    if (quality >= 2) for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

namespace
{
    // Low-pass copy of a sample for playback 'ratio' times faster: everything above
    // Nyquist / ratio would fold back, so a windowed-sinc FIR (Blackman, zero
    // phase) removes it before the pitch shift. Loop points and smpl pitch are kept.
    std::unique_ptr<Sample> makeAntiAliasCopy(const Sample& source, double ratio)
    {
        auto copy = std::make_unique<Sample>(source.numChannels, source.numFrames, source.sampleRate, source.storesFloat);
        copy->smplPitch = source.smplPitch;
        copy->sincHead = source.sincHead;
        const int frames = (int)source.numFrames;
        if (frames <= 0) { copy->finaliseLoops(source.loops); return copy; }
        // Cutoff a little below the folding limit: the transition band must end there.
        const double cutoff = juce::jlimit(0.02, 0.49, 0.5 / juce::jmax(1.0, ratio) * 0.86);   // in cycles per sample
        // The kernel grows with the ratio beyond two octaves (0.8.161): 191 taps of a
        // Blackman window have a transition band of ~0.029 cycles per sample - at a
        // ratio of 16 (four octaves up) the cutoff is 0.027 and the stopband began at
        // 0.041, above the folding limit of 0.031, so the band between aliased.
        const int taps = ratio <= 4.0 ? 191 : (int)std::llround(191.0 * ratio / 4.0) | 1, half = taps / 2;
        std::vector<float> h((size_t)taps, 0.0f);
        double sum = 0.0;
        for (int i = 0; i < taps; ++i)
        {
            const double m = (double)(i - half);
            const double sinc = m == 0.0 ? 2.0 * cutoff : std::sin(2.0 * juce::MathConstants<double>::pi * cutoff * m) / (juce::MathConstants<double>::pi * m);
            const double w = 0.42 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * i / (taps - 1))
                + 0.08 * std::cos(4.0 * juce::MathConstants<double>::pi * i / (taps - 1));   // Blackman
            h[(size_t)i] = (float)(sinc * w);
            sum += sinc * w;
        }
        for (auto& c : h) c = (float)(c / sum);   // unity gain at DC
        std::vector<float> in((size_t)frames + 2 * half, 0.0f);
        for (int ch = 0; ch < source.numChannels; ++ch)
        {
            // Padded with the edge values so the start of the attack is not smeared into silence.
            if (source.storesFloat)
            {
                std::vector<float> scratch;
                const float* s = source.read32(ch, scratch);   // a packed source decoded whole (0.8.183)
                for (int i = 0; i < frames; ++i) in[(size_t)(i + half)] = s[i];
            }
            else
            {
                std::vector<int16_t> scratch;
                const int16_t* s = source.read16(ch, scratch);   // a packed source decoded whole
                for (int i = 0; i < frames; ++i) in[(size_t)(i + half)] = (float)s[i];
            }
            for (int i = 0; i < half; ++i) { in[(size_t)i] = in[(size_t)half]; in[(size_t)(frames + half + i)] = in[(size_t)(frames + half - 1)]; }
            float* out32 = source.storesFloat ? copy->writable32(ch) : nullptr;
            int16_t* out16 = source.storesFloat ? nullptr : copy->writable16(ch);
            for (int i = 0; i < frames; ++i)
            {
                const float* x = in.data() + i;   // x[k] = in[i + k], k = 0..taps-1 centres on frame i
                float acc = 0.0f;
                for (int k = 0; k < taps; ++k) acc += x[k] * h[(size_t)k];
                if (out32 != nullptr) out32[i] = acc;
                else out16[i] = (int16_t)juce::jlimit(-32768.0f, 32767.0f, std::round(acc));
            }
        }
        copy->finaliseLoops(source.loops);
        return copy;
    }

    // Runs fn(index, thread) for index 0..count-1 on up to 16 threads (the loader
    // decodes files with it, the copies of filled pipes are shifted with it).
    template <typename Fn>
    void parallelFor(int count, Fn&& fn, int maxThreads = 16)
    {
        if (count <= 0) return;
        const int threads = juce::jlimit(1, juce::jmax(1, maxThreads), juce::jmin(count, juce::SystemStats::getNumCpus()));
        if (threads == 1)
        {
            for (int i = 0; i < count; ++i) fn(i, 0);
            return;
        }
        std::atomic<int> next{ 0 };
        std::vector<std::thread> workers;
        for (int t = 0; t < threads; ++t)
            workers.emplace_back([&, t]
            {
                lowerCurrentThreadPriority();
                // An exception on a worker (out of memory, a broken file) must not end
                // the program: the item is skipped, the others go on.
                for (int i = next.fetch_add(1); i < count; i = next.fetch_add(1))
                {
                    try { fn(i, t); }
                    catch (...) {}
                }
            });
        for (auto& w : workers) w.join();
    }

    // A transposed copy of a sample at the same length: Signalsmith Stretch (MIT)
    // shifts the pitch by 'semitones' spectrally, the duration, the loop points and
    // the release lengths stay as recorded. No tonality limit and no formant
    // correction: a pipe transposed by an octave is a pipe an octave away, its
    // whole spectrum moves. Offline, so the largest block the library offers.
    // lengthFactor < 1 shortens the copy at the same time (releases of pipes shifted
    // upwards: a small pipe's room decay is shorter); loops are only kept at factor 1.
    // A copy of 'source' shifted by 'semitones' with its length times lengthFactor
    // (Signalsmith Stretch). The harmonics move to the new pitch; the source's
    // spectral envelope (the formants, the body of the sound) is kept in place and
    // moved ALONG with the shift by formantFollow (0..1) of it: a pipe shifted -24
    // with 0.5 gets its envelope 12 semitones lower; 0 keeps the envelope exactly
    // where the recording had it (the user found a counter-shift far too sharp,
    // and the full move - plain transposition - veiled). keepLoudness: the copy
    // is scaled to the source's RMS. baseHz: the source's fundamental, the anchor
    // of the formant analysis (0 = detected).
    // The loop points of a pitch-shifted copy. The source's loops span whole
    // periods of the source pitch; the copy has another period, so the same
    // frames no longer close a cycle and every pass of the loop jumps in phase.
    // Each loop keeps its start; its end moves to the nearest whole number of the
    // copy's periods and is then refined within half a period to the position
    // where the frames before it match the frames before the start best (that is
    // what the splice blends), on every channel.
    std::vector<LoopPoints> refineLoopsForPeriod(const std::vector<std::vector<float>>& out, int frames,
        const std::vector<LoopPoints>& loops, double period)
    {
        if (period < 2.0 || out.empty() || frames < 16) return loops;
        std::vector<LoopPoints> result;
        for (auto loop : loops)
        {
            const int64_t length = loop.end - loop.start;
            if (length < 4 || loop.start < 2) { result.push_back(loop); continue; }
            const double cycles = juce::jmax(1.0, std::round((double)length / period));
            const int64_t nominal = loop.start + (int64_t)std::llround(cycles * period);
            const int64_t radius = juce::jmax<int64_t>(1, (int64_t)std::llround(period / 2.0));
            const int64_t window = juce::jmax<int64_t>(8, juce::jmin<int64_t>(loop.start, (int64_t)std::llround(period * 8.0), (int64_t)4096));
            int64_t best = -1;
            double bestScore = -2.0;
            for (int64_t end = nominal - radius; end <= nominal + radius; ++end)
            {
                if (end < loop.start + 2 || end > (int64_t)frames - 3 || end - window < 0) continue;
                double dot = 0.0, energyA = 0.0, energyB = 0.0;
                for (const auto& channel : out)
                {
                    const float* a = channel.data() + (loop.start - window);
                    const float* b = channel.data() + (end - window);
                    for (int64_t k = 0; k < window; ++k)
                    {
                        dot += (double)a[k] * b[k];
                        energyA += (double)a[k] * a[k];
                        energyB += (double)b[k] * b[k];
                    }
                }
                const double score = energyA > 1.0e-12 && energyB > 1.0e-12 ? dot / std::sqrt(energyA * energyB) : -1.0;
                if (score > bestScore) { bestScore = score; best = end; }
            }
            if (best > 0) loop.end = best;
            result.push_back(loop);
        }
        return result;
    }

    //--------------------------------------------------------------------------
    // The spectral envelope of a sample (linear magnitude per bin of an FFT of
    // 2^order points at the sample's rate): the magnitude spectrum averaged over
    // the sustained part, its harmonic peaks joined by a running maximum over one
    // harmonic spacing (f0) and smoothed. What lies between the harmonics - the
    // body of the sound, the formants - is this envelope.
    std::vector<float> spectralEnvelope(const std::vector<float>& mono, double rate, double f0Hz, int order)
    {
        const int n = 1 << order, bins = n / 2 + 1;
        std::vector<float> acc((size_t)bins, 0.0f);
        const int length = (int)mono.size();
        if (length < 64) return std::vector<float>((size_t)bins, 1.0f);
        juce::dsp::FFT fft(order);
        std::vector<float> buffer((size_t)n * 2, 0.0f);
        // Frames across the sustained part (the attack and the end left out).
        const int first = (int)(length * 0.25), last = juce::jmax(first, (int)(length * 0.75) - n);
        const int frames = juce::jlimit(1, 16, (last - first) / juce::jmax(1, n / 2) + 1);
        const int hop = frames > 1 ? (last - first) / (frames - 1) : 0;
        for (int frame = 0; frame < frames; ++frame)
        {
            const int start = juce::jlimit(0, juce::jmax(0, length - 1), first + frame * hop);
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            for (int i = 0; i < n && start + i < length; ++i)
            {
                const float window = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * (float)i / (float)(n - 1));
                buffer[(size_t)i] = mono[(size_t)(start + i)] * window;
            }
            fft.performFrequencyOnlyForwardTransform(buffer.data());
            for (int b = 0; b < bins; ++b) acc[(size_t)b] += buffer[(size_t)b];
        }
        for (auto& v : acc) v /= (float)frames;
        // Join the peaks: a running maximum over +- 0.6 harmonic spacings.
        const double binHz = rate / n;
        const int radius = juce::jmax(2, (int)std::llround(juce::jmax(20.0, f0Hz) * 0.6 / binHz));
        std::vector<float> env((size_t)bins);
        for (int b = 0; b < bins; ++b)
        {
            float m = 0.0f;
            for (int k = juce::jmax(0, b - radius); k <= juce::jmin(bins - 1, b + radius); ++k) m = juce::jmax(m, acc[(size_t)k]);
            env[(size_t)b] = m;
        }
        // Smoothed in dB (two box passes of the same radius: a triangle).
        std::vector<float> db((size_t)bins);
        for (int b = 0; b < bins; ++b) db[(size_t)b] = 20.0f * std::log10(juce::jmax(1.0e-9f, env[(size_t)b]));
        for (int pass = 0; pass < 2; ++pass)
        {
            std::vector<float> out((size_t)bins);
            for (int b = 0; b < bins; ++b)
            {
                double sum = 0.0; int count = 0;
                for (int k = juce::jmax(0, b - radius); k <= juce::jmin(bins - 1, b + radius); ++k) { sum += db[(size_t)k]; ++count; }
                out[(size_t)b] = (float)(sum / juce::jmax(1, count));
            }
            db.swap(out);
        }
        for (int b = 0; b < bins; ++b) env[(size_t)b] = std::pow(10.0f, db[(size_t)b] / 20.0f);
        return env;
    }

    // One channel resampled by 'ratio' (output frame i reads input position i *
    // ratio): a 32-tap Blackman-windowed sinc. For ratio > 1 the input must be
    // low-passed first (makeAntiAliasCopy), the sinc here interpolates only.
    // The sinc kernels for 1024 fractional positions, normalised, computed once
    // (a pipe's copy is hundreds of thousands of frames: no transcendental
    // functions per frame).
    struct SincTable
    {
        static constexpr int half = 16, taps = 2 * half, phases = 1024;
        std::vector<float> table;   // [phase][tap]
        SincTable() : table((size_t)phases * taps)
        {
            for (int p = 0; p < phases; ++p)
            {
                const double frac = (double)p / phases;
                double weight = 0.0;
                for (int t = 0; t < taps; ++t)
                {
                    const int k = t - half + 1;
                    const double x = (double)k - frac;
                    const double w = 0.42 + 0.5 * std::cos(juce::MathConstants<double>::pi * x / half) + 0.08 * std::cos(2.0 * juce::MathConstants<double>::pi * x / half);
                    const double sinc = std::abs(x) < 1.0e-9 ? 1.0 : std::sin(juce::MathConstants<double>::pi * x) / (juce::MathConstants<double>::pi * x);
                    table[(size_t)p * taps + (size_t)t] = (float)(sinc * w);
                    weight += sinc * w;
                }
                if (weight > 1.0e-6) for (int t = 0; t < taps; ++t) table[(size_t)p * taps + (size_t)t] /= (float)weight;
            }
        }
    };
    const SincTable& sincTable() { static const SincTable table; return table; }

    void resampleChannel(const float* in, int frames, double ratio, float* out, int outFrames)
    {
        const auto& table = sincTable();
        constexpr int half = SincTable::half, taps = SincTable::taps;
        for (int i = 0; i < outFrames; ++i)
        {
            const double position = (double)i * ratio;
            const int centre = (int)std::floor(position);
            const int phase = juce::jlimit(0, SincTable::phases - 1, (int)((position - centre) * SincTable::phases));
            const float* kernel = table.table.data() + (size_t)phase * taps;
            const int first = centre - half + 1;
            float sum = 0.0f;
            if (first >= 0 && first + taps <= frames)
            {
                const float* s = in + first;
                for (int t = 0; t < taps; ++t) sum += kernel[t] * s[t];
            }
            else
                for (int t = 0; t < taps; ++t)
                {
                    const int index = first + t;
                    sum += kernel[t] * (index < 0 ? in[0] : (index >= frames ? in[frames - 1] : in[index]));
                }
            out[i] = sum;
        }
    }

    // A zero-phase FIR with the magnitude response 'response' (per bin of an FFT
    // of 2^order points), applied to a channel by overlap-add FFT convolution.
    void applyStaticEq(float* data, int frames, const std::vector<float>& response, int order)
    {
        const int n = 1 << order;
        const int bins = n / 2 + 1;
        if ((int)response.size() != bins || frames <= 0) return;
        // The impulse response: inverse FFT of the real, even spectrum; the centre
        // at index 0 (wrapped), windowed to 'taps' points.
        juce::dsp::FFT fft(order);
        std::vector<float> spectrum((size_t)n * 2, 0.0f);
        for (int b = 0; b < bins; ++b) spectrum[(size_t)b * 2] = response[(size_t)b];
        fft.performRealOnlyInverseTransform(spectrum.data());
        const int taps = juce::jmin(n - 1, 4095), half = taps / 2;
        std::vector<float> fir((size_t)taps);
        for (int i = 0; i < taps; ++i)
        {
            const int k = i - half;                                  // -half .. half
            const int index = k >= 0 ? k : n + k;
            const float window = 0.5f + 0.5f * std::cos(juce::MathConstants<float>::pi * (float)k / (float)(half + 1));
            fir[(size_t)i] = spectrum[(size_t)index] * window;
        }
        // Overlap-add: blocks of (N - taps + 1) input frames, FFT size N = 2 n.
        const int fftOrder = order + 1, N = 1 << fftOrder, block = N - taps + 1;
        juce::dsp::FFT big(fftOrder);
        std::vector<float> firSpectrum((size_t)N * 2, 0.0f);
        for (int i = 0; i < taps; ++i) firSpectrum[(size_t)i] = fir[(size_t)i];
        big.performRealOnlyForwardTransform(firSpectrum.data(), true);
        std::vector<float> output((size_t)frames + (size_t)taps, 0.0f);
        std::vector<float> work((size_t)N * 2, 0.0f);
        for (int start = 0; start < frames; start += block)
        {
            const int count = juce::jmin(block, frames - start);
            std::fill(work.begin(), work.end(), 0.0f);
            for (int i = 0; i < count; ++i) work[(size_t)i] = data[start + i];
            big.performRealOnlyForwardTransform(work.data(), true);
            for (int b = 0; b <= N / 2; ++b)
            {
                const float ar = work[(size_t)b * 2], ai = work[(size_t)b * 2 + 1];
                const float br = firSpectrum[(size_t)b * 2], bi = firSpectrum[(size_t)b * 2 + 1];
                work[(size_t)b * 2] = ar * br - ai * bi;
                work[(size_t)b * 2 + 1] = ar * bi + ai * br;
            }
            big.performRealOnlyInverseTransform(work.data());
            for (int i = 0; i < count + taps - 1 && start + i < (int)output.size(); ++i) output[(size_t)(start + i)] += work[(size_t)i];
        }
        // Zero phase: the FIR's centre delays by 'half'.
        for (int i = 0; i < frames; ++i) data[i] = output[(size_t)(i + half)];
    }

    // A copy of 'source' shifted by 'semitones' - the method of 0.8.13 - as a
    // pipe would be re-pitched: resampled (every harmonic kept, nothing added),
    // with its length changed by the ratio, then the spectral envelope (the
    // formants) moved back part of the way with one static, zero-phase EQ:
    // formantFollow 0 keeps the source's envelope in place, 1 leaves it where
    // the resampling put it (plain transposition). Loops scale with the ratio
    // (they stay whole periods); a release longer than lengthFactor of the source
    // is faded to that length. keepLoudness: the copy at the source's RMS.
    // baseHz: the source's nominal pitch (the harmonic spacing of the envelope
    // analysis; the smpl pitch when the sample has one).
    // Brings a resampled release to 'target' frames without touching its pitch or
    // cutting it into grains. The resampling scaled the decay rate with the
    // pitch (an octave up: the room decays twice as fast); the decay rate the
    // copy should have is the measured one scaled by natural/target. So:
    // 1. the rate of the decay is measured (log-RMS of 20 ms windows over the
    //    later 70 % of the release, a straight line in dB per second),
    // 2. the whole release gets one smooth exponential gain that turns the
    //    measured rate into the wanted one - a pipe shifted down gets an extra
    //    exponential damping (its decaying harmonics simply fade sooner, like a
    //    release cut), a pipe shifted up gets its decay slowed (at most +24 dB),
    // 3. shifted down: cut at the target with a short fade;
    //    shifted up: beyond the material's end the decay is continued with the
    //    release's own late part - long pieces of it (a quarter second to a
    //    second and a half, forward and reversed by turns, random starts) laid
    //    over each other with equal-power crossfades, flattened to the wanted
    //    decay, at the level the material had at its end. Nothing foreign is
    //    added: the continuation is the room's own sound, same colour, same level.
    void reshapeRelease(std::vector<std::vector<float>>& out, int natural, int target, double sampleRate)
    {
        if (natural <= 0 || target <= 0 || target == natural || out.empty() || sampleRate <= 0.0) return;
        const int channels = (int)out.size();
        const int window = juce::jmax(16, (int)(0.02 * sampleRate));
        auto windowDb = [&](int start)
        {
            double power = 0.0;
            for (int ch = 0; ch < channels; ++ch)
                for (int i = start; i < juce::jmin(natural, start + window); ++i) power += (double)out[(size_t)ch][(size_t)i] * out[(size_t)ch][(size_t)i];
            return 10.0 * std::log10(juce::jmax(1.0e-14, power / (double)(window * channels)));
        };
        // 1. The decay rate (dB/s) over 30-90 % of the release (a recording's own
        //    end fade is left out), as a straight line in dB over time.
        double slopeR = -20.0;   // a typical church when nothing can be measured
        double meanT = (double)natural * 0.6 / sampleRate, meanDb = windowDb((int)(0.6 * natural));
        {
            std::vector<double> t, db;
            for (int start = (int)(0.3 * natural); start + window <= (int)(0.9 * natural); start += window)
            {
                const double level = windowDb(start);
                if (level < -80.0) continue;   // the recording's silence, not the decay
                t.push_back((double)(start + window / 2) / sampleRate);
                db.push_back(level);
            }
            if (t.size() >= 4)
            {
                double mt = 0.0, md = 0.0;
                for (size_t i = 0; i < t.size(); ++i) { mt += t[i]; md += db[i]; }
                mt /= (double)t.size(); md /= (double)t.size();
                double num = 0.0, den = 0.0;
                for (size_t i = 0; i < t.size(); ++i) { num += (t[i] - mt) * (db[i] - md); den += (t[i] - mt) * (t[i] - mt); }
                if (den > 0.0) { slopeR = juce::jlimit(-300.0, -1.0, num / den); meanT = mt; meanDb = md; }
            }
        }
        auto fittedDb = [&](int frame) { return meanDb + slopeR * ((double)frame / sampleRate - meanT); };   // before the gain
        // Where the material stops following its own decay (the recording's end
        // fade, or silence): nothing after that is used.
        int usable = natural;
        for (int start = natural - window; start >= (int)(0.5 * natural); start -= window)
        {
            const double level = windowDb(start);
            if (level > -80.0 && level > fittedDb(start + window / 2) - 8.0) { usable = start + window; break; }
            usable = start;
        }
        // The material's own envelope (dB per window, smoothed over three windows)
        // up to 'usable', the fitted line beyond it. The copy's envelope is this
        // one stretched in time by target/natural - whatever its shape (the pipe's
        // own tail first, the room after it), not a straight line.
        const int windows = juce::jmax(1, usable / window);
        std::vector<double> envelope((size_t)windows);
        for (int w = 0; w < windows; ++w) envelope[(size_t)w] = windowDb(w * window);
        {
            std::vector<double> smooth(envelope);
            for (int w = 0; w < windows; ++w)
            {
                double sum = 0.0; int n = 0;
                for (int k = juce::jmax(0, w - 1); k <= juce::jmin(windows - 1, w + 1); ++k) { sum += envelope[(size_t)k]; ++n; }
                smooth[(size_t)w] = sum / n;
            }
            envelope = std::move(smooth);
        }
        auto materialDb = [&](double frame)   // the material's envelope at a frame of the material (before the gain)
        {
            const double position = frame / window - 0.5;
            if (position <= 0.0) return envelope.front();
            if (position >= windows - 1)
            {
                const double beyond = (frame - (double)(windows - 1 + 0.5) * window) / sampleRate;   // seconds past the last window
                return envelope.back() + slopeR * juce::jmax(0.0, beyond);
            }
            const int w = (int)position;
            const double frac = position - w;
            return envelope[(size_t)w] + (envelope[(size_t)w + 1] - envelope[(size_t)w]) * frac;
        };
        const double stretch = (double)natural / (double)target;
        auto wantedDb = [&](int frame) { return materialDb((double)frame * stretch); };   // the copy's envelope
        // 2. The gain that turns the material's envelope into the wanted one
        //    (upwards at most +24 dB: the recording's noise must not come up with it).
        constexpr double cap = 24.0;
        int capped = natural;   // the first frame where the cap is reached (the material cannot follow beyond it)
        for (int i = 0; i < natural; i += window)
            if (wantedDb(i) - materialDb((double)i) > cap) { capped = i; break; }
        for (int ch = 0; ch < channels; ++ch)
            for (int i = 0; i < natural; ++i)
            {
                const double gain = juce::jmin(cap, wantedDb(i) - materialDb((double)i));
                out[(size_t)ch][(size_t)i] *= (float)std::pow(10.0, gain / 20.0);
            }
        if (target < natural)
        {
            // 3a. Cut at the target with a 20 ms fade.
            for (auto& channel : out)
            {
                channel.resize((size_t)target);
                const int fade = juce::jmin(window, target);
                for (int i = 0; i < fade; ++i) channel[(size_t)(target - fade + i)] *= (float)(fade - 1 - i) / (float)juce::jmax(1, fade - 1);
            }
            return;
        }
        // 3b. The continuation. It takes over at the join: where the material ends
        //     (its end fade left out) or where the gain reaches its cap, whichever
        //     comes first; from there the wanted envelope is followed exactly, made
        //     of long pieces of the material before the join, each flattened by the
        //     envelope it had and given the envelope wanted where it lands.
        for (auto& channel : out) channel.resize((size_t)target, 0.0f);
        const int join = juce::jmin(usable, capped);
        const int regionStart = (int)(0.3 * natural);
        int piece = juce::jlimit((int)(0.25 * sampleRate), (int)(1.5 * sampleRate), (int)(0.35 * natural));
        piece = juce::jmin(piece, join - regionStart);
        if (piece < 2 * window || join <= regionStart + window)
        {
            // Too short a release to continue: it ends where its material ends.
            for (auto& channel : out) std::fill(channel.begin() + usable, channel.end(), 0.0f);
            return;
        }
        const int half = piece / 2;
        juce::Random random((int64_t)natural * 7919 + target);
        // The pieces come from the material as it is now (after the gain), before
        // the fade-out and the pieces themselves change it.
        std::vector<std::vector<float>> material((size_t)channels);
        for (int ch = 0; ch < channels; ++ch)
            material[(size_t)ch].assign(out[(size_t)ch].begin(), out[(size_t)ch].begin() + join);
        for (int ch = 0; ch < channels; ++ch)
        {
            // The material after the join goes; before it, the last half piece fades out
            // as the first piece fades in (equal power).
            std::fill(out[(size_t)ch].begin() + join, out[(size_t)ch].end(), 0.0f);
            for (int j = 0; j < half && join - half + j >= 0; ++j)
                out[(size_t)ch][(size_t)(join - half + j)] *= (float)std::cos(juce::MathConstants<double>::pi * 0.5 * j / half);
        }
        std::vector<float> flat((size_t)piece);
        for (int k = 0, position = join - half; position < target; ++k, position += half)
        {
            const int start = regionStart + random.nextInt(juce::jmax(1, join - regionStart - piece + 1));
            const bool reversed = (k % 2) == 1;
            for (int ch = 0; ch < channels; ++ch)
            {
                // The piece flattened to 0 dB by the envelope it has (after the gain: the wanted one).
                double piecePower = 0.0;
                for (int i = 0; i < piece; ++i)
                {
                    const int s = reversed ? start + piece - 1 - i : start + i;
                    flat[(size_t)i] = material[(size_t)ch][(size_t)s] * (float)std::pow(10.0, -wantedDb(s) / 20.0);
                    piecePower += (double)flat[(size_t)i] * flat[(size_t)i];
                }
                const double pieceRms = std::sqrt(piecePower / (double)piece);
                const float level = (float)(pieceRms > 1.0e-9 ? juce::jlimit(0.03, 30.0, 1.0 / pieceRms) : 1.0);
                for (int i = 0; i < piece; ++i)
                {
                    const int o = position + i;
                    if (o < join - half || o >= target) continue;
                    const float w = (float)std::sin(juce::MathConstants<double>::pi * i / piece);   // sin^2 + cos^2 = 1 at half overlap
                    out[(size_t)ch][(size_t)o] += w * level * flat[(size_t)i] * (float)std::pow(10.0, wantedDb(o) / 20.0);
                }
            }
        }
        for (auto& channel : out)
        {
            const int fade = juce::jmin(window, target);
            for (int i = 0; i < fade; ++i) channel[(size_t)(target - fade + i)] *= (float)(fade - 1 - i) / (float)juce::jmax(1, fade - 1);
        }
    }

    // The "formants follow" share a copy is built with, from the stop's setting
    // and the shift (the user's rules): the setting as such for shifts downwards
    // up to an octave, 30 points more upwards; beyond an octave every further
    // octave in either direction adds 'perOctave' (15 points for the resampling
    // methods, 25 for the phase vocoder): the farther the shift, the more the
    // envelope goes with it - a copy two octaves away is a pipe of another size.
    float formantFollowForShift(float setting, int semitones, float perOctave)
    {
        float share = juce::jlimit(0.0f, 1.0f, setting);
        if (semitones > 0) share += 0.3f;
        const float beyondOctave = juce::jmax(0.0f, ((float)std::abs(semitones) - 12.0f) / 12.0f);
        share += perOctave * beyondOctave;
        return juce::jlimit(0.0f, 1.0f, share);
    }

    // A release copy loses its first 'trimMs' (at most half of it): the worst of
    // every method sits at a release's start. The cut begins with a 2 ms ramp so
    // that nothing clicks. Returns the new frame count.
    [[maybe_unused]] int trimReleaseStart(std::vector<std::vector<float>>& out, int frames, double sampleRate, double trimMs)
    {
        const int cut = juce::jlimit(0, frames / 2, (int)std::llround(trimMs * 0.001 * sampleRate));
        if (cut <= 0 || frames <= 0) return frames;
        const int remaining = frames - cut;
        const int ramp = juce::jmin(remaining, juce::jmax(1, (int)(0.002 * sampleRate)));
        for (auto& channel : out)
        {
            channel.erase(channel.begin(), channel.begin() + cut);
            for (int i = 0; i < ramp; ++i) channel[(size_t)i] *= (float)i / (float)ramp;
        }
        return remaining;
    }

    // The static formant correction shared by the pitch-shift methods: the copy's
    // content ('out') carries the source's spectral envelope E(f) moved to
    // E(f / movedBy) - the resampling moves it by the pitch ratio, a phase
    // vocoder with pitch compensation not at all (movedBy 1) - and it should sit
    // at E(f / wantedShift). One zero-phase EQ for all channels, from the
    // envelope of the mono sum of the source ('in', the source's frames); below
    // the source's fundamental nothing is corrected (the bass keeps its level),
    // the rule fades in over the octave above f0.
    void moveSpectralEnvelope(std::vector<std::vector<float>>& out, int outFrames, const std::vector<std::vector<float>>& in, int frames,
        double sampleRate, double f0, double movedBy, double wantedShift)
    {
        const int channels = (int)out.size();
        if (channels == 0 || frames < 2048 || std::abs(movedBy - wantedShift) < 1.0e-6) return;
        constexpr int order = 13;
        const int n = 1 << order, bins = n / 2 + 1;
        std::vector<float> mono((size_t)frames, 0.0f);
        for (int ch = 0; ch < channels; ++ch) for (int i = 0; i < frames; ++i) mono[(size_t)i] += in[(size_t)ch][(size_t)i] / (float)channels;
        const auto envelope = spectralEnvelope(mono, sampleRate, juce::jmax(20.0, f0), order);
        auto envelopeAt = [&](double bin)
        {
            bin = juce::jlimit(0.0, (double)(bins - 1), bin);
            const int b0 = (int)bin;
            const int b1 = juce::jmin(bins - 1, b0 + 1);
            const double frac = bin - b0;
            return (double)envelope[(size_t)b0] + ((double)envelope[(size_t)b1] - (double)envelope[(size_t)b0]) * frac;
        };
        std::vector<float> response((size_t)bins, 1.0f);
        const double nyquistBin = (double)(bins - 1);
        const double f0Bin = juce::jmax(20.0, f0) / (sampleRate / n);
        for (int b = 1; b < bins; ++b)
        {
            const double moved = (double)b / movedBy;        // the source bin the content at b came from
            const double wanted = (double)b / wantedShift;   // the source bin whose level the envelope should have here
            if (moved > nyquistBin || wanted > nyquistBin) { response[(size_t)b] = 1.0f; continue; }   // beyond the source's band: nothing to correct
            const double lowest = juce::jmin(moved, wanted);
            if (lowest < f0Bin) { response[(size_t)b] = 1.0f; continue; }
            const double share = juce::jlimit(0.0, 1.0, (lowest - f0Bin) / juce::jmax(1.0, f0Bin));   // 0 at f0, 1 at 2 f0
            const double gain = envelopeAt(wanted) / juce::jmax(1.0e-9, envelopeAt(moved));
            const double db = share * juce::jlimit(-18.0, 12.0, 20.0 * std::log10(juce::jmax(1.0e-9, gain)));
            response[(size_t)b] = (float)std::pow(10.0, db / 20.0);
        }
        response[0] = response[1];
        for (int ch = 0; ch < channels; ++ch) applyStaticEq(out[(size_t)ch].data(), outFrames, response, order);
    }

    // Time-scale modification of a quasi-periodic signal by overlap-add at
    // correlated positions (WSOLA): the output is built from Hann windows about
    // three periods long at half-window hops; every window is taken from the
    // input near its proportional position, at the offset (within a period
    // either way) where it matches the continuation of the previous window
    // best. No periodicity is assumed - the drift of the pitch, the modulations
    // and the noise only move the cutting points. 'positions' (when given)
    // receives (output position, input position) of every window, for mapping
    // points of the input into the output. Returns exactly outFrames.
    std::vector<std::vector<float>> wsolaResize(const std::vector<std::vector<float>>& in, int inFrames, int outFrames, int period,
        std::vector<std::pair<int, int>>* positions)
    {
        const int channels = (int)in.size();
        std::vector<std::vector<float>> out((size_t)channels, std::vector<float>((size_t)juce::jmax(0, outFrames), 0.0f));
        if (channels == 0 || inFrames <= 0 || outFrames <= 0) return out;
        period = juce::jmax(8, period);
        const int window = juce::jmin(inFrames, juce::jmax(64, 3 * period));
        if (inFrames <= window || outFrames <= window)
        {
            // Too short for windows: the frames spread out or squeezed (a few milliseconds at most).
            for (int ch = 0; ch < channels; ++ch)
                for (int i = 0; i < outFrames; ++i)
                    out[(size_t)ch][(size_t)i] = in[(size_t)ch][(size_t)juce::jmin(inFrames - 1, (int)((int64_t)i * inFrames / outFrames))];
            if (positions != nullptr) positions->push_back({ 0, 0 });
            return out;
        }
        const int hop = juce::jmax(1, window / 2);
        std::vector<float> hann((size_t)window);
        for (int i = 0; i < window; ++i) hann[(size_t)i] = (float)(0.5 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * i / window));
        std::vector<float> mono((size_t)inFrames, 0.0f);
        for (int ch = 0; ch < channels; ++ch) for (int i = 0; i < inFrames; ++i) mono[(size_t)i] += in[(size_t)ch][(size_t)i] / (float)channels;
        const int seek = juce::jmin(period, hop - 1);
        const double scale = (double)(inFrames - window) / (double)juce::jmax(1, outFrames - window);
        auto match = [&](int candidate, int reference)
        {
            double dot = 0.0, energy = 0.0, refEnergy = 0.0;
            for (int i = 0; i < hop; ++i)
            {
                const double a = mono[(size_t)(candidate + i)], b = mono[(size_t)(reference + i)];
                dot += a * b; energy += a * a; refEnergy += b * b;
            }
            return dot / std::sqrt(juce::jmax(1.0e-12, energy * refEnergy));
        };
        int previous = 0;
        for (int k = 0, outPos = 0; outPos < outFrames; ++k, outPos += hop)
        {
            const int nominal = juce::jlimit(0, inFrames - window, (int)std::llround((double)outPos * scale));
            int best = nominal;
            if (k > 0)
            {
                // The continuation the previous window would have had; the candidate
                // that matches it best over the overlapping half (coarse steps, then fine).
                const int reference = juce::jlimit(0, inFrames - window, previous + hop);
                const int lo = juce::jmax(0, nominal - seek), hi = juce::jmin(inFrames - window, nominal + seek);
                const int step = juce::jmax(1, seek / 48);
                double bestScore = -2.0;
                for (int candidate = lo; candidate <= hi; candidate += step)
                {
                    const double score = match(candidate, reference);
                    if (score > bestScore) { bestScore = score; best = candidate; }
                }
                if (step > 1)
                {
                    const int coarse = best;
                    for (int candidate = juce::jmax(lo, coarse - step + 1); candidate <= juce::jmin(hi, coarse + step - 1); ++candidate)
                    {
                        const double score = match(candidate, reference);
                        if (score > bestScore) { bestScore = score; best = candidate; }
                    }
                }
            }
            previous = best;
            if (positions != nullptr) positions->push_back({ outPos, best });
            // The first window starts at full weight: nothing fades the beginning in
            // (a release must begin as loud as the sustain it replaces, a chiff
            // must keep its first milliseconds).
            for (int ch = 0; ch < channels; ++ch)
                for (int i = 0; i < window && outPos + i < outFrames; ++i)
                    out[(size_t)ch][(size_t)(outPos + i)] += (k == 0 && i < hop ? 1.0f : hann[(size_t)i]) * in[(size_t)ch][(size_t)(best + i)];
        }
        return out;
    }

    std::unique_ptr<Sample> makeResampledPitchCopy(const Sample& source, int semitones, double lengthFactor = 1.0,
        float formantFollow = 0.3f, bool keepLoudness = true, double baseHz = 0.0, bool release = false,
        int method = 1, double attackShare = 0.5, double releaseShare = 1.0)
    {
        const int frames = (int)source.numFrames;
        const int channels = source.numChannels;
        lengthFactor = juce::jlimit(0.25, 2.0, lengthFactor);
        formantFollow = formantFollowForShift(formantFollow, semitones, 0.15f);
        const double ratio = std::pow(2.0, semitones / 12.0);   // read this much faster
        const int natural = frames <= 0 ? 0 : juce::jmax(1, (int)std::llround((double)frames / ratio));
        // An attack keeps its natural length (its loops live there); a release is
        // laid out again to lengthFactor of the source's duration (its room decay
        // does not grow or shrink with the pitch).
        const int target = frames <= 0 ? 0 : juce::jmax(1, (int)std::llround((double)frames * lengthFactor));
        const int outFrames = natural;
        if (frames <= 0 || semitones == 0)
        {
            auto copy = std::make_unique<Sample>(channels, juce::jmax(0, frames), source.sampleRate, source.storesFloat);
            copy->smplPitch = source.smplPitch;
            std::vector<int16_t> scratch;
            std::vector<float> scratch32;
            for (int ch = 0; ch < channels; ++ch)
            {
                if (source.storesFloat) std::memcpy(copy->writable32(ch), source.read32(ch, scratch32), (size_t)frames * sizeof(float));
                else std::memcpy(copy->writable16(ch), source.read16(ch, scratch), (size_t)frames * sizeof(int16_t));
            }
            copy->finaliseLoops(source.loops);
            return copy;
        }
        // The input, low-passed for upward shifts (everything above Nyquist / ratio would fold back).
        std::unique_ptr<Sample> filtered;
        const Sample* input = &source;
        if (ratio > 1.0) { filtered = makeAntiAliasCopy(source, ratio); input = filtered.get(); }
        std::vector<std::vector<float>> in((size_t)channels, std::vector<float>((size_t)frames));
        std::vector<int16_t> scratch;
        std::vector<float> scratch32;
        for (int ch = 0; ch < channels; ++ch)
        {
            if (input->storesFloat) { const float* s = input->read32(ch, scratch32); for (int i = 0; i < frames; ++i) in[(size_t)ch][(size_t)i] = s[i]; }
            else { const int16_t* s = input->read16(ch, scratch); for (int i = 0; i < frames; ++i) in[(size_t)ch][(size_t)i] = (float)s[i] / 32768.0f; }
        }
        std::vector<std::vector<float>> out((size_t)channels, std::vector<float>((size_t)outFrames, 0.0f));
        for (int ch = 0; ch < channels; ++ch) resampleChannel(in[(size_t)ch].data(), frames, ratio, out[(size_t)ch].data(), outFrames);

        // The envelope correction: the resampling moved the source's envelope E(f)
        // to E(f / ratio); wanted is E(f / ratio^follow).
        if (formantFollow < 0.999f && frames >= 2048)
        {
            double f0 = baseHz;
            if (!std::isnan(source.smplPitch)) f0 = 440.0 * std::pow(2.0, ((double)source.smplPitch - 69.0) / 12.0);
            moveSpectralEnvelope(out, outFrames, in, frames, source.sampleRate, f0, ratio, std::pow(ratio, (double)formantFollow));
        }
        // The pitch of the copy: the window sizes of the timing and the loop refinement.
        double sourceHz = baseHz;
        if (!std::isnan(source.smplPitch)) sourceHz = 440.0 * std::pow(2.0, ((double)source.smplPitch - 69.0) / 12.0);
        const double copyHz = sourceHz * ratio;
        const int period = juce::jmax(8, (int)std::llround(source.sampleRate / (copyHz > 1.0 ? copyHz : 100.0)));
        // The loops scale with the ratio (whole periods up to a frame; the refinement settles that).
        std::vector<LoopPoints> scaled = source.loops;
        for (auto& loop : scaled) { loop.start = (int64_t)std::llround((double)loop.start / ratio); loop.end = (int64_t)std::llround((double)loop.end / ratio); }
        int finalFrames = outFrames;
        if (release && method == 1 && target != natural)
        {
            // Pitch shift: the release's decay brought to the wanted duration (see reshapeRelease).
            reshapeRelease(out, natural, target, source.sampleRate);
            finalFrames = target;
            scaled.clear();
        }
        else if (method == 2)
        {
            // Timing kept: the attack (before the first loop) or the release gets a
            // share of its source's duration back by correlated overlap-add; the
            // loops are never touched (periodic - their length is a matter of pitch).
            int64_t firstLoop = -1;
            for (const auto& loop : source.loops) if (firstLoop < 0 || loop.start < firstLoop) firstLoop = loop.start;
            if (release || firstLoop < 0)
            {
                const double keep = std::pow(1.0 / ratio, 1.0 - (release ? releaseShare : attackShare));
                const int wanted = juce::jmax(1, (int)std::llround((double)frames * keep));
                if (wanted != natural)
                {
                    out = wsolaResize(out, natural, wanted, period, nullptr);
                    finalFrames = wanted;
                    scaled.clear();   // a sample without loops has none; a release neither
                }
            }
            else if (firstLoop > 0 && ratio < 1.0)
            {
                // Downwards only: the resampled attack is too slow and loses whole
                // windows. (Upwards it is shorter, which the ear accepts; stretching
                // it by repeating its windows made the speaking of a pipe waver.)
                const double keep = std::pow(1.0 / ratio, 1.0 - attackShare);
                const int attackIn = juce::jlimit(1, natural, (int)std::llround((double)firstLoop / ratio));
                const int attackOut = juce::jmax(1, (int)std::llround((double)firstLoop * keep));
                // The attack plus one window of the sustain, so that the last window lands
                // in the sustain and tells where the material goes on.
                const int window = juce::jmin(natural - attackIn, 3 * period);
                if (attackIn != attackOut && window > 0)
                {
                    std::vector<std::vector<float>> region((size_t)channels);
                    for (int ch = 0; ch < channels; ++ch)
                        region[(size_t)ch].assign(out[(size_t)ch].begin(), out[(size_t)ch].begin() + attackIn + window);
                    std::vector<std::pair<int, int>> positions;
                    const auto resized = wsolaResize(region, attackIn + window, attackOut + window, period, &positions);
                    // The join: where the last window that begins before the wanted length
                    // stands alone at full weight (half a window after its start), the
                    // material continues from that window's own input position - the
                    // two are the same frames there, so nothing jumps.
                    const int hop = juce::jmax(1, juce::jmin(attackIn + window, juce::jmax(64, 3 * period)) / 2);
                    int joinOut = attackOut, q = attackIn;
                    for (const auto& [o, i] : positions) if (o <= attackOut) { joinOut = o + hop; q = i + hop; }
                    joinOut = juce::jmin(joinOut, (int)resized[0].size());
                    q = juce::jlimit(0, natural - 1, q);
                    const int rest = natural - q;
                    for (int ch = 0; ch < channels; ++ch)
                    {
                        std::vector<float> joined((size_t)(joinOut + rest));
                        std::copy(resized[(size_t)ch].begin(), resized[(size_t)ch].begin() + joinOut, joined.begin());
                        std::copy(out[(size_t)ch].begin() + q, out[(size_t)ch].end(), joined.begin() + joinOut);
                        out[(size_t)ch] = std::move(joined);
                    }
                    finalFrames = joinOut + rest;
                    const int64_t shift = (int64_t)joinOut - q;
                    for (auto& loop : scaled)
                    {
                        loop.start = juce::jmax<int64_t>(0, loop.start + shift);
                        loop.end = juce::jmax<int64_t>(loop.start + 1, loop.end + shift);
                    }
                }
            }
        }
        if (keepLoudness)
        {
            double inPower = 0.0, outPower = 0.0;
            for (int ch = 0; ch < channels; ++ch)
            {
                for (int i = 0; i < frames; ++i) inPower += (double)in[(size_t)ch][(size_t)i] * in[(size_t)ch][(size_t)i];
                for (int i = 0; i < finalFrames; ++i) outPower += (double)out[(size_t)ch][(size_t)i] * out[(size_t)ch][(size_t)i];
            }
            const double inRms = std::sqrt(inPower / juce::jmax(1.0, (double)frames * channels));
            const double outRms = std::sqrt(outPower / juce::jmax(1.0, (double)finalFrames * channels));
            if (inRms > 1.0e-6 && outRms > 1.0e-6)
            {
                const float gain = (float)juce::jlimit(0.25, 4.0, inRms / outRms);
                for (int ch = 0; ch < channels; ++ch)
                    for (int i = 0; i < finalFrames; ++i) out[(size_t)ch][(size_t)i] = juce::jlimit(-1.0f, 1.0f, out[(size_t)ch][(size_t)i] * gain);
            }
        }
        auto copy = std::make_unique<Sample>(channels, finalFrames, source.sampleRate, source.storesFloat);
        copy->smplPitch = source.smplPitch;
        for (int ch = 0; ch < channels; ++ch)
        {
            if (source.storesFloat) { float* d = copy->writable32(ch); for (int i = 0; i < finalFrames; ++i) d[i] = out[(size_t)ch][(size_t)i]; }
            else { int16_t* d = copy->writable16(ch); for (int i = 0; i < finalFrames; ++i) d[i] = (int16_t)juce::jlimit(-32768.0f, 32767.0f, std::round(out[(size_t)ch][(size_t)i] * 32768.0f)); }
        }
        // The loops (scaled above, shifted by the timing when the attack changed) are
        // settled to whole periods of the copy's pitch.
        for (auto& loop : scaled) { loop.start = juce::jlimit<int64_t>(0, juce::jmax<int64_t>(0, finalFrames - 2), loop.start); loop.end = juce::jlimit<int64_t>(loop.start + 1, juce::jmax<int64_t>(loop.start + 1, finalFrames - 1), loop.end); }
        if (copyHz > 1.0 && !scaled.empty()) copy->finaliseLoops(refineLoopsForPeriod(out, finalFrames, scaled, source.sampleRate / copyHz));
        else copy->finaliseLoops(std::move(scaled));
        return copy;
    }

    // The previous method (0.7.27 - 0.8.12): Signalsmith Stretch, a phase
    // vocoder keeping the length. Kept for reference, no longer used: on organ
    // pipes it dropped harmonics when shifting down, modulated the level like an
    // LFO and, with the envelope processing, laid a noise floor over the tone.
    // Method 3, "phase vocoder": Signalsmith Stretch keeps the length and the
    // loop points; its block is sized from the pitch (eight periods of the lower
    // of source and copy, so that a 32' or 64' pipe's harmonics are resolved -
    // the default 120 ms block dropped harmonics and tremulated on deep tones),
    // its own formant handling only keeps the envelope in place (pitch
    // compensation); the move the stop asks for is then our static EQ, the same
    // as in the resampling methods. A release is cut or laid out again to
    // 'releaseLength' of the source by correlated overlap-add afterwards.
    std::unique_ptr<Sample> makePitchShiftedCopy(const Sample& source, int semitones, double releaseShare = 0.7,
        float formantFollow = 0.3f, bool keepLoudness = true, double baseHz = 0.0, bool release = false)
    {
        const int frames = (int)source.numFrames;
        const int channels = source.numChannels;
        releaseShare = juce::jlimit(0.0, 1.0, releaseShare);
        const int outFrames = frames;   // the vocoder keeps the length
        const bool unchanged = frames <= 0 || semitones == 0;
        auto copy = std::make_unique<Sample>(channels, juce::jmax(0, frames), source.sampleRate, source.storesFloat);
        copy->smplPitch = source.smplPitch;
        std::vector<int16_t> scratch;
        std::vector<float> scratch32;
        if (unchanged)
        {
            for (int ch = 0; ch < channels; ++ch)
            {
                if (source.storesFloat) std::memcpy(copy->writable32(ch), source.read32(ch, scratch32), (size_t)frames * sizeof(float));
                else std::memcpy(copy->writable16(ch), source.read16(ch, scratch), (size_t)frames * sizeof(int16_t));
            }
            copy->finaliseLoops(source.loops);
            return copy;
        }
        std::vector<std::vector<float>> in((size_t)channels, std::vector<float>((size_t)frames, 0.0f));
        std::vector<std::vector<float>> out((size_t)channels, std::vector<float>((size_t)outFrames, 0.0f));
        std::vector<float*> inPtr((size_t)channels), outPtr((size_t)channels);
        for (int ch = 0; ch < channels; ++ch)
        {
            if (source.storesFloat) { const float* s = source.read32(ch, scratch32); for (int i = 0; i < frames; ++i) in[(size_t)ch][(size_t)i] = s[i]; }
            else { const int16_t* s = source.read16(ch, scratch); for (int i = 0; i < frames; ++i) in[(size_t)ch][(size_t)i] = (float)s[i] / 32768.0f; }
            inPtr[(size_t)ch] = in[(size_t)ch].data();
            outPtr[(size_t)ch] = out[(size_t)ch].data();
        }
        const double ratio = std::pow(2.0, semitones / 12.0);
        double sourceHz = baseHz;
        if (!std::isnan(source.smplPitch)) sourceHz = 440.0 * std::pow(2.0, ((double)source.smplPitch - 69.0) / 12.0);
        const double copyHz = sourceHz * ratio;
        signalsmith::stretch::SignalsmithStretch<float> stretch;
        // The block from the pitch: eight periods of the lower of the two pitches
        // (0.1 s at least, 2 s at most), the interval a quarter of it.
        const double lowestHz = juce::jmax(1.0, juce::jmin(sourceHz > 1.0 ? sourceHz : 100.0, copyHz > 1.0 ? copyHz : 100.0));
        const int blockSamples = juce::jlimit((int)(0.1 * source.sampleRate), (int)(2.0 * source.sampleRate), (int)std::llround(8.0 * source.sampleRate / lowestHz));
        stretch.configure(channels, blockSamples, juce::jmax(64, blockSamples / 4));
        stretch.setTransposeSemitones((float)semitones, 0.0f);
        formantFollow = formantFollowForShift(formantFollow, semitones, 0.25f);
        // The library's formant handling only keeps the envelope where the
        // recording had it (pitch compensation, no shift of its own); the move the
        // stop asks for comes from our EQ below. The library takes the base
        // frequency as a fraction of the sample rate; twice the fundamental keeps
        // its envelope smoothing off single partials.
        stretch.setFormantSemitones(0.0f, true);
        stretch.setFormantBase((float)(juce::jmax(0.0, sourceHz) * 2.0 / juce::jmax(1.0, source.sampleRate)));
        const double timing0 = juce::Time::getMillisecondCounterHiRes();
        const bool done = stretch.exact(inPtr.data(), frames, outPtr.data(), outFrames);
        const double timing1 = juce::Time::getMillisecondCounterHiRes();
        if (done && formantFollow > 0.001f && frames >= 2048)
            moveSpectralEnvelope(out, outFrames, in, frames, source.sampleRate, sourceHz, 1.0, std::pow(ratio, (double)formantFollow));
        static const bool logTiming = std::getenv("ORGAN_BUILD_TIMING") != nullptr;   // development: where the time goes
        if (logTiming)
            CrashHandler::note("Vocoder copy: " + juce::String(frames) + " frames, block " + juce::String(blockSamples) + ", " + juce::String(semitones)
                + " st: stretch " + juce::String(timing1 - timing0, 0) + " ms, envelope " + juce::String(juce::Time::getMillisecondCounterHiRes() - timing1, 0) + " ms");
        // A release shifted upwards: the vocoder kept the source's length, too long
        // for a smaller pipe - shortened towards the resampled duration by the
        // release timing share (1 = the source's length kept). Downwards unchanged.
        // Then the start trimmed.
        int finalFrames = outFrames;
        if (done && release)
        {
            const int wanted = ratio > 1.0 ? juce::jmax(1, (int)std::llround((double)frames * std::pow(1.0 / ratio, 1.0 - releaseShare))) : frames;
            if (wanted != outFrames)
            {
                const int period = juce::jmax(8, (int)std::llround(source.sampleRate / (copyHz > 1.0 ? copyHz : 100.0)));
                out = wsolaResize(out, outFrames, wanted, period, nullptr);
                finalFrames = wanted;
            }
        }
        if (finalFrames != (int)copy->numFrames) copy = std::make_unique<Sample>(channels, finalFrames, source.sampleRate, source.storesFloat);
        copy->smplPitch = source.smplPitch;
        if (done && keepLoudness)
        {
            // The copy at the loudness of its source (the envelope processing and the
            // spectral move change the energy): RMS over the whole sample, gain limited.
            double inPower = 0.0, outPower = 0.0;
            for (int ch = 0; ch < channels; ++ch)
            {
                for (int i = 0; i < frames; ++i) inPower += (double)in[(size_t)ch][(size_t)i] * in[(size_t)ch][(size_t)i];
                for (int i = 0; i < finalFrames; ++i) outPower += (double)out[(size_t)ch][(size_t)i] * out[(size_t)ch][(size_t)i];
            }
            const double inRms = std::sqrt(inPower / juce::jmax(1.0, (double)frames * channels));
            const double outRms = std::sqrt(outPower / juce::jmax(1.0, (double)finalFrames * channels));
            if (inRms > 1.0e-6 && outRms > 1.0e-6)
            {
                const float gain = (float)juce::jlimit(0.25, 4.0, inRms / outRms);
                for (int ch = 0; ch < channels; ++ch)
                    for (int i = 0; i < finalFrames; ++i) out[(size_t)ch][(size_t)i] = juce::jlimit(-1.0f, 1.0f, out[(size_t)ch][(size_t)i] * gain);
            }
        }
        for (int ch = 0; ch < channels; ++ch)
        {
            if (source.storesFloat)
            {
                float* d = copy->writable32(ch);
                for (int i = 0; i < finalFrames; ++i) d[i] = done ? out[(size_t)ch][(size_t)i] : in[(size_t)ch][(size_t)juce::jmin(frames - 1, i)];   // too short to shift: unchanged
            }
            else
            {
                int16_t* d = copy->writable16(ch);
                for (int i = 0; i < finalFrames; ++i)
                {
                    const float value = done ? out[(size_t)ch][(size_t)i] : in[(size_t)ch][(size_t)juce::jmin(frames - 1, i)];
                    d[i] = (int16_t)juce::jlimit(-32768.0f, 32767.0f, std::round(value * 32768.0f));
                }
            }
        }
        if (finalFrames == frames)
        {
            // The loops stay where they were and close whole periods of the copy's pitch.
            if (done && semitones != 0 && copyHz > 1.0 && !source.loops.empty())
                copy->finaliseLoops(refineLoopsForPeriod(out, finalFrames, source.loops, source.sampleRate / copyHz));
            else
                copy->finaliseLoops(source.loops);
        }
        else copy->finaliseLoops({});   // a release laid out again: no loops
        return copy;
    }
}

void SamplerEngine::setTransposeMethod(int method)
{
    const int clamped = juce::jlimit(0, 3, method);
    if (transposeMethod.exchange(clamped) == clamped) return;
    // The copies of every stop are built for the other method: rebuild them.
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

void SamplerEngine::setTransposeAttackTiming(float share)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, share);
    if (std::abs(transposeAttackTiming.exchange(clamped) - clamped) < 1.0e-6f) return;
    if (transposeMethod.load(std::memory_order_relaxed) == 2) retirePitchShiftCopiesAndRebuild();
}

void SamplerEngine::setTransposeReleaseTiming(float share)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, share);
    if (std::abs(transposeReleaseTiming.exchange(clamped) - clamped) < 1.0e-6f) return;
    if (transposeMethod.load(std::memory_order_relaxed) >= 2) retireReleaseCopiesAndRebuild();
}

void SamplerEngine::setTransposeReleaseTrim(float milliseconds)
{
    transposeReleaseTrimMs.store(juce::jlimit(0.0f, 300.0f, milliseconds), std::memory_order_relaxed);   // applied at playback: nothing is rebuilt
}

void SamplerEngine::setTransposeReleaseLevelMatch(bool enabled)
{
    transposeReleaseLevelMatch.store(enabled, std::memory_order_relaxed);   // applied at playback: nothing is rebuilt
}

float SamplerEngine::formantFollowForPipe(const Stop* stop, int pipe) const
{
    // The setting of whoever asks for the pipe: the stop's own when the compass
    // of its division holds the pipe, otherwise the first transmission (the
    // stop's, then its stop folder's) whose shifted compass does.
    if (stop == nullptr) return defaultFormantFollow;
    auto covers = [&](int manual, int shift)
    {
        manual = juce::jlimit(0, maxManuals - 1, manual);
        const int low = juce::jlimit(0, 127, compassLow[(size_t)manual].load(std::memory_order_relaxed) + shift);
        const int high = juce::jlimit(0, 127, compassHigh[(size_t)manual].load(std::memory_order_relaxed) + shift);
        return pipe >= low && pipe <= high;
    };
    if (covers(stop->primaryManual, 0)) return stop->formantFollow;
    for (const auto& t : stop->transmissions) if (covers(t.manual, t.shift)) return t.formantFollow;
    if (stop->groupMasterPtr != nullptr)
        for (const auto& t : stop->groupMasterPtr->transmissions) if (covers(t.manual, t.shift)) return t.formantFollow;
    return stop->formantFollow;
}

void SamplerEngine::changeFormantFollow(Stop* stop, const std::function<void()>& apply)
{
    if (stop == nullptr) return;
    // The stop and the children of a stop folder (they sound through its transmissions).
    std::vector<Stop*> concerned{ stop };
    for (auto* other : stops) if (other != stop && other->groupMasterPtr == stop) concerned.push_back(other);
    const int method = transposeMethod.load(std::memory_order_relaxed);
    std::vector<std::array<float, 128>> before(concerned.size());
    for (size_t s = 0; s < concerned.size(); ++s)
        for (int pipe = 0; pipe < 128; ++pipe) before[s][(size_t)pipe] = formantFollowForPipe(concerned[s], pipe);
    {
        const juce::ScopedLock sl(controlLock);
        apply();
        if (method < 1) return;
        // Only the pipes whose setting changed lose their copies (a voice may
        // still read them: retired, not freed); the rest stay as they are.
        for (size_t s = 0; s < concerned.size(); ++s)
        {
            std::vector<Stop*> ranks{ concerned[s] };
            for (auto& rank : concerned[s]->ranks) ranks.push_back(rank.get());
            for (int pipe = 0; pipe < 128; ++pipe)
            {
                if (std::abs(formantFollowForPipe(concerned[s], pipe) - before[s][(size_t)pipe]) < 1.0e-4f) continue;
                for (auto* rank : ranks)
                {
                    auto retirePipe = [rank, pipe](std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, std::map<int, int>& aliasFreeOffset)
                    {
                        const auto it = aliasFree.find(pipe);
                        if (it == aliasFree.end()) return;
                        for (auto& sample : it->second) rank->retiredSamples.push_back(std::move(sample));
                        aliasFree.erase(it);
                        aliasFreeOffset.erase(pipe);
                    };
                    retirePipe(rank->aliasFree, rank->aliasFreeOffset);
                    for (auto& layer : rank->velocityLayers) retirePipe(layer.aliasFree, layer.aliasFreeOffset);
                    for (auto& layer : rank->releaseLayers) retirePipe(layer.aliasFree, layer.aliasFreeOffset);
                }
            }
        }
    }
    for (auto* s : concerned) buildAntiAliasCopies(s, true);   // launches only where copies are missing now
}

void SamplerEngine::setStopFormantFollow(Stop* stop, float share)
{
    if (stop == nullptr) return;
    const float clamped = juce::jlimit(0.0f, 1.0f, share);
    if (std::abs(stop->formantFollow - clamped) < 1.0e-6f) return;
    changeFormantFollow(stop, [stop, clamped] { stop->formantFollow = clamped; });
}

void SamplerEngine::setTransmissionFormantFollow(Stop* stop, int index, float share)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    const float clamped = juce::jlimit(0.0f, 1.0f, share);
    if (std::abs(stop->transmissions[(size_t)index].formantFollow - clamped) < 1.0e-6f) return;
    changeFormantFollow(stop, [stop, index, clamped] { stop->transmissions[(size_t)index].formantFollow = clamped; });
}

void SamplerEngine::setTransposeKeepLoudness(bool keep)
{
    if (transposeKeepLoudness.exchange(keep) == keep) return;
    if (transposeMethod.load(std::memory_order_relaxed) >= 1) retirePitchShiftCopiesAndRebuild();
}

void SamplerEngine::applyCopyLoopCrossfade(Sample& sample, double fraction)
{
    // fraction 0: the ordinary 50 ms splice before the loop end (blending the head
    // before the loop start). Otherwise that share of the loop's length is
    // blended AFTER the jump: from the loop start, the continuation past the loop
    // end fades into the loop - a long head before the start would reach into
    // the attack. Where the file ends too soon after the loop, the share is cut.
    if (sample.crossfadeAfterJump.size() != sample.loops.size()) sample.crossfadeAfterJump.assign(sample.loops.size(), 0);
    for (size_t i = 0; i < sample.loops.size() && i < sample.crossfadeFrames.size(); ++i)
    {
        const int64_t length = sample.loops[i].end - sample.loops[i].start;
        const int64_t afterEnd = sample.numFrames - 4 - sample.loops[i].end;   // frames readable past the loop end
        int64_t tail = fraction > 0.0 ? (int64_t)std::llround((double)length * fraction) : 0;
        tail = juce::jmin(tail, length - 1, afterEnd);
        if (tail >= 16)
        {
            sample.crossfadeAfterJump[i] = 1;
            sample.crossfadeFrames[i] = tail;
            continue;
        }
        int64_t xf = (int64_t)(sample.sampleRate * 0.050);
        xf = juce::jmin(xf, sample.loops[i].start, length - 1);
        sample.crossfadeAfterJump[i] = 0;
        sample.crossfadeFrames[i] = juce::jmax<int64_t>(1, xf);
    }
}

void SamplerEngine::packIfEnabled(Sample& sample) noexcept
{
    if (!compressSamples || sample.compressed) return;
    const int64_t raw = (int64_t)sample.frameBytes();
    size_t packedBytes = 0;
    const auto started = std::chrono::steady_clock::now();
    try { packedBytes = sample.storesFloat ? sample.compress32() : sample.compress16(); }   // float: as the file's integers (0.8.183)
    catch (...) { packedBytes = 0; }   // out of memory for the packed copy: the raw data stays
    packNanoseconds.fetch_add((int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - started).count(), std::memory_order_relaxed);
    if (packedBytes == 0)
    {
        if (sample.numFrames >= Sample::kPackBlock) (sample.storesFloat ? packFloatsLeftRaw : packFailed).fetch_add(1, std::memory_order_relaxed);
        return;
    }
    if (sample.packRounded) packFloatsRounded.fetch_add(1, std::memory_order_relaxed);
    packRawBytes.fetch_add(raw, std::memory_order_relaxed);
    packPackedBytes.fetch_add((int64_t)packedBytes, std::memory_order_relaxed);
    packSamples.fetch_add(1, std::memory_order_relaxed);
}

void SamplerEngine::notePackStats(const juce::String& what)
{
    const PackStats stats = takePackStats();
    if (stats.samples == 0 && stats.failed == 0 && stats.floatsLeftRaw == 0) return;
    const double saved = stats.rawBytes > 0 ? 100.0 * (1.0 - (double)stats.packedBytes / (double)stats.rawBytes) : 0.0;
    CrashHandler::note("Packed samples: " + what + ": " + juce::String(stats.samples) + " samples, "
        + juce::File::descriptionOfSizeInBytes(stats.rawBytes) + " -> " + juce::File::descriptionOfSizeInBytes(stats.packedBytes)
        + " (" + juce::String(saved, 1) + " % saved)" + (stats.failed > 0 ? ", " + juce::String(stats.failed) + " left raw (the check failed)" : juce::String())
        + (stats.floatsRounded > 0 ? ", " + juce::String(stats.floatsRounded) + " float sample(s) rounded to 24 bits (real floats, not the file's integers)" : juce::String())
        + (stats.floatsLeftRaw > 0 ? ", " + juce::String(stats.floatsLeftRaw) + " float sample(s) left raw (beyond +-2 full scale)" : juce::String())
        + ", " + juce::String((double)stats.nanoseconds * 1.0e-9, 1) + " s of thread time ("
        + juce::String(stats.rawBytes > 0 ? (double)stats.nanoseconds / ((double)stats.rawBytes / 2.0) : 0.0, 2) + " ns per frame and channel)");
}

SamplerEngine::PackStats SamplerEngine::takePackStats() noexcept
{
    PackStats stats;
    stats.rawBytes = packRawBytes.exchange(0, std::memory_order_relaxed);
    stats.packedBytes = packPackedBytes.exchange(0, std::memory_order_relaxed);
    stats.samples = packSamples.exchange(0, std::memory_order_relaxed);
    stats.failed = packFailed.exchange(0, std::memory_order_relaxed);
    stats.floatsLeftRaw = packFloatsLeftRaw.exchange(0, std::memory_order_relaxed);
    stats.floatsRounded = packFloatsRounded.exchange(0, std::memory_order_relaxed);
    stats.nanoseconds = packNanoseconds.exchange(0, std::memory_order_relaxed);
    return stats;
}

void SamplerEngine::setCopyLoopCrossfade(float fraction)
{
    const float clamped = juce::jlimit(0.0f, 1.0f, fraction);
    if (std::abs(copyLoopCrossfade.exchange(clamped) - clamped) < 1.0e-6f) return;
    // Applied to every copy in place: the splice length is read per block by the
    // voices (a change mid-splice only bends that one blend). No rebuild, no cache.
    BuildPause pause(*this);
    const juce::ScopedLock sl(controlLock);
    for (auto* stop : stops)
    {
        std::vector<Stop*> ranks{ stop };
        for (auto& rank : stop->ranks) ranks.push_back(rank.get());
        for (auto* rank : ranks)
        {
            auto applyAll = [clamped](std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree)
            {
                for (auto& [pipe, copies] : aliasFree) for (auto& copy : copies) if (copy != nullptr) applyCopyLoopCrossfade(*copy, clamped);
            };
            applyAll(rank->aliasFree);
            for (auto& layer : rank->velocityLayers) applyAll(layer.aliasFree);
            for (auto& layer : rank->releaseLayers) applyAll(layer.aliasFree);
        }
    }
}

void SamplerEngine::retirePitchShiftCopiesAndRebuild()
{
    // Every copy (attacks, velocity layers, releases) of every rank was built with
    // other parameters: retire them (a voice may still read them) and build again
    // in the background; until then a filled pipe plays resampled.
    {
        const juce::ScopedLock sl(controlLock);
        for (auto* stop : stops)
        {
            std::vector<Stop*> ranks{ stop };
            for (auto& rank : stop->ranks) ranks.push_back(rank.get());
            for (auto* rank : ranks)
            {
                auto retireAll = [rank](std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, std::map<int, int>& aliasFreeOffset)
                {
                    for (auto& [pipe, copies] : aliasFree) for (auto& sample : copies) rank->retiredSamples.push_back(std::move(sample));
                    aliasFree.clear();
                    aliasFreeOffset.clear();
                };
                retireAll(rank->aliasFree, rank->aliasFreeOffset);
                for (auto& layer : rank->velocityLayers) retireAll(layer.aliasFree, layer.aliasFreeOffset);
                for (auto& layer : rank->releaseLayers) retireAll(layer.aliasFree, layer.aliasFreeOffset);
            }
        }
    }
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

int SamplerEngine::countMissingCopies(Stop* stop, juce::String* detail)
{
    const int method = transposeMethod.load(std::memory_order_relaxed);
    if (stop == nullptr || (method == 0 && transposeQuality.load(std::memory_order_relaxed) < 2)) return 0;
    const juce::ScopedLock sl(controlLock);
    std::vector<Stop*> ranks{ stop };
    for (auto& rank : stop->ranks) ranks.push_back(rank.get());
    int planned = 0;
    for (auto* rank : ranks)
    {
        if (rank->samplesUnloaded) continue;
        auto needed = [&](int pipe)
        {
            if (!rank->isFilled(pipe)) return false;
            const int offset = rank->fillOffset[(size_t)pipe];
            return method >= 1 ? offset != 0 : offset < 0;
        };
        // A copy can only be made where the set has the source pipe's sample (a
        // velocity layer or a release set with a few files has none for most pipes).
        // Pitch shift: a copy made for another "formants follow" of its stop is missing too.
        auto missing = [&](const std::map<int, int>& offsets, const std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree,
            const std::array<std::vector<std::unique_ptr<Sample>>, 128>& samplesByNote, bool all)
        {
            int count = 0;
            for (int pipe = 0; pipe < 128; ++pipe)
            {
                if (!needed(pipe)) continue;
                const int source = pipe + rank->fillOffset[(size_t)pipe];
                if (source < 0 || source >= 128 || samplesByNote[(size_t)source].empty()) continue;
                if (all) { ++count; continue; }
                const auto it = offsets.find(pipe);
                if (it == offsets.end() || it->second != rank->fillOffset[(size_t)pipe]) { ++count; continue; }
                if (method >= 1)
                {
                    const auto have = aliasFree.find(pipe);
                    if (have != aliasFree.end() && !have->second.empty() && have->second.front() != nullptr
                        && have->second.front()->formantPct != juce::roundToInt(formantFollowForPipe(stop, pipe) * 100.0f)) ++count;
                }
            }
            return count;
        };
        const bool all = rank->copiesMethod != method;   // copies of the other method: every set again
        const int plain = missing(rank->aliasFreeOffset, rank->aliasFree, rank->samplesByNote, all);
        planned += plain;
        if (detail != nullptr) *detail << " plain " << plain;
        for (auto& layer : rank->velocityLayers) { const int m = missing(layer.aliasFreeOffset, layer.aliasFree, layer.samplesByNote, all); planned += m; if (detail != nullptr) *detail << " layer_" << layer.maxVelocity << " " << m; }
        if (method >= 1) for (auto& layer : rank->releaseLayers) { const int m = missing(layer.aliasFreeOffset, layer.aliasFree, layer.samplesByNote, all); planned += m; if (detail != nullptr) *detail << " release_" << layer.maxHoldMs << "/" << layer.maxVelocity << " " << m; }
    }
    return planned;
}

void SamplerEngine::relaunchPendingBuilds()
{
    if (buildsAborted.load(std::memory_order_acquire) || buildsHeld.load(std::memory_order_acquire)
        || relaunchHeld.load(std::memory_order_acquire)) return;   // between the unloads of a load (0.8.184)
    for (auto* stop : stops)
        if (!stop->copiesQueued.load(std::memory_order_acquire)) buildAntiAliasCopies(stop, true);   // launches only when copies are missing
}

void SamplerEngine::holdCopyBuilds()
{
    buildsHeld.store(true, std::memory_order_release);
}

void SamplerEngine::launchCopyBuilds()
{
    buildsHeld.store(false, std::memory_order_release);
    relaunchPendingBuilds();
}

int SamplerEngine::resumeIncompleteCopyBuilds()
{
    if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)
        || buildsHeld.load(std::memory_order_acquire)) return 0;
    int resumed = 0;
    for (auto* stop : stops)
    {
        if (!stop->copiesIncomplete.load(std::memory_order_acquire) || stop->copiesQueued.load(std::memory_order_acquire)) continue;
        if (countMissingCopies(stop) <= 0) { stop->copiesIncomplete.store(false, std::memory_order_release); continue; }
        buildAntiAliasCopies(stop, true);
        if (stop->copiesQueued.load(std::memory_order_acquire)) ++resumed;
    }
    return resumed;
}

SamplerEngine::BuildPause::BuildPause(SamplerEngine& e) : engine(e)
{
    engine.buildsInterrupted.store(true, std::memory_order_release);
    engine.antiAliasBuildLock.enter();   // the running build stops at its next pipe
}

SamplerEngine::BuildPause::~BuildPause()
{
    // The flag goes before the lock is released: a build waiting for the lock must
    // not see it and give up (its stop would then count as queued but never build).
    engine.buildsInterrupted.store(false, std::memory_order_release);
    engine.antiAliasBuildLock.exit();
    engine.relaunchPendingBuilds();
}

void SamplerEngine::setTransposeReleaseLength(float share)
{
    const float clamped = juce::jlimit(0.25f, 2.0f, share);
    if (std::abs(transposeReleaseLength.exchange(clamped) - clamped) < 1.0e-6f) return;
    if (transposeMethod.load(std::memory_order_relaxed) == 1) retireReleaseCopiesAndRebuild();
}

void SamplerEngine::retireReleaseCopiesAndRebuild()
{
    // The release copies were built for another length: every stop's copies again
    // (the attack copies are up to date and skipped by the offset check - the release
    // ones are forced by retiring them first).
    {
        const juce::ScopedLock sl(controlLock);
        for (auto* stop : stops)
        {
            std::vector<Stop*> ranks{ stop };
            for (auto& rank : stop->ranks) ranks.push_back(rank.get());
            for (auto* rank : ranks)
                for (auto& layer : rank->releaseLayers)
                {
                    for (auto& [pipe, copies] : layer.aliasFree) for (auto& sample : copies) rank->retiredSamples.push_back(std::move(sample));
                    layer.aliasFree.clear();
                    layer.aliasFreeOffset.clear();
                }
        }
    }
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

namespace
{
    // The copy cache file of one folder tree: "OSPC", version, the parameter key,
    // then one record per filled pipe: pipe, fill offset, the frame counts of the
    // source variants (the identity of the source), and the copies themselves.
    constexpr uint32_t kCopyCacheMagic = 0x4350534F;   // "OSPC"
    constexpr int kCopyCacheVersion = 4;   // 2: "formants follow" per record; 3: one file per rank and set; 4: the release level gain in every sample

    struct CopyCacheRecord
    {
        int offset = 0;
        int formantPct = -1;    // the "formants follow" share the copies were built with (-1: not pitch-shifted)
        std::vector<int64_t> sourceFrames;
        int64_t position = 0;   // of the variants in the file
        int variants = 0;
    };

    void writeCachedSample(juce::OutputStream& out, const Sample& s)
    {
        juce::MemoryOutputStream block;
        block.writeInt(s.numChannels);
        block.writeInt64(s.numFrames);
        block.writeDouble(s.sampleRate);
        block.writeDouble(s.sourceSampleRate);
        block.writeBool(s.storesFloat);
        block.writeFloat(s.smplPitch);
        block.writeInt(s.formantPct);
        block.writeFloat(s.releaseLevelGain);
        block.writeInt((int)s.loops.size());
        for (const auto& loop : s.loops) { block.writeInt64(loop.start); block.writeInt64(loop.end); }
        std::vector<int16_t> scratch;
        std::vector<float> scratch32;
        for (int ch = 0; ch < s.numChannels; ++ch)
        {
            if (s.storesFloat) block.write(s.read32(ch, scratch32), (size_t)s.numFrames * sizeof(float));
            else block.write(s.read16(ch, scratch), (size_t)s.numFrames * sizeof(int16_t));   // the cache holds plain frames, packed or not
        }
        out.writeInt64((int64_t)block.getDataSize());   // so a reader can skip the variant
        out.write(block.getData(), block.getDataSize());
    }

    std::unique_ptr<Sample> readCachedSample(juce::InputStream& in)
    {
        const int64_t length = in.readInt64();
        const int64_t end = in.getPosition() + length;
        const int channels = in.readInt();
        const int64_t frames = in.readInt64();
        const double rate = in.readDouble();
        const double sourceRate = in.readDouble();
        const bool asFloat = in.readBool();
        const float pitch = in.readFloat();
        const int formantPct = in.readInt();
        const float releaseLevelGain = in.readFloat();
        const int loopCount = in.readInt();
        if (channels < 1 || channels > 64 || frames < 0 || frames > (int64_t)1 << 31 || loopCount < 0 || loopCount > Sample::maxLoops) return nullptr;
        std::vector<LoopPoints> loops;
        for (int i = 0; i < loopCount; ++i) { LoopPoints loop; loop.start = in.readInt64(); loop.end = in.readInt64(); loops.push_back(loop); }
        auto sample = std::make_unique<Sample>(channels, frames, rate, asFloat);
        sample->sourceSampleRate = sourceRate;
        sample->smplPitch = pitch;
        sample->formantPct = formantPct;
        sample->releaseLevelGain = releaseLevelGain > 0.0f ? releaseLevelGain : 1.0f;
        for (int ch = 0; ch < channels; ++ch)
        {
            const size_t bytes = (size_t)frames * (asFloat ? sizeof(float) : sizeof(int16_t));
            void* target = asFloat ? (void*)sample->writable32(ch) : (void*)sample->writable16(ch);
            if (in.read(target, (int)bytes) != (int)bytes) return nullptr;
        }
        sample->finaliseLoops(std::move(loops));
        in.setPosition(end);
        return sample;
    }

    // The records of a cache file whose parameter key matches (empty otherwise).
    std::map<int, CopyCacheRecord> readCopyCacheIndex(juce::InputStream& in, const juce::String& parameters)
    {
        std::map<int, CopyCacheRecord> index;
        if ((uint32_t)in.readInt() != kCopyCacheMagic || in.readInt() != kCopyCacheVersion) return index;
        if (in.readString() != parameters) return index;
        const int count = in.readInt();
        for (int i = 0; i < count && !in.isExhausted(); ++i)
        {
            const int pipe = in.readInt();
            CopyCacheRecord record;
            record.offset = in.readInt();
            record.formantPct = in.readInt();
            const int sources = in.readInt();
            if (pipe < 0 || pipe >= 128 || sources < 0 || sources > 256) return {};
            for (int s = 0; s < sources; ++s) record.sourceFrames.push_back(in.readInt64());
            record.variants = in.readInt();
            if (record.variants < 0 || record.variants > 256) return {};
            record.position = in.getPosition();
            for (int v = 0; v < record.variants; ++v) { const int64_t length = in.readInt64(); in.setPosition(in.getPosition() + length); }
            index[pipe] = record;
        }
        return index;
    }

    std::vector<std::unique_ptr<Sample>> readCopyCacheVariants(juce::InputStream& in, const CopyCacheRecord& record)
    {
        std::vector<std::unique_ptr<Sample>> copies;
        in.setPosition(record.position);
        for (int v = 0; v < record.variants; ++v)
        {
            auto sample = readCachedSample(in);
            if (sample == nullptr) return {};
            copies.push_back(std::move(sample));
        }
        return copies;
    }

    // The file name of a rank's folder tree: readable leaf, set tag, hash of the
    // folder path. One file per rank and set whatever the settings: a set built
    // with other settings replaces the file (the parameters are in its header),
    // so the cache never holds two versions of the same copies.
    juce::String copyCachePathTag(const Stop& rank) { return juce::String::toHexString(rank.folder.getFullPathName().hashCode64()); }
    juce::File copyCacheFileFor(const juce::File& directory, const Stop& rank, const juce::String& setTag)
    {
        const auto leaf = juce::File::createLegalFileName(rank.folder.getFileName().isNotEmpty() ? rank.folder.getFileName() : rank.name).replaceCharacter(' ', '_');
        return directory.getChildFile(leaf + "_" + setTag + "_" + copyCachePathTag(rank) + ".ospc");
    }

    // Writes one folder tree's copies (every pipe in 'offsets' that has copies) as a
    // cache file: a temporary file renamed into place. The samples only change under
    // the build lock, which the caller holds.
    bool writeCopyCacheFile(const juce::File& cacheFile, const juce::String& parameters, const std::map<int, int>& offsets,
        const std::array<std::vector<std::unique_ptr<Sample>>, 128>& samplesByNote,
        const std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree)
    {
        int count = 0;
        for (const auto& [pipe, copies] : aliasFree) if (offsets.count(pipe) != 0 && !copies.empty()) ++count;
        if (count == 0) return false;
        const auto temporary = cacheFile.getSiblingFile(cacheFile.getFileName() + ".part");
        temporary.deleteFile();
        bool ok = false;
        {
            juce::FileOutputStream out(temporary);
            if (out.openedOk())
            {
                out.writeInt((int)kCopyCacheMagic);
                out.writeInt(kCopyCacheVersion);
                out.writeString(parameters);
                out.writeInt(count);
                for (const auto& [pipe, copies] : aliasFree)
                {
                    const auto offset = offsets.find(pipe);
                    if (offset == offsets.end() || copies.empty()) continue;
                    const int source = pipe + offset->second;
                    out.writeInt(pipe);
                    out.writeInt(offset->second);
                    out.writeInt(copies.front() != nullptr ? copies.front()->formantPct : -1);
                    std::vector<int64_t> sourceFrames;
                    if (source >= 0 && source < 128)
                        for (const auto& variant : samplesByNote[(size_t)source]) if (variant != nullptr) sourceFrames.push_back(variant->numFrames);
                    out.writeInt((int)sourceFrames.size());
                    for (const auto frames : sourceFrames) out.writeInt64(frames);
                    out.writeInt((int)copies.size());
                    for (const auto& copy : copies) writeCachedSample(out, *copy);
                }
                out.flush();
                ok = out.getStatus().wasOk();
            }
        }
        if (ok && temporary.moveFileTo(cacheFile)) return true;
        temporary.deleteFile();
        return false;
    }
}

juce::String SamplerEngine::copyParametersKey() const
{
    const int method = transposeMethod.load(std::memory_order_relaxed);
    // v4: resampling with the static envelope EQ (v1-v3 were the phase vocoder).
    // v5: the formant share is per stop (in every copy's record), the release
    // length replaces the shortening rule.
    juce::String key = "v5;method=" + juce::String(method) + ";quality=" + juce::String(transposeQuality.load(std::memory_order_relaxed));
    if (method >= 1)
        key << ";loudness=" << (transposeKeepLoudness.load(std::memory_order_relaxed) ? 1 : 0);
    if (method == 1) key << ";release=" << juce::String(transposeReleaseLength.load(std::memory_order_relaxed), 3);
    if (method == 2) key << ";attackTiming=" << juce::String(transposeAttackTiming.load(std::memory_order_relaxed), 3);
    if (method >= 2) key << ";releaseTiming=" << juce::String(transposeReleaseTiming.load(std::memory_order_relaxed), 3);
    return key;
}

void SamplerEngine::pruneStaleCopyCacheFiles()
{
    // Files of an older format (they would never be read) and leftover
    // temporaries go, so the folder holds nothing the cache cannot use.
    if (copyCacheDir == juce::File()) return;
    int removed = 0;
    int64_t bytes = 0;
    for (const auto& file : copyCacheDir.findChildFiles(juce::File::findFiles, false, "*.ospc*"))
    {
        bool stale = file.getFileName().endsWithIgnoreCase(".part");
        if (!stale)
        {
            juce::FileInputStream in(file);
            stale = !in.openedOk() || (uint32_t)in.readInt() != kCopyCacheMagic || in.readInt() != kCopyCacheVersion;
        }
        if (!stale) continue;
        const int64_t size = file.getSize();
        if (file.deleteFile()) { ++removed; bytes += size; }
    }
    if (removed > 0)
        CrashHandler::note("Filled pipes cache: " + juce::String(removed) + " file(s) of an older format removed ("
            + juce::File::descriptionOfSizeInBytes(bytes) + ")");
}

int SamplerEngine::removeCopyCacheFilesOfUnloadedRanks()
{
    // Every file whose rank (its folder path, hashed into the name) is not among
    // the loaded ranks: copies of another configuration's stops, or of folders
    // that are gone. Returns how many files went.
    if (copyCacheDir == juce::File()) return 0;
    std::set<juce::String> loaded;
    {
        const juce::ScopedLock sl(controlLock);
        for (auto* stop : stops)
        {
            loaded.insert(copyCachePathTag(*stop));
            for (auto& rank : stop->ranks) loaded.insert(copyCachePathTag(*rank));
        }
    }
    BuildPause pause(*this);   // no build writes while the files go
    int removed = 0;
    int64_t bytes = 0;
    for (const auto& file : copyCacheDir.findChildFiles(juce::File::findFiles, false, "*.ospc"))
    {
        const auto tag = file.getFileNameWithoutExtension().fromLastOccurrenceOf("_", false, false);
        if (loaded.count(tag) != 0) continue;
        const int64_t size = file.getSize();
        if (file.deleteFile()) { ++removed; bytes += size; }
    }
    CrashHandler::note("Filled pipes cache: " + juce::String(removed) + " file(s) of stops not loaded removed ("
        + juce::File::descriptionOfSizeInBytes(bytes) + ")");
    return removed;
}

void SamplerEngine::setCopyCache(bool enabled, const juce::File& directory)
{
    copyCacheDir = directory;
    if (enabled && directory != juce::File()) { directory.createDirectory(); pruneStaleCopyCacheFiles(); }
    const bool now = enabled && directory != juce::File();
    const bool before = copyCacheEnabled.exchange(now, std::memory_order_acq_rel);
    // Switched on with copies already built: they go to the cache now (a worker,
    // under the build lock), so the next load finds them.
    if (now && !before && !stops.isEmpty()) exportCopiesToCache();
}

void SamplerEngine::exportCopiesToCache()
{
    backgroundBuilds.fetch_add(1, std::memory_order_acq_rel);
    const juce::File directory = copyCacheDir;   // the thread's own copy (the member may be reassigned meanwhile)
    juce::Thread::launch([this, directory]
    {
        int written = 0;
        CrashHandler::note("Filled pipes cache: exporting the copies already built");
        {
            const juce::ScopedLock buildLock(antiAliasBuildLock);
            if (impl != nullptr && copyCacheEnabled.load(std::memory_order_acquire) && !buildsAborted.load(std::memory_order_acquire))
            {
                const auto parameters = copyParametersKey();
                std::vector<Stop*> ranks;
                {
                    const juce::ScopedLock sl(controlLock);
                    for (auto* stop : stops)
                    {
                        ranks.push_back(stop);
                        for (auto& rank : stop->ranks) ranks.push_back(rank.get());
                    }
                }
                const int method = transposeMethod.load(std::memory_order_relaxed);
                for (auto* rank : ranks)
                {
                    if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)) break;
                    if (rank->samplesUnloaded || rank->copiesMethod != method) continue;
                    auto writeSet = [&](const juce::String& setTag, const std::array<std::vector<std::unique_ptr<Sample>>, 128>& samplesByNote,
                        const std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, const std::map<int, int>& offsets)
                    {
                        if (aliasFree.empty() || !copyCacheEnabled.load(std::memory_order_acquire)) return;
                        const auto file = copyCacheFileFor(directory, *rank, setTag);
                        if (file.existsAsFile()) return;
                        if (writeCopyCacheFile(file, parameters, offsets, samplesByNote, aliasFree)) ++written;
                    };
                    writeSet("plain", rank->samplesByNote, rank->aliasFree, rank->aliasFreeOffset);
                    for (auto& layer : rank->velocityLayers) writeSet("layer_" + juce::String(layer.maxVelocity), layer.samplesByNote, layer.aliasFree, layer.aliasFreeOffset);
                    if (method >= 1)
                        for (auto& layer : rank->releaseLayers)
                            writeSet("release_" + juce::String(layer.maxHoldMs) + "_" + juce::String(layer.maxVelocity), layer.samplesByNote, layer.aliasFree, layer.aliasFreeOffset);
                }
            }
        }
        copyCacheWrites.fetch_add(written, std::memory_order_relaxed);
        CrashHandler::note("Filled pipes cache: " + juce::String(written) + " set file(s) written for the copies already built");
        backgroundBuilds.fetch_sub(1, std::memory_order_acq_rel);
    });
}

void SamplerEngine::clearCopyCache()
{
    if (copyCacheDir == juce::File()) return;
    BuildPause pause(*this);   // no build writes while the files go
    for (const auto& file : copyCacheDir.findChildFiles(juce::File::findFiles, false, "*.ospc")) file.deleteFile();
    CrashHandler::note("Filled pipes cache emptied");
}

int64_t SamplerEngine::copyCacheBytes() const
{
    if (copyCacheDir == juce::File()) return 0;
    int64_t total = 0;
    for (const auto& file : copyCacheDir.findChildFiles(juce::File::findFiles, false, "*.ospc")) total += file.getSize();
    return total;
}

void SamplerEngine::buildAntiAliasCopies(Stop* stop, bool background)
{
    const int method = transposeMethod.load(std::memory_order_relaxed);
    if (stop == nullptr || (method == 0 && transposeQuality.load(std::memory_order_relaxed) < 2)) return;
    if (background)
    {
        // Message-thread callers: filter on a worker so the UI does not freeze. The
        // counter lets the destructor wait for workers that have not started yet.
        // Only stops that miss copies are queued (the UI marks them).
        if (countMissingCopies(stop) <= 0) return;
        if (buildsHeld.load(std::memory_order_acquire))
        {
            // "Compiling the Stop List": launched by launchCopyBuilds at its end.
            stop->copiesIncomplete.store(true, std::memory_order_release);
            return;
        }
        stop->copiesQueued.store(true, std::memory_order_release);
        backgroundBuilds.fetch_add(1, std::memory_order_acq_rel);
        juce::Thread::launch([this, stop]
        {
            lowerCurrentThreadPriority();
            // 'stop' may be unloaded while this waits for the lock: the build checks
            // its presence under the lock and clears the queued flag itself; nothing
            // touches the pointer here afterwards.
            try { buildAntiAliasCopies(stop, false); }
            catch (...) {}   // out of memory while filtering: the pipe keeps playing the plain sample
            backgroundBuilds.fetch_sub(1, std::memory_order_acq_rel);
        });
        return;
    }
    const juce::ScopedLock buildLock(antiAliasBuildLock);   // unloading waits for a running build
    if (impl == nullptr) return;
    std::vector<Stop*> ranks;
    {
        const juce::ScopedLock sl(controlLock);
        bool present = false;
        for (auto* s : stops) if (s == stop) present = true;
        if (!present)
        {
            // Not committed yet (loader thread) or already unloaded: only a loader may proceed.
            std::lock_guard<std::mutex> lk(impl->pendingMutex);
            for (const auto& division : impl->pendingDivisions)
                for (auto* s : division->stops) if (s == stop) present = true;
        }
        if (!present) return;
        ranks.push_back(stop);
        for (auto& rank : stop->ranks) ranks.push_back(rank.get());
    }
    // The stop exists: whatever happens below, it is no longer queued when this
    // returns; a build that did not reach its end is marked for a later launch.
    struct QueuedReset
    {
        Stop* stop; bool completed = false;
        ~QueuedReset()
        {
            stop->copiesIncomplete.store(!completed, std::memory_order_release);
            stop->copiesQueued.store(false, std::memory_order_release);
        }
    } queuedReset{ stop };
    if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)) return;   // launched again later
    // Progress for the UI: how many pipes (over every rank and folder tree) still
    // need a copy; counted first, then built.
    {
        const int planned = countMissingCopies(stop);
        stop->copiesPlanned.store(planned, std::memory_order_relaxed);
        stop->copiesDone.store(0, std::memory_order_relaxed);
        stop->copiesBuilding.store(planned > 0, std::memory_order_release);
    }
    struct BuildingReset { Stop* stop; ~BuildingReset() { stop->copiesBuilding.store(false, std::memory_order_release); } } buildingReset{ stop };
    // The activity log keeps the build of every stop (start, and how far it got:
    // an interrupted build ends below its plan and is launched again later).
    if (stop->copiesPlanned.load() > 0)
        CrashHandler::note("Copy build of " + stop->name + ": " + juce::String(stop->copiesPlanned.load()) + " copies"
            + (copyCacheEnabled.load(std::memory_order_acquire) ? juce::String() : juce::String(" (the filled pipes cache is OFF: built again at every load)")));
    struct DoneNote
    {
        Stop* stop; SamplerEngine* engine; int hitsBefore;
        ~DoneNote()
        {
            if (stop->copiesPlanned.load() <= 0) return;
            juce::String why;
            if (engine->buildsAborted.load(std::memory_order_acquire)) why = ", the program is closing";
            else if (engine->buildsInterrupted.load(std::memory_order_acquire)) why = ", interrupted (launched again later)";
            CrashHandler::note("Copy build of " + stop->name + " ended at " + juce::String(stop->copiesDone.load()) + "/" + juce::String(stop->copiesPlanned.load())
                + " (" + juce::String(engine->copyCacheHits.load() - hitsBefore) + " pipes from the cache" + why + ")");
            engine->notePackStats("the copies of " + stop->name);
        }
    } doneNote{ stop, this, copyCacheHits.load() };
    std::atomic<bool> memoryShort{ false };   // the memory reserve reached by this build (0.8.180): the rest is left for later
    for (auto* rank : ranks)
    {
        // What is needed. Resampling: filled pipes whose source lies below them
        // (played faster, so an anti-alias copy). Pitch shift: every filled pipe
        // (the copy is already at the pipe's pitch, played at unity).
        std::map<int, int> wanted;   // pipe -> offset
        size_t layerCount = 0, releaseLayerCount = 0;
        {
            const juce::ScopedLock sl(controlLock);
            if (rank->samplesUnloaded) continue;
            for (int pipe = 0; pipe < 128; ++pipe)
                if (rank->isFilled(pipe) && (method >= 1 ? rank->fillOffset[(size_t)pipe] != 0 : rank->fillOffset[(size_t)pipe] < 0))
                    wanted[pipe] = rank->fillOffset[(size_t)pipe];
            layerCount = rank->velocityLayers.size();
            releaseLayerCount = rank->releaseLayers.size();
            if (rank->copiesMethod != method)
            {
                // Copies of the other method are useless: retire them all.
                auto retireAll = [rank](std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, std::map<int, int>& aliasFreeOffset)
                {
                    for (auto& [pipe, copies] : aliasFree) for (auto& sample : copies) rank->retiredSamples.push_back(std::move(sample));
                    aliasFree.clear();
                    aliasFreeOffset.clear();
                };
                retireAll(rank->aliasFree, rank->aliasFreeOffset);
                for (auto& layer : rank->velocityLayers) retireAll(layer.aliasFree, layer.aliasFreeOffset);
                for (auto& layer : rank->releaseLayers) retireAll(layer.aliasFree, layer.aliasFreeOffset);
                rank->copiesMethod = method;
            }
        }
        // The plain samples and every velocity layer get their own copies (a filled
        // pipe then plays the right layer at every velocity). The layer vector is
        // only changed by unload/reload, which wait for antiAliasBuildLock.
        // Releases keep their source's duration (times the setting) in both
        // directions: the room decay is laid out again after the resampling.
        const double releaseLength = (double)transposeReleaseLength.load(std::memory_order_relaxed);
        const double attackTiming = (double)transposeAttackTiming.load(std::memory_order_relaxed);
        const double releaseTiming = (double)transposeReleaseTiming.load(std::memory_order_relaxed);
        // "Formants follow" per pipe: the stop's own setting, or that of the
        // transmission which alone asks for the pipe (the copies carry it, so a
        // changed setting is told from the copies and the cache records).
        std::array<int, 128> formantPct{};
        {
            const juce::ScopedLock sl(controlLock);
            for (int pipe = 0; pipe < 128; ++pipe)
                formantPct[(size_t)pipe] = method >= 1 ? juce::roundToInt(formantFollowForPipe(stop, pipe) * 100.0f) : -1;
        }
        const bool keepLoudness = transposeKeepLoudness.load(std::memory_order_relaxed);
        const bool cacheOn = copyCacheEnabled.load(std::memory_order_acquire) && copyCacheDir != juce::File();
        const juce::String cacheParameters = cacheOn ? copyParametersKey() : juce::String();
        // The gain that would put a release copy at the level the source's release
        // has against the source's sustain (every method leaves the start of a
        // release at its own level, up to 6 dB off, measured). Only measured here
        // and kept with the copy (Sample::releaseLevelGain); applied at playback
        // when the option is on, so switching it changes nothing in the samples.
        // The attack copy of the pipe (the plain set, built first) gives the copy's
        // sustain; the release's start is its first 100 ms.
        auto rmsOf = [](const Sample& s, int64_t from, int64_t to)
        {
            from = juce::jlimit<int64_t>(0, s.numFrames, from);
            to = juce::jlimit<int64_t>(from, s.numFrames, to);
            if (to <= from) return 0.0;
            double power = 0.0;
            std::vector<int16_t> scratch;
            std::vector<float> scratch32;
            for (int ch = 0; ch < s.numChannels; ++ch)
            {
                const int16_t* x16 = s.storesFloat ? nullptr : s.read16(ch, scratch);
                const float* x32 = s.storesFloat ? s.read32(ch, scratch32) : nullptr;
                for (int64_t i = from; i < to; ++i)
                {
                    const double v = s.storesFloat ? (double)x32[i] : (double)x16[i] / 32768.0;
                    power += v * v;
                }
            }
            return std::sqrt(power / (double)((to - from) * juce::jmax(1, s.numChannels)));
        };
        auto sustainRms = [&](const Sample& s)
        {
            int64_t from = s.numFrames * 7 / 10, to = s.numFrames;   // the last 30 % when there are no loops
            if (!s.loops.empty()) { from = juce::jmax<int64_t>(0, s.loops.front().start); to = juce::jmax(from + 1, s.maxLoopEnd > from ? s.maxLoopEnd : s.numFrames); }
            return rmsOf(s, from, to);
        };
        auto releaseLevelGain = [&](Stop* rank, int pipe, int source, const std::array<std::vector<std::unique_ptr<Sample>>, 128>& releaseSet,
            std::vector<std::unique_ptr<Sample>>& copies)
        {
            const Sample* sourceAttack = nullptr;
            const Sample* copyAttack = nullptr;
            const Sample* sourceRelease = nullptr;
            {
                const juce::ScopedLock sl(controlLock);
                if (!rank->samplesByNote[(size_t)source].empty()) sourceAttack = rank->samplesByNote[(size_t)source][0].get();
                const auto it = rank->aliasFree.find(pipe);
                if (it != rank->aliasFree.end() && !it->second.empty()) copyAttack = it->second[0].get();
                if (!releaseSet[(size_t)source].empty()) sourceRelease = releaseSet[(size_t)source][0].get();
            }
            if (sourceAttack == nullptr || copyAttack == nullptr || sourceRelease == nullptr) return;
            const double window = 0.1 * sourceRelease->sampleRate;
            const double sourceSustain = sustainRms(*sourceAttack), copySustain = sustainRms(*copyAttack);
            const double sourceStart = rmsOf(*sourceRelease, 0, (int64_t)window);
            if (sourceSustain < 1.0e-6 || copySustain < 1.0e-6 || sourceStart < 1.0e-6) return;
            for (auto& copy : copies)
            {
                if (copy == nullptr) continue;
                const double copyStart = rmsOf(*copy, 0, (int64_t)window);
                if (copyStart < 1.0e-6) continue;
                copy->releaseLevelGain = (float)juce::jlimit(0.25, 4.0, (sourceStart / sourceSustain) / (copyStart / copySustain));
            }
        };
        auto buildSet = [&](std::array<std::vector<std::unique_ptr<Sample>>, 128>& samplesByNote,
            std::map<int, std::vector<std::unique_ptr<Sample>>>& aliasFree, std::map<int, int>& aliasFreeOffset,
            bool releases, const juce::String& setTag)
        {
            // Build the missing ones without the lock (filtering takes a while).
            std::map<int, std::vector<std::unique_ptr<Sample>>> fresh;
            // The cache file of this set: copies whose parameters and source match are read from it.
            juce::File cacheFile;
            std::unique_ptr<juce::FileInputStream> cacheIn;
            std::map<int, CopyCacheRecord> cacheIndex;
            bool computedAny = false;
            int computedCount = 0;   // pipes computed (not read from the cache) in this build
            if (cacheOn)
            {
                cacheFile = copyCacheFileFor(copyCacheDir, *rank, setTag);
                if (cacheFile.existsAsFile())
                {
                    cacheIn = std::make_unique<juce::FileInputStream>(cacheFile);
                    if (cacheIn->openedOk()) cacheIndex = readCopyCacheIndex(*cacheIn, cacheParameters);
                    if (cacheIndex.empty()) cacheIn.reset();
                }
            }
            // Phase 1, this thread: which pipes are up to date, which the cache has
            // (read here, one file stream), which must be computed.
            const double crossfade = (double)copyLoopCrossfade.load(std::memory_order_relaxed);
            struct Task { int pipe, offset, source; std::vector<const Sample*> sources; };
            std::vector<Task> todo;
            for (const auto& [pipe, offset] : wanted)
            {
                const auto existing = aliasFreeOffset.find(pipe);
                if (existing != aliasFreeOffset.end() && existing->second == offset)
                {
                    // Up to date - unless the copies were made for another "formants follow".
                    bool sameFormant = true;
                    {
                        const juce::ScopedLock sl(controlLock);
                        const auto have = aliasFree.find(pipe);
                        if (method >= 1 && have != aliasFree.end() && !have->second.empty() && have->second.front() != nullptr
                            && have->second.front()->formantPct != formantPct[(size_t)pipe]) sameFormant = false;
                    }
                    if (sameFormant) continue;
                }
                const int source = pipe + offset;
                if (source < 0 || source >= 128) continue;
                // The source samples stay alive while antiAliasBuildLock is held (every
                // unload/reload takes it), so the filtering runs without the control lock.
                std::vector<const Sample*> sources;
                {
                    const juce::ScopedLock sl(controlLock);
                    for (const auto& variant : samplesByNote[(size_t)source])
                        if (variant != nullptr) sources.push_back(variant.get());
                }
                // The engine is closing, or a message-thread operation waits for the lock:
                // no more copies now (the finished ones are kept; the rest come with the next build).
                if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)) break;
                if (cacheIn != nullptr)
                {
                    std::vector<int64_t> sourceFrames;
                    for (const auto* s : sources) sourceFrames.push_back(s->numFrames);
                    const auto record = cacheIndex.find(pipe);
                    if (record != cacheIndex.end() && record->second.offset == offset && record->second.sourceFrames == sourceFrames
                        && record->second.formantPct == formantPct[(size_t)pipe])
                    {
                        auto copies = readCopyCacheVariants(*cacheIn, record->second);
                        if (!copies.empty())
                        {
                            copyCacheHits.fetch_add(1, std::memory_order_relaxed);
                            if (crossfade > 0.0) for (auto& copy : copies) applyCopyLoopCrossfade(*copy, crossfade);
                            for (auto& copy : copies) packIfEnabled(*copy);
                            fresh[pipe] = std::move(copies);
                            stop->copiesDone.fetch_add(1, std::memory_order_relaxed);
                            continue;
                        }
                    }
                }
                todo.push_back({ pipe, offset, source, std::move(sources) });
            }
            cacheIn.reset();
            // Phase 2, every core: the copies, one pipe per worker (a stop's pipes are
            // independent of each other; the whole organ used to be built on one core).
            std::vector<std::vector<std::unique_ptr<Sample>>> results(todo.size());
            std::atomic<int> computed{ 0 };
            parallelFor((int)todo.size(), [&](int index, int)
            {
                if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)) return;
                // The memory reserve (0.8.131's rule for the decoding) holds for the copies
                // too (0.8.180): a copy build that reached it stops, the pipes left play
                // the plain sample, the build is marked incomplete - before, it went on
                // until the system paged (Stralsund in float: 11 700 copies, 98 GB held).
                if (memoryShort.load(std::memory_order_relaxed)) return;
                {
                    juce::String why;
                    if (memoryShortNow(&why))
                    {
                        if (!memoryShort.exchange(true))
                            CrashHandler::note("Copy build of " + stop->name + " stopped for memory: " + why + " - the pipes left play the plain sample");
                        return;
                    }
                }
                const auto& task = todo[(size_t)index];
                std::vector<std::unique_ptr<Sample>> copies(task.sources.size());
                if (method >= 1)
                {
                    const int up = -task.offset;   // semitones the pipe lies above its source (negative: below)
                    const double lengthFactor = releases ? releaseLength : 1.0;
                    const float formantAmount = (float)formantPct[(size_t)task.pipe] / 100.0f;
                    const double baseHz = 440.0 * std::pow(2.0, (task.source - 69) / 12.0);   // the source pipe's nominal pitch
                    for (size_t v = 0; v < task.sources.size(); ++v)
                    {
                        if (buildsAborted.load(std::memory_order_acquire) || buildsInterrupted.load(std::memory_order_acquire)) return;
                        copies[v] = method == 3
                            ? makePitchShiftedCopy(*task.sources[v], up, releaseTiming, formantAmount, keepLoudness, baseHz, releases)
                            : makeResampledPitchCopy(*task.sources[v], up, lengthFactor, formantAmount, keepLoudness, baseHz, releases,
                                method, attackTiming, releaseTiming);
                        if (copies[v] != nullptr) copies[v]->formantPct = formantPct[(size_t)task.pipe];
                    }
                }
                else
                {
                    const double ratio = std::pow(2.0, -task.offset / 12.0);
                    for (size_t v = 0; v < task.sources.size(); ++v)
                    {
                        try { copies[v] = makeAntiAliasCopy(*task.sources[v], ratio); }
                        catch (...) { copies[v] = nullptr; }   // out of memory: no copy for this variant
                    }
                }
                copies.erase(std::remove(copies.begin(), copies.end(), nullptr), copies.end());
                if (copies.empty()) return;
                if (releases && method >= 1) releaseLevelGain(rank, task.pipe, task.source, samplesByNote, copies);
                if (crossfade > 0.0) for (auto& copy : copies) applyCopyLoopCrossfade(*copy, crossfade);
                for (auto& copy : copies) packIfEnabled(*copy);
                results[(size_t)index] = std::move(copies);
                computed.fetch_add(1, std::memory_order_relaxed);
                stop->copiesDone.fetch_add(1, std::memory_order_relaxed);
            }, juce::jmax(1, juce::SystemStats::getNumPhysicalCpus() - 1));   // one physical core left to the window and the mouse (0.8.185: the pointer stuttered through a build)
            for (size_t i = 0; i < todo.size(); ++i) if (!results[i].empty()) fresh[todo[i].pipe] = std::move(results[i]);
            computedCount = computed.load();
            computedAny = computedCount > 0;
            {
                // Swap in: obsolete copies are retired, not freed (a voice may read them).
                const juce::ScopedLock sl(controlLock);
                for (auto it = aliasFree.begin(); it != aliasFree.end();)
                {
                    const auto want = wanted.find(it->first);
                    const bool keep = want != wanted.end() && aliasFreeOffset[it->first] == want->second && fresh.count(it->first) == 0;
                    if (keep) { ++it; continue; }
                    for (auto& sample : it->second) rank->retiredSamples.push_back(std::move(sample));
                    aliasFreeOffset.erase(it->first);
                    it = aliasFree.erase(it);
                }
                for (auto& [pipe, copies] : fresh)
                {
                    aliasFree[pipe] = std::move(copies);
                    aliasFreeOffset[pipe] = wanted[pipe];
                }
            }
            // The cache file of this set, written whole (the copies just made and the
            // ones kept) whenever something was computed or the file is missing (the
            // cache was switched on after the copies were built). A build interrupted
            // by a message-thread operation writes too when it computed a fair number
            // of pipes - a long build must not lose its work to the next interruption.
            const bool interrupted = buildsInterrupted.load(std::memory_order_acquire);
            if (cacheOn && (computedAny || !cacheFile.existsAsFile())
                && !buildsAborted.load(std::memory_order_acquire) && (!interrupted || computedCount >= 20))
                if (writeCopyCacheFile(cacheFile, cacheParameters, wanted, samplesByNote, aliasFree))
                    copyCacheWrites.fetch_add(1, std::memory_order_relaxed);
        };
        buildSet(rank->samplesByNote, rank->aliasFree, rank->aliasFreeOffset, false, "plain");
        for (size_t i = 0; i < layerCount && i < rank->velocityLayers.size(); ++i)
        {
            auto& layer = rank->velocityLayers[i];
            buildSet(layer.samplesByNote, layer.aliasFree, layer.aliasFreeOffset, false, "layer_" + juce::String(layer.maxVelocity));
        }
        // Pitch shift keeps the release lengths too: the releases of filled pipes get
        // their own shifted copies (resampling plays them resampled, no copies).
        if (method >= 1)
            for (size_t i = 0; i < releaseLayerCount && i < rank->releaseLayers.size(); ++i)
            {
                auto& layer = rank->releaseLayers[i];
                buildSet(layer.samplesByNote, layer.aliasFree, layer.aliasFreeOffset, true,
                    "release_" + juce::String(layer.maxHoldMs) + "_" + juce::String(layer.maxVelocity));
            }
    }
    queuedReset.completed = !buildsAborted.load(std::memory_order_acquire) && !buildsInterrupted.load(std::memory_order_acquire) && !memoryShort.load(std::memory_order_relaxed);
}

void SamplerEngine::refillStopPipes(Stop* stop, bool keepExisting)
{
    if (stop == nullptr) return;
    // The pipes the stop's routes can ask for: the compass of its own division
    // and of every division it is transmitted into, shifted by the transposition.
    int needLow = 128, needHigh = -1;
    auto extend = [&](int manual, int shift)
    {
        manual = juce::jlimit(0, maxManuals - 1, manual);
        const int low = juce::jlimit(0, 127, compassLow[(size_t)manual].load(std::memory_order_relaxed) + shift);
        const int high = juce::jlimit(0, 127, compassHigh[(size_t)manual].load(std::memory_order_relaxed) + shift);
        needLow = juce::jmin(needLow, low);
        needHigh = juce::jmax(needHigh, high);
    };
    extend(stop->primaryManual, 0);
    for (const auto& t : stop->transmissions) extend(t.manual, t.shift);
    if (stop->groupMasterPtr != nullptr)   // a stop folder's child sounds through its master's transmissions too
        for (const auto& t : stop->groupMasterPtr->transmissions) extend(t.manual, t.shift);
    if (needHigh < needLow) { needLow = 0; needHigh = 127; }
    const int groupSize = fillGroupSize.load(std::memory_order_relaxed);
    // Pipes switched off in the stop's voicing are not filled - unless a
    // transmission may still want them (a transmission switches its keys off on its own).
    const bool hasTransmissions = !stop->transmissions.empty()
        || (stop->groupMasterPtr != nullptr && !stop->groupMasterPtr->transmissions.empty());
    const std::bitset<128>* disabled = hasTransmissions ? nullptr : &stop->pipeDisabled;
    computeFilledPipes(stop, groupSize, needLow, needHigh, disabled, keepExisting);
    for (auto& rank : stop->ranks) computeFilledPipes(rank.get(), groupSize, needLow, needHigh, disabled, keepExisting);
}

void SamplerEngine::computeFilledPipes(Stop* rank, int groupSize, int needLow, int needHigh, const std::bitset<128>* disabledPipes, bool keepExisting)
{
    // A pipe filled before keeps its source (keepExisting): a wider compass (a
    // new transmission) would otherwise regroup the existing fills - the
    // "remainder" rule depends on how many pipes are needed - and every copy
    // already built for them would be built again. Only the pipes new to the
    // map get the rule. A change of the group size starts afresh.
    const std::bitset<128> before = rank->filled;
    const std::array<int8_t, 128> beforeOffset = rank->fillOffset;
    // A faulty pipe (its sample beyond the retune limit, 0.8.165) is a missing one here.
    auto has = [rank](int n) { return rank->hasNote(n) && !(n >= 0 && n < 128 && rank->faultyPipes.test((size_t)n)); };
    rank->filled.reset();
    rank->fillOffset.fill(0);
    struct KeepExisting
    {
        Stop* rank; bool keep; const std::bitset<128>& before; const std::array<int8_t, 128>& beforeOffset;
        ~KeepExisting()
        {
            if (!keep) return;
            for (int pipe = 0; pipe < 128; ++pipe)
            {
                if (!rank->filled.test((size_t)pipe) || !before.test((size_t)pipe)) continue;
                const int source = pipe + beforeOffset[(size_t)pipe];
                if (source >= 0 && source < 128 && source != pipe && rank->hasNote(source) && !rank->faultyPipes.test((size_t)source)) rank->fillOffset[(size_t)pipe] = beforeOffset[(size_t)pipe];
            }
        }
    } keepExistingFills{ rank, keepExisting, before, beforeOffset };
    std::vector<int> real;
    for (int note = 0; note < 128; ++note) if (has(note)) real.push_back(note);
    if (real.empty()) return;
    const int count = (int)real.size();
    const int lowest = real.front(), highest = real.back();
    needLow = juce::jlimit(0, 127, needLow);
    needHigh = juce::jlimit(needLow, 127, needHigh);
    auto realAtOrBelow = [&](int note)
    {
        for (int n = juce::jmin(127, note); n >= 0; --n) if (has(n)) return n;
        return -1;
    };
    auto realAtOrAbove = [&](int note)
    {
        for (int n = juce::jmax(0, note); n < 128; ++n) if (has(n)) return n;
        return -1;
    };
    auto assign = [&](int pipe, int source)
    {
        if (disabledPipes != nullptr && disabledPipes->test((size_t)pipe)) return;   // switched off: stays silent
        if (source < 0 || source > 127 || !has(source) || source == pipe) return;
        rank->filled.set((size_t)pipe);
        rank->fillOffset[(size_t)pipe] = (int8_t)juce::jlimit(-127, 127, source - pipe);
    };
    // The group: the g pipes nearest to the missing ones, transposed by g
    // semitones (2g, 3g ... further away). With g = 12 every missing pipe is the
    // octave transposition of a real one; with g = 1 the edge pipe serves alone.
    // A last group smaller than g (n missing pipes are not a multiple of g) takes
    // the outermost pipes with the smallest transposition possible: 17 missing
    // pipes above a rank with g = 12 are 12 octave transpositions and then the 5
    // highest real pipes shifted by 17, not by 24.
    // 'distance' 1.. from the edge, n = pipes needed beyond it; returns the shift.
    auto shiftFor = [](int distance, int n, int g)
    {
        const int fullBlocks = n / g;
        if (distance <= fullBlocks * g) return ((distance + g - 1) / g) * g;   // whole groups
        return n;                                                              // the remainder: outermost pipes, shift n
    };
    const int g = juce::jlimit(1, count, groupSize);
    // Below the rank: the lowest pipes shifted down, only as far as needed.
    if (needLow < lowest)
    {
        const int n = lowest - needLow;
        for (int pipe = lowest - 1; pipe >= needLow; --pipe)
        {
            int source = pipe + shiftFor(lowest - pipe, n, g);
            if (!has(source)) source = realAtOrAbove(source);   // a gap right at the edge
            assign(pipe, source);
        }
    }
    // Above the rank: the highest pipes shifted up, only as far as needed.
    if (needHigh > highest)
    {
        const int n = needHigh - highest;
        for (int pipe = highest + 1; pipe <= needHigh; ++pipe)
        {
            int source = pipe - shiftFor(pipe - highest, n, g);
            if (!has(source)) source = realAtOrBelow(source);
            assign(pipe, source);
        }
    }
    // Gaps inside the rank: the lower half from the pipes below the gap, the
    // upper half from those above (the same groups and remainder rule). An odd
    // gap gives its middle pipe to the side that has more real pipes; with equal
    // numbers to the lower side.
    for (int i = 0; i + 1 < count; ++i)
    {
        const int a = real[(size_t)i] + 1, b = real[(size_t)i + 1] - 1;
        if (a > b) continue;
        const int n = b - a + 1;
        const int below = i + 1, above = count - (i + 1);
        int lower = n / 2;
        if ((n % 2) == 1 && below >= above) ++lower;
        const int upper = n - lower;
        const int gl = juce::jmax(1, juce::jmin(g, below)), gu = juce::jmax(1, juce::jmin(g, above));
        for (int pipe = a; pipe < a + lower; ++pipe)
        {
            int source = pipe - shiftFor(pipe - a + 1, lower, gl);
            if (!has(source)) source = realAtOrBelow(source);
            assign(pipe, source);
        }
        for (int pipe = a + lower; pipe <= b; ++pipe)
        {
            int source = pipe + shiftFor(b - pipe + 1, upper, gu);
            if (!has(source)) source = realAtOrAbove(source);
            assign(pipe, source);
        }
    }
}

void SamplerEngine::setManualTremulant(int manualIndex, bool enabled, float rateHz, float depthCents,
    float rankDriftDegrees)
{
    if (manualIndex < 0 || manualIndex >= maxManuals) return;
    auto& t = impl->trem[(size_t)manualIndex];
    t.enabled.store(enabled);
    t.rateHz.store(juce::jlimit(2.0f, 11.0f, rateHz));
    t.depthCents.store(juce::jlimit(0.0f, 50.0f, depthCents));
    t.rankDriftDegrees.store(juce::jlimit(0.0f, 360.0f, rankDriftDegrees));
}

//------------------------------------------------------------------------------
std::vector<juce::String> SamplerEngine::getPerspectivesForManual(int manualIndex) const
{
    std::vector<juce::String> result;
    auto addUnique = [&result](const juce::String& name)
    {
        if (std::none_of(result.begin(), result.end(), [&](const auto& existing)
            { return existing.equalsIgnoreCase(name); })) result.push_back(name);
    };
    for (auto* stop : stops)
        if (stop->primaryManual == manualIndex && !stop->container)   // a stop folder has no samples: its "default" is a placeholder, no route
        {
            addUnique(stop->perspective);
            for (const auto& rank : stop->ranks) addUnique(rank->perspective);
        }
    if (result.empty()) result.push_back("default");
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.compareNatural(b) < 0; });
    result.push_back(auxiliaryPerspectiveName());   // the division's downmix route, always last
    return result;
}

void SamplerEngine::setStopEnabled(Stop* stop, bool enabled)
{
    if (stop == nullptr) return;
    const juce::ScopedLock sl(controlLock);
    if (stop->enabled == enabled) return;
    stop->enabled = enabled;
    impl->reclaimFinished();

    if (!enabled)
    {
        // Closing the stop: its pipes that no key holds through another route
        // (a transmission, a coupler) are released; the others keep sounding.
        impl->releaseUnheldPipes(stop);
        return;
    }

    for (int key = 0; key < kSourceKeys; ++key)
    {
        if (impl->heldVelocity[(size_t)key] < 0) continue;
        const int manualIndex = keyManual(key);
        if (manualIndex >= 0 && stop->primaryManual != manualIndex) continue;
        impl->startNote(keyNote(key), impl->heldVelocity[(size_t)key], manualIndex, key);
    }
}

void SamplerEngine::setRankEnabled(Stop* logicalStop, Stop* rank, bool enabled)
{
    if (logicalStop == nullptr || rank == nullptr) return;
    const juce::ScopedLock sl(controlLock);
    rank->rankEnabled = enabled;
    impl->reclaimFinished();
    if (!enabled)
    {
        rank->speaking.reset();
        for (const uint32_t idx : impl->controlActive)
        {
            Voice& v = impl->voices[idx];
            if (v.stop == logicalStop && v.rank == rank) impl->sendCommand(v, CmdKill);
        }
        return;
    }
    if (!logicalStop->enabled) return;
    for (int key = 0; key < kSourceKeys; ++key)
        if (impl->heldVelocity[(size_t)key] >= 0)
            impl->startNote(keyNote(key), impl->heldVelocity[(size_t)key], keyManual(key), key);
}

void SamplerEngine::setTransmissionEnabled(Stop* stop, int index, bool enabled)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    const juce::ScopedLock sl(controlLock);
    auto& transmission = stop->transmissions[(size_t)index];
    if (transmission.enabled == enabled) return;
    transmission.enabled = enabled;
    impl->reclaimFinished();
    if (!enabled)
    {
        // Switching a transmission off: pipes that are still held through the
        // stop's own division or another transmission keep sounding, the rest
        // get their normal release. A stop folder's children went through this
        // transmission as well.
        impl->releaseUnheldPipes(stop);
        for (auto* child : stops)
            if (child->groupMasterPtr == stop) impl->releaseUnheldPipes(child);
        return;
    }
    const int manualIndex = transmission.manual;
    for (int key = 0; key < kSourceKeys; ++key)
        if (impl->heldVelocity[(size_t)key] >= 0 && keyManual(key) == manualIndex)
            impl->startNote(keyNote(key), impl->heldVelocity[(size_t)key], manualIndex, key);
}

void SamplerEngine::setTransmissionShift(Stop* stop, int index, int semitones)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    semitones = juce::jlimit(-Stop::Transmission::maxShift, Stop::Transmission::maxShift, semitones);
    {
        const juce::ScopedLock sl(controlLock);
        auto& transmission = stop->transmissions[(size_t)index];
        if (transmission.shift == semitones) return;
        // Like switching the transmission off and on again with the new shift: the
        // held keys release their old pipes (unless another route holds them) and
        // start the new ones.
        const bool wasEnabled = transmission.enabled;
        if (wasEnabled)
        {
            transmission.enabled = false;
            impl->reclaimFinished();
            impl->releaseUnheldPipes(stop);
        }
        transmission.shift = semitones;
        refillStopPipes(stop);   // the shifted compass needs other pipes
        if (wasEnabled)
        {
            transmission.enabled = true;
            for (int key = 0; key < kSourceKeys; ++key)
                if (impl->heldVelocity[(size_t)key] >= 0 && keyManual(key) == transmission.manual)
                    impl->startNote(keyNote(key), impl->heldVelocity[(size_t)key], transmission.manual, key);
        }
    }
    buildAntiAliasCopies(stop, true);   // filtered without the lock; held keys use the plain source until then
}

int SamplerEngine::addTransmission(Stop* stop, int manual, int shift)
{
    if (stop == nullptr || manual < 0 || manual >= maxManuals) return -1;
    int index = -1;
    {
        const juce::ScopedLock sl(controlLock);
        Stop::Transmission transmission;
        transmission.manual = manual;
        transmission.shift = juce::jlimit(-Stop::Transmission::maxShift, Stop::Transmission::maxShift, shift);
        transmission.enabled = false;
        stop->transmissions.push_back(transmission);
        refillStopPipes(stop);
        index = (int)stop->transmissions.size() - 1;
    }
    buildAntiAliasCopies(stop, true);
    return index;
}

void SamplerEngine::removeTransmission(Stop* stop, int index)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size()) return;
    setTransmissionEnabled(stop, index, false);
    {
        const juce::ScopedLock sl(controlLock);
        stop->transmissions.erase(stop->transmissions.begin() + index);
        refillStopPipes(stop);
    }
    buildAntiAliasCopies(stop, true);
}

void SamplerEngine::replaceTransmissions(Stop* stop, std::vector<Stop::Transmission> list)
{
    if (stop == nullptr) return;
    for (int i = 0; i < (int)stop->transmissions.size(); ++i) setTransmissionEnabled(stop, i, false);
    std::vector<bool> wanted;
    {
        const juce::ScopedLock sl(controlLock);
        wanted.reserve(list.size());
        for (auto& t : list)
        {
            t.manual = juce::jlimit(0, maxManuals - 1, t.manual);
            t.shift = juce::jlimit(-Stop::Transmission::maxShift, Stop::Transmission::maxShift, t.shift);
            wanted.push_back(t.enabled);
            t.enabled = false;
        }
        stop->transmissions = std::move(list);
        refillStopPipes(stop);
    }
    buildAntiAliasCopies(stop, true);
    // The entries flagged enabled are switched on through the normal path so
    // that held keys start sounding.
    for (int i = 0; i < (int)wanted.size(); ++i)
        if (wanted[(size_t)i]) setTransmissionEnabled(stop, i, true);
}

//------------------------------------------------------------------------------
// Footages and transpositions
double SamplerEngine::pureDeviationCents(int semitones)
{
    // Harmonics m * 2^k with m in {1, 3, 5, 7, 9}: the footage offsets an organ
    // rank can have. The pure ratio's distance from the tempered semitone count.
    static const int harmonics[]{ 1, 3, 5, 7, 9 };
    for (const int m : harmonics)
        for (int k = -7; k <= 7; ++k)
        {
            const double cents = 1200.0 * std::log2((double)m * std::pow(2.0, (double)k));
            if (juce::roundToInt(cents / 100.0) == semitones) return cents - 100.0 * semitones;
        }
    // Not a harmonic footage: the pure (just) interval of that semitone class
    // (16/15, 9/8, 6/5, 5/4, 4/3, 7/5, 3/2, 8/5, 5/3, 9/5, 15/8), octaves added
    // (0.8.159; tempered before - the transposition is meant pure).
    const int octaves = (int)std::floor(semitones / 12.0);
    const int cls = semitones - 12 * octaves;
    return 1200.0 * std::log2((double)justIntervalNumerator(cls) / (double)justIntervalDenominator(cls)) + 1200.0 * octaves - 100.0 * semitones;
}

juce::String SamplerEngine::footageLabel(int semitones)
{
    return footageName(semitones).upToFirstOccurrenceOf(" (", false, false);
}

juce::String SamplerEngine::folderStopAlias(const juce::File& folder) const
{
    // The bare folder name: no parent folders ("TA - fletna 8'" -> "fletna 8'"),
    // the perspective prefix off, underscores to spaces, a footage at the end
    // with its foot mark. Without a footage in the name, the stop's display
    // name adds the measured one (the first sounding harmonic of a mixture).
    std::vector<juce::String> perspectives;
    {
        const juce::ScopedLock sl(controlLock);
        perspectives = perspectivePatterns;
    }
    juce::String leaf = stripPerspectivePrefix(perspectives, folder.getFileName(), nullptr);
    if (leaf.trim().isEmpty()) leaf = folder.getFileName();
    auto alias = footageDisplayName(displayFolderName(leaf).trim());
    // A capital first letter: the labels of a grid look even that way ("fletna 8'" -> "Fletna 8'").
    if (alias.isNotEmpty()) alias = alias.substring(0, 1).toUpperCase() + alias.substring(1);
    return StopNames::capitaliseAfterDivisionPrefix(alias);   // "P. trombone 16" -> "P. Trombone 16"
}

namespace
{
    // "113", "135", "223", "513", "1023": a mutation footage written without
    // spaces and slash - the last digit is the denominator (3, 5, 7 or 9), the
    // one before it the numerator, the rest the whole feet. Accepted only when
    // the result is a harmonic footage (8' * 2^k / 3, 5, 7 or 9), so "1 1/3",
    // "1 3/5", "2 2/3", "5 1/3", "10 2/3", "1 1/7", "2 2/7", "1 7/9" ... and a
    // two-digit "89" (8/9') or "23" (2/3') only where no plain footage fits.
    bool compactFootage(const juce::String& digits, double& feet)
    {
        if (digits.length() < 2 || digits.length() > 4) return false;
        const int den = digits[digits.length() - 1] - '0';
        const int num = digits[digits.length() - 2] - '0';
        if ((den != 3 && den != 5 && den != 7 && den != 9) || num < 1 || num >= den) return false;
        const juce::String wholePart = digits.dropLastCharacters(2);
        const double whole = wholePart.isEmpty() ? 0.0 : wholePart.getDoubleValue();
        const double value = whole + (double)num / (double)den;
        for (int m : { 3, 5, 7, 9 })
            for (int k = -4; k <= 4; ++k)
                if (std::abs(value - 8.0 * std::pow(2.0, k) / m) < 0.005) { feet = value; return true; }
        return false;
    }

    // A footage written as a bare number at the end of a name ("Fletna 8",
    // "Kvinta 2 2/3", "Larigot 113"): the character range and the length in feet;
    // 'compact' tells that the digits were the spaceless mutation form.
    bool findTrailingFootage(const juce::String& name, int& start, int& end, double& feet, bool* compact = nullptr)
    {
        const juce::String trimmed = name.trimEnd();
        int j = trimmed.length() - 1;
        if (j < 0 || !juce::CharacterFunctions::isDigit(trimmed[j])) return false;
        const int tokenEnd = j + 1;
        const int lastDigitsEnd = j;
        while (j >= 0 && juce::CharacterFunctions::isDigit(trimmed[j])) --j;
        juce::String whole, numerator, denominator;
        if (j >= 0 && trimmed[j] == '/')
        {
            denominator = trimmed.substring(j + 1, lastDigitsEnd + 1);
            --j;
            const int numEnd = j;
            if (j < 0 || !juce::CharacterFunctions::isDigit(trimmed[j])) return false;
            while (j >= 0 && juce::CharacterFunctions::isDigit(trimmed[j])) --j;
            numerator = trimmed.substring(j + 1, numEnd + 1);
            if (j >= 1 && trimmed[j] == ' ' && juce::CharacterFunctions::isDigit(trimmed[j - 1]))
            {
                --j;
                const int wholeEnd = j;
                while (j >= 0 && juce::CharacterFunctions::isDigit(trimmed[j])) --j;
                whole = trimmed.substring(j + 1, wholeEnd + 1);
            }
        }
        else whole = trimmed.substring(j + 1, lastDigitsEnd + 1);
        if (j < 0 || trimmed[j] != ' ') return false;   // "Mixtur3" or a bare number: not a footage
        double value = whole.getDoubleValue();
        if (denominator.isNotEmpty() && denominator.getDoubleValue() > 0.0)
            value += numerator.getDoubleValue() / denominator.getDoubleValue();
        bool isCompact = false;
        if (denominator.isEmpty() && (whole.length() >= 3 || value > 64.0))
        {
            // "113" is no 113-foot pipe: the spaceless mutation form, 1 1/3'.
            double compactFeet = 0.0;
            if (!compactFootage(whole, compactFeet)) return false;
            value = compactFeet;
            isCompact = true;
        }
        if (value <= 0.0 || value > 64.0) return false;
        start = j + 1;
        end = tokenEnd;
        feet = value;
        if (compact != nullptr) *compact = isCompact;
        return true;
    }

    // Locates the footage written in a stop name ("Quinte 2 2/3'", "Principal 8’",
    // "Octave 4 ft", "Fletna 8"): the character range and the length in feet.
    bool findFootageToken(const juce::String& name, int& start, int& end, double& feet, bool* compact = nullptr)
    {
        if (compact != nullptr) *compact = false;
        const int length = name.length();
        for (int i = 0; i < length; ++i)
        {
            const juce::juce_wchar c = name[i];
            const bool apostrophe = c == '\'' || c == 0x2019 || c == 0x2032;   // ' ’ ′
            bool feetWord = false;
            if (!apostrophe && (c == 'f' || c == 'F'))
            {
                // "ft" / "ft." / "Fuss" after a number and a space
                const juce::String rest = name.substring(i).toLowerCase();
                feetWord = (rest.startsWith("ft") || rest.startsWith("fuss") || rest.startsWith("fuß"))
                    && i > 0 && (name[i - 1] == ' ' || juce::CharacterFunctions::isDigit(name[i - 1]));
            }
            if (!apostrophe && !feetWord) continue;
            // Walk back over "[whole] [num/den]" allowing single spaces between the parts.
            int j = i - 1;
            if (feetWord) while (j >= 0 && name[j] == ' ') --j;
            const int tokenEnd = j + 1;
            if (j < 0 || !juce::CharacterFunctions::isDigit(name[j])) continue;
            int digitsEnd = j;
            while (j >= 0 && juce::CharacterFunctions::isDigit(name[j])) --j;
            juce::String denominator, numerator, whole;
            if (j >= 0 && name[j] == '/')
            {
                denominator = name.substring(j + 1, digitsEnd + 1);
                --j;
                const int numEnd = j;
                if (j < 0 || !juce::CharacterFunctions::isDigit(name[j])) continue;
                while (j >= 0 && juce::CharacterFunctions::isDigit(name[j])) --j;
                numerator = name.substring(j + 1, numEnd + 1);
                if (j >= 0 && name[j] == ' ' && j - 1 >= 0 && juce::CharacterFunctions::isDigit(name[j - 1]))
                {
                    --j;
                    const int wholeEnd = j;
                    while (j >= 0 && juce::CharacterFunctions::isDigit(name[j])) --j;
                    whole = name.substring(j + 1, wholeEnd + 1);
                }
            }
            else whole = name.substring(j + 1, digitsEnd + 1);
            double value = whole.getDoubleValue();
            if (denominator.isNotEmpty() && denominator.getDoubleValue() > 0.0)
                value += numerator.getDoubleValue() / denominator.getDoubleValue();
            if (value <= 0.0) continue;
            start = j + 1;
            end = apostrophe ? i + 1 : tokenEnd;   // "8 ft": only the number is replaced
            feet = value;
            return true;
        }
        return findTrailingFootage(name, start, end, feet, compact);
    }
}

bool SamplerEngine::footageFromName(const juce::String& name, int& semitones)
{
    int start = 0, end = 0;
    double feet = 0.0;
    if (!findFootageToken(name, start, end, feet)) return false;
    // The organ builders' shorthand (0.8.178, the user's rule): "3" is the quint
    // 2 2/3', "6" the 5 1/3', "12" the 10 2/3' and "1 1/2" the 1 1/3' - no rank of
    // 3', 6', 12' or 1 1/2' exists ("CH RohrNasat 3" of the Aeolian-Skinner set was
    // read as 3' and transposed by -14 against its files sounding 1 1/3').
    if (feet == 3.0) feet = 8.0 / 3.0;
    else if (feet == 6.0) feet = 16.0 / 3.0;
    else if (feet == 12.0) feet = 32.0 / 3.0;
    else if (feet == 1.5) feet = 4.0 / 3.0;
    semitones = juce::roundToInt(12.0 * std::log2(8.0 / feet));
    return true;
}

int SamplerEngine::stopFootageSemitones(const Stop* stop)
{
    if (stop == nullptr) return 0;
    if (stop->footageKnown) return stop->footageSemitones + stop->sampleOffset;   // the files' real footage, transposed = what sounds on the keys
    int semitones = 0;
    return footageFromName(stop->name, semitones) ? semitones : 0;
}

int SamplerEngine::untransposedFootageSemitones(const Stop* stop)
{
    // Measured: the real footage of the files on their own numbers. Not measured:
    // the name is taken as the sounding footage, the transposition taken off.
    return stop == nullptr ? 0 : stopFootageSemitones(stop) - stop->sampleOffset;
}

juce::String SamplerEngine::nameWithoutFootage(const juce::String& name)
{
    int start = 0, end = 0;
    double feet = 0.0;
    if (!findFootageToken(name, start, end, feet)) return name;
    return (name.substring(0, start).trimEnd() + " " + name.substring(end).trimStart()).trim();
}


juce::String footageDisplayName(const juce::String& name)
{
    int start = 0, end = 0;
    double feet = 0.0;
    bool compact = false;
    if (!findFootageToken(name, start, end, feet, &compact)) return name;
    const juce::String token = name.substring(start, end);
    if (token.isEmpty() || !juce::CharacterFunctions::isDigit(token[token.length() - 1])) return name;   // has its mark
    // "Larigot 113" reads "Larigot 1 1/3'": the odd number goes, the real footage comes.
    if (compact) return name.substring(0, start) + SamplerEngine::footageLabel(juce::roundToInt(12.0 * std::log2(8.0 / feet))) + name.substring(end);
    return name.substring(0, end) + "'" + name.substring(end);
}

juce::String romanNumeral(int n)
{
    if (n <= 0) return "0";
    static const std::pair<int, const char*> parts[]{ { 1000, "M" }, { 900, "CM" }, { 500, "D" }, { 400, "CD" }, { 100, "C" }, { 90, "XC" },
        { 50, "L" }, { 40, "XL" }, { 10, "X" }, { 9, "IX" }, { 5, "V" }, { 4, "IV" }, { 1, "I" } };
    juce::String result;
    for (const auto& [value, text] : parts)
        while (n >= value) { result << text; n -= value; }
    return result;
}

juce::String Stop::displayName() const
{
    // An alias gets its foot mark like a name ("I. Quintadeen 8" reads "I. Quintadeen 8'").
    const juce::String base = footageDisplayName(alias.isNotEmpty() ? alias : name);
    int written = 0;
    if (SamplerEngine::footageFromName(base, written))   // the name (or alias) says the footage itself ...
    {
        // ... and a transposed stop shows the footage it really sounds at (0.8.158):
        // the measured one when the samples carry a pitch, else the name's plus the
        // transposition. Untransposed stops keep the name's footage as ever.
        if (sampleOffset == 0) return base;
        const Stop* measured = footageKnown ? this : nullptr;
        if (measured == nullptr)
            for (const auto& rank : ranks)
                if (rank->footageKnown) { measured = rank.get(); break; }
        return SamplerEngine::nameWithFootage(base, measured != nullptr ? measured->footageSemitones + sampleOffset : written + sampleOffset);
    }
    // No footage in the caption: the one measured from the lowest pipe's smpl pitch.
    const Stop* measured = footageKnown ? this : nullptr;
    if (measured == nullptr)
        for (const auto& rank : ranks)
            if (rank->footageKnown) { measured = rank.get(); break; }
    if (measured == nullptr) return base;
    return base + " " + SamplerEngine::footageLabel(measured->lowestFootageSemitones + sampleOffset);   // the real footage, transposed
}

int SamplerEngine::displayedFootageSemitones(const Stop* stop)
{
    // The same base as the Transposition box of a transmission (the measured
    // smpl footage when known, else the name): a "Quinte 3" whose pipes measure
    // 2 2/3' shifted +5 reads 2', not 2 2/7' (3' + 5 is no harmonic footage).
    return stopFootageSemitones(stop);
}

juce::String SamplerEngine::transposedStopName(const Stop* stop, int shift)
{
    if (stop == nullptr) return {};
    const juce::String name = stop->displayName();   // an aliased stop passes its alias on; the footage carries its mark
    if (shift == 0) return name;
    // The transposition counts from the footage the Transposition box counts from.
    return nameWithFootage(name, displayedFootageSemitones(stop) + shift);
}

juce::String SamplerEngine::nameWithFootage(const juce::String& name, int semitones)
{
    const juce::String newFootage = footageLabel(semitones);   // always with the foot mark
    // A division prefix ("4. Tuba 8", "P. tuba 4", "Rec: Hautbois 8") carries no
    // footage: the search starts after it (0.8.142: "4. Tuba 8" shifted an
    // octave came out as "2'. Tuba 8" - the "4." taken for the footage).
    int skip = 0;
    {
        int i = 0;
        while (i < name.length() && name[i] == ' ') ++i;
        int j = i;
        while (j < name.length() && name[j] != ' ' && name[j] != '.' && name[j] != ':') ++j;
        if (j < name.length() && (name[j] == '.' || name[j] == ':') && j - i >= 1 && j - i <= 5 && j + 1 < name.length() && name[j + 1] == ' ') skip = j + 1;
    }
    int start = 0, end = 0;
    double feet = 0.0;
    if (findFootageToken(name.substring(skip), start, end, feet))
        return name.substring(0, skip + start) + newFootage + name.substring(skip + end);
    return name + " " + newFootage;
}

juce::String SamplerEngine::transmissionSuffix(const Stop* stop, const Stop::Transmission& t)
{
    return stop != nullptr && t.manual == stop->primaryManual ? " (Ext.)" : " (Trsm.)";
}

juce::String SamplerEngine::transmissionCaption(const Stop* stop, const Stop::Transmission& t)
{
    // The transmission's own alias when it has one (a Hauptwerk import gives the
    // definition's stop name, "P. tuba 4"), else the owner's name with its footage
    // shifted (0.8.142: the alias was never shown on the button).
    return (t.alias.isNotEmpty() ? t.alias : transposedStopName(stop, t.shift)) + transmissionSuffix(stop, t);
}

//------------------------------------------------------------------------------
void SamplerEngine::setOneShotPatterns(const std::vector<juce::String>& patterns)
{
    oneShotPatterns = patterns;
    updateOneShotFlags();
}

bool SamplerEngine::folderNameMatchesPattern(const juce::String& folderName, const juce::String& patternIn)
{
    // A pattern without a star matches anywhere in the name ("trem"); a star at
    // the start anchors the pattern to the end of the name ("*_tr": names ending
    // in "_tr"), a star at the end to its start ("dir-*"); stars at both ends
    // match anywhere. Case does not matter; underscores and the like do count.
    const auto pattern = patternIn.trim();
    if (pattern.isEmpty()) return false;
    const bool starFront = pattern.startsWithChar('*'), starBack = pattern.endsWithChar('*');
    const auto core = pattern.trimCharactersAtStart("*").trimCharactersAtEnd("*");
    if (core.isEmpty()) return false;
    if (starFront && !starBack) return folderName.endsWithIgnoreCase(core);
    if (starBack && !starFront) return folderName.startsWithIgnoreCase(core);
    return folderName.containsIgnoreCase(core);
}

void SamplerEngine::updateOneShotFlags()
{
    const juce::ScopedLock sl(controlLock);
    for (auto* stop : stops)
    {
        bool isOneShot = stop->folder.getFileName().containsIgnoreCase("chiff");
        for (const auto& pattern : oneShotPatterns)
            if (folderNameMatchesPattern(stop->folder.getFileName(), pattern)) isOneShot = true;
        // The voicing's own switch (0.8.158) overrides the patterns.
        if (stop->oneShotManual >= 0) isOneShot = stop->oneShotManual == 1;
        stop->isOneShot = isOneShot;
    }
}

void SamplerEngine::setIgnoredPatterns(const std::vector<juce::String>& patterns)
{
    ignoredPatterns = patterns;
}

bool SamplerEngine::isIgnoredFolderName(const juce::String& folderName) const
{
    for (const auto& pattern : ignoredPatterns)
        if (folderNameMatchesPattern(folderName, pattern)) return true;
    return false;
}

int SamplerEngine::extractMidiFromFilename(const juce::String& name)
{
    juce::String digits;
    for (int i = 0; i < name.length(); ++i)
    {
        if (juce::CharacterFunctions::isDigit(name[i])) digits += name[i];
        else break;
    }
    if (digits.isEmpty()) return -1;
    const int note = digits.getIntValue();
    return note < 0 || note > 127 ? -1 : note;
}

int SamplerEngine::extractVelocityLayerFromFilename(const juce::String& filename)
{
    // "060_070", "060-C4_070", "060-C_070-2"? No: the layer is the very last part,
    // "_" followed by two or three digits and nothing else; one digit ("036_1")
    // stays a round-robin variant, not a layer.
    const int dot = filename.lastIndexOfChar('.');
    const juce::String base = dot > 0 ? filename.substring(0, dot) : filename;
    const int underscore = base.lastIndexOfChar('_');
    if (underscore < 0 || underscore == 0) return -1;
    const juce::String digits = base.substring(underscore + 1);
    if (digits.length() < 2 || digits.length() > 3) return -1;
    for (int i = 0; i < digits.length(); ++i)
        if (!juce::CharacterFunctions::isDigit(digits[i])) return -1;
    const int value = digits.getIntValue();
    return value >= 1 && value <= 127 ? value : -1;
}

int SamplerEngine::extractReleaseHoldMs(const juce::String& folderName)
{
    if (!folderName.startsWithIgnoreCase("rel")) return -1;
    int position = 3;
    while (position < folderName.length() && juce::CharacterFunctions::isWhitespace(folderName[position])) ++position;
    juce::String digits;
    while (position < folderName.length() && juce::CharacterFunctions::isDigit(folderName[position]))
        digits += folderName[position++];
    while (position < folderName.length() && juce::CharacterFunctions::isWhitespace(folderName[position])) ++position;
    if (digits.isEmpty() || position != folderName.length()) return -1;
    const auto value = digits.getLargeIntValue();
    if (value < 0 || value > (juce::int64)(std::numeric_limits<int>::max)()) return -1;
    return (int)value;
}

//==============================================================================
// Loading
//==============================================================================
namespace
{
    // Every sample is kept at the base rate of 48 kHz. A set recorded at 96 or
    // 44.1 kHz is resampled once, when it is loaded, with a Kaiser-windowed sinc
    // kernel: 32 zero crossings on each side, beta 9 (about 90 dB of stop-band
    // rejection, flat to 20 kHz), evaluated from a table with 512 points per
    // input frame (linear interpolation between them: kernel error below -100 dB).
    // Downsampling lowers the cut-off to the new Nyquist limit and widens the
    // kernel in proportion, so nothing above 24 kHz folds back; upsampling keeps
    // the source's own band. Loop points follow the new frame grid.
    constexpr double kBaseSampleRate = 48000.0;

    // Polyphase form: the ratio in/out = P/Q is rational (48000/44100 = 160/147),
    // so only Q different kernel phases occur; each is a fixed tap vector and the
    // work per output frame is one plain dot product (2:1 needs a single phase).
    struct SincResampler
    {
        int64_t P = 1, Q = 1;              // input frames per output frame = P/Q (reduced)
        int halfWidth = 32, taps = 65;     // per phase: input frames on each side of the centre, and their count
        std::vector<float> coefficients;   // Q phases x taps

        SincResampler(double inputRate, double outputRate)
        {
            const int64_t in = juce::jmax<int64_t>(1, (int64_t)std::llround(inputRate));
            const int64_t out = juce::jmax<int64_t>(1, (int64_t)std::llround(outputRate));
            const int64_t g = std::gcd(in, out);
            P = in / g;
            Q = out / g;
            const double ratio = (double)in / (double)out;
            const double stretch = juce::jmax(1.0, ratio);   // downsampling: wider kernel, lower cut-off
            const double support = 32.0 * stretch;            // zero crossings on each side, in input frames
            halfWidth = (int)std::ceil(support) + 1;
            taps = 2 * halfWidth + 1;
            const double cutoff = 0.5 / stretch;              // cycles per input frame
            const double beta = 9.0;
            auto besselI0 = [](double x)
            {
                double sum = 1.0, term = 1.0;
                for (int k = 1; k < 80; ++k)
                {
                    const double f = x / (2.0 * k);
                    term *= f * f;
                    sum += term;
                    if (term < 1.0e-14 * sum) break;
                }
                return sum;
            };
            const double i0Beta = besselI0(beta);
            coefficients.assign((size_t)Q * (size_t)taps, 0.0f);
            for (int64_t q = 0; q < Q; ++q)
            {
                // Output frame n lies frac = (n*P mod Q)/Q input frames after its base frame.
                const double frac = (double)q / (double)Q;
                float* h = &coefficients[(size_t)q * (size_t)taps];
                double sum = 0.0;
                for (int k = 0; k < taps; ++k)
                {
                    const double t = (double)(k - halfWidth) - frac;   // input frame minus the output's centre
                    const double x = std::abs(t) / support;
                    const double window = x < 1.0 ? besselI0(beta * std::sqrt(1.0 - x * x)) / i0Beta : 0.0;
                    const double arg = 2.0 * cutoff * t;
                    const double sinc = std::abs(arg) < 1.0e-12 ? 1.0
                        : std::sin(juce::MathConstants<double>::pi * arg) / (juce::MathConstants<double>::pi * arg);
                    h[k] = (float)(2.0 * cutoff * sinc * window);
                    sum += h[k];
                }
                if (sum > 1.0e-9) for (int k = 0; k < taps; ++k) h[k] = (float)(h[k] / sum);   // unity gain in every phase
            }
        }
        int64_t outputFrames(int64_t inputFrames) const
        {
            return juce::jmax<int64_t>(1, (int64_t)((inputFrames * Q + P / 2) / P));
        }
        // One channel; frames before the start and after the end count as silence.
        void process(const float* in, int64_t inFrames, float* out, int64_t outFrames) const
        {
            for (int64_t n = 0; n < outFrames; ++n)
            {
                const int64_t numerator = n * P;
                const int64_t base = numerator / Q;
                const float* h = &coefficients[(size_t)(numerator % Q) * (size_t)taps];
                const int64_t first = base - halfWidth;
                const int kStart = (int)juce::jmax<int64_t>(0, -first);
                const int kEnd = (int)juce::jmin<int64_t>(taps, inFrames - first);
                const float* src = in + first;
                float acc = 0.0f;
                for (int k = kStart; k < kEnd; ++k) acc += src[k] * h[k];
                out[n] = acc;
            }
        }
    };

    // The share of a signal's energy above `fromHz` in its first second (dB
    // below the whole; -200 for none): whether a lower storage rate would cut
    // something audible. Windows of 4096 frames over the first second, the
    // loudest window counts (a reed's chiff or a bright treble start).
    double energyAboveDb(const juce::AudioBuffer<float>& audio, double sampleRate, double fromHz)
    {
        constexpr int order = 12, n = 1 << order;
        const int frames = audio.getNumSamples();
        if (frames < n || sampleRate <= 0.0) return -200.0;
        juce::dsp::FFT fft(order);
        std::vector<float> buffer((size_t)n * 2);
        const int64_t span = juce::jmin<int64_t>(frames, (int64_t)sampleRate) - n;
        const int windows = span <= 0 ? 1 : (int)juce::jmin<int64_t>(8, span / 2048 + 1);
        const int firstBin = juce::jlimit(1, n / 2, (int)std::ceil(fromHz / sampleRate * n));
        double worst = -200.0;
        for (int w = 0; w < windows; ++w)
        {
            const int64_t start = windows > 1 ? span * w / (windows - 1) : 0;
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            for (int ch = 0; ch < audio.getNumChannels(); ++ch)
            {
                const float* x = audio.getReadPointer(ch) + start;
                for (int i = 0; i < n; ++i)
                {
                    const float window = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * (float)i / (float)(n - 1));
                    buffer[(size_t)i] += x[i] * window;
                }
            }
            fft.performFrequencyOnlyForwardTransform(buffer.data(), true);
            double total = 0.0, above = 0.0;
            for (int k = 1; k <= n / 2; ++k)
            {
                const double e = (double)buffer[(size_t)k] * buffer[(size_t)k];
                total += e;
                if (k >= firstBin) above += e;
            }
            if (total > 1.0e-18) worst = juce::jmax(worst, 10.0 * std::log10(above / total + 1.0e-20));
        }
        return worst;
    }

    // Reads a sample file into a Sample at `baseRate` (resampled when the file's
    // rate differs; `resampleToBase` false keeps the file's own rate). `fullRate`
    // (0.8.146, the releases at half the rate): a higher rate the file falls back
    // to when its first second carries treble above 0.42 x baseRate louder than
    // -30 dB of the whole - a reed or a bright treble pipe would otherwise lose
    // it and, played by the kernel's linear interpolation at twice the rate,
    // ring with the images of what is left (Caen Clairon 4': the images in
    // 12-16 kHz sat 20 dB under the signal). `keptFull` tells the caller.
    std::unique_ptr<Sample> decodeSampleFile(const juce::File& file, juce::AudioFormatManager& manager,
        bool asFloat, SmplInfo smpl, bool resampleToBase = true, double baseRate = kBaseSampleRate,
        double fullRate = 0.0, bool* keptFull = nullptr, std::shared_ptr<juce::MemoryBlock> data = nullptr)
    {
        std::vector<LoopPoints> loops = std::move(smpl.loops);
        // The file from the disk, or its bytes read out of a RAR archive (0.8.157).
        std::unique_ptr<juce::AudioFormatReader> reader(data != nullptr
            ? manager.createReaderFor(std::make_unique<juce::MemoryInputStream>(data->getData(), data->getSize(), false))
            : manager.createReaderFor(file));
        if (reader == nullptr || reader->lengthInSamples <= 0 || reader->numChannels == 0) return nullptr;
        if (reader->lengthInSamples > (int64_t)1 << 30) return nullptr;   // > 2^30 frames: not a pipe sample
        const int channels = (int)juce::jmin<unsigned int>(reader->numChannels, 8u);
        const int64_t frames = reader->lengthInSamples;
        const double sourceRate = reader->sampleRate;
        // Files up to 2^27 frames (47 minutes at 48 kHz) are resampled to the base rate.
        const bool resample = resampleToBase && sourceRate > 1000.0 && std::abs(sourceRate - baseRate) > 0.5
            && frames <= ((int64_t)1 << 27);
        std::unique_ptr<Sample> sample;
        juce::AudioBuffer<float> temp;
        constexpr int chunk = 1 << 15;
        if (resample)
        {
            // The whole file as float first: the treble check decides the rate.
            temp.setSize(channels, (int)frames);   // std::bad_alloc: handled by the caller, which stops the whole load
            temp.clear();
            if (!reader->read(&temp, 0, (int)frames, 0, true, true)) return nullptr;
            double targetRate = baseRate;
            bool sincHead = false;
            if (fullRate > baseRate + 0.5)
            {
                // 0.8.147: the sinc kernel plays the first 1.3 s of a halved file without
                // the images, so only a file whose treble above the halved rate's reach
                // is a real part of it (louder than -18 dB of the whole: a reed in the
                // treble) keeps the full rate; the -30 dB rule of 0.8.146 kept too many.
                // 0.8.149: the sinc costs ten times the linear kernel - a halved file
                // with nothing above a quarter of its rate (6 kHz at 24 kHz) louder than
                // -40 dB has nothing to mirror and plays linear (the Core 2 Duo Mac fell
                // from 4100 to 2200 voices with every release through the sinc, #64).
                const double top = energyAboveDb(temp, sourceRate, 0.46 * baseRate);
                if (top > -18.0) { targetRate = fullRate; if (keptFull != nullptr) *keptFull = true; }
                else sincHead = energyAboveDb(temp, sourceRate, 0.25 * baseRate) > -40.0;
            }
            const bool convert = std::abs(sourceRate - targetRate) > 0.5;
            std::unique_ptr<SincResampler> resampler;
            int64_t storedFrames = frames;
            if (convert)
            {
                resampler = std::make_unique<SincResampler>(sourceRate, targetRate);
                storedFrames = resampler->outputFrames(frames);
            }
            sample = std::make_unique<Sample>(channels, storedFrames, convert ? targetRate : sourceRate, asFloat);
            sample->fileName = file.getFileName();
            sample->sourceSampleRate = sourceRate;
            sample->sincHead = sincHead;
            std::vector<float> resampled(convert ? (size_t)storedFrames : (size_t)0);
            for (int c = 0; c < channels; ++c)
            {
                const float* src = temp.getReadPointer(c);
                if (convert) { resampler->process(src, frames, resampled.data(), storedFrames); src = resampled.data(); }
                if (asFloat) std::memcpy(sample->writable32(c), src, (size_t)storedFrames * sizeof(float));
                else
                {
                    int16_t* dst = sample->writable16(c);
                    for (int64_t i = 0; i < storedFrames; ++i)
                        dst[i] = (int16_t)std::lrint(juce::jlimit(-32768.0f, 32767.0f, src[(size_t)i] * 32768.0f));
                }
            }
            if (convert)
            {
                const double scale = targetRate / sourceRate;
                for (auto& loop : loops)
                {
                    loop.start = (int64_t)std::llround((double)loop.start * scale);
                    loop.end = (int64_t)std::llround((double)loop.end * scale);
                }
            }
            sample->finaliseLoops(std::move(loops));
            sample->smplPitch = smpl.pitchSemitones;
            return sample;
        }
        sample = std::make_unique<Sample>(channels, frames, sourceRate, asFloat);   // std::bad_alloc: the caller stops the load
        sample->fileName = file.getFileName();
        temp.setSize(channels, chunk);
        sample->sourceSampleRate = sourceRate;
        for (int64_t position = 0; position < frames; position += chunk)
        {
            const int count = (int)juce::jmin<int64_t>(chunk, frames - position);
            temp.clear();
            if (!reader->read(&temp, 0, count, position, true, true)) break;
            for (int c = 0; c < channels; ++c)
            {
                const float* src = temp.getReadPointer(c);
                if (asFloat)
                {
                    std::memcpy(sample->writable32(c) + position, src, (size_t)count * sizeof(float));
                }
                else
                {
                    int16_t* dst = sample->writable16(c) + position;
                    for (int i = 0; i < count; ++i)
                    {
                        const float scaled = src[i] * 32768.0f;
                        const int q = (int)std::lrint(juce::jlimit(-32768.0f, 32767.0f, scaled));
                        dst[i] = (int16_t)q;
                    }
                }
            }
        }
        sample->finaliseLoops(std::move(loops));
        sample->smplPitch = smpl.pitchSemitones;
        return sample;
    }

}

SmplInfo SamplerEngine::parseSmpl(const juce::File& file, std::shared_ptr<juce::MemoryBlock> data)
{
    SmplInfo info;
    // The file from the disk, or its bytes read out of a RAR archive (0.8.157).
    std::unique_ptr<juce::InputStream> stream = data != nullptr
        ? std::unique_ptr<juce::InputStream>(new juce::MemoryInputStream(data->getData(), data->getSize(), false))
        : std::unique_ptr<juce::InputStream>(file.createInputStream());
    if (!stream) return info;
    char header[12];
    if (stream->read(header, 12) < 12) return info;
    if (std::strncmp(header, "RIFF", 4) != 0) return info;
    while (!stream->isExhausted())
    {
        char chunkID[4];
        uint32_t chunkSize = 0;
        if (stream->read(chunkID, 4) < 4) break;
        if (stream->read(&chunkSize, 4) < 4) break;
        if (std::strncmp(chunkID, "smpl", 4) == 0)
        {
            // dwManufacturer, dwProduct, dwSamplePeriod, dwMIDIUnityNote,
            // dwMIDIPitchFraction, dwSMPTEFormat, dwSMPTEOffset, cSampleLoops, cbSamplerData
            stream->skipNextBytes(12);
            uint32_t unityNote = 0, pitchFraction = 0;
            stream->read(&unityNote, 4);
            stream->read(&pitchFraction, 4);
            stream->skipNextBytes(8);
            if (unityNote < 128)
                info.pitchSemitones = (float)unityNote + (float)((double)pitchFraction / 4294967296.0);
            uint32_t numLoops = 0;
            stream->read(&numLoops, 4);
            stream->skipNextBytes(4);
            numLoops = juce::jmin<uint32_t>(numLoops, 256u);
            for (uint32_t i = 0; i < numLoops; ++i)
            {
                stream->skipNextBytes(8);
                uint32_t start = 0, end = 0;
                if (stream->read(&start, 4) < 4 || stream->read(&end, 4) < 4) break;
                stream->skipNextBytes(8);
                info.loops.push_back({ (int64_t)start, (int64_t)end });
            }
            return info;
        }
        if (!stream->setPosition(stream->getPosition() + chunkSize + (chunkSize % 2))) break;
    }
    return info;
}

void SamplerEngine::loadSamplesIntoStop(Stop* stop, const juce::File& folder, bool isRelease, int releaseMaxHoldMs)
{
    struct Entry { juce::File file; int midi; int velocityLayer; std::shared_ptr<juce::MemoryBlock> data; bool archived; };
    std::vector<Entry> entries;
    // The folder's files: from the disk, or (0.8.157) read out of the RAR archive
    // the folder lies in - each file by the thread that decodes it (a solid
    // archive comes read already), the data in memory until decoded.
    for (const auto& found : SampleFiles::readFolderFiles(folder, { ".wav", ".flac", ".aiff" }))
    {
        const int midi = extractMidiFromFilename(found.name);
        // "_NNN" at the end: a velocity layer (attacks and releases alike).
        const int layer = extractVelocityLayerFromFilename(found.name);
        // Files whose key (number - offset) lies off the 0..127 range are not decoded.
        if (midi >= 0 && midi - stop->keyShift() >= 0 && midi - stop->keyShift() < 128) entries.push_back({ folder.getChildFile(found.name), midi, layer, found.data, found.archived });
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
    {
        return a.file.getFileName().compareNatural(b.file.getFileName()) < 0;
    });
    std::vector<std::unique_ptr<Sample>> decoded(entries.size());
    std::array<std::unique_ptr<juce::AudioFormatManager>, 16> managers;
    juce::SpinLock managerLock;
    const bool asFloat = storeSamplesAsFloat;
    const bool resample = resampleOnLoad;
    // The storage rate (0.8.143): 48 kHz as ever or lower; releases at half of it when asked.
    const bool halfRate = isRelease && releaseHalfRate;
    const double baseRate = halfRate ? storageSampleRate * 0.5 : storageSampleRate;
    parallelFor((int)entries.size(), [&](int index, int thread)
    {
        if (loadOutOfMemory.load(std::memory_order_relaxed)) return;
        // One stop of thousands of files: the memory checked every 32 files too.
        if ((index & 31) == 31)
        {
            if (loadCancelRequested.load(std::memory_order_relaxed)) { loadCancelled.store(true, std::memory_order_relaxed); loadOutOfMemory.store(true, std::memory_order_relaxed); return; }
            juce::String memoryReason;
            if (memoryShortNow(&memoryReason))
            {
                if (!loadStoppedForMemory.exchange(true, std::memory_order_relaxed))
                {
                    const juce::SpinLock::ScopedLockType lk(managerLock);
                    loadMemoryStopReason = memoryReason;
                    CrashHandler::note("Loading stopped for memory inside \"" + stop->name + "\": " + memoryReason);
                }
                loadOutOfMemory.store(true, std::memory_order_relaxed);
                return;
            }
        }
        juce::AudioFormatManager* manager = nullptr;
        {
            const juce::SpinLock::ScopedLockType lk(managerLock);
            auto& slot = managers[(size_t)juce::jlimit(0, 15, thread)];
            if (slot == nullptr) { slot = std::make_unique<juce::AudioFormatManager>(); slot->registerBasicFormats(); }
            manager = slot.get();
        }
        try
        {
            if (entries[(size_t)index].archived && entries[(size_t)index].data == nullptr) entries[(size_t)index].data = SampleFiles::readFile(entries[(size_t)index].file);
            if (entries[(size_t)index].archived && entries[(size_t)index].data == nullptr) return;   // could not be read from the archive (logged)
            bool keptFull = false;
            decoded[(size_t)index] = decodeSampleFile(entries[(size_t)index].file, *manager, asFloat,
                parseSmpl(entries[(size_t)index].file, entries[(size_t)index].data), resample || baseRate < kBaseSampleRate - 0.5, baseRate,
                halfRate ? storageSampleRate : 0.0, &keptFull, entries[(size_t)index].data);
            // The bytes read for this file (0.8.205: the loading window's "Reading from
            // the disk at N MB/s" line): the archive entry's bytes, else the file's size.
            loadBytesRead.fetch_add(entries[(size_t)index].data != nullptr ? (juce::int64)entries[(size_t)index].data->getSize()
                : entries[(size_t)index].file.getSize(), std::memory_order_relaxed);
            entries[(size_t)index].data.reset();   // the archive bytes go as soon as the sample is decoded
            if (halfRate && decoded[(size_t)index] != nullptr)
            {
                (keptFull ? halfRateKeptFull : halfRateReleases).fetch_add(1, std::memory_order_relaxed);
                if (decoded[(size_t)index]->sincHead) halfRateSincHeads.fetch_add(1, std::memory_order_relaxed);
            }
            if (decoded[(size_t)index] != nullptr) packIfEnabled(*decoded[(size_t)index]);   // 0.8.145: packed on the loader thread, before it counts
        }
        catch (const std::bad_alloc&)
        {
            // Out of RAM: stop decoding, the division stays incomplete but valid.
            loadOutOfMemory.store(true, std::memory_order_relaxed);
            decoded[(size_t)index] = nullptr;
        }
        catch (...)
        {
            decoded[(size_t)index] = nullptr;
        }
    });
    // Cut by the memory stop (or a failed allocation) in the middle of its
    // files: half a stop is no stop - what was decoded is dropped and the tree
    // loader leaves the stop out, named among the skipped ones; it stays in the
    // pool (0.8.135; before, the part loaded stood on the grid as a stop).
    if (loadOutOfMemory.load(std::memory_order_relaxed))
    {
        loadMemoryCutStops.insert(stop);
        return;
    }
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (decoded[i] == nullptr) continue;
        // The key the file plays on: its number minus the stop's transposition
        // (0.8.155: Doesburg's "024" on key 36 with -12; before, the number was the
        // key and the organ sounded an octave high, its lowest octave off the keyboard).
        const int midi = entries[i].midi - stop->keyShift();
        if (midi < 0 || midi > 127) continue;
        if (isRelease) stop->getOrCreateReleaseLayer(releaseMaxHoldMs, entries[i].velocityLayer > 0 ? entries[i].velocityLayer : -1)
            .samplesByNote[(size_t)midi].push_back(std::move(decoded[i]));
        else if (entries[i].velocityLayer > 0) stop->getOrCreateVelocityLayer(entries[i].velocityLayer).samplesByNote[(size_t)midi].push_back(std::move(decoded[i]));
        else stop->samplesByNote[(size_t)midi].push_back(std::move(decoded[i]));
    }
    if (!isRelease && !stop->velocityLayers.empty())
    {
        stop->sortVelocityLayers();
        // A key with layers but no plain file: its highest layer plays above its
        // range too (it moves up and becomes the key's plain sample).
        for (int note = 0; note < 128; ++note)
        {
            if (stop->hasNote(note)) continue;
            for (auto it = stop->velocityLayers.rbegin(); it != stop->velocityLayers.rend(); ++it)
            {
                auto& variants = it->samplesByNote[(size_t)note];
                if (variants.empty()) continue;
                stop->samplesByNote[(size_t)note] = std::move(variants);
                variants.clear();
                break;
            }
        }
        // Layers that lost every sample this way are dropped.
        stop->velocityLayers.erase(std::remove_if(stop->velocityLayers.begin(), stop->velocityLayers.end(), [](const Stop::VelocityLayer& layer)
        {
            for (const auto& variants : layer.samplesByNote) if (!variants.empty()) return false;
            return true;
        }), stop->velocityLayers.end());
    }
    for (int note = 0; note < 128; ++note)
        if (stop->hasNote(note)) stop->setNoteDb(note, 0.0f);
}

void SamplerEngine::loadTopFolder(const juce::File& topFolder, double targetSR)
{
    loadManualFolder(topFolder, 0, targetSR);
    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) commitPendingDivisions();
}

bool SamplerEngine::folderHasNumberedSamples(const juce::File& folder)
{
    // A stop folder: WAV files whose names start with the three-digit MIDI note.
    bool foundWav = false;
    for (const auto& name : fileNamesOf(folder))
    {
        if (!name.endsWithIgnoreCase(".wav")) continue;
        foundWav = true;
        const auto fileName = name.dropLastCharacters(4);
        if (fileName.length() < 3) return false;
        for (int i = 0; i < 3; ++i)
            if (!juce::CharacterFunctions::isDigit(fileName[i])) return false;
    }
    return foundWav;
}

std::vector<juce::File> SamplerEngine::listSampleFolders(const juce::File& tree) const
{
    std::vector<juce::File> result;
    const auto allFolders = subdirectoriesRecursive(tree);
    const auto folderKeys = folderKeysOf(allFolders);
    for (const juce::File& folder : allFolders)
    {
        if (folder.getFileName().startsWithIgnoreCase("rel")) continue;
        if (isIgnoredFolderName(folder.getFileName()) || isTremmedVariantFolder(folder, folderKeys)) continue;
        if (!folderHasNumberedSamples(folder)) continue;
        result.push_back(folder);
    }
    return result;
}

std::vector<SamplerEngine::FolderAssignment> SamplerEngine::scanTreeAssignments(const juce::File& topFolder, bool learnPerspectives)
{
    std::vector<juce::String> perspectives = perspectivePatterns;
    auto folderHasValidWavFiles = [](const juce::File& folder) { return folderHasNumberedSamples(folder); };
    const auto allFolders = subdirectoriesRecursive(topFolder);
    const auto folderKeys = folderKeysOf(allFolders);

    // Perspective prefixes the folder names reveal themselves: when the same
    // stops appear under several prefixes ("01-dirMk2H-OW_Flote8",
    // "02-difMk2H-OW_Flote8", "03-rearMk4-OW_Flote8" for every stop) those
    // prefixes are microphone perspectives. A prefix that shares its stem with
    // a known perspective (dir ~ Direct, dif ~ Diffuse, rea ~ Rear) joins that
    // perspective's line ("Direct;01-dirMk2H-;dir"), an unknown one gets a line
    // of its own. The list is kept for later loads.
    {
        auto patternMatches = [&](const juce::String& name)
        {
            const auto comparableName = comparableFolderName(name);
            for (const auto& rawLine : perspectives)
                for (const auto& pattern : juce::StringArray::fromTokens(rawLine, ";", ""))
                {
                    const auto comparablePattern = comparableFolderName(pattern.trim());
                    if (comparablePattern.isNotEmpty() && comparableName.startsWith(comparablePattern)) return true;
                }
            return false;
        };
        // Every folder with numbered samples (release and ignored folders aside)
        // is cut after each run of separators within its first 24 characters:
        // "01-dirMk2H-OW_Flote8" gives the candidates "01-", "01-dirMk2H-" and
        // "01-dirMk2H-OW_" with the rest of the name as the stop.
        struct PrefixCandidate { std::set<juce::String> stops; juce::String spelling; };
        std::map<juce::String, PrefixCandidate> byPrefix;   // lower-case prefix (separators included)
        // The L and R halves of a stereo stop ("quintaton16L" beside "quintaton16R",
        // 0.8.159): one stop for the comparison, else a perspective seen only with
        // its left half never matched the one with the right ("AB_glockentonL" and
        // "AB_glockentonR" against "Re_glockenton" kept AB and Re from being learned).
        auto componentOf = [&folderKeys](const juce::File& folder) -> juce::String
        {
            const auto name = folder.getFileName();
            if (name.length() < 3) return {};
            const juce::juce_wchar last = name.getLastCharacter(), before = name[name.length() - 2];
            if (!(last == 'L' || last == 'l' || last == 'R' || last == 'r')) return {};
            // After the footage ("quintaton16L"), or a capital after a small letter ("glockentonL").
            if (!juce::CharacterFunctions::isDigit(before) && !((last == 'L' || last == 'R') && juce::CharacterFunctions::isLowerCase(before))) return {};
            const juce::String other = name.dropLastCharacters(1) + ((last == 'L' || last == 'l') ? "R" : "L");
            if (folderKeys.count(folder.getSiblingFile(other).getFullPathName().toLowerCase()) == 0
                && folderKeys.count(folder.getSiblingFile(other.toLowerCase()).getFullPathName().toLowerCase()) == 0) return {};
            return juce::String::charToString(juce::CharacterFunctions::toUpperCase(last));
        };
        int consideredStops = 0;   // 0.8.223: the stop folders held against the candidates (a head ALL of them carry)
        for (const juce::File& stopFolder : allFolders)
        {
            const auto name = stopFolder.getFileName();
            if (name.startsWithIgnoreCase("rel") || isIgnoredFolderName(name) || isTremmedVariantFolder(stopFolder, folderKeys) || !folderHasValidWavFiles(stopFolder)) continue;
            if (patternMatches(name)) continue;   // already a known perspective
            ++consideredStops;
            auto isSeparator = [](juce::juce_wchar c) { return c == '-' || c == '_' || c == ' '; };
            const bool component = componentOf(stopFolder).isNotEmpty();
            for (int i = 1; i < name.length() && i <= 24; ++i)
            {
                if (!isSeparator(name[i - 1]) || isSeparator(name[i])) continue;   // cut right after a separator run
                const auto prefix = name.substring(0, i);
                const auto stop = comparableFolderName(component ? name.substring(i).dropLastCharacters(1) : name.substring(i));
                if (stop.isEmpty() || comparableFolderName(prefix).isEmpty()) continue;
                auto& candidate = byPrefix[prefix.toLowerCase()];
                candidate.stops.insert(stop);
                candidate.spelling = prefix;
            }
        }
        // A prefix is a perspective when the stops carrying it also appear under
        // another prefix: at least two stops in common, and most of the stops of
        // both prefixes ("dir" and "dif" name the same organ). Stops without a
        // counterpart elsewhere ("Principal8_chiff" alone) never qualify.
        std::vector<juce::String> qualifying;
        for (const auto& [prefix, candidate] : byPrefix)
        {
            if (candidate.stops.size() < 2) continue;
            bool partner = false;
            for (const auto& [other, otherCandidate] : byPrefix)
            {
                if (other == prefix || otherCandidate.stops.size() < 2) continue;
                int common = 0;
                for (const auto& stop : candidate.stops) if (otherCandidate.stops.count(stop) > 0) ++common;
                if (common >= 2 && common * 2 >= (int)candidate.stops.size() && common * 2 >= (int)otherCandidate.stops.size())
                { partner = true; break; }
            }
            if (partner) qualifying.push_back(prefix);
        }
        // 0.8.223 (the user's rule): a head that EVERY stop folder of the tree carries and that
        // names a perspective (dir, dif, rea, far, near, close, dry, wet, front, rear, ab, ortf ...)
        // is a perspective even without a counterpart in this tree (a package of one perspective
        // alone: "06-far__" on all of Bergen's Distant folders). A lone head without such a word
        // ("HW_", "Ped_": a division mark) is never taken - it would fuse the divisions' stops.
        auto perspectiveWord = [](const juce::String& prefix)
        {
            static const char* words[]{ "direct", "diffuse", "distant", "dir", "dif", "rea", "rear", "far", "near", "close", "dry", "wet", "front", "ab", "ortf", "dist", "surr", "amb" };
            juce::StringArray tokens;
            juce::String token;
            for (int i = 0; i < prefix.length(); ++i)
            {
                const auto c = juce::CharacterFunctions::toLowerCase(prefix[i]);
                if (juce::CharacterFunctions::isLetter(c)) token << juce::String::charToString(c);
                else { if (token.isNotEmpty()) tokens.add(token); token = {}; }
            }
            if (token.isNotEmpty()) tokens.add(token);
            for (const auto& t : tokens)
                for (const char* w : words)
                    if (t == w || (juce::String(w).length() >= 3 && t.startsWith(w))) return true;
            return false;
        };
        if (consideredStops >= 2)
            for (const auto& [prefix, candidate] : byPrefix)
                if ((int)candidate.stops.size() >= consideredStops && std::find(qualifying.begin(), qualifying.end(), prefix) == qualifying.end() && perspectiveWord(candidate.spelling))
                    qualifying.push_back(prefix);
        // The shortest form of each prefix: "01-dirMk2H-" rather than "01-dirMk2H-OW_".
        std::vector<juce::String> found;
        for (const auto& prefix : qualifying)
        {
            bool longerForm = false;
            for (const auto& other : qualifying) if (other != prefix && prefix.startsWith(other)) longerForm = true;
            if (!longerForm) found.push_back(byPrefix[prefix].spelling);
        }
        bool changed = false;
        for (const auto& prefix : found)
        {
            const auto lower = prefix.toLowerCase();
            int matchedLine = -1, matches = 0;
            for (int i = 0; i < (int)perspectives.size(); ++i)
            {
                const auto perspectiveName = juce::StringArray::fromTokens(perspectives[(size_t)i], ";", "")[0].trim().toLowerCase();
                if (perspectiveName.length() < 3) continue;
                const auto stem = perspectiveName.substring(0, 3);
                if (lower.contains(stem) || (lower.length() >= 3 && perspectiveName.contains(lower)))
                {
                    matchedLine = i;
                    ++matches;
                }
            }
            if (matches == 1)
            {
                auto tokens = juce::StringArray::fromTokens(perspectives[(size_t)matchedLine], ";", "");
                tokens.trim();
                tokens.removeEmptyStrings();
                bool present = false;
                for (const auto& token : tokens) if (token.equalsIgnoreCase(prefix)) present = true;
                if (!present) { tokens.add(prefix); perspectives[(size_t)matchedLine] = tokens.joinIntoString(";"); changed = true; }
            }
            else
            {
                // A readable perspective name from the prefix: "01-dirMk2H-" -> "DirMk2H".
                juce::String label = prefix.trimCharactersAtStart("0123456789-_ ").trimCharactersAtEnd("-_ ");
                if (label.isEmpty()) label = prefix.trimCharactersAtEnd("-_ ");
                label = label.substring(0, 1).toUpperCase() + label.substring(1);
                perspectives.push_back(label + ";" + prefix);
                changed = true;
            }
        }
        if (changed && learnPerspectives)
        {
            const juce::ScopedLock sl(controlLock);
            perspectivePatterns = perspectives;
        }
    }

    auto stripPerspective = [&perspectives](juce::String name, juce::String* matchedPerspective)
    {
        return stripPerspectivePrefix(perspectives, name, matchedPerspective);
    };
    // The L/R half mark of a folder (see the learning above): the same test.
    auto componentOf = [&folderKeys](const juce::File& folder) -> juce::String
    {
        const auto name = folder.getFileName();
        if (name.length() < 3) return {};
        const juce::juce_wchar last = name.getLastCharacter(), before = name[name.length() - 2];
        if (!(last == 'L' || last == 'l' || last == 'R' || last == 'r')) return {};
        if (!juce::CharacterFunctions::isDigit(before) && !((last == 'L' || last == 'R') && juce::CharacterFunctions::isLowerCase(before))) return {};
        const juce::String other = name.dropLastCharacters(1) + ((last == 'L' || last == 'l') ? "R" : "L");
        if (folderKeys.count(folder.getSiblingFile(other).getFullPathName().toLowerCase()) == 0
            && folderKeys.count(folder.getSiblingFile(other.toLowerCase()).getFullPathName().toLowerCase()) == 0) return {};
        return juce::String::charToString(juce::CharacterFunctions::toUpperCase(last));
    };

    std::vector<FolderAssignment> assignments;
    for (const juce::File& stopFolder : allFolders)
    {
        if (stopFolder.getFileName().startsWithIgnoreCase("rel")) continue;
        if (isIgnoredFolderName(stopFolder.getFileName()) || isTremmedVariantFolder(stopFolder, folderKeys)) continue;
        if (!folderHasValidWavFiles(stopFolder)) continue;

        juce::String perspective;
        juce::String leafName = stripPerspective(stopFolder.getFileName(), &perspective);
        juce::File perspectiveRoot;
        juce::String perspectiveRootName;
        if (perspective.isNotEmpty())
        {
            perspectiveRoot = stopFolder;
            perspectiveRootName = leafName;
        }
        else
        {
            for (auto ancestor = stopFolder.getParentDirectory(); ancestor != topFolder
                 && ancestor.getFullPathName().isNotEmpty(); ancestor = ancestor.getParentDirectory())
            {
                juce::String inheritedPerspective;
                const auto stripped = stripPerspective(ancestor.getFileName(), &inheritedPerspective);
                if (inheritedPerspective.isNotEmpty())
                {
                    perspective = inheritedPerspective;
                    perspectiveRoot = ancestor;
                    perspectiveRootName = stripped;
                    break;
                }
                if (ancestor.getParentDirectory() == ancestor) break;
            }
        }
        const bool patterned = perspective.isNotEmpty();
        juce::String stopName;
        if (patterned && perspectiveRoot != stopFolder)
        {
            const auto descendant = stopFolder.getRelativePathFrom(perspectiveRoot)
                .replaceCharacter('\\', '/').replace("/", " - ");
            stopName = StopNames::capitaliseAfterDivisionPrefix(displayFolderName(perspectiveRootName)) + " - " + StopNames::capitaliseAfterDivisionPrefix(displayFolderName(descendant));
        }
        else
        {
            if (!patterned) perspective = "default";
            auto displayParent = stopFolder.getParentDirectory();
            // The parent named like the folder itself - perspective prefixes, the
            // footage and an L/R mark aside ("HW_Bl.Flute2" over "02-direct-HW_Bl.Flute",
            // 0.8.160) - adds nothing to the name: the last folder names the stop.
            auto stem = [](const juce::String& comparable)
            {
                juce::String s = comparable;
                while (s.isNotEmpty() && (s.endsWithChar('l') || s.endsWithChar('r'))) s = s.dropLastCharacters(1);
                while (s.isNotEmpty() && (juce::CharacterFunctions::isDigit(s.getLastCharacter()) || s.endsWithChar('\''))) s = s.dropLastCharacters(1);
                return s;
            };
            if (displayParent != topFolder
                && (comparableFolderName(stripPerspective(displayParent.getFileName(), nullptr)) == comparableFolderName(leafName)
                    || (stem(comparableFolderName(stripPerspective(displayParent.getFileName(), nullptr))).isNotEmpty()
                        && stem(comparableFolderName(stripPerspective(displayParent.getFileName(), nullptr))) == stem(comparableFolderName(leafName)))))
                displayParent = displayParent.getParentDirectory();
            auto relativeParent = displayFolderName(displayParent.getRelativePathFrom(topFolder)
                .replaceCharacter('\\', '/').replace("/", " - "));
            leafName = StopNames::capitaliseAfterDivisionPrefix(displayFolderName(leafName));
            stopName = relativeParent == "." || relativeParent.isEmpty() ? leafName : relativeParent + " - " + leafName;
        }
        // The L and R halves of a stereo stop (0.8.160, the user's rule): "quintaton16L"
        // and "quintaton16R" are two components of ONE stop "quintaton 16" - both
        // folders become ranks of it. Only a single L or R right after the footage
        // with the other half beside it; "Mixtur4x" or "Zimbel3" say the rank count.
        const juce::String component = componentOf(stopFolder);
        if (component.isNotEmpty() && stopName.endsWithIgnoreCase(component)) stopName = stopName.dropLastCharacters(1).trimEnd();
        // Perspective folders of one stop name form one stop; a folder without a
        // perspective prefix is a stop of its own ("default" perspective).
        FolderAssignment assignment{ stopFolder, stopName, perspective, patterned || component.isNotEmpty() };
        assignment.component = component;
        assignments.push_back(assignment);
    }
    return assignments;
}

void SamplerEngine::loadManualFolder(const juce::File& topFolder, int manualIndex, double targetSR,
    std::function<void(double)> progress, bool append, const std::vector<FolderAssignment>* map, const std::set<int>* replacing)
{
    // The divisions this load replaces (shared by every batch of the call): the
    // commit retires their old stops once, whether a batch comes for them or not.
    auto replacingAll = replacing != nullptr ? std::make_shared<const std::set<int>>(*replacing) : nullptr;
    // One tree, possibly several divisions: the map names the division of every
    // folder (the tree pool loads each tree once), otherwise everything goes to
    // 'manualIndex'. Every division gets its own pending batch.
    const int baseManual = juce::jlimit(0, maxManuals - 1, manualIndex);
    struct Group { std::unique_ptr<Impl::PendingDivision> division; std::vector<FolderAssignment> assignments; };
    std::map<int, Group> groups;
    auto groupFor = [&](int manual) -> Group&
    {
        auto& group = groups[manual];
        if (group.division == nullptr)
        {
            group.division = std::make_unique<Impl::PendingDivision>();
            group.division->manual = manual;
            group.division->targetSR = targetSR;
            group.division->append = append;
            group.division->replace = !append || (replacing != nullptr && replacing->count(manual) != 0);
            group.division->replacingAll = replacingAll;
            group.division->tree = topFolder;
            group.division->mapped = map != nullptr;
        }
        return group;
    };
    const std::vector<juce::String> oneShot = oneShotPatterns;

    // Does the committed division hold this folder with its samples in RAM?
    // Samples the division holds for that folder, decoded with the storage
    // choices in force now (0.8.149: with other choices they are decoded anew).
    const juce::String signatureNow = storageSignature();
    // ... and with the same "Files vs keys" offset (0.8.155): samples keyed for
    // another offset are decoded afresh, never adopted.
    auto hasLoadedRankForFolder = [this, &signatureNow](int manual, const juce::File& folder, int keyShift)
    {
        const juce::ScopedLock sl(controlLock);
        for (auto* stop : stops)
        {
            if (stop->primaryManual != manual) continue;
            if (!stop->container && stop->folder == folder && !stop->samplesUnloaded && stop->storageSignature == signatureNow && stop->keyShift() == keyShift) return true;
            for (const auto& rank : stop->ranks)
                if (rank->folder == folder && !rank->samplesUnloaded && rank->storageSignature == signatureNow && rank->keyShift() == keyShift) return true;
        }
        return false;
    };

    auto loadRank = [&](Stop* rank, const juce::File& stopFolder, int manualIndex)
    {
        rank->primaryManual = manualIndex;
        for (const auto& pattern : oneShot)
            if (comparableFolderName(stopFolder.getFileName()).contains(comparableFolderName(pattern)))
                rank->isOneShot = true;
        if (stopFolder.getFileName().containsIgnoreCase("chiff")) rank->isOneShot = true;
        // Stops sitting in the division's stop pool keep their samples out of RAM.
        // A folder the division already holds with its samples is not decoded
        // again: the commit adopts the old rank's samples, copies and analysis.
        const bool replaces = !append || (replacing != nullptr && replacing->count(manualIndex) != 0);
        if (isPooledStop(manualIndex, rank->name)) rank->samplesUnloaded = true;
        else if (replaces && hasLoadedRankForFolder(manualIndex, stopFolder, rank->keyShift())) { rank->samplesUnloaded = true; rank->adoptSamples = true; }
        else loadRankSamples(rank);
        rank->folderHash = (uint32_t)stopFolder.getFullPathName().hashCode();
        rank->windContribution = windStopContribution(rank->name);
    };

    // Which folder becomes which stop: the Stop building map when the user
    // made one (folders of this tree only; folders it leaves out stay unloaded),
    // otherwise the folder names.
    size_t totalAssignments = 0;
    if (map != nullptr)
    {
        for (const auto& assignment : *map)
        {
            const int manual = juce::jlimit(0, maxManuals - 1, assignment.manual >= 0 ? assignment.manual : baseManual);
            if (assignment.container)
            {
                // An empty stop (a stop folder's master): created with the tree it names.
                if (assignment.folder == topFolder) { groupFor(manual).assignments.push_back(assignment); ++totalAssignments; }
                continue;
            }
            if (!assignment.folder.isAChildOf(topFolder) || !SampleFiles::isDirectory(assignment.folder)) continue;   // disk or archive (0.8.157)
            if (!folderHasNumberedSamples(assignment.folder)) continue;
            FolderAssignment copy = assignment;
            copy.grouped = true;
            if (copy.perspective.trim().isEmpty()) copy.perspective = "default";
            if (copy.stopName.trim().isEmpty()) copy.stopName = displayFolderName(assignment.folder.getFileName());
            groupFor(manual).assignments.push_back(copy);
            ++totalAssignments;
        }
        if (groups.empty()) groupFor(baseManual);   // nothing of this tree is used: an empty batch keeps the bookkeeping
        // Folders inside a RAR archive (0.8.157) are read in the archive's order: the
        // reader keeps its place and never goes back when the stops come in that order.
        for (auto& [groupManual, group] : groups)
        {
            std::vector<std::pair<juce::int64, size_t>> order;
            bool anyArchive = false;
            for (size_t i = 0; i < group.assignments.size(); ++i)
            {
                const juce::int64 position = group.assignments[i].container ? -1 : SampleFiles::archiveOrderOf(group.assignments[i].folder);
                if (position >= 0) anyArchive = true;
                order.push_back({ position < 0 ? std::numeric_limits<juce::int64>::max() : position, i });
            }
            if (!anyArchive) continue;
            std::stable_sort(order.begin(), order.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            std::vector<FolderAssignment> sorted;
            for (const auto& [position, i] : order) sorted.push_back(group.assignments[i]);
            group.assignments = std::move(sorted);
        }
    }
    else
    {
        groupFor(baseManual).assignments = scanTreeAssignments(topFolder, true);
        totalAssignments = groups[baseManual].assignments.size();
    }

    const double totalFolders = (double)std::max<size_t>(1, totalAssignments);   // not jmax<size_t>: clang instantiates the SIMDRegister overload
    int processedFolders = 0;
    loadOutOfMemory.store(false, std::memory_order_relaxed);
    loadStoppedForMemory.store(false, std::memory_order_relaxed);
    loadMemoryCutStops.clear();
    loadMemorySkippedKeys.clear();
    if (gSwapBaseline.load() < 0)   // the first tree of a load
    {
        gSwapBaseline.store(MemoryInfo::swappedBytes());   // the paging rule counts the growth from here
        lastLoadMemoryLimited.store(false, std::memory_order_relaxed);
        loadCancelRequested.store(false, std::memory_order_relaxed);
        loadCancelled.store(false, std::memory_order_relaxed);
    }
    for (auto& [groupManual, group] : groups)
    {
        auto& newStops = group.division->stops;
        std::map<juce::String, Stop*> perspectiveStops;
        for (const auto& assignment : group.assignments)
        {
            if (progress) progress((double)processedFolders++ / totalFolders);
            const juce::File& stopFolder = assignment.folder;
            const juce::String& stopName = assignment.stopName;
            // The memory reserve (0.8.131): before every stop, the free physical
            // memory against the reserve the system needs to stay out of paging;
            // below it the loading stops here and now - the stops loaded so far
            // make the instrument, the rest is listed in the message. (A stop
            // that merely failed to allocate, bad_alloc, stops the same way.)
            // A stop whose samples the division holds already (a reload: the commit
            // adopts them, nothing is decoded) or that sits in the pool needs no
            // memory: loaded whatever the memory says (0.8.139, #33: a reload on the
            // 8 GB Mac "skipped" 25 stops that were in memory all along and named them).
            const bool replacesHere = !append || (replacing != nullptr && replacing->count(groupManual) != 0);
            const bool adoptable = !assignment.container && (isPooledStop(groupManual, stopName) || (replacesHere && hasLoadedRankForFolder(groupManual, stopFolder, assignment.sampleOffset)));
            juce::String memoryReason;
            if (!assignment.container && !adoptable && !loadOutOfMemory.load(std::memory_order_relaxed))
            {
                if (loadCancelRequested.load(std::memory_order_relaxed))
                {
                    loadCancelled.store(true, std::memory_order_relaxed);
                    loadOutOfMemory.store(true, std::memory_order_relaxed);
                    CrashHandler::note("Loading cancelled by the user before \"" + stopName + "\"");
                }
                else if (memoryShortNow(&memoryReason))
                {
                    loadStoppedForMemory.store(true, std::memory_order_relaxed);
                    loadOutOfMemory.store(true, std::memory_order_relaxed);
                    loadMemoryStopReason = memoryReason;
                    CrashHandler::note("Loading stopped for memory before \"" + stopName + "\": " + memoryReason);
                }
            }
            if (loadOutOfMemory.load(std::memory_order_relaxed) && !adoptable)
            {
                // Once per stop, not once per perspective folder (#22: "35 stops" of 15).
                if (!assignment.container) { loadMemorySkipped.addIfNotAlreadyThere(stopName); loadMemorySkippedKeys.insert(comparableFolderName(stopName)); }
                continue;
            }
            // The loading window's second line names the stop being decoded (0.8.180:
            // only the analysis and fill phases of a tree reported before, so a fast
            // load showed one line all along).
            if (!assignment.container && loadStatusCallback)
                loadStatusCallback("Loading " + (stopName.length() > 44 ? stopName.substring(0, 43) + juce::String(juce::CharPointer_UTF8("\xe2\x80\xa6")) : stopName) + " ...");   // one line, never wrapped
            if (assignment.container)
            {
                // No samples: the stop only carries its children (and their transmissions).
                auto empty = std::make_unique<Stop>(stopName, juce::File());
                empty->container = true;
                empty->perspective = "default";
                empty->primaryManual = groupManual;
                empty->folderHash = (uint32_t)stopName.hashCode();
                perspectiveStops[comparableFolderName(stopName)] = empty.get();
                newStops.add(empty.release());
                continue;
            }
            if (assignment.grouped)
            {
                const auto groupKey = comparableFolderName(stopName);
                if (auto found = perspectiveStops.find(groupKey); found != perspectiveStops.end())
                {
                    auto rank = std::make_unique<Stop>(found->second->name, stopFolder);
                    rank->perspective = assignment.perspective;
                    rank->sampleOffset = assignment.sampleOffset;
                    rank->sampleOffsetSet = assignment.sampleOffsetSet;
                    loadRank(rank.get(), stopFolder, groupManual);
                    if (!found->second->sampleOffsetDetected && rank->sampleOffsetDetected) { found->second->sampleOffset = rank->sampleOffset; found->second->sampleOffsetDetected = true; found->second->sampleOffsetSet = true; }
                    found->second->isOneShot = found->second->isOneShot || rank->isOneShot;
                    found->second->ranks.push_back(std::move(rank));
                    continue;
                }
                auto mainStop = std::make_unique<Stop>(stopName, stopFolder);
                mainStop->perspective = assignment.perspective;
                mainStop->sampleOffset = assignment.sampleOffset;
                mainStop->sampleOffsetSet = assignment.sampleOffsetSet;
                loadRank(mainStop.get(), stopFolder, groupManual);
                perspectiveStops[groupKey] = mainStop.get();
                newStops.add(mainStop.release());
            }
            else
            {
                auto miscStop = std::make_unique<Stop>(stopName, stopFolder);
                miscStop->perspective = "default";
                loadRank(miscStop.get(), stopFolder, groupManual);
                newStops.add(miscStop.release());
            }
        }
        // The stops the memory stop cut in the middle (a perspective rank of
        // theirs included): dropped whole, named with the skipped ones.
        // So is a stop with one perspective loaded and another skipped: it was
        // on the grid AND in the pool with its missing perspectives (#23).
        if (!loadMemoryCutStops.empty() || !loadMemorySkippedKeys.empty())
        {
            for (int i = newStops.size(); --i >= 0;)
            {
                Stop* stop = newStops[i];
                bool cut = loadMemoryCutStops.count(stop) > 0 || loadMemorySkippedKeys.count(comparableFolderName(stop->name)) > 0;
                for (const auto& rank : stop->ranks) if (loadMemoryCutStops.count(rank.get()) > 0) cut = true;
                if (!cut) continue;
                if (!loadMemorySkipped.contains(stop->name)) loadMemorySkipped.insert(0, stop->name);
                perspectiveStops.erase(comparableFolderName(stop->name));
                newStops.remove(i);
            }
            loadMemoryCutStops.clear();
            loadMemorySkippedKeys.clear();
        }
        juce::Random random;
        for (auto* stop : newStops) regenerateRandomTuning(stop, random);
        // All ranks of this folder come from one organ: estimate its diapason once
        // and derive the per-note corrections to a1 = 440 Hz equal temperament.
        {
            // Short texts (0.8.181): the loading window's second line must never wrap -
            // a wrapped line came and went as a flickering third line.
            if (loadStatusCallback) loadStatusCallback("Analysing tuning, temperament and footage ...");
            std::vector<Stop*> ranks;
            for (auto* stop : newStops)
            {
                ranks.push_back(stop);
                for (auto& rank : stop->ranks) ranks.push_back(rank.get());
            }
            computeTuning440(ranks);
            if (loadStatusCallback) loadStatusCallback("Filling the missing pipes ...");
            // No transmissions yet: its own division's compass. The copies of the
            // filled pipes are built in the background after the commit
            // (launchCopyBuilds), with their progress on the stop buttons.
            for (auto* stop : newStops) refillStopPipes(stop);
            // A rank recognised as a celeste starts with its recorded undulation; the
            // saved voicing may still override this afterwards.
            const float ratio = undulationBeatRatio.load(std::memory_order_relaxed);
            for (auto* stop : newStops)
            {
                float beats = stop->detectedCelesteBeats;
                for (const auto& rank : stop->ranks)
                    if (std::abs(beats) < 1.0e-3f) beats = rank->detectedCelesteBeats;
                if (std::abs(beats) < 1.0e-3f) continue;
                beats = juce::jlimit(-7.0f, 7.0f, beats);
                computeUndulationTable(stop, beats, ratio);
                for (auto& rank : stop->ranks) computeUndulationTable(rank.get(), beats, ratio);
            }
        }
        for (auto* stop : newStops)
        {
            stop->tree = topFolder;
            for (auto& rank : stop->ranks) rank->tree = topFolder;
        }
    }
    if (progress) progress(1.0);

    std::lock_guard<std::mutex> lk(impl->pendingMutex);
    if (loadOutOfMemory.load(std::memory_order_relaxed))
    {
        if (!loadCancelled.load(std::memory_order_relaxed)) lastLoadMemoryLimited.store(true, std::memory_order_relaxed);
        const int skipped = loadMemorySkipped.size();
        juce::StringArray shown;
        for (int i = 0; i < juce::jmin(12, skipped); ++i) shown.add(loadMemorySkipped[i]);
        const juce::String list = shown.joinIntoString(", ") + (skipped > 12 ? ", and " + juce::String(skipped - 12) + " more" : juce::String());
        if (loadCancelled.load(std::memory_order_relaxed))
            lastLoadError = "The loading was cancelled. The instrument is built from the stops loaded so far; " + juce::String(skipped) + " stop(s) were not loaded: " + list
                + ".\n\nThey are not on the grid; the Stop List keeps their rows, and \"Add stop\" on the grid offers them as ready-made stops.";
        else if (loadStoppedForMemory.load(std::memory_order_relaxed))
            lastLoadError = "The memory is full: the loading stopped (" + (loadMemoryStopReason.isNotEmpty() ? loadMemoryStopReason : juce::String((double)freePhysicalMemoryBytes() / 1073741824.0, 1) + " GB free")
                + " of " + juce::String((double)juce::SystemStats::getMemorySizeInMegabytes() / 1024.0, 1) + " GB installed; the system would page samples out, and every stroke would stall the sound).\n\n"
                "The instrument is built from the stops loaded so far; " + juce::String(skipped) + " stop(s) were not loaded: " + list
                + ".\n\nThey are not on the grid; the Stop List keeps their rows, and \"Add stop\" on the grid offers them as ready-made stops (taking one loads it again). Load fewer stops (a smaller configuration), keep the 16-bit sample storage, or add memory.";
        else
            lastLoadError = "The computer ran out of memory while loading\n" + topFolder.getFullPathName()
                + "\n\nOnly the stops loaded so far are available" + (skipped > 0 ? " (not loaded: " + list + ")" : juce::String()) + ". Unload another division or close other programs.";
    }
    if (lastLoadError.isNotEmpty()) CrashHandler::note("Load message: " + lastLoadError.replace("\n", " | "));   // what the dialog says (0.8.138: the reports did not carry it)
    notePackStats("the tree " + topFolder.getFileName());
    {
        const int half = halfRateReleases.exchange(0, std::memory_order_relaxed), full = halfRateKeptFull.exchange(0, std::memory_order_relaxed);
        const int sinc = halfRateSincHeads.exchange(0, std::memory_order_relaxed);
        if (half + full > 0)
            CrashHandler::note("Releases at half the rate in the tree " + topFolder.getFileName() + ": " + juce::String(half) + " stored at half ("
                + juce::String(sinc) + " of them with treble, played through the sinc kernel for 1.3 s), "
                + juce::String(full) + " kept at the full rate for their treble (energy above 0.46 x the half rate louder than -18 dB)");
    }
    for (auto& [groupManual, group] : groups) impl->pendingDivisions.push_back(std::move(group.division));
}

bool SamplerEngine::hasPendingDivisions() const
{
    std::lock_guard<std::mutex> lk(impl->pendingMutex);
    return !impl->pendingDivisions.empty();
}

void SamplerEngine::commitPendingDivisions()
{
    std::vector<std::unique_ptr<Impl::PendingDivision>> pending;
    {
        std::lock_guard<std::mutex> lk(impl->pendingMutex);
        pending.swap(impl->pendingDivisions);
    }
    if (pending.empty()) return;
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        // The old stops of every division this commit replaces retire here, all
        // at once; the new ranks marked by the loader take their samples, layers,
        // copies and analysis over before they are freed (nothing expensive is
        // decoded or built again). A replaced division without a batch ends empty.
        std::set<int> toReplace;
        for (auto& division : pending)
        {
            if (division->replace) toReplace.insert(division->manual);
            if (division->replacingAll != nullptr) toReplace.insert(division->replacingAll->begin(), division->replacingAll->end());
        }
        juce::OwnedArray<Stop> retiring;
        for (int i = stops.size(); --i >= 0;)
            if (toReplace.count(stops[i]->primaryManual) != 0) retiring.add(stops.removeAndReturn(i));
        int adoptedRanks = 0;
        for (auto& division : pending)
        {
            while (!division->stops.isEmpty())
            {
                auto* stop = division->stops.removeAndReturn(0);
                if (division->mapped)
                {
                    // Stop building: the perspectives of one stop may lie in several
                    // folder trees. A stop of that name already in the division (from
                    // an earlier tree) takes these folders as further ranks.
                    Stop* existing = nullptr;
                    for (auto* candidate : stops)
                        if (candidate->primaryManual == stop->primaryManual
                            && comparableFolderName(candidate->name) == comparableFolderName(stop->name))
                            existing = candidate;
                    if (existing != nullptr)
                    {
                        std::unique_ptr<Stop> incoming(stop);
                        incoming->name = existing->name;
                        for (auto& rank : incoming->ranks) { rank->name = existing->name; existing->ranks.push_back(std::move(rank)); }
                        incoming->ranks.clear();
                        existing->isOneShot = existing->isOneShot || incoming->isOneShot;
                        existing->ranks.push_back(std::move(incoming));
                        continue;
                    }
                }
                // Composite divisions: a name already present (from another
                // folder tree) gets a suffix so grid, pool and voicing stay unique.
                juce::String name = stop->name;
                for (int suffix = 2; ; ++suffix)
                {
                    bool taken = false;
                    for (auto* existing : stops)
                        if (existing->primaryManual == stop->primaryManual && existing->name == name) taken = true;
                    if (!taken) break;
                    name = stop->name + " (" + juce::String(suffix) + ")";
                }
                stop->name = name;
                stops.add(stop);
            }
            // Adoption: every rank of this division the loader left empty on purpose.
            auto adopt = [&](Stop* rank)
            {
                if (!rank->adoptSamples) return;
                Stop* source = nullptr;
                const juce::String wanted = storageSignature();
                for (auto* old : retiring)
                {
                    if (!old->container && old->folder == rank->folder && !old->samplesUnloaded && old->storageSignature == wanted && old->keyShift() == rank->keyShift()) source = old;
                    for (auto& r : old->ranks)
                        if (source == nullptr && r->folder == rank->folder && !r->samplesUnloaded && r->storageSignature == wanted && r->keyShift() == rank->keyShift()) source = r.get();
                    if (source != nullptr) break;
                }
                rank->adoptSamples = false;
                if (source == nullptr) { loadRankSamples(rank); return; }   // gone meanwhile, or decoded with other choices: decoded after all
                rank->storageSignature = source->storageSignature;
                for (int note = 0; note < 128; ++note) rank->samplesByNote[(size_t)note] = std::move(source->samplesByNote[(size_t)note]);
                rank->velocityLayers = std::move(source->velocityLayers);
                rank->releaseLayers = std::move(source->releaseLayers);
                rank->aliasFree = std::move(source->aliasFree);
                rank->aliasFreeOffset = std::move(source->aliasFreeOffset);
                rank->copiesMethod = source->copiesMethod;
                rank->retiredSamples = std::move(source->retiredSamples);
                rank->nextSampleIndex = source->nextSampleIndex;
                rank->tuningInfoAvailable = source->tuningInfoAvailable;
                rank->measuredDiapasonHz = source->measuredDiapasonHz;
                rank->tuning440Cents = source->tuning440Cents;
                rank->measuredTemperamentCents = source->measuredTemperamentCents;
                rank->measuredTemperamentValid = source->measuredTemperamentValid;
                rank->detectedCelesteBeats = source->detectedCelesteBeats;
                rank->footageKnown = source->footageKnown;
                rank->footageSemitones = source->footageSemitones;
                rank->lowestFootageSemitones = source->lowestFootageSemitones;
                rank->sampleOffset = source->sampleOffset;
                rank->sampleOffsetSet = true;
                rank->soundingHarmonic = source->soundingHarmonic;
                rank->soundingMeasured = source->soundingMeasured;
                rank->footageSounding = source->footageSounding;
                rank->lowestNoteHarmonicDb = source->lowestNoteHarmonicDb;
                rank->lowestNoteResidualDb = source->lowestNoteResidualDb;
                rank->footageBreaks = source->footageBreaks;
                rank->undulationBeats = source->undulationBeats;
                rank->undulationManual = source->undulationManual;
                rank->undulationCents = source->undulationCents;
                rank->faultyPipes = source->faultyPipes;         // the retune limit's verdict travels with the samples (0.8.183:
                rank->rawTuningCents = source->rawTuningCents;   // lost on adoption - the faulty pipe played itself again after a reload)
                rank->samplesUnloaded = false;
                source->samplesUnloaded = true;   // nothing left to free twice
            };
            for (auto* stop : stops)
            {
                if (stop->primaryManual != division->manual) continue;
                bool any = stop->adoptSamples;
                for (auto& rank : stop->ranks) any = any || rank->adoptSamples;
                if (!any) continue;
                if (stop->adoptSamples) ++adoptedRanks;
                for (auto& rank : stop->ranks) if (rank->adoptSamples) ++adoptedRanks;
                adopt(stop);
                for (auto& rank : stop->ranks) adopt(rank.get());
                refillStopPipes(stop);   // the fill map needs the samples, which the loader did not have
            }
            defaultSampleRate.store(division->targetSR);
        }
        if (adoptedRanks > 0) CrashHandler::note("Commit: " + juce::String(adoptedRanks) + " rank(s) kept their samples and copies (not loaded again)");
        // The global "equal at 440 Hz" switch covers stops loaded later as well.
        for (auto* stop : stops)
        {
            stop->tuneTo440 = tuneAllTo440;
            for (auto& rank : stop->ranks) rank->tuneTo440 = tuneAllTo440;
        }
        std::array<int, maxManuals> ordinals{};
        for (auto* stop : stops)
            stop->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, stop->primaryManual)]++;
        impl->ensureRoutesForStops();
    }
    impl->resume();
    updateOneShotFlags();
}

void SamplerEngine::unloadManualTree(int manualIndex, const juce::File& tree)
{
    BuildPause buildLock(*this);   // no background build may touch a stop being removed (the running one stops at its next pipe)
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        for (int i = stops.size(); --i >= 0;)
            if (stops[i]->primaryManual == manualIndex && stops[i]->tree == tree) stops.remove(i);
        // Stops built from several trees (Stop building) lose the ranks of this tree.
        for (auto* stop : stops)
            if (stop->primaryManual == manualIndex)
                stop->ranks.erase(std::remove_if(stop->ranks.begin(), stop->ranks.end(),
                    [&tree](const std::unique_ptr<Stop>& rank) { return rank->tree == tree; }), stop->ranks.end());
        std::array<int, maxManuals> ordinals{};
        for (auto* stop : stops)
            stop->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, stop->primaryManual)]++;
    }
    impl->resume();
}

void SamplerEngine::removeStop(Stop* stop)
{
    if (stop == nullptr) return;
    BuildPause buildLock(*this);   // no background build may touch the stop (the running one stops at its next pipe)
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        // A stop folder's children lose their master pointer (the GUI re-groups them).
        for (auto* other : stops) if (other->groupMasterPtr == stop) other->groupMasterPtr = nullptr;
        const int index = stops.indexOf(stop);
        if (index < 0) { impl->resume(); return; }
        CrashHandler::note("Stop removed: " + stop->name + " (division " + juce::String(stop->primaryManual) + ")");
        stops.remove(index);   // deletes it: samples, copies, transmissions
        std::array<int, maxManuals> ordinals{};
        for (auto* other : stops)
            other->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, other->primaryManual)]++;
        impl->ensureRoutesForStops();
    }
    impl->resume();
}

void SamplerEngine::moveStopToDivision(Stop* stop, int manual)
{
    if (stop == nullptr) return;
    manual = juce::jlimit(0, maxManuals - 1, manual);
    if (stop->primaryManual == manual) return;
    BuildPause buildLock(*this);
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        CrashHandler::note("Stop moved: " + stop->name + " from division " + juce::String(stop->primaryManual) + " to " + juce::String(manual));
        stop->primaryManual = manual;
        for (auto& rank : stop->ranks) rank->primaryManual = manual;
        std::array<int, maxManuals> ordinals{};
        for (auto* other : stops)
            other->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, other->primaryManual)]++;
        impl->ensureRoutesForStops();
        refillStopPipes(stop);   // the other division's compass decides the filled pipes now
    }
    impl->resume();
    buildAntiAliasCopies(stop, true);   // only the copies the new compass still misses
}

Stop* SamplerEngine::addContainerStop(int manual, const juce::String& name)
{
    manual = juce::jlimit(0, maxManuals - 1, manual);
    auto empty = std::make_unique<Stop>(name, juce::File());
    empty->container = true;
    empty->perspective = "default";
    empty->primaryManual = manual;
    empty->folderHash = (uint32_t)name.hashCode();
    empty->tuneTo440 = tuneAllTo440;
    Stop* result = empty.get();
    {
        const juce::ScopedLock sl(controlLock);
        stops.add(empty.release());
        std::array<int, maxManuals> ordinals{};
        for (auto* other : stops)
            other->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, other->primaryManual)]++;
    }
    CrashHandler::note("Stop folder added: " + name + " (division " + juce::String(manual) + ")");
    return result;
}

void SamplerEngine::refillStop(Stop* stop)
{
    if (stop == nullptr) return;
    {
        const juce::ScopedLock sl(controlLock);
        refillStopPipes(stop);
    }
    buildAntiAliasCopies(stop, true);   // launches only when copies are missing
}

void SamplerEngine::swapDivisions(int a, int b)
{
    a = juce::jlimit(0, maxManuals - 1, a);
    b = juce::jlimit(0, maxManuals - 1, b);
    if (a == b) return;
    BuildPause buildLock(*this);   // the running build stops at its next pipe instead of being waited for
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        auto swapped = [a, b](int manual) { return manual == a ? b : (manual == b ? a : manual); };
        for (auto* stop : stops)
        {
            // The stop (all its ranks) moves to the other division; a transmission
            // into one of the two divisions moves with the division's buttons.
            stop->primaryManual = swapped(stop->primaryManual);
            for (auto& rank : stop->ranks) rank->primaryManual = stop->primaryManual;
            for (auto& t : stop->transmissions) t.manual = swapped(t.manual);
        }
        std::array<int, maxManuals> ordinals{};
        for (auto* stop : stops)
            stop->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, stop->primaryManual)]++;
        impl->ensureRoutesForStops();
        refillAllStops();   // the other division's compass decides the filled pipes now
    }
    impl->resume();
    for (auto* stop : stops) buildAntiAliasCopies(stop, true);
}

void SamplerEngine::unloadManual(int manualIndex, bool freeLater)
{
    BuildPause buildLock(*this);   // the running build stops at its next pipe instead of being waited for
    impl->clearMidi();
    impl->suspend();
    bool removed = false;
    {
        const juce::ScopedLock sl(controlLock);
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        // The stops leave the list now and are freed on a thread of their own
        // (0.8.184): freeing 23 GB of packed samples here took 11 s per organ on
        // the message thread.
        const juce::ScopedLock gl(removedStopsLock);
        for (int i = stops.size(); --i >= 0;)
            if (stops[i]->primaryManual == manualIndex) { removedStops.add(stops.removeAndReturn(i)); removed = true; }
        std::array<int, maxManuals> ordinals{};
        for (auto* stop : stops)
            stop->tremulantOrdinal = ordinals[(size_t)juce::jlimit(0, maxManuals - 1, stop->primaryManual)]++;
    }
    impl->resume();
    if (removed && freeLater) freeRemovedStopsLater();   // a load frees them on its loader thread instead (freeRemovedStopsNow)
}

void SamplerEngine::freeRemovedStopsNow()
{
    // The removed stops freed on the calling thread, one at a time (0.8.185: the
    // four threads of 0.8.184 freeing at once held the heap lock against the message
    // thread for 8 s - its stack in RtlFreeHeap under a StopRow destructor, a text
    // layout, the settings file; the loading window never came up). The heap's
    // freed blocks go back to the system once at the end: no residue in the next
    // RAM figure.
    int count = 0;
    const double started = juce::Time::getMillisecondCounterHiRes();
    for (;;)
    {
        std::unique_ptr<Stop> stop;
        {
            const juce::ScopedLock gl(removedStopsLock);
            if (removedStops.isEmpty()) break;
            stop.reset(removedStops.removeAndReturn(removedStops.size() - 1));
        }
        stop.reset();
        ++count;
    }
    if (count == 0) return;
    MemoryInfo::releaseFreedMemory();
    CrashHandler::note("Freed " + juce::String(count) + " removed stop(s) in " + juce::String(juce::Time::getMillisecondCounterHiRes() - started, 0) + " ms");
}

bool SamplerEngine::hasRemovedStops() const
{
    const juce::ScopedLock gl(removedStopsLock);
    return !removedStops.isEmpty();
}

void SamplerEngine::freeRemovedStopsLater()
{
    // One draining thread at most: a thread already running takes the new stops too
    // (it looks again under the lock before it ends).
    {
        const juce::ScopedLock gl(removedStopsLock);
        if (removedStops.isEmpty() || removedStopsFreeing.load(std::memory_order_acquire) > 0) return;
        removedStopsFreeing.store(1, std::memory_order_release);
    }
    juce::Thread::launch([this]
    {
        lowerCurrentThreadPriority();
        for (;;)
        {
            freeRemovedStopsNow();
            const juce::ScopedLock gl(removedStopsLock);
            if (removedStops.isEmpty()) { removedStopsFreeing.store(0, std::memory_order_release); break; }
        }
    });
}

void SamplerEngine::waitForRemovedStopsFreed()
{
    for (int waited = 0; removedStopsFreeing.load(std::memory_order_acquire) > 0 && waited < 600000; waited += 10)
        juce::Thread::sleep(10);
}

//==============================================================================
namespace
{
    bool folderHasNumberedWavFiles(const juce::File& folder)
    {
        bool foundWav = false;
        for (const auto& name : fileNamesOf(folder))   // disk or archive (0.8.157)
        {
            if (!name.endsWithIgnoreCase(".wav")) continue;
            foundWav = true;
            const auto fileName = name.dropLastCharacters(4);
            if (fileName.length() < 3) return false;
            for (int i = 0; i < 3; ++i)
                if (!juce::CharacterFunctions::isDigit(fileName[i])) return false;
        }
        return foundWav;
    }
}

SamplerEngine::MinimumRamEstimate SamplerEngine::estimateMinimumRam() const
{
    MinimumRamEstimate e;
    struct Item { const Sample* sample; bool release; };
    std::vector<Item> items;
    auto addList = [&items](const std::vector<std::unique_ptr<Sample>>& list, bool release)
    {
        for (const auto& s : list) if (s != nullptr) items.push_back({ s.get(), release });
    };
    // Every rank of a stop (the stop itself is the first, the other perspectives
    // follow in `ranks`): 0.8.151 took the first alone - Doesburg counted 2.3 GB of
    // its 7.2 GB.
    std::function<void(const Stop&)> addStop = [&](const Stop& stop)
    {
        for (const auto& list : stop.samplesByNote) addList(list, false);
        for (const auto& entry : stop.aliasFree) addList(entry.second, false);
        for (const auto& layer : stop.velocityLayers)
        {
            for (const auto& list : layer.samplesByNote) addList(list, false);
            for (const auto& entry : layer.aliasFree) addList(entry.second, false);
        }
        for (const auto& layer : stop.releaseLayers)
        {
            for (const auto& list : layer.samplesByNote) addList(list, true);
            for (const auto& entry : layer.aliasFree) addList(entry.second, true);
        }
        for (const auto& rank : stop.ranks) if (rank != nullptr) addStop(*rank);
    };
    for (const auto* stop : stops) if (stop != nullptr) addStop(*stop);
    e.samplesTotal = (int)items.size();
    for (const auto& item : items) e.bytesNow += (int64_t)item.sample->frameBytes();
    if (items.empty()) return e;
    constexpr double lowRate = 32000.0;   // the lowest "Sample rate on loading" of the load box
    const size_t step = std::max<size_t>(1, items.size() / 160);   // not juce::jmax<size_t>: juce_dsp's SIMD overload takes the explicit argument (macOS build)
    double nowTested = 0.0, minTested = 0.0;
    std::vector<int16_t> scratch;
    for (size_t i = 0; i < items.size(); i += step)
    {
        const Sample& s = *items[i].sample;
        if (s.numFrames < Sample::kPackBlock || s.numChannels < 1 || s.sampleRate <= 0.0) continue;
        const int64_t frames = juce::jmin<int64_t>(s.numFrames, (int64_t)(3.0 * s.sampleRate));
        juce::AudioBuffer<float> audio(s.numChannels, (int)frames);
        for (int c = 0; c < s.numChannels; ++c)
        {
            float* dst = audio.getWritePointer(c);
            if (s.storesFloat) { std::vector<float> scratch32; std::memcpy(dst, s.read32(c, scratch32), (size_t)frames * sizeof(float)); }
            else { const int16_t* x = s.read16(c, scratch); for (int64_t f = 0; f < frames; ++f) dst[f] = (float)x[f] / 32768.0f; }
        }
        double target = juce::jmin(s.sampleRate, lowRate);
        if (items[i].release)
        {
            // The halved releases' rule of decodeSampleFile (0.8.147), at the low rate:
            // treble above 0.46 x the halved rate louder than -18 dB keeps the full rate.
            if (s.sampleRate <= lowRate * 0.5 + 0.5) ++e.releasesAlreadyHalved;
            else if (energyAboveDb(audio, s.sampleRate, 0.46 * lowRate * 0.5) <= -18.0) { target = lowRate * 0.5; ++e.releasesHalved; }
            else ++e.releasesKeptFull;
        }
        const bool convert = std::abs(s.sampleRate - target) > 0.5;
        int64_t outFrames = frames;
        std::unique_ptr<SincResampler> resampler;
        if (convert) { resampler = std::make_unique<SincResampler>(s.sampleRate, target); outFrames = resampler->outputFrames(frames); }
        if (outFrames < Sample::kPackBlock) continue;
        Sample low(s.numChannels, outFrames, target, false);
        std::vector<float> resampled(convert ? (size_t)outFrames : (size_t)0);
        for (int c = 0; c < s.numChannels; ++c)
        {
            const float* src = audio.getReadPointer(c);
            if (convert) { resampler->process(src, frames, resampled.data(), outFrames); src = resampled.data(); }
            int16_t* dst = low.writable16(c);
            for (int64_t f = 0; f < outFrames; ++f) dst[f] = (int16_t)juce::jlimit(-32768, 32767, juce::roundToInt(src[f] * 32768.0f));
        }
        size_t lowBytes = low.compress16();
        if (lowBytes == 0) lowBytes = low.frameBytes();
        // What the tested frames take now: their share of the sample's bytes.
        nowTested += (double)s.frameBytes() * (double)frames / (double)s.numFrames;
        minTested += (double)lowBytes;
        ++e.samplesTested;
    }
    if (nowTested <= 0.0) return e;
    e.ratio = minTested / nowTested;
    e.bytesMinimum = (int64_t)((double)e.bytesNow * e.ratio);
    return e;
}

juce::String SamplerEngine::storageSignature() const
{
    return juce::String(storeSamplesAsFloat ? "32-bit float" : "16-bit") + ", "
        + (resampleOnLoad ? juce::String(juce::roundToInt(storageSampleRate / 1000.0)) + " kHz" : juce::String("the files' own rate"))
        + (releaseHalfRate ? ", releases at half the rate" : "") + (compressSamples ? ", packed" : "");   // float too since 0.8.183
}

void SamplerEngine::loadRankSamples(Stop* rank)
{
    rank->storageSignature = storageSignature();
    loadSamplesIntoStop(rank, rank->folder, false, 99999);
    // The automatic transposition (0.8.158): when the row carries none (no
    // definition, nothing chosen), the smpl pitches tell at what footage the
    // files sound on their own numbers and the footage in the folder's name
    // what they should - Doesburg's "024" (sounding C of key 36) in a folder
    // named "... 8" goes on key 36: -12, as the Hauptwerk definition says too.
    // Without a footage in the name, or without pitches, nothing moves.
    if (!rank->sampleOffsetSet && !rank->samplesUnloaded)
    {
        // The median of (pitch - key) over the pipes, unrounded: an organ at a1 = 409 Hz
        // (Adlington) sounds 1.2 semitones under its footage, which is its pitch, not
        // a transposition - only a difference of two semitones or more (no two organ
        // footages are a semitone apart) moves the files (0.8.160).
        std::vector<double> raws;
        for (int key = 0; key < 128; ++key)
        {
            const auto& variants = rank->samplesByNote[(size_t)key];
            if (variants.empty() || variants[0] == nullptr || !std::isfinite(variants[0]->smplPitch)) continue;
            raws.push_back((double)variants[0]->smplPitch - key);
        }
        std::sort(raws.begin(), raws.end());
        const int measured = (int)raws.size();
        const double medianRaw = measured > 0 ? raws[(size_t)measured / 2] : 0.0;
        int agreeing = 0;
        for (const double raw : raws) if (std::abs(raw - medianRaw) < 1.0) ++agreeing;
        int named = 0;
        const bool nameKnown = footageFromName(displayFolderName(rank->folder.getFileName()), named) || footageFromName(rank->name, named);
        const int difference = juce::roundToInt(medianRaw - named);
        const int sounding = named + difference;
        if (nameKnown && measured >= 3 && agreeing * 10 >= measured * 7 && std::abs(difference) >= 2)
        {
            const int offset = juce::jlimit(-48, 48, rank->sampleOffset + (named - sounding));
            CrashHandler::note("Transposition of \"" + rank->name + "\" (" + rank->folder.getFileName() + "): the files sound at " + footageLabel(sounding - rank->sampleOffset)
                + " on their own numbers, the name says " + footageLabel(named) + " - transposed by " + juce::String(offset));
            rank->sampleOffset = offset;
            rank->sampleOffsetDetected = true;
            for (auto& variants : rank->samplesByNote) variants.clear();
            rank->velocityLayers.clear();
            loadSamplesIntoStop(rank, rank->folder, false, 99999);
        }
        rank->sampleOffsetSet = true;
    }
    for (const auto& relFolder : SampleFiles::childDirectories(rank->folder))   // disk or archive (0.8.157)
    {
        const juce::String relName = relFolder.getFileName();
        if (!relName.startsWithIgnoreCase("rel") || !folderHasNumberedWavFiles(relFolder)) continue;
        int maxHoldMs = extractReleaseHoldMs(relName);
        if (maxHoldMs < 0)
        {
            if (relName.trim().equalsIgnoreCase("rel")) maxHoldMs = 99999;
            else continue;
        }
        loadSamplesIntoStop(rank, relFolder, true, maxHoldMs);
    }
    rank->sortReleaseLayers();
    rank->samplesUnloaded = false;
}

void SamplerEngine::setPooledStopNames(std::map<int, std::set<juce::String>> names)
{
    std::lock_guard<std::mutex> lk(pooledMutex);
    pooledStopNames = std::move(names);
}

bool SamplerEngine::isPooledStop(int manual, const juce::String& name)
{
    std::lock_guard<std::mutex> lk(pooledMutex);
    const auto it = pooledStopNames.find(manual);
    return it != pooledStopNames.end() && it->second.count(name) != 0;
}

void SamplerEngine::unloadStopSamples(Stop* stop)
{
    if (stop == nullptr || stop->samplesUnloaded) return;
    BuildPause buildLock(*this);   // the running build stops at its next pipe instead of being waited for
    impl->clearMidi();
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        // No voice may keep pointing at freed sample data.
        impl->killAllVoices();
        impl->resetAllVoicesUnsafe();
        auto release = [](Stop* rank)
        {
            for (auto& variants : rank->samplesByNote) { variants.clear(); variants.shrink_to_fit(); }
            rank->velocityLayers.clear();
            rank->velocityLayers.shrink_to_fit();
            rank->releaseLayers.clear();
            rank->releaseLayers.shrink_to_fit();
            rank->aliasFree.clear();
            rank->aliasFreeOffset.clear();
            rank->retiredSamples.clear();
            rank->speaking.reset();
            rank->samplesUnloaded = true;
        };
        release(stop);
        for (auto& rank : stop->ranks) release(rank.get());
    }
    impl->resume();
}

void SamplerEngine::setStopSampleOffset(Stop* stop, int offset)
{
    if (stop == nullptr || stop->container) return;
    offset = juce::jlimit(-48, 48, offset);
    if (stop->sampleOffset == offset) return;
    {
        const juce::ScopedLock sl(controlLock);
        stop->sampleOffset = offset;
        stop->sampleOffsetSet = true;
        for (auto& rank : stop->ranks) { rank->sampleOffset = offset; rank->sampleOffsetSet = true; }
    }
    CrashHandler::note("Transposition of \"" + stop->name + "\" set to " + juce::String(offset) + ": its samples are loaded again on their keys");
    reloadStopSamples(stop, true);
}

void SamplerEngine::setStopOneShot(Stop* stop, int manual)
{
    if (stop == nullptr) return;
    stop->oneShotManual = juce::jlimit(-1, 1, manual);
    updateOneShotFlags();
}

void SamplerEngine::reloadStopSamples(Stop* stop, bool evenIfLoaded)
{
    if (stop == nullptr || (!stop->samplesUnloaded && !evenIfLoaded)) return;
    BuildPause buildLock(*this);   // a background build may still read the old samples (it stops at its next pipe)
    loadOutOfMemory.store(false, std::memory_order_relaxed);
    // No new voice of this stop while its samples are decoded anew (0.8.160: the
    // keys kept playing between the kill below and the swap, and those voices read
    // the old samples when they were freed - the 0.8.159 crash in renderVoiceSpans).
    stop->reloading.store(true, std::memory_order_release);
    if (evenIfLoaded)
    {
        // The stop sounds, maybe: its voices read the samples that are about to go
        // (0.8.155 crashed in renderVoiceSpans when "Files vs keys" changed while a
        // key was held). Every voice is killed first, as removing a stop does.
        impl->clearMidi();
        impl->suspend();
        {
            const juce::ScopedLock sl(controlLock);
            impl->killAllVoices();
            impl->resetAllVoicesUnsafe();
        }
        impl->resume();
    }
    // Decode into detached stand-ins first, then swap the sample lists in while
    // the engine is suspended; the live Stop objects stay where they are.
    std::vector<std::pair<Stop*, std::unique_ptr<Stop>>> staged;
    staged.emplace_back(stop, std::make_unique<Stop>(stop->name, stop->folder));
    for (auto& rank : stop->ranks) staged.emplace_back(rank.get(), std::make_unique<Stop>(rank->name, rank->folder));
    for (auto& entry : staged)
    {
        entry.second->sampleOffset = entry.first->sampleOffset;
        entry.second->sampleOffsetSet = true;   // a reload keeps the transposition as it is
        loadRankSamples(entry.second.get());
    }
    impl->suspend();
    {
        const juce::ScopedLock sl(controlLock);
        // Whatever started meanwhile (0.8.160) - of a stop that sounded. A stop whose
        // samples were unloaded (a pooled stop back on the grid, a transmission added to
        // it) has no voice reading them: 0.8.183 no longer cuts every held tone of the
        // whole organ for those callers.
        if (evenIfLoaded)
        {
            impl->killAllVoices();
            impl->resetAllVoicesUnsafe();
        }
        for (auto& entry : staged)
        {
            Stop* live = entry.first;
            live->samplesByNote = std::move(entry.second->samplesByNote);
            live->velocityLayers = std::move(entry.second->velocityLayers);
            live->releaseLayers = std::move(entry.second->releaseLayers);
            live->storageSignature = entry.second->storageSignature;
            // The filled pipes' copies were made for the old keys: all of them anew.
            live->aliasFree.clear();
            live->aliasFreeOffset.clear();
            live->nextSampleIndex.fill(0);
            for (int note = 0; note < 128; ++note)
                if (live->hasNote(note) && std::abs(live->getNoteDb(note)) < 1.0e-6f) live->setNoteDb(note, 0.0f);
            live->samplesUnloaded = false;
        }
        // Tuning data of the whole division again (the organ diapason is shared).
        std::vector<Stop*> ranks;
        for (auto* s : stops)
            if (s->primaryManual == stop->primaryManual)
            {
                ranks.push_back(s);
                for (auto& rank : s->ranks) ranks.push_back(rank.get());
            }
        computeTuning440(ranks);
        refillStopPipes(stop);
    }
    stop->reloading.store(false, std::memory_order_release);
    impl->resume();
    buildAntiAliasCopies(stop);
    if (loadOutOfMemory.load(std::memory_order_relaxed))
        lastLoadError = "The computer ran out of memory while reloading\n" + stop->folder.getFullPathName()
            + "\n\nOnly part of the stop is available. Unload another division or close other programs.";
}

void SamplerEngine::setVelocityLayerEnabled(Stop* stop, int maxVelocity, bool enabled)
{
    // maxVelocity 128 (or 0): the plain samples, the top layer.
    if (stop == nullptr || maxVelocity < 0 || maxVelocity > 128) return;
    const size_t bit = maxVelocity == 128 ? Stop::plainLayerBit : (size_t)maxVelocity;
    const juce::ScopedLock sl(controlLock);   // startNote reads the bits under this lock
    stop->velocityLayerDisabled.set(bit, !enabled);
}

void SamplerEngine::setPipeEnabled(Stop* stop, int pipe, bool enabled)
{
    if (stop == nullptr || pipe < 0 || pipe >= 128) return;
    if (stop->pipeDisabled.test((size_t)pipe) == !enabled) return;
    // No background build may read what is freed; the running build is interrupted
    // at its next pipe rather than waited for (the voicing must not freeze).
    BuildPause buildLock(*this);
    std::vector<Stop*> ranks{ stop };
    for (auto& rank : stop->ranks) ranks.push_back(rank.get());
    if (!enabled && !stop->transmissions.empty())
    {
        // The stop's own button falls silent on this key; its transmissions may
        // still play the pipe, so nothing is freed and the fill map stays.
        const juce::ScopedLock sl(controlLock);
        stop->pipeDisabled.set((size_t)pipe);
        return;
    }
    if (!enabled)
    {
        // A real pipe keeps its samples (they are wanted again the moment the
        // pipe is switched back on; the saving would be small). Only the
        // anti-alias copies of an extension are freed - voices may still read
        // them, so the engine is silenced first, like when a stop leaves for the pool.
        bool copies = false;
        for (auto* rank : ranks)
        {
            copies = copies || rank->aliasFree.count(pipe) > 0;
            for (const auto& layer : rank->velocityLayers) copies = copies || layer.aliasFree.count(pipe) > 0;
            for (const auto& layer : rank->releaseLayers) copies = copies || layer.aliasFree.count(pipe) > 0;
        }
        if (copies) impl->clearMidi();
        impl->suspend();
        {
            const juce::ScopedLock sl(controlLock);
            stop->pipeDisabled.set((size_t)pipe);
            if (copies)
            {
                impl->killAllVoices();
                impl->resetAllVoicesUnsafe();
            }
            for (auto* rank : ranks)
            {
                rank->aliasFree.erase(pipe);
                rank->aliasFreeOffset.erase(pipe);
                for (auto& layer : rank->velocityLayers) { layer.aliasFree.erase(pipe); layer.aliasFreeOffset.erase(pipe); }
                for (auto& layer : rank->releaseLayers) { layer.aliasFree.erase(pipe); layer.aliasFreeOffset.erase(pipe); }
                if (copies) rank->retiredSamples.clear();   // no voice reads them any more
                rank->speaking.reset((size_t)pipe);
            }
            stop->pipeStartMs[(size_t)pipe] = -1.0;
            refillStopPipes(stop);   // the pipe is no longer filled; nothing else changes
        }
        impl->resume();
        return;
    }
    // Switching on: the pipe's own samples are still there; only the fill map
    // (and the anti-alias copies of a filled pipe) are recomputed.
    {
        const juce::ScopedLock sl(controlLock);
        stop->pipeDisabled.reset((size_t)pipe);
        refillStopPipes(stop);
    }
    buildAntiAliasCopies(stop, true);
}

void SamplerEngine::setTransmissionKeyEnabled(Stop* stop, int index, int key, bool enabled)
{
    if (stop == nullptr || index < 0 || index >= (int)stop->transmissions.size() || key < 0 || key >= 128) return;
    const juce::ScopedLock sl(controlLock);
    auto& t = stop->transmissions[(size_t)index];
    if (enabled) t.keyDisabled.reset((size_t)key);
    else t.keyDisabled.set((size_t)key);   // a key sounding now finishes when it is released
}

void SamplerEngine::setStopTuneTo440(Stop* stop, bool enabled)
{
    if (stop == nullptr) return;
    const juce::ScopedLock sl(controlLock);
    stop->tuneTo440 = enabled;
    for (auto& rank : stop->ranks) rank->tuneTo440 = enabled;
    // Pipes already sounding (and their releases) follow at once; the render
    // interpolates the increment across the block, so there is no click.
    for (const uint32_t idx : impl->controlActive)
    {
        auto& v = impl->voices[idx];
        if (v.stop == stop && v.rank != nullptr)
        {
            v.tuningCents.store(v.rank->tuningCentsFor((int)v.sampleNote, enabled), std::memory_order_relaxed);
            v.tempMode.store(enabled ? 1 : (v.rank->measuredTemperamentValid ? 2 : 0), std::memory_order_relaxed);
            if (v.shift != 0 || v.sampleNote != v.note)   // the pure-interval correction depends on the mode
                v.shiftCents.store(Impl::transpositionCents(stop, v.rank, v.shift, (int)v.note, (int)v.sampleNote),
                    std::memory_order_relaxed);
        }
    }
}

void SamplerEngine::setStopUndulation(Stop* stop, float beatsAtA1)
{
    if (stop == nullptr) return;
    const juce::ScopedLock sl(controlLock);
    beatsAtA1 = juce::jlimit(-7.0f, 7.0f, beatsAtA1);
    const float ratio = undulationBeatRatio.load(std::memory_order_relaxed);
    computeUndulationTable(stop, beatsAtA1, ratio);
    for (auto& rank : stop->ranks) computeUndulationTable(rank.get(), beatsAtA1, ratio);
    for (const uint32_t idx : impl->controlActive)
    {
        auto& v = impl->voices[idx];
        if (v.stop == stop && v.rank != nullptr)
            v.tuningCents.store(v.rank->tuningCentsFor((int)v.sampleNote, stop->tuneTo440), std::memory_order_relaxed);
    }
}

namespace
{
    // The lowest harmonic of f0 that really sounds in a pipe's sustain. A
    // mixture's smpl pitch names the key's fundamental, which its ranks
    // (2 2/3', 2', ...) do not produce; the stop's name then shows the lowest
    // sounding rank. Every harmonic k*f0 (k = 1..16) is measured in a band of
    // +-1.5 % of a Hann-windowed FFT of the sustain; it sounds when it is within
    // 34 dB of the strongest harmonic (strings keep their weak fundamental: a
    // Viola di Gamba measured 27 dB down; an absent mixture fundamental was
    // 58 dB down; a reed's absent fundamental under a complete series is kept,
    // see below) and stands
    // 12 dB above the spectrum between the harmonics (room rumble is no
    // fundamental). 'levels' (when given) receives the levels below the strongest.
    int lowestSoundingHarmonic(const Sample& sample, double f0Hz, std::array<float, 16>* levels, float* residualDb = nullptr)
    {
        if (levels != nullptr) levels->fill(0.0f);
        if (residualDb != nullptr) *residualDb = 0.0f;
        if (sample.numChannels < 1 || sample.numFrames < 1 || !(f0Hz > 1.0) || sample.sampleRate <= 0.0) return 1;
        const double sr = sample.sampleRate;
        // The sustain: after the attack (300 ms), the longest power-of-two stretch
        // up to 65536 frames; a short sample gives 4096 frames after a shorter skip.
        int64_t start = (int64_t)(0.3 * sr);
        int order = 16;
        while (order >= 12 && start + ((int64_t)1 << order) > sample.numFrames) --order;
        if (order < 12)
        {
            start = (int64_t)(0.05 * sr);
            order = 12;
            if (start + ((int64_t)1 << order) > sample.numFrames) return 1;
        }
        const int n = 1 << order;
        std::vector<float> buffer((size_t)n * 2, 0.0f);
        std::vector<int16_t> scratch;
        std::vector<float> scratch32;
        const int16_t* x16 = sample.storesFloat ? nullptr : sample.read16(0, scratch);
        const float* x32 = sample.storesFloat ? sample.read32(0, scratch32) : nullptr;
        for (int i = 0; i < n; ++i)   // channel 0 (every channel carries the same partials)
        {
            // Blackman-Harris window: side lobes below -92 dB, so the spectrum between
            // the harmonics shows the real noise and aliasing, not window leakage.
            const float phase = juce::MathConstants<float>::twoPi * (float)i / (float)(n - 1);
            const float w = 0.35875f - 0.48829f * std::cos(phase) + 0.14128f * std::cos(2.0f * phase) - 0.01168f * std::cos(3.0f * phase);
            const float v = sample.storesFloat ? x32[start + i] : (float)x16[start + i] / 32768.0f;
            buffer[(size_t)i] = v * w;
        }
        juce::dsp::FFT fft(order);
        fft.performFrequencyOnlyForwardTransform(buffer.data(), true);
        const double binHz = sr / (double)n;
        auto magnitudeDb = [&](int bin) { return 20.0 * std::log10((double)buffer[(size_t)juce::jlimit(0, n / 2, bin)] + 1.0e-9); };
        struct Band { bool valid = false; double peakDb = -200.0, floorDb = -200.0; };
        std::array<Band, 16> bands;
        double strongest = -200.0;
        for (int k = 1; k <= 16; ++k)
        {
            const double centre = k * f0Hz;
            if (centre >= 0.45 * sr) break;
            const int centreBin = (int)std::round(centre / binHz);
            const int halfWidth = juce::jmax(4, (int)std::ceil(centre * 0.015 / binHz));   // >= the window's main lobe
            Band band;
            band.valid = true;
            for (int bin = centreBin - halfWidth; bin <= centreBin + halfWidth; ++bin)
                band.peakDb = juce::jmax(band.peakDb, magnitudeDb(bin));
            // The floor: the median level between this harmonic and its neighbours.
            std::vector<double> around;
            const int from = juce::jmax(1, (int)std::round((k - 0.5) * f0Hz / binHz));
            const int to = (int)std::round((k + 0.5) * f0Hz / binHz);
            for (int bin = from; bin <= to; ++bin)
                if (std::abs(bin - centreBin) > halfWidth) around.push_back(magnitudeDb(bin));
            if (around.size() >= 3) { std::sort(around.begin(), around.end()); band.floorDb = around[around.size() / 2]; }
            bands[(size_t)(k - 1)] = band;
            strongest = juce::jmax(strongest, band.peakDb);
        }
        // Everything between the harmonics (from half f0 up to the last harmonic
        // measured) against the harmonics themselves: the noise floor, hum and
        // any aliasing or imaging a resampling left behind.
        if (residualDb != nullptr)
        {
            int lastValid = 0;
            for (int k = 1; k <= 16; ++k) if (bands[(size_t)(k - 1)].valid) lastValid = k;
            if (lastValid > 0)
            {
                double inBand = 0.0, offBand = 0.0;
                const int from = juce::jmax(1, (int)std::round(0.5 * f0Hz / binHz));
                const int to = juce::jmin(n / 2, (int)std::round((lastValid + 0.5) * f0Hz / binHz));
                for (int bin = from; bin <= to; ++bin)
                {
                    const double power = (double)buffer[(size_t)bin] * (double)buffer[(size_t)bin];
                    const double harmonic = (double)bin * binHz / f0Hz;
                    const int k = (int)std::round(harmonic);
                    const int hw = juce::jmax(4, (int)std::ceil(k * f0Hz * 0.015 / binHz));
                    if (k >= 1 && k <= lastValid && std::abs(bin - (int)std::round(k * f0Hz / binHz)) <= hw) inBand += power;
                    else offBand += power;
                }
                *residualDb = (float)(10.0 * std::log10((offBand + 1.0e-20) / (inBand + 1.0e-20)));
            }
        }
        int sounding = 1;
        bool found = false;
        for (int k = 1; k <= 16; ++k)
        {
            const auto& band = bands[(size_t)(k - 1)];
            if (!band.valid) break;
            if (levels != nullptr) (*levels)[(size_t)(k - 1)] = (float)(band.peakDb - strongest);
            if (!found && band.peakDb >= strongest - 34.0 && band.peakDb >= band.floorDb + 12.0) { sounding = k; found = true; }
        }
        // A missing fundamental under a complete harmonic series is still heard
        // as the fundamental (a 16' Bombarde measured 87 dB down at 32 Hz, a 32'
        // Posaune 58 dB down, with every harmonic above in place): such a stop
        // keeps its footage. A compound stop has gaps in the series (a mixture
        // of 2 2/3' and 2' has the 3rd and 4th harmonic but not the 5th), so the
        // four harmonics above the lowest sounding one are all there only for a
        // real series.
        if (sounding > 1)
        {
            bool completeSeries = true;
            for (int k = sounding + 1; k <= sounding + 4 && completeSeries; ++k)
                completeSeries = k <= 16 && bands[(size_t)(k - 1)].valid && bands[(size_t)(k - 1)].peakDb >= strongest - 34.0;
            if (completeSeries) sounding = 1;
        }
        return sounding;
    }
}

void SamplerEngine::computeTuning440(const std::vector<Stop*>& ranksOfOrgan)
{
    // Footage offsets (semitones above the written key) available to organ
    // ranks: harmonics m * 2^k with m in {1, 3, 5, 7, 9} - octaves (32', 16',
    // 8', 4', 2', 1'), fifths (5 1/3', 2 2/3', 1 1/3'), thirds (3 1/5', 1 3/5'),
    // sevenths (1 1/7') and ninths (8/9'). Breaks inside a rank simply switch a
    // range of notes to another offset.
    struct Candidate { double semitones; bool common; };
    std::vector<Candidate> candidates;
    // 64' (-36 st) up to 1/16' (+84 st) for octaves and fifths, thirds up to
    // 2/5' (+52 st), sevenths and ninths up to about 1' pitch. 10 2/3', 6 2/5',
    // 4/5', 2 2/7' etc. are all harmonics m * 2^k of some octave.
    for (const int m : { 1, 3, 5, 7, 9 })
    {
        const double maxSt = m <= 3 ? 85.0 : (m == 5 ? 53.0 : 45.0);
        for (int k = -8; k <= 8; ++k)
        {
            const double st = 12.0 * std::log2((double)m) + 12.0 * k;
            if (st >= -40.0 && st <= maxSt) candidates.push_back({ st, m <= 5 });
        }
    }
    auto nearest = [&](double value, bool commonOnly)
    {
        const Candidate* best = nullptr;
        for (const auto& c : candidates)
            if ((!commonOnly || c.common) && (best == nullptr || std::abs(c.semitones - value) < std::abs(best->semitones - value)))
                best = &c;
        return best;
    };
    auto median = [](std::vector<double> values)
    {
        if (values.empty()) return 0.0;
        std::sort(values.begin(), values.end());
        const size_t n = values.size();
        return n % 2 == 1 ? values[n / 2] : 0.5 * (values[n / 2 - 1] + values[n / 2]);
    };

    struct NoteData { int note; double raw; };   // raw = recorded pitch - written key (semitones)
    struct RankData { Stop* rank; std::vector<NoteData> notes; bool valid = false; };
    std::vector<RankData> data;
    for (auto* rank : ranksOfOrgan)
    {
        RankData rd{ rank, {}, false };
        for (int note = 0; note < 128; ++note)
        {
            double sum = 0.0;
            int count = 0;
            for (const auto& sample : rank->samplesByNote[(size_t)note])
                if (sample != nullptr && std::isfinite(sample->smplPitch)) { sum += sample->smplPitch; ++count; }
            // The REAL footage of the files (0.8.158): recorded pitch minus the file's
            // own number (= the key plus the stop's transposition), so a transposed
            // stop measures what its samples are, not where they were put.
            if (count > 0) rd.notes.push_back({ note, sum / count - (note + rank->sampleOffset) });
        }
        // Usable only when the chunks carry measured pitches: enough notes, not
        // all "unity = key, fraction 0", and not one constant unity note.
        if (rd.notes.size() >= 3)
        {
            bool allZero = true, allSameAbsolute = true;
            const double firstAbsolute = rd.notes[0].raw + rd.notes[0].note;
            for (const auto& n : rd.notes)
            {
                if (std::abs(n.raw) > 1.0e-6) allZero = false;
                if (std::abs(n.raw + n.note - firstAbsolute) > 1.0e-6) allSameAbsolute = false;
            }
            rd.valid = !allZero && !allSameAbsolute;
        }
        data.push_back(std::move(rd));
    }

    // 1. The organ's diapason, from the common footages only (octaves, fifths,
    //    thirds): their offsets are at least 3.1 semitones apart, so a residual
    //    within +-1.4 semitones identifies the footage even when the organ is
    //    tuned a semitone away from 440 Hz (Chorton, 415 Hz sets...).
    std::vector<double> organResiduals;
    for (const auto& rd : data)
        if (rd.valid)
            for (const auto& n : rd.notes)
                if (const auto* c = nearest(n.raw, true))
                {
                    const double r = n.raw - c->semitones;
                    if (std::abs(r) <= 1.4) organResiduals.push_back(r);
                }
    const double organ = organResiduals.size() >= 5 ? median(organResiduals) : 0.0;

    // 2. Per rank: the footage (or break) offset of every note relative to the
    //    organ's diapason, isolated mistuned pipes smoothed, then the corrections.
    for (auto& rd : data)
    {
        Stop* rank = rd.rank;
        rank->tuningInfoAvailable = rd.valid;
        rank->measuredDiapasonHz = 0.0f;
        rank->tuning440Cents.fill(0.0f);
        if (!rd.valid) continue;
        std::vector<double> offsets(rd.notes.size(), 0.0);
        for (size_t i = 0; i < rd.notes.size(); ++i)
            if (const auto* c = nearest(rd.notes[i].raw - organ, false)) offsets[i] = c->semitones;
        // A break needs at least three pipes in a row at the new footage (the
        // user's rule, 0.8.159): a run of fewer than three equal offsets is a
        // mistuned pipe (or a wrong unity note), not a break, wherever it lies -
        // at the start or the end of the compass too (Adlington's "Ch. Flute": the
        // B and C# at the bottom carry pitches a third low, and the C between them
        // was swallowed into their "break", so every C came out a semitone off).
        // It takes the value of its longer neighbouring run.
        for (int pass = 0; pass < 8; ++pass)
        {
            bool changed = false;
            size_t i = 0;
            while (i < offsets.size())
            {
                size_t j = i;
                while (j < offsets.size() && std::abs(offsets[j] - offsets[i]) < 1.0e-9) ++j;
                if (j - i < 3 && j - i < offsets.size())
                {
                    size_t previousLength = 0, nextLength = 0;
                    for (size_t p = i; p > 0 && std::abs(offsets[p - 1] - offsets[i - 1]) < 1.0e-9; --p) ++previousLength;
                    for (size_t n = j; n < offsets.size() && std::abs(offsets[n] - offsets[j]) < 1.0e-9; ++n) ++nextLength;
                    if (previousLength > 0 || nextLength > 0)
                    {
                        const double value = previousLength >= nextLength && i > 0 ? offsets[i - 1] : (j < offsets.size() ? offsets[j] : offsets[i - 1]);
                        if (std::abs(value - offsets[i]) > 1.0e-9)
                        {
                            for (size_t k = i; k < j; ++k) offsets[k] = value;
                            changed = true;
                        }
                    }
                }
                i = j;
            }
            if (!changed) break;
        }
        // Residual of every measured note relative to the organ's diapason (cents).
        std::vector<double> residualCents(rd.notes.size()), trendCents(rd.notes.size(), 0.0);
        for (size_t i = 0; i < rd.notes.size(); ++i)
            residualCents[i] = (rd.notes[i].raw - offsets[i] - organ) * 100.0;
        // Celeste detection: relative to the organ's diapason every pipe of an
        // undulating rank is sharp (or flat) by a clearly audible amount that
        // shrinks towards the treble. Random tuning jitter has mixed signs and
        // no trend, so it is not mistaken for undulation. The detune trend is
        // fitted (|cents| = exp(p + q*key)) and removed before the rank's own
        // diapason and temperament are measured.
        rank->detectedCelesteBeats = 0.0f;
        if (rd.notes.size() >= 8)
        {
            // Single mistuned pipes (a wrong unity note, a pipe a semitone off) must
            // not spoil the trend test: notes whose detune lies far from the rank's
            // typical detune (median +- 4 MADs, at least 6 cents) are left out of
            // the statistics; the fitted trend is applied to every note afterwards.
            std::vector<double> absAll;
            for (const double cents : residualCents) absAll.push_back(std::abs(cents));
            const double medianAbs = median(absAll);
            std::vector<double> deviations;
            for (const double a : absAll) deviations.push_back(std::abs(a - medianAbs));
            const double window = juce::jmax(6.0, 4.0 * median(deviations));
            std::vector<char> typical(rd.notes.size(), 1);
            for (size_t i = 0; i < rd.notes.size(); ++i)
                typical[i] = std::abs(absAll[i] - medianAbs) <= window ? 1 : 0;

            int positive = 0, negative = 0;
            double sumAbs = 0.0, sumNote = 0.0, sumAbsNote = 0.0, sumNote2 = 0.0, sumAbs2 = 0.0, count = 0.0;
            for (size_t i = 0; i < rd.notes.size(); ++i)
            {
                if (!typical[i]) continue;
                const double cents = residualCents[i];
                if (cents > 0.0) ++positive; else ++negative;
                const double a = std::abs(cents), n = (double)rd.notes[i].note;
                sumAbs += a; sumNote += n; sumAbsNote += a * n; sumNote2 += n * n; sumAbs2 += a * a;
                count += 1.0;
            }
            const double meanAbs = count > 0.0 ? sumAbs / count : 0.0;
            const double sameSign = count > 0.0 ? (double)juce::jmax(positive, negative) / count : 0.0;
            const double cov = count > 0.0 ? sumAbsNote / count - (sumNote / count) * meanAbs : 0.0;
            const double varNote = count > 0.0 ? sumNote2 / count - (sumNote / count) * (sumNote / count) : 0.0;
            const double varAbs = count > 0.0 ? sumAbs2 / count - meanAbs * meanAbs : 0.0;
            const double correlation = varNote > 0.0 && varAbs > 0.0 ? cov / std::sqrt(varNote * varAbs) : 0.0;
            if (count >= 8.0 && meanAbs >= 5.0 && sameSign >= 0.85 && correlation < -0.3)
            {
                double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
                int n = 0;
                for (size_t i = 0; i < rd.notes.size(); ++i)
                {
                    if (!typical[i]) continue;
                    const double a = std::abs(residualCents[i]);
                    if (a < 0.5) continue;
                    const double x = (double)rd.notes[i].note, y = std::log(a);
                    sx += x; sy += y; sxx += x * x; sxy += x * y; ++n;
                }
                double p = std::log(meanAbs), q = 0.0;
                if (n >= 3 && n * sxx - sx * sx > 1.0e-9)
                {
                    q = (n * sxy - sx * sy) / (n * sxx - sx * sx);
                    p = (sy - q * sx) / n;
                }
                const double sign = positive >= negative ? 1.0 : -1.0;
                for (size_t i = 0; i < rd.notes.size(); ++i) trendCents[i] = sign * std::exp(p + q * rd.notes[i].note);
                const double centsAtA1 = sign * std::exp(p + q * 69.0);
                const double footage = offsets.empty() ? 0.0 : offsets[offsets.size() / 2];
                const double fA1 = 440.0 * std::pow(2.0, footage / 12.0);
                rank->detectedCelesteBeats = (float)(fA1 * (std::pow(2.0, centsAtA1 / 1200.0) - 1.0));
            }
        }
        // The rank's own diapason: the pipes with the undulation trend removed
        // (a celeste's real mean pitch is the organ's, not its detuned one).
        std::vector<double> detrended(rd.notes.size());
        for (size_t i = 0; i < rd.notes.size(); ++i) detrended[i] = (residualCents[i] - trendCents[i]) / 100.0;
        const double rankDiapason = organ + median(detrended);   // semitones off 440
        rank->measuredDiapasonHz = (float)(440.0 * std::pow(2.0, rankDiapason / 12.0));
        // Notes without pitch data get the rank's diapason shift; measured notes
        // go exactly to (key + offset) in equal temperament at a1 = 440 Hz.
        for (auto& cents : rank->tuning440Cents) cents = (float)juce::jlimit(-200.0, 200.0, -rankDiapason * 100.0);
        // The retune limit (0.8.165): a note further off than it is a faulty sample
        // (never played, filled from the neighbours - computeFilledPipes); the
        // correction is clamped to the limit, the raw deviation kept for a new limit.
        const double limit = (double)gRetuneLimitCents.load(std::memory_order_relaxed);
        rank->faultyPipes.reset();
        rank->rawTuningCents.fill(0.0f);
        for (size_t i = 0; i < rd.notes.size(); ++i)
        {
            const double cents = -(rd.notes[i].raw - offsets[i]) * 100.0;
            rank->rawTuningCents[(size_t)rd.notes[i].note] = (float)cents;
            if (std::abs(cents) > limit) rank->faultyPipes.set((size_t)rd.notes[i].note);
            rank->tuning440Cents[(size_t)rd.notes[i].note] = (float)juce::jlimit(-limit, limit, cents);
        }
        // Footage of the rank: the most common offset (mixtures with breaks
        // report their lowest, most frequent rank).
        {
            std::map<int, int> offsetCounts;
            for (const double offset : offsets) ++offsetCounts[juce::roundToInt(offset)];
            int bestCount = 0;
            for (const auto& entry : offsetCounts)
                if (entry.second > bestCount) { bestCount = entry.second; rank->footageSemitones = entry.first; }
            rank->footageKnown = bestCount > 0;
            rank->lowestFootageSemitones = offsets.empty() ? rank->footageSemitones : juce::roundToInt(offsets.front());
            // Breaks: every change of the footage along the compass, e.g. "C 2' | c1 4' | c3 8'".
            rank->footageBreaks.clear();
            auto helmholtz = [](int midi)
            {
                static const char* names[12]{ "C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "B", "H" };
                juce::String name(names[((midi % 12) + 12) % 12]);
                if (midi >= 48)   // small octave and up: lower case, c1 = 60
                {
                    name = name.toLowerCase();
                    if (midi >= 60) name << juce::String((midi - 60) / 12 + 1);
                }
                else if (midi < 36) name << juce::String((35 - midi) / 12 + 1);   // contra octaves: C1, C2
                return name;
            };
            if (offsetCounts.size() > 1)
            {
                int previous = std::numeric_limits<int>::min();
                for (size_t i = 0; i < rd.notes.size(); ++i)
                {
                    const int st = juce::roundToInt(offsets[i]);
                    if (st == previous) continue;
                    previous = st;
                    if (rank->footageBreaks.isNotEmpty()) rank->footageBreaks << " | ";
                    rank->footageBreaks << helmholtz(rd.notes[i].note) << " "
                        << footageName(st).upToFirstOccurrenceOf(" (", false, false);
                }
            }
            // The footage the pipes really sound at: the lowest sounding harmonic of
            // the lowest pipes (the median of up to three) of every footage segment.
            // Measured once per rank from the samples in memory; the name shows the
            // sounding footage of the lowest segment, the voicing both footages.
            // The offset is per key, so the footage is the same wherever the rank
            // starts (a 4' rank beginning at c1 is still 4').
            if (!rank->soundingMeasured)
            {
                rank->soundingMeasured = true;
                rank->lowestNoteHarmonicDb.fill(0.0f);
                rank->soundingHarmonic = 1;
                juce::String text;
                int previous = std::numeric_limits<int>::min();
                bool firstSegment = true;
                for (size_t i = 0; i < rd.notes.size(); ++i)
                {
                    const int st = juce::roundToInt(offsets[i]);
                    if (st == previous) continue;
                    previous = st;
                    std::vector<int> harmonics;
                    for (size_t j = i; j < rd.notes.size() && harmonics.size() < 3 && juce::roundToInt(offsets[j]) == st; ++j)
                    {
                        const auto& variants = rank->samplesByNote[(size_t)rd.notes[j].note];
                        if (variants.empty() || variants[0] == nullptr || !std::isfinite(variants[0]->smplPitch)) continue;
                        const double f0 = 440.0 * std::pow(2.0, ((double)variants[0]->smplPitch - 69.0) / 12.0);
                        const bool lowestPipe = firstSegment && harmonics.empty();
                        harmonics.push_back(lowestSoundingHarmonic(*variants[0], f0,
                            lowestPipe ? &rank->lowestNoteHarmonicDb : nullptr, lowestPipe ? &rank->lowestNoteResidualDb : nullptr));
                    }
                    int harmonic = 1;
                    if (!harmonics.empty()) { std::sort(harmonics.begin(), harmonics.end()); harmonic = harmonics[harmonics.size() / 2]; }
                    // 0.8.225 (the user's rule, Billerbeck "P. Grosse flute 32" and "P. Bourdon 32" captioned 16'):
                    // below 45 Hz a recording loses the fundamental (the 32' C of both: h1 47-59 dB under h2,
                    // whatever the FFT size), while a fifth higher the same rank has h1 30-50 dB ABOVE h2. So
                    // when the lowest pipes of the first segment say "harmonic > 1" and the lowest sounds below
                    // 45 Hz, the pipes +6, +7, +9, +10 and +12 semitones up in the SAME smpl segment answer one
                    // binary question only: do they have a clear fundamental? Two of them -> the rank has one and
                    // the deep pipes merely lost it -> harmonic 1. A mixture never shows h1 there (its hidden
                    // breaks change 3 to 2, never to 1), so k is only ever turned into 1, never 3 into 2.
                    if (firstSegment && harmonic > 1)
                    {
                        double lowestHz = 0.0;
                        for (size_t j = i; j < rd.notes.size() && juce::roundToInt(offsets[j]) == st && lowestHz <= 0.0; ++j)
                        {
                            const auto& variants = rank->samplesByNote[(size_t)rd.notes[j].note];
                            if (!variants.empty() && variants[0] != nullptr && std::isfinite(variants[0]->smplPitch))
                                lowestHz = 440.0 * std::pow(2.0, ((double)variants[0]->smplPitch - 69.0) / 12.0);
                        }
                        if (lowestHz > 0.0 && lowestHz < 45.0)
                        {
                            int confirmed = 0, looked = 0;
                            for (const int delta : { 6, 7, 9, 10, 12 })
                            {
                                const int note = rd.notes[i].note + delta;
                                bool inSegment = false;
                                for (size_t j = i; j < rd.notes.size(); ++j)
                                    if (rd.notes[j].note == note) { inSegment = juce::roundToInt(offsets[j]) == st; break; }
                                if (!inSegment || note < 0 || note >= 128) continue;
                                const auto& variants = rank->samplesByNote[(size_t)note];
                                if (variants.empty() || variants[0] == nullptr || !std::isfinite(variants[0]->smplPitch)) continue;
                                ++looked;
                                const double f0 = 440.0 * std::pow(2.0, ((double)variants[0]->smplPitch - 69.0) / 12.0);
                                if (lowestSoundingHarmonic(*variants[0], f0, nullptr, nullptr) == 1) ++confirmed;
                            }
                            if (confirmed >= 2)
                            {
                                CrashHandler::note("Tuning " + rank->name + (rank->perspective.isNotEmpty() ? " [" + rank->perspective + "]" : juce::String())
                                    + ": the lowest pipes (" + juce::String(lowestHz, 1) + " Hz) measured harmonic " + juce::String(harmonic)
                                    + " as the first sounding one, but " + juce::String(confirmed) + " of " + juce::String(looked)
                                    + " pipe(s) up to an octave above have a clear fundamental - the recording lost the deep fundamental, the footage is the fundamental's");
                                harmonic = 1;
                            }
                        }
                    }
                    const int soundingSt = st + juce::roundToInt(12.0 * std::log2((double)harmonic));
                    if (firstSegment)
                    {
                        rank->soundingHarmonic = harmonic;
                        rank->lowestFootageSemitones = soundingSt;
                    }
                    if (text.isNotEmpty()) text << " | ";
                    if (offsetCounts.size() > 1) text << helmholtz(rd.notes[i].note) << " ";
                    text << (offsetCounts.size() > 1 || harmonic > 1 ? footageLabel(st) : footageName(st));
                    if (harmonic > 1) text << " (" << footageLabel(soundingSt) << ")";
                    firstSegment = false;
                }
                rank->footageSounding = text;
            }
        }
        // Recorded temperament: mean deviation per pitch class of the KEY (a
        // mutation pipe on key C carries the deviation of C, as the engine also
        // applies the temperament by key), mean removed.
        std::array<double, 12> sum{}, count{};
        for (size_t i = 0; i < rd.notes.size(); ++i)
        {
            const int pitchClass = ((rd.notes[i].note % 12) + 12) % 12;
            // detrended (celeste) and relative to this rank's own diapason
            const double deviation = residualCents[i] - trendCents[i] - (rankDiapason - organ) * 100.0;
            // A pipe far off (a wrong unity note, a pipe a third low - Adlington's
            // bottom B and C#) is a mistuned pipe, never the temperament (0.8.159).
            if (std::abs(deviation) > 50.0) continue;
            sum[(size_t)pitchClass] += deviation;
            count[(size_t)pitchClass] += 1.0;
        }
        int classes = 0;
        double mean = 0.0;
        for (int pc = 0; pc < 12; ++pc)
            if (count[(size_t)pc] > 0.0) { ++classes; mean += sum[(size_t)pc] / count[(size_t)pc]; }
        rank->measuredTemperamentValid = classes >= 8;
        rank->measuredTemperamentCents.fill(0.0f);
        if (rank->measuredTemperamentValid)
        {
            mean /= classes;
            for (int pc = 0; pc < 12; ++pc)
                rank->measuredTemperamentCents[(size_t)pc] = count[(size_t)pc] > 0.0
                    ? (float)(sum[(size_t)pc] / count[(size_t)pc] - mean) : 0.0f;
        }
    }
}

// Beats grow with the sounding frequency as f^alpha, alpha chosen so that the
// top of the compass beats 'ratio' times faster than the bottom; the a1 key
// beats exactly 'beatsAtA1'. Detune is clamped to +-30 cents.
void SamplerEngine::computeUndulationTable(Stop* rank, float beatsAtA1, float ratio)
{
    rank->undulationBeats = beatsAtA1;
    rank->undulationCents.fill(0.0f);
    if (std::abs(beatsAtA1) < 1.0e-3f) return;
    int lowest = 128, highest = -1;
    for (int note = 0; note < 128; ++note)
        if (rank->hasNote(note)) { lowest = juce::jmin(lowest, note); highest = juce::jmax(highest, note); }
    if (highest < 0) return;
    double footage = 0.0;
    if (rank->tuningInfoAvailable)
        for (int note = lowest; note <= highest; ++note)
            if (rank->hasNote(note) && rank->samplesByNote[(size_t)note][0] != nullptr
                && std::isfinite(rank->samplesByNote[(size_t)note][0]->smplPitch))
            {
                footage = std::round(rank->samplesByNote[(size_t)note][0]->smplPitch
                    + rank->tuning440Cents[(size_t)note] / 100.0f - note);
                break;
            }
    const double octaves = juce::jmax(1.0, (highest - lowest) / 12.0);
    const double alpha = std::log2((double)juce::jlimit(1.0f, 20.0f, ratio)) / octaves;
    const double fA1 = 440.0 * std::pow(2.0, footage / 12.0);
    for (int note = 0; note < 128; ++note)
    {
        const double f = 440.0 * std::pow(2.0, (note - 69 + footage) / 12.0);
        const double beats = std::abs(beatsAtA1) * std::pow(f / fA1, alpha);
        const double cents = juce::jmin(30.0, 1200.0 * std::log2(1.0 + beats / f));
        rank->undulationCents[(size_t)note] = (float)(beatsAtA1 < 0.0f ? -cents : cents);
    }
}

juce::String SamplerEngine::footageName(int semitones)
{
    // Octaves, fifths, thirds, sevenths and ninths relative to 8' (= key pitch).
    static const std::map<int, const char*> names{
        { -36, "64'" }, { -24, "32'" }, { -12, "16'" }, { 0, "8'" }, { 12, "4'" }, { 24, "2'" }, { 36, "1'" }, { 48, "1/2'" },
        { -17, "21 1/3' (fifth)" }, { -5, "10 2/3' (fifth)" }, { 7, "5 1/3' (fifth)" }, { 19, "2 2/3' (fifth)" },
        { 31, "1 1/3' (fifth)" }, { 43, "2/3' (fifth)" }, { 55, "1/3' (fifth)" },
        { -8, "12 4/5' (third)" }, { 4, "6 2/5' (third)" }, { 16, "3 1/5' (third)" }, { 28, "1 3/5' (third)" },
        { 40, "4/5' (third)" }, { 52, "2/5' (third)" },
        { -2, "9 1/7' (seventh)" }, { 10, "4 4/7' (seventh)" }, { 22, "2 2/7' (seventh)" }, { 34, "1 1/7' (seventh)" },
        { 2, "7 1/9' (ninth)" }, { 14, "3 5/9' (ninth)" }, { 26, "1 7/9' (ninth)" }, { 38, "8/9' (ninth)" },
        // The small ones of mixtures, Zimbeln and the top ranks of a Scharff: fractions, never "0.12'".
        { 60, "1/4'" }, { 72, "1/8'" }, { 84, "1/16'" }, { 67, "1/6' (fifth)" }, { 79, "1/12' (fifth)" },
        { 64, "1/5' (third)" }, { 76, "1/10' (third)" }, { 46, "4/7' (seventh)" }, { 58, "2/7' (seventh)" }, { 70, "1/7' (seventh)" },
        { 50, "4/9' (ninth)" }, { 62, "2/9' (ninth)" }, { 74, "1/9' (ninth)" } };
    const auto it = names.find(semitones);
    if (it != names.end()) return it->second;
    // Every other semitone is a pure (just) interval of the 8' and reads as the
    // fraction it gives, never a decimal (0.8.159, the user's rule: the pure
    // minor second 16/15 = 7 1/2', the minor third 6/5 = 6 2/3', the fourth 4/3 =
    // 6', the tritone 7/5 = 5 5/7', the minor sixth 8/5 = 5', the major sixth
    // 5/3 = 4 4/5', the major seventh 15/8 = 4 4/15'; an octave lower doubles).
    int octaves = (int)std::floor(semitones / 12.0);
    const int cls = semitones - 12 * octaves;   // 0..11
    juce::int64 numerator = 8 * (juce::int64)justIntervalDenominator(cls), denominator = justIntervalNumerator(cls);   // feet = 8 / (num/den) / 2^octaves
    for (; octaves > 0; --octaves) denominator *= 2;
    for (; octaves < 0; ++octaves) numerator *= 2;
    const juce::int64 g = std::gcd(numerator, denominator);
    numerator /= g; denominator /= g;
    const juce::int64 whole = numerator / denominator, rest = numerator % denominator;
    juce::String text;
    if (whole > 0) text << whole;
    if (rest > 0) text << (whole > 0 ? " " : "") << rest << "/" << denominator;
    return text + "' (" + juce::String(semitones) + " st)";
}

int SamplerEngine::justIntervalNumerator(int semitoneClass)
{
    static const int numerators[12]{ 1, 16, 9, 6, 5, 4, 7, 3, 8, 5, 9, 15 };
    return numerators[((semitoneClass % 12) + 12) % 12];
}

int SamplerEngine::justIntervalDenominator(int semitoneClass)
{
    static const int denominators[12]{ 1, 15, 8, 5, 4, 3, 5, 2, 5, 3, 5, 8 };
    return denominators[((semitoneClass % 12) + 12) % 12];
}

void SamplerEngine::setUndulationBeatRatio(float ratio)
{
    undulationBeatRatio.store(juce::jlimit(1.0f, 20.0f, ratio), std::memory_order_relaxed);
    for (auto* stop : stops)
        if (std::abs(stop->undulationBeats) > 1.0e-3f) setStopUndulation(stop, stop->undulationBeats);
}
