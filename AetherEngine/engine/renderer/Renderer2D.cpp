#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "../core/Log.h"
#include <glm/gtc/matrix_transform.hpp> 

namespace aether {

    struct Renderer2DStorage
    {
        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<Shader> FlatColorShader;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init()
    {
        s_Data = new Renderer2DStorage();

        s_Data->QuadVertexArray = std::make_shared<VertexArray>();

        float squareVertices[4 * 3] = {
            -0.5f, -0.5f, 0.0f,  // Bottom Left
             0.5f, -0.5f, 0.0f,  // Bottom Right
             0.5f,  0.5f, 0.0f,  // Top Right
            -0.5f,  0.5f, 0.0f   // Top Left
        };

        // PRODUCTION FIX: Explicitly cast size_t to uint32_t for cross-platform safety
        uint32_t vertexBufferSize = static_cast<uint32_t>(sizeof(squareVertices));
        std::shared_ptr<VertexBuffer> squareVB = std::make_shared<VertexBuffer>(squareVertices, vertexBufferSize);

        squareVB->SetLayout({
            { ShaderDataType::Float3, "a_Position" }
            });

        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

        // PRODUCTION FIX: Explicitly cast count to uint32_t
        uint32_t indexBufferCount = static_cast<uint32_t>(sizeof(squareIndices) / sizeof(uint32_t));
        std::shared_ptr<IndexBuffer> squareIB = std::make_shared<IndexBuffer>(squareIndices, indexBufferCount);

        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        s_Data->FlatColorShader = std::make_shared<Shader>("/assets/shaders/FlatColor.glsl", "/assets/shaders/FlatColor.glsl");
        AETHER_ASSERT(s_Data->FlatColorShader, "Failed to load Renderer2D FlatColor Shader!");
    }

    void Renderer2D::Shutdown()
    {
        delete s_Data;
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetMat4("u_ViewProjection", &viewProjection[0][0]);
    }

    void Renderer2D::EndScene()
    {
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetFloat4("u_Color", color.r, color.g, color.b, color.a);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        s_Data->FlatColorShader->SetMat4("u_Transform", &transform[0][0]);

        s_Data->QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data->QuadVertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}