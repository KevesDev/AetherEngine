#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "../core/Log.h"

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

namespace aether {

    struct Renderer2DData
    {
        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<Shader> FlatColorShader;
        std::shared_ptr<Shader> TextureShader;

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData* s_Data = nullptr;

    void Renderer2D::Init()
    {
        AETHER_CORE_INFO("Renderer2D: Initializing...");
        s_Data = new Renderer2DData();

        s_Data->QuadVertexArray = std::make_shared<VertexArray>();

        float squareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };

        auto squareVB = std::make_shared<VertexBuffer>(squareVertices, sizeof(squareVertices));
        squareVB->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
            });
        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
        auto squareIB = std::make_shared<IndexBuffer>(squareIndices, 6);
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        AETHER_CORE_INFO("Renderer2D: Loading Shaders...");
        s_Data->FlatColorShader = std::make_shared<Shader>("/engine/shaders/FlatColor.glsl", "/engine/shaders/FlatColor.glsl");
        s_Data->TextureShader = std::make_shared<Shader>("/engine/shaders/Texture.glsl", "/engine/shaders/Texture.glsl");

        AETHER_CORE_INFO("Renderer2D: Initialized Successfully.");
    }

    void Renderer2D::Shutdown()
    {
        delete s_Data;
        s_Data = nullptr;
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        if (!s_Data) return;

        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetMat4("u_ViewProjection", &viewProjection[0][0]);

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", &viewProjection[0][0]);
    }

    void Renderer2D::EndScene()
    {
        Flush();
    }

    void Renderer2D::Flush()
    {
        // Batch rendering would submit buffered quads here
        // For now, immediate mode is used
    }

    // --- Axis-Aligned Colored Quads ---
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color);
    }

    // --- Axis-Aligned Textured Quads ---
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    // --- Rotated Colored Quads ---
    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color);
    }

    // --- Rotated Textured Quads ---
    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    // --- Transform Matrix Colored Quad (Core Implementation) ---
    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        if (!s_Data) return;

        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetFloat4("u_Color", color.r, color.g, color.b, color.a);
        s_Data->FlatColorShader->SetMat4("u_Transform", &transform[0][0]);

        s_Data->QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data->QuadVertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

        s_Data->Stats.DrawCalls++;
        s_Data->Stats.QuadCount++;
    }

    // --- Transform Matrix Textured Quad (Core Implementation) ---
    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        if (!s_Data || !texture) return;

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetFloat4("u_Color", tintColor.r, tintColor.g, tintColor.b, tintColor.a);
        s_Data->TextureShader->SetMat4("u_Transform", &transform[0][0]);
        s_Data->TextureShader->SetInt("u_Texture", 0);

        texture->Bind(0);

        s_Data->QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data->QuadVertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

        s_Data->Stats.DrawCalls++;
        s_Data->Stats.QuadCount++;
    }

    void Renderer2D::ResetStats()
    {
        if (!s_Data) return;
        memset(&s_Data->Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        if (!s_Data) return {};
        return s_Data->Stats;
    }

    void Renderer2D::OnWindowResize(uint32_t width, uint32_t height)
    {
        glViewport(0, 0, width, height);
    }
}