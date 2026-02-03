#include "Engine.h"
#include "../renderer/Renderer2D.h"
#include "../scene/World.h"
#include "Log.h"
#include "AetherTime.h"
#include "EngineVersion.h"
#include "VFS.h"

namespace aether {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings)
        : m_Spec(engineSpec), m_ImGuiLayer(nullptr)
    {
        Log::Init();
        AetherTime::Init();

        AETHER_ASSERT(!s_Instance, "Engine already exists!");
        s_Instance = this;

        VFS::Mount("/assets", "assets");

        if (m_Spec.Type != ApplicationType::Server)
        {
            WindowProps props(windowSettings.Title, windowSettings.Width, windowSettings.Height, windowSettings.Mode);
            props.VSync = windowSettings.VSync;
            m_Window = std::unique_ptr<Window>(Window::Create(props));
            m_Window->SetEventCallback(AETHER_BIND_EVENT_FN(Engine::OnEvent));
            Renderer2D::Init();
        }
    }

    Engine::~Engine() { Renderer2D::Shutdown(); }

    void Engine::Close() { m_Running = false; }

    void Engine::PushLayer(Layer* layer) { m_LayerOperations.emplace_back([this, layer]() { m_LayerStack.PushLayer(layer); }); }
    void Engine::PushOverlay(Layer* layer) { m_LayerOperations.emplace_back([this, layer]() { m_LayerStack.PushOverlay(layer); }); }
    void Engine::PopLayer(Layer* layer) { m_LayerOperations.emplace_back([this, layer]() { m_LayerStack.PopLayer(layer); }); }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(AETHER_BIND_EVENT_FN(Engine::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& e) {
            Renderer2D::OnWindowResize(e.GetWidth(), e.GetHeight());
            return false;
            });

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
            if (e.Handled) break;
            (*it)->OnEvent(e);
        }
    }

    void Engine::SetWorld(std::unique_ptr<World> newWorld) {
        m_ActiveWorld = std::move(newWorld);
        m_ActiveWorld->OnRuntimeStart();
    }

    void Engine::Run()
    {
        while (m_Running)
        {
            if (!m_LayerOperations.empty()) {
                for (auto& op : m_LayerOperations) op();
                m_LayerOperations.clear();
            }

            AetherTime::Update();
            float timestep = (float)AetherTime::DeltaTime();

            // 1. Update Layers (Game Logic)
            // Client/Editor layers are responsible for calling World::OnUpdate with their specific Camera
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            // 2. Render UI
            if (m_ImGuiLayer) {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack) layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

            // 3. Swap Buffers
            if (m_Window) m_Window->OnUpdate();
        }
    }

    bool Engine::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
}