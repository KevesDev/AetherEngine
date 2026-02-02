#include "EditorLayer.h"
#include "../../engine/core/Engine.h" 
#include "../../engine/scene/World.h"
#include "../../engine/core/Theme.h"
#include "../../engine/project/Project.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/ecs/Registry.h"
#include "../../engine/events/Event.h"
#include "../../engine/input/KeyCodes.h"
#include "../../engine/renderer/Renderer2D.h" // Required for rendering

#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>
#include <glad/glad.h> // Required for glClear

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

        // 3. Set Window Title
        std::string title = Project::GetActiveConfig().Name + " - Aether Editor";
        Engine::Get().GetWindow().SetTitle(title);

        AETHER_CORE_INFO("EditorLayer Attached. Layout File: {0}", m_IniFilePath);

        // 4. RESTORED: Create Framebuffer
        // This was missing, causing the crash in OnUpdate!
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
        // Guard: Ensure Framebuffer exists before using it
        if (!m_Framebuffer) return;

        // 1. Handle Resize
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != (uint32_t)m_ViewportSize.x || spec.Height != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // 2. Update Editor Camera
        if (m_ViewportFocused)
        {
            m_EditorCamera.OnUpdate(ts);
        }

        // 3. RENDER SCENE (Off-Screen)
        m_Framebuffer->Bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

        // --- 1. Panels ---
        m_SceneHierarchyPanel.SetContext(world->GetScene());
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        m_InspectorPanel.SetContext(selectedEntity);

        // --- 2. Dockspace ---
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Aether Master DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("AetherMasterDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        if (m_IsFirstFrame) {
            if (!std::filesystem::exists(m_IniFilePath)) EnsureLayout(dockspace_id);
            m_IsFirstFrame = false;
        }
        ImGui::End();

        // --- 3. Render Panels ---
        m_SceneHierarchyPanel.OnImGuiRender();
        m_InspectorPanel.OnImGuiRender();

        // --- 4. Render Viewport ---
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        // Guards
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Engine::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        // Resize Hook
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        // Draw Framebuffer Image
        if (m_Framebuffer) {
            uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        }

        // Overlay Stats
        ImGui::SetCursorPos(ImVec2(10, 10));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "Viewport: %.0fx%.0f", m_ViewportSize.x, m_ViewportSize.y);

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