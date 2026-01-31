#pragma once
#include "Registry.h"
#include "../core/Log.h" 

namespace aether {

    class Entity {
    public:
        Entity() = default;
        Entity(EntityID id, Registry* registry)
            : m_EntityID(id), m_Registry(registry) {
        }

        // --- ADD COMPONENT ---

        // Overload 1: Create default component
        template<typename T>
        T& AddComponent() {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");
            T component{};
            m_Registry->AddComponent<T>(m_EntityID, component);
            return m_Registry->GetComponent<T>(m_EntityID);
        }

        // Overload 2: Copy existing component
        template<typename T>
        T& AddComponent(const T& component) {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");
            m_Registry->AddComponent<T>(m_EntityID, component);
            return m_Registry->GetComponent<T>(m_EntityID);
        }

        // --- Overload 3: Construct component in-place (Variadic) ---
        // Usage: entity.AddComponent<SpriteComponent>(1.0f, 0.0f, 0.0f, 1.0f);
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");

            // Construct T using the provided arguments
            // We use braces {} to support simple Structs (Aggregates) as well as Classes
            T component{ std::forward<Args>(args)... };

            // Delegate to the standard Add function
            m_Registry->AddComponent<T>(m_EntityID, component);
            return m_Registry->GetComponent<T>(m_EntityID);
        }
        // -------------------------------------------------------------

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
        operator bool() const { return m_EntityID != -1 && m_Registry != nullptr; }

        bool operator==(const Entity& other) const { return m_EntityID == other.m_EntityID && m_Registry == other.m_Registry; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        EntityID m_EntityID = -1;
        Registry* m_Registry = nullptr;
    };
}