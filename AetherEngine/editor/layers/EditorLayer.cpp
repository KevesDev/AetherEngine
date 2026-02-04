#include "EditorLayer.h"
#include <imgui/imgui.h>

#include "aether/scene/SceneSerializer.h"
#include "aether/utils/PlatformUtils.h"
#include "aether/math/Math.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace aether {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer"), m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f)
    {
    }

    void EditorLayer::OnAttach()
    {
        // Load default texture for generic use
        m_CheckerboardTexture = AssetManager::GetAsset<Texture2D>("assets/textures/T_checkerboard.png");

        // Initialize Framebuffer
        // We use a high-precision format for the entity ID attachment (RED_INTEGER)
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = std::make_shared<Framebuffer>(fbSpec);

        // Initialize Scene
        m_ActiveScene = std::make_shared<Scene>();

        // Connect Panels
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        // -------------------------------------------------------------------------
        // 1. Viewport Sizing & Framebuffer Management
        // -------------------------------------------------------------------------
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

            // ECO-001: Explicitly inform Scene of the new aspect ratio.
            // The Scene does not access the Window or EditorCamera directly.
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // -------------------------------------------------------------------------
        // 2. Simulation Step
        // -------------------------------------------------------------------------

        Renderer2D::ResetStats();

        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        // Clear Entity ID attachment (used for mouse picking)
        m_Framebuffer->ClearAttachment(1, -1);

        switch (m_SceneState)
        {
        case SceneState::Edit:
        {
            // Update Editor Camera (Input/Movement)
            if (m_ViewportFocused)
                m_EditorCamera.OnUpdate(ts);

            // In Edit Mode, we DO NOT step the simulation.
            // We only invoke the Presentation Pipeline using the Editor Camera.
            m_ActiveScene->OnRender(m_EditorCamera.GetViewProjection());
            break;
        }
        case SceneState::Play:
        {
            // Run Authoritative Simulation (Fixed Step Logic)
            m_ActiveScene->OnUpdateSimulation(ts);

            // Render using the Game Camera (Primary Camera Component)
            // This mimics the Client behavior exactly.
            glm::mat4 viewProj = m_ActiveScene->GetPrimaryCameraViewProjection();
            m_ActiveScene->OnRender(viewProj);
            break;
        }
        }

        // -------------------------------------------------------------------------
        // 3. Editor Overlays & Picking
        // -------------------------------------------------------------------------

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, &m_ActiveScene->GetRegistry());
        }

        // Note: Overlay rendering (colliders/gizmos) would be injected here via a dedicated OverlayRenderer

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
		// -------------------------------------------------------------------------
		//     DOCKSPACE SETUP
		//  -------------------------------------------------------------------------
        // Ensure blocking stays disabled
        Engine::Get().GetImGuiLayer()->SetBlockEvents(false);

        World* world = Engine::Get().GetWorld();
        if (!world) { ImGui::Text("Loading World..."); return; }

        Theme theme;
        m_SceneHierarchyPanel.SetContext(world->GetScene());
        m_InspectorPanel.SetContext(m_SceneHierarchyPanel.GetSelectedEntity());

        m_ContentBrowserPanel.SetShowRawAssets(m_Settings.ShowRawAssets);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, theme.PanelHover);
        ImGui::Begin("Aether DockSpace", nullptr, window_flags);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGuiID dockspace_id = ImGui::GetID("AetherMasterDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        if (ImGui::BeginMenuBar())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, theme.AccentPrimary);

            if (ImGui::BeginMenu("File")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                    ProjectSerializer s(Project::GetActive());
                    s.Serialize(Project::GetActive()->GetProjectDirectory() / (Project::GetActiveConfig().Name + ".aether"));
                }
                if (ImGui::MenuItem("Exit")) Engine::Get().Close();
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Undo", "Ctrl+Z", false, CommandHistory::CanUndo())) {
                    CommandHistory::Undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, CommandHistory::CanRedo())) {
                    CommandHistory::Redo();
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Preferences...", nullptr, m_ShowPreferences)) {
                    m_ShowPreferences = true;
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Reset Layout")) EnsureLayout(dockspace_id);
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }
            ImGui::PopStyleColor();
            ImGui::EndMenuBar();
        }

        if (m_IsFirstFrame) {
            if (!std::filesystem::exists(m_IniFilePath)) EnsureLayout(dockspace_id);
            m_IsFirstFrame = false;
        }

        m_SceneHierarchyPanel.OnImGuiRender();
        m_InspectorPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();

        RenderPreferencesPanel();

        for (auto it = m_AssetEditors.begin(); it != m_AssetEditors.end(); ) {
            (*it)->OnImGuiRender();
            if (!(*it)->IsOpen()) it = m_AssetEditors.erase(it);
            else ++it;
        }

		// -------------------------------------------------------------------------
		//     VIEWPORT PANEL
		// -------------------------------------------------------------------------

        // --- Viewport Panel ---
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        // Content Browser Drag & Drop
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(std::filesystem::path(g_AssetPath) / path);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
        ImGui::PopStyleVar();

        // --- Stats Panel ---
        ImGui::Begin("Renderer Stats");
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Hovered Entity: %s", m_HoveredEntity ? m_HoveredEntity.GetComponent<TagComponent>().Tag.c_str() : "None");
        ImGui::End();

        // --- Management Panels ---
        m_SceneHierarchyPanel.OnImGuiRender();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        if (m_SceneState == SceneState::Edit)
        {
            m_EditorCamera.OnEvent(e);
        }

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }
}