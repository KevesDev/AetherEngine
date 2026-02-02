#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "../core/Log.h"

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

namespace aether {

    struct Renderer2DStorage
    {
        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<Shader> FlatColorShader;
    };

    static Renderer2DStorage* s_Data = nullptr;

    void Renderer2D::Init()
    {
        AETHER_CORE_INFO("Renderer2D: Initializing...");
        s_Data = new Renderer2DStorage();

        // Phase 1: Vertex Array
        s_Data->QuadVertexArray = std::make_shared<VertexArray>();

        // Phase 2: Vertex Buffer
        float squareVertices[4 * 3] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        auto squareVB = std::make_shared<VertexBuffer>(squareVertices, static_cast<uint32_t>(sizeof(squareVertices)));
        squareVB->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        // Phase 3: Index Buffer
        uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
        auto squareIB = std::make_shared<IndexBuffer>(squareIndices, 6);
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        // Phase 4: Shader (CRITICAL FIX: Load from /engine/ mount point)
        // We moved the shader to EngineContent/shaders/FlatColor.glsl, which is mounted to /engine
        AETHER_CORE_INFO("Renderer2D: Loading FlatColor Shader...");
        s_Data->FlatColorShader = std::make_shared<Shader>("/engine/shaders/FlatColor.glsl", "/engine/shaders/FlatColor.glsl");

        AETHER_ASSERT(s_Data->FlatColorShader, "Renderer2D: Shader failed to initialize!");
        AETHER_CORE_INFO("Renderer2D: Initialized Successfully.");
    }

    void Renderer2D::Shutdown() {
        delete s_Data;
        s_Data = nullptr;
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection) {
        if (!s_Data) return;
        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetMat4("u_ViewProjection", &viewProjection[0][0]);
    }

    void Renderer2D::EndScene() {}

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        if (!s_Data) return;

        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetFloat4("u_Color", color.r, color.g, color.b, color.a);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        s_Data->FlatColorShader->SetMat4("u_Transform", &transform[0][0]);

        s_Data->QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data->QuadVertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}