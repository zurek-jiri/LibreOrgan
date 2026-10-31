#pragma once

#include <JuceHeader.h>
#include <functional>

//==============================================================================
// Unpacking of sample archives inside the program: RAR archives (RAR 4 and
// RAR 5, the form most sample folders are distributed in) read by libarchive
// (BSD licence, ThirdParty/libarchive) and written as plain files into a folder
// of the user's choice - for the computers without an unpacker with a window
// (macOS above all). Only what sample folders need is written: directories and
// regular files. Links, device nodes and names that would lead outside the
// destination (absolute paths, "..") are skipped and counted.
struct ArchiveExtractor
{
    struct Result
    {
        int files = 0, directories = 0, skipped = 0;
        juce::int64 bytes = 0;
        juce::String error;          // empty = success (or cancelled)
        bool cancelled = false;
        juce::File topFolder;        // the single top-level folder the archive unpacked into, if it has one
    };

    static bool looksLikeArchive(const juce::File& file);
    static juce::String fileFilter();   // the pattern for the file chooser

    // Unpacks 'archive' into 'destination' (created when missing). 'progress'
    // is called from the calling thread with the share done (0..1) and the
    // current entry's name; returning false cancels. Blocking: call it on a
    // worker thread (juce::ThreadWithProgressWindow).
    static Result extract(const juce::File& archive, const juce::File& destination,
        const std::function<bool(double, const juce::String&)>& progress);

    // The version of the library doing the work ("libarchive 3.8.9").
    static juce::String libraryVersion();
};
