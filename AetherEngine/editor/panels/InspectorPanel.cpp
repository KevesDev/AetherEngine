#include "InspectorPanel.h"
#include "../../engine/ecs/Components.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <string>

namespace aether {

    void InspectorPanel::SetContext(Entity entity)
    {
        m_SelectionContext = entity;
    }

    void InspectorPanel::OnImGuiRender()
    {
        ImGui::Begin("Inspector");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        ImGui::End();
    }

    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        // Use Public API instead of internal GImGui->Font
        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        if (ImGui::Button("X", buttonSize)) values.x = resetValue;
        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    void InspectorPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, tag.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                if (!m_SelectionContext.HasComponent<CameraComponent>())
                    m_SelectionContext.AddComponent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Sprite Renderer"))
            {
                if (!m_SelectionContext.HasComponent<SpriteComponent>())
                    m_SelectionContext.AddComponent<SpriteComponent>();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        if (entity.HasComponent<TransformComponent>())
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& tc = entity.GetComponent<TransformComponent>();
                glm::vec3 pos = { tc.X, tc.Y, 0.0f };
                DrawVec3Control("Position", pos);
                tc.X = pos.x; tc.Y = pos.y;

                glm::vec3 rot = { 0.0f, 0.0f, tc.Rotation };
                DrawVec3Control("Rotation", rot);
                tc.Rotation = rot.z;

                glm::vec3 scale = { tc.ScaleX, tc.ScaleY, 1.0f };
                DrawVec3Control("Scale", scale, 1.0f);
                tc.ScaleX = scale.x; tc.ScaleY = scale.y;
            }
        }

        if (entity.HasComponent<CameraComponent>())
        {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& cameraComp = entity.GetComponent<CameraComponent>();

                ImGui::Checkbox("Primary", &cameraComp.Primary);

                const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
                const char* currentProjectionTypeString = projectionTypeStrings[(int)cameraComp.ProjectionType];

                if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                        if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                        {
                            currentProjectionTypeString = projectionTypeStrings[i];
                            cameraComp.ProjectionType = (CameraComponent::Type)i;
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (cameraComp.ProjectionType == CameraComponent::Type::Orthographic)
                {
                    ImGui::DragFloat("Size", &cameraComp.OrthographicSize);
                    ImGui::DragFloat("Near Clip", &cameraComp.OrthographicNear);
                    ImGui::DragFloat("Far Clip", &cameraComp.OrthographicFar);
                }

                if (cameraComp.ProjectionType == CameraComponent::Type::Perspective)
                {
                    float fovDeg = glm::degrees(cameraComp.PerspectiveFOV);
                    if (ImGui::DragFloat("FOV", &fovDeg))
                        cameraComp.PerspectiveFOV = glm::radians(fovDeg);

                    ImGui::DragFloat("Near Clip", &cameraComp.PerspectiveNear);
                    ImGui::DragFloat("Far Clip", &cameraComp.PerspectiveFar);
                }
            }
        }

        if (entity.HasComponent<SpriteComponent>())
        {
            if (ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& sprite = entity.GetComponent<SpriteComponent>();
                ImGui::ColorEdit4("Color", &sprite.R);
            }
        }
    }
}