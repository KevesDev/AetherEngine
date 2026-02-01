#pragma once

#include <glm/glm.hpp> // Vector and Matrix types
#include "Shader.h" // Our Shader Class

namespace aether {

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const glm::mat4& viewProjection);
        static void EndScene();

        // The Command: Draw a flat colored rectangle
        // position: where is it?
        // size: how big is it?
        // color: what color is it? (R, G, B, A)
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    };

}