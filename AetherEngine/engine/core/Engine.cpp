#include "Engine.h"
#include "../renderer/Renderer2D.h"
#include "../renderer/CameraUtils.h"
#include "../ecs/Components.h"
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
            AETHER_ASSERT(m_Window, "Window failed to create!");

            // REFACTOR: Use Lambda instead of std::bind
            m_Window->SetEventCallback([this](Event& e) {
                this->OnEvent(e);
                });

            AETHER_CORE_INFO("Starting Renderer2D Initialization...");
            Renderer2D::Init();
        }
    }

    Engine::~Engine()
    {
        s_Instance = nullptr;
    }

    void Engine::PushLayer(Layer* layer) { m_LayerStack.PushLayer(layer); }

    void Engine::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        if (layer->GetName() == "ImGuiLayer") {
            m_ImGuiLayer = static_cast<ImGuiLayer*>(layer);
        }
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);

        // REFACTOR: Use Lambda instead of std::bind
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) {
            return this->OnWindowClose(e);
            });

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled) break;
            (*it)->OnEvent(e);
        }
    }

    void Engine::SetWorld(std::unique_ptr<World> newWorld)
    {
        m_ActiveWorld = std::move(newWorld);
        if (m_ActiveWorld) {
            AETHER_CORE_INFO("World Loaded: {0}", m_ActiveWorld->GetName());
        }
    }

    void Engine::Run()
    {
        AETHER_CORE_INFO("Aether Engine Loop Started.");
        AetherTime::Init();

        static bool s_WarnedNoCamera = false;

        while (m_Running) {
            AetherTime::Update();
            TimeStep timestep = AetherTime::DeltaTime();

            if (m_Window) {
                m_Window->Clear();
            }

            // --- CAMERA SYSTEM CALCULATION ---
            glm::mat4 cameraMatrix(1.0f);
            bool cameraFound = false;

#ifndef AETHER_SERVER
            if (m_ActiveWorld && m_Window) {
                if (m_Spec.Type == ApplicationType::Client) {
                    Scene* scene = m_ActiveWorld->GetScene();
                    auto& registry = scene->GetRegistry();

                    auto& cameras = registry.View<CameraComponent>();
                    auto& owners = registry.GetOwnerMap<CameraComponent>();

                    EntityID primaryCamID = (EntityID)-1;

                    for (size_t i = 0; i < cameras.size(); i++) {
                        if (cameras[i].Primary) {
                            primaryCamID = owners.at(i);
                            break;
                        }
                    }

                    if (primaryCamID != (EntityID)-1) {
                        auto* camData = registry.GetComponent<CameraComponent>(primaryCamID);
                        auto* camTransform = registry.GetComponent<TransformComponent>(primaryCamID);

                        if (camData && camTransform) {
                            float width = (float)m_Window->GetWidth();
                            float height = (float)m_Window->GetHeight();
                            float aspectRatio = (height > 0) ? (width / height) : 1.0f;

                            glm::mat4 projection = CameraUtils::CalculateProjection(camData->Size, aspectRatio, camData->Near, camData->Far);
                            glm::mat4 view = CameraUtils::CalculateView({ camTransform->X, camTransform->Y, 0.0f }, camTransform->Rotation);

                            cameraMatrix = projection * view;
                            cameraFound = true;
                            s_WarnedNoCamera = false;
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