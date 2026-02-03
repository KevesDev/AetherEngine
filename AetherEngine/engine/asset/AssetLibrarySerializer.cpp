#include "AssetLibrarySerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace aether {

    AssetLibrarySerializer::AssetLibrarySerializer(AssetLibrary& library)
        : m_Library(library)
    {
    }

    bool AssetLibrarySerializer::Serialize(const std::filesystem::path& filepath)
    {
        json root = json::array();

        for (auto& [uuid, metadata] : m_Library)
        {
            json entry;
            entry["Handle"] = (uint64_t)metadata.Handle;
            entry["FilePath"] = metadata.FilePath.generic_string();
            entry["Type"] = (int)metadata.Type;
            root.push_back(entry);
        }

        std::ofstream fout(filepath);
        if (!fout)
        {
            AETHER_CORE_ERROR("Failed to open Asset Library for writing: {}", filepath.string());
            return false;
        }

        fout << root.dump(4);
        fout.close();
        return true;
    }

    bool AssetLibrarySerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream stream(filepath);
        if (!stream)
        {
            // It's okay if it doesn't exist yet (new project)
            return false;
        }

        json root;
        try
        {
            stream >> root;
        }
        catch (const json::parse_error& e)
        {
            AETHER_CORE_ERROR("Failed to parse Asset Library: {}", e.what());
            return false;
        }

        for (auto& entry : root)
        {
            AssetMetadata meta;
            meta.Handle = UUID(entry["Handle"].get<uint64_t>());
            meta.FilePath = entry["FilePath"].get<std::string>();
            meta.Type = (AssetType)entry["Type"].get<int>();

            m_Library.AddAsset(meta);
        }

        return true;
    }
}