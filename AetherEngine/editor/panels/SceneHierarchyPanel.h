#pragma once
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"

namespace aether {

    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        // Takes raw pointer because World owns the scene exclusively
        SceneHierarchyPanel(Scene* context);

        void SetContext(Scene* context);
        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(Entity entity);

    private:
        void DrawEntityNode(Entity entity);

    private:
        Scene* m_Context = nullptr; // : Raw pointer
        Entity m_SelectionContext;
    };
}