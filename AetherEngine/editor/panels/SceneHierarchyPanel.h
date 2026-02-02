#pragma once
#include "EditorPanel.h"
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"

namespace aether {

    class SceneHierarchyPanel : public EditorPanel
    {
    public:
        SceneHierarchyPanel() = default;

        // We set the context (Which scene are we looking at?)
        void SetContext(const std::shared_ptr<Scene>& scene);

        virtual void OnImGuiRender() override;

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

    private:
        // Recursive draw function for the Scene Graph
        void DrawEntityNode(Entity entity);

        std::shared_ptr<Scene> m_Context;
        Entity m_SelectionContext;
    };

}