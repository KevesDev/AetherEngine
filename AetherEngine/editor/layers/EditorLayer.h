#pragma once
#include "../../engine/core/Layers/Layer.h"
#include "../panels/SceneHierarchyPanel.h"

namespace aether {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(TimeStep ts) override {}
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& e) override {}

    private:
        void EnsureLayout(unsigned int dockspace_id);

    private:
        bool m_IsFirstFrame = true;

        // --- The Panel System ---
        SceneHierarchyPanel m_SceneHierarchyPanel;
    };
}