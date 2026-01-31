#pragma once

#include "../ecs/Registry.h"
#include "../core/AetherTime.h"
#include <string>

namespace aether {

    class Entity; // Forward declaration to avoid circular include hell

    class Scene {
    public:
        Scene();
        ~Scene();

        // --- Lifecycle ---
        void OnUpdate(double dt);

        // --- Entity Management ---
        // Creates an entity with a UUID and default components
        Entity CreateEntity(const std::string& name = std::string());

        // Safely marks an entity for destruction at the end of the frame
        void DestroyEntity(Entity entity);

        // --- Getters ---
        Registry& GetRegistry() { return m_Registry; }
        const Registry& GetRegistry() const { return m_Registry; }

    private:
        Registry m_Registry;

        // TODO: Add Friend classes for Serializer and HierarchyPanel later
        friend class Entity;
    };

}