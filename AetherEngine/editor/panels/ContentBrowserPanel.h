#pragma once
#include "EditorPanel.h"
#include "FileBrowser.h"
#include "../../engine/asset/AssetMetadata.h"
#include <filesystem>
#include <string>
#include <functional>

namespace aether {

    /**
     * Panel responsible for displaying the project's asset directory.
     * Handles file navigation, asset creation, and drag-and-drop operations.
     */
    class ContentBrowserPanel
    {
    public:
        using AssetCallback = std::function<void(const std::filesystem::path&)>;

        ContentBrowserPanel();

        void OnImGuiRender();

        // --- Wizard Triggers ---
        void TriggerCreateAsset(AssetType type);
        void TriggerImport();

        // --- Listeners ---
        void SetOnAssetOpenedCallback(const AssetCallback& callback) { m_OnAssetOpened = callback; }

        /**
         * Sets the visibility of raw source files (e.g., .png, .jpg) in the browser.
         * Controlled by Editor Settings.
         */
        void SetShowRawAssets(bool show) { m_ShowRawAssets = show; }

        /**
         * @brief Returns the current visibility state of raw source files.
         * Added to support the View menu toggle in EditorLayer.
         */
        bool GetShowRawAssets() const { return m_ShowRawAssets; }

        std::filesystem::path GetCurrentDirectory() const { return m_CurrentDirectory; }

    private:
        void RenderCreationModal();
        // Note: FileBrowser::Render is handled within OnImGuiRender

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;

        // --- Configuration ---
        bool m_ShowRawAssets = false; // By default, only .aeth assets are shown to reduce clutter

        // --- Creation Wizard State ---
        bool m_OpenCreationPopup = false;
        AssetType m_PendingAssetType = AssetType::None;
        char m_CreationBuffer[64] = "";
        std::string m_CreationErrorMessage;

        // --- Import Tool ---
        FileBrowser m_FileBrowser;

        AssetCallback m_OnAssetOpened;
    };
}