#include "EditorLayer.h"
#include "../../engine/core/Engine.h" 
#include "../../engine/scene/World.h"
#include "../../engine/core/Theme.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/project/Project.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace aether {

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    void EditorLayer::OnAttach()
    {
        AETHER_ASSERT(ImGui::GetCurrentContext(), "EditorLayer attached but ImGui Context is missing!");

        // 1. Apply Theme (Production Style)
        Theme theme;
        ThemeManager::ApplyTheme(theme);

        // 2. Disable ImGui's native INI file (We use JSON serialization)
        ImGui::GetIO().IniFilename = nullptr;

        std::string title = Project::GetActiveConfig().Name + " - Aether Editor";
        Engine::Get().GetWindow().SetTitle(title);

        AETHER_CORE_INFO("EditorLayer Attached.");
    }

    void EditorLayer::OnDetach()
    {
        AETHER_CORE_INFO("EditorLayer Detached.");
    }

    void EditorLayer::OnImGuiRender()
    {
        World* world = Engine::Get().GetWorld();

        if (!world) {
            ImGui::Begin("Aether Editor");
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "CRITICAL: No World Loaded!");
            ImGui::End();
            return;
        }

        // --- SYNC PANEL CONTEXT ---
        // We pass the raw Scene pointer to the panel every frame.
        // This ensures if we load a new project, the UI updates instantly.
        m_SceneHierarchyPanel.SetContext(world->GetScene());

        // --- DOCKSPACE SETUP ---
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
            EnsureLayout(dockspace_id);
            m_IsFirstFrame = false;
        }
        ImGui::End();

        // --- RENDER PANELS ---
        // This draws the "Scene Hierarchy" and "Inspector" windows
        m_SceneHierarchyPanel.OnImGuiRender();

        // --- VIEWPORT (Visual Representation) ---
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImVec2 viewportPos = ImGui::GetCursorScreenPos();
        float centerX = viewportPos.x + (viewportSize.x * 0.5f);
        float centerY = viewportPos.y + (viewportSize.y * 0.5f);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        Scene* scene = world->GetScene();
        Registry& registry = scene->GetRegistry();

        auto& transforms = registry.View<TransformComponent>();
        auto& ownerMap = registry.GetOwnerMap<TransformComponent>();

        for (size_t i = 0; i < transforms.size(); i++)
        {
            EntityID entityID = ownerMap.at(i);
            auto* sprite = registry.GetComponent<SpriteComponent>(entityID);

            if (sprite)
            {
                auto& t = transforms[i];
                float screenX = centerX + t.X;
                float screenY = centerY + t.Y;

                drawList->AddRectFilled(
                    ImVec2(screenX - (t.ScaleX * 0.5f), screenY - (t.ScaleY * 0.5f)),
                    ImVec2(screenX + (t.ScaleX * 0.5f), screenY + (t.ScaleY * 0.5f)),
                    IM_COL32(sprite->R * 255, sprite->G * 255, sprite->B * 255, sprite->A * 255)
                );
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorLayer::EnsureLayout(ImGuiID dockspace_id)
    {
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            ImGuiID dock_main_id = dockspace_id;

            // Split Right (25%) for Inspector
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

            // Split Left (20%) of the remaining main area for Hierarchy
            ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);

            // Dock the windows by name
            ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_right_id);       // Matches SceneHierarchyPanel.cpp
            ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left_id);  // Matches SceneHierarchyPanel.cpp

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}