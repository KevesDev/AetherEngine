#pragma once

#include <string>
#include <filesystem>
#include <imgui.h>

namespace aether {

    enum class EditorSaveResult
    {
        Success,
        Failure,
        Cancelled
    };

    /**
     * AssetEditorPanel:
     * Base class for all asset-specific editors.
     * Manages the layout (Content View + Side Inspector), focus, and saving.
     */
    class AssetEditorPanel
    {
    public:
        AssetEditorPanel(const std::string& title, const std::filesystem::path& assetPath)
            : m_Title(title), m_AssetPath(assetPath) {
        }

        virtual ~AssetEditorPanel() = default;

        void OnImGuiRender();

        void SetFocus() { m_FocusRequested = true; }
        bool IsOpen() const { return m_IsOpen; }
        const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }

    protected:
        // Main view area (Left Panel). Implemented by derived class.
        virtual void RenderContent() = 0;

        // Tool/Settings area (Right Panel). Implemented by derived class.
        // Optional: Default implementation renders nothing.
        virtual void RenderInspector() {}

        virtual EditorSaveResult Save() { return EditorSaveResult::Success; }
        void SetDirty(bool dirty) { m_IsDirty = dirty; }

    protected:
        std::string m_Title;
        std::filesystem::path m_AssetPath;
        bool m_IsOpen = true;
        bool m_IsDirty = false;
        bool m_FocusRequested = false;

        ImVec2 m_DefaultWindowSize = { 600.0f, 400.0f };
    };
}