#pragma once
#include "Registry.h"
#include "../core/Log.h" // Ensure logging macros are available

namespace aether {
    /* Represents an entity in the ECS system. Provides methods to add, remove, and access components.
    * An Entity is essentially a lightweight handle (ID + Registry pointer) to interact with the ECS.
    */
    class Entity {
    public:
        Entity() = default;
        Entity(EntityID id, Registry* registry)
            : m_EntityID(id), m_Registry(registry) {
        }

        // --- ADD COMPONENT ---

        // Overload 1: Create default component, add it, and return handle
        // Usage: auto& tag = entity.AddComponent<TagComponent>();
        template<typename T>
        T& AddComponent() {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");

            T component{}; // Default construct
            m_Registry->AddComponent<T>(m_EntityID, component);

            // Return the reference from the pool so the user can modify it immediately
            return m_Registry->GetComponent<T>(m_EntityID);
        }

        // Overload 2: Copy existing component, add it, and return handle
        // Usage: entity.AddComponent<TransformComponent>(myTransform);
        template<typename T>
        T& AddComponent(const T& component) {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");

            m_Registry->AddComponent<T>(m_EntityID, component);
            return m_Registry->GetComponent<T>(m_EntityID);
        }

        // --- REMOVE COMPONENT ---

        template<typename T>
        void RemoveComponent() {
            AETHER_ASSERT(m_Registry, "Cannot remove component from null entity!");
            AETHER_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            m_Registry->RemoveComponent<T>(m_EntityID);
        }

        // --- GET COMPONENT ---

        template<typename T>
        T& GetComponent() {
            AETHER_ASSERT(m_Registry, "Cannot get component from null entity!");

            // Strict check: crashing here is better than returning garbage memory
            bool hasComponent = m_Registry->HasComponent<T>(m_EntityID);
            AETHER_ASSERT(hasComponent, "Entity {0} does not have component '{1}'!", (uint32_t)m_EntityID, typeid(T).name());

            return m_Registry->GetComponent<T>(m_EntityID);
        }

        // --- UTILS ---

        template<typename T>
        bool HasComponent() {
            if (!m_Registry) return false;
            return m_Registry->HasComponent<T>(m_EntityID);
        }

        EntityID GetID() const { return m_EntityID; }

        // Allows "if (entity)" checks
        operator bool() const { return m_EntityID != -1 && m_Registry != nullptr; }

        // Comparison operators for standard containers/checks
        bool operator==(const Entity& other) const {
            return m_EntityID == other.m_EntityID && m_Registry == other.m_Registry;
        }
        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }

    private:
        EntityID m_EntityID = -1;
        Registry* m_Registry = nullptr;
    };
}