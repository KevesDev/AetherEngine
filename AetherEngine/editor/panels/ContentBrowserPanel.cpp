#include "ContentBrowserPanel.h"
#include "../EditorResources.h"
#include "../../engine/project/Project.h"
#include "../../engine/core/Theme.h"
#include <imgui.h>

namespace aether {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_BaseDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_BaseDirectory;
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        Theme theme;
        static float padding = 16.0f;
        static float thumbnailSize = 64.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        if (m_CurrentDirectory != m_BaseDirectory)
        {
            if (ImGui::Button("<- Back"))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            ImGui::Separator();
        }

        ImGui::Columns(columnCount, 0, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            // Push a unique ID for every item to prevent ImGui state collisions
            ImGui::PushID(filenameString.c_str());

            // Determine icon with fallback safety
            std::shared_ptr<Texture2D> icon = directoryEntry.is_directory() ? EditorResources::FolderIcon : EditorResources::FileIcon;
            if (!icon) icon = EditorResources::FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

            // Guard against nullptr renderer IDs
            if (icon)
            {
                // Corrected ImageButton signature for modern ImGui
                ImGui::ImageButton("asset_icon", (ImTextureID)(uintptr_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
            }
            else
            {
                ImGui::Button("??", { thumbnailSize, thumbnailSize });
            }

            if (ImGui::BeginDragDropSource())
            {
                std::string relativePath = std::filesystem::relative(path, m_BaseDirectory).generic_string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", relativePath.c_str(), relativePath.size() + 1);
                ImGui::Text("%s", filenameString.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }

            ImGui::TextWrapped("%s", filenameString.c_str());
            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::End();
    }
}