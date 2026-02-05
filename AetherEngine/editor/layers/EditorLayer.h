#pragma once

#include "../../engine/core/Layers/Layer.h"
#include "../../engine/renderer/Framebuffer.h"
#include "../../engine/renderer/Texture.h"
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"
#include "../../engine/events/ApplicationEvent.h"
#include "../../engine/events/KeyEvent.h"
#include "../../engine/events/MouseEvent.h"
#include "../../engine/input/Input.h"
#include "../../engine/input/KeyCodes.h"
#include "../../engine/core/AetherTime.h"

#include "../panels/SceneHierarchyPanel.h"
#include "../EditorCamera.h"
#include "../panels/NetworkSettingsPanel.h"
#include "PerformanceOverlay.h"

#include <glm/glm.hpp>
#include <filesystem>
#include <memory>

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
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();

        // Docking infrastructure - establishes DockSpace for layout restoration
        void SetupDockSpace();

    private:
        enum class SceneState
        {
            Edit = 0,
            Play = 1
        };
        SceneState m_SceneState = SceneState::Edit;

        std::shared_ptr<Scene> m_ActiveScene;
        std::filesystem::path m_EditorScenePath;

        SceneHierarchyPanel m_SceneHierarchyPanel;
        NetworkSettingsPanel m_NetworkSettingsPanel;

        std::shared_ptr<Framebuffer> m_Framebuffer;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec2 m_ViewportBounds[2];

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;

        EditorCamera m_EditorCamera;

        // Editor-only performance overlay, scoped to the viewport.
        PerformanceOverlay m_PerformanceOverlay;

        std::shared_ptr<Texture2D> m_CheckerboardTexture;

        Entity m_HoveredEntity;
    };

}