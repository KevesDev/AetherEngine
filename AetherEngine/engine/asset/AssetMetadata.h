#pragma once
#include "../core/UUID.h"
#include <filesystem>
#include <string>

namespace aether {

    // Locking IDs to prevent binary breakage when inserting new types.
    enum class AssetType : uint16_t
    {
        None = 0,

        // --- Core Engine Assets ---
        Scene = 1,
        Prefab = 2,
        Material = 3,
        PhysicsMaterial = 4,
        LogicGraph = 5,

        // --- Imported Media Assets ---
        // (Must keep these IDs stable to load existing .aeth files)
        Texture2D = 6,
        Audio = 7,
        Font = 8,

        // --- New Additions (Append Only) ---
        InputMappingContext = 9
    };

    struct AssetHeader
    {
        char Magic[4] = { 'A', 'E', 'T', 'H' };
        uint32_t Version = 1;
        AssetType Type = AssetType::None;
        uint64_t AssetID = 0;
    };

    struct AssetMetadata
    {
        UUID Handle;
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;

        std::string FilePathString() const { return FilePath.generic_string(); }
    };
}