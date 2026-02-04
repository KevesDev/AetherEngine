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
        template<typename T>
        T& AddComponent() {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");
            T component{};
            m_Registry->AddComponent<T>(m_EntityID, component);
            return GetComponent<T>();
        }

        template<typename T>
        T& AddComponent(const T& component) {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");
            m_Registry->AddComponent<T>(m_EntityID, component);
            return GetComponent<T>();
        }

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            AETHER_ASSERT(m_Registry, "Cannot add component to null entity!");
            m_Registry->AddComponent<T>(m_EntityID, T{ std::forward<Args>(args)... });
            return GetComponent<T>();
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
            T* component = m_Registry->GetComponent<T>(m_EntityID);
            AETHER_ASSERT(component, "Entity {0} does not have component!", (uint32_t)m_EntityID);
            return *component;
        }

        // --- UTILS ---
        template<typename T>
        bool HasComponent() {
            return m_Registry && m_Registry->HasComponent<T>(m_EntityID);
        }

        EntityID GetID() const { return m_EntityID; }
        Registry* GetRegistry() const { return m_Registry; }

        operator bool() const { return m_EntityID != (EntityID)-1 && m_Registry != nullptr; }
        bool operator==(const Entity& other) const { return m_EntityID == other.m_EntityID && m_Registry == other.m_Registry; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        EntityID m_EntityID = (EntityID)-1;
        Registry* m_Registry = nullptr;
    };
}