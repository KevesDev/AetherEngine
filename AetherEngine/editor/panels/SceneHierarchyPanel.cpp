#include "SceneHierarchyPanel.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/ecs/Registry.h" 
#include <imgui.h>

namespace aether {

    SceneHierarchyPanel::SceneHierarchyPanel(Scene* context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(Scene* context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
    {
        m_SelectionContext = entity;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            auto& registry = m_Context->GetRegistry();

            auto& relationships = registry.View<RelationshipComponent>();
            auto& ownerMap = registry.GetOwnerMap<RelationshipComponent>();

            for (size_t i = 0; i < relationships.size(); i++)
            {
                if (relationships[i].Parent == NULL_ENTITY)
                {
                    EntityID entityID = ownerMap.at(i);
                    Entity entity{ entityID, &registry };
                    DrawEntityNode(entity);
                }
            }

            if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_Context->CreateEntity("Empty Entity");

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        auto& rc = entity.GetComponent<RelationshipComponent>();
        if (rc.FirstChild == NULL_ENTITY)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        // Cast to uintptr_t first to avoid pointer truncation warning on 64-bit
        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.GetID(), flags, tag.c_str());

        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            EntityID childID = rc.FirstChild;
            while (childID != NULL_ENTITY)
            {
                Entity child{ childID, entity.GetRegistry() };
                DrawEntityNode(child);
                childID = child.GetComponent<RelationshipComponent>().NextSibling;
            }

            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }
    }
}