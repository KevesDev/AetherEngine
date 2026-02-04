#pragma once
#include "AssetLibrary.h"
#include "../project/Project.h"
#include "../renderer/Texture.h"
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <any>

namespace aether {

    /**
     * AssetManager: Runtime Resource Manager
     * Thread-safe asset loading with automatic caching and typed retrieval.
     * Follows the "Black Box" principle - handles all resource lifecycle internally.
     */
    class AssetManager
    {
    public:
        // --- Lifecycle ---
        static void Init();
        static void Shutdown();

        // --- Runtime Asset Retrieval (Production API) ---
        /**
         * Get<T>: Typed asset retrieval with automatic loading and caching.
         *
         * @param filepath: Relative path from Asset Directory OR absolute path
         * @return Shared pointer to loaded asset, nullptr if not found/failed
         *
         * Thread-safe. First call loads from disk, subsequent calls return cached instance.
         * Example: auto texture = AssetManager::Get<Texture2D>("Textures/Hero.aeth");
         */
        template<typename T>
        static std::shared_ptr<T> Get(const std::filesystem::path& filepath);

        /**
         * GetAsset<T>: Typed asset retrieval by UUID.
         *
         * @param handle: Asset UUID from metadata
         * @return Shared pointer to loaded asset, nullptr if not found/failed
         */
        template<typename T>
        static std::shared_ptr<T> GetAsset(UUID handle);

        // --- Metadata API (Existing) ---
        static AssetMetadata& GetMetadata(UUID handle);
        static AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

        static bool HasAsset(UUID handle);
        static bool HasAsset(const std::filesystem::path& filepath);

        static const AssetLibrary& GetLibrary();

        // --- Helper ---
        static AssetType GetAssetTypeFromExtension(const std::filesystem::path& extension);
        static std::vector<std::string> GetImportableExtensions();

        // --- Factory ---
        static void CreateAsset(const std::string& filename, const std::filesystem::path& directory, AssetType type);
        static void ImportSourceFile(const std::filesystem::path& sourcePath);

    private:
        static void ProcessDirectory(const std::filesystem::path& directory);
        static void ImportTexture(const std::filesystem::path& sourcePath);

        // --- Asset Factories (Internal) ---
        static std::shared_ptr<Texture2D> LoadTexture2D(const std::filesystem::path& assetPath);

    private:
        static std::unique_ptr<AssetLibrary> s_CurrentLibrary;

        // Runtime Cache: UUID -> Asset Instance
        static std::unordered_map<UUID, std::any> s_AssetCache;
        static std::mutex s_CacheMutex;
    };

    // --- Template Implementation ---
    template<typename T>
    std::shared_ptr<T> AssetManager::Get(const std::filesystem::path& filepath)
    {
        // Resolve to absolute path
        std::filesystem::path absolutePath = filepath;
        if (filepath.is_relative()) {
            absolutePath = Project::GetAssetDirectory() / filepath;
        }

        // Check if asset exists in metadata
        std::filesystem::path relativePath = std::filesystem::relative(absolutePath, Project::GetAssetDirectory());
        if (!HasAsset(relativePath)) {
            AETHER_CORE_ERROR("AssetManager::Get - Asset not found: {}", filepath.string());
            return nullptr;
        }

        auto& metadata = GetMetadata(relativePath);
        return GetAsset<T>(metadata.Handle);
    }

    template<typename T>
    std::shared_ptr<T> AssetManager::GetAsset(UUID handle)
    {
        std::lock_guard<std::mutex> lock(s_CacheMutex);

        // Check cache first
        auto it = s_AssetCache.find(handle);
        if (it != s_AssetCache.end()) {
            try {
                return std::any_cast<std::shared_ptr<T>>(it->second);
            }
            catch (const std::bad_any_cast&) {
                AETHER_CORE_ERROR("AssetManager::GetAsset - Type mismatch for cached asset");
                return nullptr;
            }
        }

        // Load from disk
        if (!HasAsset(handle)) {
            AETHER_CORE_ERROR("AssetManager::GetAsset - Asset handle not found");
            return nullptr;
        }

        auto& metadata = GetMetadata(handle);
        std::filesystem::path fullPath = Project::GetAssetDirectory() / metadata.FilePath;

        std::shared_ptr<T> asset = nullptr;

        // Type-specific factory dispatch
        if constexpr (std::is_same_v<T, Texture2D>) {
            asset = LoadTexture2D(fullPath);
        }
        else {
            AETHER_CORE_ERROR("AssetManager::GetAsset - Unsupported asset type");
            return nullptr;
        }

        // Cache the loaded asset
        if (asset) {
            s_AssetCache[handle] = asset;
        }

        return asset;
    }
}