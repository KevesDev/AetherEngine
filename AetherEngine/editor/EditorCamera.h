#pragma once

#include <glm/glm.hpp>
#include "../../engine/core/AetherTime.h"
#include "../../engine/events/Event.h"
#include "../../engine/events/MouseEvent.h"

namespace aether {

    class EditorCamera {
    public:
        EditorCamera();

        void OnUpdate(TimeStep ts);
        void OnEvent(Event& e);

        // Resize behavior
        void SetViewportSize(float width, float height);

        // Matrices for Rendering
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_ViewMatrix; }

        // Getters for Debug/UI
        float GetZoom() const { return m_ZoomLevel; }
        const glm::vec3& GetPosition() const { return m_Position; }

    private:
        void RecalculateView();
        bool OnMouseScrolled(MouseScrolledEvent& e);

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::mat4 m_ViewMatrix;

        float m_AspectRatio = 1.777f;
        float m_ZoomLevel = 10.0f; // Orthographic Size
        float m_Rotation = 0.0f;   // 2D Rotation in degrees

        float m_MoveSpeed = 5.0f;
        float m_ZoomSpeed = 0.5f;
    };
}