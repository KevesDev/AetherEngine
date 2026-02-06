#include "PerformanceOverlay.h"
#include "../../engine/core/Theme.h"
#include <algorithm>
#include <cstdio> // For sprintf if needed by ImGui internal formatting

namespace aether {

    PerformanceOverlay::PerformanceOverlay()
    {
        m_FrameTimeHistory.resize(MaxFrameHistory, 16.67f); // Pre-fill with 60 FPS baseline
    }

    void PerformanceOverlay::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        // -------------------------------------------------------------------------
        // Logic Update (Frame Timing)
        // -------------------------------------------------------------------------
        float deltaTime = (float)AetherTime::GetFrameDelta();
        UpdateFrameHistory(deltaTime);

        // Update FPS counter (smoothed)
        m_UpdateTimer += deltaTime;
        if (m_UpdateTimer >= UpdateInterval)
        {
            m_CurrentFPS = 1.0f / std::max(deltaTime, 0.0001f);
            m_FrameTimeMS = deltaTime * 1000.0f;

            // Calculate average FPS over last 120 frames
            float sum = 0.0f;
            for (float ft : m_FrameTimeHistory)
                sum += ft;
            float avgFrameTime = sum / m_FrameTimeHistory.size();
            m_AverageFPS = 1.0f / std::max(avgFrameTime, 0.0001f);

            m_UpdateTimer = 0.0f;
        }

        // -------------------------------------------------------------------------
        // UI Rendering
        // -------------------------------------------------------------------------
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking; // Important: Prevent docking

        // Position: Top-Right of Viewport (Pivot = 1.0, 0.0)
        ImGui::SetNextWindowPos(ImVec2(m_PosX, m_PosY), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        // Style: Transparent background
        ImGui::SetNextWindowBgAlpha(0.35f);

        // Style: Tighter padding for a smaller footprint
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));

        if (ImGui::Begin("Performance Statistics", &m_Enabled, window_flags))
        {
            Theme theme;

            // 1. FPS Counter (Compact)
            ImVec4 fpsColor = GetFPSColor(m_CurrentFPS);
            ImGui::TextColored(fpsColor, "FPS: %.0f", m_CurrentFPS);

            ImGui::SameLine();
            ImGui::TextColored(theme.TextMuted, "(%.2f ms)", m_FrameTimeMS);

            // 2. Renderer Stats (One Line)
            auto stats = Renderer2D::GetStats();
            ImGui::Text("Draws: %d  Quads: %d", stats.DrawCalls, stats.QuadCount);

            // 3. Mini Frame Time Graph
            std::vector<float> frameTimesMS;
            frameTimesMS.reserve(m_FrameTimeHistory.size());
            for (float ft : m_FrameTimeHistory)
                frameTimesMS.push_back(ft * 1000.0f); // Convert to milliseconds

            // Scale graph max to either the current max spike or 30 FPS limit (33ms)
            float maxTime = *std::max_element(frameTimesMS.begin(), frameTimesMS.end());
            maxTime = std::max(maxTime, 33.33f);

            ImGui::PlotLines(
                "##FrameTimeGraph",
                frameTimesMS.data(),
                static_cast<int>(frameTimesMS.size()),
                0,
                nullptr,
                0.0f,
                maxTime,
                ImVec2(200, 40) // Small fixed size
            );
        }
        ImGui::End();
        ImGui::PopStyleVar(); // Pop WindowPadding
    }

    void PerformanceOverlay::UpdateFrameHistory(float deltaTime)
    {
        // Add new frame time to history
        m_FrameTimeHistory.push_back(deltaTime);

        // Maintain fixed size (rolling buffer)
        if (m_FrameTimeHistory.size() > MaxFrameHistory)
            m_FrameTimeHistory.pop_front();
    }

    ImVec4 PerformanceOverlay::GetFPSColor(float fps)
    {
        if (fps >= 60.0f)
            return ImVec4(0.3f, 1.0f, 0.3f, 1.0f); // Green (excellent)
        else if (fps >= 30.0f)
            return ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Yellow (acceptable)
        else
            return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red (poor)
    }
}