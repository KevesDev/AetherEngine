#include "EditorLayer.h"
#include "../../engine/core/Engine.h" 
#include "../../engine/scene/World.h"
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/ecs/Entity.h"
#include "../../engine/core/Log.h" 

#include <imgui_internal.h>

namespace aether {

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        AETHER_ASSERT(ImGui::GetCurrentContext(), "EditorLayer attached but ImGui Context is missing!");
        ImGui::GetIO().IniFilename = "editor.ini";
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

        Scene* scene = world->GetScene();
        Registry& registry = scene->GetRegistry();

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

        // --- VIEWPORT (Scene Hierarchy View) ---
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImVec2 viewportPos = ImGui::GetCursorScreenPos();
        float centerX = viewportPos.x + (viewportSize.x * 0.5f);
        float centerY = viewportPos.y + (viewportSize.y * 0.5f);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // RENDER LOOP: Safely iterate the Engine's Registry for the UI Viewport
        auto& transforms = registry.View<TransformComponent>();
        auto& ownerMap = registry.GetOwnerMap<TransformComponent>();

        for (size_t i = 0; i < transforms.size(); i++)
        {
            EntityID entityID = ownerMap.at(i);

            // Accessing Component via Pointer (Safe Pattern)
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

        // --- INSPECTOR ---
        ImGui::Begin("Aether Inspector");

        if (m_SelectedEntityID != -1)
        {
            Entity selectedEntity((EntityID)m_SelectedEntityID, &registry);

            // Re-validate that the entity still exists by checking its Tag pointer
            auto* tag = registry.GetComponent<TagComponent>((EntityID)m_SelectedEntityID);
            if (tag)
            {
                ImGui::Text("Selected: %s (ID: %d)", tag->Tag.c_str(), m_SelectedEntityID);
                ImGui::Separator();

                if (selectedEntity.HasComponent<TransformComponent>()) {
                    auto& t = selectedEntity.GetComponent<TransformComponent>();
                    ImGui::DragFloat("X", &t.X);
                    ImGui::DragFloat("Y", &t.Y);
                    ImGui::DragFloat("Scale X", &t.ScaleX);
                    ImGui::DragFloat("Scale Y", &t.ScaleY);
                }

                if (selectedEntity.HasComponent<SpriteComponent>()) {
                    auto& s = selectedEntity.GetComponent<SpriteComponent>();
                    ImGui::ColorEdit4("Color", &s.R);
                }

                if (selectedEntity.HasComponent<CameraComponent>()) {
                    auto& c = selectedEntity.GetComponent<CameraComponent>();
                    ImGui::Separator();
                    ImGui::Text("Camera Settings");
                    ImGui::DragFloat("Size (Zoom)", &c.Size);
                    ImGui::Checkbox("Primary Camera", &c.Primary);
                }
            }
            else {
                m_SelectedEntityID = -1;
            }
        }
        else {
            ImGui::Text("No Entity Selected");
            if (!transforms.empty()) {
                m_SelectedEntityID = (int)ownerMap.at(0);
            }
        }
        ImGui::End();
    }

    void EditorLayer::EnsureLayout(ImGuiID dockspace_id)
    {
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

            ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
            ImGui::DockBuilderDockWindow("Aether Inspector", dock_right_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}