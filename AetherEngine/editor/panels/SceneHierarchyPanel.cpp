#include "SceneHierarchyPanel.h"
#include "../../engine/ecs/Components.h"
#include "../../engine/ecs/Registry.h" 
#include <imgui.h>

namespace aether {

    SceneHierarchyPanel::SceneHierarchyPanel(Scene* context)
    {
        SetContext(context);
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(Scene* context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene>& context)
    {
        m_Context = context.get();
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

            // Iterate all entities with IDComponent
            auto view = registry.view<IDComponent>();
            for (auto entityID : view)
            {
                Entity entity{ entityID, &registry };

                // Only render root entities (no parent)
                if (entity.HasComponent<RelationshipComponent>())
                {
                    auto& rel = entity.GetComponent<RelationshipComponent>();
                    if (rel.Parent == NULL_ENTITY)
                    {
                        DrawEntityNode(entity);
                    }
                }
                else
                {
                    // Entity without relationship is implicitly a root
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

        bool hasChildren = false;
        if (entity.HasComponent<RelationshipComponent>())
        {
            auto& rc = entity.GetComponent<RelationshipComponent>();
            hasChildren = (rc.FirstChild != NULL_ENTITY);
        }

        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)entity.GetID(), flags, "%s", tag.c_str());

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
            if (entity.HasComponent<RelationshipComponent>())
            {
                auto& rc = entity.GetComponent<RelationshipComponent>();
                EntityID childID = rc.FirstChild;
                while (childID != NULL_ENTITY)
                {
                    Entity child{ childID, entity.GetRegistry() };
                    DrawEntityNode(child);

                    if (child.HasComponent<RelationshipComponent>())
                        childID = child.GetComponent<RelationshipComponent>().NextSibling;
                    else
                        break;
                }
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