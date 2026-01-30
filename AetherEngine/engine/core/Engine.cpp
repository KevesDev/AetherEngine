#include "Engine.h"
#include "Log.h"
#include "AetherTime.h"
#include "EngineVersion.h"

namespace aether {

	// --- Constructor / Destructor ---
    Engine::Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings)
        : m_Spec(engineSpec) // Store the spec
    {
        Log::Write(LogLevel::Info, "Initializing " + m_Spec.Name + "...");


        // LOGIC: Check the Application Type to decide on Window creation
        if (m_Spec.Type == ApplicationType::Server) {
            Log::Write(LogLevel::Info, "Running in HEADLESS mode (Server Type Detected).");
            // Server -> No Window, regardless of WindowSettings
        }

        else {
            // Client/Editor -> Create Window using WindowSettings

            // Format Title: "Title [Version]"
            std::string fullTitle = windowSettings.Title + " [" + EngineVersion::ToString() + "]";

            WindowProps props(fullTitle, windowSettings.Width, windowSettings.Height);
            props.VSync = windowSettings.VSync;

            m_Window = std::unique_ptr<Window>(Window::Create(props));
            m_Window->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));

            Log::Write(LogLevel::Info, "Window created: " + std::to_string(windowSettings.Width) + "x" + std::to_string(windowSettings.Height));
        }
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
			// Only update window if it exists (i.e., not a Server)
            if (m_Window) {
                m_Window->OnUpdate();
            }
        }
    }

    bool Engine::OnWindowClose(WindowCloseEvent& e)
    {
        Log::Write(LogLevel::Info, "Window Close Requested.");
        m_Running = false;
        return true;
    }

}