#pragma once

#include "../../engine/core/Layers/Layer.h" 
#include "../../engine/ecs/Registry.h"
#include "../../engine/ecs/Entity.h"
#include "../../engine/ecs/Components.h"
#include <imgui.h>
#include <imgui_internal.h> // Required for DockBuilder & Internal Window checks

namespace aether {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer() : Layer("EditorLayer")
        {
            m_PlayerEntity = Entity(m_Registry.CreateEntity(), &m_Registry);
            m_PlayerEntity.AddComponent<TagComponent>({ "Player One" });
            m_PlayerEntity.AddComponent<TransformComponent>({ 0.0f, 0.0f, 0.0f, 100.0f, 100.0f });
            m_PlayerEntity.AddComponent<SpriteComponent>({ 1.0f, 0.2f, 0.2f, 1.0f });
        }

        virtual void OnAttach() override
        {
            // Standard: Save layout to the project root so it's consistent
            ImGui::GetIO().IniFilename = "editor.ini";
        }

        virtual void OnImGuiRender() override
        {
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
            ImGuiIO& io = ImGui::GetIO();

            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

                // --- LAYOUT INTEGRITY CHECK (Run Once) ---
                if (m_IsFirstFrame)
                {
                    EnsureLayout(dockspace_id);
                    m_IsFirstFrame = false;
                }
            }
            ImGui::End();
            // ---------------------------------------------------

            // --- VIEWPORT ---
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Viewport");

            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            ImVec2 viewportPos = ImGui::GetCursorScreenPos();
            float centerX = viewportPos.x + (viewportSize.x * 0.5f);
            float centerY = viewportPos.y + (viewportSize.y * 0.5f);

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Render Entities
            auto& transforms = m_Registry.View<TransformComponent>();
            for (size_t i = 0; i < transforms.size(); i++)
            {
                EntityID entityID = m_Registry.GetOwnerMap<TransformComponent>().at(i);
                if (m_Registry.HasComponent<SpriteComponent>(entityID))
                {
                    auto& transform = transforms[i];
                    auto& sprite = m_Registry.GetComponent<SpriteComponent>(entityID);

                    // Draw centered
                    float screenX = centerX + transform.X;
                    float screenY = centerY + transform.Y;
                    float halfWidth = transform.ScaleX * 0.5f;
                    float halfHeight = transform.ScaleY * 0.5f;

                    drawList->AddRectFilled(
                        ImVec2(screenX - halfWidth, screenY - halfHeight),
                        ImVec2(screenX + halfWidth, screenY + halfHeight),
                        IM_COL32(sprite.R * 255, sprite.G * 255, sprite.B * 255, sprite.A * 255)
                    );
                }
            }

            ImGui::End();
            ImGui::PopStyleVar();

            // --- INSPECTOR ---
            ImGui::Begin("Aether Inspector");
            if (m_PlayerEntity)
            {
                auto& t = m_PlayerEntity.GetComponent<TransformComponent>();
                ImGui::Text("ID: %d | Tag: %s", m_PlayerEntity.GetID(), m_PlayerEntity.GetComponent<TagComponent>().Tag.c_str());
                ImGui::Separator();
                ImGui::DragFloat("X", &t.X, 1.0f);
                ImGui::DragFloat("Y", &t.Y, 1.0f);
                ImGui::DragFloat("W", &t.ScaleX, 1.0f);
                ImGui::DragFloat("H", &t.ScaleY, 1.0f);
                ImGui::ColorEdit4("Color", &m_PlayerEntity.GetComponent<SpriteComponent>().R);
            }
            ImGui::End();
        }

    private:
        void EnsureLayout(ImGuiID dockspace_id)
        {
            // 1. Check if the "Viewport" window is currently docked.
            // ImGui::FindWindowByName is an internal function that lets us inspect window state.
            ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");

            // INTEGRITY CHECK:
            // If the window is found BUT it has no Dock Node, it means it's floating (The Tiny Box Issue).
            // Or if the dockspace node itself doesn't exist, we must build.
            bool isViewportDocked = (viewportWindow && viewportWindow->DockNode != nullptr && !viewportWindow->DockNode->IsHiddenTabBar());
            bool isDockspaceCreated = ImGui::DockBuilderGetNode(dockspace_id) != nullptr;

            if (!isDockspaceCreated || !isViewportDocked)
            {
                // Force Rebuild
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear garbage
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

                // Create the Split: Right side (Inspector), Remaining (Viewport)
                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

                // Assign Windows to IDs
                ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
                ImGui::DockBuilderDockWindow("Aether Inspector", dock_right_id);

                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        bool m_IsFirstFrame = true;
        Registry m_Registry;
        Entity m_PlayerEntity;
    };
}