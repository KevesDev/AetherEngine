#include "EditorResources.h"
#include "../../engine/core/VFS.h"
#include "../../engine/core/Log.h"

namespace aether {

    std::shared_ptr<Texture2D> EditorResources::FolderIcon = nullptr;
    std::shared_ptr<Texture2D> EditorResources::FileIcon = nullptr;
    std::shared_ptr<Texture2D> EditorResources::TextureIcon = nullptr;

    void EditorResources::Init()
    {
        // Since EngineContent is mounted to "/engine", these are the correct virtual paths
        FolderIcon = LoadIcon("/engine/textures/icons/DirectoryIcon.png");
        FileIcon = LoadIcon("/engine/textures/icons/FileIcon.png");
        TextureIcon = LoadIcon("/engine/textures/icons/TextureIcon.png");
    }

    void EditorResources::Shutdown()
    {
        FolderIcon.reset();
        FileIcon.reset();
        TextureIcon.reset();
    }

    std::shared_ptr<Texture2D> EditorResources::LoadIcon(const std::string& virtualPath)
    {
        std::filesystem::path physicalPath;

        // Using your VFS signature: bool Resolve(const std::string&, std::filesystem::path&)
        if (VFS::Resolve(virtualPath, physicalPath))
        {
            // physicalPath.string() gives the absolute path on the disk for stb_image to load
            return std::make_shared<Texture2D>(physicalPath.string());
        }

        AETHER_CORE_ERROR("EditorResources: Failed to resolve icon path: {}", virtualPath);
        return nullptr;
    }
}