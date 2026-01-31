#include "Engine.h"
#include "Log.h"
#include "AetherTime.h"
#include "EngineVersion.h"
#include "VFS.h"

// NOTE: No Platform/Graphics headers allowed here. 
// The Engine uses the Window interface for all rendering commands.

namespace aether {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings)
        : m_Spec(engineSpec), m_ImGuiLayer(nullptr)
    {
        // Strict Singleton Enforcement
        // If this triggers, we have a major architectural flaw in the application entry point.
        AETHER_ASSERT(!s_Instance, "Engine already exists! Do not instantiate multiple Engines.");
        s_Instance = this;

        AETHER_CORE_INFO("Initializing {0}...", m_Spec.Name);

        // --- VFS INITIALIZATION ---
        // Mount the local "assets" folder to the virtual root "/assets"
        // TODO: In the future, this will be replaced by mounting a .pak file for Release builds.
        VFS::Mount("/assets", "assets");
        // --------------------------

        // Headless vs Windowed Initialization
        if (m_Spec.Type == ApplicationType::Server)
        {
            AETHER_CORE_INFO("Running in HEADLESS mode (Server Type Detected).");
            // No Window created. m_Window remains null.
        }
        else
        {
            std::string fullTitle = windowSettings.Title + " [" + EngineVersion::ToString() + "]";

            WindowProps props(fullTitle, windowSettings.Width, windowSettings.Height, windowSettings.Mode);
            props.VSync = windowSettings.VSync;

            m_Window = std::unique_ptr<Window>(Window::Create(props));

            // Validate Window Creation using our new macro
            AETHER_ASSERT(m_Window, "Window failed to create!");

            m_Window->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));

            AETHER_CORE_INFO("Window Initialized: {0}x{1} (VSync: {2})",
                windowSettings.Width, windowSettings.Height, windowSettings.VSync ? "On" : "Off");
        }
    }

    Engine::~Engine()
    {
        s_Instance = nullptr;
    }

    void Engine::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
		// layer->OnAttach(); handled by LayerStack
    }

    void Engine::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
		// layer->OnAttach(); handled by LayerStack

        if (layer->GetName() == "ImGuiLayer") {
            m_ImGuiLayer = static_cast<ImGuiLayer*>(layer);
        }
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Engine::OnWindowClose, this, std::placeholders::_1));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled) break;
            (*it)->OnEvent(e);
        }
    }

    // --- World Management ---
    void Engine::SetWorld(std::unique_ptr<World> newWorld)
    {
        // Move assignment cleanly destroys the previous world
        m_ActiveWorld = std::move(newWorld);

        if (m_ActiveWorld) {
            AETHER_CORE_INFO("World Loaded: {0}", m_ActiveWorld->GetName());
        }
    }

    void Engine::Run()
    {
        AETHER_CORE_INFO("Aether Engine Loop Started.");
        AetherTime::Init();

        while (m_Running) {
            // 1. Time Management
            AetherTime::Update();
            TimeStep timestep = AetherTime::DeltaTime();

            // 2. Render Prep (Abstracted)
            // If we have a window, we clear it via the interface. 
            // If headless, we skip this entirely.
            if (m_Window) {
                m_Window->Clear();
            }

            // 3. Simulation Step (The World Loop)
            // This runs the ECS, Logic, and Physics.
            // Safe to run without a window (Headless Server).
            if (m_ActiveWorld) {
                m_ActiveWorld->OnUpdate(timestep);
            }

            // 4. Layer Update (Editor, UI, Overlays)
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            // 5. ImGui Rendering (Client/Editor Only)
            if (m_ImGuiLayer)
            {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

            // 6. Window Swap & Poll
            if (m_Window) {
                m_Window->OnUpdate();
            }
        }
    }

    bool Engine::OnWindowClose(WindowCloseEvent& e)
    {
        AETHER_CORE_INFO("Window Close Requested. Shutting down.");
        m_Running = false;
        return true;
    }

}