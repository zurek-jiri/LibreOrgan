#pragma once

#include <JuceHeader.h>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #ifndef PSAPI_VERSION
  #define PSAPI_VERSION 2   // GetProcessMemoryInfo from kernel32 (K32...), no psapi.lib
 #endif
 #include <psapi.h>
 #include <malloc.h>   // _heapmin (releaseFreedMemory)
#elif JUCE_MAC
 #include <mach/mach.h>
 #include <mach/mach_host.h>
 #include <mach/task_info.h>
 #include <sys/sysctl.h>
 #include <malloc/malloc.h>   // malloc_zone_pressure_relief (releaseFreedMemory)
#elif JUCE_LINUX
 #include <malloc.h>   // malloc_trim (releaseFreedMemory)
#endif

//==============================================================================
// The memory figures the loading, the merge, the polyphony display and the
// measurement report share. Every figure means the same on every system:
//  - free: what the system could give the program now (Windows: available
//    physical; Linux: MemAvailable; macOS: free + inactive pages);
//  - held: what the program holds, the pages the system has compressed or
//    swapped out included (Windows: the larger of working set and private
//    bytes; Linux: VmRSS + VmSwap; macOS: the physical footprint Activity
//    Monitor shows - the resident size alone said 995 MB for an organ of
//    several GB on an 8 GB Mac, the rest sat compressed);
//  - swapped: the program's pages the system has swapped out (Linux VmSwap;
//    macOS: the compressor's pages of the whole system; Windows: 0);
//  - installed: the physical memory of the computer.
namespace MemoryInfo
{
    inline juce::int64 installedBytes()
    {
        return (juce::int64)juce::SystemStats::getMemorySizeInMegabytes() * 1024 * 1024;
    }

    inline juce::int64 freeBytes()
    {
       #if JUCE_WINDOWS
        MEMORYSTATUSEX status{};
        status.dwLength = sizeof(status);
        if (GlobalMemoryStatusEx(&status)) return (juce::int64)status.ullAvailPhys;
       #elif JUCE_LINUX
        const auto text = juce::File("/proc/meminfo").loadFileAsString();
        for (const auto& line : juce::StringArray::fromLines(text))
            if (line.startsWith("MemAvailable:"))
                return line.fromFirstOccurrenceOf(":", false, false).trim().upToFirstOccurrenceOf(" ", false, false).getLargeIntValue() * 1024;
       #elif JUCE_MAC
        vm_statistics64_data_t stats{};
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&stats, &count) == KERN_SUCCESS)
        {
            vm_size_t pageSize = 0;
            host_page_size(mach_host_self(), &pageSize);
            return (juce::int64)(stats.free_count + stats.inactive_count) * (juce::int64)pageSize;
        }
       #endif
        return installedBytes() / 2;   // unknown: half of the installed memory
    }

    // Gives the memory the program freed back to the system (0.8.184): the heap
    // keeps freed blocks committed, so the RAM figure of the next load carried the
    // previous organ's residue (the user's series in one process: 23 GB where a
    // fresh start held 14). Called once the removed stops are freed.
    inline void releaseFreedMemory()
    {
       #if JUCE_WINDOWS
        _heapmin();
       #elif JUCE_LINUX
        malloc_trim(0);
       #elif JUCE_MAC
        malloc_zone_pressure_relief(nullptr, 0);
       #endif
    }

    inline juce::int64 heldBytes()
    {
       #if JUCE_WINDOWS
        PROCESS_MEMORY_COUNTERS_EX counters{};
        counters.cb = sizeof(counters);
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters)))
            return (juce::int64)juce::jmax(counters.WorkingSetSize, counters.PrivateUsage);
       #elif JUCE_LINUX
        const auto text = juce::File("/proc/self/status").loadFileAsString();
        juce::int64 resident = 0, swapped = 0;
        for (const auto& line : juce::StringArray::fromLines(text))
        {
            auto kb = [&line] { return line.fromFirstOccurrenceOf(":", false, false).trim().upToFirstOccurrenceOf(" ", false, false).getLargeIntValue() * 1024; };
            if (line.startsWith("VmRSS:")) resident = kb();
            else if (line.startsWith("VmSwap:")) swapped = kb();
        }
        if (resident > 0) return resident + swapped;
       #elif JUCE_MAC
        {
            task_vm_info_data_t vm{};
            mach_msg_type_number_t vmCount = TASK_VM_INFO_COUNT;
            if (task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vm, &vmCount) == KERN_SUCCESS && vm.phys_footprint > 0)
                return (juce::int64)vm.phys_footprint;
        }
        mach_task_basic_info_data_t info{};
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS)
            return (juce::int64)info.resident_size;
       #endif
        return 0;
    }

    inline juce::int64 swappedBytes()
    {
       #if JUCE_LINUX
        const auto text = juce::File("/proc/self/status").loadFileAsString();
        for (const auto& line : juce::StringArray::fromLines(text))
            if (line.startsWith("VmSwap:"))
                return line.fromFirstOccurrenceOf(":", false, false).trim().upToFirstOccurrenceOf(" ", false, false).getLargeIntValue() * 1024;
       #elif JUCE_MAC
        vm_statistics64_data_t stats{};
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&stats, &count) == KERN_SUCCESS)
        {
            vm_size_t pageSize = 0;
            host_page_size(mach_host_self(), &pageSize);
            return (juce::int64)stats.compressor_page_count * (juce::int64)pageSize;
        }
       #endif
        return 0;
    }

    // The free memory the loading leaves untouched: 512 MB or a sixteenth
    // (768 MB until 0.8.134 - too much of a small memory; the footprint rule
    // below is the one that bites on the 8 GB machines), 2 GB at the most
    // (0.9.123, #261: a sixteenth of 128 GB kept 8 GB idle; up to 32 GB nothing
    // changes). macOS keeps the sixteenth: its free figure is another kind.
    inline juce::int64 reserveBytes()
    {
        const juce::int64 least = (juce::int64)512 * 1024 * 1024, sixteenth = installedBytes() / 16;
       #if JUCE_MAC
        return juce::jmax(least, sixteenth);
       #else
        return juce::jmax(least, juce::jmin(sixteenth, (juce::int64)2048 * 1024 * 1024));
       #endif
    }

    // What the system and every other program hold now.
    inline juce::int64 othersBytes()
    {
        return juce::jmax((juce::int64)0, installedBytes() - freeBytes() - heldBytes());
    }

    // The memory the system keeps for itself and the window: 1.5 GB or a fifth
    // of the installed memory. The program must stop loading before its own
    // footprint reaches installed - share: beyond it the system pages samples
    // out, and every stroke that touches them stalls the sound (two-core
    // laptops with 8 GB and a 7 GB organ, issues #16, #17, #19).
    // On a small computer (4 GB and less) 1 GB: 1.5 GB of 4 would leave the
    // organ a bare half (0.8.135).
    // 0.9.123 (#261): a fifth of 128 GB is 25.5 GB while the system and the
    // other programs held 8.6 GB. Above 16 GB (Windows and Linux) the share is
    // what the others really held when the load began (never under 1.5 GB)
    // plus the reserve - and never more than the fifth, so the rule only gets
    // looser. Up to 16 GB, and on macOS, the fifth as before.
    inline juce::int64 systemShareBytes(juce::int64 othersAtLoadStart = -1)
    {
        const juce::int64 installed = installedBytes();
        if (installed <= (juce::int64)4608 * 1024 * 1024) return (juce::int64)1024 * 1024 * 1024;
        const juce::int64 least = (juce::int64)1536 * 1024 * 1024;
        const juce::int64 fifth = juce::jmax(least, installed / 5);
       #if JUCE_MAC
        juce::ignoreUnused(othersAtLoadStart);
        return fifth;
       #else
        if (installed <= (juce::int64)16896 * 1024 * 1024) return fifth;
        const juce::int64 others = othersAtLoadStart >= 0 ? othersAtLoadStart : othersBytes();
        return juce::jmin(fifth, juce::jmax(least, others) + reserveBytes());
       #endif
    }

    // Windows: what may still be committed (RAM + page file - what every program
    // has committed). Not a figure of the RAM: with a small or no page file it
    // runs out first, and then allocations fail in every program. -1 elsewhere.
    inline juce::int64 commitHeadroomBytes()
    {
       #if JUCE_WINDOWS
        MEMORYSTATUSEX status{};
        status.dwLength = sizeof(status);
        if (GlobalMemoryStatusEx(&status)) return (juce::int64)status.ullAvailPageFile;
       #endif
        return -1;
    }

    // Windows: the program's memory that is NOT in its working set - what the
    // system took out of the RAM towards the page file (plus whatever was
    // allocated and never touched, so only the GROWTH since a baseline tells).
    // 0 elsewhere (Linux and macOS have swappedBytes()).
    inline juce::int64 trimmedBytes()
    {
       #if JUCE_WINDOWS
        PROCESS_MEMORY_COUNTERS_EX counters{};
        counters.cb = sizeof(counters);
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters)) && counters.PrivateUsage > counters.WorkingSetSize)
            return (juce::int64)(counters.PrivateUsage - counters.WorkingSetSize);
       #endif
        return 0;
    }

    // Windows: the program's working set (what of it is in the RAM now). 0 elsewhere.
    inline juce::int64 workingSetBytes()
    {
       #if JUCE_WINDOWS
        PROCESS_MEMORY_COUNTERS_EX counters{};
        counters.cb = sizeof(counters);
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&counters, sizeof(counters)))
            return (juce::int64)counters.WorkingSetSize;
       #endif
        return 0;
    }

    // The free memory under which the RAM cell turns orange and the watch after
    // a load speaks: a tenth of the installed memory up to 32 GB (as before),
    // three quarters of the reserve above (a tenth of 128 GB is 12.8 GB - a full
    // big computer would be orange for good).
    inline juce::int64 tightBelowBytes()
    {
        const juce::int64 installed = installedBytes();
       #if ! JUCE_MAC
        if (installed > (juce::int64)33792 * 1024 * 1024) return reserveBytes() * 3 / 4;
       #endif
        return installed / 10;
    }

    // macOS: the kernel's own memory pressure level (1 normal, 2 warning, 4
    // critical) - it already counts what freeing the inactive pages would give,
    // which a user process cannot force (purge needs root and only drops the
    // disk cache; madvise touches our own pages only). 0 where unknown.
    inline int pressureLevel()
    {
       #if JUCE_MAC
        int level = 0;
        size_t size = sizeof(level);
        if (sysctlbyname("kern.memorystatus_vm_pressure_level", &level, &size, nullptr, 0) == 0) return level;
       #endif
        return 0;
    }

    // Is the memory short now - must a loading stop? Any one sign: the free
    // memory under the reserve; the program's footprint within the system's
    // share; the program (Linux) or the system (macOS compressor) swapping
    // more than 256 MB beyond what it held when the load began (the macOS
    // figure is the whole system's: other programs' compressed pages sit there
    // at idle, 0.43 GB on the 8 GB Mac of #21 - the absolute figure stopped a
    // load of 2.4 GB); the kernel reporting pressure (macOS, warning or worse).
    // 0.9.123: 'othersAtLoadStart' (what the others held when the load began; -1 =
    // read now), 'room' (memory owed to something that comes after: the filled
    // pipes' copies of the stops loaded so far), 'trimmedBaseline' (Windows: the
    // trimmedBytes() of the load's start; -1 = that sign is not looked at).
    inline bool shortNow(juce::String* reason = nullptr, juce::int64 swappedBaseline = 0, juce::int64 othersAtLoadStart = -1,
        juce::int64 room = 0, juce::int64 trimmedBaseline = -1)
    {
        auto gb = [](juce::int64 b) { return juce::String((double)b / 1073741824.0, 2) + " GB"; };
        const juce::int64 installed = installedBytes(), free = freeBytes(), held = heldBytes(), swapped = swappedBytes();
        const juce::String roomText = room > 0 ? " and " + gb(room) + " are kept for what comes after the samples (the copies of the filled pipes, the grid)" : juce::String();
        if (const int level = pressureLevel(); level >= 2)
        {
            if (reason != nullptr) *reason = juce::String("the system reports memory pressure (level ") + juce::String(level) + ", " + (level >= 4 ? "critical" : "warning") + "), " + gb(free) + " free, the program holds " + gb(held);
            return true;
        }
        if (free < reserveBytes() + room)
        {
            if (reason != nullptr) *reason = gb(free) + " free, the reserve is " + gb(reserveBytes()) + roomText;
            return true;
        }
        const juce::int64 share = systemShareBytes(othersAtLoadStart);
        if (held > 0 && held > installed - share - room)
        {
            if (reason != nullptr) *reason = "the program holds " + gb(held) + " of " + gb(installed) + " installed and the system needs " + gb(share) + roomText;
            return true;
        }
        if (const juce::int64 commit = commitHeadroomBytes(); commit >= 0 && commit < reserveBytes() + room)
        {
            if (reason != nullptr) *reason = "the system's commit limit is nearly reached (" + gb(commit) + " left of the RAM and the page file together; " + gb(free) + " of the RAM free)" + roomText;
            return true;
        }
        if (trimmedBaseline >= 0)
            if (const juce::int64 trimmed = trimmedBytes(); trimmed - trimmedBaseline > (juce::int64)1024 * 1024 * 1024)
            {
                if (reason != nullptr) *reason = "the system is taking the program's memory out of the RAM already (" + gb(trimmed - trimmedBaseline) + " since the load began), " + gb(free) + " free";
                return true;
            }
        const bool swapGrew = swapped - swappedBaseline > (juce::int64)256 * 1024 * 1024;
       #if JUCE_MAC
        // The compressor figure is the whole system's: it grows as macOS squeezes
        // other programs to make room - which is what it should do. A sign only
        // when little is left to give (#22, #25: it stopped a 2 GB load with
        // 3.4 GB free; the footprint and the kernel's pressure level catch the rest).
        const bool swapSign = swapGrew && free < 2 * reserveBytes();
       #else
        const bool swapSign = swapGrew;
       #endif
        if (swapSign)
        {
            if (reason != nullptr) *reason = "the system is paging already (" + gb(swapped) + " swapped or compressed, " + gb(swappedBaseline) + " when the load began), " + gb(free) + " free";
            return true;
        }
        return false;
    }
}
