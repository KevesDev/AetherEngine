#pragma once

#include "../../engine/core/Layers/Layer.h"
#include "../../engine/renderer/Framebuffer.h"
#include "../../engine/renderer/Texture.h"
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"
#include "../../engine/events/ApplicationEvent.h"
#include "../../engine/events/KeyEvent.h"
#include "../../engine/events/MouseEvent.h"

#include "../panels/SceneHierarchyPanel.h"
#include "../EditorCamera.h"

#include <glm/glm.hpp>
#include <filesystem>
#include <memory>

namespace aether {

    /**
     * EditorLayer
     * The primary application layer for the Aether Engine Editor.
     * Manages the Scene lifecycle, Editor Viewport, and Tool Panels.
     */
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
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();

    private:
        // Editor State Machine
        enum class SceneState
        {
            Edit = 0,
            Play = 1
        };
        SceneState m_SceneState = SceneState::Edit;

        // Scene Context
        // The editor owns the active scene.
        std::shared_ptr<Scene> m_ActiveScene;
        std::filesystem::path m_EditorScenePath;

        // UI Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;

        // Viewport & Rendering Resources
        std::shared_ptr<Framebuffer> m_Framebuffer;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec2 m_ViewportBounds[2];

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;

        // Editor Camera
        // Used exclusively for navigation in 'Edit' mode.
        // In 'Play' mode, the scene uses its own primary camera entity.
        EditorCamera m_EditorCamera;

        // Internal Resources
        std::shared_ptr<Texture2D> m_CheckerboardTexture;

        // Entity Selection
        Entity m_HoveredEntity;
    };

}