#pragma once
#include "../core/UUID.h"
#include <filesystem>
#include <string>

namespace aether {

    enum class AssetType : uint16_t
    {
        None = 0,
        Texture2D,
        Scene,
        Script,
        Audio
        // TODO: We will add "Material" or "Prefab" here later
    };

    struct AssetMetadata
    {
        UUID Handle; // The Unique ID
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath; // Relative to Project Asset Directory

        // Helper to convert path to string for serialization
        std::string FilePathString() const { return FilePath.generic_string(); }
    };
}