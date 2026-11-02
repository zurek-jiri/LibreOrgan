#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <cstring>
#include <exception>
#include <csignal>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <dbghelp.h>
#else
 #include <execinfo.h>   // 0.9.37: the message thread's stack on macOS and Linux (backtrace through a signal)
 #include <pthread.h>
 #include <cstdlib>
 #include <unistd.h>
 #if JUCE_MAC
  #include <mach/mach.h>   // 0.9.40: the message thread sampled by thread_suspend + a frame-pointer walk (backtrace() through a signal frame stopped at the first system leaf)
 #endif
 #if JUCE_LINUX
  #include <sys/syscall.h>   // 0.9.42: tgkill - every thread of the process sampled through the signal
 #endif
#endif

//==============================================================================
// Writes a crash report (text log plus, on Windows, a minidump) into
//   <user app data>/LibreOrgan/CrashLogs/
// whenever the process dies from an unhandled exception, an abort, a
// std::terminate or an invalid CRT parameter.
//
// Breadcrumbs: call CrashHandler::note("...") at coarse milestones (folder
// loading, device switches).  The last 32 notes are included in the report.
//==============================================================================
namespace CrashHandler
{
    namespace detail
    {
        struct Crumb { char text[120]; };
        inline std::array<Crumb, 32>& crumbs() { static std::array<Crumb, 32> c{}; return c; }
        inline std::atomic<uint32_t>& crumbIndex() { static std::atomic<uint32_t> i{ 0 }; return i; }
        inline std::atomic<bool>& alreadyHandling() { static std::atomic<bool> b{ false }; return b; }
        inline juce::File& logDirectory() { static juce::File dir; return dir; }
        inline juce::String& applicationInfo() { static juce::String info; return info; }
        inline std::function<juce::String()>& stateProvider() { static std::function<juce::String()> f; return f; }

        inline juce::String timestamp()
        {
            return juce::Time::getCurrentTime().formatted("%Y%m%d-%H%M%S");
        }

        inline juce::String breadcrumbText()
        {
            juce::String result;
            const uint32_t index = crumbIndex().load();
            auto& list = crumbs();
            for (uint32_t i = 0; i < list.size(); ++i)
            {
                const auto& crumb = list[(size_t)((index + i) % list.size())];
                if (crumb.text[0] != 0) result << "  " << juce::String(crumb.text) << "\n";
            }
            return result;
        }

       #if JUCE_WINDOWS
        inline juce::String describeException(EXCEPTION_POINTERS* info)
        {
            if (info == nullptr || info->ExceptionRecord == nullptr) return "(no exception record)";
            auto* record = info->ExceptionRecord;
            juce::String text;
            text << "Exception code: 0x" << juce::String::toHexString((juce::int64)record->ExceptionCode)
                 << "  at address: 0x" << juce::String::toHexString((juce::int64)(juce::pointer_sized_int)record->ExceptionAddress) << "\n";
            switch (record->ExceptionCode)
            {
                case EXCEPTION_ACCESS_VIOLATION:
                    text << "Access violation: " << (record->ExceptionInformation[0] == 0 ? "read" : "write")
                         << " of address 0x" << juce::String::toHexString((juce::int64)record->ExceptionInformation[1]) << "\n";
                    break;
                case EXCEPTION_STACK_OVERFLOW: text << "Stack overflow\n"; break;
                case EXCEPTION_INT_DIVIDE_BY_ZERO: text << "Integer divide by zero\n"; break;
                case EXCEPTION_ILLEGAL_INSTRUCTION: text << "Illegal instruction\n"; break;
                case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: text << "Array bounds exceeded\n"; break;
                default: break;
            }
            HMODULE module = nullptr;
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCWSTR)record->ExceptionAddress, &module) && module != nullptr)
            {
                wchar_t path[MAX_PATH]{};
                GetModuleFileNameW(module, path, MAX_PATH);
                text << "Faulting module: " << juce::String(path)
                     << "  (base 0x" << juce::String::toHexString((juce::int64)(juce::pointer_sized_int)module) << ")\n";
            }
            return text;
        }

        // The symbol, source line and module of an address (the stack listings).
        inline juce::String describeAddress(HANDLE process, DWORD64 pc)
        {
            juce::String text;
            alignas(SYMBOL_INFO) char symbolBuffer[sizeof(SYMBOL_INFO) + 512]{};
            auto* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = 511;
            DWORD64 displacement = 0;
            if (SymFromAddr(process, pc, &displacement, symbol))
                text << "  " << juce::String(symbol->Name) << " + " << (juce::int64)displacement;
            IMAGEHLP_LINE64 line{};
            line.SizeOfStruct = sizeof(line);
            DWORD lineDisplacement = 0;
            if (SymGetLineFromAddr64(process, pc, &lineDisplacement, &line))
                text << "  (" << juce::String(line.FileName) << ":" << (int)line.LineNumber << ")";
            HMODULE module = nullptr;
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCWSTR)pc, &module) && module != nullptr)
            {
                wchar_t path[MAX_PATH]{};
                GetModuleFileNameW(module, path, MAX_PATH);
                text << "  [" << juce::File(juce::String(path)).getFileName() << "]";
            }
            return text;
        }

       #if defined(_M_X64)
        // The return addresses of a suspended thread, with the unwinder of the OS
        // only (no locks, no allocation: the thread may be inside the heap or the
        // loader). Plain C: a bad stack pointer ends the walk instead of the program.
        inline int collectFrames(CONTEXT* context, DWORD64* pcs, int maxFrames)
        {
            int depth = 0;
            __try
            {
                while (depth < maxFrames && context->Rip != 0)
                {
                    pcs[depth++] = context->Rip;
                    DWORD64 imageBase = 0;
                    auto* function = RtlLookupFunctionEntry(context->Rip, &imageBase, nullptr);
                    if (function == nullptr)
                    {
                        // A leaf function: the return address lies on the stack.
                        context->Rip = *reinterpret_cast<DWORD64*>(context->Rsp);
                        context->Rsp += 8;
                    }
                    else
                    {
                        void* handlerData = nullptr;
                        DWORD64 establisherFrame = 0;
                        RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, context->Rip, function, context,
                            &handlerData, &establisherFrame, nullptr);
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
            return depth;
        }
       #endif

        // The stack of another thread (the message thread when it stalls): the
        // thread is suspended only while the raw addresses are collected; the
        // symbols are looked up after it runs again.
        inline juce::String stackTraceOfThread(DWORD threadId)
        {
           #if defined(_M_X64)
            HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, threadId);
            if (thread == nullptr) return "  (thread not accessible)\n";
            DWORD64 pcs[64]{};
            int depth = 0;
            if (SuspendThread(thread) != (DWORD)-1)
            {
                CONTEXT context{};
                context.ContextFlags = CONTEXT_FULL;
                if (GetThreadContext(thread, &context)) depth = collectFrames(&context, pcs, 64);
                ResumeThread(thread);
            }
            CloseHandle(thread);
            HANDLE process = GetCurrentProcess();
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
            SymInitialize(process, nullptr, TRUE);
            juce::String text;
            for (int i = 0; i < depth; ++i)
                text << "  #" << i << "  0x" << juce::String::toHexString((juce::int64)pcs[i]) << describeAddress(process, pcs[i]) << "\n";
            if (depth == 0) text << "  (no frames)\n";
            return text;
           #else
            juce::ignoreUnused(threadId);
            return "  (the stack of another thread is listed on x64 only)\n";
           #endif
        }

        inline juce::String stackTrace(EXCEPTION_POINTERS* info)
        {
            juce::String text;
            HANDLE process = GetCurrentProcess();
            HANDLE thread = GetCurrentThread();
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
            SymInitialize(process, nullptr, TRUE);
            CONTEXT context{};
            if (info != nullptr && info->ContextRecord != nullptr) context = *info->ContextRecord;
            else RtlCaptureContext(&context);
            STACKFRAME64 frame{};
            DWORD machine;
           #if defined(_M_X64)
            machine = IMAGE_FILE_MACHINE_AMD64;
            frame.AddrPC.Offset = context.Rip; frame.AddrFrame.Offset = context.Rbp; frame.AddrStack.Offset = context.Rsp;
           #elif defined(_M_ARM64)
            machine = IMAGE_FILE_MACHINE_ARM64;
            frame.AddrPC.Offset = context.Pc; frame.AddrFrame.Offset = context.Fp; frame.AddrStack.Offset = context.Sp;
           #else
            machine = IMAGE_FILE_MACHINE_I386;
            frame.AddrPC.Offset = context.Eip; frame.AddrFrame.Offset = context.Ebp; frame.AddrStack.Offset = context.Esp;
           #endif
            frame.AddrPC.Mode = frame.AddrFrame.Mode = frame.AddrStack.Mode = AddrModeFlat;
            for (int depth = 0; depth < 64; ++depth)
            {
                if (!StackWalk64(machine, process, thread, &frame, &context, nullptr,
                    SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) break;
                if (frame.AddrPC.Offset == 0) break;
                text << "  #" << depth << "  0x" << juce::String::toHexString((juce::int64)frame.AddrPC.Offset)
                     << describeAddress(process, frame.AddrPC.Offset) << "\n";
            }
            return text;
        }

        inline void writeMiniDump(const juce::File& file, EXCEPTION_POINTERS* info)
        {
            HANDLE handle = CreateFileW(file.getFullPathName().toWideCharPointer(), GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (handle == INVALID_HANDLE_VALUE) return;
            MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{};
            exceptionInfo.ThreadId = GetCurrentThreadId();
            exceptionInfo.ExceptionPointers = info;
            exceptionInfo.ClientPointers = FALSE;
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), handle,
                (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithThreadInfo | MiniDumpScanMemory),
                info != nullptr ? &exceptionInfo : nullptr, nullptr, nullptr);
            CloseHandle(handle);
        }
       #endif

        inline void writeReport(const juce::String& reason, void* platformInfo)
        {
            if (alreadyHandling().exchange(true)) return;   // one report per crash
            auto dir = logDirectory();
            if (dir == juce::File())
                dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("LibreOrgan").getChildFile("CrashLogs");
            dir.createDirectory();
            const auto stamp = timestamp();
            const auto logFile = dir.getChildFile("crash-" + stamp + ".txt");

            juce::String report;
            report << "LibreOrgan Sampler crash report\n"
                   << "Time: " << juce::Time::getCurrentTime().toString(true, true, true, true) << "\n"
                   << applicationInfo() << "\n"
                   << "Reason: " << reason << "\n";
            report << "Thread: " << juce::String::toHexString((juce::int64)juce::Thread::getCurrentThreadId()) << "\n";
            report << "OS: " << juce::SystemStats::getOperatingSystemName()
                   << "  CPU: " << juce::SystemStats::getCpuModel()
                   << " (" << juce::SystemStats::getNumCpus() << " logical)  RAM: "
                   << juce::SystemStats::getMemorySizeInMegabytes() << " MB\n\n";
           #if JUCE_WINDOWS
            auto* info = static_cast<EXCEPTION_POINTERS*>(platformInfo);
            report << describeException(info) << "\nStack:\n" << stackTrace(info) << "\n";
           #else
            juce::ignoreUnused(platformInfo);
            report << "Stack:\n" << juce::SystemStats::getStackBacktrace() << "\n";
           #endif
            if (auto& provider = stateProvider())
            {
                try { report << "Engine state:\n" << provider() << "\n"; }
                catch (...) { report << "Engine state: unavailable\n"; }
            }
            report << "Recent activity (oldest first):\n" << breadcrumbText() << "\n";
            logFile.replaceWithText(report);
           #if JUCE_WINDOWS
            writeMiniDump(dir.getChildFile("crash-" + stamp + ".dmp"), info);
           #endif
        }

        inline void juceCrashHandler(void* platformInfo)
        {
            writeReport("Unhandled exception", platformInfo);
        }

        inline void terminateHandler()
        {
            juce::String reason = "std::terminate";
            if (auto current = std::current_exception())
            {
                try { std::rethrow_exception(current); }
                catch (const std::exception& e) { reason << " (" << e.what() << ")"; }
                catch (...) { reason << " (non-standard exception)"; }
            }
            writeReport(reason, nullptr);
            std::abort();
        }

        inline void abortSignalHandler(int)
        {
            writeReport("SIGABRT", nullptr);
        }

       #if JUCE_MSVC
        inline void invalidParameterHandler(const wchar_t* expression, const wchar_t* function,
            const wchar_t* file, unsigned int line, uintptr_t)
        {
            juce::String reason = "Invalid CRT parameter";
            if (function != nullptr) reason << " in " << juce::String(function);
            if (expression != nullptr) reason << ": " << juce::String(expression);
            if (file != nullptr) reason << " (" << juce::String(file) << ":" << (int)line << ")";
            writeReport(reason, nullptr);
            std::abort();
        }
        inline void pureCallHandler() { writeReport("Pure virtual call", nullptr); std::abort(); }
       #endif
    }

    inline void note(const juce::String& text) noexcept
    {
        auto& list = detail::crumbs();
        const uint32_t index = detail::crumbIndex().fetch_add(1) % (uint32_t)list.size();
        auto& crumb = list[(size_t)index];
        const auto stamped = juce::Time::getCurrentTime().formatted("%H:%M:%S ") + text;
        std::memset(crumb.text, 0, sizeof(crumb.text));
        stamped.copyToUTF8(crumb.text, sizeof(crumb.text) - 1);
        // Also kept in CrashLogs/activity.log so device problems can be traced
        // without a crash. Rotated at about 1 MB. Only called from non-real-time threads.
        try
        {
            if (juce::MessageManager::getInstanceWithoutCreating() == nullptr) return;
            auto dir = detail::logDirectory();
            if (dir == juce::File()) return;
            auto log = dir.getChildFile("activity.log");
            if (log.getSize() > 1000000) log.moveFileTo(dir.getChildFile("activity-previous.log"));
            log.appendText(juce::Time::getCurrentTime().formatted("%Y-%m-%d ") + stamped + "\n");
        }
        catch (...) {}
    }

    inline void setStateProvider(std::function<juce::String()> provider)
    {
        detail::stateProvider() = std::move(provider);
    }

    //==========================================================================
    // The message thread is watched: a timer on it beats four times a second;
    // when the beat stops for 8 s (a lock it waits for, a long synchronous
    // operation), the activity log gets a note with the thread's stack, again
    // every minute while it lasts, and a note when the thread answers again.
    // Nothing is suspended or changed otherwise. Created on the message thread.
    class StallWatchdog final : private juce::Thread, private juce::Timer
    {
    public:
        StallWatchdog() : juce::Thread("Message thread watchdog")
        {
           #if JUCE_WINDOWS
            messageThreadId = GetCurrentThreadId();
           #else
            // 0.9.37 (#167 Mac, #168 Linux: the window thread silent for 3-6 s, nothing in the log says
            // where): the watchdog asks the message thread for its stack with a signal - the handler
            // writes backtrace() into a static buffer, the watchdog prints it with backtrace_symbols.
            posixMessageThread() = pthread_self();
           #if JUCE_MAC
            machMessageThread() = mach_thread_self();   // 0.9.40: the port the watchdog suspends and reads the registers of
           #endif
            struct sigaction action{};
            action.sa_handler = [](int) { stackDepth().store(backtrace(stackBuffer(), 64), std::memory_order_release); };
            sigemptyset(&action.sa_mask);
            action.sa_flags = SA_RESTART;
            sigaction(SIGUSR2, &action, nullptr);
           #endif
            lastBeatMs.store(juce::Time::getMillisecondCounterHiRes());
            startTimer(250);
            startThread();
        }
       #if ! JUCE_WINDOWS
        static pthread_t& posixMessageThread() { static pthread_t t{}; return t; }
       #if JUCE_MAC
        static mach_port_t& machMessageThread() { static mach_port_t t = 0; return t; }   // 0.9.40
       #endif
        static void** stackBuffer() { static void* buffer[64]; return buffer; }
        static std::atomic<int>& stackDepth() { static std::atomic<int> depth{ -1 }; return depth; }
        // The message thread's stack as text (called from the watchdog thread): the signal, up to
        // 200 ms for the handler, then the symbols (names where the executable exports them).
        static juce::String messageThreadStack()
        {
           #if JUCE_MAC
            // 0.9.40 (#175: every sample of 0.9.37-0.9.39 ended at the first system function, the rest
            // garbage - backtrace() in a signal handler walks frame pointers and the signal frame breaks
            // the chain at a leaf without one): the message thread is SUSPENDED, its registers read, the
            // frame-pointer chain walked with safe reads (no malloc while it is suspended - it may hold the
            // heap lock), the thread resumed, and only then the addresses symbolised. What `sample` does.
            const mach_port_t thread = machMessageThread();
            if (thread == 0) return "(no thread port)";
            // One thread sampled: suspended, registers, the frame chain, resumed. No allocation inside.
            auto sample = [](mach_port_t t, void** frames, int maxDepth) -> int
            {
                if (thread_suspend(t) != KERN_SUCCESS) return 0;
                int depth = 0;
                uintptr_t fp = 0;
               #if JUCE_ARM
                arm_thread_state64_t state{};
                mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
                if (thread_get_state(t, ARM_THREAD_STATE64, (thread_state_t)&state, &count) == KERN_SUCCESS)
                {
                    frames[depth++] = (void*)arm_thread_state64_get_pc(state);
                    fp = (uintptr_t)arm_thread_state64_get_fp(state);
                }
               #else
                x86_thread_state64_t state{};
                mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
                if (thread_get_state(t, x86_THREAD_STATE64, (thread_state_t)&state, &count) == KERN_SUCCESS)
                {
                    frames[depth++] = (void*)state.__rip;
                    fp = (uintptr_t)state.__rbp;
                }
               #endif
                for (int i = 0; i < maxDepth && fp != 0 && depth < maxDepth; ++i)
                {
                    uintptr_t pair[2]{};   // [fp] = the caller's fp, [fp + 8] = the return address
                    vm_size_t got = 0;
                    if (vm_read_overwrite(mach_task_self(), (vm_address_t)fp, sizeof(pair), (vm_address_t)pair, &got) != KERN_SUCCESS || got != sizeof(pair)) break;
                    if (pair[1] == 0) break;
                    frames[depth++] = (void*)pair[1];
                    if (pair[0] <= fp) break;   // the chain must climb the stack
                    fp = pair[0];
                }
                thread_resume(t);
                return depth;
            };
            void* frames[64];
            const int depth = sample(thread, frames, 64);
            if (depth <= 0) return "(the registers could not be read)";
            juce::String text;
            if (char** symbols = backtrace_symbols(frames, depth))
            {
                for (int i = 0; i < depth; ++i) text << "  #" << i << "  " << symbols[i] << "\n";
                free(symbols);
            }
            // 0.9.42 (#180: the message thread sits in the queue's removeAndReturn - a flood of messages;
            // who posts them?): every other thread of the process sampled the same way, 20 frames each,
            // named where it has a name. The real-time threads mostly show their waits; a thread caught
            // in MessageQueue::post / callAsync is the producer.
            thread_act_array_t threads = nullptr;
            mach_msg_type_number_t threadCount = 0;
            if (task_threads(mach_task_self(), &threads, &threadCount) == KERN_SUCCESS)
            {
                const mach_port_t self = mach_thread_self();
                int shown = 0;
                for (mach_msg_type_number_t i = 0; i < threadCount && shown < 24; ++i)
                {
                    if (threads[i] == thread || threads[i] == self) continue;
                    void* other[20];
                    const int otherDepth = sample(threads[i], other, 20);
                    if (otherDepth <= 0) continue;
                    ++shown;
                    char name[64]{};
                    if (pthread_t p = pthread_from_mach_thread_np(threads[i])) pthread_getname_np(p, name, sizeof(name));
                    text << "  -- thread " << (int)i << (name[0] != 0 ? juce::String(" \"") + name + "\"" : juce::String()) << ":\n";
                    if (char** symbols = backtrace_symbols(other, otherDepth))
                    {
                        for (int k = 0; k < otherDepth; ++k) text << "     " << symbols[k] << "\n";
                        free(symbols);
                    }
                }
                for (mach_msg_type_number_t i = 0; i < threadCount; ++i) mach_port_deallocate(mach_task_self(), threads[i]);
                vm_deallocate(mach_task_self(), (vm_address_t)threads, threadCount * sizeof(thread_act_t));
                mach_port_deallocate(mach_task_self(), self);
            }
            return text;
           #else
            stackDepth().store(-1, std::memory_order_release);
            if (pthread_kill(posixMessageThread(), SIGUSR2) != 0) return "(the signal was not delivered)";
            for (int waited = 0; stackDepth().load(std::memory_order_acquire) < 0 && waited < 200; waited += 5) juce::Thread::sleep(5);
            const int depth = stackDepth().load(std::memory_order_acquire);
            if (depth <= 0) return "(no answer within 200 ms)";
            juce::String text;
            if (char** symbols = backtrace_symbols(stackBuffer(), depth))
            {
                for (int i = 0; i < depth; ++i) text << "  #" << i << "  " << symbols[i] << "\n";
                free(symbols);
            }
           #if JUCE_LINUX
            // 0.9.42 (#176/#180: the message queue flooded - who posts?): every other thread of the process
            // sampled through the same signal (tgkill by its id from /proc/self/task), 20 frames each, with
            // its name from /proc/self/task/<id>/comm.
            const juce::File tasks("/proc/self/task");
            const pid_t self = (pid_t)syscall(SYS_gettid);
            int shown = 0;
            for (const auto& task : tasks.findChildFiles(juce::File::findDirectories, false))
            {
                const pid_t tid = (pid_t)task.getFileName().getIntValue();
                if (tid <= 0 || tid == self || shown >= 24) continue;
                stackDepth().store(-1, std::memory_order_release);
                if (syscall(SYS_tgkill, getpid(), tid, SIGUSR2) != 0) continue;
                for (int waited = 0; stackDepth().load(std::memory_order_acquire) < 0 && waited < 100; waited += 5) juce::Thread::sleep(5);
                const int otherDepth = juce::jmin(20, stackDepth().load(std::memory_order_acquire));
                if (otherDepth <= 0) continue;
                ++shown;
                const auto name = task.getChildFile("comm").loadFileAsString().trim();
                text << "  -- thread " << (int)tid << (name.isNotEmpty() ? " \"" + name + "\"" : juce::String()) << ":\n";
                if (char** symbols = backtrace_symbols(stackBuffer(), otherDepth))
                {
                    for (int k = 0; k < otherDepth; ++k) text << "     " << symbols[k] << "\n";
                    free(symbols);
                }
            }
           #endif
            return text;
           #endif
        }
       #endif
        ~StallWatchdog() override
        {
            stopTimer();
            signalThreadShouldExit();
            notify();
            stopThread(3000);
        }
    private:
        void timerCallback() override { lastBeatMs.store(juce::Time::getMillisecondCounterHiRes()); }
    public:
        // What to do when the message thread has answered nothing for three
        // seconds (called once per stall, from the watchdog thread): the polyphony
        // measurement sets it to release every key - its strokes run on the
        // message thread, so a stall left the keys down, the load on, and the
        // starved thread never came back (#15, a two-core Linux laptop).
        static std::function<void(double)>& stallAction()
        {
            static std::function<void(double)> action;
            return action;
        }
        static juce::CriticalSection& stallActionLock() { static juce::CriticalSection lock; return lock; }
        static void setStallAction(std::function<void(double)> action)
        {
            const juce::ScopedLock sl(stallActionLock());
            stallAction() = std::move(action);
        }
    private:
        void run() override
        {
            bool stalled = false, actionDone = false, sampled = false;
            double lastReportMs = 0.0, longest = 0.0, previousSilent = 0.0, lastShortNoteMs = 0.0;
            while (!threadShouldExit())
            {
                wait(500);
                if (threadShouldExit()) return;
                const double now = juce::Time::getMillisecondCounterHiRes();
                const double silent = now - lastBeatMs.load();
                if (silent >= 3000.0 && !actionDone)
                {
                    actionDone = true;
                    std::function<void(double)> action;
                    { const juce::ScopedLock sl(stallActionLock()); action = stallAction(); }
                    if (action) { note("Message thread silent for 3 s: the stall action runs (keys released)"); action(silent); }
                }
               #if ! JUCE_WINDOWS
                // 0.9.37: a stall of 2 s is sampled while it lasts - once per stall.
                if (silent >= 2000.0 && !sampled)
                {
                    sampled = true;
                    note("Message thread silent for " + juce::String(silent / 1000.0, 1) + " s; its stack now:\n" + messageThreadStack());
                }
                if (silent < 1000.0) sampled = false;
               #endif
                if (silent < 8000.0)
                {
                    if (silent < 3000.0) actionDone = false;
                    if (stalled)
                        note("Message thread responsive again (it answered nothing for about " + juce::String(juce::roundToInt(longest / 1000.0)) + " s)");
                    // 0.9.36 (#165: "everything freezes" on the Core 2 Duo Mac with nothing in the log): a stall
                    // of 1-8 s is noted when it ends - the duration alone (no stack), at most one line in 10 s.
                    else if (previousSilent >= 1000.0 && silent < previousSilent && now - lastShortNoteMs > 10000.0)
                    {
                        lastShortNoteMs = now;
                        note("Message thread was silent for " + juce::String(previousSilent / 1000.0, 1) + " s (a short stall; nothing above 8 s)");
                    }
                    previousSilent = silent;
                    stalled = false;
                    longest = 0.0;
                    continue;
                }
                previousSilent = silent;
                longest = juce::jmax(longest, silent);
                if (!stalled || now - lastReportMs > 60000.0)
                {
                    stalled = true;
                    lastReportMs = now;
                    juce::String text = "Message thread unresponsive for " + juce::String(juce::roundToInt(silent / 1000.0)) + " s";
                   #if JUCE_WINDOWS
                    text << "; its stack:\n" << detail::stackTraceOfThread(messageThreadId);
                   #endif
                    note(text);
                }
            }
        }
        std::atomic<double> lastBeatMs{ 0.0 };
       #if JUCE_WINDOWS
        DWORD messageThreadId = 0;
       #endif
    };

    inline void install(const juce::String& applicationName, const juce::String& version)
    {
        detail::logDirectory() = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("LibreOrgan").getChildFile("CrashLogs");
        detail::logDirectory().createDirectory();
       #if JUCE_DEBUG
        detail::applicationInfo() = "Application: " + applicationName + " " + version + " (debug)";
       #else
        detail::applicationInfo() = "Application: " + applicationName + " " + version + " (release)";
       #endif
        juce::SystemStats::setApplicationCrashHandler(detail::juceCrashHandler);
        std::set_terminate(detail::terminateHandler);
        std::signal(SIGABRT, detail::abortSignalHandler);
       #if JUCE_MSVC
        _set_invalid_parameter_handler(detail::invalidParameterHandler);
        _set_purecall_handler(detail::pureCallHandler);
       #endif
        note("Crash handler installed (build " __DATE__ " " __TIME__ ")");
    }
}
