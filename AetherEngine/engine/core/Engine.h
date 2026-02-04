#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "../platform/Window.h"
#include "Layers/LayerStack.h"
#include "Layers/ImGuiLayer.h"

#include "../events/Event.h"
#include "../events/ApplicationEvent.h"
#include "../core/AetherTime.h"

namespace aether {

    /**
     * ApplicationType
     * Defines the runtime mode of the engine.
     * - Client: Standard game with Rendering, Audio, and Input polling.
     * - Server: Headless authoritative simulation. No Window, No GPU context.
	 * - Editor: Editing and creating projects; includes rendering and input.
     */
    enum class ApplicationType {
        Client = 0,
        Server = 1,
		Editor = 2
    };

    struct EngineSpecification {
        std::string Name = "Aether Engine";
        uint32_t Width = 1600;
        uint32_t Height = 900;
        ApplicationType Type = ApplicationType::Client;
        std::string WorkingDirectory;
    };

    /**
     * Engine
     * The root lifecycle manager for the application.
     * Handles the main loop, window creation, layer stack management, and system initialization.
     * * * ARCHITECTURAL NOTE:
     * The Engine ensures that the Simulation Loop (Fixed-Step) and Presentation Loop (Variable-Step)
     * are driven correctly. It acts as the host for the "World" (Scene).
     */
    class Engine {
    public:
        Engine(const EngineSpecification& spec);
        virtual ~Engine();

        void Run();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* layer);

        Window& GetWindow() { return *m_Window; }
        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

        static Engine& Get() { return *s_Instance; }
        ApplicationType GetAppType() const { return m_Spec.Type; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        EngineSpecification m_Spec;
        bool m_Running = true;
        bool m_Minimized = false;

        // Host Window (Client Only)
        // Nullptr in Server builds to ensure headless compliance.
        std::unique_ptr<Window> m_Window;

        // Debug/Editor Overlay (Client Only)
        ImGuiLayer* m_ImGuiLayer = nullptr;

        // Layer Management
        LayerStack m_LayerStack;
        std::vector<std::function<void()>> m_LayerOperations;

        // Timekeeping
        float m_LastFrameTime = 0.0f;

        static Engine* s_Instance;
    };

    // Defined by the client application to provide the entry point
    Engine* CreateApplication(int argc, char** argv);
}