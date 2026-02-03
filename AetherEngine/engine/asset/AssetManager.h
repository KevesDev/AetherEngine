#pragma once
#include "AssetLibrary.h"
#include "../project/Project.h"
#include <filesystem>

namespace aether {

    class AssetManager
    {
    public:
        // --- Lifecycle ---
        // Called when a project is loaded to sync the library with the disk
        static void Init();
        static void Shutdown();

        // --- Core API ---
        static AssetMetadata& GetMetadata(UUID handle);
        static AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

        static bool HasAsset(UUID handle);
        static bool HasAsset(const std::filesystem::path& filepath);

        // Returns the underlying Library (for the Content Browser to list items)
        static const AssetLibrary& GetLibrary();

        // --- Helper ---
        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension);

    private:
        static void ProcessDirectory(const std::filesystem::path& directory);
        static void ImportAsset(const std::filesystem::path& filepath);

    private:
        static std::unique_ptr<AssetLibrary> s_CurrentLibrary;
    };
}