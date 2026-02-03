#include "AssetLibrary.h"
#include "../core/Log.h" //

namespace aether {

    void AssetLibrary::AddAsset(const AssetMetadata& metadata)
    {
        if (HasAsset(metadata.Handle)) {
            // Use {} for format string to be safe
            AETHER_CORE_WARN("AssetLibrary: Attempted to add duplicate asset UUID: {}", (uint64_t)metadata.Handle);
            return;
        }

        std::string pathStr = metadata.FilePath.generic_string();

        m_Assets[metadata.Handle] = metadata;
        m_PathToUUID[pathStr] = metadata.Handle;
    }

    void AssetLibrary::RemoveAsset(UUID handle)
    {
        if (!HasAsset(handle)) return;

        AssetMetadata& meta = m_Assets[handle];
        std::string pathStr = meta.FilePath.generic_string();

        m_PathToUUID.erase(pathStr);
        m_Assets.erase(handle);
    }

    bool AssetLibrary::HasAsset(UUID handle) const
    {
        return m_Assets.find(handle) != m_Assets.end();
    }

    bool AssetLibrary::HasAsset(const std::filesystem::path& filepath) const
    {
        return m_PathToUUID.find(filepath.generic_string()) != m_PathToUUID.end();
    }

    AssetMetadata& AssetLibrary::GetMetadata(UUID handle)
    {
        AETHER_ASSERT(HasAsset(handle), "Asset not found in library!");
        return m_Assets[handle];
    }

    const AssetMetadata& AssetLibrary::GetMetadata(UUID handle) const
    {
        AETHER_ASSERT(HasAsset(handle), "Asset not found in library!");
        return m_Assets.at(handle);
    }

    AssetMetadata& AssetLibrary::GetMetadata(const std::filesystem::path& filepath)
    {
        std::string pathStr = filepath.generic_string();
        AETHER_ASSERT(HasAsset(filepath), "Asset path not found in library!");
        return m_Assets[m_PathToUUID.at(pathStr)];
    }
}