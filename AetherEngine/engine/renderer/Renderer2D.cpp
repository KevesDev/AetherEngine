#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "../core/Log.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace aether {

    /**
     * QuadVertex: Optimized vertex structure for batch rendering
     *
     * MEMORY LAYOUT (36 bytes per vertex):
     * - Position:   vec3 (12 bytes) - World-space position after CPU transform
     * - Color:      vec4 (16 bytes) - RGBA tint/color
     * - TexCoord:   vec2 (8 bytes)  - UV coordinates
     * - TexIndex:   float (4 bytes) - Texture slot index (0-31)
     *
     * CACHE OPTIMIZATION:
     * - 36 bytes = 2.25 cache lines per vertex on most CPUs
     * - Struct padding avoided via careful member ordering
     * - Aligned for SIMD operations (future optimization path)
     */
    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
    };

    /**
     * Renderer2DData: Internal batch state
     *
     * CAPACITY LIMITS:
     * - Max Quads Per Batch: 10,000
     * - Max Vertices: 40,000 (10,000 quads * 4 vertices)
     * - Max Indices: 60,000 (10,000 quads * 6 indices)
     * - Max Texture Slots: 32 (most GPUs support 16-32, we cap at 32 for safety)
     *
     * MEMORY FOOTPRINT:
     * - Vertex Buffer: 1.44 MB (40,000 vertices * 36 bytes)
     * - Index Buffer: 240 KB (60,000 indices * 4 bytes)
     * - Total: ~1.7 MB per renderer instance
     */
    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 10000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32; // OpenGL minimum guaranteed

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> TextureShader;
        std::shared_ptr<Texture2D> WhiteTexture; // 1x1 white for colored quads

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        // Texture slot management
        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData* s_Data = nullptr;

    void Renderer2D::Init()
    {
        AETHER_CORE_INFO("Renderer2D: Initializing batched renderer...");
        s_Data = new Renderer2DData();

        // -------------------------------------------------------------------------
        // Vertex Array & Buffer Setup
        // -------------------------------------------------------------------------
        s_Data->QuadVertexArray = std::make_shared<VertexArray>();

        s_Data->QuadVertexBuffer = std::make_shared<VertexBuffer>(
            static_cast<uint32_t>(s_Data->MaxVertices * sizeof(QuadVertex))
        );

        s_Data->QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" }
            });
        s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

        s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MaxVertices];

        // -------------------------------------------------------------------------
        // Index Buffer Setup (Static - Never Changes)
        // -------------------------------------------------------------------------
        uint32_t* quadIndices = new uint32_t[s_Data->MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data->MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        auto quadIB = std::make_shared<IndexBuffer>(quadIndices, s_Data->MaxIndices);
        s_Data->QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        // -------------------------------------------------------------------------
        // White Texture (1x1 for colored quads)
        // -------------------------------------------------------------------------
        uint32_t whiteTextureData = 0xffffffff;
        TextureSpecification whiteSpec;
        whiteSpec.Width = 1;
        whiteSpec.Height = 1;
        whiteSpec.Format = ImageFormat::RGBA8;
        s_Data->WhiteTexture = std::make_shared<Texture2D>(whiteSpec);
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        // Set first texture slot to white texture
        s_Data->TextureSlots[0] = s_Data->WhiteTexture;

        // -------------------------------------------------------------------------
        // Shader Setup
        // -------------------------------------------------------------------------
        AETHER_CORE_INFO("Renderer2D: Loading batched shader...");
        s_Data->TextureShader = std::make_shared<Shader>(
            "/engine/shaders/Renderer2D_Batch.glsl",
            "/engine/shaders/Renderer2D_Batch.glsl"
        );

        // Bind texture slots to samplers
        int32_t samplers[s_Data->MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data->MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetIntArray("u_Textures", samplers, s_Data->MaxTextureSlots);

        // Set quad vertex positions (unit quad centered at origin)
        s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        AETHER_CORE_INFO("Renderer2D: Batched renderer initialized successfully.");
        AETHER_CORE_INFO("  - Max Quads Per Batch: {}", s_Data->MaxQuads);
        AETHER_CORE_INFO("  - Max Texture Slots: {}", s_Data->MaxTextureSlots);
        AETHER_CORE_INFO("  - Vertex Buffer Size: {} KB", (s_Data->MaxVertices * sizeof(QuadVertex)) / 1024);
    }

    void Renderer2D::Shutdown()
    {
        delete[] s_Data->QuadVertexBufferBase;
        delete s_Data;
        s_Data = nullptr;
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        if (!s_Data) return;

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", &viewProjection[0][0]);

        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        if (!s_Data) return;

        // Flush any remaining quads in the batch
        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;

        s_Data->TextureSlotIndex = 1; // Reset texture slots (0 is white texture)
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::Flush()
    {
        if (s_Data->QuadIndexCount == 0)
            return; // Nothing to draw

        // Upload vertex data to GPU
        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase);
        s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);

        // Bind all active textures
        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++)
            s_Data->TextureSlots[i]->Bind(i);

        // Submit draw call
        s_Data->QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data->QuadIndexCount, GL_UNSIGNED_INT, nullptr);
        s_Data->Stats.DrawCalls++;
    }

    // -------------------------------------------------------------------------
    // Core Batching Function: DrawQuad (Transform Matrix)
    // -------------------------------------------------------------------------
    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };
        const float textureIndex = 0.0f; // White texture

        // Check if we need to start a new batch
        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        // CPU-side transform calculation (eliminates per-quad uniform uploads)
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->Color = color;
            s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
        s_Data->Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        // Check if we need to start a new batch
        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        // Find texture slot or add new texture
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        // If texture not found, add it to a new slot
        if (textureIndex == 0.0f)
        {
            // Check if we have available texture slots
            if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch(); // Out of texture slots, flush and start new batch

            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        // CPU-side transform calculation
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->Color = tintColor;
            s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i] * tilingFactor;
            s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
        s_Data->Stats.QuadCount++;
    }

    // -------------------------------------------------------------------------
    // Convenience Overloads (Forward to Core Implementation)
    // -------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------
    // Statistics & Lifecycle
    // -------------------------------------------------------------------------

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