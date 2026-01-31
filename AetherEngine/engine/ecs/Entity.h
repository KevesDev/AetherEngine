#pragma once
#include "Registry.h"

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

        template<typename T>
        void AddComponent(const T& component) {
            if (m_Registry) m_Registry->AddComponent<T>(m_EntityID, component);
        }

        template<typename T>
        void RemoveComponent() {
            if (m_Registry) m_Registry->RemoveComponent<T>(m_EntityID);
        }

        template<typename T>
        T& GetComponent() {
            // TODO: Add an assert here if registry is null
            return m_Registry->GetComponent<T>(m_EntityID);
        }

        template<typename T>
        bool HasComponent() {
            if (!m_Registry) return false;
            return m_Registry->HasComponent<T>(m_EntityID);
        }

        EntityID GetID() const { return m_EntityID; }

        // Allows "if (entity)" checks
        operator bool() const { return m_EntityID != -1 && m_Registry != nullptr; }

    private:
        EntityID m_EntityID = -1;
        Registry* m_Registry = nullptr;
    };

}