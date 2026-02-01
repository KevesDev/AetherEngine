#include "World.h"
#include "../core/Log.h"

namespace aether {

    World::World(const std::string& name)
        : m_Name(name)
    {
        AETHER_CORE_INFO("-------------------");
        AETHER_CORE_INFO("Initializing World: {0}", m_Name);

        // 1. Initialize ECS
        m_Scene = std::make_unique<Scene>();

        // 2. (Future) Initialize Physics/Partitioning
        // m_Grid = std::make_unique<PartitionGrid>(...);
    }

    World::~World() {
        AETHER_CORE_INFO("Unloading World: {0}", m_Name);
        AETHER_CORE_INFO("-------------------");
        // m_Scene is automatically destroyed here by unique_ptr
    }

    void World::OnUpdate(double dt) {
        // 1. Update Subsystems
        if (m_Scene)
            m_Scene->OnUpdate(dt);

        // 2. (Future) Update Streaming
        // if (m_Grid) m_Grid->Update(dt);
    }
}