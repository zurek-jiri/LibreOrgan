#pragma once

#include <JuceHeader.h>
#include <map>
#include <mutex>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <winioctl.h>
#elif JUCE_LINUX
 #include <sys/stat.h>
 #include <sys/statfs.h>
 #include <sys/sysmacros.h>
 #include <limits.h>
 #include <stdlib.h>
#endif

//==============================================================================
// 0.9.140 (the user: "Měl by zvážit typ disku a podle toho si to seřadit!!! Musí vyhrávat rychlé disky!"): the
// KIND of disk a path lies on, so that among the places the program finds by itself the fastest one is tried
// first. Measured on his computer (one organ, 66 GB of WAV, cold): NVMe 51 s, SATA SSD 163 s.
//   3 = NVMe, 2 = another SSD or a disk whose kind cannot be told, 1 = a hard disk, 0 = a network folder.
// Nothing is read from the disk for it: the system is asked about the device (Linux: /sys; Windows: the storage
// properties of the volume); every failure answers 2, the neutral class. Answers are kept per device.
namespace DiskInfo
{
    inline const char* className(int speedClass) noexcept
    {
        return speedClass >= 3 ? "NVMe" : speedClass == 2 ? "SSD" : speedClass == 1 ? "hard disk" : "network folder";
    }

   #if JUCE_LINUX
    inline int classOfDevice(dev_t device, const juce::String& anExistingPath)
    {
        struct statfs fs;
        if (::statfs(anExistingPath.toRawUTF8(), &fs) == 0)
        {
            const auto type = (unsigned long)fs.f_type;
            if (type == 0x6969ul || type == 0xFF534D42ul || type == 0xFE534D42ul || type == 0x517Bul || type == 0x01021997ul) return 0;   // NFS, CIFS, SMB2, SMB, 9p
        }
        char resolved[PATH_MAX];
        const auto link = "/sys/dev/block/" + juce::String((int)major(device)) + ":" + juce::String((int)minor(device));
        if (::realpath(link.toRawUTF8(), resolved) == nullptr) return 2;
        juce::File node(juce::String::fromUTF8(resolved));
        if (!node.getChildFile("queue").isDirectory()) node = node.getParentDirectory();   // a partition: the disk is the folder above
        if (!node.getChildFile("queue").isDirectory()) return 2;
        if (node.getFileName().startsWith("nvme")) return 3;
        return node.getChildFile("queue").getChildFile("rotational").loadFileAsString().trim() == "1" ? 1 : 2;
    }
   #endif

    // The class of the disk the path lies on (the nearest folder above it that exists decides).
    inline int speedClassOf(const juce::File& path)
    {
        static std::mutex lock;
       #if JUCE_LINUX
        static std::map<unsigned long long, int> known;
        juce::File existing = path;
        struct stat st;
        for (int up = 0; up < 64; ++up)
        {
            if (::stat(existing.getFullPathName().toRawUTF8(), &st) == 0)
            {
                const std::lock_guard<std::mutex> guard(lock);
                const auto key = (unsigned long long)st.st_dev;
                if (const auto it = known.find(key); it != known.end()) return it->second;
                return known[key] = classOfDevice(st.st_dev, existing.getFullPathName());
            }
            const auto parent = existing.getParentDirectory();
            if (parent == existing) break;
            existing = parent;
        }
        return 2;
       #elif JUCE_WINDOWS
        static std::map<juce::String, int> known;
        const auto full = path.getFullPathName();
        if (full.length() < 2 || full[1] != ':') return full.startsWith("\\\\") ? 0 : 2;
        const auto letter = full.substring(0, 1).toUpperCase();
        const std::lock_guard<std::mutex> guard(lock);
        if (const auto it = known.find(letter); it != known.end()) return it->second;
        int result = 2;
        const auto root = letter + ":\\";
        const UINT kind = GetDriveTypeW(root.toWideCharPointer());
        if (kind == DRIVE_REMOTE) result = 0;
        else if (kind == DRIVE_FIXED || kind == DRIVE_REMOVABLE)
        {
            const auto volume = "\\\\.\\" + letter + ":";
            HANDLE handle = CreateFileW(volume.toWideCharPointer(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
            if (handle != INVALID_HANDLE_VALUE)
            {
                STORAGE_PROPERTY_QUERY query{};
                query.PropertyId = StorageDeviceProperty;
                query.QueryType = PropertyStandardQuery;
                BYTE buffer[1024] = {};
                DWORD got = 0;
                bool nvme = false;
                if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer, sizeof(buffer), &got, nullptr) && got >= sizeof(STORAGE_DEVICE_DESCRIPTOR))
                    nvme = ((const STORAGE_DEVICE_DESCRIPTOR*)buffer)->BusType == BusTypeNvme;
                if (nvme) result = 3;
                else
                {
                    STORAGE_PROPERTY_QUERY seekQuery{};
                    seekQuery.PropertyId = StorageDeviceSeekPenaltyProperty;
                    seekQuery.QueryType = PropertyStandardQuery;
                    DEVICE_SEEK_PENALTY_DESCRIPTOR seek{};
                    if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &seekQuery, sizeof(seekQuery), &seek, sizeof(seek), &got, nullptr) && got >= sizeof(seek))
                        result = seek.IncursSeekPenalty ? 1 : 2;
                }
                CloseHandle(handle);
            }
        }
        return known[letter] = result;
       #else
        juce::ignoreUnused(path);
        return 2;   // macOS: not told apart (the order of the lines stays)
       #endif
    }
}
