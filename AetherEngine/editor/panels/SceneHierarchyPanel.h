#pragma once
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"
#include <memory>

namespace aether {

    /**
     * SceneHierarchyPanel: Displays the entity tree for the active scene.
     * Compatible with both raw pointers and shared_ptr Scene references.
     */
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(Scene* context);
        SceneHierarchyPanel(const std::shared_ptr<Scene>& context);

        void SetContext(Scene* context);
        void SetContext(const std::shared_ptr<Scene>& context);

        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(Entity entity);

    private:
        void DrawEntityNode(Entity entity);

    private:
        Scene* m_Context = nullptr;
        Entity m_SelectionContext;
    };
}