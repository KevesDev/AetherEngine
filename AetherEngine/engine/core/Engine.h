#pragma once
#include "../platform/Window.h"
#include "../events/Event.h"
#include "../events/ApplicationEvent.h"
#include "Layers/LayerStack.h"
#include "Layers/ImGuiLayer.h"
#include "../scene/World.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>

namespace aether {

    enum class ApplicationType {
        Client,
        Server,
        Editor
    };

	// Immutable engine specification
    struct EngineSpecification {
        std::string Name = "Aether Engine";
        ApplicationType Type = ApplicationType::Client;
    };

	// User-configurable window settings
    struct WindowSettings {
        std::string Title = "Aether Application";
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool VSync = true;
        WindowMode Mode = WindowMode::Maximized;
    };


    class Engine
    {
    public:
		// Constructor - Takes EngineSpecification and optional WindowSettings
        Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings = WindowSettings());
        virtual ~Engine();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);

        static Engine& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }

        // Getters so we can check "Am I a Server?" later
        const EngineSpecification& GetSpec() const { return m_Spec; }

        // --- World Management API ---
        // The Application (Client/Editor) calls this to load a specific map/level.
        void SetWorld(std::unique_ptr<World> newWorld);

        // Returns null if no world is loaded (e.g., during startup or main menu)
        World* GetWorld() { return m_ActiveWorld.get(); }

    private:
        // Handles specific events (e.g., clicking X to close)
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        EngineSpecification m_Spec;
        LayerStack m_LayerStack;
        static Engine* s_Instance;
        ImGuiLayer* m_ImGuiLayer;

        // This holds the current ECS Registry and Simulation State.
        std::unique_ptr<World> m_ActiveWorld;

        // --- Operation Queue ---
        // Stores pending layer operations to be executed safely at start of frame
        std::vector<std::function<void()>> m_LayerOperations;
    };
}