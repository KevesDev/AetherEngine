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
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        entity.AddComponent<TransformComponent>();

        AETHER_CORE_TRACE("Created Entity: '{0}' (ID: {1})", tag.Tag, (uint32_t)entity.GetID());
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        AETHER_ASSERT((bool)entity, "Attempted to destroy an invalid entity!");
        m_Registry.DestroyEntity(entity);
    }

    void Scene::OnUpdate(TimeStep ts, const glm::mat4& viewProjection) {
        float dt = ts.GetSeconds();
        if (dt > 0.1f) dt = 0.1f;

        // --- Logic Systems ---
        // (Physics, Scripts, and other non-graphical systems run here on both Client and Server)

        // --- Rendering System (Client/Editor Only) ---
#ifndef AETHER_SERVER
        Renderer2D::BeginScene(viewProjection);

        auto& registry = GetRegistry();
        auto& sprites = registry.View<SpriteComponent>();
        auto& owners = registry.GetOwnerMap<SpriteComponent>();

        for (size_t i = 0; i < sprites.size(); i++) {
            EntityID id = owners.at(i);

            // Refactored for Pointer Safety:
            // We fetch the transform pointer. if the entity has a sprite but no transform, 
            // the pointer will be null, and we safely skip drawing.
            auto* transform = registry.GetComponent<TransformComponent>(id);
            if (!transform) continue;

            auto& sprite = sprites[i];

            // Use -> access because transform is now a pointer
            Renderer2D::DrawQuad(
                { transform->X, transform->Y },
                { transform->ScaleX, transform->ScaleY },
                { sprite.R, sprite.G, sprite.B, sprite.A }
            );
        }

        Renderer2D::EndScene();
#endif
    }
}