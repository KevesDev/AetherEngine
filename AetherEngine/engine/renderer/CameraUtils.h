#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace aether {

    class CameraUtils {
    public:
        // Standard Orthographic Projection (The default for 2D)
        static glm::mat4 CalculateOrthographic(float size, float aspectRatio, float nearClip, float farClip) {
            float left = -size * aspectRatio * 0.5f;
            float right = size * aspectRatio * 0.5f;
            float bottom = -size * 0.5f;
            float top = size * 0.5f;

            return glm::ortho(left, right, bottom, top, nearClip, farClip);
        }

        // Perspective Projection (Used for 2.5D Parallax / VFX layers)
        static glm::mat4 CalculatePerspective(float fovRadians, float aspectRatio, float nearClip, float farClip) {
            return glm::perspective(fovRadians, aspectRatio, nearClip, farClip);
        }

        // View Matrix (Inverse of Camera Transform)
        static glm::mat4 CalculateView(const glm::vec3& position, float rotation) {
            // 2D Rotation around Z axis
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));

            return glm::inverse(transform);
        }
    };
}