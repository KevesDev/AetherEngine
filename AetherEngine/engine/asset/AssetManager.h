#pragma once
#include "AssetLibrary.h"
#include "../project/Project.h"
#include <filesystem>
#include <string>
#include <vector>

namespace aether {

    class AssetManager
    {
    public:
        // --- Lifecycle ---
        static void Init();
        static void Shutdown();

        // --- Core API ---
        static AssetMetadata& GetMetadata(UUID handle);
        static AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

        static bool HasAsset(UUID handle);
        static bool HasAsset(const std::filesystem::path& filepath);

        static const AssetLibrary& GetLibrary();

        // --- Helper ---
        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension);

        // Returns a list of all extensions that can be imported (e.g. { ".png", ".jpg" })
        static std::vector<std::string> GetImportableExtensions();

        // --- Factory ---
        static void CreateAsset(const std::string& filename, const std::filesystem::path& directory, AssetType type);

        // Generic Import: Detects file type and runs specific import logic (e.g. generates .aeth wrapper)
        static void ImportSourceFile(const std::filesystem::path& sourcePath);

    private:
        static void ProcessDirectory(const std::filesystem::path& directory);
        static void ImportTexture(const std::filesystem::path& sourcePath);

    private:
        static std::unique_ptr<AssetLibrary> s_CurrentLibrary;
    };
}