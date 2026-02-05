#include "PerformanceOverlay.h"
#include "../../engine/core/Theme.h"
#include <algorithm>

namespace aether {

    PerformanceOverlay::PerformanceOverlay()
    {
        m_FrameTimeHistory.resize(MaxFrameHistory, 16.67f); // Pre-fill with 60 FPS baseline
    }

    void PerformanceOverlay::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        // Update frame timing (real-time frame delta)
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
        // Render overlay window
        // -------------------------------------------------------------------------
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove;

        if (ImGui::Begin("Performance Statistics", &m_Enabled, window_flags))
        {
            Theme theme;

            // Header
            ImGui::TextColored(theme.AccentCyan, "PERFORMANCE STATS");
            ImGui::Separator();

            // FPS Counter (large, color-coded)
            RenderFPSCounter();

            ImGui::Spacing();

            // Frame timing details
            RenderFrameStats();

            ImGui::Spacing();

            // Renderer statistics
            RenderRendererStats();

            ImGui::Spacing();

            // Frame time graph
            RenderFrameGraph();

            // Footer hint
            ImGui::Spacing();
            ImGui::TextDisabled("Toggle in Settings > Performance");
        }
        ImGui::End();
    }

    void PerformanceOverlay::UpdateFrameHistory(float deltaTime)
    {
        // Add new frame time to history
        m_FrameTimeHistory.push_back(deltaTime);

        // Maintain fixed size (rolling buffer)
        if (m_FrameTimeHistory.size() > MaxFrameHistory)
            m_FrameTimeHistory.pop_front();
    }

    void PerformanceOverlay::RenderFPSCounter()
    {
        Theme theme;

        // Large FPS display with color coding
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font but scaled
        ImGui::SetWindowFontScale(2.0f);

        ImVec4 fpsColor = GetFPSColor(m_CurrentFPS);
        ImGui::TextColored(fpsColor, "%.0f FPS", m_CurrentFPS);

        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        // Show average in smaller text
        ImGui::SameLine();
        ImGui::TextColored(theme.TextMuted, " (avg: %.0f)", m_AverageFPS);
    }

    void PerformanceOverlay::RenderFrameStats()
    {
        Theme theme;

        ImGui::Text("Frame Time:");
        ImGui::SameLine(120);

        // Color code frame time
        ImVec4 color = theme.Text;
        if (m_FrameTimeMS > 33.33f)      // < 30 FPS
            color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
        else if (m_FrameTimeMS > 16.67f) // < 60 FPS
            color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
        else
            color = ImVec4(0.3f, 1.0f, 0.3f, 1.0f); // Green

        ImGui::TextColored(color, "%.2f ms", m_FrameTimeMS);

        // Simulation timing diagnostics
        const double fixedStep = AetherTime::GetFixedTimeStep();
        const std::uint64_t simTick = AetherTime::GetSimTick();

        ImGui::TextColored(theme.TextMuted, "Sim Tick:   %llu", static_cast<unsigned long long>(simTick));
        ImGui::TextColored(theme.TextMuted, "Fixed Step: %.3f ms", fixedStep * 1000.0);
    }

    void PerformanceOverlay::RenderRendererStats()
    {
        Theme theme;
        auto stats = Renderer2D::GetStats();

        ImGui::Text("Renderer:");

        // Draw calls (critical metric)
        ImGui::Text("  Draw Calls:");
        ImGui::SameLine(120);
        ImGui::TextColored(theme.AccentCyan, "%d", stats.DrawCalls);

        // Quad count
        ImGui::Text("  Quads:");
        ImGui::SameLine(120);
        ImGui::Text("%d", stats.QuadCount);

        // Vertices
        ImGui::Text("  Vertices:");
        ImGui::SameLine(120);
        ImGui::TextColored(theme.TextMuted, "%d", stats.GetTotalVertexCount());

        // Batching efficiency
        if (stats.DrawCalls > 0)
        {
            float efficiency = (float)stats.QuadCount / (float)stats.DrawCalls;
            ImGui::Text("  Efficiency:");
            ImGui::SameLine(120);

            // Color code efficiency (higher is better)
            ImVec4 effColor = theme.Text;
            if (efficiency > 1000.0f)
                effColor = theme.AccentSuccess; // Excellent batching
            else if (efficiency > 100.0f)
                effColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f); // Good batching
            else if (efficiency < 10.0f)
                effColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Poor batching

            ImGui::TextColored(effColor, "%.0f q/call", efficiency);
        }
    }

    void PerformanceOverlay::RenderFrameGraph()
    {
        // Convert deque to vector for ImGui::PlotLines
        std::vector<float> frameTimesMS;
        frameTimesMS.reserve(m_FrameTimeHistory.size());
        for (float ft : m_FrameTimeHistory)
            frameTimesMS.push_back(ft * 1000.0f); // Convert to milliseconds

        // Draw frame time graph
        ImGui::Text("Frame Time History (120 frames):");

        // Calculate min/max for scale
        float minTime = *std::min_element(frameTimesMS.begin(), frameTimesMS.end());
        float maxTime = *std::max_element(frameTimesMS.begin(), frameTimesMS.end());

        // Add some padding to max for better visualization
        maxTime = std::max(maxTime * 1.2f, 33.33f); // At least show 30 FPS threshold

        ImGui::PlotLines(
            "##FrameTimeGraph",
            frameTimesMS.data(),
            static_cast<int>(frameTimesMS.size()),
            0,
            nullptr,
            minTime,
            maxTime,
            ImVec2(250, 80)
        );

        // Draw reference lines (16.67ms = 60 FPS, 33.33ms = 30 FPS)
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "16.67ms");
        ImGui::SameLine();
        ImGui::TextDisabled("(60 FPS)");
        ImGui::SameLine(150);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "33.33ms");
        ImGui::SameLine();
        ImGui::TextDisabled("(30 FPS)");
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