#include "Engine.h"
#include "Log.h"
#include "AetherTime.h"

namespace aether {

    Engine::Engine()
    {
        Log::Write(LogLevel::Info, "Aether Engine initialized.");

        // 1. Create the Window (Abstracted)
        // This calls SDLWindow internally because of your Factory in SDLWindow.cpp
        m_Window = std::unique_ptr<Window>(Window::Create());

        // 2. Bind the Event Callback
        // This tells the Window: "When something happens, call Engine::OnEvent"
        m_Window->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));
    }

    Engine::~Engine()
    {
    }

    void Engine::OnEvent(Event& e)
    {
        // Example: Log every event to console (Great for debugging Phase 3)
        // Log::Write(LogLevel::Trace, e.ToString());

        // Dispatcher: Routes the event to the correct function
        EventDispatcher dispatcher(e);

        // If the event is "WindowClose", call "OnWindowClose"
        dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Engine::OnWindowClose, this, std::placeholders::_1));
    }

    void Engine::Run()
    {
        Log::Write(LogLevel::Info, "Aether Engine starting.");
        AetherTime::Init();

        while (m_Running) {
            AetherTime::Update();

            // Optional: Log time occasionally, not every frame (spammy)
            // Log::Write(LogLevel::Info, "dt: " + std::to_string(AetherTime::DeltaTime()));

            // 3. Update the Window (Poll inputs, Swap buffers)
            m_Window->OnUpdate();
        }
    }

    bool Engine::OnWindowClose(WindowCloseEvent& e)
    {
        Log::Write(LogLevel::Info, "Window Close Requested.");
        m_Running = false;
        return true;
    }

}