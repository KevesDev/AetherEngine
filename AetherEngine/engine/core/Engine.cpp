#include "Engine.h"
#include "Log.h"
#include "AetherTime.h"
#include "EngineVersion.h"
#include <SDL_opengl.h>

namespace aether {

    /// Singleton instance pointer for the Engine.
    /// Only one Engine should be constructed at a time; creating a second instance will log an error.
    Engine* Engine::s_Instance = nullptr;

    /**
     * Engine constructor.
     *
     * Initializes the engine singleton, logs startup, and sets up a window when not running in server/headless mode.
     *
     * @param engineSpec     Engine configuration (name, type, etc.).
     * @param windowSettings Settings used to create the application window (title, width, height, vsync).
     *
     * Behavior notes:
     * - If an Engine already exists, an error is logged but construction continues (s_Instance is overwritten).
     * - When running with ApplicationType::Server the engine operates in HEADLESS mode and no Window is created.
     * - Otherwise a Window is created using the combined title format: "<Title> [<EngineVersion>]".
     * - The created Window will forward events to Engine::OnEvent via a bound callback.
     */
    Engine::Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings)
        : m_Spec(engineSpec), m_ImGuiLayer(nullptr)
    {
        if (s_Instance) {
            Log::Write(LogLevel::Error, "Engine already exists!");
        }
        s_Instance = this;

        Log::Write(LogLevel::Info, "Initializing " + m_Spec.Name + "...");

        if (m_Spec.Type == ApplicationType::Server) {
            Log::Write(LogLevel::Info, "Running in HEADLESS mode (Server Type Detected).");
        }
        else {
            std::string fullTitle = windowSettings.Title + " [" + EngineVersion::ToString() + "]";
            WindowProps props(fullTitle, windowSettings.Width, windowSettings.Height, windowSettings.Mode);
            props.VSync = windowSettings.VSync;

            m_Window = std::unique_ptr<Window>(Window::Create(props));
            m_Window->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));

            Log::Write(LogLevel::Info, "Window created: " + std::to_string(windowSettings.Width) + "x" + std::to_string(windowSettings.Height));
        }
    }

    /**
     * Engine destructor.
     *
     * Clears the singleton pointer on destruction so a new Engine may be constructed later.
     */
    Engine::~Engine()
    {
        s_Instance = nullptr;
    }

    // --- Layer Management ---
    /**
     * Push a layer onto the stack.
     *
     * The LayerStack is responsible for calling layer->OnAttach(); this method simply forwards the layer to the stack.
     *
     * @param layer Pointer to a Layer instance to be added to the stack.
     */
    void Engine::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    // --- Overlay Management ---
    /**
     * Push an overlay layer onto the stack.
     *
     * Overlays are rendered on top of regular layers. If the overlay's name is "ImGuiLayer",
     * the engine keeps a typed pointer to it so immediate-mode GUI frames can be driven from Run().
     *
     * @param layer Pointer to a Layer instance to be added as an overlay.
     */
    void Engine::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);

        if (layer->GetName() == "ImGuiLayer") {
            m_ImGuiLayer = static_cast<ImGuiLayer*>(layer);
        }
    }

    /**
     * Top-level event handler for the Engine.
     *
     * - Creates an EventDispatcher for the incoming event and registers a handler for WindowCloseEvent
     *   (mapped to Engine::OnWindowClose).
     * - Forwards the event to layers in reverse order (from topmost overlay down to the base layers),
     *   stopping early if the event is marked handled.
     *
     * @param e Reference to the incoming Event.
     */
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

    /**
     * Main application loop.
     *
     * Responsibilities performed each frame:
     * - Update global time via AetherTime.
     * - Call OnUpdate(timestep) on every layer in the stack.
     * - If an ImGui layer is present:
     *     - Begin the ImGui frame, call OnImGuiRender() on every layer, then End the ImGui frame.
     * - Poll and update the platform window (if present).
     *
     * The loop continues while m_Running is true; Window close requests set m_Running to false.
     */
    void Engine::Run()
    {
        Log::Write(LogLevel::Info, "Aether Engine starting.");
        AetherTime::Init();

        while (m_Running) {
            AetherTime::Update();
            TimeStep timestep = AetherTime::DeltaTime();

            // This paints the window a dark gray before we draw anything else
            if (m_Window) {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }

			// Update all layers
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

			// ImGui Rendering
            if (m_ImGuiLayer)
            {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

			// Update the window (swap buffers, poll events)
            if (m_Window) {
                m_Window->OnUpdate();
            }
        }
    }

    /**
     * Window close event handler.
     *
     * Marks the engine main loop to stop and returns true to indicate the event was handled.
     *
     * @param e Reference to the WindowCloseEvent.
     * @return true indicating the event was handled and the engine should shut down its run loop.
     */
    bool Engine::OnWindowClose(WindowCloseEvent& e)
    {
        Log::Write(LogLevel::Info, "Window Close Requested.");
        m_Running = false;
        return true;
    }

}