#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace aether {

    // VFS: Decouples asset requests from physical storage.
    // Supports "Mounting" directories or (in the future) archives to virtual paths.
    class VFS
    {
    public:
        // Mounts a physical disk path to a virtual path.
        // Example: Mount("/assets", "C:/Dev/Aether/Content");
        static void Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath);

        // Unmounts a virtual path (useful for mod loading/unloading).
        static void Unmount(const std::string& virtualPath);

        // Converts a Virtual Path to a Physical Path if the file exists.
        // Returns false if the file cannot be found in any mount point.
        static bool Resolve(const std::string& virtualPath, std::filesystem::path& outPhysicalPath);

        // Reads a text file fully into a string. Returns empty on failure.
        static std::string ReadText(const std::string& virtualPath);

        // Reads a binary file fully into a buffer. Returns empty vector on failure.
        static std::vector<unsigned char> ReadBytes(const std::string& virtualPath);

        // Writes text to a file. Attempts to map virtual path to the first matching mount.
        static bool WriteText(const std::string& virtualPath, const std::string& text);

        // Writes binary data to a file.
        static bool WriteBytes(const std::string& virtualPath, const std::vector<unsigned char>& data);

    private:
        struct MountPoint
        {
            std::string VirtualRoot;
            std::filesystem::path PhysicalRoot;
        };

        // Static list of mount points. Ordered by priority (LIFO or FIFO depends on impl, usually FIFO).
        static std::vector<MountPoint> s_MountPoints;
    };

}