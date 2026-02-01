#pragma once
#include <glm/glm.hpp>

/* An orthographic camera class for 2D rendering.
* Sets up an orthographic projection matrix and provides a view-projection matrix.
*/
namespace aether {
    class OrthographicCamera {
    public:
        // Sets up the projection (e.g., 0 to 1280 wide, 720 to 0 high)
        OrthographicCamera(float left, float right, float bottom, float top);

        void SetProjection(float left, float right, float bottom, float top);

        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix;
    };
}