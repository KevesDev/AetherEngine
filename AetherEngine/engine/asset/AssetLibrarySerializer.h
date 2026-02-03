#pragma once
#include "AssetLibrary.h"
#include <filesystem>

namespace aether {

    class AssetLibrarySerializer
    {
    public:
        AssetLibrarySerializer(AssetLibrary& library);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

    private:
        AssetLibrary& m_Library;
    };
}