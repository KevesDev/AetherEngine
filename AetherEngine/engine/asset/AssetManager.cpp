#include "AssetManager.h"
#include "AssetLibrarySerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp" // Required for default content generation
#include <fstream>

using json = nlohmann::json;

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
        if (!s_CurrentLibrary) return;

        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = filepath;
        metadata.Type = GetAssetTypeFromExtension(filepath);

        if (metadata.Type != AssetType::None)
        {
            s_CurrentLibrary->AddAsset(metadata);
            // We don't log every auto-import to keep console clean, 
            // but we can add AETHER_CORE_TRACE here if you want debugging.
        }
    }

    void AssetManager::CreateAsset(const std::string& filename, const std::filesystem::path& directory, AssetType type)
    {
        // 1. Sanitize Filename (ensure extension)
        std::string finalFilename = filename;
        if (finalFilename.find(".aeth") == std::string::npos)
            finalFilename += ".aeth";

        std::filesystem::path fullPath = directory / finalFilename;

        if (std::filesystem::exists(fullPath))
        {
            AETHER_CORE_WARN("AssetManager: Creation failed. '{0}' already exists!", finalFilename);
            return;
        }

        // 2. Write Security Header + Default Content
        std::ofstream fout(fullPath, std::ios::binary);
        if (!fout)
        {
            AETHER_CORE_ERROR("AssetManager: Failed to write to '{0}'", fullPath.string());
            return;
        }

        AssetHeader header;
        header.Type = type;

        // Write Header
        fout.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        // Write Body (Template)
        json defaultData;
        if (type == AssetType::Scene)
        {
            defaultData["Scene"] = "Untitled Scene";
            defaultData["Entities"] = json::array();
        }
        else if (type == AssetType::LogicGraph)
        {
            defaultData["Graph"] = "New Logic Graph";
            defaultData["Nodes"] = json::array();
        }

        std::string dump = defaultData.dump(4);
        fout.write(dump.c_str(), dump.size());
        fout.close();

        // 3. Register immediately
        std::filesystem::path relativePath = std::filesystem::relative(fullPath, Project::GetAssetDirectory());
        ImportAsset(relativePath);

        AETHER_CORE_INFO("AssetManager: Created and registered '{0}'", finalFilename);
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

    AssetType AssetManager::GetAssetTypeFromExtension(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // Security Check: Sniff Header
        if (ext == ".aeth")
        {
            std::filesystem::path absolutePath = Project::GetAssetDirectory() / path;
            std::ifstream stream(absolutePath, std::ios::binary);
            if (!stream) return AssetType::None;

            AssetHeader header;
            stream.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

            // Validate "AETH"
            if (header.Magic[0] != 'A' || header.Magic[1] != 'E' ||
                header.Magic[2] != 'T' || header.Magic[3] != 'H')
            {
                return AssetType::None;
            }

            return header.Type;
        }

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") return AssetType::Texture2D;
        if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") return AssetType::Audio;
        if (ext == ".ttf" || ext == ".otf") return AssetType::Font;

        return AssetType::None;
    }
}