#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include "../../engine/ecs/Components.h"

namespace aether {

    void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene>& scene)
    {
        m_Context = scene;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            auto& registry = m_Context->GetRegistry();

            auto& tags = registry.View<TagComponent>();
            auto& ownerMap = registry.GetOwnerMap<TagComponent>();

            for (size_t i = 0; i < tags.size(); i++)
            {
                EntityID id = ownerMap.at(i);

                // Use 'auto*' or 'RelationshipComponent*' because GetComponent returns a pointer
                if (registry.HasComponent<RelationshipComponent>(id)) {
                    auto* relationship = registry.GetComponent<RelationshipComponent>(id);
                    if (relationship->Parent != NULL_ENTITY)
                        continue; // Skip children, they will be drawn by their parent
                }

                Entity entity{ id, &registry };
                DrawEntityNode(entity);
            }

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
                m_SelectionContext = {};
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool hasChildren = false;
        if (entity.HasComponent<RelationshipComponent>()) {
            // Entity::GetComponent calls Registry::GetComponent, checks for null, and returns *component (Ref).
            // So here we use dot (.) syntax.
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

        if (opened && hasChildren)
        {
            // Use reference here since Entity::GetComponent returns T&
            auto& relation = entity.GetComponent<RelationshipComponent>();
            EntityID currentChildID = relation.FirstChild;

            while (currentChildID != NULL_ENTITY) {
                Entity childEntity{ currentChildID, entity.GetRegistry() };

                DrawEntityNode(childEntity);

                if (childEntity.HasComponent<RelationshipComponent>()) {
                    // Entity::GetComponent returns T&
                    currentChildID = childEntity.GetComponent<RelationshipComponent>().NextSibling;
                }
                else {
                    break;
                }
            }

            ImGui::TreePop();
        }
    }
}