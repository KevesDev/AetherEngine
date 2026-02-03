#include "ContentBrowserPanel.h"
#include "../EditorResources.h"
#include "../../engine/project/Project.h"
#include "../../engine/core/Theme.h"
#include "../../engine/asset/AssetManager.h"
#include <imgui.h>
#include <cstring>

namespace aether {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_BaseDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_BaseDirectory;
    }

    void ContentBrowserPanel::TriggerCreateAsset(AssetType type)
    {
        m_PendingAssetType = type;
        memset(m_CreationBuffer, 0, sizeof(m_CreationBuffer)); // Clear buffer
        m_CreationErrorMessage = "";
        m_OpenCreationPopup = true; // Signal to open on next frame
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        Theme theme; // Use our unified theme system
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

        // --- CONTEXT MENU ---
        // Right-clicking empty space triggers the creation wizard
        if (ImGui::BeginPopupContextWindow("ContentBrowserContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("New Scene..."))
            {
                TriggerCreateAsset(AssetType::Scene);
            }
            if (ImGui::MenuItem("New Logic Graph..."))
            {
                TriggerCreateAsset(AssetType::LogicGraph);
            }
            ImGui::EndPopup();
        }

        // --- GRID RENDERING ---
        ImGui::Columns(columnCount, 0, false);
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());

            std::shared_ptr<Texture2D> icon = EditorResources::FileIcon;
            ImVec4 iconTint = { 1, 1, 1, 1 };

            if (directoryEntry.is_directory())
            {
                icon = EditorResources::FolderIcon;
            }
            else
            {
                // Registry Check: Is this a valid Aether Asset?
                auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
                if (AssetManager::HasAsset(relativePath))
                {
                    AssetType type = AssetManager::GetMetadata(relativePath).Type;
                    // Assign icon based on type (We can expand this list later)
                    if (type == AssetType::Texture2D) icon = EditorResources::TextureIcon;
                    // else if (type == AssetType::Scene) icon = EditorResources::SceneIcon; 
                }
                else
                {
                    // Tint red-ish if not imported/valid
                    iconTint = { 1.0f, 0.7f, 0.7f, 1.0f };
                }
            }

            if (!icon) icon = EditorResources::FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton("asset_btn", (ImTextureID)(uintptr_t)icon->GetRendererID(),
                { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 },
                ImVec4(0, 0, 0, 0), iconTint);

            if (ImGui::BeginDragDropSource())
            {
                std::string relativePathStr = std::filesystem::relative(path, m_BaseDirectory).generic_string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", relativePathStr.c_str(), relativePathStr.size() + 1);
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

        // --- DRAW WIZARD (Modal) ---
        RenderCreationModal();

        ImGui::End();
    }

    void ContentBrowserPanel::RenderCreationModal()
    {
        // Check if we need to open the popup (signaled by TriggerCreateAsset)
        if (m_OpenCreationPopup)
        {
            ImGui::OpenPopup("Create Asset Wizard");
            m_OpenCreationPopup = false;
        }

        // Center the wizard
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // Modal = Blocking. Background becomes dimmed.
        if (ImGui::BeginPopupModal("Create Asset Wizard", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            Theme theme; // Access our unified colors

            ImGui::Text("Create New %s", (m_PendingAssetType == AssetType::Scene ? "Scene" : "Logic Graph"));
            ImGui::Separator();

            // Location Info (Using Theme Muted Text)
            std::string location = "Location: " + std::filesystem::relative(m_CurrentDirectory, Project::GetAssetDirectory()).string();
            ImGui::TextColored(theme.TextMuted, "%s", location.c_str());
            ImGui::Spacing();

            // Input Field
            // ImGuiInputTextFlags_EnterReturnsTrue allows pressing 'Enter' to confirm
            bool enterPressed = ImGui::InputText("Filename", m_CreationBuffer, sizeof(m_CreationBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

            // Live Validation
            bool isValid = true;
            m_CreationErrorMessage = "";
            std::string nameStr = m_CreationBuffer;

            if (nameStr.empty()) {
                isValid = false;
            }
            else {
                if (nameStr.find(' ') != std::string::npos) {
                    m_CreationErrorMessage = "Filename cannot contain spaces.";
                    isValid = false;
                }
                else if (nameStr.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") != std::string::npos) {
                    m_CreationErrorMessage = "Alphanumeric, '-', and '_' only.";
                    isValid = false;
                }

                std::string ext = ".aeth";
                if (std::filesystem::exists(m_CurrentDirectory / (nameStr + ext))) {
                    m_CreationErrorMessage = "File already exists!";
                    isValid = false;
                }
            }

            // Error Message (Red is universal, so we keep it distinct from the theme for safety)
            if (!m_CreationErrorMessage.empty())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_CreationErrorMessage.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Buttons
            if (isValid)
            {
                // Valid: Use Theme Primary Accent (Lavender)
                ImGui::PushStyleColor(ImGuiCol_Button, theme.AccentPrimary);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.AccentSecondary);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.AccentCyan);
            }
            else
            {
                // Invalid: Use Panel Background (looks disabled/flat)
                ImGui::PushStyleColor(ImGuiCol_Button, theme.PanelBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.PanelBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.PanelBg);
            }

            // The "Create" Button
            if (ImGui::Button("Create", ImVec2(120, 0)) || (isValid && enterPressed))
            {
                if (isValid)
                {
                    AssetManager::CreateAsset(m_CreationBuffer, m_CurrentDirectory, m_PendingAssetType);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::PopStyleColor(3); // Pop the 3 button colors
            ImGui::SameLine();

            // The "Cancel" Button
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}