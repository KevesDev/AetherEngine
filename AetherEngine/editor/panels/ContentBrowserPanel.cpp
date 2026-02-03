#include "ContentBrowserPanel.h"
#include "../../engine/project/Project.h"
#include "../../engine/core/Theme.h"
#include <imgui.h>

namespace aether {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        // Start at the project's asset root
        m_BaseDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_BaseDirectory;
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        Theme theme;

        // --- Navigation Header ---
        if (m_CurrentDirectory != m_BaseDirectory)
        {
            if (ImGui::Button("<- BACK"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
            ImGui::SameLine();
        }

        ImGui::TextColored(theme.TextMuted, "Path: %s", m_CurrentDirectory.string().c_str());
        ImGui::Separator();

        // --- Grid/List of Files ---
        // For now, we use a simple selectable list. 
        // We will add icons and columns in a future pass.
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, m_BaseDirectory);
            std::string filenameString = relativePath.filename().string();

            if (directoryEntry.is_directory())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.AccentPrimary);
                if (ImGui::Selectable(("[DIR]  " + filenameString).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        m_CurrentDirectory /= path.filename();
                }
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::Text("       %s", filenameString.c_str());

                // --- Prepare for Drag & Drop ---
                if (ImGui::BeginDragDropSource())
                {
                    const wchar_t* itemPath = (const wchar_t*)path.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::Text("%s", filenameString.c_str());
                    ImGui::EndDragDropSource();
                }
            }
        }

        ImGui::End();
    }
}