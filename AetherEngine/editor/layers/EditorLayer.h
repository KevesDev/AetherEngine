#pragma once

#include "../../engine/core/Layers/Layer.h" 
#include <imgui.h>

namespace aether {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

    private:
        // Internal helper to manage the dockspace layout
        void EnsureLayout(ImGuiID dockspace_id);

        bool m_IsFirstFrame = true;

        // We track the ID, not the object, to avoid dangling references
        int m_SelectedEntityID = -1;
    };
}