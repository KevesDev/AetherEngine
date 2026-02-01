#pragma once

#include "Scene.h"
#include <memory>
#include <string>

namespace aether {

    class World {
    public:
        // Default to "New World" if no name provided
        World(const std::string& name = "New World");
        ~World();

        // The Master Simulation Loop for this Map
        void OnUpdate(double dt);

        // Accessors
        Scene* GetScene() { return m_Scene.get(); }
        const Scene* GetScene() const { return m_Scene.get(); }
        const std::string& GetName() const { return m_Name; }

    private:
        std::string m_Name;

        // Ownership: World owns the Scene (ECS Registry)
        std::unique_ptr<Scene> m_Scene;

        // Future:
        // std::unique_ptr<PartitionGrid> m_Grid;
        // std::unique_ptr<PhysicsWorld> m_Physics; we don't use REAL physics: see documentation
    };
}