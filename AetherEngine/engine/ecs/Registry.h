#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <algorithm>
#include "../core/Log.h"

namespace aether {

    using EntityID = uint32_t;

    // Interface ensures we can store different Component Pools in one list
	// and call Remove/Has without knowing the type T
    struct IPool {
        virtual ~IPool() = default;
        virtual void Remove(EntityID entity) = 0;
        virtual bool Has(EntityID entity) = 0;
    };

    // A Pool stores ONE type of component for ALL entities (Contiguous Memory = Fast)
	// Uses "Fast Removal" technique to avoid holes in the array
    template<typename T>
    class ComponentPool : public IPool {
    public:
        std::vector<T> Data;

        // Maps to keep track of which Entity owns which index in the vector
        std::unordered_map<EntityID, size_t> EntityToIndex;
        std::unordered_map<size_t, EntityID> IndexToEntity;

        void Add(EntityID entity, T component) {
            if (EntityToIndex.find(entity) != EntityToIndex.end()) {
                // Entity already has this component, just update it
                Data[EntityToIndex[entity]] = component;
                return;
            }

            Data.push_back(component);
            size_t index = Data.size() - 1;
            EntityToIndex[entity] = index;
            IndexToEntity[index] = entity;
        }

        void Remove(EntityID entity) override {
            if (EntityToIndex.find(entity) == EntityToIndex.end()) return;

            // Fast Removal: Swap the removed element with the last element, then pop back.
            // This avoids "holes" in the array.
            size_t removedIndex = EntityToIndex[entity];
            size_t lastIndex = Data.size() - 1;

            if (removedIndex != lastIndex) {
                T lastElement = Data[lastIndex];
                EntityID lastEntity = IndexToEntity[lastIndex];

                Data[removedIndex] = lastElement;
                EntityToIndex[lastEntity] = removedIndex;
                IndexToEntity[removedIndex] = lastEntity;
            }

            Data.pop_back();
            EntityToIndex.erase(entity);
            IndexToEntity.erase(lastIndex);
        }

        T* Get(EntityID entity) {
            auto it = EntityToIndex.find(entity);
            if (it == EntityToIndex.end()) return nullptr;
            return &Data[it->second];
        }

        bool Has(EntityID entity) override {
            return EntityToIndex.find(entity) != EntityToIndex.end();
        }
    };

	// The Registry manages all entities and their components
	// Provides methods to create/destroy entities and add/remove/get components
    class Registry {
    public:
        EntityID CreateEntity() {
            return m_NextEntityID++;
        }

        void DestroyEntity(EntityID entity) {
            // Remove this entity from ALL pools
            for (auto& pair : m_ComponentPools) {
                pair.second->Remove(entity);
            }
        }

        template<typename T>
        void AddComponent(EntityID entity, T component) {
            GetPool<T>()->Add(entity, component);
        }

        template<typename T>
        void RemoveComponent(EntityID entity) {
            GetPool<T>()->Remove(entity);
        }

        template<typename T>
        T* GetComponent(EntityID entity) {
            return GetPool<T>()->Get(entity);
        }

        template<typename T>
        bool HasComponent(EntityID entity) {
            return GetPool<T>()->Has(entity);
        }

        // Returns the raw vector of components for extremely fast iteration
        template<typename T>
        std::vector<T>& View() {
            return GetPool<T>()->Data;
        }

        // Returns the list of Entity IDs associated with the components in View()
        // Useful if you need to know WHICH entity owns the component during iteration
        template<typename T>
        const std::unordered_map<size_t, EntityID>& GetOwnerMap() {
            return GetPool<T>()->IndexToEntity;
        }

    private:
        EntityID m_NextEntityID = 0;
        std::unordered_map<std::type_index, std::shared_ptr<IPool>> m_ComponentPools;

        template<typename T>
        std::shared_ptr<ComponentPool<T>> GetPool() {
            std::type_index type = std::type_index(typeid(T));
            if (m_ComponentPools.find(type) == m_ComponentPools.end()) {
                m_ComponentPools[type] = std::make_shared<ComponentPool<T>>();
            }
            return std::static_pointer_cast<ComponentPool<T>>(m_ComponentPools[type]);
        }
    };

}