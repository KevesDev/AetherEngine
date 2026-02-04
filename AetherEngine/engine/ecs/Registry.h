#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <algorithm>
#include <tuple>
#include "../core/Log.h"

namespace aether {

    using EntityID = uint32_t;

    struct IPool {
        virtual ~IPool() = default;
        virtual void Remove(EntityID entity) = 0;
        virtual bool Has(EntityID entity) = 0;
        virtual size_t Size() const = 0;
    };

    template<typename T>
    class ComponentPool : public IPool {
    public:
        std::vector<T> Data;
        std::unordered_map<EntityID, size_t> EntityToIndex;
        std::unordered_map<size_t, EntityID> IndexToEntity;

        void Add(EntityID entity, T component) {
            if (EntityToIndex.find(entity) != EntityToIndex.end()) {
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

        size_t Size() const override { return Data.size(); }
    };

    class Registry;

    // View class handles multi-component intersection logic
    template<typename... Components>
    class RegistryView {
    public:
        RegistryView(Registry* registry, std::shared_ptr<IPool> smallestPool, std::vector<std::shared_ptr<IPool>> otherPools)
            : m_Registry(registry), m_SmallestPool(smallestPool), m_OtherPools(otherPools) {
        }

        struct Iterator {
            Iterator(Registry* registry, std::shared_ptr<IPool> pool, std::vector<std::shared_ptr<IPool>> others, size_t index)
                : m_Registry(registry), m_Pool(pool), m_Others(others), m_Index(index) {
                if (m_Index < m_Pool->Size() && !Valid()) ++(*this);
            }

            // Yields the EntityID. Entity.h will handle the conversion to Entity handle.
            EntityID operator*() const {
                auto pool = std::static_pointer_cast<ComponentPool<typename std::tuple_element<0, std::tuple<Components...>>::type>>(m_Pool);
                return pool->IndexToEntity.at(m_Index);
            }

            Iterator& operator++() {
                do {
                    m_Index++;
                } while (m_Index < m_Pool->Size() && !Valid());
                return *this;
            }

            bool operator!=(const Iterator& other) const { return m_Index != other.m_Index; }

        private:
            bool Valid() {
                auto pool = std::static_pointer_cast<ComponentPool<typename std::tuple_element<0, std::tuple<Components...>>::type>>(m_Pool);
                EntityID entity = pool->IndexToEntity.at(m_Index);
                for (auto& other : m_Others) {
                    if (!other->Has(entity)) return false;
                }
                return true;
            }

            Registry* m_Registry;
            std::shared_ptr<IPool> m_Pool;
            std::vector<std::shared_ptr<IPool>> m_Others;
            size_t m_Index;
        };

        Iterator begin() { return Iterator(m_Registry, m_SmallestPool, m_OtherPools, 0); }
        Iterator end() { return Iterator(m_Registry, m_SmallestPool, m_OtherPools, m_SmallestPool->Size()); }

    private:
        Registry* m_Registry;
        std::shared_ptr<IPool> m_SmallestPool;
        std::vector<std::shared_ptr<IPool>> m_OtherPools;
    };

    class Registry {
    public:
        EntityID CreateEntity() { return m_NextEntityID++; }

        void DestroyEntity(EntityID entity) {
            for (auto& pair : m_ComponentPools) {
                pair.second->Remove(entity);
            }
        }

        template<typename T> void AddComponent(EntityID entity, T component) { GetPool<T>()->Add(entity, component); }
        template<typename T> void RemoveComponent(EntityID entity) { GetPool<T>()->Remove(entity); }
        template<typename T> T* GetComponent(EntityID entity) { return GetPool<T>()->Get(entity); }
        template<typename T> bool HasComponent(EntityID entity) { return GetPool<T>()->Has(entity); }

        // Variadic View
        template<typename... T>
        RegistryView<T...> view() {
            std::vector<std::shared_ptr<IPool>> pools = { GetPool<T>()... };
            auto smallest = std::min_element(pools.begin(), pools.end(), [](auto a, auto b) {
                return a->Size() < b->Size();
                });

            std::shared_ptr<IPool> smallestPool = *smallest;
            pools.erase(smallest); // Move others to the check-list
            return RegistryView<T...>(this, smallestPool, pools);
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