#pragma once

#include "Scene.h"
#include "../core/AetherTime.h"
#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace aether {

    class World {
    public:
        World(const std::string& name = "World");
        ~World();

        void OnUpdate(TimeStep ts, const glm::mat4& viewProjection);

        // Declarations only (removes the C2084 error)
        Scene* GetScene();
        const std::string& GetName() const;

    private:
        std::string m_Name;
        std::unique_ptr<Scene> m_Scene;
    };

}