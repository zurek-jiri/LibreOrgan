#pragma once

#include <JuceHeader.h>
#include "CrashHandler.h"   // (windows.h with NOMINMAX)

// A helper process of ours - the question to a sound card, the scan of a plug-in, the trial of its window -
// started WITHOUT the system's "starting an application" cursor (0.9.108).
//
// Windows shows the arrow with the ring for up to two seconds after ANY program is started, wherever the mouse is.
// With the Audio routing open a helper was started for every grey card every five seconds: measured on 0.9.107,
// the cursor showed the ring 70 % of the time (the user: "každé 4 sekundy se z myši stane čekací kolečko"), while
// the program itself never waited. juce::ChildProcess cannot switch that off, so on Windows the helper is started
// here with STARTF_FORCEOFFFEEDBACK; elsewhere juce::ChildProcess does the work. The same three calls as
// juce::ChildProcess: start, isRunning, kill.
class QuietProcess final
{
public:
    QuietProcess() = default;
    ~QuietProcess() { release(); }

    bool start(const juce::StringArray& arguments, int = 0)
    {
       #if JUCE_WINDOWS
        release();
        juce::String line;
        for (const auto& argument : arguments)   // (juce::ChildProcess's own quoting)
            line << (argument.containsAnyOf("\" ") ? argument.replace("\"", "\\\"").quoted() : argument) << ' ';
        line = line.trim();
        std::wstring text(line.toWideCharPointer());   // (CreateProcessW may write into its command line)
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_FORCEOFFFEEDBACK;
        PROCESS_INFORMATION information{};
        if (!CreateProcessW(nullptr, text.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &startup, &information))
            return false;
        CloseHandle(information.hThread);
        process = information.hProcess;
        return true;
       #else
        return child.start(arguments, 0);
       #endif
    }

    bool isRunning() const
    {
       #if JUCE_WINDOWS
        return process != nullptr && WaitForSingleObject(process, 0) == WAIT_TIMEOUT;
       #else
        return child.isRunning();
       #endif
    }

    // What the process ended with (0 = in order); -1 while it runs or when it never started.
    int exitCode() const
    {
       #if JUCE_WINDOWS
        DWORD code = 0;
        return process != nullptr && GetExitCodeProcess(process, &code) != 0 && code != STILL_ACTIVE ? (int)code : -1;
       #else
        return child.isRunning() ? -1 : (int)child.getExitCode();
       #endif
    }

    bool kill()
    {
       #if JUCE_WINDOWS
        return process != nullptr && TerminateProcess(process, 1) != 0;
       #else
        return child.kill();
       #endif
    }

private:
   #if JUCE_WINDOWS
    void release() { if (process != nullptr) { CloseHandle(process); process = nullptr; } }
    HANDLE process = nullptr;
   #else
    void release() {}
    juce::ChildProcess child;
   #endif
    JUCE_DECLARE_NON_COPYABLE(QuietProcess)
};
