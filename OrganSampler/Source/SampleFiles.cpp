#include "SampleFiles.h"
#include "CrashHandler.h"

#include <archive.h>
#include <archive_entry.h>
#if ORGAN_HAVE_UNARR
extern "C" {
#include <unarr.h>
// ar_open_stream is unarr's own (common/unarr-imp.h, not in unarr.h): declared here with its signature.
ar_stream* ar_open_stream(void* data, void (*close)(void*), size_t (*read)(void*, void*, size_t), bool (*seek)(void*, off64_t, int), off64_t (*tell)(void*));
}
#endif
#include <algorithm>
#include <atomic>
#include <cstring>
#include <map>
#include <mutex>
#include <set>

namespace
{
    juce::String slashes(juce::String path) { return path.replaceCharacter('\\', '/'); }
    juce::String lower(const juce::String& s) { return s.toLowerCase(); }

    // Multi-volume sets: "name.part1.rar" (+ part2 ...), "name.part01.rar", or the
    // old "name.rar" + "name.r00", "name.r01" ... Only the first volume is an
    // archive of its own; the others are its continuation.
    int partNumberOf(const juce::File& file)
    {
        const auto name = file.getFileName().toLowerCase();
        if (!name.endsWith(".rar")) return -1;
        const auto stem = name.dropLastCharacters(4);
        const int at = stem.lastIndexOf(".part");
        if (at < 0) return -1;
        const auto digits = stem.substring(at + 5);
        if (digits.isEmpty() || !digits.containsOnly("0123456789")) return -1;
        return digits.getIntValue();
    }
    bool isOldVolume(const juce::File& file)   // "name.r00" .. "name.r99", "name.s00" ...
    {
        const auto ext = file.getFileExtension().toLowerCase();
        return ext.length() == 4 && (ext[1] == 'r' || ext[1] == 's') && juce::CharacterFunctions::isDigit(ext[2]) && juce::CharacterFunctions::isDigit(ext[3]);
    }

    // The index of one archive, walked by our own header reader (RAR 4 and RAR 5):
    // every entry's path, size, order and the PLACE of its packed data - a file
    // split over volumes has one piece per volume. libarchive is given, per file,
    // a stream of just those bytes (the volume's signature and main block, the
    // file block, its data, and the end block + the next volume's head between the
    // pieces), so a file is read on its own, in parallel with others, and nothing
    // libarchive stumbles on in a real volume set (a recovery record, padding
    // after the end block, its signature scan missing a 512-byte chunk edge) is
    // ever seen. Stored files (no compression) are read straight from the volumes.
    struct Piece { int volume = 0; juce::int64 headerOffset = 0, headerLength = 0, dataOffset = 0, dataLength = 0; };
    struct VolumeInfo { juce::int64 mainOffset = 0, mainLength = 0, endOffset = 0, endLength = 0; };

    struct ArchiveIndex
    {
        juce::File first;                 // the first volume
        std::vector<juce::File> volumes;
        std::vector<VolumeInfo> volumeInfo;
        int format = 0;                   // 4 or 5
        bool solid = false;               // a solid archive: the files depend on each other, read in sequence
        struct Entry
        {
            juce::String path; juce::int64 size = 0; juce::int64 order = 0; bool dir = false;
            bool stored = false, encrypted = false;
            std::vector<Piece> pieces;
        };
        std::vector<Entry> entries;
        std::map<juce::String, size_t> byPath;        // lower-case path -> entry
        std::set<juce::String> dirs;                  // lower-case paths of every folder (explicit or implied)
        std::map<juce::String, std::vector<size_t>> children;   // lower-case folder ("" = root) -> entries directly in it
        std::map<juce::String, std::set<juce::String>> childDirs;   // lower-case folder -> lower-case child folder paths
        std::map<juce::String, juce::String> spelling;              // lower-case path -> the path as spelt
        bool valid = false;
        juce::String error;

        int signatureLength() const { return format == 5 ? 8 : 7; }

        static juce::String normalise(const juce::String& rawPath)
        {
            juce::String path = slashes(rawPath).trim();
            while (path.startsWithChar('/')) path = path.substring(1);
            while (path.endsWithChar('/')) path = path.dropLastCharacters(1);
            if (path.contains("../") || path.startsWith("..")) return {};
            return path;
        }
        void addDirectory(const juce::String& path)
        {
            if (path.isEmpty()) return;
            const auto key = lower(path);
            if (dirs.count(key) != 0) return;
            dirs.insert(key);
            spelling[key] = path;
            const auto parent = path.contains("/") ? path.upToLastOccurrenceOf("/", false, false) : juce::String();
            addDirectory(parent);
            childDirs[lower(parent)].insert(key);
        }
        // Returns the entry's index, -1 when the path is empty, outside, or already there.
        int addEntry(const juce::String& rawPath, juce::int64 size, bool dir, juce::int64 order)
        {
            const juce::String path = normalise(rawPath);
            if (path.isEmpty()) return -1;
            if (dir) { addDirectory(path); return -1; }
            const auto key = lower(path);
            if (byPath.count(key) != 0) return -1;
            const auto parent = path.contains("/") ? path.upToLastOccurrenceOf("/", false, false) : juce::String();
            addDirectory(parent);
            byPath[key] = entries.size();
            spelling[key] = path;
            children[lower(parent)].push_back(entries.size());
            Entry entry;
            entry.path = path; entry.size = size; entry.order = order;
            entries.push_back(std::move(entry));
            return (int)entries.size() - 1;
        }
    };

    juce::File gIndexFolder;
    std::mutex gMutex;
    std::atomic<int> gIndexingFiles{ -1 };   // the walk's progress: files found so far (-1 = no walk running)
    juce::String gIndexingName;             // the archive being walked (set under gMutex before the walk)
    std::atomic<int> gIndexingVolume{ 0 }, gIndexingVolumes{ 0 };   // the part being walked and the set's size (0.8.176)
    juce::StringArray gWalking;             // archives being walked right now (keys; under gMutex) - the walk runs with the mutex released
    std::map<juce::String, std::shared_ptr<ArchiveIndex>> gIndexes;      // lower-case first-volume path -> index
    std::map<juce::String, std::vector<juce::File>> gArchivesOfFolder;   // lower-case disk folder -> its first volumes (scanned once)
    std::vector<juce::File> gSearchFolders;   // folders named as a whole whose archives serve paths (0.8.158; folder lines of ArchiveFolders.txt, OrganRarPackages)
    std::vector<juce::File> gChosenArchives;  // the archives the user chose (first volumes): the only ones that serve paths otherwise (0.8.177)

    // ---- the RAR header walk ------------------------------------------------------------
    const juce::uint8 kSignature4[7] = { 'R', 'a', 'r', '!', 0x1a, 0x07, 0x00 };
    const juce::uint8 kSignature5[8] = { 'R', 'a', 'r', '!', 0x1a, 0x07, 0x01, 0x00 };

    juce::uint32 le32(const juce::uint8* p) { return (juce::uint32)p[0] | ((juce::uint32)p[1] << 8) | ((juce::uint32)p[2] << 16) | ((juce::uint32)p[3] << 24); }
    juce::uint32 le16(const juce::uint8* p) { return (juce::uint32)p[0] | ((juce::uint32)p[1] << 8); }

    // RAR 5 variable-length integer: seven bits per byte, the top bit says "more".
    bool readVint(const juce::uint8* p, size_t avail, size_t& at, juce::uint64& value)
    {
        value = 0;
        for (int shift = 0; shift < 70 && at < avail; shift += 7)
        {
            const juce::uint8 b = p[at++];
            value |= (juce::uint64)(b & 0x7f) << shift;
            if ((b & 0x80) == 0) return true;
        }
        return false;
    }

    // RAR 4 names: the name in the archiver's code page, or (FHD_UNICODE) that name,
    // a nul, and the UTF-16 name encoded against it - the scheme of unrar's
    // DecodeFileName, as libarchive decodes it.
    juce::String decodeRar4Name(const juce::uint8* p, size_t size, bool unicode)
    {
        size_t asciiLength = 0;
        while (asciiLength < size && p[asciiLength] != 0) ++asciiLength;
        if (!unicode || asciiLength >= size) return juce::String::createStringFromData(p, (int)asciiLength);
        std::vector<juce::uint16> units;
        size_t offset = asciiLength + 1;
        const juce::uint8 highByte = offset < size ? p[offset++] : 0;
        int flagBits = 0;
        juce::uint8 flagByte = 0;
        while (offset < size && units.size() < size)
        {
            if (flagBits == 0) { flagByte = p[offset++]; flagBits = 8; }
            flagBits -= 2;
            switch ((flagByte >> flagBits) & 3)
            {
                case 0: if (offset >= size) continue; units.push_back((juce::uint16)p[offset++]); break;
                case 1: if (offset >= size) continue; units.push_back((juce::uint16)(((juce::uint32)highByte << 8) | p[offset++])); break;
                case 2:
                    if (offset + 1 >= size) { offset = size; continue; }
                    units.push_back((juce::uint16)((juce::uint32)p[offset] | ((juce::uint32)p[offset + 1] << 8)));
                    offset += 2;
                    break;
                default:
                {
                    if (offset >= size) continue;
                    juce::uint8 length = p[offset++];
                    juce::uint8 extra = 0, high = 0;
                    if (length & 0x80)
                    {
                        if (offset >= size) continue;
                        extra = p[offset++];
                        high = highByte;
                    }
                    length = (juce::uint8)((length & 0x7f) + 2);
                    while (length > 0 && units.size() < size)
                    {
                        const size_t cp = units.size();
                        units.push_back((juce::uint16)(((juce::uint32)high << 8) | (juce::uint8)(p[cp] + extra)));
                        --length;
                    }
                    break;
                }
            }
        }
        while (!units.empty() && units.back() == 0) units.pop_back();
        juce::String result;
        for (size_t i = 0; i < units.size(); ++i)
        {
            juce::uint32 code = units[i];
            if (code >= 0xd800 && code < 0xdc00 && i + 1 < units.size() && units[i + 1] >= 0xdc00 && units[i + 1] < 0xe000)
            {
                code = 0x10000 + ((code - 0xd800) << 10) + (units[i + 1] - 0xdc00);
                ++i;
            }
            result += juce::String::charToString((juce::juce_wchar)code);
        }
        return result;
    }

    struct Walker
    {
        ArchiveIndex& index;
        juce::int64 order = 0;
        int splitEntry = -1;   // the entry whose data goes on in the next volume

        explicit Walker(ArchiveIndex& i) : index(i) {}

        bool fail(const juce::String& what) { if (index.error.isEmpty()) index.error = what; return false; }

        void fileFound(const juce::String& name, juce::int64 size, bool dir, bool stored, bool encrypted, bool splitBefore, bool splitAfter, const Piece& piece)
        {
            if (dir) { index.addEntry(name, 0, true, order++); splitEntry = -1; return; }
            int at = -1;
            if (splitBefore && splitEntry >= 0 && index.entries[(size_t)splitEntry].path.equalsIgnoreCase(ArchiveIndex::normalise(name))) at = splitEntry;
            else { at = index.addEntry(name, size, false, order++); gIndexingFiles.store((int)index.entries.size(), std::memory_order_relaxed); }
            splitEntry = -1;
            if (at < 0) return;
            auto& entry = index.entries[(size_t)at];
            entry.stored = stored;
            entry.encrypted = entry.encrypted || encrypted;
            entry.pieces.push_back(piece);
            if (splitAfter) splitEntry = at;
        }

        bool walkVolume(int vol)
        {
            const auto& file = index.volumes[(size_t)vol];
            gIndexingVolume.store(vol + 1, std::memory_order_relaxed);   // the progress text names the part being read (0.8.176)
            gIndexingVolumes.store((int)index.volumes.size(), std::memory_order_relaxed);
            juce::FileInputStream in(file);
            if (!in.openedOk()) return fail(file.getFileName() + " cannot be opened");
            juce::uint8 signature[8] = {};
            const int got = in.read(signature, 8);
            int format = 0;
            if (got >= 7 && std::memcmp(signature, kSignature4, 7) == 0) format = 4;
            if (got >= 8 && std::memcmp(signature, kSignature5, 8) == 0) format = 5;
            if (format == 0) return fail(file.getFileName() + " is not a RAR archive");
            if (vol == 0) index.format = format;
            else if (format != index.format) return fail(file.getFileName() + " is a volume of another RAR version");
            index.volumeInfo.resize(index.volumes.size());
            return format == 5 ? walk5(in, vol) : walk4(in, vol);
        }

        // One read per block: a header is a few dozen bytes, so a chunk from its start
        // holds it whole nearly always (a seek per block on a hard disk is what the
        // walk costs; libarchive's way read two or three times per block).
        static constexpr int kChunk = 1024;
        juce::uint8 chunk[kChunk];
        int chunkBytes = 0;
        bool readAt(juce::FileInputStream& in, juce::int64 pos, int minimum)
        {
            if (!in.setPosition(pos)) return false;
            chunkBytes = in.read(chunk, kChunk);
            return chunkBytes >= minimum;
        }
        // The whole header of 'size' bytes from 'pos': the chunk when it holds it, else read again.
        bool headerAt(juce::FileInputStream& in, juce::int64 pos, int size, std::vector<juce::uint8>& header)
        {
            header.resize((size_t)size);
            if (size <= chunkBytes) { std::memcpy(header.data(), chunk, (size_t)size); return true; }
            return in.setPosition(pos) && in.read(header.data(), size) == size;
        }

        // RAR 4: blocks of CRC(2) TYPE(1) FLAGS(2) SIZE(2) [ADD_SIZE(4)]; a FILE block's
        // ADD_SIZE is its packed data, which follows the header.
        bool walk4(juce::FileInputStream& in, int vol)
        {
            auto& info = index.volumeInfo[(size_t)vol];
            const juce::int64 fileSize = in.getTotalLength();
            juce::int64 pos = 7;
            std::vector<juce::uint8> header;
            while (pos + 7 <= fileSize)
            {
                if (!readAt(in, pos, 7)) break;
                const juce::uint8* head = chunk;
                const int type = head[2];
                const juce::uint32 flags = le16(head + 3);
                const juce::int64 size = (juce::int64)le16(head + 5);
                if (size < 7) break;   // padding or damage after the last block: the volume ends here
                juce::int64 add = 0;
                if ((flags & 0x8000) != 0 || type == 0x74 || type == 0x7a)
                {
                    if (size < 11 || chunkBytes < 11) break;
                    add = (juce::int64)le32(head + 7);
                }
                if (type == 0x73)   // MAIN
                {
                    if (flags & 0x0080) return fail("the archive is encrypted");
                    if (vol == 0) index.solid = (flags & 0x0008) != 0;
                    info.mainOffset = pos;
                    info.mainLength = size + add;
                }
                else if (type == 0x74 || type == 0x7a)   // FILE, NEWSUB (a recovery record, a comment ...)
                {
                    if (!headerAt(in, pos, (int)size, header)) break;
                    if (type == 0x74 && header.size() >= 32)
                    {
                        const juce::uint8* p = header.data() + 7;
                        const size_t bodySize = header.size() - 7;
                        juce::int64 packed = (juce::int64)le32(p), unpacked = (juce::int64)le32(p + 4);
                        const int method = p[18];
                        const size_t nameSize = le16(p + 19);
                        size_t at = 25;
                        if (flags & 0x0100)   // LARGE: the high halves of both sizes
                        {
                            if (bodySize < 33) break;
                            packed |= (juce::int64)le32(p + 25) << 32;
                            unpacked |= (juce::int64)le32(p + 29) << 32;
                            at = 33;
                        }
                        if (at + nameSize > bodySize) break;
                        const auto name = decodeRar4Name(p + at, nameSize, (flags & 0x0200) != 0);
                        Piece piece;
                        piece.volume = vol; piece.headerOffset = pos; piece.headerLength = size; piece.dataOffset = pos + size; piece.dataLength = packed;
                        fileFound(name, unpacked, (flags & 0x00e0) == 0x00e0, method == 0x30, (flags & 0x0004) != 0, (flags & 0x0001) != 0, (flags & 0x0002) != 0, piece);
                        add = packed;
                    }
                }
                else if (type == 0x7b)   // ENDARC: the volume ends; a recovery record or padding may follow
                {
                    info.endOffset = pos;
                    info.endLength = size + add;
                    return true;
                }
                pos += size + add;
            }
            return true;
        }

        // RAR 5: blocks of CRC32(4) SIZE(vint) TYPE(vint) FLAGS(vint) [EXTRA SIZE] [DATA SIZE] fields... extra;
        // the data follows the block.
        bool walk5(juce::FileInputStream& in, int vol)
        {
            auto& info = index.volumeInfo[(size_t)vol];
            const juce::int64 fileSize = in.getTotalLength();
            juce::int64 pos = 8;
            std::vector<juce::uint8> header;
            while (pos + 7 <= fileSize)
            {
                if (!readAt(in, pos, 5)) break;
                size_t at = 4;
                juce::uint64 size = 0;
                if (!readVint(chunk, (size_t)chunkBytes, at, size) || size == 0 || size > (2u << 20)) break;
                const juce::int64 headerStart = pos + (juce::int64)at;
                header.resize((size_t)size);
                if ((juce::int64)at + (juce::int64)size <= chunkBytes) std::memcpy(header.data(), chunk + at, (size_t)size);
                else if (!in.setPosition(headerStart) || in.read(header.data(), (int)size) != (int)size) break;
                const juce::uint8* p = header.data();
                size_t q = 0;
                juce::uint64 type = 0, flags = 0, extraSize = 0, dataSize = 0;
                if (!readVint(p, (size_t)size, q, type) || !readVint(p, (size_t)size, q, flags)) break;
                if ((flags & 1) != 0 && !readVint(p, (size_t)size, q, extraSize)) break;
                if ((flags & 2) != 0 && !readVint(p, (size_t)size, q, dataSize)) break;
                const juce::int64 blockEnd = headerStart + (juce::int64)size;
                const juce::int64 blockLength = blockEnd - pos;
                if (type == 1)   // MAIN
                {
                    juce::uint64 archiveFlags = 0;
                    if (!readVint(p, (size_t)size, q, archiveFlags)) break;
                    if (vol == 0) index.solid = (archiveFlags & 4) != 0;
                    info.mainOffset = pos;
                    info.mainLength = blockLength;
                }
                else if (type == 2)   // FILE
                {
                    juce::uint64 fileFlags = 0, unpacked = 0, attributes = 0, compression = 0, hostOs = 0, nameLength = 0;
                    if (!readVint(p, (size_t)size, q, fileFlags) || !readVint(p, (size_t)size, q, unpacked) || !readVint(p, (size_t)size, q, attributes)) break;
                    if (fileFlags & 2) q += 4;   // mtime
                    if (fileFlags & 4) q += 4;   // crc32
                    if (!readVint(p, (size_t)size, q, compression) || !readVint(p, (size_t)size, q, hostOs) || !readVint(p, (size_t)size, q, nameLength)) break;
                    if (q + nameLength > size) break;
                    const auto name = juce::String::fromUTF8((const char*)p + q, (int)nameLength);
                    const int method = (int)((compression >> 7) & 7);
                    bool encrypted = false;
                    if (extraSize > 0 && extraSize <= size)   // the extra records: type 1 = file encryption
                    {
                        size_t e = (size_t)(size - extraSize);
                        while (e < size)
                        {
                            juce::uint64 recordSize = 0, recordType = 0;
                            const size_t recordStart = e;
                            if (!readVint(p, (size_t)size, e, recordSize)) break;
                            const size_t recordEnd = e + (size_t)recordSize;
                            if (!readVint(p, (size_t)size, e, recordType) || recordEnd > size || recordEnd <= recordStart) break;
                            if (recordType == 1) encrypted = true;
                            e = recordEnd;
                        }
                    }
                    Piece piece;
                    piece.volume = vol; piece.headerOffset = pos; piece.headerLength = blockLength; piece.dataOffset = blockEnd; piece.dataLength = (juce::int64)dataSize;
                    fileFound(name, (juce::int64)unpacked, (fileFlags & 1) != 0, method == 0, encrypted, (flags & 8) != 0, (flags & 16) != 0, piece);
                }
                else if (type == 4) return fail("the archive is encrypted");
                else if (type == 5)   // ENDARC
                {
                    info.endOffset = pos;
                    info.endLength = blockLength;
                    return true;
                }
                pos = blockEnd + (juce::int64)dataSize;
            }
            return true;
        }

        bool walk()
        {
            for (int vol = 0; vol < (int)index.volumes.size(); ++vol)
                if (!walkVolume(vol)) return false;
            return true;
        }
    };

    void buildIndex(ArchiveIndex& index)
    {
        Walker walker(index);
        walker.walk();
        index.valid = index.error.isEmpty() && index.format != 0;
    }

    // ---- the index cache --------------------------------------------------------------------
    juce::String indexCacheName(const juce::File& first)
    {
        return juce::String::toHexString(first.getFullPathName().toLowerCase().hashCode64()) + ".idx";
    }

    bool readCachedIndex(ArchiveIndex& index)
    {
        if (gIndexFolder == juce::File()) return false;
        const auto cache = gIndexFolder.getChildFile(indexCacheName(index.first));
        if (!cache.existsAsFile()) return false;
        juce::StringArray lines;
        lines.addLines(cache.loadFileAsString());
        if (lines.size() < 1) return false;
        const auto head = juce::StringArray::fromTokens(lines[0], "|", "");
        if (head.size() < 6 || head[0] != "v2") return false;
        if ((juce::int64)head[2].getLargeIntValue() != index.first.getSize()
            || (juce::int64)head[3].getLargeIntValue() != index.first.getLastModificationTime().toMilliseconds()) return false;
        index.format = head[4].getIntValue();
        index.solid = head[5] == "1";
        for (int i = 1; i < lines.size(); ++i)
        {
            const auto& line = lines[i];
            if (line.startsWith("V "))
            {
                const auto fields = juce::StringArray::fromTokens(line.substring(2), "|", "");
                if (fields.size() < 4) return false;
                VolumeInfo info;
                info.mainOffset = fields[0].getLargeIntValue(); info.mainLength = fields[1].getLargeIntValue();
                info.endOffset = fields[2].getLargeIntValue(); info.endLength = fields[3].getLargeIntValue();
                index.volumeInfo.push_back(info);
            }
            else if (line.startsWith("D ")) index.addEntry(line.substring(2), 0, true, 0);
            else if (line.startsWith("F "))
            {
                // "F order|size|stored|encrypted|pieces|path" - the path last, it may hold anything.
                juce::String rest = line.substring(2);
                juce::StringArray fields;
                for (int f = 0; f < 5; ++f)
                {
                    const int bar = rest.indexOfChar('|');
                    if (bar < 0) return false;
                    fields.add(rest.substring(0, bar));
                    rest = rest.substring(bar + 1);
                }
                const int at = index.addEntry(rest, fields[1].getLargeIntValue(), false, fields[0].getLargeIntValue());
                if (at < 0) continue;
                auto& entry = index.entries[(size_t)at];
                entry.stored = fields[2] == "1";
                entry.encrypted = fields[3] == "1";
                for (const auto& pieceText : juce::StringArray::fromTokens(fields[4], ";", ""))
                {
                    const auto numbers = juce::StringArray::fromTokens(pieceText, ":", "");
                    if (numbers.size() < 5) return false;
                    Piece piece;
                    piece.volume = numbers[0].getIntValue(); piece.headerOffset = numbers[1].getLargeIntValue(); piece.headerLength = numbers[2].getLargeIntValue();
                    piece.dataOffset = numbers[3].getLargeIntValue(); piece.dataLength = numbers[4].getLargeIntValue();
                    if (piece.volume < 0 || piece.volume >= (int)index.volumes.size()) return false;
                    entry.pieces.push_back(piece);
                }
            }
        }
        if (index.volumeInfo.size() != index.volumes.size() || (index.format != 4 && index.format != 5)) return false;
        index.valid = true;
        return true;
    }

    void writeCachedIndex(const ArchiveIndex& index)
    {
        if (gIndexFolder == juce::File()) return;
        gIndexFolder.createDirectory();
        // A stream, not a String: appending to a String copies it every time (18 000 entries took minutes).
        juce::MemoryOutputStream text;
        text << "v2|" << index.first.getFullPathName() << "|" << index.first.getSize() << "|" << index.first.getLastModificationTime().toMilliseconds()
             << "|" << index.format << "|" << (index.solid ? "1" : "0") << "\n";
        for (const auto& info : index.volumeInfo) text << "V " << info.mainOffset << "|" << info.mainLength << "|" << info.endOffset << "|" << info.endLength << "\n";
        for (const auto& dir : index.dirs) text << "D " << index.spelling.at(dir) << "\n";
        for (const auto& entry : index.entries)
        {
            text << "F " << entry.order << "|" << entry.size << "|" << (entry.stored ? "1" : "0") << "|" << (entry.encrypted ? "1" : "0") << "|";
            for (size_t i = 0; i < entry.pieces.size(); ++i)
            {
                const auto& piece = entry.pieces[i];
                text << (i > 0 ? ";" : "") << piece.volume << ":" << piece.headerOffset << ":" << piece.headerLength << ":" << piece.dataOffset << ":" << piece.dataLength;
            }
            text << "|" << entry.path << "\n";
        }
        gIndexFolder.getChildFile(indexCacheName(index.first)).replaceWithData(text.getData(), text.getDataSize());
    }

    // ---- libarchive over the real volumes (solid archives: the files in sequence) ----------
    struct archive* openReader(const std::vector<juce::File>& volumes, juce::String& error)
    {
        struct archive* a = archive_read_new();
        if (a == nullptr) { error = "libarchive could not be started"; return nullptr; }
        archive_read_support_format_rar(a);
        archive_read_support_format_rar5(a);
        archive_read_support_filter_none(a);
        int opened = ARCHIVE_FATAL;
       #if JUCE_WINDOWS
        std::vector<std::wstring> names;
        for (const auto& v : volumes) names.push_back(std::wstring(v.getFullPathName().toWideCharPointer()));
        std::vector<const wchar_t*> pointers;
        for (const auto& n : names) pointers.push_back(n.c_str());
        pointers.push_back(nullptr);
        opened = archive_read_open_filenames_w(a, pointers.data(), 1 << 20);
       #else
        std::vector<std::string> names;
        for (const auto& v : volumes) names.push_back(v.getFullPathName().toStdString());
        std::vector<const char*> pointers;
        for (const auto& n : names) pointers.push_back(n.c_str());
        pointers.push_back(nullptr);
        opened = archive_read_open_filenames(a, pointers.data(), 1 << 20);
       #endif
        if (opened != ARCHIVE_OK)
        {
            error = juce::String(juce::CharPointer_UTF8(archive_error_string(a) != nullptr ? archive_error_string(a) : "the archive could not be opened"));
            archive_read_free(a);
            return nullptr;
        }
        return a;
    }

    juce::String archiveError(struct archive* a, const char* fallback)
    {
        return juce::String(juce::CharPointer_UTF8(archive_error_string(a) != nullptr ? archive_error_string(a) : fallback));
    }

    // Reads the open entry's data whole into a block (nullptr on an error).
    std::shared_ptr<juce::MemoryBlock> readOpenEntry(struct archive* a, juce::int64 expectedSize)
    {
        auto block = std::make_shared<juce::MemoryBlock>();
        if (expectedSize > 0) block->ensureSize((size_t)expectedSize);
        juce::MemoryOutputStream out(*block, false);
        for (;;)
        {
            const void* data = nullptr;
            size_t length = 0;
            la_int64_t offset = 0;
            const int r = archive_read_data_block(a, &data, &length, &offset);
            if (r == ARCHIVE_EOF) break;
            if (r < ARCHIVE_WARN)
            {
                CrashHandler::note("libarchive: data block read returned " + juce::String(r) + " after " + juce::String(out.getPosition()) + " of "
                    + juce::String(expectedSize) + " bytes - " + archiveError(a, "no error text"));
                return nullptr;
            }
            if ((juce::int64)offset != out.getPosition()) out.setPosition((juce::int64)offset);
            if (length > 0) out.write(data, length);
        }
        block->setSize((size_t)out.getPosition(), false);
        return block;
    }

    // The archives of a disk folder that SERVE paths (0.8.177): the archives the user
    // chose (Load folder, Load Configuration, Merge; the archive lines of
    // ArchiveFolders.txt) lying in that folder - never another archive that happens
    // to lie beside them (every .rar of the folder before: the Aeolian-Skinner set's
    // folder also holds its 15 MB definition-only package, whose empty folder was
    // taken) - and every archive of a folder named as a whole (a folder line of
    // ArchiveFolders.txt, OrganRarPackages). Scanned once per folder.
    const std::vector<juce::File>& archivesOfFolderLocked(const juce::File& folder)
    {
        const auto key = lower(folder.getFullPathName());
        auto found = gArchivesOfFolder.find(key);
        if (found != gArchivesOfFolder.end()) return found->second;
        std::vector<juce::File> list;
        for (const auto& archive : gChosenArchives)
            if (archive.getParentDirectory() == folder && archive.existsAsFile()) list.push_back(archive);
        if (std::find(gSearchFolders.begin(), gSearchFolders.end(), folder) != gSearchFolders.end())
            for (const auto& entry : juce::RangedDirectoryIterator(folder, false, "*.rar", juce::File::findFiles))
                if (SampleFiles::isFirstVolume(entry.getFile()) && std::find(list.begin(), list.end(), entry.getFile()) == list.end()) list.push_back(entry.getFile());
        std::sort(list.begin(), list.end(), [](const juce::File& a, const juce::File& b) { return a.getFileName().compareNatural(b.getFileName()) < 0; });
        return gArchivesOfFolder[key] = list;
    }

    // Every archive serving paths that lie elsewhere: the chosen ones and those of the folders named as a whole.
    std::vector<juce::File> servingArchivesLocked()
    {
        std::vector<juce::File> all = gChosenArchives;
        for (const auto& folder : gSearchFolders)
        {
            if (!folder.isDirectory()) continue;
            for (const auto& archive : archivesOfFolderLocked(folder))
                if (std::find(all.begin(), all.end(), archive) == all.end()) all.push_back(archive);
        }
        return all;
    }

    std::shared_ptr<ArchiveIndex> indexOfLocked(const juce::File& given)
    {
        // A later part of a multi-part set (name.part03.rar chosen, or listed) is
        // the set itself: its first part, read once from the start (0.8.176 - each
        // part made a set of its own before: the parts' common definition was found
        // six times, "several definitions").
        const juce::File first = SampleFiles::firstVolumeOf(given);
        const auto key = lower(first.getFullPathName());
        auto found = gIndexes.find(key);
        if (found != gIndexes.end()) return found->second;
        // Being walked by another thread (0.8.172): the message thread never waits
        // for a walk (minutes on a cold disk - the "Reading the archives" window
        // stood frozen with its count, the whole program with it), it gets "no index"
        // for now; any other thread waits for the result.
        if (gWalking.contains(key))
        {
            if (auto* mm = juce::MessageManager::getInstanceWithoutCreating(); mm != nullptr && mm->isThisTheMessageThread()) return nullptr;
            while (gWalking.contains(key)) { gMutex.unlock(); juce::Thread::sleep(50); gMutex.lock(); }
            found = gIndexes.find(key);
            return found != gIndexes.end() ? found->second : nullptr;
        }
        auto index = std::make_shared<ArchiveIndex>();
        index->first = first;
        index->volumes = SampleFiles::volumesOf(first);
        const double started = juce::Time::getMillisecondCounterHiRes();
        const bool cached = readCachedIndex(*index);
        if (!cached)
        {
            *index = ArchiveIndex();
            index->first = first;
            index->volumes = SampleFiles::volumesOf(first);
            gIndexingName = first.getFileName();
            gIndexingFiles.store(0, std::memory_order_relaxed);
            // The walk itself without the mutex: it is called under gMutex (every
            // caller holds it), and the message thread's own lookups (the missing-tree
            // check once a second, the pool) blocked on it for the whole walk.
            gWalking.add(key);
            gMutex.unlock();
            buildIndex(*index);
            gMutex.lock();
            gWalking.removeString(key);
            gIndexingFiles.store(-1, std::memory_order_relaxed);
            if (index->valid) writeCachedIndex(*index);
        }
        juce::int64 bytes = 0, split = 0, stored = 0;
        for (const auto& entry : index->entries) { bytes += entry.size; if (entry.pieces.size() > 1) ++split; if (entry.stored) ++stored; }
        CrashHandler::note("Archive " + first.getFileName() + ": " + juce::String((int)index->entries.size()) + " file(s), "
            + juce::String((int)index->dirs.size()) + " folder(s), " + juce::String((int)index->volumes.size()) + " volume(s), " + juce::String(bytes >> 20) + " MB unpacked"
            + (index->format != 0 ? ", RAR " + juce::String(index->format) : juce::String()) + (index->solid ? ", solid" : "")
            + (split > 0 ? ", " + juce::String(split) + " split over volumes" : juce::String()) + (stored > 0 ? ", " + juce::String(stored) + " stored" : juce::String())
            + (cached ? " (index from the cache)" : " (indexed in " + juce::String(juce::Time::getMillisecondCounterHiRes() - started, 0) + " ms)")
            + (index->error.isNotEmpty() ? " - " + index->error : juce::String()));
        gIndexes[key] = index;
        return index;
    }

    // A folder of an archive with a subfolder or a file under it (0.8.176).
    bool dirHasContent(const ArchiveIndex& index, const juce::String& key)
    {
        if (index.childDirs.count(key) != 0) return true;
        const auto prefix = key + "/";
        const auto it = index.byPath.lower_bound(prefix);
        return it != index.byPath.end() && it->first.startsWith(prefix);
    }

    // Where a path lives: the disk, or an archive of an existing ancestor folder
    // with the path relative to that folder.
    struct Place
    {
        bool onDisk = false;
        std::shared_ptr<ArchiveIndex> index;   // when in an archive
        juce::String relative;                 // the path inside the archive ('/'), spelt as the archive has it
        bool isDir = false, isFile = false;
    };

    Place locate(const juce::File& path)
    {
        Place place;
        if (path == juce::File()) return place;
        if (path.isDirectory()) { place.onDisk = true; place.isDir = true; return place; }
        if (path.existsAsFile()) { place.onDisk = true; place.isFile = true; return place; }
        // Up to the nearest existing folder, then further up: each existing ancestor's
        // archives are asked for the rest of the path.
        juce::File ancestor = path.getParentDirectory();
        juce::String rest = path.getFileName();
        for (int depth = 0; depth < 12 && ancestor != juce::File(); ++depth)
        {
            if (ancestor.isDirectory())
            {
                std::lock_guard<std::mutex> lock(gMutex);
                // A folder held by several archives of the folder (0.8.176: the Aeolian-Skinner
                // set and its 15 MB definition-only archive both hold "OrganInstallationPackages",
                // the small one empty and first by name): the archive with something under it wins.
                std::shared_ptr<ArchiveIndex> emptyDir;
                for (const auto& archive : archivesOfFolderLocked(ancestor))
                {
                    auto index = indexOfLocked(archive);
                    if (index == nullptr || !index->valid) continue;
                    const auto key = lower(rest);
                    if (index->dirs.count(key) != 0)
                    {
                        if (dirHasContent(*index, key)) { place.index = index; place.relative = index->spelling[key]; place.isDir = true; return place; }
                        if (emptyDir == nullptr) emptyDir = index;
                        continue;
                    }
                    if (index->byPath.count(key) != 0) { place.index = index; place.relative = index->spelling[key]; place.isFile = true; return place; }
                }
                if (emptyDir != nullptr) { place.index = emptyDir; place.relative = emptyDir->spelling[lower(rest)]; place.isDir = true; return place; }
            }
            const auto parent = ancestor.getParentDirectory();
            if (parent == ancestor) break;
            rest = ancestor.getFileName() + "/" + rest;
            ancestor = parent;
        }
        // Elsewhere (0.8.158): the serving archives (the chosen ones, those of the
        // folders named as a whole), by the longest tail of the path they hold (at
        // least two parts: "002735/pipes/PE" yes, a bare folder name no).
        std::lock_guard<std::mutex> lock(gMutex);
        const auto serving = servingArchivesLocked();
        if (serving.empty()) return place;
        juce::StringArray parts = juce::StringArray::fromTokens(slashes(path.getFullPathName()), "/", "");
        parts.removeEmptyStrings();
        if (parts.size() > 0 && parts[0].endsWithChar(':')) parts.remove(0);
        std::shared_ptr<ArchiveIndex> emptyDir;
        juce::String emptyKey;
        for (const auto& archive : serving)
        {
            auto index = indexOfLocked(archive);
            if (index == nullptr || !index->valid) continue;
            for (int start = 0; start + 2 <= parts.size(); ++start)
            {
                juce::String key;
                for (int i = start; i < parts.size(); ++i) key << (i > start ? "/" : "") << parts[i];
                key = lower(key);
                if (index->dirs.count(key) != 0)
                {
                    if (dirHasContent(*index, key)) { place.index = index; place.relative = index->spelling[key]; place.isDir = true; return place; }
                    if (emptyDir == nullptr) { emptyDir = index; emptyKey = key; }   // an empty folder of another archive: only when no archive has content (0.8.176)
                    continue;
                }
                if (index->byPath.count(key) != 0) { place.index = index; place.relative = index->spelling[key]; place.isFile = true; return place; }
            }
        }
        if (emptyDir != nullptr) { place.index = emptyDir; place.relative = emptyDir->spelling[emptyKey]; place.isDir = true; return place; }
        return place;
    }

    // ---- one file on its own: a stream of its bytes for libarchive ------------------------------
    struct Range { int volume; juce::int64 offset, length; };   // volume -1: zeros

    struct VirtualStream
    {
        const ArchiveIndex& index;
        std::vector<Range> ranges;
        std::vector<juce::int64> starts;
        juce::int64 total = 0, position = 0;
        std::map<int, std::unique_ptr<juce::FileInputStream>> streams;
        std::vector<char> buffer;
        std::vector<char> synthetic;   // volume -2: bytes of our own (a file header rewritten for unarr, 0.8.217)
        juce::String error;

        explicit VirtualStream(const ArchiveIndex& i) : index(i) { buffer.resize(1 << 20); }

        void add(int volume, juce::int64 offset, juce::int64 length)
        {
            if (length <= 0) return;
            ranges.push_back({ volume, offset, length });
            starts.push_back(total);
            total += length;
        }
        juce::FileInputStream* streamOf(int volume)
        {
            auto& s = streams[volume];
            if (s == nullptr)
            {
                s = std::make_unique<juce::FileInputStream>(index.volumes[(size_t)volume]);
                if (!s->openedOk()) { error = index.volumes[(size_t)volume].getFileName() + " cannot be opened"; s.reset(); return nullptr; }
            }
            return s.get();
        }
        la_ssize_t read(const void** out)
        {
            if (position >= total) return 0;
            const size_t r = (size_t)(std::upper_bound(starts.begin(), starts.end(), position) - starts.begin()) - 1;
            const auto& range = ranges[r];
            const juce::int64 within = position - starts[r];
            const juce::int64 n = std::min<juce::int64>((juce::int64)buffer.size(), range.length - within);
            if (range.volume == -1) std::memset(buffer.data(), 0, (size_t)n);
            else if (range.volume == -2) std::memcpy(buffer.data(), synthetic.data() + range.offset + within, (size_t)n);
            else
            {
                auto* s = streamOf(range.volume);
                if (s == nullptr) return -1;
                if (!s->setPosition(range.offset + within) || s->read(buffer.data(), (int)n) != (int)n) { error = "read error in " + index.volumes[(size_t)range.volume].getFileName(); return -1; }
            }
            position += n;
            *out = buffer.data();
            return (la_ssize_t)n;
        }
        // The same stream for unarr (0.8.217): exactly 'count' bytes into the caller's
        // buffer. unarr reads in small pieces, so the bytes come through a 1 MB window
        // (a seek and a read of the file per piece made it 6 MB/s; libarchive asks for
        // a megabyte at a time).
        juce::int64 windowStart = -1, windowLength = 0;
        size_t readInto(void* dest, size_t count)
        {
            size_t done = 0;
            while (done < count && position < total)
            {
                if (position < windowStart || position >= windowStart + windowLength)
                {
                    // Fill the window from here: the ranges in a row, up to a megabyte.
                    windowStart = position;
                    windowLength = 0;
                    juce::int64 at = position;
                    while (windowLength < (juce::int64)buffer.size() && at < total)
                    {
                        const size_t r = (size_t)(std::upper_bound(starts.begin(), starts.end(), at) - starts.begin()) - 1;
                        const auto& range = ranges[r];
                        const juce::int64 within = at - starts[r];
                        const juce::int64 n = std::min<juce::int64>((juce::int64)buffer.size() - windowLength, range.length - within);
                        char* d = buffer.data() + windowLength;
                        if (range.volume == -1) std::memset(d, 0, (size_t)n);
                        else if (range.volume == -2) std::memcpy(d, synthetic.data() + range.offset + within, (size_t)n);
                        else
                        {
                            auto* s = streamOf(range.volume);
                            if (s == nullptr) break;
                            if (!s->setPosition(range.offset + within) || s->read(d, (int)n) != (int)n) { error = "read error in " + index.volumes[(size_t)range.volume].getFileName(); break; }
                        }
                        windowLength += n;
                        at += n;
                    }
                    if (windowLength == 0) break;
                }
                const juce::int64 n = std::min<juce::int64>((juce::int64)(count - done), windowStart + windowLength - position);
                std::memcpy(static_cast<char*>(dest) + done, buffer.data() + (position - windowStart), (size_t)n);
                position += n;
                done += (size_t)n;
            }
            return done;
        }
        la_int64_t seek(la_int64_t offset, int whence)
        {
            juce::int64 target = whence == SEEK_SET ? offset : whence == SEEK_CUR ? position + offset : total + offset;
            if (target < 0 || target > total) return ARCHIVE_FATAL;
            position = target;
            return position;
        }
        la_int64_t skip(la_int64_t request)
        {
            const juce::int64 n = std::max<juce::int64>(0, std::min<juce::int64>(request, total - position));
            position += n;
            return n;
        }
    };

    la_ssize_t virtualRead(struct archive*, void* data, const void** out) { return static_cast<VirtualStream*>(data)->read(out); }
    la_int64_t virtualSeek(struct archive*, void* data, la_int64_t offset, int whence) { return static_cast<VirtualStream*>(data)->seek(offset, whence); }
    la_int64_t virtualSkip(struct archive*, void* data, la_int64_t request) { return static_cast<VirtualStream*>(data)->skip(request); }

   #if ORGAN_HAVE_UNARR
    // ---- unarr (0.8.217): the RAR 4 files libarchive cannot unpack ----------------------------
    // libarchive 3.8.9 fails on two kinds of RAR 4 data: a PPMd block (what RAR makes of
    // text - an organ definition packed with the "best" method: "File CRC error" or
    // nothing at all, however it is fed) and the RAR VM filters of the WinRAR versions
    // of 2011 ("Internal error extracting RAR file" on the Dom Bedos set's WAVs). unarr
    // (LGPL-3.0, C, every platform; Masterpiece reads such archives with it too) has both.
    // libarchive stays first; after one failure an archive is read with unarr from then on.
    std::mutex gUnarrMutex;
    std::set<juce::String> gUnarrFirst;   // lower-case first-volume paths

    bool unarrFirst(const ArchiveIndex& index)
    {
        static const bool forced = std::getenv("ORGAN_UNARR_FIRST") != nullptr;   // tests: every archive through unarr
        if (forced) return true;
        std::lock_guard<std::mutex> lock(gUnarrMutex);
        return gUnarrFirst.count(lower(index.first.getFullPathName())) != 0;
    }
    void preferUnarr(const ArchiveIndex& index, const juce::String& why)
    {
        std::lock_guard<std::mutex> lock(gUnarrMutex);
        if (gUnarrFirst.insert(lower(index.first.getFullPathName())).second)
            CrashHandler::note("Archive " + index.first.getFileName() + ": read with unarr from now on (libarchive: " + why + ")");
    }

    void unarrClose(void*) {}
    size_t unarrRead(void* data, void* buffer, size_t count) { return static_cast<VirtualStream*>(data)->readInto(buffer, count); }
    bool unarrSeek(void* data, off64_t offset, int origin) { return static_cast<VirtualStream*>(data)->seek(offset, origin) >= 0; }
    off64_t unarrTell(void* data) { return static_cast<VirtualStream*>(data)->position; }

    uint32_t crc32Of(const uint8_t* bytes, size_t length)
    {
        static uint32_t table[256];
        static bool made = false;
        if (!made)
        {
            for (uint32_t i = 0; i < 256; ++i)
            {
                uint32_t c = i;
                for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
                table[i] = c;
            }
            made = true;
        }
        uint32_t crc = 0xFFFFFFFFu;
        for (size_t i = 0; i < length; ++i) crc = table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
        return crc ^ 0xFFFFFFFFu;
    }

    // One file read through unarr: the same bytes libarchive gets ([signature + main
    // block][file header][data]). A file over several volumes: unarr knows no volumes,
    // so the first part's header is rewritten as the header of a whole file (the split
    // flags cleared, the packed size the sum of the parts, the header CRC anew) and the
    // parts' data follow in a row - what Masterpiece does as well.
    std::shared_ptr<juce::MemoryBlock> readEntryWithUnarr(const ArchiveIndex& index, const ArchiveIndex::Entry& entry, juce::String& error)
    {
        if (entry.pieces.empty()) { error = "no data"; return nullptr; }
        if (entry.encrypted) { error = "encrypted"; return nullptr; }
        if (index.solid) { error = "solid archive"; return nullptr; }
        VirtualStream stream(index);
        const auto& first = entry.pieces.front();
        const auto& info = index.volumeInfo[(size_t)first.volume];
        stream.add(first.volume, 0, info.mainOffset + info.mainLength);
        if (entry.pieces.size() == 1) stream.add(first.volume, first.headerOffset, first.headerLength);
        else
        {
            juce::FileInputStream in(index.volumes[(size_t)first.volume]);
            std::vector<char> header((size_t)first.headerLength);
            if (header.size() < 32 || !in.openedOk() || !in.setPosition(first.headerOffset) || in.read(header.data(), (int)header.size()) != (int)header.size())
            { error = "the file header could not be read"; return nullptr; }
            // The FILE_CRC of a part's header is the CRC of that part's unpacked bytes; the
            // whole file's CRC stands in the LAST part's header (unarr checks it at the end).
            const auto& last = entry.pieces.back();
            juce::FileInputStream lastIn(index.volumes[(size_t)last.volume]);
            uint8_t lastCrc[4];
            if (last.headerLength < 20 || !lastIn.openedOk() || !lastIn.setPosition(last.headerOffset + 16) || lastIn.read(lastCrc, 4) != 4)
            { error = "the last part's header could not be read"; return nullptr; }
            juce::int64 packed = 0;
            for (const auto& piece : entry.pieces) packed += piece.dataLength;
            auto* h = reinterpret_cast<uint8_t*>(header.data());
            const uint16_t flags = (uint16_t)((h[3] | (h[4] << 8)) & ~0x0003);   // LHD_SPLIT_BEFORE | LHD_SPLIT_AFTER cleared
            h[3] = (uint8_t)flags; h[4] = (uint8_t)(flags >> 8);
            std::memcpy(h + 16, lastCrc, 4);                                     // FILE_CRC of the whole file
            auto write32 = [](uint8_t* at, uint32_t v) { at[0] = (uint8_t)v; at[1] = (uint8_t)(v >> 8); at[2] = (uint8_t)(v >> 16); at[3] = (uint8_t)(v >> 24); };
            write32(h + 7, (uint32_t)packed);                                        // PACK_SIZE
            if ((flags & 0x0100) != 0) write32(h + 32, (uint32_t)(packed >> 32));  // LHD_LARGE: HIGH_PACK_SIZE
            const uint32_t crc = crc32Of(h + 2, header.size() - 2);
            h[0] = (uint8_t)crc; h[1] = (uint8_t)(crc >> 8);
            stream.synthetic = header;
            stream.add(-2, 0, (juce::int64)header.size());
        }
        for (const auto& piece : entry.pieces) stream.add(piece.volume, piece.dataOffset, piece.dataLength);
        stream.add(-1, 0, 4096);
        ar_stream* s = ar_open_stream(&stream, unarrClose, unarrRead, unarrSeek, unarrTell);
        if (s == nullptr) { error = "unarr: no stream"; return nullptr; }
        std::shared_ptr<juce::MemoryBlock> block;
        ar_archive* ar = ar_open_rar_archive(s);
        if (ar == nullptr) error = "unarr did not open the archive";
        else if (!ar_parse_entry(ar)) error = "unarr found no file entry";
        else
        {
            const size_t size = ar_entry_get_size(ar);
            if ((juce::int64)size != entry.size) error = "unarr sees " + juce::String((juce::int64)size) + " bytes, the index " + juce::String(entry.size);
            else
            {
                block = std::make_shared<juce::MemoryBlock>(size, false);
                if (size > 0 && !ar_entry_uncompress(ar, block->getData(), size)) { error = "unarr could not unpack the file"; block.reset(); }
            }
        }
        if (ar != nullptr) ar_close_archive(ar);
        ar_close(s);
        if (block == nullptr && stream.error.isNotEmpty()) error = stream.error;
        return block;
    }

    // A file of a solid archive through unarr: the real volume, the entry by its header
    // offset (unarr rebuilds the solid state from the first entry when it has to).
    std::shared_ptr<juce::MemoryBlock> readSolidEntryWithUnarr(const ArchiveIndex& index, const ArchiveIndex::Entry& entry, juce::String& error)
    {
        if (entry.pieces.empty()) { error = "no data"; return nullptr; }
        if (index.volumes.size() != 1 || entry.pieces.size() != 1) { error = "a solid archive in several volumes"; return nullptr; }
       #if JUCE_WINDOWS
        ar_stream* s = ar_open_file_w(index.volumes.front().getFullPathName().toWideCharPointer());
       #else
        ar_stream* s = ar_open_file(index.volumes.front().getFullPathName().toRawUTF8());
       #endif
        if (s == nullptr) { error = "the archive could not be opened"; return nullptr; }
        std::shared_ptr<juce::MemoryBlock> block;
        ar_archive* ar = ar_open_rar_archive(s);
        if (ar == nullptr) error = "unarr did not open the archive";
        else if (!ar_parse_entry_at(ar, (off64_t)entry.pieces.front().headerOffset)) error = "unarr found no file entry at the offset";
        else
        {
            const size_t size = ar_entry_get_size(ar);
            if ((juce::int64)size != entry.size) error = "unarr sees " + juce::String((juce::int64)size) + " bytes, the index " + juce::String(entry.size);
            else
            {
                block = std::make_shared<juce::MemoryBlock>(size, false);
                if (size > 0 && !ar_entry_uncompress(ar, block->getData(), size)) { error = "unarr could not unpack the file"; block.reset(); }
            }
        }
        if (ar != nullptr) ar_close_archive(ar);
        ar_close(s);
        return block;
    }
   #endif

    // The file read on its own. Stored data comes straight from the volumes; packed
    // data goes through libarchive fed with the file's bytes alone. nullptr (with
    // 'error') when it cannot: a solid archive (read in sequence instead), encryption.
    std::shared_ptr<juce::MemoryBlock> readEntryAlone(const ArchiveIndex& index, const ArchiveIndex::Entry& entry, juce::String& error)
    {
        if (entry.pieces.empty()) { error = "no data"; return nullptr; }
        if (entry.encrypted) { error = "encrypted"; return nullptr; }
        if (entry.stored)
        {
            auto block = std::make_shared<juce::MemoryBlock>();
            juce::int64 total = 0;
            for (const auto& piece : entry.pieces) total += piece.dataLength;
            block->setSize((size_t)total, false);
            juce::int64 at = 0;
            std::map<int, std::unique_ptr<juce::FileInputStream>> streams;
            for (const auto& piece : entry.pieces)
            {
                auto& s = streams[piece.volume];
                if (s == nullptr) s = std::make_unique<juce::FileInputStream>(index.volumes[(size_t)piece.volume]);
                if (!s->openedOk() || !s->setPosition(piece.dataOffset)) { error = index.volumes[(size_t)piece.volume].getFileName() + " cannot be read"; return nullptr; }
                juce::int64 left = piece.dataLength;
                while (left > 0)
                {
                    const int chunk = (int)std::min<juce::int64>(left, 64 << 20);
                    if (s->read(static_cast<char*>(block->getData()) + at, chunk) != chunk) { error = "read error in " + index.volumes[(size_t)piece.volume].getFileName(); return nullptr; }
                    at += chunk;
                    left -= chunk;
                }
            }
            return block;
        }
        if (index.solid) { error = "solid"; return nullptr; }
        VirtualStream stream(index);
        for (size_t i = 0; i < entry.pieces.size(); ++i)
        {
            const auto& piece = entry.pieces[i];
            const auto& info = index.volumeInfo[(size_t)piece.volume];
            if (i > 0)
            {
                const auto& previous = index.volumeInfo[(size_t)entry.pieces[i - 1].volume];
                stream.add(entry.pieces[i - 1].volume, previous.endOffset, previous.endLength);
            }
            stream.add(piece.volume, 0, info.mainOffset + info.mainLength);   // the signature and the main block
            stream.add(piece.volume, piece.headerOffset, piece.headerLength);
            stream.add(piece.volume, piece.dataOffset, piece.dataLength);
        }
        stream.add(-1, 0, 4096);   // zeros: the RAR 5 reader looks 512 bytes ahead after an end block
        struct archive* a = archive_read_new();
        if (a == nullptr) { error = "libarchive could not be started"; return nullptr; }
        archive_read_support_format_rar(a);
        archive_read_support_format_rar5(a);
        archive_read_support_filter_none(a);
        archive_read_set_callback_data(a, &stream);
        archive_read_set_read_callback(a, virtualRead);
        archive_read_set_seek_callback(a, virtualSeek);
        archive_read_set_skip_callback(a, virtualSkip);
        std::shared_ptr<juce::MemoryBlock> block;
        if (archive_read_open1(a) != ARCHIVE_OK) error = archiveError(a, "the archive could not be opened");
        else
        {
            struct archive_entry* opened = nullptr;
            int header = archive_read_next_header(a, &opened);
            while (header == ARCHIVE_RETRY) header = archive_read_next_header(a, &opened);
            if (header != ARCHIVE_OK || opened == nullptr) error = archiveError(a, "the file's header could not be read");
            else
            {
                block = readOpenEntry(a, entry.size);
                if (block == nullptr) error = archiveError(a, "the file's data could not be read");
            }
        }
        if (stream.error.isNotEmpty()) error = stream.error;
        archive_read_close(a);
        archive_read_free(a);
        if (block != nullptr && error.isNotEmpty()) block.reset();
        return block;
    }

    // One reader per solid archive that stays where it stopped: a folder behind
    // the cursor reopens, a folder ahead is reached by skipping headers alone.
    struct Cursor
    {
        struct archive* reader = nullptr;
        juce::int64 position = 0;   // the order of the entry the next header read returns
        ~Cursor() { close(); }
        void close() { if (reader != nullptr) { archive_read_close(reader); archive_read_free(reader); reader = nullptr; } position = 0; }
    };
    std::map<juce::String, std::unique_ptr<Cursor>> gCursors;   // lower-case first-volume path

    // Reads the entries wanted (by their order) into blocks, in archive order (solid archives).
    void readEntriesInSequence(const ArchiveIndex& index, std::map<juce::int64, std::shared_ptr<juce::MemoryBlock>>& wanted)
    {
        if (wanted.empty()) return;
        std::lock_guard<std::mutex> lock(gMutex);
        auto& cursor = gCursors[lower(index.first.getFullPathName())];
        if (cursor == nullptr) cursor = std::make_unique<Cursor>();
        const juce::int64 firstWanted = wanted.begin()->first;
        if (cursor->reader != nullptr && cursor->position > firstWanted) cursor->close();
        if (cursor->reader == nullptr)
        {
            juce::String error;
            cursor->reader = openReader(index.volumes, error);
            cursor->position = 0;
            if (cursor->reader == nullptr) { CrashHandler::note("Archive " + index.first.getFileName() + ": " + error); return; }
        }
        const juce::int64 lastWanted = wanted.rbegin()->first;
        for (;;)
        {
            struct archive_entry* entry = nullptr;
            const int header = archive_read_next_header(cursor->reader, &entry);
            if (header == ARCHIVE_EOF) { cursor->close(); break; }
            if (header == ARCHIVE_RETRY) continue;
            if (header == ARCHIVE_FATAL || entry == nullptr)
            {
                CrashHandler::note("Archive " + index.first.getFileName() + ": " + archiveError(cursor->reader, "unreadable"));
                cursor->close();
                break;
            }
            const juce::int64 order = cursor->position++;
            auto want = wanted.find(order);
            if (want == wanted.end()) { archive_read_data_skip(cursor->reader); if (order >= lastWanted) break; continue; }
            want->second = readOpenEntry(cursor->reader, (juce::int64)archive_entry_size(entry));
            if (order >= lastWanted) break;
        }
    }

    // The order of entries in a solid archive counts every file and folder block
    // libarchive reports; our walk counts the same blocks (files, and RAR 5 folder
    // entries) - the same order.
    std::shared_ptr<juce::MemoryBlock> readEntry(const ArchiveIndex& index, const ArchiveIndex::Entry& entry)
    {
        juce::String error;
        if (!index.solid || entry.stored)
        {
            std::shared_ptr<juce::MemoryBlock> block;
           #if ORGAN_HAVE_UNARR
            if (!entry.stored && unarrFirst(index))
            {
                block = readEntryWithUnarr(index, entry, error);
                if (block == nullptr) CrashHandler::note("Archive " + index.first.getFileName() + ": " + entry.path + " could not be read by unarr - " + error);
                return block;
            }
           #endif
            block = readEntryAlone(index, entry, error);
           #if ORGAN_HAVE_UNARR
            if (block == nullptr && !entry.stored && !entry.encrypted)
            {
                preferUnarr(index, error);
                juce::String why;
                block = readEntryWithUnarr(index, entry, why);
                if (block == nullptr) error += "; unarr: " + why;
            }
           #endif
            if (block == nullptr) CrashHandler::note("Archive " + index.first.getFileName() + ": " + entry.path + " could not be read - " + error);
            return block;
        }
        std::map<juce::int64, std::shared_ptr<juce::MemoryBlock>> wanted;
        wanted[entry.order] = nullptr;
       #if ORGAN_HAVE_UNARR
        if (!unarrFirst(index))
       #endif
            readEntriesInSequence(index, wanted);
        auto block = wanted.begin()->second;
       #if ORGAN_HAVE_UNARR
        if (block == nullptr && !entry.encrypted)
        {
            preferUnarr(index, "the solid stream");
            block = readSolidEntryWithUnarr(index, entry, error);
            if (block == nullptr) CrashHandler::note("Archive " + index.first.getFileName() + ": " + entry.path + " could not be read by unarr - " + error);
            return block;
        }
       #endif
        if (block == nullptr) CrashHandler::note("Archive " + index.first.getFileName() + ": " + entry.path + " could not be read (solid archive)");
        return block;
    }

    bool nameMatches(const juce::String& name, const juce::StringArray& extensions)
    {
        if (extensions.isEmpty()) return true;
        for (const auto& ext : extensions) if (name.endsWithIgnoreCase(ext)) return true;
        return false;
    }
}

namespace SampleFiles
{
    void setIndexFolder(const juce::File& folder) { std::lock_guard<std::mutex> lock(gMutex); gIndexFolder = folder; }

    void setSearchFolders(const std::vector<juce::File>& folders)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        gSearchFolders.clear();
        for (const auto& folder : folders)
            if (folder != juce::File() && std::find(gSearchFolders.begin(), gSearchFolders.end(), folder) == gSearchFolders.end()) gSearchFolders.push_back(folder);
        gArchivesOfFolder.clear();   // the folders' serving archives are listed again
    }

    void setChosenArchives(const std::vector<juce::File>& archives)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        gChosenArchives.clear();
        for (const auto& archive : firstVolumes(archives))
            if (archive.existsAsFile()) gChosenArchives.push_back(archive);
        gArchivesOfFolder.clear();
    }

    void addChosenArchives(const std::vector<juce::File>& archives)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        for (const auto& archive : firstVolumes(archives))
            if (archive.existsAsFile() && std::find(gChosenArchives.begin(), gChosenArchives.end(), archive) == gChosenArchives.end()) gChosenArchives.push_back(archive);
        gArchivesOfFolder.clear();
    }

    juce::File archiveOf(const juce::File& path)
    {
        const auto place = locate(path);
        return place.onDisk || place.index == nullptr ? juce::File() : place.index->first;
    }

    void forgetArchives()
    {
        std::lock_guard<std::mutex> lock(gMutex);
        gCursors.clear();
        gIndexes.clear();
        gArchivesOfFolder.clear();
    }

    bool isFirstVolume(const juce::File& archive)
    {
        if (!archive.existsAsFile() || !archive.hasFileExtension("rar")) return false;
        const int part = partNumberOf(archive);
        return part < 0 || part <= 1;
    }

    juce::File firstVolumeOf(const juce::File& file)
    {
        // name.part03.rar -> name.part01.rar (the lowest-numbered part beside it); anything else as it is.
        const int part = partNumberOf(file);
        if (part < 0 || part <= 1) return file;
        const auto stem = file.getFileName().toLowerCase().dropLastCharacters(4);
        const auto prefix = stem.substring(0, stem.lastIndexOf(".part") + 5);
        juce::File best = file;
        int bestNumber = part;
        for (const auto& entry : juce::RangedDirectoryIterator(file.getParentDirectory(), false, "*.rar", juce::File::findFiles))
        {
            const auto candidate = entry.getFile();
            if (!candidate.getFileName().toLowerCase().startsWith(prefix)) continue;
            const int number = partNumberOf(candidate);
            if (number >= 0 && number < bestNumber) { bestNumber = number; best = candidate; }
        }
        return best;
    }

    std::vector<juce::File> firstVolumes(const std::vector<juce::File>& archives)
    {
        // The chosen files as sets: every part replaced by its set's first part, each set once, the order kept.
        std::vector<juce::File> result;
        for (const auto& archive : archives)
        {
            const auto first = firstVolumeOf(archive);
            if (std::find(result.begin(), result.end(), first) == result.end()) result.push_back(first);
        }
        return result;
    }

    std::vector<juce::File> volumesRead(const juce::File& archive)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        auto index = indexOfLocked(archive);
        return index != nullptr ? index->volumes : std::vector<juce::File>{};
    }

    std::vector<juce::File> volumesOf(const juce::File& first)
    {
        std::vector<juce::File> volumes{ first };
        const auto folder = first.getParentDirectory();
        const int part = partNumberOf(first);
        if (part >= 0)
        {
            // name.part1.rar, name.part2.rar ... (the digits may be padded: part01, part02)
            const auto stem = first.getFileName().toLowerCase().dropLastCharacters(4);
            const auto prefix = stem.substring(0, stem.lastIndexOf(".part") + 5);
            std::vector<std::pair<int, juce::File>> others;
            for (const auto& entry : juce::RangedDirectoryIterator(folder, false, "*.rar", juce::File::findFiles))
            {
                const auto file = entry.getFile();
                const auto name = file.getFileName().toLowerCase();
                if (file == first || !name.startsWith(prefix)) continue;
                const int number = partNumberOf(file);
                if (number > part) others.push_back({ number, file });
            }
            std::sort(others.begin(), others.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            for (const auto& o : others) volumes.push_back(o.second);
            return volumes;
        }
        // name.rar + name.r00, name.r01 ... (then name.s00 ...)
        const auto stem = first.getFileNameWithoutExtension();
        std::vector<juce::File> others;
        for (const auto& entry : juce::RangedDirectoryIterator(folder, false, stem + ".*", juce::File::findFiles))
            if (isOldVolume(entry.getFile()) && entry.getFile().getFileNameWithoutExtension().equalsIgnoreCase(stem)) others.push_back(entry.getFile());
        std::sort(others.begin(), others.end(), [](const juce::File& a, const juce::File& b) { return a.getFileExtension().compareIgnoreCase(b.getFileExtension()) < 0; });
        for (const auto& o : others) volumes.push_back(o);
        return volumes;
    }

    std::vector<juce::File> archivesIn(const juce::File& folder)
    {
        if (!folder.isDirectory()) return {};
        std::lock_guard<std::mutex> lock(gMutex);
        return archivesOfFolderLocked(folder);
    }

    std::vector<juce::String> topLevelFoldersOf(const juce::File& archive)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        auto index = indexOfLocked(archive);
        std::vector<juce::String> result;
        if (index == nullptr || !index->valid) return result;
        for (const auto& key : index->childDirs[juce::String()]) result.push_back(index->spelling[key]);
        return result;
    }

    std::vector<juce::String> entriesOf(const juce::File& archive, const std::function<bool(const juce::String&)>& wanted)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        auto index = indexOfLocked(archive);
        std::vector<juce::String> result;
        if (index == nullptr || !index->valid) return result;
        for (const auto& entry : index->entries) if (wanted(entry.path)) result.push_back(entry.path);
        return result;
    }

    void indexArchives(const std::vector<juce::File>& archives)
    {
        for (const auto& archive : archives)
        {
            std::lock_guard<std::mutex> lock(gMutex);
            indexOfLocked(archive);
        }
    }

    juce::String indexingProgress()
    {
        const int files = gIndexingFiles.load(std::memory_order_relaxed);
        if (files < 0) return {};
        const int volume = gIndexingVolume.load(std::memory_order_relaxed), volumes = gIndexingVolumes.load(std::memory_order_relaxed);
        return "Reading the contents of " + gIndexingName + (volumes > 1 ? " (part " + juce::String(volume) + " of " + juce::String(volumes) + ")" : juce::String())
            + ": " + juce::String(files) + " file(s) so far";
    }

    juce::String archiveSummary(const juce::File& archive)
    {
        std::lock_guard<std::mutex> lock(gMutex);
        auto index = indexOfLocked(archive);
        if (index == nullptr) return "no index";
        juce::int64 bytes = 0;
        for (const auto& entry : index->entries) bytes += entry.size;
        return (index->valid ? juce::String("RAR ") + juce::String(index->format) : juce::String("invalid")) + (index->solid ? " solid" : "") + ", "
            + juce::String((int)index->entries.size()) + " file(s), " + juce::String(bytes >> 20) + " MB" + (index->error.isNotEmpty() ? " - " + index->error : juce::String());
    }

    bool isDirectory(const juce::File& path) { return locate(path).isDir; }
    bool existsAsFile(const juce::File& path) { return locate(path).isFile; }
    bool inArchive(const juce::File& path) { const auto p = locate(path); return !p.onDisk && p.index != nullptr; }
    juce::String archiveNameOf(const juce::File& path) { const auto p = locate(path); return p.onDisk || p.index == nullptr ? juce::String() : p.index->first.getFileName(); }

    std::vector<juce::File> childDirectories(const juce::File& dir)
    {
        std::vector<juce::File> result;
        const auto place = locate(dir);
        if (place.onDisk)
        {
            for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*", juce::File::findDirectories)) result.push_back(entry.getFile());
            // ... and the folders the archives of this folder hold at their top level,
            // or under this folder's path inside the archives of an ancestor.
            std::set<juce::String> seen;
            for (const auto& f : result) seen.insert(lower(f.getFileName()));
            juce::File ancestor = dir;
            juce::String rest;
            for (int depth = 0; depth < 12 && ancestor != juce::File(); ++depth)
            {
                if (ancestor.isDirectory())
                {
                    std::lock_guard<std::mutex> lock(gMutex);
                    for (const auto& archive : archivesOfFolderLocked(ancestor))
                    {
                        auto index = indexOfLocked(archive);
                        if (index == nullptr || !index->valid) continue;
                        const auto found = index->childDirs.find(lower(rest));
                        if (found == index->childDirs.end()) continue;
                        for (const auto& key : found->second)
                        {
                            const auto leaf = key.contains("/") ? key.fromLastOccurrenceOf("/", false, false) : key;
                            if (seen.count(leaf) != 0) continue;
                            seen.insert(leaf);
                            result.push_back(dir.getChildFile(index->spelling[key].fromLastOccurrenceOf("/", false, false)));
                        }
                    }
                }
                const auto parent = ancestor.getParentDirectory();
                if (parent == ancestor) break;
                rest = rest.isEmpty() ? ancestor.getFileName() : ancestor.getFileName() + "/" + rest;
                ancestor = parent;
            }
            return result;
        }
        if (place.index == nullptr || !place.isDir) return result;
        const auto found = place.index->childDirs.find(lower(place.relative));
        if (found == place.index->childDirs.end()) return result;
        for (const auto& key : found->second) result.push_back(dir.getChildFile(place.index->spelling[key].fromLastOccurrenceOf("/", false, false)));
        return result;
    }

    std::vector<juce::String> fileNames(const juce::File& dir)
    {
        std::vector<juce::String> names;
        const auto place = locate(dir);
        if (place.onDisk)
        {
            for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*", juce::File::findFiles)) names.push_back(entry.getFile().getFileName());
            return names;
        }
        if (place.index == nullptr || !place.isDir) return names;
        const auto found = place.index->children.find(lower(place.relative));
        if (found == place.index->children.end()) return names;
        for (const auto i : found->second) names.push_back(place.index->entries[i].path.fromLastOccurrenceOf("/", false, false));
        return names;
    }

    std::vector<juce::File> subdirectoriesRecursive(const juce::File& top)
    {
        std::vector<juce::File> result;
        std::vector<juce::File> queue{ top };
        while (!queue.empty())
        {
            const auto dir = queue.front();
            queue.erase(queue.begin());
            for (const auto& child : childDirectories(dir)) { result.push_back(child); queue.push_back(child); }
        }
        return result;
    }

    juce::int64 totalBytes(const juce::File& dir, const juce::StringArray& extensions, bool recursive)
    {
        juce::int64 bytes = 0;
        const auto place = locate(dir);
        if (place.onDisk)
        {
            for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*", juce::File::findFiles))
                if (nameMatches(entry.getFile().getFileName(), extensions)) bytes += entry.getFileSize();
        }
        else if (place.index != nullptr && place.isDir)
        {
            const auto found = place.index->children.find(lower(place.relative));
            if (found != place.index->children.end())
                for (const auto i : found->second) if (nameMatches(place.index->entries[i].path, extensions)) bytes += place.index->entries[i].size;
        }
        if (recursive) for (const auto& child : childDirectories(dir)) bytes += totalBytes(child, extensions, true);
        return bytes;
    }

    std::vector<FileData> readFolderFiles(const juce::File& dir, const juce::StringArray& extensions)
    {
        std::vector<FileData> result;
        const auto place = locate(dir);
        if (place.onDisk)
        {
            for (const auto& name : fileNames(dir)) if (nameMatches(name, extensions)) result.push_back({ name, nullptr, false });
            return result;
        }
        if (place.index == nullptr || !place.isDir) return result;
        const auto found = place.index->children.find(lower(place.relative));
        if (found == place.index->children.end()) return result;
        if (!place.index->solid)
        {
            // Every file on its own: the caller reads them with readFile, in parallel.
            for (const auto i : found->second)
            {
                const auto name = place.index->entries[i].path.fromLastOccurrenceOf("/", false, false);
                if (nameMatches(name, extensions)) result.push_back({ name, nullptr, true });
            }
            return result;
        }
        std::map<juce::int64, std::shared_ptr<juce::MemoryBlock>> wanted;
        std::vector<std::pair<juce::String, juce::int64>> names;
        for (const auto i : found->second)
        {
            const auto& entry = place.index->entries[i];
            const auto name = entry.path.fromLastOccurrenceOf("/", false, false);
            if (!nameMatches(name, extensions)) continue;
            wanted[entry.order] = nullptr;
            names.push_back({ name, entry.order });
        }
        readEntriesInSequence(*place.index, wanted);
        for (const auto& [name, order] : names)
        {
            auto block = wanted[order];
            if (block == nullptr) { CrashHandler::note("Archive " + place.index->first.getFileName() + ": " + place.relative + "/" + name + " could not be read"); continue; }
            result.push_back({ name, block, true });
        }
        return result;
    }

    std::shared_ptr<juce::MemoryBlock> readFile(const juce::File& file)
    {
        const auto place = locate(file);
        if (place.onDisk || place.index == nullptr || !place.isFile) return nullptr;
        const auto found = place.index->byPath.find(lower(place.relative));
        if (found == place.index->byPath.end()) return nullptr;
        return readEntry(*place.index, place.index->entries[found->second]);
    }

    std::unique_ptr<juce::InputStream> openInput(const juce::File& file)
    {
        const auto place = locate(file);
        if (place.onDisk) return std::unique_ptr<juce::InputStream>(file.createInputStream());
        auto block = readFile(file);
        if (block == nullptr) return nullptr;
        struct BlockStream final : public juce::MemoryInputStream
        {
            BlockStream(std::shared_ptr<juce::MemoryBlock> b) : juce::MemoryInputStream(b->getData(), b->getSize(), false), keep(std::move(b)) {}
            std::shared_ptr<juce::MemoryBlock> keep;
        };
        return std::make_unique<BlockStream>(block);
    }

    juce::int64 archiveOrderOf(const juce::File& dir)
    {
        const auto place = locate(dir);
        if (place.onDisk || place.index == nullptr || !place.isDir) return -1;
        const auto found = place.index->children.find(lower(place.relative));
        if (found == place.index->children.end() || found->second.empty()) return -1;
        juce::int64 first = -1;
        for (const auto i : found->second) { const auto order = place.index->entries[i].order; if (first < 0 || order < first) first = order; }
        return first;
    }
}
