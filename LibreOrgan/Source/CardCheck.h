#pragma once

#include <JuceHeader.h>
#include "QuietProcess.h"
#include <deque>
#include <functional>

// Is a sound card there? Asked in a process of its own (0.9.107).
//
// Windows lists an ASIO driver whether its device is plugged in or not; only the driver itself can tell, and to
// ask it the driver must be loaded - which may take seconds, hang or crash with a dead driver. So the question
// runs in a helper: this program started with --cardcheck "<request file>" (the request: the result file, the
// device type, the device name - a line each). The helper opens the device with a throw-away manager and writes
// "ok" or "no<TAB>reason"; no result = the driver took the helper down. The window thread never waits: Checker
// starts one helper at a time and looks at it ten times a second; a helper silent for eight seconds is killed.
namespace CardCheck
{
    inline bool isWorker(const juce::String& commandLine) { return commandLine.contains("--cardcheck"); }

    [[noreturn]] inline void runWorker(const juce::String& commandLine)
    {
        const juce::File request(commandLine.fromFirstOccurrenceOf("--cardcheck", false, false).trim().unquoted().trim());
        juce::StringArray lines;
        lines.addLines(request.loadFileAsString());
        if (lines.size() >= 3)
        {
            const juce::File result(lines[0].trim());
            const auto type = lines[1].trim(), name = lines[2].trim();
            juce::String verdict("no\tits driver did not open");
            // ORGAN_TEST_HELPER_ABSENT="part;part" (tests; "*" = every card): a card whose name holds a part is "not there".
            bool pretendAbsent = false;
            if (const char* env = std::getenv("ORGAN_TEST_HELPER_ABSENT"))
                for (const auto& part : juce::StringArray::fromTokens(juce::String::fromUTF8(env), ";", ""))
                    if (part.trim() == "*" || (part.trim().isNotEmpty() && name.containsIgnoreCase(part.trim()))) pretendAbsent = true;
            if (pretendAbsent) verdict = "no\tnot there (pretended by ORGAN_TEST_HELPER_ABSENT)";
            else
            {
                juce::AudioDeviceManager probe;
                probe.getAvailableDeviceTypes();
                juce::XmlElement setup("DEVICESETUP");
                setup.setAttribute("deviceType", type);
                setup.setAttribute("audioOutputDeviceName", name);
                setup.setAttribute("audioInputDeviceName", "");
                // A fourth line of the request: further attributes of the device state, "name=value;name=value" (the
                // rate, the buffer, the channels the program itself would open the card with).
                if (lines.size() >= 4)
                    for (const auto& pair : juce::StringArray::fromTokens(lines[3], ";", ""))
                        if (pair.containsChar('=')) setup.setAttribute(pair.upToFirstOccurrenceOf("=", false, false).trim(), pair.fromFirstOccurrenceOf("=", false, false).trim());
                const auto error = probe.initialise(0, 2, &setup, false);
                if (auto* device = probe.getCurrentAudioDevice(); device != nullptr && device->getName() == name)
                    verdict = "ok\t" + juce::String(device->getOutputChannelNames().size()) + " outputs, " + juce::String(device->getActiveOutputChannels().countNumberOfSetBits()) + " open, "
                        + juce::String(juce::roundToInt(device->getCurrentSampleRate())) + " Hz, buffer " + juce::String(device->getCurrentBufferSizeSamples());
                else verdict = "no\t" + (error.trim().isNotEmpty() ? error.trim() : juce::String("its driver did not open"));
                probe.closeAudioDevice();
            }
            result.replaceWithText(verdict);
        }
        std::fflush(nullptr);
        std::_Exit(0);   // no destructors: a driver's left-over threads must not hold the exit
    }

    struct Result { juce::String type, name; bool ok = false; juce::String reason; int tookMs = 0; };

    class Checker final : private juce::Timer
    {
    public:
        std::function<void(const Result&)> onResult;   // on the message thread

        // Queued once: a card already waiting or being asked is not queued again.
        void ask(const juce::String& type, const juce::String& name)
        {
            if (current.name == name && current.type == type && running) return;
            for (const auto& waiting : queue) if (waiting.first == type && waiting.second == name) return;
            queue.push_back({ type, name });
            if (!isTimerRunning()) startTimer(100);
        }
        bool busy() const { return running || !queue.empty(); }
        ~Checker() override { stopTimer(); if (running) process.kill(); requestFile.deleteFile(); resultFile.deleteFile(); }

    private:
        void timerCallback() override
        {
            const double nowMs = juce::Time::getMillisecondCounterHiRes();
            if (running)
            {
                const bool finished = !process.isRunning();
                if (!finished && nowMs - startedMs < 8000.0) return;
                Result result = current;
                result.tookMs = juce::roundToInt(nowMs - startedMs);
                if (!finished)
                {
                    process.kill();
                    result.reason = "its driver does not answer (asked for 8 s)";
                }
                else
                {
                    const auto text = resultFile.loadFileAsString().trim();
                    if (text == "ok" || text.startsWith("ok\t")) result.ok = true;
                    else if (text.startsWith("no")) result.reason = text.fromFirstOccurrenceOf("\t", false, false).trim();
                    else result.reason = "its driver ended the question (it crashed while it was asked)";
                    if (!result.ok && result.reason.isEmpty()) result.reason = "its driver did not open";
                }
                requestFile.deleteFile(); resultFile.deleteFile();
                running = false;
                if (onResult) onResult(result);
            }
            if (queue.empty()) { stopTimer(); return; }
            current = {};
            current.type = queue.front().first; current.name = queue.front().second;
            queue.pop_front();
            const auto stamp = juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
            const auto temp = juce::File::getSpecialLocation(juce::File::tempDirectory);
            requestFile = temp.getChildFile("LibreOrgan-cardcheck-" + stamp + ".request.txt");
            resultFile = temp.getChildFile("LibreOrgan-cardcheck-" + stamp + ".result.txt");
            requestFile.replaceWithText(resultFile.getFullPathName() + "\n" + current.type + "\n" + current.name + "\n");
            juce::StringArray args;
            args.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName());
            args.add("--cardcheck"); args.add(requestFile.getFullPathName());
            startedMs = nowMs;
            running = process.start(args, 0);
            if (!running)
            {
                Result result = current;
                result.reason = "the question could not be started";
                requestFile.deleteFile();
                if (onResult) onResult(result);
            }
        }

        std::deque<std::pair<juce::String, juce::String>> queue;
        Result current;
        QuietProcess process;   // 0.9.108: started without the system's "starting an application" cursor
        juce::File requestFile, resultFile;
        double startedMs = 0.0;
        bool running = false;
    };
}
