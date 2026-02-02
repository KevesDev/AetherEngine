#pragma once

#include "../ecs/Registry.h"
#include "../core/AetherTime.h"
#include <string>
#include <glm/glm.hpp>

namespace aether {

    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        // --- Lifecycle ---
        // Refactored: Scene now accepts the camera matrix from the caller (Client/Editor)
        void OnUpdate(TimeStep ts, const glm::mat4& viewProjection);

        // --- Entity Management ---
        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        Registry& GetRegistry() { return m_Registry; }
        const Registry& GetRegistry() const { return m_Registry; }

    private:
        Registry m_Registry;

        friend class Entity;
    };
}