#include "ContentBrowserPanel.h"
#include "../EditorResources.h"
#include "../../engine/project/Project.h"
#include "../../engine/core/Theme.h"
#include "../../engine/asset/AssetManager.h"
#include "../commands/CommandHistory.h"     // Command System Integration
#include "../commands/DeleteAssetCommand.h" // Concrete Command
#include <imgui.h>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <vector>

namespace aether {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_BaseDirectory = Project::GetAssetDirectory();
        m_CurrentDirectory = m_BaseDirectory;

        m_FileBrowser.SetTitle("Import Asset");
        m_FileBrowser.SetSearchHint("Search supported files...");
        m_FileBrowser.SetFileExtensions(AssetManager::GetImportableExtensions());
    }

    void ContentBrowserPanel::TriggerCreateAsset(AssetType type)
    {
        m_PendingAssetType = type;
        memset(m_CreationBuffer, 0, sizeof(m_CreationBuffer));
        m_CreationErrorMessage = "";
        m_OpenCreationPopup = true;
    }

    void ContentBrowserPanel::TriggerImport()
    {
        // Defer the open call to the main update loop to avoid ID Stack issues
        m_ShowImportPopup = true;
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");
        Theme theme;

        // --- TOOLBAR ---
        if (m_CurrentDirectory != m_BaseDirectory)
        {
            if (ImGui::Button("<- Back"))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            ImGui::SameLine();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, theme.AccentCyan);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
        if (ImGui::Button("Import Asset")) TriggerImport();
        ImGui::PopStyleColor(2);

        ImGui::Separator();

        // --- CONTEXT MENU (Global) ---
        if (ImGui::BeginPopupContextWindow("ContentBrowserContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("New Scene...")) TriggerCreateAsset(AssetType::Scene);
            if (ImGui::MenuItem("New Logic Graph...")) TriggerCreateAsset(AssetType::LogicGraph);
            ImGui::Separator();
            if (ImGui::MenuItem("Import...")) TriggerImport();
            ImGui::EndPopup();
        }

        // --- GRID RENDERING ---
        static float padding = 16.0f;
        static float thumbnailSize = 64.0f;
        float cellSize = thumbnailSize + padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        // Cache importable extensions for the filter loop
        auto importableExts = AssetManager::GetImportableExtensions();

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            std::string filenameString = path.filename().string();
            std::string ext = path.extension().string();

            // --- FILTERING LOGIC ---
            // If "Show Raw Assets" is disabled, we skip importable files (png, jpg)
            if (!m_ShowRawAssets && !directoryEntry.is_directory())
            {
                std::string lowerExt = ext;
                std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

                bool isRawSource = false;
                for (const auto& imp : importableExts) {
                    if (imp == lowerExt) { isRawSource = true; break; }
                }

                if (isRawSource) continue; // Skip rendering this item
            }
            // -----------------------

            ImGui::PushID(filenameString.c_str());

            std::shared_ptr<Texture2D> icon = EditorResources::FileIcon;
            ImVec4 iconTint = { 1, 1, 1, 1 };
            AssetType fileType = AssetType::None;

            if (directoryEntry.is_directory())
            {
                icon = EditorResources::FolderIcon;
            }
            else
            {
                auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

                if (AssetManager::HasAsset(relativePath))
                {
                    fileType = AssetManager::GetMetadata(relativePath).Type;
                    if (fileType == AssetType::Texture2D) icon = EditorResources::TextureIcon;
                }
                else
                {
                    // Check if importable
                    auto importable = AssetManager::GetImportableExtensions();
                    std::string lowerExt = ext;
                    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
                    bool isImportable = false;
                    for (const auto& imp : importable) { if (imp == lowerExt) isImportable = true; }

                    if (isImportable) iconTint = { 1.0f, 1.0f, 1.0f, 0.5f };
                }
            }

            if (!icon) icon = EditorResources::FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton("asset_btn", (ImTextureID)(uintptr_t)icon->GetRendererID(),
                { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 },
                ImVec4(0, 0, 0, 0), iconTint);
            ImGui::PopStyleColor();

            // --- ITEM CONTEXT MENU (Right-Click on Asset) ---
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
                    CommandHistory::Execute(std::make_shared<DeleteAssetCommand>(relativePath));
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource())
            {
                std::string relativePathStr = std::filesystem::relative(path, m_BaseDirectory).generic_string();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", relativePathStr.c_str(), relativePathStr.size() + 1);
                ImGui::Text("%s", filenameString.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
                else if (fileType != AssetType::None && m_OnAssetOpened)
                    m_OnAssetOpened(path);
            }

            ImGui::TextWrapped("%s", filenameString.c_str());
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);

        RenderCreationModal();

        // Handle deferred popup opening (Fixes Context Menu issue)
        if (m_ShowImportPopup) {
            m_FileBrowser.Open();
            m_ShowImportPopup = false;
        }

        // FILE BROWSER
        std::filesystem::path selectedPath;
        if (m_FileBrowser.Render(selectedPath))
        {
            std::string filename = selectedPath.filename().string();
            std::filesystem::path destPath = m_CurrentDirectory / filename;

            try {
                if (!std::filesystem::exists(destPath)) {
                    std::filesystem::copy_file(selectedPath, destPath);
                    AssetManager::ImportSourceFile(destPath);
                }
            }
            catch (std::filesystem::filesystem_error& e) { /* Log error if necessary */ }
        }

        ImGui::End();
    }

    void ContentBrowserPanel::RenderCreationModal()
    {
        if (m_OpenCreationPopup) { ImGui::OpenPopup("Create New Asset"); m_OpenCreationPopup = false; }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Create New Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            Theme theme;
            ImGui::Text("Enter a name for the new asset:");
            ImGui::InputText("##assetname", m_CreationBuffer, sizeof(m_CreationBuffer));
            if (!m_CreationErrorMessage.empty()) ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", m_CreationErrorMessage.c_str());
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, theme.AccentPrimary);
            if (ImGui::Button("Create", ImVec2(120, 0))) {
                std::string filename(m_CreationBuffer);
                if (filename.empty()) m_CreationErrorMessage = "Filename cannot be empty.";
                else { AssetManager::CreateAsset(filename, m_CurrentDirectory, m_PendingAssetType); ImGui::CloseCurrentPopup(); }
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }
}