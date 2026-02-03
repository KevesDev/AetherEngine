#pragma once
#include "../core/UUID.h"
#include <filesystem>
#include <string>

namespace aether {

    // The canonical list of all data types the engine understands.
    enum class AssetType : uint16_t
    {
        None = 0,

        // --- Aether Binary Containers (.aeth) ---
        Scene,           // A full level/world
        Prefab,          // A reusable entity template
        Material,        // Shader reference + uniform data
        PhysicsMaterial, // Friction/Bounciness data
        LogicGraph,      // Visual scripting/Node graph

        // --- Raw Importable Types ---
        Texture2D,       // .png, .jpg (Waiting to be packed)
        Audio,           // .wav, .ogg
        Font             // .ttf, .otf
    };

    // Every .aeth file begins with this header.
    // This ensures we never load a corrupted or malicious file.
    struct AssetHeader
    {
        char Magic[4] = { 'A', 'E', 'T', 'H' }; // "AETH" Magic Number
        uint32_t Version = 1;                   // For future backward compatibility
        AssetType Type = AssetType::None;       // The actual content type
        uint64_t AssetID = 0;                   // The persistent UUID of this asset
    };

    struct AssetMetadata
    {
        UUID Handle;
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;

        std::string FilePathString() const { return FilePath.generic_string(); }
    };
}