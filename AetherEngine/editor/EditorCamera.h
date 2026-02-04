#pragma once

#include <glm/glm.hpp>
#include "../engine/core/AetherTime.h"
#include "../engine/events/Event.h"
#include "../engine/events/MouseEvent.h"

namespace aether {

    /**
     * EditorCamera: Free-flying camera for scene editing.
     * Supports WASD movement, Q/E zoom, and mouse scroll zoom.
     */
    class EditorCamera {
    public:
        EditorCamera();
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void OnUpdate(TimeStep ts);
        void OnEvent(Event& e);

        void SetViewportSize(float width, float height);

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_ViewMatrix; }

        float GetZoom() const { return m_ZoomLevel; }
        const glm::vec3& GetPosition() const { return m_Position; }

    private:
        void RecalculateView();
        bool OnMouseScrolled(MouseScrolledEvent& e);

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::mat4 m_ViewMatrix;

        float m_FOV = 45.0f;
        float m_AspectRatio = 1.778f;
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;

        float m_ZoomLevel = 10.0f;
        float m_Rotation = 0.0f;

        float m_MoveSpeed = 5.0f;
        float m_ZoomSpeed = 0.5f;
    };
}