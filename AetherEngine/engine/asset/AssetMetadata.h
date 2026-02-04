#pragma once

#include "../core/UUID.h"
#include <filesystem>
#include <string>

namespace aether {

    using AssetHandle = UUID;

    /**
     * AssetType
     * * Defines the specific runtime type of an asset.
     * * CRITICAL ARCHITECTURE NOTE:
     * These IDs are serialized into binary headers.
     * DO NOT reorder or insert values in the middle. Always append new types.
     * Explicit assignment is used here to guarantee stability against refactors.
     */
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
        Texture2D = 6,
        Audio = 7,
        Font = 8,

        // --- Mappings ---
        InputMappingContext = 9
    };

    /**
     * AssetHeader
     * * The binary prefix for all .aeth files.
     * Validates file integrity and type before loading the full payload.
     */
    struct AssetHeader
    {
        char Magic[4] = { 'A', 'E', 'T', 'H' };
        uint32_t Version = 1;
        AssetType Type = AssetType::None;
        uint64_t AssetID = 0;
    };

    /**
     * AssetMetadata
     * * Runtime registry data for an asset.
     * Used by the AssetManager to look up files without loading them.
     */
    struct AssetMetadata
    {
        AssetHandle Handle;
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;

        std::string FilePathString() const { return FilePath.generic_string(); }
    };
}