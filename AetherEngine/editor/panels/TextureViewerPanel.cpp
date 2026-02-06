#include "TextureViewerPanel.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/asset/AssetMetadata.h"
#include "../../engine/core/VFS.h"
#include <imgui.h>
#include <fstream>

namespace aether {

    TextureViewerPanel::TextureViewerPanel(const std::string& title, const std::filesystem::path& path)
        : AssetEditorPanel(title, path)
    {
        m_DefaultWindowSize = { 400.0f, 420.0f };

        m_Texture = AssetManager::Get<Texture2D>(path.string());

        if (m_Texture)
        {
            m_IsPixelArt = (m_Texture->GetSpecification().MagFilter == GL_NEAREST);
        }

        m_CheckerboardTexture = AssetManager::Get<Texture2D>("/engine/textures/T_checkerboard.png");

        if (!m_CheckerboardTexture)
        {
            std::filesystem::path p;
            if (VFS::Resolve("/engine/textures/T_checkerboard.png", p))
            {
                if (std::filesystem::exists(p))
                    m_CheckerboardTexture = std::make_shared<Texture2D>(p.string());
            }
        }

        if (!m_CheckerboardTexture)
        {
            TextureSpecification spec;
            spec.Width = 1;
            spec.Height = 1;
            spec.Format = ImageFormat::RGBA8;
            m_CheckerboardTexture = std::make_shared<Texture2D>(spec);
            uint32_t white = 0xffffffff;
            m_CheckerboardTexture->SetData(&white, sizeof(uint32_t));
        }
    }

    void TextureViewerPanel::RenderInspector()
    {
        // Settings Section
        ImGui::SeparatorText("Settings");

        // Zoom Slider
        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1); // Stretch to edge
        ImGui::SliderFloat("##Zoom", &m_Zoom, 0.1f, 5.0f, "%.2fx");

        // Pixel Art Toggle
        if (ImGui::Checkbox("Pixel Art Mode", &m_IsPixelArt))
        {
            SetDirty(true);
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Asset Info");

        // Asset Details
        if (m_Texture)
        {
            ImGui::TextWrapped("Path: %s", m_AssetPath.filename().string().c_str());
            ImGui::Text("Size: %dx%d", m_Texture->GetWidth(), m_Texture->GetHeight());

            // Calculate file size if possible (approximate based on memory)
            uint32_t memSize = m_Texture->GetWidth() * m_Texture->GetHeight() * 4;
            ImGui::TextDisabled("VRAM: %.2f KB", memSize / 1024.0f);
        }
    }

    void TextureViewerPanel::RenderContent()
    {
        if (m_Texture && m_CheckerboardTexture)
        {
            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // 1. Calculate Base Fit Size (fitting image into current view)
            float aspect = (float)m_Texture->GetWidth() / (float)m_Texture->GetHeight();
            ImVec2 baseSize = availSize;

            if (baseSize.x / baseSize.y > aspect)
                baseSize.x = baseSize.y * aspect;
            else
                baseSize.y = baseSize.x / aspect;

            // 2. Apply Zoom
            ImVec2 renderSize = { baseSize.x * m_Zoom, baseSize.y * m_Zoom };

            // 3. Render
            // Store cursor to overlap the two images
            ImVec2 startPos = ImGui::GetCursorPos();

            // Background (Checkerboard)
            ImGui::Image((void*)(uintptr_t)m_CheckerboardTexture->GetRendererID(), renderSize, { 0, 1 }, { 1, 0 });

            // Reset cursor to draw Texture on top
            ImGui::SetCursorPos(startPos);

            // Texture
            ImGui::Image((void*)(uintptr_t)m_Texture->GetRendererID(), renderSize, { 0, 1 }, { 1, 0 });
        }
        else
        {
            ImGui::Text("Texture not loaded.");
        }
    }

    EditorSaveResult TextureViewerPanel::Save()
    {
        if (!m_Texture) return EditorSaveResult::Failure;

        TextureSpecification spec = m_Texture->GetSpecification();
        if (m_IsPixelArt)
        {
            spec.MinFilter = GL_NEAREST;
            spec.MagFilter = GL_NEAREST;
        }
        else
        {
            spec.MinFilter = GL_LINEAR;
            spec.MagFilter = GL_LINEAR;
        }

        if (!AssetManager::HasAsset(m_AssetPath))
            return EditorSaveResult::Failure;

        const auto& metadata = AssetManager::GetMetadata(m_AssetPath);

        AssetHeader header;
        header.AssetID = (uint64_t)metadata.Handle;
        header.Type = AssetType::Texture2D;
        header.Version = 1;

        std::ofstream fout(m_AssetPath, std::ios::binary);
        if (!fout)
        {
            return EditorSaveResult::Failure;
        }

        fout.write(reinterpret_cast<const char*>(&header), sizeof(AssetHeader));
        fout.write(reinterpret_cast<const char*>(&spec), sizeof(TextureSpecification));

        std::string sourcePath = m_Texture->GetPath();
        size_t pathLen = sourcePath.size();
        fout.write(reinterpret_cast<const char*>(&pathLen), sizeof(size_t));
        fout.write(sourcePath.data(), pathLen);

        fout.close();

        SetDirty(false);
        return EditorSaveResult::Success;
    }
}