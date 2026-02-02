#pragma once
#include "../../engine/core/Layers/Layer.h"
#include "../../engine/renderer/Framebuffer.h" // Required for m_Framebuffer
#include "../panels/SceneHierarchyPanel.h"
#include "../panels/InspectorPanel.h"
#include "../EditorCamera.h"
#include <string>
#include <glm/glm.hpp>
#include <memory> // Required for std::shared_ptr

namespace aether {

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

    private:
        bool m_IsFirstFrame = true;
        std::string m_IniFilePath;

        // --- Viewport State (Production Guards) ---
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f }; // Tracks the panel size for resizing

        // --- Panels ---
        SceneHierarchyPanel m_SceneHierarchyPanel;
        InspectorPanel m_InspectorPanel;

        // --- Editor Tools ---
        EditorCamera m_EditorCamera;

        // --- Rendering ---
        std::shared_ptr<Framebuffer> m_Framebuffer; // The off-screen render target
    };
}