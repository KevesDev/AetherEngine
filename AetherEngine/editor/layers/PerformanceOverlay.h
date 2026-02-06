#pragma once

#include "../../engine/core/AetherTime.h"
#include "../../engine/renderer/Renderer2D.h"
#include <imgui.h>
#include <deque>
#include <chrono>

namespace aether {

    /**
     * PerformanceOverlay: Real-time performance statistics display
     *
     * Provides comprehensive performance metrics in an unobtrusive overlay.
     *
     * FEATURES:
     * - Frame time graph (rolling 120 frames)
     * - Draw call statistics
     * - Memory usage tracking
     * - FPS counter with color coding
     * - Toggleable via settings panel
     * - Minimal performance overhead (<0.1ms)
     */
    class PerformanceOverlay
    {
    public:
        PerformanceOverlay();

        /**
         * OnImGuiRender: Renders the overlay as a semi-transparent window
         * Position: Top-right corner (configurable)
         * Style: Minimal, non-intrusive, auto-hides when not enabled
         */
        void OnImGuiRender();

        /**
         * Toggle visibility via settings panel
         */
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        /**
         * SetPosition: Configure overlay placement
         * 0 = Top-Left, 1 = Top-Right, 2 = Bottom-Left, 3 = Bottom-Right
         */
        void SetPosition(int position) { m_CornerPosition = position; }

        /**
         * SetCornerPosition: Dynamically set the overlay position (e.g. from Editor viewport bounds)
         */
        void SetCornerPosition(float x, float y) { m_PosX = x; m_PosY = y; }

    private:
        /**
         * UpdateFrameHistory: Records frame time for graph rendering
         * Maintains rolling buffer of last 120 frames
         */
        void UpdateFrameHistory(float deltaTime);

        /**
         * RenderFPSCounter: Large, color-coded FPS display
         * Green: 60+ FPS, Yellow: 30-60 FPS, Red: <30 FPS
         */
        void RenderFPSCounter();

        /**
         * RenderFrameStats: Detailed frame timing breakdown
         */
        void RenderFrameStats();

        /**
         * RenderRendererStats: Draw calls, quad count, batching efficiency
         */
        void RenderRendererStats();

        /**
         * RenderFrameGraph: Visual frame time history
         */
        void RenderFrameGraph();

        /**
         * GetFPSColor: Color coding based on performance
         */
        ImVec4 GetFPSColor(float fps);

    private:
        bool m_Enabled = true;
        int m_CornerPosition = 1; // 0=TL, 1=TR, 2=BL, 3=BR

        // Dynamic position coordinates
        float m_PosX = 0.0f;
        float m_PosY = 0.0f;

        // Frame timing data
        std::deque<float> m_FrameTimeHistory; // Last 120 frames
        static const size_t MaxFrameHistory = 120;

        float m_CurrentFPS = 0.0f;
        float m_AverageFPS = 0.0f;
        float m_FrameTimeMS = 0.0f;

        // Running averages
        float m_FPSAccumulator = 0.0f;
        int m_FPSFrameCount = 0;

        // Update interval (avoid updating every frame for readability)
        float m_UpdateTimer = 0.0f;
        static constexpr float UpdateInterval = 0.1f; // 10 updates per second
    };
}