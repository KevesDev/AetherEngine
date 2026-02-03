#pragma once
#include "../../engine/renderer/Texture.h"
#include <memory>
#include <string>

namespace aether {

    class EditorResources
    {
    public:
        static void Init();
        static void Shutdown();

        // System Icons
        static std::shared_ptr<Texture2D> FolderIcon;
        static std::shared_ptr<Texture2D> FileIcon;
        static std::shared_ptr<Texture2D> TextureIcon;

    private:
        // Helper to load icons using your existing VFS logic
        static std::shared_ptr<Texture2D> LoadIcon(const std::string& virtualPath);
    };
}