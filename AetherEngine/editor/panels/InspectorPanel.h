#pragma once
#include "../../engine/ecs/Entity.h"

namespace aether {

    class InspectorPanel
    {
    public:
        InspectorPanel() = default;

        void SetContext(Entity entity);
        void OnImGuiRender();

    private:
        void DrawComponents(Entity entity);

    private:
        Entity m_SelectionContext;
    };
}