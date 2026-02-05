#include "World.h"
#include "../core/Log.h"

namespace aether {

    World::World(const std::string& name)
        : m_Name(name)
    {
        AETHER_CORE_INFO("-------------------");
        AETHER_CORE_INFO("Initializing World: {0}", m_Name);

        m_Scene = std::make_unique<Scene>();
    }

    World::~World() {
        AETHER_CORE_INFO("Unloading World: {0}", m_Name);
        AETHER_CORE_INFO("-------------------");
    }

    void World::OnUpdate(TimeStep ts, const glm::mat4& viewProjection) {
        // The World is the high-level container that delegates
        // simulation and rendering to the Scene.
        //
        // IMPORTANT:
        // - ts is the variable frame delta from the Engine frame clock.
        // - Authoritative logic executes inside Scene::OnUpdateSimulation,
        //   which uses SystemScheduler with a fixed timestep.
        if (m_Scene) {
            m_Scene->OnUpdateSimulation(ts.GetSeconds());
            m_Scene->OnRender(viewProjection);
        }
    }

    // Returns the registry from the internal scene
    Registry& World::GetRegistry() {
        return m_Scene->GetRegistry();
    }

    // Accessors required for Serializers and Wrappers
    Scene* World::GetScene() { return m_Scene.get(); }
    const std::string& World::GetName() const { return m_Name; }
}