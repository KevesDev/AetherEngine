#pragma once
#include "EditorPanel.h"
#include "../../engine/scene/Scene.h"
#include "../../engine/ecs/Entity.h"

namespace aether {

    class SceneHierarchyPanel : public EditorPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(Scene* scene); // Raw pointer constructor

        // We set the context (Which scene are we looking at?)
        void SetContext(Scene* scene); // Raw pointer setter

        virtual void OnImGuiRender() override;

        Entity GetSelectedEntity() const { return m_SelectionContext; }
        void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

    private:
        // Recursive draw function for the Scene Graph
        void DrawEntityNode(Entity entity);

        // Inspector UI: Draws components for the selected entity
        void DrawComponents(Entity entity);

        Scene* m_Context = nullptr; // Raw pointer
        Entity m_SelectionContext;
    };

}