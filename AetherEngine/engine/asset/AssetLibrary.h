#pragma once
#include "AssetMetadata.h"
#include <unordered_map>
#include <memory>

namespace aether {

    class AssetLibrary
    {
    public:
        AssetLibrary() = default;
        ~AssetLibrary() = default;

        // --- Core API ---
        void AddAsset(const AssetMetadata& metadata);
        void RemoveAsset(UUID handle);

        // --- Queries ---
        bool HasAsset(UUID handle) const;
        bool HasAsset(const std::filesystem::path& filepath) const;

        // Returns Metadata if found, otherwise throws or returns empty (safe check required via HasAsset)
        AssetMetadata& GetMetadata(UUID handle);
        const AssetMetadata& GetMetadata(UUID handle) const;

        // Lookup by Path (Used when Dragging files in)
        AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

        // --- Iterators (For the Content Browser) ---
        std::unordered_map<UUID, AssetMetadata>::iterator begin() { return m_Assets.begin(); }
        std::unordered_map<UUID, AssetMetadata>::iterator end() { return m_Assets.end(); }
        std::unordered_map<UUID, AssetMetadata>::const_iterator begin() const { return m_Assets.begin(); }
        std::unordered_map<UUID, AssetMetadata>::const_iterator end() const { return m_Assets.end(); }

    private:
        // Primary Map: UUID -> Data
        std::unordered_map<UUID, AssetMetadata> m_Assets;

        // Inverse Map: Path -> UUID (For fast import checking)
        // We use string keys to ensure consistency across Windows/Linux path separators
        std::unordered_map<std::string, UUID> m_PathToUUID;
    };
}