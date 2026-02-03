#include "TextureViewerPanel.h"
#include "../../engine/core/Log.h"
#include "../../engine/core/Theme.h"
#include "../../engine/project/Project.h"
#include "../../engine/asset/AssetMetadata.h"
#include "../../engine/vendor/json.hpp"

#include <imgui.h>
#include <fstream>

using json = nlohmann::json;

namespace aether {

    TextureViewerPanel::TextureViewerPanel(const std::string& title, const std::filesystem::path& assetPath)
        : AssetEditorPanel(title, assetPath)
    {
        LoadAsset();
    }

    void TextureViewerPanel::LoadAsset()
    {
        // Resolve the relative path (stored in m_AssetPath) to an Absolute Path for IO
        std::filesystem::path fullPath = Project::GetAssetDirectory() / m_AssetPath;

        // 1. Open the .aeth wrapper file
        std::ifstream stream(fullPath, std::ios::binary);
        if (!stream) {
            AETHER_CORE_ERROR("TextureViewer: Could not open asset file {}", fullPath.string());
            return;
        }

        // 2. Read Header
        AssetHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        // 3. Parse JSON Body
        json meta;
        try {
            stream >> meta;
        }
        catch (const json::parse_error& e) {
            AETHER_CORE_ERROR("TextureViewer: JSON Parse Error: {}", e.what());
            return;
        }

        // 4. Extract Settings
        std::string sourceRel = meta.value("Source", "");
        std::string filter = meta.value("Filter", "Linear");

        m_IsPixelArt = (filter == "Nearest");

        // 5. Load the Actual Texture Source
        if (!sourceRel.empty())
        {
            // The Source path in JSON is relative to Asset Dir, so we resolve it too
            std::filesystem::path sourceFullPath = Project::GetAssetDirectory() / sourceRel;

            TextureSpecification spec;
            spec.MinFilter = m_IsPixelArt ? GL_NEAREST : GL_LINEAR;
            spec.MagFilter = m_IsPixelArt ? GL_NEAREST : GL_LINEAR;
            spec.WrapS = GL_REPEAT;
            spec.WrapT = GL_REPEAT;

            m_Texture = std::make_shared<Texture2D>(sourceFullPath.string(), spec);
        }
    }

    EditorSaveResult TextureViewerPanel::Save()
    {
        // Resolve to Absolute Path for writing
        std::filesystem::path fullPath = Project::GetAssetDirectory() / m_AssetPath;

        // 1. Read existing header to preserve UUID
        std::ifstream in(fullPath, std::ios::binary);
        if (!in) return EditorSaveResult::Failure;

        AssetHeader header;
        in.read(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        // Read existing JSON
        json meta;
        try {
            in >> meta;
        }
        catch (...) {
            return EditorSaveResult::Failure;
        }
        in.close();

        // 2. Update Metadata
        meta["Filter"] = m_IsPixelArt ? "Nearest" : "Linear";

        // 3. Write back to disk
        std::ofstream out(fullPath, std::ios::binary | std::ios::trunc);
        if (!out) return EditorSaveResult::Failure;

        out.write(reinterpret_cast<char*>(&header), sizeof(AssetHeader));

        std::string dump = meta.dump(4);
        out.write(dump.c_str(), dump.size());
        out.close();

        // 4. Reload Texture to reflect changes
        if (m_Texture) {
            std::filesystem::path sourceFullPath = m_Texture->GetPath();

            TextureSpecification spec;
            spec.MinFilter = m_IsPixelArt ? GL_NEAREST : GL_LINEAR;
            spec.MagFilter = m_IsPixelArt ? GL_NEAREST : GL_LINEAR;

            m_Texture = std::make_shared<Texture2D>(sourceFullPath.string(), spec);
        }

        SetDirty(false);
        return EditorSaveResult::Success;
    }

    void TextureViewerPanel::RenderContent()
    {
        // NEW: Set default size on first use (400x420)
        // This works because we are inside the window's Begin()/End() scope from the base class
        ImGui::SetWindowSize(ImVec2(400, 420), ImGuiCond_FirstUseEver);

        if (!m_Texture) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Texture source not found or invalid.");
            std::filesystem::path fullPath = Project::GetAssetDirectory() / m_AssetPath;
            ImGui::TextWrapped("Attempted: %s", fullPath.string().c_str());
            return;
        }

        RenderToolbar();
        ImGui::Separator();
        RenderPreview();
    }

    void TextureViewerPanel::RenderToolbar()
    {
        Theme theme;

        // NEW: Save Button
        ImGui::PushStyleColor(ImGuiCol_Button, theme.AccentPrimary);
        if (ImGui::Button("Save")) {
            Save();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(theme.TextMuted, "Zoom:");
        ImGui::SameLine();

        if (ImGui::Button("-")) m_Zoom = std::max(0.1f, m_Zoom - 0.1f);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(100);
        ImGui::SliderFloat("##zoom", &m_Zoom, 0.1f, 5.0f, "%.1fx");
        ImGui::SameLine();

        if (ImGui::Button("+")) m_Zoom = std::min(5.0f, m_Zoom + 0.1f);
        ImGui::SameLine();

        if (ImGui::Button("1:1")) m_Zoom = 1.0f;

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        bool oldState = m_IsPixelArt;
        if (ImGui::Checkbox("Pixel Art Preview", &m_IsPixelArt))
        {
            if (m_IsPixelArt != oldState) {
                SetDirty(true);
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Preview how this looks in a retro/pixel-art game (keeps edges sharp instead of blurry).");
        }
    }

    void TextureViewerPanel::RenderPreview()
    {
        ImGui::BeginChild("TextureCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        float width = (float)m_Texture->GetWidth() * m_Zoom;
        float height = (float)m_Texture->GetHeight() * m_Zoom;

        // Draw Checkerboard
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        ImU32 col1 = IM_COL32(60, 60, 60, 255);
        ImU32 col2 = IM_COL32(40, 40, 40, 255);
        float checkSize = 16.0f;

        int numX = (int)(width / checkSize) + 1;
        int numY = (int)(height / checkSize) + 1;

        for (int y = 0; y < numY; y++)
        {
            for (int x = 0; x < numX; x++)
            {
                float xPos = x * checkSize;
                float yPos = y * checkSize;

                if (xPos >= width || yPos >= height) continue;

                float xEnd = std::min(xPos + checkSize, width);
                float yEnd = std::min(yPos + checkSize, height);

                bool isWhite = (x + y) % 2 == 0;
                drawList->AddRectFilled(
                    ImVec2(p.x + xPos, p.y + yPos),
                    ImVec2(p.x + xEnd, p.y + yEnd),
                    isWhite ? col1 : col2
                );
            }
        }

        ImGui::Image((void*)(uintptr_t)m_Texture->GetRendererID(), { width, height }, { 0, 1 }, { 1, 0 });

        ImGui::EndChild();
    }
}