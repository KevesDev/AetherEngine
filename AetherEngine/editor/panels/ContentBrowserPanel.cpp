#include "ContentBrowserPanel.h"
#include "../EditorResources.h"
#include "../../engine/project/Project.h"
#include "../../engine/core/Theme.h"
#include "../../engine/asset/AssetManager.h"
#include "../commands/CommandHistory.h"     
#include "../commands/DeleteAssetCommand.h" 
#include "../../engine/core/Log.h"

#include <imgui.h>
#include <iostream>

namespace aether {

    ContentBrowserPanel::ContentBrowserPanel()
        : m_BaseDirectory(Project::GetAssetDirectory()),
        m_CurrentDirectory(m_BaseDirectory),
        m_ShowRawAssets(false)
    {
        // Initialize the FileBrowser properties
        m_FileBrowser.SetTitle("Import Asset");
    }

    void ContentBrowserPanel::TriggerCreateAsset(AssetType type)
    {
        m_PendingAssetType = type;
        m_OpenCreationPopup = true;

        // Reset creation buffer and error state
        memset(m_CreationBuffer, 0, sizeof(m_CreationBuffer));
        m_CreationErrorMessage.clear();
    }

    void ContentBrowserPanel::TriggerImport()
    {
        // Open the file browser popup for importing raw assets
        m_FileBrowser.Open();
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        // --- 1. Render Sub-Components (Popups) ---

        RenderCreationModal();

        // File Browser Logic (Immediate Mode)
        // Render() returns true if the user selected a file and clicked OK.
        std::filesystem::path importPath;
        if (m_FileBrowser.Render(importPath))
        {
            if (!importPath.empty())
            {
                AssetManager::ImportSourceFile(importPath);
                AETHER_CORE_INFO("ContentBrowser: Imported asset from {}", importPath.string());
            }
        }

        // --- 2. Render Main Window ---

        ImGui::Begin("Content Browser");

        // Top Bar: Directory Navigation
        if (m_CurrentDirectory != std::filesystem::path(Project::GetAssetDirectory()))
        {
            if (ImGui::Button("<- Back"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
            ImGui::SameLine();
        }
        ImGui::TextDisabled("%s", m_CurrentDirectory.string().c_str());

        // --- Content Grid ---

        static float padding = 16.0f;
        static float thumbnailSize = 96.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        if (ImGui::BeginTable("ContentBrowserTable", columnCount))
        {
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
            {
                const auto& path = directoryEntry.path();
                std::string filenameString = path.filename().string();
                bool isDirectory = directoryEntry.is_directory();

                // Filter - Visibility Logic
                // If "Show Raw Sources" is disabled, we hide any file that isn't an engine asset (.aeth).
                // Directories are always shown.
                if (!m_ShowRawAssets && !isDirectory)
                {
                    if (path.extension() != ".aeth")
                    {
                        continue;
                    }
                }

                ImGui::PushID(filenameString.c_str());
                ImGui::TableNextColumn();

                // --- Icon Selection ---
                std::shared_ptr<Texture2D> icon = isDirectory ? EditorResources::FolderIcon : EditorResources::FileIcon;

                // If it's an .aeth file, check the metadata to determine the specific asset type icon (Texture, etc.)
                if (!isDirectory && path.extension() == ".aeth")
                {
                    AssetType type = AssetManager::GetAssetTypeFromExtension(path);
                    if (type == AssetType::Texture2D) icon = EditorResources::TextureIcon;
                }

                // --- Thumbnail Button ---
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                // Cast to uintptr_t first to handle 64-bit pointers correctly for ImGui
                uint64_t texID = icon ? (uint64_t)icon->GetRendererID() : 0;

                // Using "##btn" as the string ID to satisfy the specific ImageButton overload
                ImGui::ImageButton("##btn", (ImTextureID)(uintptr_t)texID, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                // Drag & Drop Source
                if (ImGui::BeginDragDropSource())
                {
                    const wchar_t* itemPath = path.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }

                ImGui::PopStyleColor();

                // --- Interaction ---
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (isDirectory)
                    {
                        m_CurrentDirectory /= path.filename();
                    }
                    else if (m_OnAssetOpened)
                    {
                        m_OnAssetOpened(path);
                    }
                }

                // --- Context Menu ---
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        // [Fix] Use Execute() to push the command to the history stack
                        CommandHistory::Execute(std::make_shared<DeleteAssetCommand>(path));
                    }
                    ImGui::EndPopup();
                }

                ImGui::TextWrapped("%s", filenameString.c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        // --- Footer: Settings ---
        ImGui::Separator();
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
    }

    void ContentBrowserPanel::RenderCreationModal()
    {
        if (m_OpenCreationPopup)
        {
            ImGui::OpenPopup("Create New Asset");
            m_OpenCreationPopup = false;
        }

        if (ImGui::BeginPopupModal("Create New Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter a name for the new asset:");
            ImGui::InputText("##AssetName", m_CreationBuffer, sizeof(m_CreationBuffer));

            if (!m_CreationErrorMessage.empty())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", m_CreationErrorMessage.c_str());
            }

            ImGui::Separator();

            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                std::string name = m_CreationBuffer;
                if (!name.empty())
                {
                    AssetManager::CreateAsset(name, m_CurrentDirectory, m_PendingAssetType);
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    m_CreationErrorMessage = "Name cannot be empty.";
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}