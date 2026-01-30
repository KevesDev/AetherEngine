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

	// --- Layer Management ---
    void Engine::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

	// --- Overlay Management ---
    void Engine::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Engine::OnWindowClose, this, std::placeholders::_1));

        // --- EVENT PROPAGATION ---
        // Iterate BACKWARDS through the stack (Overlay -> Layer)
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Engine::Run()
    {
        Log::Write(LogLevel::Info, "Aether Engine starting.");
        AetherTime::Init();

        while (m_Running) {
            AetherTime::Update();
            TimeStep timestep = AetherTime::DeltaTime(); // Implicit cast from double to float

			// --- UPDATE LAYERS ---
            // Forward: Layer 0 (Game) -> Layer N (Overlay)
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

			// --- IMGUI RENDERING ---
            // We only do this if we aren't a headless server!
            if (m_ImGuiLayer)
            {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

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