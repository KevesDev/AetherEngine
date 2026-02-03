#pragma once
#include "EditorPanel.h"
#include "../../engine/asset/AssetMetadata.h"
#include <filesystem>
#include <string>

namespace aether {

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

        // Opens the Modal Wizard. Can be called by Context Menu or Main Menu Bar.
        void TriggerCreateAsset(AssetType type);

    private:
        // Draws the blocking modal window if active
        void RenderCreationModal();

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;

        // Wizard State
        bool m_OpenCreationPopup = false;     // Triggers the OpenPopup call
        AssetType m_PendingAssetType = AssetType::None;
        char m_CreationBuffer[64] = "";       // Fixed buffer for input (max 64 chars)
        std::string m_CreationErrorMessage;   // Live validation feedback
    };
}