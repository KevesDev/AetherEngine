#include "EditorCamera.h"
#include "../../engine/input/Input.h"
#include "../../engine/input/KeyCodes.h"
#include "../../engine/renderer/CameraUtils.h"
#include <algorithm> 

namespace aether {

    EditorCamera::EditorCamera()
    {
        RecalculateView();
    }

    void EditorCamera::OnUpdate(TimeStep ts)
    {
        glm::vec3 moveDelta(0.0f);

        if (Input::IsKeyPressed(Key::W)) moveDelta.y += m_MoveSpeed * ts;
        if (Input::IsKeyPressed(Key::S)) moveDelta.y -= m_MoveSpeed * ts;
        if (Input::IsKeyPressed(Key::A)) moveDelta.x -= m_MoveSpeed * ts;
        if (Input::IsKeyPressed(Key::D)) moveDelta.x += m_MoveSpeed * ts;

        if (moveDelta.x != 0.0f || moveDelta.y != 0.0f) {
            m_Position += moveDelta;
            RecalculateView();
        }

        if (Input::IsKeyPressed(Key::Q)) {
            m_ZoomLevel += m_ZoomSpeed * 10.0f * ts;
            m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        }
        if (Input::IsKeyPressed(Key::E)) {
            m_ZoomLevel -= m_ZoomSpeed * 10.0f * ts;
            m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        }
    }

    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        // Use Correct Macro Name
        dispatcher.Dispatch<MouseScrolledEvent>(AETHER_BIND_EVENT_FN(EditorCamera::OnMouseScrolled));
    }

    bool EditorCamera::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_ZoomLevel -= e.GetYOffset() * m_ZoomSpeed;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        return false;
    }

    void EditorCamera::SetViewportSize(float width, float height)
    {
        if (height > 0) {
            m_AspectRatio = width / height;
        }
    }

    glm::mat4 EditorCamera::GetProjectionMatrix() const
    {
        return CameraUtils::CalculateOrthographic(m_ZoomLevel, m_AspectRatio, -1.0f, 1.0f);
    }

    void EditorCamera::RecalculateView()
    {
        m_ViewMatrix = CameraUtils::CalculateView(m_Position, glm::radians(m_Rotation));
    }
}