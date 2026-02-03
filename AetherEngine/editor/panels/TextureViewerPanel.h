#pragma once

#include "AssetEditorPanel.h"
#include "../../engine/renderer/Texture.h"
#include <memory>

namespace aether {

    class TextureViewerPanel : public AssetEditorPanel
    {
    public:
        TextureViewerPanel(const std::string& title, const std::filesystem::path& assetPath);
        virtual ~TextureViewerPanel() = default;

    protected:
        virtual void RenderContent() override;
        virtual EditorSaveResult Save() override;

    private:
        void RenderToolbar();
        void RenderPreview();

        // Helper to load the .aeth metadata + source png
        void LoadAsset();

    private:
        std::shared_ptr<Texture2D> m_Texture;

        // Editor State
        float m_Zoom = 1.0f;

        // Metadata State (Loaded from .aeth)
        bool m_IsPixelArt = false;
    };
}