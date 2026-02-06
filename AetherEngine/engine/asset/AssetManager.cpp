#include "AssetManager.h"
#include "AssetLibrarySerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp" 
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

namespace aether {

    std::unique_ptr<AssetLibrary> AssetManager::s_CurrentLibrary = nullptr;
    std::unordered_map<UUID, std::any> AssetManager::s_AssetCache;
    std::mutex AssetManager::s_CacheMutex;

    void AssetManager::Init()
    {
        s_CurrentLibrary = std::make_unique<AssetLibrary>();
        std::filesystem::path libraryPath = Project::GetSettingsDirectory() / "AssetLibrary.aethlib";
        AssetLibrarySerializer serializer(*s_CurrentLibrary);

        if (serializer.Deserialize(libraryPath)) {
            AETHER_CORE_INFO("AssetManager: Loaded existing Asset Library.");
        }
        else {
            AETHER_CORE_WARN("AssetManager: No existing library found. Creating new one.");
        }

        std::filesystem::path assetDir = Project::GetAssetDirectory();
        if (std::filesystem::exists(assetDir)) ProcessDirectory(assetDir);
        serializer.Serialize(libraryPath);
    }

    void AssetManager::Shutdown()
    {
        std::lock_guard<std::mutex> lock(s_CacheMutex);
        s_AssetCache.clear();
        s_CurrentLibrary.reset();
    }

    std::vector<std::string> AssetManager::GetImportableExtensions() { return { ".png", ".jpg", ".jpeg" }; }

    static bool ReadAssetHeader(const std::filesystem::path& path, AssetHeader& outHeader)
    {
        std::filesystem::path absolutePath = Project::GetAssetDirectory() / path;
        std::ifstream stream(absolutePath, std::ios::binary);
        if (!stream) return false;
        stream.read(reinterpret_cast<char*>(&outHeader), sizeof(AssetHeader));
        return (outHeader.Magic[0] == 'A' && outHeader.Magic[1] == 'E' && outHeader.Magic[2] == 'T' && outHeader.Magic[3] == 'H');
    }

    void AssetManager::ProcessDirectory(const std::filesystem::path& directory)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                std::filesystem::path fullPath = entry.path();
                std::string ext = fullPath.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                auto importable = GetImportableExtensions();
                if (std::find(importable.begin(), importable.end(), ext) != importable.end())
                {
                    std::filesystem::path assetPath = fullPath;
                    assetPath.replace_extension(".aeth");
                    if (!std::filesystem::exists(assetPath)) ImportSourceFile(fullPath);
                }
                else if (ext == ".aeth")
                {
                    std::filesystem::path relativePath = std::filesystem::relative(fullPath, Project::GetAssetDirectory());
                    AssetHeader header;
                    if (ReadAssetHeader(relativePath, header))
                    {
                        if (!s_CurrentLibrary->HasAsset((UUID)header.AssetID))
                        {
                            AssetMetadata metadata;
                            metadata.Handle = (UUID)header.AssetID;
                            metadata.FilePath = relativePath;
                            metadata.Type = header.Type;
                            s_CurrentLibrary->AddAsset(metadata);
                        }
                    }
                }
            }
        }
    }

    void AssetManager::ImportSourceFile(const std::filesystem::path& sourcePath)
    {
        std::string ext = sourcePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") ImportTexture(sourcePath);
    }

    void AssetManager::ImportTexture(const std::filesystem::path& sourcePath)
    {
        std::filesystem::path assetPath = sourcePath;
        assetPath.replace_extension(".aeth");
        std::ofstream fout(assetPath, std::ios::binary);
        if (!fout) return;

        UUID newHandle;
        AssetHeader header;
        header.Type = AssetType::Texture2D;
        header.AssetID = (uint64_t)newHandle;
        fout.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        json meta;
        meta["Source"] = std::filesystem::relative(sourcePath, Project::GetAssetDirectory()).generic_string();
        meta["Filter"] = "Nearest";
        meta["Wrap"] = "Repeat";

        std::string dump = meta.dump(4);
        fout.write(dump.c_str(), dump.size());
        fout.close();

        std::filesystem::path relativePath = std::filesystem::relative(assetPath, Project::GetAssetDirectory());
        AssetMetadata metadata;
        metadata.Handle = newHandle;
        metadata.FilePath = relativePath;
        metadata.Type = AssetType::Texture2D;
        s_CurrentLibrary->AddAsset(metadata);
    }

    void AssetManager::CreateAsset(const std::string& filename, const std::filesystem::path& directory, AssetType type)
    {
        std::string finalFilename = filename;
        if (finalFilename.find(".aeth") == std::string::npos) finalFilename += ".aeth";
        std::filesystem::path fullPath = directory / finalFilename;
        if (std::filesystem::exists(fullPath)) return;

        std::ofstream fout(fullPath, std::ios::binary);
        if (!fout) return;

        UUID newHandle;
        AssetHeader header;
        header.Type = type;
        header.AssetID = (uint64_t)newHandle;
        fout.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        json defaultData;
        if (type == AssetType::Scene) { defaultData["Scene"] = "Untitled Scene"; defaultData["Entities"] = json::array(); }
        else if (type == AssetType::LogicGraph) { defaultData["Graph"] = "New Logic Graph"; defaultData["Nodes"] = json::array(); }

        std::string dump = defaultData.dump(4);
        fout.write(dump.c_str(), dump.size());
        fout.close();

        std::filesystem::path relativePath = std::filesystem::relative(fullPath, Project::GetAssetDirectory());
        AssetMetadata metadata;
        metadata.Handle = newHandle;
        metadata.FilePath = relativePath;
        metadata.Type = type;
        s_CurrentLibrary->AddAsset(metadata);
    }

    AssetMetadata& AssetManager::GetMetadata(UUID handle) { return s_CurrentLibrary->GetMetadata(handle); }
    AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath) { return s_CurrentLibrary->GetMetadata(filepath); }
    bool AssetManager::HasAsset(UUID handle) { return s_CurrentLibrary && s_CurrentLibrary->HasAsset(handle); }
    bool AssetManager::HasAsset(const std::filesystem::path& filepath) { return s_CurrentLibrary && s_CurrentLibrary->HasAsset(filepath); }
    const AssetLibrary& AssetManager::GetLibrary() { return *s_CurrentLibrary; }

    AssetType AssetManager::GetAssetTypeFromExtension(const std::filesystem::path& path)
    {
        AssetHeader header;
        if (ReadAssetHeader(path, header)) return header.Type;
        return AssetType::None;
    }

    std::shared_ptr<Texture2D> AssetManager::LoadTexture2D(const std::filesystem::path& assetPath)
    {
        std::ifstream stream(assetPath, std::ios::binary);
        if (!stream) {
            AETHER_CORE_ERROR("AssetManager::LoadTexture2D - Failed to open file: {}", assetPath.string());
            return nullptr;
        }

        AssetHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        json meta;
        try {
            stream >> meta;
        }
        catch (const json::parse_error& e) {
            AETHER_CORE_ERROR("AssetManager::LoadTexture2D - JSON parse error: {}", e.what());
            return nullptr;
        }

        std::string sourceRel = meta.value("Source", "");
        if (sourceRel.empty()) {
            AETHER_CORE_ERROR("AssetManager::LoadTexture2D - No source file specified");
            return nullptr;
        }

        std::filesystem::path sourcePath = Project::GetAssetDirectory() / sourceRel;

        TextureSpecification spec;
        std::string filter = meta.value("Filter", "Linear");
        spec.MinFilter = (filter == "Nearest") ? GL_NEAREST : GL_LINEAR;
        spec.MagFilter = (filter == "Nearest") ? GL_NEAREST : GL_LINEAR;

        std::string wrap = meta.value("Wrap", "Repeat");
        spec.WrapS = (wrap == "Clamp") ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        spec.WrapT = (wrap == "Clamp") ? GL_CLAMP_TO_EDGE : GL_REPEAT;

        try {
            return std::make_shared<Texture2D>(sourcePath.string(), spec);
        }
        catch (const std::exception& e) {
            AETHER_CORE_ERROR("AssetManager: Failed to load texture '{0}'. Reason: {1}", sourceRel, e.what());
            return nullptr;
        }
    }
}