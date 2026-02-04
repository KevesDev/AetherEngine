#include "Scene.h"
#include "SceneSerializer.h"

#include "../ecs/Components.h"
#include "../renderer/Renderer2D.h"
#include "../ecs/Entity.h"
#include "../core/systems/SystemRegistry.h"

#include <glm/glm.hpp>
#include <sstream>

namespace aether {

    Scene::Scene() {
        // Default to InputSystem for new scenes to ensure playability.
        m_SystemConfigs.push_back("InputSystem");
    }

    std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> other) {
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

        newScene->m_SystemConfigs = other->m_SystemConfigs;

        // Deep Copy via Memory Serialization
        // This ensures that the runtime copy is bit-exact to the edit state.
        // It leverages the existing Serializer logic, reducing code duplication.
        std::stringstream ss;
        SceneSerializer srcSerializer(other);
        srcSerializer.SerializeToStream(ss);

        SceneSerializer dstSerializer(newScene);
        dstSerializer.DeserializeFromStream(ss);

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& name) {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
        Entity entity = { m_Registry.CreateEntity(), &m_Registry };

        // IDComponent is the anchor for all persistence. Must be added first.
        entity.AddComponent<IDComponent>(uuid);

        entity.AddComponent<TransformComponent>();

        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.DestroyEntity(entity);
    }

    void Scene::EnsureSystemsLoaded() {
        if (m_SystemsLoaded) return;

        for (const auto& systemName : m_SystemConfigs) {
            auto system = SystemRegistry::Create(systemName);
            if (system) {
                // Determine System Group
                // In the future, systems will self-report their group.
                SystemGroup group = SystemGroup::Logic;
                if (systemName == "InputSystem") group = SystemGroup::Input;

                m_Scheduler.AddSystem(group, std::move(system));
            }
        }
        m_SystemsLoaded = true;
    }

    void Scene::OnUpdateSimulation(float deltaTime) {
        EnsureSystemsLoaded();
        // Pure Logic Step. No Rendering.
        m_Scheduler.Update(m_Registry, deltaTime);
    }

    void Scene::OnRender(const glm::mat4& viewProjection) {
        // Pure Presentation Step.
        Renderer2D::BeginScene(viewProjection);

        auto group = m_Registry.view<TransformComponent, SpriteComponent>();
        for (auto entityID : group) {
            // Registry view yields EntityIDs, we access components via the Registry directly for speed
            auto* transform = m_Registry.GetComponent<TransformComponent>(entityID);
            auto* sprite = m_Registry.GetComponent<SpriteComponent>(entityID);

            if (transform && sprite) {
                glm::vec4 color = { sprite->R, sprite->G, sprite->B, sprite->A };
                Renderer2D::DrawQuad(transform->GetTransform(), color);
            }
        }

        Renderer2D::EndScene();
    }

    glm::mat4 Scene::GetPrimaryCameraViewProjection() {
        auto view = m_Registry.view<CameraComponent, TransformComponent>();
        for (auto entityID : view) {
            auto* camera = m_Registry.GetComponent<CameraComponent>(entityID);
            if (camera && camera->Primary) {
                auto* transform = m_Registry.GetComponent<TransformComponent>(entityID);
                if (transform) {
                    return camera->GetProjection() * glm::inverse(transform->GetTransform());
                }
            }
        }
        return glm::mat4(1.0f);
    }

    Entity Scene::GetPrimaryCameraEntity() {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entityID : view) {
            auto* camera = m_Registry.GetComponent<CameraComponent>(entityID);
            if (camera && camera->Primary) {
                return { entityID, &m_Registry };
            }
        }
        return {};
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height) {
        if (height == 0) return;

        float aspectRatio = (float)width / (float)height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entityID : view) {
            auto* camera = m_Registry.GetComponent<CameraComponent>(entityID);
            if (camera && !camera->FixedAspectRatio) {
                camera->AspectRatio = aspectRatio;
            }
        }
    }
}