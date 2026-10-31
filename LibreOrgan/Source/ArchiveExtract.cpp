#include "ArchiveExtract.h"
#include "CrashHandler.h"

#include <archive.h>
#include <archive_entry.h>
#include <set>

bool ArchiveExtractor::looksLikeArchive(const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".rar";
}

juce::String ArchiveExtractor::fileFilter()
{
    return "*.rar;*.RAR";
}

juce::String ArchiveExtractor::libraryVersion()
{
    return juce::String(archive_version_string());   // "libarchive 3.8.9"
}

namespace
{
    // The entry's path inside the destination, or an empty file when the name
    // leads outside it: every component is checked, nothing is resolved later.
    juce::File targetOf(const juce::File& destination, const juce::String& pathInArchive)
    {
        juce::String path = pathInArchive.replaceCharacter('\\', '/').trim();
        while (path.startsWithChar('/')) path = path.substring(1);
        if (path.isEmpty()) return {};
        if (path.length() > 1 && path[1] == ':') return {};   // "C:..." written into a RAR
        juce::File target = destination;
        for (const auto& part : juce::StringArray::fromTokens(path, "/", ""))
        {
            const auto component = part.trim();
            if (component.isEmpty() || component == ".") continue;
            if (component == "..") return {};
            target = target.getChildFile(component);
        }
        return target == destination ? juce::File() : target;
    }
}

ArchiveExtractor::Result ArchiveExtractor::extract(const juce::File& archiveFile, const juce::File& destination,
    const std::function<bool(double, const juce::String&)>& progress)
{
    Result result;
    if (!archiveFile.existsAsFile()) { result.error = "The archive does not exist."; return result; }
    if (!destination.createDirectory()) { result.error = "The destination folder could not be created."; return result; }

    struct archive* a = archive_read_new();
    if (a == nullptr) { result.error = "libarchive could not be started."; return result; }
    archive_read_support_format_rar(a);
    archive_read_support_format_rar5(a);
    archive_read_support_filter_none(a);
   #if JUCE_WINDOWS
    const int opened = archive_read_open_filename_w(a, archiveFile.getFullPathName().toWideCharPointer(), 1 << 20);
   #else
    const int opened = archive_read_open_filename(a, archiveFile.getFullPathName().toRawUTF8(), 1 << 20);
   #endif
    if (opened != ARCHIVE_OK)
    {
        result.error = "The archive could not be opened: " + juce::String(juce::CharPointer_UTF8(archive_error_string(a) != nullptr ? archive_error_string(a) : "unknown error"));
        archive_read_free(a);
        return result;
    }
    const double archiveBytes = (double)juce::jmax((juce::int64)1, archiveFile.getSize());   // no explicit template argument: juce_dsp's jmax for SIMDRegister would be instantiated (GCC)
    std::set<juce::String> topLevel;
    juce::HeapBlock<char> copyBuffer;
    for (;;)
    {
        struct archive_entry* entry = nullptr;
        const int header = archive_read_next_header(a, &entry);
        if (header == ARCHIVE_EOF) break;
        if (header == ARCHIVE_FATAL || entry == nullptr)
        {
            result.error = "The archive could not be read: " + juce::String(juce::CharPointer_UTF8(archive_error_string(a) != nullptr ? archive_error_string(a) : "unknown error"))
                + (result.files > 0 ? " (" + juce::String(result.files) + " file(s) were unpacked before)" : juce::String());
            break;
        }
        if (header == ARCHIVE_RETRY) continue;
        // The name: UTF-8 when the archive says so, the wide form otherwise.
        juce::String name;
        if (const char* utf8 = archive_entry_pathname_utf8(entry)) name = juce::String(juce::CharPointer_UTF8(utf8));
        else if (const wchar_t* wide = archive_entry_pathname_w(entry)) name = juce::String(wide);
        else if (const char* raw = archive_entry_pathname(entry)) name = juce::String(juce::CharPointer_UTF8(raw));
        const double share = juce::jlimit(0.0, 1.0, (double)archive_filter_bytes(a, -1) / archiveBytes);
        if (!progress(share, name)) { result.cancelled = true; break; }
        const auto target = targetOf(destination, name);
        const auto type = archive_entry_filetype(entry);
        if (target == juce::File() || (type != AE_IFREG && type != AE_IFDIR))
        {
            ++result.skipped;
            archive_read_data_skip(a);
            continue;
        }
        {
            juce::String top = name.replaceCharacter('\\', '/');
            while (top.startsWithChar('/')) top = top.substring(1);
            topLevel.insert(top.upToFirstOccurrenceOf("/", false, false));
        }
        if (type == AE_IFDIR)
        {
            if (target.createDirectory()) ++result.directories;
            else ++result.skipped;
            continue;
        }
        target.getParentDirectory().createDirectory();
        if (target.exists()) target.deleteFile();
        auto stream = std::unique_ptr<juce::FileOutputStream>(target.createOutputStream());
        if (stream == nullptr || !stream->openedOk())
        {
            result.error = "The file could not be written: " + target.getFullPathName();
            break;
        }
        bool ok = true;
        for (;;)
        {
            const void* block = nullptr;
            size_t size = 0;
            la_int64_t offset = 0;
            const int r = archive_read_data_block(a, &block, &size, &offset);
            if (r == ARCHIVE_EOF) break;
            if (r < ARCHIVE_WARN)
            {
                result.error = "The archive could not be unpacked: " + juce::String(juce::CharPointer_UTF8(archive_error_string(a) != nullptr ? archive_error_string(a) : "unknown error"))
                    + " (" + target.getFileName() + ")";
                ok = false;
                break;
            }
            if (offset != stream->getPosition()) stream->setPosition(offset);   // a sparse gap: zeros
            if (size > 0 && !stream->write(block, size))
            {
                result.error = "The disk is full or the file could not be written: " + target.getFullPathName();
                ok = false;
                break;
            }
            result.bytes += (juce::int64)size;
        }
        stream->flush();
        stream.reset();
        if (!ok) { target.deleteFile(); break; }
        ++result.files;
    }
    archive_read_close(a);
    archive_read_free(a);
    if (topLevel.size() == 1 && topLevel.begin()->isNotEmpty()) result.topFolder = destination.getChildFile(*topLevel.begin());
    CrashHandler::note("Archive " + archiveFile.getFullPathName() + " -> " + destination.getFullPathName() + ": " + juce::String(result.files)
        + " file(s), " + juce::String(result.directories) + " folder(s), " + juce::String(result.skipped) + " skipped, "
        + juce::String(result.bytes / (1024 * 1024)) + " MB" + (result.cancelled ? ", cancelled" : "") + (result.error.isNotEmpty() ? ", ERROR " + result.error : ""));
    return result;
}
