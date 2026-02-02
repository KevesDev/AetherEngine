#pragma once
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace aether {
	// Utility class for camera projection and view matrix calculations
    class CameraUtils {
    public:
        // Standard Orthographic Projection Math
        static glm::mat4 CalculateProjection(float size, float aspectRatio, float nearClip, float farClip) {
            float left = -size * aspectRatio * 0.5f;
            float right = size * aspectRatio * 0.5f;
            float bottom = -size * 0.5f;
            float top = size * 0.5f;

            return glm::ortho(left, right, bottom, top, nearClip, farClip);
        }

        // Standard View Matrix Math (Inverse of Transform)
        static glm::mat4 CalculateView(const glm::vec3& position, float rotation) {
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));

            return glm::inverse(transform);
        }
    };
}