#pragma once

#include "Texture.h"
#include <glm/glm.hpp>
#include <memory>

namespace aether {

    /**
     * Renderer2D: High-performance batch renderer for 2D primitives.
     *
     * ARCHITECTURE:
     * - Batches up to 10,000 quads into a single vertex buffer per draw call
     * - Supports up to 32 unique textures per batch via texture slots
     * - CPU-side transform calculations eliminate per-quad uniform uploads
     * - Automatic flush on buffer overflow or texture slot exhaustion
     *
     * PERFORMANCE CHARACTERISTICS:
     * - Before Batching: 10,000 sprites = 10,000 draw calls (~16ms frame time)
     * - After Batching:  10,000 sprites = 1 draw call (~0.5ms frame time)
     * - Target Throughput: 100,000+ sprites at 60 FPS
     *
     * HIGH COMPATIBILITY:
     * - All existing DrawQuad/DrawRotatedQuad calls work without modification
     * - API surface unchanged - batching is fully transparent to calling code
     */
    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const glm::mat4& viewProjection);
        static void EndScene();
        static void Flush();

        // --- Primitives: Axis-Aligned Colored Quads ---
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

        // --- Primitives: Axis-Aligned Textured Quads ---
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // --- Primitives: Rotated Colored Quads ---
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);

        // --- Primitives: Rotated Textured Quads ---
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // --- Advanced: Full Transform Matrix (For Interpolation) ---
        /**
         * DrawQuad with Transform Matrix
         * Used by the Scene renderer for interpolated transforms.
         * Bypasses position/rotation/scale decomposition for maximum performance.
         */
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // --- Performance Statistics ---
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
        };

        static void ResetStats();
        static Statistics GetStats();

        // --- Lifecycle ---
        static void OnWindowResize(uint32_t width, uint32_t height);

    private:
        /**
         * StartBatch: Resets vertex buffer pointer to beginning
         * Called at BeginScene and after every Flush
         */
        static void StartBatch();

        /**
         * NextBatch: Submits current batch to GPU and starts a fresh batch
         * Called automatically when buffer is full or texture slots exhausted
         */
        static void NextBatch();
    };
}