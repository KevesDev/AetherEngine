#pragma once
#include "../../engine/core/Layers/Layer.h"
#include "../../engine/renderer/Framebuffer.h" 
#include "../panels/SceneHierarchyPanel.h"
#include "../panels/InspectorPanel.h"
#include "../panels/ContentBrowserPanel.h"
#include "../panels/AssetEditorPanel.h"
#include "../EditorCamera.h"
#include "../../engine/vendor/json.hpp" 
#include "../../engine/events/ApplicationEvent.h" 
#include "../../engine/events/KeyEvent.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory> 
#include <fstream>
#include <filesystem>

namespace aether {

	// --- Editor Settings Structure ---
    // Container for Editor-specific user preferences.
    // Serializes to 'editor_config.json'.
	// ----------------------------------
    struct EditorSettings {
        bool ShowRawAssets = false;

        void Serialize(const std::filesystem::path& filepath) {
            nlohmann::json j;
            j["ShowRawAssets"] = ShowRawAssets;
            std::ofstream o(filepath);
            if (o) o << j.dump(4);
        }

        void Deserialize(const std::filesystem::path& filepath) {
            std::ifstream i(filepath);
            if (i) {
                nlohmann::json j;
                i >> j;
                ShowRawAssets = j.value("ShowRawAssets", false);
            }
        }
    };

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(TimeStep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& e) override;

    private:
        void EnsureLayout(unsigned int dockspace_id);
        void OpenAsset(const std::filesystem::path& path);

        // --- Event Handlers ---
        bool OnFileDrop(FileDropEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);

        // --- Settings Management ---
        void LoadSettings();
        void SaveSettings();
        void RenderPreferencesPanel();

    private:
        bool m_IsFirstFrame = true;
        std::string m_IniFilePath;
        std::string m_ConfigFilePath;

        // --- Editor State ---
        EditorSettings m_Settings;
        bool m_ShowPreferences = false;

        // --- Viewport State ---
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

        // --- Panels ---
        SceneHierarchyPanel m_SceneHierarchyPanel;
        InspectorPanel m_InspectorPanel;
        ContentBrowserPanel m_ContentBrowserPanel;

        // --- Asset Tools ---
        std::vector<std::shared_ptr<AssetEditorPanel>> m_AssetEditors;

        // --- Editor Tools ---
        EditorCamera m_EditorCamera;

        // --- Rendering ---
        std::shared_ptr<Framebuffer> m_Framebuffer;
    };
}