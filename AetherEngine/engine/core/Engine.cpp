#include "Engine.h"
#include "../renderer/Renderer2D.h"
#include "../renderer/CameraUtils.h"
#include "../ecs/Components.h"
#include "../ecs/Registry.h" 
#include "../scene/World.h"
#include "Log.h"
#include "AetherTime.h"
#include "EngineVersion.h"
#include "VFS.h"
#include <glm/gtc/matrix_transform.hpp>

namespace aether {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings)
        : m_Spec(engineSpec), m_ImGuiLayer(nullptr)
    {
        Log::Init();
        AetherTime::Init();

        AETHER_ASSERT(!s_Instance, "Engine already exists!");
        s_Instance = this;

        AETHER_CORE_INFO("Initializing {0}...", m_Spec.Name);

        VFS::Mount("/assets", "assets");

        if (m_Spec.Type == ApplicationType::Server)
        {
            AETHER_CORE_INFO("Running in HEADLESS mode (Server Type Detected).");
        }
        else
        {
            std::string fullTitle = windowSettings.Title + " [" + EngineVersion::ToString() + "]";
            WindowProps props(fullTitle, windowSettings.Width, windowSettings.Height, windowSettings.Mode);
            props.VSync = windowSettings.VSync;

            m_Window = std::unique_ptr<Window>(Window::Create(props));
            m_Window->SetEventCallback(AETHER_BIND_EVENT_FN(Engine::OnEvent));

            Renderer2D::Init();
        }
    }

    Engine::~Engine()
    {
        Renderer2D::Shutdown();
    }

    void Engine::PushLayer(Layer* layer)
    {
        // Queue the operation to avoid iterator invalidation
        m_LayerOperations.emplace_back([this, layer]() {
            m_LayerStack.PushLayer(layer);
            });
    }

    void Engine::PushOverlay(Layer* layer)
    {
        // Queue the operation
        m_LayerOperations.emplace_back([this, layer]() {
            m_LayerStack.PushOverlay(layer);
            });
    }

    void Engine::PopLayer(Layer* layer)
    {
        // Queue the operation
        m_LayerOperations.emplace_back([this, layer]() {
            m_LayerStack.PopLayer(layer);
            });
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(AETHER_BIND_EVENT_FN(Engine::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& e) {
            Renderer2D::OnWindowResize(e.GetWidth(), e.GetHeight());
            return false;
            });

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled) break;
            (*it)->OnEvent(e);
        }
    }

    void Engine::SetWorld(std::unique_ptr<World> newWorld) {
        m_ActiveWorld = std::move(newWorld);
        m_ActiveWorld->OnRuntimeStart();
        AETHER_CORE_INFO("World Loaded: {0}", m_ActiveWorld->GetName());
    }

    void Engine::Run()
    {
        static bool s_WarnedNoCamera = false;

        while (m_Running)
        {
            // --- 1. PROCESS PENDING COMMANDS (Safe Zone) ---
            if (!m_LayerOperations.empty()) {
                for (auto& op : m_LayerOperations) {
                    op(); // Executes LayerStack::Push, which calls OnAttach
                }
                m_LayerOperations.clear();
            }

            AetherTime::Update();
            float timestep = (float)AetherTime::DeltaTime();

            // --- CAMERA LOGIC ---
            bool cameraFound = false;
            glm::mat4 cameraMatrix = glm::mat4(1.0f);

#if 1 
            if (m_ActiveWorld && m_Spec.Type != ApplicationType::Server)
            {
                auto& registry = m_ActiveWorld->GetRegistry();
                auto& camView = registry.View<CameraComponent>();
                auto& ownerMap = registry.GetOwnerMap<CameraComponent>();

                for (size_t i = 0; i < camView.size(); i++)
                {
                    auto& camComp = camView[i];

                    if (camComp.Primary)
                    {
                        EntityID id = ownerMap.at(i);
                        if (registry.HasComponent<TransformComponent>(id))
                        {
                            auto* camTransform = registry.GetComponent<TransformComponent>(id);

                            float aspectRatio = 1.777f;
                            if (m_Window) {
                                aspectRatio = (float)m_Window->GetWidth() / (float)m_Window->GetHeight();
                            }

                            glm::mat4 projection;
                            if (camComp.ProjectionType == CameraComponent::Type::Perspective)
                            {
                                projection = CameraUtils::CalculatePerspective(
                                    camComp.PerspectiveFOV,
                                    aspectRatio,
                                    camComp.PerspectiveNear,
                                    camComp.PerspectiveFar
                                );
                            }
                            else
                            {
                                projection = CameraUtils::CalculateOrthographic(
                                    camComp.OrthographicSize,
                                    aspectRatio,
                                    camComp.OrthographicNear,
                                    camComp.OrthographicFar
                                );
                            }

                            glm::mat4 view = CameraUtils::CalculateView(
                                { camTransform->X, camTransform->Y, 0.0f },
                                camTransform->Rotation
                            );

                            cameraMatrix = projection * view;
                            cameraFound = true;
                            s_WarnedNoCamera = false;
                            break;
                        }
                    }
                }
            }
#endif

            if (!cameraFound && !s_WarnedNoCamera && m_Spec.Type == ApplicationType::Client) {
                AETHER_CORE_WARN("No Primary Camera found! Using default Identity View.");
                s_WarnedNoCamera = true;
            }

            if (m_ActiveWorld) {
                m_ActiveWorld->OnUpdate(timestep, cameraMatrix);
            }

            // Iterate Layers (Safe now because stack queue is flushed)
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            if (m_ImGuiLayer)
            {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

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