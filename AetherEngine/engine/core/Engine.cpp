#include "Engine.h"
#include "Log.h"
#include "Config.h"
#include "EngineVersion.h"
#include "../input/Input.h"
#include "../renderer/Renderer2D.h"

// Systems Infrastructure
#include "systems/SystemRegistry.h"
#include "systems/InputSystem.h"

// Events
#include "../events/ApplicationEvent.h"
#include "../events/KeyEvent.h"
#include "../events/MouseEvent.h"

namespace aether {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& spec)
        : m_Spec(spec), m_Running(true)
    {
        s_Instance = this;

        // Initialize Core Logger
        Log::Init();

        // Initialize Configuration
		// TODO: Check if needed - Config has LoadBootConfig called from editor/client entrypoints.
        //Config::Init();

        // ------------------------------------------------------------
        // Core System Registration
        // Registers fundamental systems so they can be instantiated by Scenes via data.
        // This adheres to our decoupling requirements.
        // ------------------------------------------------------------
        SystemRegistry::Register<InputSystem>("InputSystem");

        // Create the Window (Client/Editor only)
        // Server builds bypass window creation as per "Headless" requirement.
        if (m_Spec.Type != ApplicationType::Server) {
            WindowProps props(m_Spec.Name, m_Spec.Width, m_Spec.Height);
            m_Window = std::unique_ptr<Window>(Window::Create(props));
            m_Window->SetEventCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));
        }

        // Initialize Renderer (Client/Editor only)
        if (m_Spec.Type != ApplicationType::Server) {
            Renderer2D::Init();
        }

        // Initialize ImGui Layer (Client/Editor only)
        if (m_Spec.Type != ApplicationType::Server) {
            m_ImGuiLayer = new ImGuiLayer();
            PushOverlay(m_ImGuiLayer);
        }
    }

    Engine::~Engine() {
        if (m_Spec.Type != ApplicationType::Server) {
            Renderer2D::Shutdown();
        }
    }

    void Engine::Run() {
        while (m_Running) {
            // Process Layer Operations (Push/Pop) safely at start of frame
            if (!m_LayerOperations.empty()) {
                for (auto& op : m_LayerOperations) op();
                m_LayerOperations.clear();
            }

            // -----------------------------------------------------------------
            // 1. Frame Clock Update (Real Time)
            // -----------------------------------------------------------------
            // This computes the variable frame delta used for rendering/UI/editor.
            AetherTime::UpdateFrame();
            float frameDelta = static_cast<float>(AetherTime::GetFrameDelta());
            TimeStep frameTimeStep(frameDelta);

            // 2. Core Window/Input Update (Client/Editor only)
            if (m_Window) {
                m_Window->OnUpdate();
            }

            // 3. Application Layer Update (Variable Step)
            // NOTE:
            // - Editor/UI layers should use the variable frame delta only.
            // - Authoritative gameplay simulation is executed via Scene::OnUpdateSimulation,
            //   which internally uses SystemScheduler with a fixed timestep.
            if (!m_Minimized) {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(frameTimeStep);
            }

            // 4. ImGui Render (Editor/UI)
            if (m_ImGuiLayer && m_Spec.Type != ApplicationType::Server) {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }
        }
    }

    void Engine::OnEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Engine::OnWindowClose, this, std::placeholders::_1));
        dispatcher.Dispatch<WindowResizeEvent>(std::bind(&Engine::OnWindowResize, this, std::placeholders::_1));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
            if (e.Handled) break;
            (*it)->OnEvent(e);
        }
    }

    void Engine::PushLayer(Layer* layer) {
        m_LayerOperations.push_back([this, layer]() {
            m_LayerStack.PushLayer(layer);
            layer->OnAttach();
            });
    }

    void Engine::PushOverlay(Layer* layer) {
        m_LayerOperations.push_back([this, layer]() {
            m_LayerStack.PushOverlay(layer);
            layer->OnAttach();
            });
    }

    void Engine::PopLayer(Layer* layer) {
        m_LayerOperations.push_back([this, layer]() {
            m_LayerStack.PopLayer(layer);
            layer->OnDetach();
            });
    }

    void Engine::PopOverlay(Layer* layer) {
        m_LayerOperations.push_back([this, layer]() {
            m_LayerStack.PopOverlay(layer);
            layer->OnDetach();
            });
    }

    bool Engine::OnWindowClose(WindowCloseEvent& e) {
        m_Running = false;
        return true;
    }

    bool Engine::OnWindowResize(WindowResizeEvent& e) {
        if (e.GetWidth() == 0 || e.GetHeight() == 0) {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer2D::OnWindowResize(e.GetWidth(), e.GetHeight());
        return false;
    }
}