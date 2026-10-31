#pragma once

#include <JuceHeader.h>
#include <vector>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

//==============================================================================
// The performance and efficiency cores of a hybrid processor (Intel 12th
// generation and later; 0.8.140): a render worker on an E core renders
// several times slower than one on a P core, and the device thread waits for
// it (#34: 2.4 ms of the 10 ms period waiting for the lanes on an Ultra 7).
// Windows names the efficiency class of every core; the other systems report
// no hybrid here (macOS decides itself through the QoS classes).
namespace CpuInfo
{
    struct Hybrid
    {
        int performanceCores = 0, efficiencyCores = 0;
        juce::uint64 performanceMask = 0;   // the logical processors of the performance cores (group 0)
        bool hybrid() const noexcept { return performanceCores > 0 && efficiencyCores > 0; }
    };

    inline Hybrid detectHybrid()
    {
        Hybrid result;
       #if JUCE_WINDOWS
        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
        if (length == 0) return result;
        std::vector<char> buffer((size_t)length);
        if (!GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &length)) return result;
        struct Core { int efficiencyClass; juce::uint64 mask; };
        std::vector<Core> cores;
        int maxClass = 0;
        for (DWORD offset = 0; offset < length;)
        {
            auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);
            if (info->Size == 0) break;
            if (info->Relationship == RelationProcessorCore && info->Processor.GroupCount > 0 && info->Processor.GroupMask[0].Group == 0)
            {
                cores.push_back({ (int)info->Processor.EfficiencyClass, (juce::uint64)info->Processor.GroupMask[0].Mask });
                maxClass = juce::jmax(maxClass, (int)info->Processor.EfficiencyClass);
            }
            offset += info->Size;
        }
        for (const auto& core : cores)
        {
            if (core.efficiencyClass == maxClass) { ++result.performanceCores; result.performanceMask |= core.mask; }
            else ++result.efficiencyCores;
        }
       #endif
        return result;
    }

    inline const Hybrid& hybrid()
    {
        static const Hybrid detected = detectHybrid();
        return detected;
    }
}