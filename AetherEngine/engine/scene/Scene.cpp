#include "Scene.h"
#include "../ecs/Components.h"
#include "../ecs/Entity.h"
#include "../core/Log.h"
#include "../renderer/Renderer2D.h"
#include <glm/glm.hpp>

namespace aether {

    Scene::Scene() {
        AETHER_CORE_INFO("Scene System Initialized");
    }

    Scene::~Scene() {
        AETHER_CORE_INFO("Scene System Shutdown");
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = { m_Registry.CreateEntity(), &m_Registry };

        // Every entity must have a Tag and Transform by engine standard
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        entity.AddComponent<TransformComponent>();

        AETHER_CORE_TRACE("Created Entity: '{0}' (ID: {1})", tag.Tag, (uint32_t)entity.GetID());
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        AETHER_ASSERT((bool)entity, "Attempted to destroy an invalid entity!");

        if (entity.HasComponent<TagComponent>()) {
            AETHER_CORE_TRACE("Destroying Entity: '{0}'", entity.GetComponent<TagComponent>().Tag);
        }

        m_Registry.DestroyEntity(entity);
    }

    void Scene::OnUpdate(double dt) {
        // 1. Stability: Clamp DeltaTime to 100ms to prevent physics/logic spirals
        if (dt > 0.1) {
            AETHER_CORE_WARN("DeltaTime high ({0}s); clamped to 0.1s.", dt);
            dt = 0.1;
        }

        // 2. Logic/Physics Phase (Stubs for future networking/scripts)

        // 3. Rendering Phase
        // Using Identity matrix until CameraComponent is implemented
        glm::mat4 viewProjection = glm::mat4(1.0f);
        Renderer2D::BeginScene(viewProjection);

        auto& registry = GetRegistry();
        auto& sprites = registry.View<SpriteComponent>();
        auto& owners = registry.GetOwnerMap<SpriteComponent>();

        for (size_t i = 0; i < sprites.size(); i++) {
            EntityID id = owners.at(i);

            // Safety check: Ensure the entity with a Sprite also has a Transform
            if (!registry.HasComponent<TransformComponent>(id)) {
                AETHER_CORE_WARN("Entity {0} has SpriteComponent but lacks TransformComponent! Skipping draw.", (uint32_t)id);
                continue;
            }

            auto& transform = registry.GetComponent<TransformComponent>(id);
            auto& sprite = sprites[i];

            // Submit to GPU. Normalized coordinates (/100.0f) used until Camera System Phase.
            Renderer2D::DrawQuad(
                { transform.X / 100.0f, transform.Y / 100.0f },
                { transform.ScaleX / 100.0f, transform.ScaleY / 100.0f },
                { sprite.R, sprite.G, sprite.B, sprite.A }
            );
        }

        Renderer2D::EndScene();
    }

}