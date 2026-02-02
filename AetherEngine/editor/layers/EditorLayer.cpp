#include "EditorLayer.h"
#include "../../engine/core/Engine.h" 
#include "../../engine/scene/World.h"
#include "../../engine/core/Theme.h"
#include "../../engine/project/Project.h"
#include "../../engine/project/ProjectSerializer.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/ecs/Registry.h"
#include "../../engine/events/Event.h"
#include "../../engine/events/ApplicationEvent.h"
#include "../../engine/input/KeyCodes.h"
#include "../../engine/renderer/Renderer2D.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>
#include <glad/glad.h>

namespace aether {

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    void EditorLayer::OnAttach()
    {
        AETHER_ASSERT(ImGui::GetCurrentContext(), "EditorLayer attached but ImGui Context is missing!");

        // 1. Apply Theme
        Theme theme;
        ThemeManager::ApplyTheme(theme);

        // 2. Setup Persistent Layout
        auto settingsDir = Project::GetSettingsDirectory();
        if (!std::filesystem::exists(settingsDir)) {
            std::filesystem::create_directories(settingsDir);
        }

        m_IniFilePath = (settingsDir / "editor.ini").string();
        ImGui::GetIO().IniFilename = m_IniFilePath.c_str();
        ImGui::LoadIniSettingsFromDisk(m_IniFilePath.c_str());

        // 3. Set Window Title
        std::string title = Project::GetActiveConfig().Name + " - Aether Editor";
        Engine::Get().GetWindow().SetTitle(title);

        AETHER_CORE_INFO("EditorLayer Attached. Layout File: {0}", m_IniFilePath);

        // 4. Create Framebuffer
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        AETHER_CORE_INFO("Framebuffer Initialized successfully.");
    }

    void EditorLayer::OnDetach()
    {
        if (ImGui::GetCurrentContext()) {
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        AETHER_CORE_INFO("EditorLayer Detached.");
    }

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        if (!m_Framebuffer) return;

        // 1. Resize Framebuffer
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != (uint32_t)m_ViewportSize.x || spec.Height != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // 2. Update Camera
        if (m_ViewportFocused)
        {
            m_EditorCamera.OnUpdate(ts);
        }

        // 3. Render Scene
        m_Framebuffer->Bind();

        Theme theme;
        glClearColor(theme.WindowBg.x, theme.WindowBg.y, theme.WindowBg.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer2D::BeginScene(m_EditorCamera.GetViewProjection());

        World* world = Engine::Get().GetWorld();
        if (world) {
            auto& registry = world->GetRegistry();
            auto& view = registry.View<TransformComponent>();
            auto& ownerMap = registry.GetOwnerMap<TransformComponent>();

            for (size_t i = 0; i < view.size(); i++) {
                EntityID id = ownerMap.at(i);
                if (registry.HasComponent<SpriteComponent>(id)) {
                    auto& transform = view[i];
                    auto* sprite = registry.GetComponent<SpriteComponent>(id);

                    Renderer2D::DrawQuad(
                        { transform.X, transform.Y },
                        { transform.ScaleX, transform.ScaleY },
                        { sprite->R, sprite->G, sprite->B, sprite->A }
                    );
                }
            }
        }

        Renderer2D::EndScene();
        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        if (m_ViewportHovered)
        {
            m_EditorCamera.OnEvent(e);
        }
    }

    void EditorLayer::OnImGuiRender()
    {
        World* world = Engine::Get().GetWorld();
        if (!world) {
            ImGui::Begin("Aether Editor Error");
            ImGui::Text("Loading...");
            ImGui::End();
            return;
        }

        Theme theme;

        // --- 1. Panels Context ---
        m_SceneHierarchyPanel.SetContext(world->GetScene());
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        m_InspectorPanel.SetContext(selectedEntity);

        // --- 2. Dockspace Setup ---
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // Background for Menu Bar
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, theme.PanelHover);

        ImGui::Begin("Aether Master DockSpace", nullptr, window_flags);

        ImGui::PopStyleColor(); // Pop MenuBarBg
        ImGui::PopStyleVar(3);  // Pop Window Vars

        ImGuiID dockspace_id = ImGui::GetID("AetherMasterDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // --- 3. Main Menu Bar ---
        if (ImGui::BeginMenuBar())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, theme.AccentPrimary);

            if (ImGui::BeginMenu("File"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);

                if (ImGui::MenuItem("Save Project", "Ctrl+S"))
                {
                    auto project = Project::GetActive();
                    if (project) {
                        // : Use project name for filename
                        std::string filename = project->GetConfig().Name + ".aether";
                        ProjectSerializer serializer(project);
                        serializer.Serialize(project->GetProjectDirectory() / filename);
                        AETHER_CORE_INFO("Project Saved: {0}", filename);
                    }
                }
                if (ImGui::MenuItem("Exit")) {
                    WindowCloseEvent event;
                    Engine::Get().OnEvent(event);
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Reset Layout"))
                {
                    EnsureLayout(dockspace_id);
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            ImGui::PopStyleColor(); // Pop AccentPrimary
            ImGui::EndMenuBar();

            // : Draw Lavender Border for visibility
            // We draw this *after* EndMenuBar so it overlays on top
            ImVec2 p_min = ImGui::GetWindowPos();
            ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetFrameHeight());

            // Draw a 1px border rectangle using AccentPrimary
            ImGui::GetWindowDrawList()->AddRect(p_min, p_max, ImGui::ColorConvertFloat4ToU32(theme.AccentPrimary));
        }

        if (m_IsFirstFrame) {
            if (!std::filesystem::exists(m_IniFilePath)) EnsureLayout(dockspace_id);
            m_IsFirstFrame = false;
        }
        ImGui::End();

        // --- 4. Render Panels ---
        m_SceneHierarchyPanel.OnImGuiRender();
        m_InspectorPanel.OnImGuiRender();

        // --- 5. Render Viewport ---
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        // Guards
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Engine::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        // Resize Hook
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        // Draw Framebuffer
        if (m_Framebuffer) {
            uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        }

        // Overlay Stats
        ImGui::SetCursorPos(ImVec2(20, 30));
        ImGui::TextColored(theme.AccentPrimary, "Viewport: %.0fx%.0f", m_ViewportSize.x, m_ViewportSize.y);

        ImGui::SetCursorPos(ImVec2(20, 50));
        ImGui::TextColored(theme.TextMuted, "FPS: %.0f", ImGui::GetIO().Framerate);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorLayer::EnsureLayout(unsigned int dockspace_id)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
        ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
        ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
        ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }
}