#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "../../engine/ecs/Components.h"

namespace aether {

    SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene)
    {
        SetContext(scene);
    }

    void SceneHierarchyPanel::SetContext(Scene* scene)
    {
        m_Context = scene;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        // --- SCENE HIERARCHY ---
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            auto& registry = m_Context->GetRegistry();

            // Right-click on blank space to create entity
            if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }

            auto& tags = registry.View<TagComponent>();
            auto& ownerMap = registry.GetOwnerMap<TagComponent>();

            for (size_t i = 0; i < tags.size(); i++)
            {
                EntityID id = ownerMap.at(i);

                // Skip children, they are drawn recursively by their parent
                if (registry.HasComponent<RelationshipComponent>(id)) {
                    // Raw pointer access for speed here is fine
                    auto* relationship = registry.GetComponent<RelationshipComponent>(id);
                    if (relationship && relationship->Parent != NULL_ENTITY)
                        continue;
                }

                Entity entity{ id, &registry };
                DrawEntityNode(entity);
            }

            // Click empty space to deselect
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_SelectionContext = {};
        }

        ImGui::End(); // End Hierarchy

        // --- INSPECTOR ---
        ImGui::Begin("Inspector");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        else
        {
            ImGui::Text("No entity selected.");
        }
        ImGui::End(); // End Inspector
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool hasChildren = false;
        if (entity.HasComponent<RelationshipComponent>()) {
            if (entity.GetComponent<RelationshipComponent>().ChildrenCount > 0) {
                hasChildren = true;
            }
        }

        if (!hasChildren) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.GetID(), flags, tag.c_str());

        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        // Context Menu (Right-click entity)
        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened && hasChildren)
        {
            auto& relation = entity.GetComponent<RelationshipComponent>();
            EntityID currentChildID = relation.FirstChild;

            while (currentChildID != NULL_ENTITY) {
                Entity childEntity{ currentChildID, entity.GetRegistry() };

                DrawEntityNode(childEntity);

                if (childEntity.HasComponent<RelationshipComponent>()) {
                    currentChildID = childEntity.GetComponent<RelationshipComponent>().NextSibling;
                }
                else {
                    break;
                }
            }

            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            if (m_Context) {
                m_Context->DestroyEntity(entity);
                if (m_SelectionContext == entity)
                    m_SelectionContext = {};
            }
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        // Tag (Name)
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tag.c_str());
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
            if (!entity.HasComponent<CameraComponent>()) {
                if (ImGui::MenuItem("Camera")) {
                    entity.AddComponent<CameraComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<SpriteComponent>()) {
                if (ImGui::MenuItem("Sprite")) {
                    entity.AddComponent<SpriteComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        ImGui::Separator();

        // Transform
        if (entity.HasComponent<TransformComponent>())
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& tc = entity.GetComponent<TransformComponent>();
                ImGui::DragFloat("X", &tc.X, 0.1f);
                ImGui::DragFloat("Y", &tc.Y, 0.1f);
                ImGui::DragFloat("Scale X", &tc.ScaleX, 0.1f);
                ImGui::DragFloat("Scale Y", &tc.ScaleY, 0.1f);
                ImGui::DragFloat("Rotation", &tc.Rotation, 0.1f);
            }
        }

        // Camera
        if (entity.HasComponent<CameraComponent>())
        {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& cc = entity.GetComponent<CameraComponent>();
                ImGui::Checkbox("Primary", &cc.Primary);
                ImGui::DragFloat("Size", &cc.Size, 0.1f);
                ImGui::DragFloat("Near", &cc.Near);
                ImGui::DragFloat("Far", &cc.Far);
            }
        }

        // Sprite
        if (entity.HasComponent<SpriteComponent>())
        {
            if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& src = entity.GetComponent<SpriteComponent>();
                ImGui::ColorEdit4("Color", &src.R);
            }
        }
    }
}