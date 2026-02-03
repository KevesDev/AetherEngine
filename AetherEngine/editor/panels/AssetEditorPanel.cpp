#include "AssetEditorPanel.h"
#include "../../engine/core/Theme.h"
#include "../../engine/core/Log.h"
#include <imgui.h>

namespace aether {

    AssetEditorPanel::AssetEditorPanel(const std::string& title, const std::filesystem::path& assetPath)
        : m_Title(title), m_AssetPath(assetPath)
    {
    }

    void AssetEditorPanel::OnImGuiRender()
    {
        // If the panel is closed and we aren't resolving a save prompt, stop rendering.
        if (!m_IsOpen && !m_ShowUnsavedModal) return;

        // 1. Construct the Unique Window Title
        // Format: "ToolName - FileName *"
        // The "###Path" ensures ImGui tracks this window by file path, not by the changing title.
        std::string filename = m_AssetPath.filename().string();
        std::string displayTitle = m_Title + " - " + filename;
        if (m_IsDirty) displayTitle += " *";

        std::string uniqueID = displayTitle + "###" + m_AssetPath.string();

        // 2. Handle Focus Request
        if (m_RequestFocus) {
            ImGui::SetNextWindowFocus();
            m_RequestFocus = false;
        }

        // 3. Render Window Shell
        // We do not add the NoCollapse flag to allow users to minimize tools.
        // We rely on ImGui's docking system to handle placement.
        bool openParams = true;
        if (ImGui::Begin(uniqueID.c_str(), &openParams))
        {
            RenderContent();
        }
        ImGui::End();

        // 4. Handle Close Event (The 'X' button)
        if (!openParams) {
            if (m_IsDirty) {
                // If dirty, we DO NOT close. We open the modal instead.
                m_ShowUnsavedModal = true;
                // We keep m_IsOpen true for one more frame to prevent flickering
                m_IsOpen = true;
            }
            else {
                // Clean close
                m_IsOpen = false;
            }
        }

        // 5. Render Modal (if active)
        if (m_ShowUnsavedModal) {
            RenderUnsavedChangesModal();
        }
    }

    void AssetEditorPanel::RenderUnsavedChangesModal()
    {
        // Ensure the popup is queued to open
        if (!ImGui::IsPopupOpen("Unsaved Changes")) {
            ImGui::OpenPopup("Unsaved Changes");
        }

        // Center the modal on the main viewport
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // AlwaysAutoResize fits the window to the buttons/text
        if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            Theme theme;

            ImGui::Text("Save changes to '%s'?", m_AssetPath.filename().string().c_str());
            ImGui::TextColored(theme.TextMuted, "If you don't save, your changes will be lost.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // --- SAVE BUTTON ---
            ImGui::PushStyleColor(ImGuiCol_Button, theme.AccentPrimary);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.AccentSecondary);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.AccentCyan);

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                EditorSaveResult result = Save();
                if (result == EditorSaveResult::Success) {
                    m_IsOpen = false;
                    m_ShowUnsavedModal = false;
                    ImGui::CloseCurrentPopup();
                }
                else {
                    AETHER_CORE_ERROR("Failed to save asset during close operation!");
                    // We stay open if save failed, so the user doesn't lose work
                }
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            // --- DON'T SAVE BUTTON ---
            if (ImGui::Button("Don't Save", ImVec2(120, 0)))
            {
                m_IsOpen = false;
                m_ShowUnsavedModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            // --- CANCEL BUTTON ---
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                m_ShowUnsavedModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}