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

    struct EngineSpecification {
        std::string Name = "Aether Engine";
        ApplicationType Type = ApplicationType::Client;
    };

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
        Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings = WindowSettings());
        virtual ~Engine();

        void Run();
        void OnEvent(Event& e);

        // Public API for shutdown
        void Close();

        // API queues operations to be executed safely at the start of the frame
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);

        static Engine& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }

        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        void SetImGuiLayer(ImGuiLayer* layer) { m_ImGuiLayer = layer; }

        const EngineSpecification& GetSpec() const { return m_Spec; }

        void SetWorld(std::unique_ptr<World> newWorld);
        World* GetWorld() { return m_ActiveWorld.get(); }

    private:
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        EngineSpecification m_Spec;
        LayerStack m_LayerStack;
        static Engine* s_Instance;
        ImGuiLayer* m_ImGuiLayer;

        std::unique_ptr<World> m_ActiveWorld;

        // Command Queue for safe layer operations
        std::vector<std::function<void()>> m_LayerOperations;
    };
}