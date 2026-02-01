#include "Scene.h"
#include "../ecs/Components.h"
#include "../ecs/Entity.h"
#include "../core/Log.h"
#include "../renderer/Renderer2D.h"
#include "../renderer/OrthographicCamera.h"
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

        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        // Every entity requires a Transform for spatial logic
        entity.AddComponent<TransformComponent>();

        AETHER_CORE_TRACE("Created Entity: '{0}' (ID: {1})", tag.Tag, (uint32_t)entity.GetID());
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        AETHER_ASSERT((bool)entity, "Attempted to destroy an invalid entity!");
        m_Registry.DestroyEntity(entity);
    }

    void Scene::OnUpdate(TimeStep ts) {
        // 1. Stability: Clamp DeltaTime to 100ms to prevent physics tunneling
        float dt = ts.GetSeconds();
        if (dt > 0.1f) {
            AETHER_CORE_WARN("High DeltaTime detected ({0}s). Clamped to 0.1s.", dt);
            dt = 0.1f;
        }

        // 2. Camera: Industry standard 720p Orthographic Projection
        // Static prevents re-allocation every frame; will move to Component in Phase 5
        static OrthographicCamera camera(0.0f, 1280.0f, 720.0f, 0.0f);

        Renderer2D::BeginScene(camera.GetViewProjectionMatrix());

        // 3. Render Sprites
        auto& registry = GetRegistry();
        auto& sprites = registry.View<SpriteComponent>();
        auto& owners = registry.GetOwnerMap<SpriteComponent>();

        for (size_t i = 0; i < sprites.size(); i++) {
            EntityID id = owners.at(i);

            if (!registry.HasComponent<TransformComponent>(id)) {
                AETHER_CORE_WARN("Entity {0} has a Sprite but no Transform. Skipping draw.", (uint32_t)id);
                continue;
            }

            auto& transform = registry.GetComponent<TransformComponent>(id);
            auto& sprite = sprites[i];

            // Render using world coordinates
            Renderer2D::DrawQuad(
                { transform.X, transform.Y },
                { transform.ScaleX, transform.ScaleY },
                { sprite.R, sprite.G, sprite.B, sprite.A }
            );
        }

        Renderer2D::EndScene();
    }
}