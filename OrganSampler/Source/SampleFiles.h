#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

// Sample folders that may lie inside RAR archives (0.8.157). A path is a disk
// path as ever ("E:\sonus\caen\OrganInstallationPackages\001318\pipe\ped"); what
// is not on the disk is looked up in the RAR archives lying in the nearest
// existing ancestor folder (the organ's folder with its archives: every first
// volume of a multi-volume set, RAR 4 and 5). So a configuration never names an
// archive, the same file serves unpacked and packed sample folders, and an
// unpacked copy on the disk always wins.
//
// Every function takes the plain path; the archive behind it is found and
// indexed once by our own walk of its headers (the index is kept under the
// settings folder, keyed by the archive's size and date). A file is read on its
// own: stored data straight from the volumes, packed data through libarchive fed
// with that file's bytes alone - so the loader reads files in parallel. Only a
// solid archive is read in sequence with a cursor that stays where it stopped.
namespace SampleFiles
{
    bool isDirectory(const juce::File& path);
    bool existsAsFile(const juce::File& path);
    // True when the path is served by an archive (not on the disk).
    bool inArchive(const juce::File& path);
    // The archive a path lies in ("Caen_Surround_1318_1692.CompPkg.Hauptwerk.rar"), empty for disk paths.
    juce::String archiveNameOf(const juce::File& path);
    // Direct subfolders / plain file names of a folder (disk and archives together; no stat per entry).
    std::vector<juce::File> childDirectories(const juce::File& dir);
    std::vector<juce::String> fileNames(const juce::File& dir);
    std::vector<juce::File> subdirectoriesRecursive(const juce::File& top);
    // The bytes of the files of a folder (and its subfolders when 'recursive') whose names end with one of the extensions.
    juce::int64 totalBytes(const juce::File& dir, const juce::StringArray& extensions, bool recursive);
    // A folder's files: for a disk folder the names alone (read from the disk as
    // ever); for an archive folder 'archived' is set and the data is either there
    // (a solid archive, read in one pass) or to be fetched with readFile - by the
    // caller's threads, in parallel.
    struct FileData { juce::String name; std::shared_ptr<juce::MemoryBlock> data; bool archived = false; };
    std::vector<FileData> readFolderFiles(const juce::File& dir, const juce::StringArray& extensions);
    // An archive entry read whole (nullptr for a disk path or when it cannot be read - logged).
    std::shared_ptr<juce::MemoryBlock> readFile(const juce::File& file);
    // One file read whole as a stream (disk or archive); nullptr when absent.
    std::unique_ptr<juce::InputStream> openInput(const juce::File& file);
    // The position of a folder's first entry in its archive (the loader orders
    // the folders by it); -1 for disk folders.
    juce::int64 archiveOrderOf(const juce::File& dir);
    // The archives (first volumes) directly in a disk folder, and the top-level
    // folders they hold: what "Load folder" with archives chosen loads as trees.
    std::vector<juce::File> archivesIn(const juce::File& folder);
    bool isFirstVolume(const juce::File& archive);
    std::vector<juce::File> volumesOf(const juce::File& firstVolume);
    // A later part of a multi-part set -> its first part; the chosen archives as sets,
    // each once (0.8.176: a user choosing part02 or all the parts gets the set once).
    juce::File firstVolumeOf(const juce::File& file);
    std::vector<juce::File> firstVolumes(const std::vector<juce::File>& archives);
    // The volumes an indexed set was read from (one for a single archive).
    std::vector<juce::File> volumesRead(const juce::File& archive);
    std::vector<juce::String> topLevelFoldersOf(const juce::File& archive);
    // Every file name (with its path inside) of an archive matching a predicate - the Hauptwerk definition search.
    std::vector<juce::String> entriesOf(const juce::File& archive, const std::function<bool(const juce::String&)>& wanted);
    // Builds (or loads from the cache) the indexes of these archives - to be called on
    // a worker thread before the archives are used: a 60 GB set on a hard disk takes
    // minutes of seeks. indexingProgress() tells where the walk is ("" when none runs).
    void indexArchives(const std::vector<juce::File>& archives);
    juce::String indexingProgress();
    // "RAR 5, 17938 file(s), 27081 MB" (tests and the log).
    juce::String archiveSummary(const juce::File& archive);
    // Folders whose archives serve paths lying ELSEWHERE (0.8.158): a path not on
    // the disk and not in an archive beside it is looked for in these archives by
    // the longest tail of the path they hold ("OrganInstallationPackages/002735/
    // pipes/PE"). The owner sets them from the configuration's folder: the folder
    // itself, the folders listed in its ArchiveFolders.txt, OrganRarPackages beside.
    void setSearchFolders(const std::vector<juce::File>& folders);
    // The folder the archive indexes are kept in (under the settings folder).
    void setIndexFolder(const juce::File& folder);
    void forgetArchives();   // drops the open cursors and the indexes in memory (the files on the disk stay)
}
