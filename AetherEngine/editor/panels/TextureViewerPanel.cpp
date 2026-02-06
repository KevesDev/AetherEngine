#include "TextureViewerPanel.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/asset/AssetMetadata.h" // Required for AssetHeader
#include "../../engine/core/VFS.h"
#include <imgui.h>
#include <fstream>

namespace aether {

    TextureViewerPanel::TextureViewerPanel(const std::string& title, const std::filesystem::path& path)
        : AssetEditorPanel(title, path)
    {
        // 1. Load the target asset
        m_Texture = AssetManager::Get<Texture2D>(path.string());

        // Initialize metadata state from the loaded asset
        if (m_Texture)
        {
            // If MagFilter is Nearest, we treat it as "Pixel Art" mode
            m_IsPixelArt = (m_Texture->GetSpecification().MagFilter == GL_NEAREST);
        }

        // 2. Load the background checkerboard (Safe Load)
        m_CheckerboardTexture = AssetManager::Get<Texture2D>("/engine/textures/T_checkerboard.png");

        // If that failed, try manual VFS resolution (Engine scope)
        if (!m_CheckerboardTexture)
        {
            std::filesystem::path p;
            if (VFS::Resolve("/engine/textures/T_checkerboard.png", p))
            {
                if (std::filesystem::exists(p))
                    m_CheckerboardTexture = std::make_shared<Texture2D>(p.string());
            }
        }

        // Fallback: Use TextureSpecification to create 1x1 texture
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

    void TextureViewerPanel::RenderContent()
    {
        // Toolbar
        ImGui::BeginGroup();
        if (ImGui::Checkbox("Pixel Art", &m_IsPixelArt))
        {
            SetDirty(true);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("| %s", m_AssetPath.filename().string().c_str());
        ImGui::EndGroup();

        ImGui::Separator();

        // Preview
        if (m_Texture && m_CheckerboardTexture)
        {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            // Calculate aspect ratio to fit image within window
            float aspect = (float)m_Texture->GetWidth() / (float)m_Texture->GetHeight();
            ImVec2 size = viewportSize;

            if (size.x / size.y > aspect)
                size.x = size.y * aspect;
            else
                size.y = size.x / aspect;

            // Draw Checkerboard Background
            ImGui::SetCursorPos(ImGui::GetCursorStartPos());
            ImGui::Image((void*)(uintptr_t)m_CheckerboardTexture->GetRendererID(), size, { 0, 1 }, { 1, 0 });

            // Draw Actual Texture on top
            ImGui::SetCursorPos(ImGui::GetCursorStartPos());
            ImGui::Image((void*)(uintptr_t)m_Texture->GetRendererID(), size, { 0, 1 }, { 1, 0 });

            // Tooltip
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Dimensions: %dx%d", m_Texture->GetWidth(), m_Texture->GetHeight());
                ImGui::Text("Path: %s", m_Texture->GetPath().c_str());
                ImGui::EndTooltip();
            }
        }
        else
        {
            ImGui::Text("Texture not loaded.");
        }
    }

    EditorSaveResult TextureViewerPanel::Save()
    {
        if (!m_Texture) return EditorSaveResult::Failure;

        // 1. Prepare Metadata (TextureSpecification)
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

        // 2. Prepare Header (Maintain existing UUID)
        if (!AssetManager::HasAsset(m_AssetPath))
            return EditorSaveResult::Failure;

        const auto& metadata = AssetManager::GetMetadata(m_AssetPath);

        AssetHeader header;
        header.AssetID = (uint64_t)metadata.Handle;
        header.Type = AssetType::Texture2D;
        header.Version = 1;

        // 3. Serialize to Disk (Binary Format: Header + Spec + SourcePath)
        // Note: We write to the .aeth file, which AssetManager reads to load the texture.
        std::ofstream fout(m_AssetPath, std::ios::binary);
        if (!fout)
        {
            AETHER_CORE_ERROR("TextureViewer: Failed to open file for saving: {}", m_AssetPath.string());
            return EditorSaveResult::Failure;
        }

        // Write Header
        fout.write(reinterpret_cast<const char*>(&header), sizeof(AssetHeader));

        // Write Specification
        fout.write(reinterpret_cast<const char*>(&spec), sizeof(TextureSpecification));

        // Write Source Path (Length + String)
        // Use the texture's stored path to the raw image (e.g., "assets/textures/wood.png")
        std::string sourcePath = m_Texture->GetPath();
        size_t pathLen = sourcePath.size();
        fout.write(reinterpret_cast<const char*>(&pathLen), sizeof(size_t));
        fout.write(sourcePath.data(), pathLen);

        fout.close();

        // Mark clean
        SetDirty(false);
        AETHER_CORE_INFO("TextureViewer: Saved metadata for {}", m_AssetPath.string());
        return EditorSaveResult::Success;
    }
}