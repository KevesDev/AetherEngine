#include "VFS.h"
#include "Log.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace aether {

    std::vector<VFS::MountPoint> VFS::s_MountPoints;

    void VFS::Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath)
    {
        // Auto-create directory if it's missing.
        // This ensures the "assets" folder exists immediately upon startup.
        if (!std::filesystem::exists(physicalPath))
        {
            try {
                std::filesystem::create_directories(physicalPath);
                AETHER_CORE_INFO("VFS: Auto-created missing directory '{0}'", physicalPath.string());
            }
            catch (const std::filesystem::filesystem_error& e) {
                // If we can't create the folder, we have a permission issue or bad path.
                AETHER_CORE_ERROR("VFS: Critical Failure - Could not create directory '{0}'. Error: {1}", physicalPath.string(), e.what());
                return;
            }
        }

        s_MountPoints.push_back({ virtualPath, physicalPath });
        AETHER_CORE_INFO("VFS: Mounted '{0}' -> '{1}'", virtualPath, physicalPath.string());
    }

    void VFS::Unmount(const std::string& virtualPath)
    {
        // Standard erase-remove idiom to remove the mount point by virtual path
        auto it = std::remove_if(s_MountPoints.begin(), s_MountPoints.end(),
            [&](const MountPoint& mp) { return mp.VirtualRoot == virtualPath; });

        if (it != s_MountPoints.end())
        {
            s_MountPoints.erase(it, s_MountPoints.end());
            AETHER_CORE_INFO("VFS: Unmounted '{0}'", virtualPath);
        }
    }

    bool VFS::Resolve(const std::string& virtualPath, std::filesystem::path& outPhysicalPath)
    {
        // Search all mount points for the file
        for (const auto& mp : s_MountPoints)
        {
            // Check if virtual path starts with this mount point's root
            if (virtualPath.find(mp.VirtualRoot) == 0)
            {
                // Strip the virtual root and replace with physical root
                std::string remainder = virtualPath.substr(mp.VirtualRoot.length());

                // Handle optional slash consistency (ensure we don't end up with // or missing /)
                if (remainder.find("/") == 0 || remainder.find("\\") == 0)
                    remainder = remainder.substr(1);

                std::filesystem::path candidate = mp.PhysicalRoot / remainder;

                if (std::filesystem::exists(candidate))
                {
                    outPhysicalPath = candidate;
                    return true;
                }
            }
        }
        return false;
    }

    std::string VFS::ReadText(const std::string& virtualPath)
    {
        std::filesystem::path physicalPath;
        if (!Resolve(virtualPath, physicalPath))
        {
            AETHER_CORE_ERROR("VFS: File not found: {0}", virtualPath);
            return "";
        }

        // Binary mode prevents CRLF issues on Windows vs Linux
        std::ifstream in(physicalPath, std::ios::in | std::ios::binary);
        if (in)
        {
            std::string result;
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
            return result;
        }

        AETHER_CORE_ERROR("VFS: Failed to read file: {0}", physicalPath.string());
        return "";
    }

    std::vector<unsigned char> VFS::ReadBytes(const std::string& virtualPath)
    {
        std::filesystem::path physicalPath;
        if (!Resolve(virtualPath, physicalPath))
        {
            AETHER_CORE_ERROR("VFS: File not found: {0}", virtualPath);
            return {};
        }

        std::ifstream in(physicalPath, std::ios::in | std::ios::binary);
        if (in)
        {
            // Stop eating new lines in binary mode (safety check)
            in.unsetf(std::ios::skipws);

            // Get file size
            in.seekg(0, std::ios::end);
            std::streampos fileSize = in.tellg();
            in.seekg(0, std::ios::beg);

            std::vector<unsigned char> data;
            data.reserve(fileSize);

            // Read the data
            data.insert(data.begin(),
                std::istream_iterator<unsigned char>(in),
                std::istream_iterator<unsigned char>());

            return data;
        }

        AETHER_CORE_ERROR("VFS: Failed to read binary file: {0}", physicalPath.string());
        return {};
    }

    bool VFS::WriteText(const std::string& virtualPath, const std::string& text)
    {
        // For writing, we can't check 'Resolve' because the file might not exist yet.
        // We find the first matching mount point and assume we want to write there.
        for (const auto& mp : s_MountPoints)
        {
            if (virtualPath.find(mp.VirtualRoot) == 0)
            {
                std::string remainder = virtualPath.substr(mp.VirtualRoot.length());
                if (remainder.find("/") == 0 || remainder.find("\\") == 0)
                    remainder = remainder.substr(1);

                std::filesystem::path targetPath = mp.PhysicalRoot / remainder;

                // NEW: Ensure the parent folder structure exists
                // e.g. writing to "assets/scenes/level1.json" will create "scenes" folder
                if (!std::filesystem::exists(targetPath.parent_path()))
                {
                    try {
                        std::filesystem::create_directories(targetPath.parent_path());
                    }
                    catch (...) {
                        AETHER_CORE_ERROR("VFS: Failed to create parent directory for {0}", targetPath.string());
                        return false;
                    }
                }

                std::ofstream out(targetPath);
                if (out)
                {
                    out << text;
                    out.close();
                    return true;
                }
                else
                {
                    AETHER_CORE_ERROR("VFS: Failed to open write stream: {0}", targetPath.string());
                    return false;
                }
            }
        }

        AETHER_CORE_ERROR("VFS: No mount point found for write: {0}", virtualPath);
        return false;
    }

    bool VFS::WriteBytes(const std::string& virtualPath, const std::vector<unsigned char>& data)
    {
        for (const auto& mp : s_MountPoints)
        {
            if (virtualPath.find(mp.VirtualRoot) == 0)
            {
                std::string remainder = virtualPath.substr(mp.VirtualRoot.length());
                if (remainder.find("/") == 0 || remainder.find("\\") == 0)
                    remainder = remainder.substr(1);

                std::filesystem::path targetPath = mp.PhysicalRoot / remainder;

                // Ensure parent folder exists
                if (!std::filesystem::exists(targetPath.parent_path()))
                {
                    try {
                        std::filesystem::create_directories(targetPath.parent_path());
                    }
                    catch (...) {
                        AETHER_CORE_ERROR("VFS: Failed to create parent directory for {0}", targetPath.string());
                        return false;
                    }
                }

                std::ofstream out(targetPath, std::ios::out | std::ios::binary);
                if (out)
                {
                    out.write(reinterpret_cast<const char*>(data.data()), data.size());
                    out.close();
                    return true;
                }
                else
                {
                    AETHER_CORE_ERROR("VFS: Failed to open write stream: {0}", targetPath.string());
                    return false;
                }
            }
        }

        AETHER_CORE_ERROR("VFS: No mount point found for write: {0}", virtualPath);
        return false;
    }
}