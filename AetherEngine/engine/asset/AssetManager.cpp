#include "AssetManager.h"
#include "AssetLibrarySerializer.h"
#include "../core/Log.h"

namespace aether {

    std::unique_ptr<AssetLibrary> AssetManager::s_CurrentLibrary = nullptr;

    void AssetManager::Init()
    {
        s_CurrentLibrary = std::make_unique<AssetLibrary>();

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

        std::filesystem::path assetDir = Project::GetAssetDirectory();
        if (std::filesystem::exists(assetDir)) {
            ProcessDirectory(assetDir);
        }

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
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), Project::GetAssetDirectory());

                if (!s_CurrentLibrary->HasAsset(relativePath))
                {
                    ImportAsset(relativePath);
                }
            }
        }
    }

    void AssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        // Production safety guard: Do not proceed if library is missing
        if (!s_CurrentLibrary) return;

        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = filepath;
        metadata.Type = GetAssetTypeFromExtension(filepath.extension());

        if (metadata.Type != AssetType::None) {
            s_CurrentLibrary->AddAsset(metadata);
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
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") return AssetType::Texture2D;
        if (ext == ".aeth") return AssetType::Scene;
        if (ext == ".cs" || ext == ".lua") return AssetType::Script;
        if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") return AssetType::Audio;

        return AssetType::None;
    }
}