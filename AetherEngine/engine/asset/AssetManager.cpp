#include "AssetManager.h"
#include "AssetLibrarySerializer.h"
#include "../core/Log.h"

namespace aether {

    std::unique_ptr<AssetLibrary> AssetManager::s_CurrentLibrary = nullptr;

    void AssetManager::Init()
    {
        // 1. Create a new empty library
        s_CurrentLibrary = std::make_unique<AssetLibrary>();

        // 2. Load existing registry from disk (if it exists)
        auto project = Project::GetActive();
        AETHER_ASSERT(project, "AssetManager::Init called with no active project!");

        std::filesystem::path libraryPath = Project::GetSettingsDirectory() / "AssetLibrary.aethlib";
        AssetLibrarySerializer serializer(*s_CurrentLibrary);

        if (serializer.Deserialize(libraryPath)) {
            AETHER_CORE_INFO("AssetManager: Loaded existing Asset Library.");
        }
        else {
            AETHER_CORE_WARN("AssetManager: No existing library found. Creating new one.");
        }

        // 3. Scan the disk to find NEW files (Auto-Import)
        std::filesystem::path assetDir = Project::GetAssetDirectory();
        if (std::filesystem::exists(assetDir)) {
            ProcessDirectory(assetDir);
        }

        // 4. Save the library immediately to sync any new imports
        serializer.Serialize(libraryPath);
    }

    void AssetManager::Shutdown()
    {
        s_CurrentLibrary.reset();
    }

    void AssetManager::ProcessDirectory(const std::filesystem::path& directory)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                // Check if file is already known
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), Project::GetAssetDirectory());

                if (!s_CurrentLibrary->HasAsset(relativePath))
                {
                    // New file detected! Import it.
                    ImportAsset(relativePath);
                }
            }
        }
    }

    void AssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetMetadata metadata;
        metadata.Handle = UUID(); // Generate new ID
        metadata.FilePath = filepath;
        metadata.Type = GetAssetTypeFromExtension(filepath.extension());

        if (metadata.Type != AssetType::None) {
            s_CurrentLibrary->AddAsset(metadata);
            // Use standard logging macro
            AETHER_CORE_INFO("AssetManager: Auto-Imported '{}'", filepath.generic_string());
        }
    }

    AssetMetadata& AssetManager::GetMetadata(UUID handle)
    {
        return s_CurrentLibrary->GetMetadata(handle);
    }

    AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath)
    {
        return s_CurrentLibrary->GetMetadata(filepath);
    }

    bool AssetManager::HasAsset(UUID handle)
    {
        return s_CurrentLibrary && s_CurrentLibrary->HasAsset(handle);
    }

    bool AssetManager::HasAsset(const std::filesystem::path& filepath)
    {
        return s_CurrentLibrary && s_CurrentLibrary->HasAsset(filepath);
    }

    const AssetLibrary& AssetManager::GetLibrary()
    {
        return *s_CurrentLibrary;
    }

    AssetType AssetManager::GetAssetTypeFromExtension(const std::filesystem::path& extension)
    {
        std::string ext = extension.string();

        // Normalize to lowercase to ensure .PNG and .png are treated the same
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // Raw Texture Formats
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
            return AssetType::Texture2D;

        // Aether Internal Assets (Scenes, Materials, Prefabs)
        if (ext == ".aeth")
            return AssetType::Scene;

        // Scripts
        if (ext == ".cs" || ext == ".lua")
            return AssetType::Script;

        // Audio
        if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
            return AssetType::Audio;

        return AssetType::None;
    }
}