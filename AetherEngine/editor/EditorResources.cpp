#include "EditorResources.h"
#include "../../engine/core/VFS.h"
#include "../../engine/core/Log.h"
#include "../../engine/renderer/Texture.h"

namespace aether {

    std::shared_ptr<Texture2D> EditorResources::FolderIcon = nullptr;
    std::shared_ptr<Texture2D> EditorResources::FileIcon = nullptr;
    std::shared_ptr<Texture2D> EditorResources::TextureIcon = nullptr;

    static std::shared_ptr<Texture2D> CreateFallbackIcon()
    {
        TextureSpecification spec;
        spec.Width = 1;
        spec.Height = 1;
        spec.Format = ImageFormat::RGBA8;

        auto tex = std::make_shared<Texture2D>(spec);
        uint32_t magenta = 0xff00ffff;
        tex->SetData(&magenta, sizeof(uint32_t));
        return tex;
    }

    void EditorResources::Init()
    {
        FolderIcon = LoadIcon("/engine/textures/icons/DirectoryIcon.png");
        if (!FolderIcon) {
            AETHER_CORE_WARN("EditorResources: Using fallback for FolderIcon");
            FolderIcon = CreateFallbackIcon();
        }

        FileIcon = LoadIcon("/engine/textures/icons/FileIcon.png");
        if (!FileIcon) {
            AETHER_CORE_WARN("EditorResources: Using fallback for FileIcon");
            FileIcon = CreateFallbackIcon();
        }

        TextureIcon = LoadIcon("/engine/textures/icons/TextureIcon.png");
        if (!TextureIcon) {
            AETHER_CORE_WARN("EditorResources: Using fallback for TextureIcon");
            TextureIcon = CreateFallbackIcon();
        }
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
        if (VFS::Resolve(virtualPath, physicalPath))
        {
            try {
                return std::make_shared<Texture2D>(physicalPath.string());
            }
            catch (const std::exception& e) {
                AETHER_CORE_ERROR("EditorResources: Exception loading '{0}': {1}", virtualPath, e.what());
                return nullptr;
            }
        }

        AETHER_CORE_ERROR("EditorResources: Could not resolve path: {}", virtualPath);
        return nullptr;
    }
}