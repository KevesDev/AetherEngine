#pragma once

#include "Layer.h"
#include "../../events/ApplicationEvent.h"
#include "../../events/KeyEvent.h"
#include "../../events/MouseEvent.h"

namespace aether {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& e) override;

        void Begin();
        void End();

        void SetBlockEvents(bool block) { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
    };

}