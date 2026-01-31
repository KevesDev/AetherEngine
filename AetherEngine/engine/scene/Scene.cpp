#include "Scene.h"
#include "../ecs/Components.h"
#include "../ecs/Entity.h"
#include "../core/Log.h"

namespace aether {

    Scene::Scene() {
        AETHER_CORE_INFO("Scene System Initialized");
    }

    Scene::~Scene() {
        // Registry cleans itself up automatically, but we log the shutdown
        AETHER_CORE_INFO("Scene System Shutdown");
    }

    Entity Scene::CreateEntity(const std::string& name) {
        // 1. Raw Data Allocation
        Entity entity = { m_Registry.CreateEntity(), &m_Registry };

        // 2. Enforce Engine Standards (Every entity MUST have these)
        // This prevents "invisible" or "untraceable" entities in the system.
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        entity.AddComponent<TransformComponent>();

        AETHER_CORE_TRACE("Created Entity: '{0}' (ID: {1})", tag.Tag, (uint32_t)entity.GetID());

        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        // strict error checking:
        // We cannot destroy what does not exist.
        AETHER_ASSERT((bool)entity, "Attempted to destroy a null or invalid entity!");

        // Log it so we can trace "disappearing" objects during debugging
        if (entity.HasComponent<TagComponent>()) {
            AETHER_CORE_TRACE("Destroying Entity: '{0}'", entity.GetComponent<TagComponent>().Tag);
        }

        m_Registry.DestroyEntity(entity);
    }

    void Scene::OnUpdate(double dt) {
        // --- Production Guardrail: DeltaTime Clamping ---
        // If the game pauses (debugging) or hangs, dt could be huge (e.g., 5.0s).
        // This would break physics and cause objects to clip through walls.
        // We clamp it to a maximum of 100ms per frame.
        if (dt > 0.1) {
            AETHER_CORE_WARN("High DeltaTime detected ({0}s). Clamped to 0.1s to preserve physics stability.", dt);
            dt = 0.1;
        }

        // --- 1. Logic Phase (Scripting) ---
                // Iterate over entities with 'NativeScriptComponent' or 'LogicGraphComponent'.
                // This is where the user's "RPGMaker" events (e.g., "OnInteract") will trigger.

                // --- 2. Movement & Collision Phase (Deterministic) ---
                // Replacing standard "Physics" with RPG-style Collision.
                // We will iterate TransformComponent + ColliderComponent.
                // calculated_pos = transform.pos + (velocity * dt);
                // if (!CheckCollision(calculated_pos)) transform.pos = calculated_pos;

                // --- 3. Network Replication Phase ---
                // We iterate over all entities that have a TransformComponent (or any replicated component).
                // The 'ReplicationMode' enum dictates behavior:

                // auto view = m_Registry.view<TransformComponent>();
                // for (auto entity : view) {
                //     auto& transform = view.get<TransformComponent>(entity);
                //
                //     switch (transform.Replication) {
                //         case ReplicationMode::None: 
                //             continue; // Local only
                //
                //         case ReplicationMode::ServerToAll:
                //             // Todo: Add to "GlobalSnapshot" packet
                //             break;
                //
                //         case ReplicationMode::ServerToOwner:
                //             // Todo: Add to "PlayerSpecific" packet
                //             break;
                //
                //         case ReplicationMode::InitialOnly:
                //             // Handled only during 'Entity Created' event, ignored here.
                //             break;
                //     }
                // }
    }
}