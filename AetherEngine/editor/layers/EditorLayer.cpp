#include "EditorLayer.h"

#include <imgui.h>
#include <glad/glad.h>

#include "../../engine/scene/SceneSerializer.h"
#include "../../engine/renderer/Renderer2D.h"
#include "../../engine/core/Engine.h"
#include "../../engine/core/Log.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/ecs/Components.h"
#include "../panels/TextureViewerPanel.h"
#include "../../engine/core/VFS.h"
#include "../EditorResources.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

namespace aether {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    //-----------------------------------------------------------------------------------------
    // Layer Lifecycle
    //-----------------------------------------------------------------------------------------

    void EditorLayer::OnAttach()
    {
        // Initialize shared editor resources (Icons, Fonts)
        EditorResources::Init();

        // Load the editor background texture (Checkerboard)
        std::filesystem::path checkerPath;
        bool loaded = false;

        if (VFS::Resolve("/engine/textures/T_checkerboard.png", checkerPath))
        {
            if (std::filesystem::exists(checkerPath))
            {
                m_CheckerboardTexture = std::make_shared<Texture2D>(checkerPath.string());
                loaded = true;
            }
        }

        // Fallback: Generate white texture if checkerboard is missing to prevent crashes
        if (!loaded)
        {
            AETHER_CORE_WARN("EditorLayer: Checkerboard texture not found. Generating fallback.");

            TextureSpecification spec;
            spec.Width = 1;
            spec.Height = 1;
            spec.Format = ImageFormat::RGBA8;

            m_CheckerboardTexture = std::make_shared<Texture2D>(spec);
            uint32_t whiteData = 0xffffffff;
            m_CheckerboardTexture->SetData(&whiteData, sizeof(uint32_t));
        }

        // Initialize Framebuffer for the Viewport panel
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        // Attachments: Color, EntityID (for picking), Depth
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };

        m_Framebuffer = Framebuffer::Create(fbSpec);

        // Initialize Scene Context
        m_ActiveScene = std::make_shared<Scene>();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

        // Define Content Browser behavior (Asset Double-Click)
        m_ContentBrowserPanel.SetOnAssetOpenedCallback([this](const std::filesystem::path& path) {
            // Only handle Engine Assets (.aeth)
            if (path.extension() == ".aeth")
            {
                AssetType type = AssetManager::GetAssetTypeFromExtension(path);

                if (type == AssetType::Texture2D)
                {
                    // Check for existing editor to avoid duplicates
                    for (const auto& panel : m_AssetEditors) {
                        if (panel->GetAssetPath() == path) {
                            panel->SetFocus();
                            return;
                        }
                    }
                    m_AssetEditors.push_back(std::make_shared<TextureViewerPanel>("Texture Viewer", path));
                }
                else if (type == AssetType::Scene)
                {
                    OpenScene(path);
                }
                else
                {
                    AETHER_CORE_WARN("EditorLayer: Unknown or unsupported asset type for: {}", path.string());
                }
            }
            });
    }

    void EditorLayer::OnDetach()
    {
        EditorResources::Shutdown();
    }

    //-----------------------------------------------------------------------------------------
    // Main Update Loop
    //-----------------------------------------------------------------------------------------

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        // 1. Resize Framebuffer if Viewport size changed
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // 2. Reset Statistics
        Renderer2D::ResetStats();

        // 3. Render Scene
        m_Framebuffer->Bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_Framebuffer->ClearAttachment(1, -1); // Clear EntityID attachment

        switch (m_SceneState)
        {
        case SceneState::Edit:
            if (m_ViewportFocused) m_EditorCamera.OnUpdate(ts);
            m_ActiveScene->OnRender(m_EditorCamera.GetViewProjection());
            break;
        case SceneState::Play:
            m_ActiveScene->OnUpdateSimulation(ts.GetSeconds());
            m_ActiveScene->OnRender(m_ActiveScene->GetPrimaryCameraViewProjection());
            break;
        }

        // 4. Mouse Picking (Entity Selection)
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my; // Flip Y
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((EntityID)pixelData, &m_ActiveScene->GetRegistry());
        }

        m_Framebuffer->Unbind();
    }

    //-----------------------------------------------------------------------------------------
    // ImGui Layout & Rendering
    //-----------------------------------------------------------------------------------------

    void EditorLayer::SetupDockSpace()
    {
        static bool dockspaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Aether DockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("AetherEditorDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) OpenScene();
                if (ImGui::MenuItem("Save", "Ctrl+S")) SaveScene();
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneAs();
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { WindowCloseEvent e; Engine::Get().OnEvent(e); }
                ImGui::EndMenu();
            }

            // [Modified] View Menu with Submenus
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::BeginMenu("Content Browser"))
                {
                    bool showRaw = m_ContentBrowserPanel.GetShowRawAssets();
                    if (ImGui::MenuItem("Raw Sources", nullptr, &showRaw))
                    {
                        m_ContentBrowserPanel.SetShowRawAssets(showRaw);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Statistics"))
                {
                    bool perfEnabled = m_PerformanceOverlay.IsEnabled();
                    if (ImGui::MenuItem("Performance Overlay", nullptr, &perfEnabled))
                        m_PerformanceOverlay.SetEnabled(perfEnabled);

                    if (ImGui::MenuItem("Renderer Stats", nullptr, &m_ShowRendererStats)) {}
                    if (ImGui::MenuItem("Network Overlay", nullptr, &m_ShowNetworkPanel)) {}

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }

    void EditorLayer::OnImGuiRender()
    {
        SetupDockSpace();

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

        // Block input if viewport is not focused
        Engine::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(std::filesystem::path(path));
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
        ImGui::PopStyleVar();

        // --- Render Overlays ---
        m_PerformanceOverlay.SetCornerPosition(m_ViewportBounds[1].x - 10.0f, m_ViewportBounds[0].y + 10.0f);
        m_PerformanceOverlay.OnImGuiRender();

        m_SceneHierarchyPanel.OnImGuiRender();

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity) m_InspectorPanel.SetContext(selectedEntity);
        m_InspectorPanel.OnImGuiRender();

        m_ContentBrowserPanel.OnImGuiRender();

        // Render Asset Editors
        for (auto it = m_AssetEditors.begin(); it != m_AssetEditors.end(); )
        {
            (*it)->OnImGuiRender();
            if (!(*it)->IsOpen()) it = m_AssetEditors.erase(it);
            else ++it;
        }

        if (m_ShowRendererStats)
        {
            ImGui::Begin("Renderer Stats", &m_ShowRendererStats);
            auto stats = Renderer2D::GetStats();
            ImGui::Text("Draw Calls: %d", stats.DrawCalls);
            ImGui::Text("Quads: %d", stats.QuadCount);
            if (m_HoveredEntity && m_HoveredEntity.HasComponent<TagComponent>())
                ImGui::Text("Hovered Entity: %s", m_HoveredEntity.GetComponent<TagComponent>().Tag.c_str());
            else
                ImGui::Text("Hovered Entity: None");
            ImGui::End();
        }

        if (m_ShowNetworkPanel)
        {
            ImVec2 viewportMin(m_ViewportBounds[0].x, m_ViewportBounds[0].y);
            ImGui::SetNextWindowPos(ImVec2(viewportMin.x + 10.0f, viewportMin.y + 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowBgAlpha(0.35f);
            ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
            if (ImGui::Begin("Network Overlay", &m_ShowNetworkPanel, overlayFlags))
            {
                m_NetworkSettingsPanel.OnImGuiRender();
            }
            ImGui::End();
        }
    }

    void EditorLayer::OnEvent(Event& e)
    {
        if (m_SceneState == SceneState::Edit) m_EditorCamera.OnEvent(e);
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
        dispatcher.Dispatch<FileDropEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnFileDrop));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0) return false;
        bool control = Input::IsKeyPressed(Key::LeftCtrl) || Input::IsKeyPressed(Key::RightCtrl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode())
        {
        case Key::N: if (control) NewScene(); break;
        case Key::O: if (control) OpenScene(); break;
        case Key::S: if (control) { if (shift) SaveSceneAs(); else SaveScene(); } break;
        }
        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportHovered && !ImGui::IsAnyItemHovered())
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    bool EditorLayer::OnFileDrop(FileDropEvent& e)
    {
        std::filesystem::path targetDir = m_ContentBrowserPanel.GetCurrentDirectory();
        for (const auto& pathStr : e.GetPaths())
        {
            if (pathStr.empty()) continue;
            try {
                std::filesystem::path sourcePath(pathStr);
                if (!std::filesystem::exists(sourcePath)) {
                    AETHER_CORE_WARN("OnFileDrop: File not found: {}", pathStr);
                    continue;
                }
                std::filesystem::path destPath = targetDir / sourcePath.filename();
                if (!std::filesystem::exists(destPath)) {
                    std::filesystem::copy_file(sourcePath, destPath);
                    AssetManager::ImportSourceFile(destPath);
                    AETHER_CORE_INFO("Imported: {}", destPath.string());
                }
                else {
                    AETHER_CORE_WARN("File already exists: {}", destPath.string());
                }
            }
            catch (const std::exception& err) {
                AETHER_CORE_ERROR("OnFileDrop Error: {}", err.what());
            }
        }
        return true;
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = std::make_shared<Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_EditorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        AETHER_CORE_WARN("OpenScene (Dialog) unavailable. Use Content Browser.");
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (path.extension().string() != ".aeth") {
            AETHER_CORE_WARN("Invalid scene file: {}", path.string());
            return;
        }
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path)) {
            m_ActiveScene = newScene;
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_EditorScenePath = path;
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty()) {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(m_EditorScenePath);
        }
        else {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        AETHER_CORE_WARN("SaveSceneAs (Dialog) unavailable.");
    }
}