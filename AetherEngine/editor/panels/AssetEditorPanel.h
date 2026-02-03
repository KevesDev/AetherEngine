#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <imgui.h>
#include "../../engine/core/UUID.h"

namespace aether {

    // Defines the outcome of a Save operation
    enum class EditorSaveResult
    {
        Success,
        Failure,
        Cancelled
    };

    class AssetEditorPanel
    {
    public:
        AssetEditorPanel(const std::string& title, const std::filesystem::path& assetPath);
        virtual ~AssetEditorPanel() = default;

        // Main update loop called by EditorLayer
        void OnImGuiRender();

        // Requests the window to move to the front of the docking stack
        void SetFocus() { m_RequestFocus = true; }

        // Forces the window to close (ignoring dirty state), used during engine shutdown
        void ForceClose() { m_IsOpen = false; }

        // State Queries
        bool IsOpen() const { return m_IsOpen; }
        bool IsDirty() const { return m_IsDirty; }
        const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }

    protected:
        // Contract: Derived classes MUST implement the UI rendering logic here
        virtual void RenderContent() = 0;

        // Contract: Derived classes MUST implement the serialization logic here
        // Returns Success if the file was written, Failure otherwise.
        virtual EditorSaveResult Save() = 0;

        // Helper for child classes to mark the asset as modified
        void SetDirty(bool dirty = true) { m_IsDirty = dirty; }

    private:
        void RenderUnsavedChangesModal();

    protected:
        std::filesystem::path m_AssetPath;
        std::string m_Title; // The tool name (e.g. "Texture Viewer")

        bool m_IsOpen = true;
        bool m_IsDirty = false;
        bool m_RequestFocus = true; // Focus on open

        // Internal state for the modal popup
        bool m_ShowUnsavedModal = false;
    };
}