#include "EditorLayer.h"

#include <imgui.h>
#include <glad/glad.h>

#include "../../engine/scene/SceneSerializer.h"
#include "../../engine/renderer/Renderer2D.h"
#include "../../engine/core/Engine.h"
#include "../../engine/core/Log.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/ecs/Components.h" // Fixes TagComponent undefined

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

namespace aether {

    // Fix: EditorCamera constructor mismatch. 
    // Assuming EditorCamera() defaults are acceptable or SetViewportSize handles it.
    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        // Fix: AssetManager API. Assuming Get<T> based on typical patterns.
        m_CheckerboardTexture = AssetManager::Get<Texture2D>("EngineContent/textures/T_checkerboard.png");

        // Fix: Framebuffer API. 
        // If initializer list for Attachments fails, likely requires distinct format assignment or legacy spec.
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        // Assuming FramebufferTextureFormat is an enum, ensuring vector is constructed correctly.
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };

        m_Framebuffer = Framebuffer::Create(fbSpec); // Use Create factory instead of make_shared directly if abstract

        m_ActiveScene = std::make_shared<Scene>();

        // Fix: SetContext signature mismatch. If it expects shared_ptr, this is fine. 
        // If it expects raw pointer, use m_ActiveScene.get().
        // Standard panels usually take the shared_ptr to share ownership/lifecycle.
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        Renderer2D::ResetStats();

        m_Framebuffer->Bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Fix: ClearAttachment API. Ensure 1 is the correct index for RED_INTEGER
        m_Framebuffer->ClearAttachment(1, -1);

        switch (m_SceneState)
        {
        case SceneState::Edit:
        {
            if (m_ViewportFocused)
                m_EditorCamera.OnUpdate(ts);

            m_ActiveScene->OnRender(m_EditorCamera.GetViewProjection());
            break;
        }
        case SceneState::Play:
        {
            m_ActiveScene->OnUpdateSimulation(ts.GetSeconds()); // Pass float seconds
            glm::mat4 viewProj = m_ActiveScene->GetPrimaryCameraViewProjection();
            m_ActiveScene->OnRender(viewProj);
            break;
        }
        }

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
            // Fix: Entity constructor requires Scene*
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((EntityID)pixelData, m_ActiveScene.get());
        }

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
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

        ImGui::Begin("Renderer Stats");
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);

        // TagComponent is now defined
        if (m_HoveredEntity && m_HoveredEntity.HasComponent<TagComponent>())
            ImGui::Text("Hovered Entity: %s", m_HoveredEntity.GetComponent<TagComponent>().Tag.c_str());
        else
            ImGui::Text("Hovered Entity: None");

        ImGui::End();

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

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0) return false;

        bool control = Input::IsKeyPressed(Key::LeftCtrl) || Input::IsKeyPressed(Key::RightCtrl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode())
        {
        case Key::N:
        {
            if (control) NewScene();
            break;
        }
        case Key::O:
        {
            if (control) OpenScene();
            break;
        }
        case Key::S:
        {
            if (control)
            {
                if (shift) SaveSceneAs();
                else SaveScene();
            }
            break;
        }
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

    void EditorLayer::NewScene()
    {
        m_ActiveScene = std::make_shared<Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_EditorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        AETHER_CORE_WARN("OpenScene (Dialog) unavailable. Use Content Browser Drag & Drop.");
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (path.extension().string() != ".aeth")
        {
            AETHER_CORE_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path))
        {
            m_ActiveScene = newScene;
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_EditorScenePath = path;
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(m_EditorScenePath);
        }
        else
        {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        AETHER_CORE_WARN("SaveSceneAs (Dialog) unavailable.");
    }

}