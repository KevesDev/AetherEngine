#pragma once
#include "../platform/Window.h"
#include "../events/Event.h"
#include "../events/ApplicationEvent.h"
#include <memory>

namespace aether {

    class Engine
    {
    public:
        Engine();
        virtual ~Engine();

        void Run();

        // This is the function that receives ALL events from the window
        void OnEvent(Event& e);

    private:
        // Handles specific events (e.g., clicking X to close)
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

}